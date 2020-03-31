/* This file is part of the KDE project
 * Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QDebug>

#include <QAction>

#include "KoToolBase.h"
#include "KoToolBase_p.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoDocumentResourceManager.h"
#include "KoCanvasResourceProvider.h"
#include "KoViewConverter.h"
#include "KoShapeController.h"
#include "KoShapeControllerBase.h"
#include "KoToolSelection.h"
#include "KoCanvasController.h"

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <QWidget>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>

KoToolBase::KoToolBase(KoCanvasBase *canvas)
    : d_ptr(new KoToolBasePrivate(this, canvas))
{
    Q_D(KoToolBase);
    d->connectSignals();
}

KoToolBase::KoToolBase(KoToolBasePrivate &dd)
    : d_ptr(&dd)
{
    Q_D(KoToolBase);
    d->connectSignals();
}

KoToolBase::~KoToolBase()
{
    qDeleteAll(d_ptr->optionWidgets);
    delete d_ptr;
}


bool KoToolBase::isActivated() const
{
    Q_D(const KoToolBase);
    return d->isActivated;
}


void KoToolBase::activate(KoToolBase::ToolActivation toolActivation, const QSet<KoShape *> &shapes)
{
    Q_UNUSED(toolActivation);
    Q_UNUSED(shapes);

    Q_D(KoToolBase);
    d->isActivated = true;
}

void KoToolBase::deactivate()
{
    Q_D(KoToolBase);
    d->isActivated = false;
}

void KoToolBase::canvasResourceChanged(int key, const QVariant & res)
{
    Q_UNUSED(key);
    Q_UNUSED(res);
}

void KoToolBase::documentResourceChanged(int key, const QVariant &res)
{
    Q_UNUSED(key);
    Q_UNUSED(res);
}

bool KoToolBase::wantsAutoScroll() const
{
    return true;
}

void KoToolBase::mouseDoubleClickEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KoToolBase::mouseTripleClickEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KoToolBase::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void KoToolBase::keyReleaseEvent(QKeyEvent *e)
{
    e->ignore();
}


void KoToolBase::explicitUserStrokeEndRequest()
{
}

QVariant KoToolBase::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &) const
{
    Q_D(const KoToolBase);
    if (d->canvas->canvasWidget() == 0)
        return QVariant();

    switch (query) {
    case Qt::ImMicroFocus:
        return QRect(d->canvas->canvasWidget()->width() / 2, 0, 1, d->canvas->canvasWidget()->height());
    case Qt::ImFont:
        return d->canvas->canvasWidget()->font();
    default:
        return QVariant();
    }
}

void KoToolBase::inputMethodEvent(QInputMethodEvent * event)
{
    if (! event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
    }
    event->accept();
}

void KoToolBase::customPressEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoToolBase::customReleaseEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoToolBase::customMoveEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoToolBase::useCursor(const QCursor &cursor)
{
    Q_D(KoToolBase);
    d->currentCursor = cursor;
    emit cursorChanged(d->currentCursor);
}

QList<QPointer<QWidget> > KoToolBase::optionWidgets()
{
    Q_D(KoToolBase);
    if (!d->optionWidgetsCreated) {
        d->optionWidgets = createOptionWidgets();
        d->optionWidgetsCreated = true;
    }
    return d->optionWidgets;
}

QAction *KoToolBase::action(const QString &name) const
{
    Q_D(const KoToolBase);
    if (d->canvas && d->canvas->canvasController() && d->canvas->canvasController()) {
        return d->canvas->canvasController()->actionCollection()->action(name);
    }
    return 0;
}

QWidget * KoToolBase::createOptionWidget()
{
    return 0;
}

QList<QPointer<QWidget> >  KoToolBase::createOptionWidgets()
{
    QList<QPointer<QWidget> > ow;
    if (QWidget *widget = createOptionWidget()) {
        if (widget->objectName().isEmpty()) {
            widget->setObjectName(toolId());
        }
        ow.append(widget);
    }
    return ow;
}

void KoToolBase::setToolId(const QString &id)
{
    Q_D(KoToolBase);
    d->toolId = id;
}

QString KoToolBase::toolId() const
{
    Q_D(const KoToolBase);
    return d->toolId;
}

QCursor KoToolBase::cursor() const
{
    Q_D(const KoToolBase);
    return d->currentCursor;
}

void KoToolBase::deleteSelection()
{
}

void KoToolBase::cut()
{
    copy();
    deleteSelection();
}

QMenu *KoToolBase::popupActionsMenu()
{
    return 0;
}

KoCanvasBase * KoToolBase::canvas() const
{
    Q_D(const KoToolBase);
    return d->canvas;
}

void KoToolBase::setStatusText(const QString &statusText)
{
    emit statusTextChanged(statusText);
}

uint KoToolBase::handleRadius() const
{
    Q_D(const KoToolBase);
    if (d->canvas
            && d->canvas->shapeController()
            && d->canvas->shapeController()->resourceManager()
       )
    {
        return d->canvas->shapeController()->resourceManager()->handleRadius();
    }
    else {
        return 3;
    }
}

uint KoToolBase::grabSensitivity() const
{
    Q_D(const KoToolBase);
    if(d->canvas->shapeController()->resourceManager())
    {
        return d->canvas->shapeController()->resourceManager()->grabSensitivity();
    } else {
        return 3;
    }
}

QRectF KoToolBase::handleGrabRect(const QPointF &position) const
{
    Q_D(const KoToolBase);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*grabSensitivity();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

QRectF KoToolBase::handlePaintRect(const QPointF &position) const
{
    Q_D(const KoToolBase);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*handleRadius();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

void KoToolBase::setTextMode(bool value)
{
    Q_D(KoToolBase);
    d->isInTextMode=value;
}

bool KoToolBase::paste()
{
    return false;
}

void KoToolBase::copy() const
{
}

void KoToolBase::dragMoveEvent(QDragMoveEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

void KoToolBase::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
}

void KoToolBase::dropEvent(QDropEvent *event, const QPointF &point)
{
    Q_UNUSED(event);
    Q_UNUSED(point);
}

bool KoToolBase::hasSelection()
{
    KoToolSelection *sel = selection();
    return (sel && sel->hasSelection());
}

KoToolSelection *KoToolBase::selection()
{
    return 0;
}

void KoToolBase::repaintDecorations()
{
}

bool KoToolBase::isInTextMode() const
{
    Q_D(const KoToolBase);
    return d->isInTextMode;
}

void KoToolBase::requestUndoDuringStroke()
{
    /**
     * Default implementation just cancels the stroke
     */
    requestStrokeCancellation();
}

void KoToolBase::requestStrokeCancellation()
{
}

void KoToolBase::requestStrokeEnd()
{
}

bool KoToolBase::maskSyntheticEvents() const
{
    Q_D(const KoToolBase);
    return d->maskSyntheticEvents;
}

void KoToolBase::setMaskSyntheticEvents(bool value)
{
    Q_D(KoToolBase);
    d->maskSyntheticEvents = value;
}
