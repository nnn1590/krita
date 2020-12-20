/*
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_select_path.h"

#include <KoPathShape.h>

#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_selection_options.h"
#include "kis_canvas_resource_provider.h"
#include "kis_canvas2.h"
#include "kis_pixel_selection.h"
#include "kis_selection_tool_helper.h"
#include <KisView.h>


KisToolSelectPath::KisToolSelectPath(KoCanvasBase * canvas)
    : KisToolSelectBase<KisDelegatedSelectPathWrapper>(canvas,
                                                       KisCursor::load("tool_polygonal_selection_cursor.png", 6, 6),
                                                       i18n("Select path"),
                                                       new __KisToolSelectPathLocalTool(canvas, this))
{
}

void KisToolSelectPath::requestStrokeEnd()
{
    localTool()->endPathWithoutLastPoint();
}

void KisToolSelectPath::requestStrokeCancellation()
{
    localTool()->cancelPath();
}

// Install an event filter to catch right-click events.
// This code is duplicated in kis_tool_path.cc
bool KisToolSelectPath::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton && hasUserInteractionRunning()) {
            localTool()->removeLastPoint();
            return true;
        }
    } else if (event->type() == QEvent::TabletPress) {
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        if (tabletEvent->button() == Qt::RightButton && hasUserInteractionRunning()) {
            localTool()->removeLastPoint();
            return true;
        }
    }
    return false;
}

QList<QPointer<QWidget> > KisToolSelectPath::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgetsList =
            DelegatedSelectPathTool::createOptionWidgets();
    QList<QPointer<QWidget> > filteredWidgets;
    Q_FOREACH (QWidget* widget, widgetsList) {
        if (widget->objectName() != "Stroke widget") {
            filteredWidgets.push_back(widget);
        }
    }
    return filteredWidgets;
}

void KisDelegatedSelectPathWrapper::beginPrimaryAction(KoPointerEvent *event) {
    DelegatedSelectPathTool::mousePressEvent(event);
}

void KisDelegatedSelectPathWrapper::continuePrimaryAction(KoPointerEvent *event){
    DelegatedSelectPathTool::mouseMoveEvent(event);
}

void KisDelegatedSelectPathWrapper::endPrimaryAction(KoPointerEvent *event) {
    DelegatedSelectPathTool::mouseReleaseEvent(event);
}

void KisDelegatedSelectPathWrapper::beginPrimaryDoubleClickAction(KoPointerEvent *event)
{
    DelegatedSelectPathTool::mouseDoubleClickEvent(event);
}

void KisDelegatedSelectPathWrapper::mousePressEvent(KoPointerEvent *event)
{
    // this event will be forwarded using beginPrimaryAction
    Q_UNUSED(event);
}

void KisDelegatedSelectPathWrapper::mouseMoveEvent(KoPointerEvent *event)
{
    DelegatedSelectPathTool::mouseMoveEvent(event);

    // WARNING: the code is duplicated from KisToolPaint::requestUpdateOutline
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());
    KisPaintingAssistantsDecorationSP decoration = kiscanvas->paintingAssistantsDecoration();
    if (decoration && decoration->visible() && decoration->hasPaintableAssistants()) {
        kiscanvas->updateCanvas();
    }
}

void KisDelegatedSelectPathWrapper::mouseReleaseEvent(KoPointerEvent *event)
{
    // this event will be forwarded using continuePrimaryAction
    Q_UNUSED(event);
}

void KisDelegatedSelectPathWrapper::mouseDoubleClickEvent(KoPointerEvent *event)
{
    // this event will be forwarded using endPrimaryAction
    Q_UNUSED(event);
}

bool KisDelegatedSelectPathWrapper::hasUserInteractionRunning() const
{
    return localTool()->pathStarted();
}


__KisToolSelectPathLocalTool::__KisToolSelectPathLocalTool(KoCanvasBase * canvas, KisToolSelectPath* parentTool)
    : KoCreatePathTool(canvas), m_selectionTool(parentTool)
{
    setEnableClosePathShortcut(false);
}

void __KisToolSelectPathLocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    QTransform matrix;
    matrix.scale(kisCanvas->image()->xRes(), kisCanvas->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_selectionTool->paintToolOutline(&painter, m_selectionTool->pixelToView(matrix.map(pathShape.outline())));
}

void __KisToolSelectPathLocalTool::addPathShape(KoPathShape* pathShape)
{
    pathShape->normalize();
    pathShape->close();

    KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (!kisCanvas)
        return;

    KisImageWSP image = kisCanvas->image();

    KisSelectionToolHelper helper(kisCanvas, kundo2_i18n("Select by Bezier Curve"));

    const SelectionMode mode =
        helper.tryOverrideSelectionMode(kisCanvas->viewManager()->selection(),
                                        m_selectionTool->selectionMode(),
                                        m_selectionTool->selectionAction());

    if (mode == PIXEL_SELECTION) {

        KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

        KisPainter painter(tmpSel);
        painter.setPaintColor(KoColor(Qt::black, tmpSel->colorSpace()));
        painter.setFillStyle(KisPainter::FillStyleForegroundColor);
        painter.setAntiAliasPolygonFill(m_selectionTool->antiAliasSelection());
        painter.setStrokeStyle(KisPainter::StrokeStyleNone);

        QTransform matrix;
        matrix.scale(image->xRes(), image->yRes());
        matrix.translate(pathShape->position().x(), pathShape->position().y());

        QPainterPath path = matrix.map(pathShape->outline());
        painter.fillPainterPath(path);
        tmpSel->setOutlineCache(path);

        helper.selectPixelSelection(tmpSel, m_selectionTool->selectionAction());

        delete pathShape;
    } else {
        helper.addSelectionShape(pathShape, m_selectionTool->selectionAction());
    }
}

void KisToolSelectPath::resetCursorStyle()
{
    if (selectionAction() == SELECTION_ADD) {
        useCursor(KisCursor::load("tool_polygonal_selection_cursor_add.png", 6, 6));
    } else if (selectionAction() == SELECTION_SUBTRACT) {
        useCursor(KisCursor::load("tool_polygonal_selection_cursor_sub.png", 6, 6));
    } else {
        KisToolSelectBase<KisDelegatedSelectPathWrapper>::resetCursorStyle();
    }
}


