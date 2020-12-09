/* This file is part of the KDE project
 *
 *  Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_manager.h"

#include <kis_debug.h>
#include <QQueue>
#include <klocalizedstring.h>
#include <QApplication>
#include <QTouchEvent>
#include <QElapsedTimer>

#include <KoToolManager.h>

#include "kis_tool_proxy.h"

#include <kis_config.h>
#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_canvas_resource_provider.h>
#include <kis_favorite_resource_manager.h>

#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_pan_action.h"
#include "kis_alternate_invocation_action.h"
#include "kis_rotate_canvas_action.h"
#include "kis_zoom_action.h"
#include "kis_show_palette_action.h"
#include "kis_change_primary_setting_action.h"

#include "kis_shortcut_matcher.h"
#include "kis_stroke_shortcut.h"
#include "kis_single_action_shortcut.h"
#include "kis_touch_shortcut.h"

#include "kis_input_profile.h"
#include "kis_input_profile_manager.h"
#include "kis_shortcut_configuration.h"

#include <input/kis_tablet_debugger.h>
#include <kis_signal_compressor.h>

#include "kis_extended_modifiers_mapper.h"
#include "kis_input_manager_p.h"
#include "kis_algebra_2d.h"

template <typename T>
uint qHash(QPointer<T> value) {
    return reinterpret_cast<quintptr>(value.data());
}

KisInputManager::KisInputManager(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    d->setupActions();

    connect(KoToolManager::instance(), SIGNAL(aboutToChangeTool(KoCanvasController*)), SLOT(slotAboutToChangeTool()));
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)), SLOT(slotToolChanged()));
    connect(&d->moveEventCompressor, SIGNAL(timeout()), SLOT(slotCompressedMoveEvent()));


    QApplication::instance()->
            installEventFilter(new Private::ProximityNotifier(d, this));

    // on macos global Monitor listen to keypresses when krita is not in focus
    // and local monitor listen presses when krita is in focus.
#ifdef Q_OS_MACOS
    KisExtendedModifiersMapper::setGlobalMonitor(true);
    KisExtendedModifiersMapper::setLocalMonitor(true, &d->matcher);
#endif
}

KisInputManager::~KisInputManager()
{
#ifdef Q_OS_MACOS
    KisExtendedModifiersMapper::setGlobalMonitor(false);
    KisExtendedModifiersMapper::setLocalMonitor(false);
#endif
    delete d;
}

void KisInputManager::addTrackedCanvas(KisCanvas2 *canvas)
{
    d->canvasSwitcher.addCanvas(canvas);
}

void KisInputManager::removeTrackedCanvas(KisCanvas2 *canvas)
{
    d->canvasSwitcher.removeCanvas(canvas);
}

void KisInputManager::toggleTabletLogger()
{
    KisTabletDebugger::instance()->toggleDebugging();
}

void KisInputManager::attachPriorityEventFilter(QObject *filter, int priority)
{
    Private::PriorityList::iterator begin = d->priorityEventFilter.begin();
    Private::PriorityList::iterator it = begin;
    Private::PriorityList::iterator end = d->priorityEventFilter.end();

    it = std::find_if(begin, end,
                      [filter] (const Private::PriorityPair &a) { return a.second == filter; });

    if (it != end) return;

    it = std::find_if(begin, end,
                      [priority] (const Private::PriorityPair &a) { return a.first > priority; });

    d->priorityEventFilter.insert(it, qMakePair(priority, filter));
    d->priorityEventFilterSeqNo++;
}

void KisInputManager::detachPriorityEventFilter(QObject *filter)
{
    Private::PriorityList::iterator it = d->priorityEventFilter.begin();
    Private::PriorityList::iterator end = d->priorityEventFilter.end();

    it = std::find_if(it, end,
                      [filter] (const Private::PriorityPair &a) { return a.second == filter; });

    if (it != end) {
        d->priorityEventFilter.erase(it);
    }
}

void KisInputManager::setupAsEventFilter(QObject *receiver)
{
    if (d->eventsReceiver) {
        d->eventsReceiver->removeEventFilter(this);
    }

    d->eventsReceiver = receiver;

    if (d->eventsReceiver) {
        d->eventsReceiver->installEventFilter(this);
    }
}

#if defined (__clang__)
#pragma GCC diagnostic ignored "-Wswitch"
#endif

bool KisInputManager::eventFilter(QObject* object, QEvent* event)
{
    if (object != d->eventsReceiver) return false;

    if (d->eventEater.eventFilter(object, event)) return false;

    if (!d->matcher.hasRunningShortcut()) {

        int savedPriorityEventFilterSeqNo = d->priorityEventFilterSeqNo;

        for (auto it = d->priorityEventFilter.begin(); it != d->priorityEventFilter.end(); /*noop*/) {
            const QPointer<QObject> &filter = it->second;

            if (filter.isNull()) {
                it = d->priorityEventFilter.erase(it);

                d->priorityEventFilterSeqNo++;
                savedPriorityEventFilterSeqNo++;
                continue;
            }

            if (filter->eventFilter(object, event)) return true;

            /**
             * If the filter removed itself from the filters list or
             * added something there, just exit the loop
             */
            if (d->priorityEventFilterSeqNo != savedPriorityEventFilterSeqNo) {
                return true;
            }

            ++it;
        }

        // KoToolProxy needs to pre-process some events to ensure the
        // global shortcuts (not the input manager's ones) are not
        // executed, in particular, this line will accept events when the
        // tool is in text editing, preventing shortcut triggering
        if (d->toolProxy) {
            d->toolProxy->processEvent(event);
        }
    }

    // Continue with the actual switch statement...
    return eventFilterImpl(event);
}

// Qt's events do not have copy-ctors yet, so we should emulate them
// See https://bugreports.qt.io/browse/QTBUG-72488

template <class Event> void copyEventHack(Event *src, QScopedPointer<QEvent> &dst);

template<> void copyEventHack(QMouseEvent *src, QScopedPointer<QEvent> &dst) {
    QMouseEvent *tmp = new QMouseEvent(src->type(),
                                       src->localPos(), src->windowPos(), src->screenPos(),
                                       src->button(), src->buttons(), src->modifiers(),
                                       src->source());
    tmp->setTimestamp(src->timestamp());
    dst.reset(tmp);
}

template<> void copyEventHack(QTabletEvent *src, QScopedPointer<QEvent> &dst) {
    QTabletEvent *tmp = new QTabletEvent(src->type(),
                                         src->posF(), src->globalPosF(),
                                         src->device(), src->pointerType(),
                                         src->pressure(),
                                         src->xTilt(), src->yTilt(),
                                         src->tangentialPressure(),
                                         src->rotation(),
                                         src->z(),
                                         src->modifiers(),
                                         src->uniqueId(),
                                         src->button(), src->buttons());
    tmp->setTimestamp(src->timestamp());
    dst.reset(tmp);
}

template<> void copyEventHack(QTouchEvent *src, QScopedPointer<QEvent> &dst) {
    QTouchEvent *tmp = new QTouchEvent(src->type(),
                                       src->device(),
                                       src->modifiers(),
                                       src->touchPointStates(),
                                       src->touchPoints());
    tmp->setTimestamp(src->timestamp());
    dst.reset(tmp);
}


template <class Event>
bool KisInputManager::compressMoveEventCommon(Event *event)
{
    /**
     * We construct a copy of this event object, so we must ensure it
     * has a correct type.
     */
    static_assert(std::is_same<Event, QMouseEvent>::value ||
                  std::is_same<Event, QTabletEvent>::value ||
                  std::is_same<Event, QTouchEvent>::value,
                  "event should be a mouse or a tablet event");

#ifdef Q_OS_WIN32
    /**
     * On Windows, when the user presses some global window manager shortcuts,
     * e.g. Alt+Space (to show window title menu), events for these key presses
     * and releases are not delivered (see bug 424319). This code is a workaround
     * for this problem. It checks consistency of standard modifiers and resets
     * shortcut's matcher state in case of a trouble.
     */
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseMove ||
        event->type() == QEvent::TabletMove ||
        event->type() == QEvent::TabletPress ||
        event->type() == QEvent::TabletRelease) {

        QInputEvent *inputEvent = static_cast<QInputEvent*>(event);
        if (!d->matcher.sanityCheckModifiersCorrectness(inputEvent->modifiers())) {
            qWarning() << "WARNING: modifiers state became inconsistent! Trying to fix that...";
            qWarning() << "    " << ppVar(inputEvent->modifiers());
            qWarning() << "    " << ppVar(d->matcher.debugPressedKeys());

            d->fixShortcutMatcherModifiersState();
        }
    }
#endif

    bool retval = false;

    /**
     * Compress the events if the tool doesn't need high resolution input
     */
    if ((event->type() == QEvent::MouseMove ||
         event->type() == QEvent::TabletMove ||
         event->type() == QEvent::TouchUpdate) &&
            (!d->matcher.supportsHiResInputEvents() ||
             d->testingCompressBrushEvents)) {

        copyEventHack(event, d->compressedMoveEvent);
        d->moveEventCompressor.start();

        /**
         * On Linux Qt eats the rest of unneeded events if we
         * ignore the first of the chunk of tablet events. So
         * generally we should never activate this feature. Only
         * for testing purposes!
         */
        if (d->testingAcceptCompressedTabletEvents) {
            event->setAccepted(true);
        }

        retval = true;
    } else {
        slotCompressedMoveEvent();
        retval = d->handleCompressedTabletEvent(event);
    }

    return retval;
}

bool shouldResetWheelDelta(QEvent * event)
{
    return
        event->type() == QEvent::FocusIn ||
        event->type() == QEvent::FocusOut ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::TabletPress ||
        event->type() == QEvent::TabletRelease ||
        event->type() == QEvent::Enter ||
        event->type() == QEvent::Leave ||
        event->type() == QEvent::TouchBegin ||
        event->type() == QEvent::TouchEnd ||
        event->type() == QEvent::TouchCancel ||
        event->type() == QEvent::NativeGesture;

}

bool KisInputManager::eventFilterImpl(QEvent * event)
{
    bool retval = false;

    if (shouldResetWheelDelta(event)) {
        d->accumulatedScrollDelta = 0;
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        d->debugEvent<QMouseEvent, true>(event);
        if (d->touchHasBlockedPressEvents) break;

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (d->tryHidePopupPalette()) {
            retval = true;
        } else {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.buttonPressed(mouseEvent->button(), mouseEvent);
        }
        //Reset signal compressor to prevent processing events before press late
        d->resetCompressor();
        event->setAccepted(retval);
        break;
    }
    case QEvent::MouseButtonRelease: {
        d->debugEvent<QMouseEvent, true>(event);
        if (d->touchHasBlockedPressEvents) break;

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = d->matcher.buttonReleased(mouseEvent->button(), mouseEvent);
        event->setAccepted(retval);
        break;
    }
    case QEvent::ShortcutOverride: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);

        if (!keyEvent->isAutoRepeat()) {
            retval = d->matcher.keyPressed(key);
        } else {
            retval = d->matcher.autoRepeatedKeyPressed(key);
        }

        // In case we matched ashortcut we should accept the event to
        // notify Qt that it shouldn't try to trigger its partially matched
        // shortcuts.
        if (retval) {
            keyEvent->setAccepted(true);
        }

        break;
    }
    case QEvent::KeyRelease: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (!keyEvent->isAutoRepeat()) {
            Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);
            retval = d->matcher.keyReleased(key);
        }
        break;
    }
    case QEvent::MouseMove: {
        d->debugEvent<QMouseEvent, true>(event);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = compressMoveEventCommon(mouseEvent);

        break;
    }
    case QEvent::Wheel: {
        d->debugEvent<QWheelEvent, false>(event);
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);

#ifdef Q_OS_MACOS
        // Some QT wheel events are actually touch pad pan events. From the QT docs:
        // "Wheel events are generated for both mouse wheels and trackpad scroll gestures."

        // We differentiate between touchpad events and real mouse wheels by inspecting the
        // event source.

        if (wheelEvent->source() == Qt::MouseEventSource::MouseEventSynthesizedBySystem) {
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.wheelEvent(KisSingleActionShortcut::WheelTrackpad, wheelEvent);
            break;
        }
#endif

        d->accumulatedScrollDelta += wheelEvent->delta();
        KisSingleActionShortcut::WheelAction action;

        /**
         * Ignore delta 0 events on OSX, since they are triggered by tablet
         * proximity when using Wacom devices.
         */
#ifdef Q_OS_MACOS
        if(wheelEvent->delta() == 0) {
            retval = true;
            break;
        }
#endif

        if (wheelEvent->orientation() == Qt::Horizontal) {
            if(wheelEvent->delta() < 0) {
                action = KisSingleActionShortcut::WheelRight;
            }
            else {
                action = KisSingleActionShortcut::WheelLeft;
            }
        }
        else {
            if(wheelEvent->delta() > 0) {
                action = KisSingleActionShortcut::WheelUp;
            }
            else {
                action = KisSingleActionShortcut::WheelDown;
            }
        }

        bool wasScrolled = false;

        while (qAbs(d->accumulatedScrollDelta) >= QWheelEvent::DefaultDeltasPerStep) {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.wheelEvent(action, wheelEvent);
            d->accumulatedScrollDelta -=
                KisAlgebra2D::signPZ(d->accumulatedScrollDelta) *
                QWheelEvent::DefaultDeltasPerStep;
            wasScrolled = true;
        }

        if (wasScrolled) {
            d->accumulatedScrollDelta = 0;
        }

        retval = !wasScrolled;
        break;
    }
#ifndef Q_OS_ANDROID
    case QEvent::Enter:
        d->debugEvent<QEvent, false>(event);
        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);
        if (!d->containsPointer) {
            d->containsPointer = true;

            d->allowMouseEvents();
            d->touchHasBlockedPressEvents = false;
        }
        d->matcher.enterEvent();
        break;
    case QEvent::Leave:
        d->debugEvent<QEvent, false>(event);
        d->containsPointer = false;
        /**
         * We won't get a TabletProximityLeave event when the tablet
         * is hovering above some other widget, so restore cursor
         * events processing right now.
         */
        d->allowMouseEvents();
        d->touchHasBlockedPressEvents = false;

        d->matcher.leaveEvent();
        break;
#endif
    case QEvent::FocusIn:
        d->debugEvent<QEvent, false>(event);
        KisAbstractInputAction::setInputManager(this);

        //Clear all state so we don't have half-matched shortcuts dangling around.
        d->matcher.reinitialize();

    { // Emulate pressing of the key that are already pressed
        KisExtendedModifiersMapper mapper;

        Qt::KeyboardModifiers modifiers = mapper.queryStandardModifiers();
        Q_FOREACH (Qt::Key key, mapper.queryExtendedModifiers()) {
            QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
            eventFilterImpl(&kevent);
        }
    }

        d->allowMouseEvents();
        break;

    case QEvent::FocusOut: {
        d->debugEvent<QEvent, false>(event);
        KisAbstractInputAction::setInputManager(this);

        QPointF currentLocalPos =
                canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());

        d->matcher.lostFocusEvent(currentLocalPos);

        break;
    }
    case QEvent::TabletPress: {
        d->debugEvent<QTabletEvent, false>(event);
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        if (d->tryHidePopupPalette()) {
            retval = true;
        } else {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.buttonPressed(tabletEvent->button(), tabletEvent);
            if (!d->containsPointer) {
                d->containsPointer = true;
                d->touchHasBlockedPressEvents = false;
            }
        }
        event->setAccepted(true);
        retval = true;
        d->blockMouseEvents();
        //Reset signal compressor to prevent processing events before press late
        d->resetCompressor();


#if defined Q_OS_LINUX && !defined QT_HAS_ENTER_LEAVE_PATCH
        // remove this hack when this patch is integrated:
        // https://codereview.qt-project.org/#/c/255384/
        event->setAccepted(false);
        d->eatOneMousePress();
#elif defined Q_OS_WIN32
        /**
         * Windows is the only platform that synthesizes mouse events for
         * the tablet on OS-level, that is, even when we accept the event
         */
        d->eatOneMousePress();
#endif

        break;
    }
    case QEvent::TabletMove: {
        d->debugEvent<QTabletEvent, false>(event);

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        retval = compressMoveEventCommon(tabletEvent);

        if (d->tabletLatencyTracker) {
            d->tabletLatencyTracker->push(tabletEvent->timestamp());
        }

        /**
         * The flow of tablet events means the tablet is in the
         * proximity area, so activate it even when the
         * TabletEnterProximity event was missed (may happen when
         * changing focus of the window with tablet in the proximity
         * area)
         */
        d->blockMouseEvents();

#if defined Q_OS_LINUX && !defined QT_HAS_ENTER_LEAVE_PATCH
        // remove this hack when this patch is integrated:
        // https://codereview.qt-project.org/#/c/255384/
        event->setAccepted(false);
#endif

        break;
    }
    case QEvent::TabletRelease: {
#if defined(Q_OS_MAC) || defined(Q_OS_ANDROID)
        d->allowMouseEvents();
#endif
        d->debugEvent<QTabletEvent, false>(event);

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        retval = d->matcher.buttonReleased(tabletEvent->button(), tabletEvent);
        retval = true;
        event->setAccepted(true);

#if defined Q_OS_LINUX && !defined QT_HAS_ENTER_LEAVE_PATCH
        // remove this hack when this patch is integrated:
        // https://codereview.qt-project.org/#/c/255384/
        event->setAccepted(false);
#endif

        break;
    }

    case QEvent::TouchBegin:
    {
        d->debugEvent<QTouchEvent, false>(event);
        if (startTouch(retval)) {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *> (event);
            KisAbstractInputAction::setInputManager(this);

            if (!KisConfig(true).disableTouchOnCanvas()
                && touchEvent->touchPoints().count() == 1)
            {
                d->previousPos = touchEvent->touchPoints().at(0).pos();
                d->buttonPressed = false;
                d->resetCompressor();
            }
            else {
                retval = d->matcher.touchBeginEvent(touchEvent);
            }
            event->accept();
        }

        // if the event isn't handled, Qt starts to send MouseEvents
        if (!KisConfig(true).disableTouchOnCanvas())
            retval = true;
        break;
    }

    case QEvent::TouchUpdate:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        d->debugEvent<QTouchEvent, false>(event);

#ifdef Q_OS_MAC
        int count = 0;
        Q_FOREACH (const QTouchEvent::TouchPoint &point, touchEvent->touchPoints()) {
            if (point.state() != Qt::TouchPointReleased) {
                count++;
            }
        }

        if (count < 2 && touchEvent->touchPoints().length() > count) {
            d->touchHasBlockedPressEvents = false;
            retval = d->matcher.touchEndEvent(touchEvent);
        } else {
#endif
            QPointF currentPos = touchEvent->touchPoints().at(0).pos();
            if (d->touchStrokeStarted || (!KisConfig(true).disableTouchOnCanvas()
                && !d->touchHasBlockedPressEvents
                && touchEvent->touchPoints().count() == 1
                && touchEvent->touchPointStates() != Qt::TouchPointStationary
                && (qAbs(currentPos.x() - d->previousPos.x()) > 1		// stop wobbiliness which Qt sends us
                ||  qAbs(currentPos.y() - d->previousPos.y()) > 1)))
            {
                d->previousPos = currentPos;
                if (!d->buttonPressed)
                {
                    // we start it here not in TouchBegin, because Qt::TouchPointStationary doesn't work with hpdi devices.
                    retval = d->matcher.buttonPressed(Qt::LeftButton, touchEvent);
                    d->buttonPressed = true;
                    break;
                }

                // if it is a full-fledged stroke, then ignore (currentPos.x - previousPos.x)
                d->touchStrokeStarted = true;
                retval = compressMoveEventCommon(touchEvent);
                d->blockMouseEvents();
            }
            else if (!d->touchStrokeStarted){
                KisAbstractInputAction::setInputManager(this);

                retval = d->matcher.touchUpdateEvent(touchEvent);
                d->touchHasBlockedPressEvents = retval;
            }
#ifdef Q_OS_MACOS
        }
#endif
        // if the event isn't handled, Qt starts to send MouseEvents
        if (!KisConfig(true).disableTouchOnCanvas())
            retval = true;

        event->accept();
        break;
    }

    case QEvent::TouchEnd:
    {
        d->debugEvent<QTouchEvent, false>(event);
        endTouch();
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        retval = d->matcher.touchEndEvent(touchEvent);
        if (d->touchStrokeStarted)
        {
            retval = d->matcher.buttonReleased(Qt::LeftButton, touchEvent);

            d->previousPos = {0, 0};
            d->touchStrokeStarted = false; // stroke ended
        }

        // if the event isn't handled, Qt starts to send MouseEvents
        if (!KisConfig(true).disableTouchOnCanvas())
            retval = true;

        event->accept();
        break;
    }
    case QEvent::TouchCancel:
    {
        d->debugEvent<QTouchEvent, false>(event);
        endTouch();
        d->matcher.touchCancelEvent(d->previousPos);
        // reset state
        d->previousPos = {0, 0};
        d->touchStrokeStarted = false;
        retval = true;
        event->accept();
        break;
    }

    case QEvent::NativeGesture:
    {
        QNativeGestureEvent *gevent = static_cast<QNativeGestureEvent*>(event);
        switch (gevent->gestureType()) {
            case Qt::BeginNativeGesture:
            {
                if (startTouch(retval)) {
                    KisAbstractInputAction::setInputManager(this);
                    retval = d->matcher.nativeGestureBeginEvent(gevent);
                    event->accept();
                }
                break;
            }
            case Qt::EndNativeGesture:
            {
                endTouch();
                retval = d->matcher.nativeGestureEndEvent(gevent);
                event->accept();
                break;
            }
            default:
            {
                KisAbstractInputAction::setInputManager(this);
                retval = d->matcher.nativeGestureEvent(gevent);
                event->accept();
                break;
            }
        }
        break;
    }

    default:
        break;
    }

    return !retval ? d->processUnhandledEvent(event) : true;
}

bool KisInputManager::startTouch(bool &retval)
{
    // Touch rejection: if touch is disabled on canvas, no need to block mouse press events
    if (KisConfig(true).disableTouchOnCanvas()) {
        d->eatOneMousePress();
    }
    if (d->tryHidePopupPalette()) {
        retval = true;
        return false;
    } else {
        return true;
    }
}

void KisInputManager::endTouch()
{
    d->touchHasBlockedPressEvents = false;
}

void KisInputManager::slotCompressedMoveEvent()
{
    if (d->compressedMoveEvent) {
        // d->touchHasBlockedPressEvents = false;

        (void) d->handleCompressedTabletEvent(d->compressedMoveEvent.data());
        d->compressedMoveEvent.reset();
        //dbgInput << "Compressed move event received.";
    } else {
        //dbgInput << "Unexpected empty move event";
    }
}

KisCanvas2* KisInputManager::canvas() const
{
    return d->canvas;
}

QPointer<KisToolProxy> KisInputManager::toolProxy() const
{
    return d->toolProxy;
}

void KisInputManager::slotAboutToChangeTool()
{
    QPointF currentLocalPos;
    if (canvas() && canvas()->canvasWidget()) {
        currentLocalPos = canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());
    }
    d->matcher.lostFocusEvent(currentLocalPos);
}

void KisInputManager::slotToolChanged()
{
    if (!d->canvas) return;
    KoToolManager *toolManager = KoToolManager::instance();
    KoToolBase *tool = toolManager->toolById(canvas(), toolManager->activeToolId());
    if (tool) {
        d->setMaskSyntheticEvents(tool->maskSyntheticEvents());
        if (tool->isInTextMode()) {
            d->forwardAllEventsToTool = true;
            d->matcher.suppressAllActions(true);
        } else {
            d->forwardAllEventsToTool = false;
            d->matcher.suppressAllActions(false);
        }
    }
}


void KisInputManager::profileChanged()
{
    d->matcher.clearShortcuts();

    KisInputProfile *profile = KisInputProfileManager::instance()->currentProfile();
    if (profile) {
        const QList<KisShortcutConfiguration*> shortcuts = profile->allShortcuts();

        for (KisShortcutConfiguration * const shortcut : shortcuts) {
            dbgUI << "Adding shortcut" << shortcut->keys() << "for action" << shortcut->action()->name();
            switch(shortcut->type()) {
            case KisShortcutConfiguration::KeyCombinationType:
                d->addKeyShortcut(shortcut->action(), shortcut->mode(), shortcut->keys());
                break;
            case KisShortcutConfiguration::MouseButtonType:
                d->addStrokeShortcut(shortcut->action(), shortcut->mode(), shortcut->keys(), shortcut->buttons());
                break;
            case KisShortcutConfiguration::MouseWheelType:
                d->addWheelShortcut(shortcut->action(), shortcut->mode(), shortcut->keys(), shortcut->wheel());
                break;
            case KisShortcutConfiguration::GestureType:
                if (!d->addNativeGestureShortcut(shortcut->action(), shortcut->mode(), shortcut->gesture())) {
                    d->addTouchShortcut(shortcut->action(), shortcut->mode(), shortcut->gesture());
                }
                break;
            default:
                break;
            }
        }
    }
    else {
        dbgInput << "No Input Profile Found: canvas interaction will be impossible";
    }
}
