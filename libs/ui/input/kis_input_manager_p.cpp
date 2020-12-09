/*
 *  Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_manager_p.h"

#include <QMap>
#include <QApplication>
#include <QScopedPointer>
#include <QtGlobal>

#include <boost/preprocessor/repeat_from_to.hpp>

#include "kis_input_manager.h"
#include "kis_config.h"
#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_stroke_shortcut.h"
#include "kis_touch_shortcut.h"
#include "kis_native_gesture_shortcut.h"
#include "kis_input_profile_manager.h"
#include "kis_extended_modifiers_mapper.h"

#include "kis_zoom_and_rotate_action.h"

/**
 * This hungry class EventEater encapsulates event masking logic.
 *
 * Its basic role is to kill synthetic mouseMove events sent by Xorg or Qt after
 * tablet events. Those events are sent in order to allow widgets that haven't
 * implemented tablet specific functionality to seamlessly behave as if one were
 * using a mouse. These synthetic events are *supposed* to be optional, or at
 * least come with a flag saying "This is a fake event!!" but neither of those
 * methods is trustworthy. (This is correct as of Qt 5.4 + Xorg.)
 *
 * Qt 5.4 provides no reliable way to see if a user's tablet is being hovered
 * over the pad, since it converts all tablethover events into mousemove, with
 * no option to turn this off. Moreover, sometimes the MouseButtonPress event
 * from the tapping their tablet happens BEFORE the TabletPress event. This
 * means we have to resort to a somewhat complicated logic. What makes this
 * truly a joke is that we are not guaranteed to observe TabletProximityEnter
 * events when we're using a tablet, either, you may only see an Enter event.
 *
 * Once we see tablet events heading our way, we can say pretty confidently that
 * every mouse event is fake. There are two painful cases to consider - a
 * mousePress event could arrive before the tabletPress event, or it could
 * arrive much later, e.g. after tabletRelease. The first was only seen on Linux
 * with Qt's XInput2 code, the solution was to hold onto mousePress events
 * temporarily and wait for tabletPress later, this is contained in git history
 * but is now removed. The second case is currently handled by the
 * eatOneMousePress function, which waits as long as necessary to detect and
 * block a single mouse press event.
 */

static bool isMouseEventType(QEvent::Type t)
{
    return (t == QEvent::MouseMove ||
            t == QEvent::MouseButtonPress ||
            t == QEvent::MouseButtonRelease ||
            t == QEvent::MouseButtonDblClick);
}

KisInputManager::Private::EventEater::EventEater()
{
    KisConfig cfg(true);
    activateSecondaryButtonsWorkaround = cfg.useRightMiddleTabletButtonWorkaround();
}

bool KisInputManager::Private::EventEater::eventFilter(QObject* target, QEvent* event )
{
    Q_UNUSED(target);

    auto debugEvent = [&](int i) {
        if (KisTabletDebugger::instance()->debugEnabled()) {
            QString pre = QString("[BLOCKED %1:]").arg(i);
            QMouseEvent *ev = static_cast<QMouseEvent*>(event);
            dbgTablet << KisTabletDebugger::instance()->eventToString(*ev, pre);
        }
    };

    auto debugTabletEvent = [&](int i) {
        if (KisTabletDebugger::instance()->debugEnabled()) {
            QString pre = QString("[BLOCKED %1:]").arg(i);
            QTabletEvent *ev = static_cast<QTabletEvent*>(event);
            dbgTablet << KisTabletDebugger::instance()->eventToString(*ev, pre);
        }
    };

    if (peckish && event->type() == QEvent::MouseButtonPress
        // Drop one mouse press following tabletPress or touchBegin
        && (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)) {
        peckish = false;
        debugEvent(1);
        return true;
    }

    if (activateSecondaryButtonsWorkaround) {
        if (event->type() == QEvent::TabletPress ||
                event->type() == QEvent::TabletRelease) {

            QTabletEvent *te = static_cast<QTabletEvent*>(event);
            if (te->button() != Qt::LeftButton) {
                debugTabletEvent(3);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonPress ||
                   event->type() == QEvent::MouseButtonRelease ||
                   event->type() == QEvent::MouseButtonDblClick) {

            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() != Qt::LeftButton) {
                return false;
            }
        }
    }

    if (isMouseEventType(event->type()) &&
               (hungry
            // On Mac, we need mouse events when the tablet is in proximity, but not pressed down
            // since tablet move events are not generated until after tablet press.
            #ifndef Q_OS_MAC
                || (eatSyntheticEvents && static_cast<QMouseEvent*>(event)->source() != Qt::MouseEventNotSynthesized)
            #endif
                )) {
        // Drop mouse events if enabled or event was synthetic & synthetic events are disabled
        debugEvent(2);
        return true;
    }
    return false; // All clear - let this one through!
}


void KisInputManager::Private::EventEater::activate()
{
    if (!hungry && (KisTabletDebugger::instance()->debugEnabled())) {
        dbgTablet << "Start blocking mouse events";
    }
    hungry = true;
}

void KisInputManager::Private::EventEater::deactivate()
{
    if (hungry && (KisTabletDebugger::instance()->debugEnabled())) {
        dbgTablet << "Stop blocking mouse events";
    }
    hungry = false;
}

void KisInputManager::Private::EventEater::eatOneMousePress()
{
    // Enable on other platforms if getting full-pressure splotches
    peckish = true;
}

bool KisInputManager::Private::ignoringQtCursorEvents()
{
    return eventEater.hungry;
}

void KisInputManager::Private::setMaskSyntheticEvents(bool value)
{
    eventEater.eatSyntheticEvents = value;
}

KisInputManager::Private::Private(KisInputManager *qq)
    : q(qq)
    , moveEventCompressor(10 /* ms */,
                          KisSignalCompressor::FIRST_ACTIVE,
                          KisSignalCompressor::ADDITIVE_INTERVAL)
    , priorityEventFilterSeqNo(0)
    , canvasSwitcher(this, qq)
{
    KisConfig cfg(true);


    moveEventCompressor.setDelay(cfg.tabletEventsDelay());
    testingAcceptCompressedTabletEvents = cfg.testingAcceptCompressedTabletEvents();
    testingCompressBrushEvents = cfg.testingCompressBrushEvents();

    if (cfg.trackTabletEventLatency()) {
        tabletLatencyTracker = new TabletLatencyTracker();
    }

    matcher.setInputActionGroupsMaskCallback(
        [this] () {
            return this->canvas ? this->canvas->inputActionGroupsMask() : AllActionGroup;
        });
}

static const int InputWidgetsThreshold = 2000;
static const int OtherWidgetsThreshold = 400;

KisInputManager::Private::CanvasSwitcher::CanvasSwitcher(Private *_d, QObject *p)
    : QObject(p),
      d(_d),
      eatOneMouseStroke(false),
      focusSwitchThreshold(InputWidgetsThreshold)
{
}

void KisInputManager::Private::CanvasSwitcher::setupFocusThreshold(QObject* object)
{
    QWidget *widget = qobject_cast<QWidget*>(object);
    KIS_SAFE_ASSERT_RECOVER_RETURN(widget);

    thresholdConnections.clear();
    thresholdConnections.addConnection(&focusSwitchThreshold, SIGNAL(timeout()), widget, SLOT(setFocus()));
}

void KisInputManager::Private::CanvasSwitcher::addCanvas(KisCanvas2 *canvas)
{
    if (!canvas) return;

    QObject *canvasWidget = canvas->canvasWidget();

    if (!canvasResolver.contains(canvasWidget)) {
        canvasResolver.insert(canvasWidget, canvas);
    } else {
        // just a sanity cheeck to find out if we are
        // trying to add two canvases concurrently.
        KIS_SAFE_ASSERT_RECOVER_NOOP(d->canvas == canvas);
    }

    if (canvas != d->canvas) {
        d->q->setupAsEventFilter(canvasWidget);
        canvasWidget->installEventFilter(this);

        setupFocusThreshold(canvasWidget);
        focusSwitchThreshold.setEnabled(false);

        d->canvas = canvas;
        d->toolProxy = qobject_cast<KisToolProxy*>(canvas->toolProxy());
    }
}

void KisInputManager::Private::CanvasSwitcher::removeCanvas(KisCanvas2 *canvas)
{
    QObject *widget = canvas->canvasWidget();

    canvasResolver.remove(widget);

    if (d->eventsReceiver == widget) {
        d->q->setupAsEventFilter(0);
    }

    widget->removeEventFilter(this);

    if (d->canvas == canvas) {
        d->canvas = 0;
        d->toolProxy = 0;
    }
}

bool isInputWidget(QWidget *w)
{
    if (!w) return false;


    QList<QLatin1String> types;
    types << QLatin1String("QAbstractSlider");
    types << QLatin1String("QAbstractSpinBox");
    types << QLatin1String("QLineEdit");
    types << QLatin1String("QTextEdit");
    types << QLatin1String("QPlainTextEdit");
    types << QLatin1String("QComboBox");
    types << QLatin1String("QKeySequenceEdit");

    Q_FOREACH (const QLatin1String &type, types) {
        if (w->inherits(type.data())) {
            return true;
        }
    }

    return false;
}

bool KisInputManager::Private::CanvasSwitcher::eventFilter(QObject* object, QEvent* event )
{
    if (canvasResolver.contains(object)) {
        switch (event->type()) {
        case QEvent::FocusIn: {
            QFocusEvent *fevent = static_cast<QFocusEvent*>(event);
            KisCanvas2 *canvas = canvasResolver.value(object);

            // only relevant canvases from the same main window should be
            // registered in the switcher
            KIS_SAFE_ASSERT_RECOVER_BREAK(canvas);

            if (canvas != d->canvas) {
                eatOneMouseStroke = 2 * (fevent->reason() == Qt::MouseFocusReason);
            }

            d->canvas = canvas;
            d->toolProxy = qobject_cast<KisToolProxy*>(canvas->toolProxy());

            d->q->setupAsEventFilter(object);

            object->removeEventFilter(this);
            object->installEventFilter(this);

            setupFocusThreshold(object);
            focusSwitchThreshold.setEnabled(false);

            const QPoint globalPos = QCursor::pos();
            const QPoint localPos = d->canvas->canvasWidget()->mapFromGlobal(globalPos);
            QWidget *canvasWindow = d->canvas->canvasWidget()->window();
            const QPoint windowsPos = canvasWindow ? canvasWindow->mapFromGlobal(globalPos) : localPos;

            QEnterEvent event(localPos, windowsPos, globalPos);
            d->q->eventFilter(object, &event);
            break;
        }
        case QEvent::FocusOut: {
            focusSwitchThreshold.setEnabled(true);
            break;
        }
        case QEvent::Enter: {
            break;
        }
        case QEvent::Leave: {
            focusSwitchThreshold.stop();
            break;
        }
        case QEvent::Wheel: {
            QWidget *widget = static_cast<QWidget*>(object);
            widget->setFocus();
            break;
        }
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
            focusSwitchThreshold.forceDone();

            if (eatOneMouseStroke) {
                eatOneMouseStroke--;
                return true;
            }
            break;
        case QEvent::MouseButtonDblClick:
            focusSwitchThreshold.forceDone();
            if (eatOneMouseStroke) {
                return true;
            }
            break;
        case QEvent::MouseMove:
        case QEvent::TabletMove: {
            QWidget *widget = static_cast<QWidget*>(object);

            if (!widget->hasFocus()) {
                const int delay =
                    isInputWidget(QApplication::focusWidget()) ?
                    InputWidgetsThreshold : OtherWidgetsThreshold;

                focusSwitchThreshold.setDelayThreshold(delay);
                focusSwitchThreshold.start();
            }
        }
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(object, event);
}

KisInputManager::Private::ProximityNotifier::ProximityNotifier(KisInputManager::Private *_d, QObject *p)
    : QObject(p), d(_d)
{}

bool KisInputManager::Private::ProximityNotifier::eventFilter(QObject* object, QEvent* event )
{
    /**
     * All Qt builds in range 5.7.0...5.11.X on X11 had a problem that made all
     * the tablet events be accepted by default. It meant that no mouse
     * events were synthesized, and, therefore, no Enter/Leave were generated.
     *
     * The fix for this bug has been added only in Qt 5.12.0:
     * https://codereview.qt-project.org/#/c/239918/
     *
     * To avoid this problem we should explicitly ignore all the tablet events.
     */
#if defined Q_OS_LINUX && \
    QT_VERSION >= QT_VERSION_CHECK(5, 9, 0) && \
    QT_VERSION < QT_VERSION_CHECK(5, 12, 0)

    if (event->type() == QEvent::TabletMove ||
        event->type() == QEvent::TabletPress ||
        event->type() == QEvent::TabletRelease) {

        event->ignore();
    }
#endif

    switch (event->type()) {
    case QEvent::TabletEnterProximity:
        d->debugEvent<QEvent, false>(event);
        // Tablet proximity events are unreliable AND fake mouse events do not
        // necessarily come after tablet events, so this is insufficient.
        // d->eventEater.eatOneMousePress();

        // Qt sends fake mouse events instead of hover events, so not very useful.
        // Don't block mouse events on tablet since tablet move events are not generated until
        // after tablet press.
#ifndef Q_OS_MACOS
        d->blockMouseEvents();
#endif
        break;
    case QEvent::TabletLeaveProximity:
        d->debugEvent<QEvent, false>(event);
        d->allowMouseEvents();
        break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

#define EXTRA_BUTTON(z, n, _) \
    if(buttons & Qt::ExtraButton##n) { \
        buttonSet << Qt::ExtraButton##n; \
    }

void KisInputManager::Private::addStrokeShortcut(KisAbstractInputAction* action, int index,
                                                 const QList<Qt::Key> &modifiers,
                                                 Qt::MouseButtons buttons)
{
    KisStrokeShortcut *strokeShortcut =
        new KisStrokeShortcut(action, index);

    QSet<Qt::MouseButton> buttonSet;
    if(buttons & Qt::LeftButton) {
        buttonSet << Qt::LeftButton;
    }
    if(buttons & Qt::RightButton) {
        buttonSet << Qt::RightButton;
    }
    if(buttons & Qt::MidButton) {
        buttonSet << Qt::MidButton;
    }

BOOST_PP_REPEAT_FROM_TO(1, 25, EXTRA_BUTTON, _)

    if (!buttonSet.empty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        strokeShortcut->setButtons(QSet<Qt::Key>(modifiers.cbegin(), modifiers.cend()), buttonSet);
#else
        strokeShortcut->setButtons(QSet<Qt::Key>::fromList(modifiers), buttonSet);
#endif
        matcher.addShortcut(strokeShortcut);
    }
    else {
        delete strokeShortcut;
    }
}

void KisInputManager::Private::addKeyShortcut(KisAbstractInputAction* action, int index,
                                              const QList<Qt::Key> &keys)
{
    if (keys.size() == 0) return;

    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);

    //Note: Ordering is important here, Shift + V is different from V + Shift,
    //which is the reason we use the last key here since most users will enter
    //shortcuts as "Shift + V". Ideally this should not happen, but this is
    //the way the shortcut matcher is currently implemented.
    QList<Qt::Key> allKeys = keys;
    Qt::Key key = allKeys.takeLast();
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    QSet<Qt::Key> modifiers = QSet<Qt::Key>(allKeys.begin(), allKeys.end());
#else
    QSet<Qt::Key> modifiers = QSet<Qt::Key>::fromList(allKeys);
#endif
    keyShortcut->setKey(modifiers, key);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::addWheelShortcut(KisAbstractInputAction* action, int index,
                                                const QList<Qt::Key> &modifiers,
                                                KisShortcutConfiguration::MouseWheelMovement wheelAction)
{
    QScopedPointer<KisSingleActionShortcut> keyShortcut(
        new KisSingleActionShortcut(action, index));

    KisSingleActionShortcut::WheelAction a;
    switch(wheelAction) {
    case KisShortcutConfiguration::WheelUp:
        a = KisSingleActionShortcut::WheelUp;
        break;
    case KisShortcutConfiguration::WheelDown:
        a = KisSingleActionShortcut::WheelDown;
        break;
    case KisShortcutConfiguration::WheelLeft:
        a = KisSingleActionShortcut::WheelLeft;
        break;
    case KisShortcutConfiguration::WheelRight:
        a = KisSingleActionShortcut::WheelRight;
        break;
    case KisShortcutConfiguration::WheelTrackpad:
        a = KisSingleActionShortcut::WheelTrackpad;
        break;
    default:
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    keyShortcut->setWheel(QSet<Qt::Key>(modifiers.begin(), modifiers.end()), a);
#else
    keyShortcut->setWheel(QSet<Qt::Key>::fromList(modifiers), a);
#endif
    matcher.addShortcut(keyShortcut.take());
}

void KisInputManager::Private::addTouchShortcut(KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture)
{
    KisTouchShortcut *shortcut = new KisTouchShortcut(action, index, gesture);
    dbgKrita << "TouchAction:" << action->name();
    switch(gesture) {
    case KisShortcutConfiguration::RotateGesture:
    case KisShortcutConfiguration::PinchGesture:
#ifndef Q_OS_MACOS
    case KisShortcutConfiguration::ZoomAndRotateGesture:
#endif
        shortcut->setMinimumTouchPoints(2);
        shortcut->setMaximumTouchPoints(2);
        break;
    case KisShortcutConfiguration::PanGesture:
        shortcut->setMinimumTouchPoints(3);
        shortcut->setMaximumTouchPoints(10);
        break;
    default:
        break;
    }
    matcher.addShortcut(shortcut);
}

bool KisInputManager::Private::addNativeGestureShortcut(KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture)
{
    // Qt5 only implements QNativeGestureEvent for macOS
    Qt::NativeGestureType type;
    switch (gesture) {
#ifdef Q_OS_MACOS
        case KisShortcutConfiguration::PinchGesture:
            type = Qt::ZoomNativeGesture;
            break;
        case KisShortcutConfiguration::PanGesture:
            type = Qt::PanNativeGesture;
            break;
        case KisShortcutConfiguration::RotateGesture:
            type = Qt::RotateNativeGesture;
            break;
        case KisShortcutConfiguration::SmartZoomGesture:
            type = Qt::SmartZoomNativeGesture;
            break;
#endif
        default:
            return false;
    }

    KisNativeGestureShortcut *shortcut = new KisNativeGestureShortcut(action, index, type);
    matcher.addShortcut(shortcut);
    return true;
}

void KisInputManager::Private::setupActions()
{
    QList<KisAbstractInputAction*> actions = KisInputProfileManager::instance()->actions();
    Q_FOREACH (KisAbstractInputAction *action, actions) {
        KisToolInvocationAction *toolAction =
            dynamic_cast<KisToolInvocationAction*>(action);

        if(toolAction) {
            defaultInputAction = toolAction;
        }
    }

    connect(KisInputProfileManager::instance(), SIGNAL(currentProfileChanged()), q, SLOT(profileChanged()));
    if(KisInputProfileManager::instance()->currentProfile()) {
        q->profileChanged();
    }
}

bool KisInputManager::Private::processUnhandledEvent(QEvent *event)
{
    bool retval = false;

    if (forwardAllEventsToTool ||
        event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease) {

        defaultInputAction->processUnhandledEvent(event);
        retval = true;
    }

    return retval && !forwardAllEventsToTool;
}

bool KisInputManager::Private::tryHidePopupPalette()
{
    if (canvas->isPopupPaletteVisible()) {
        canvas->slotShowPopupPalette();
        return true;
    }
    return false;
}

#ifdef HAVE_X11
inline QPointF dividePoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() / pt2.x(), pt1.y() / pt2.y());
}

inline QPointF multiplyPoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() * pt2.x(), pt1.y() * pt2.y());
}
#endif

void KisInputManager::Private::blockMouseEvents()
{
    eventEater.activate();
}

void KisInputManager::Private::allowMouseEvents()
{
    /**
     * On Windows tablet events may arrive asynchronously to the
     * mouse events (in WinTab mode). The problem is that Qt
     * generates Enter/Leave and FocusIn/Out events via mouse
     * events only. It means that TabletPress may come much before
     * Enter and FocusIn event and start the stroke. In such a case
     * we shouldn't unblock mouse events.
     *
     * See https://bugs.kde.org/show_bug.cgi?id=417040
     *
     * PS:
     * Ideally, we should fix Qt to generate Enter/Leave and
     * FocusIn/Out events based on tablet events as well, but
     * it is a lot of work.
     */
#ifdef Q_OS_WIN32
    if (eventEater.hungry && matcher.hasRunningShortcut()) {
        return;
    }
#endif

    eventEater.deactivate();
}

void KisInputManager::Private::eatOneMousePress()
{
    eventEater.eatOneMousePress();
}

void KisInputManager::Private::resetCompressor() {
    compressedMoveEvent.reset();
    moveEventCompressor.stop();
}

bool KisInputManager::Private::handleCompressedTabletEvent(QEvent *event)
{
    bool retval = false;

    /**
     * When Krita (as an application) has no input focus, we cannot
     * handle key events. But at the same time, when the user hovers
     * Krita canvas, we should still show him the correct cursor.
     *
     * So here we just add a simple workaround to resync shortcut
     * matcher's state at least against the basic modifiers, like
     * Shift, Control and Alt.
     */
    QWidget *recievingWidget = dynamic_cast<QWidget*>(eventsReceiver);
    if (recievingWidget && !recievingWidget->hasFocus()) {
        fixShortcutMatcherModifiersState();
    }

    if (!matcher.pointerMoved(event) && toolProxy) {
        toolProxy->forwardHoverEvent(event);
    }
    retval = true;
    event->setAccepted(true);

    return retval;
}

void KisInputManager::Private::fixShortcutMatcherModifiersState()
{
    QVector<Qt::Key> guessedKeys;
    KisExtendedModifiersMapper mapper;
    Qt::KeyboardModifiers modifiers = mapper.queryStandardModifiers();
    Q_FOREACH (Qt::Key key, mapper.queryExtendedModifiers()) {
        QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
        guessedKeys << KisExtendedModifiersMapper::workaroundShiftAltMetaHell(&kevent);
    }

    matcher.recoveryModifiersWithoutFocus(guessedKeys);
}

qint64 KisInputManager::Private::TabletLatencyTracker::currentTimestamp() const
{
    // on OS X, we need to compute the timestamp that compares correctly against the native event timestamp,
    // which seems to be the msecs since system startup. On Linux with WinTab, we produce the timestamp that
    // we compare against ourselves in QWindowSystemInterface.

    QElapsedTimer elapsed;
    elapsed.start();
    return elapsed.msecsSinceReference();
}

void KisInputManager::Private::TabletLatencyTracker::print(const QString &message)
{
    dbgTablet << qUtf8Printable(message);
}
