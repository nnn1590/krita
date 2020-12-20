/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_figure_painting_tool_helper.h"

#include <KoCanvasResourceProvider.h>

#include "kis_resources_snapshot.h"
#include <kis_distance_information.h>
#include "kis_image.h"
#include "kis_painter.h"
#include <strokes/KisFreehandStrokeInfo.h>
#include "KisAsyncronousStrokeUpdateHelper.h"


KisFigurePaintingToolHelper::KisFigurePaintingToolHelper(const KUndo2MagicString &name,
                                                         KisImageWSP image,
                                                         KisNodeSP currentNode,
                                                         KoCanvasResourceProvider *resourceManager,
                                                         KisToolShapeUtils::StrokeStyle strokeStyle,
                                                         KisToolShapeUtils::FillStyle fillStyle,
                                                         QTransform fillTransform)
{
    m_strokesFacade = image.data();

    m_resources =
        new KisResourcesSnapshot(image,
                                 currentNode,
                                 resourceManager);

    setupPaintStyles(m_resources, strokeStyle, fillStyle, fillTransform);

    KisFreehandStrokeInfo *strokeInfo = new KisFreehandStrokeInfo();

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(m_resources, strokeInfo, name);

    m_strokeId = m_strokesFacade->startStroke(stroke);
}

void KisFigurePaintingToolHelper::setupPaintStyles(KisResourcesSnapshotSP resources,
                                                   KisToolShapeUtils::StrokeStyle strokeStyle,
                                                   KisToolShapeUtils::FillStyle fillStyle,
                                                   QTransform fillTransform)
{
    using namespace KisToolShapeUtils;

    const KoColor fgColor = resources->currentFgColor();
    const KoColor bgColor = resources->currentBgColor();

    switch (strokeStyle) {
    case StrokeStyleNone:
        resources->setStrokeStyle(KisPainter::StrokeStyleNone);
        break;
    case StrokeStyleForeground:
        resources->setStrokeStyle(KisPainter::StrokeStyleBrush);
        break;
    case StrokeStyleBackground:
        resources->setStrokeStyle(KisPainter::StrokeStyleBrush);

        resources->setFGColorOverride(bgColor);
        resources->setBGColorOverride(fgColor);

        if (fillStyle == FillStyleForegroundColor) {
            fillStyle = FillStyleBackgroundColor;
        } else if (fillStyle == FillStyleBackgroundColor) {
            fillStyle = FillStyleForegroundColor;
        }

        break;
    };

    switch (fillStyle) {
    case FillStyleForegroundColor:
        resources->setFillStyle(KisPainter::FillStyleForegroundColor);
        break;
    case FillStyleBackgroundColor:
        resources->setFillStyle(KisPainter::FillStyleBackgroundColor);
        break;
    case FillStylePattern:
        resources->setFillStyle(KisPainter::FillStylePattern);
        break;
    case FillStyleNone:
        resources->setFillStyle(KisPainter::FillStyleNone);
        break;
    }

    resources->setFillTransform(fillTransform);
}

KisFigurePaintingToolHelper::~KisFigurePaintingToolHelper()
{
    m_strokesFacade->addJob(m_strokeId,
        new KisAsyncronousStrokeUpdateHelper::UpdateData(true));
    m_strokesFacade->endStroke(m_strokeId);
}

void KisFigurePaintingToolHelper::paintLine(const KisPaintInformation &pi0,
                                            const KisPaintInformation &pi1)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         pi0, pi1));
}

void KisFigurePaintingToolHelper::paintPolyline(const vQPointF &points)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::POLYLINE,
                                         points));
}

void KisFigurePaintingToolHelper::paintPolygon(const vQPointF &points)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::POLYGON,
                                         points));
}

void KisFigurePaintingToolHelper::paintRect(const QRectF &rect)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::RECT,
                                         rect));
}

void KisFigurePaintingToolHelper::paintEllipse(const QRectF &rect)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::ELLIPSE,
                                         rect));
}

void KisFigurePaintingToolHelper::paintPainterPath(const QPainterPath &path)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::PAINTER_PATH,
                                         path));
}

void KisFigurePaintingToolHelper::setFGColorOverride(const KoColor &color)
{
    m_resources->setFGColorOverride(color);
}

void KisFigurePaintingToolHelper::setBGColorOverride(const KoColor &color)
{
    m_resources->setBGColorOverride(color);
}

void KisFigurePaintingToolHelper::setSelectionOverride(KisSelectionSP m_selection)
{
    m_resources->setSelectionOverride(m_selection);
}

void KisFigurePaintingToolHelper::setBrush(const KisPaintOpPresetSP &brush)
{
    m_resources->setBrush(brush);
}

void KisFigurePaintingToolHelper::paintPainterPathQPen(const QPainterPath path, const QPen &pen, const KoColor &color)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::QPAINTER_PATH,
                                         path, pen, color));
}

void KisFigurePaintingToolHelper::paintPainterPathQPenFill(const QPainterPath path, const QPen &pen, const KoColor &color)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::QPAINTER_PATH_FILL,
                                         path, pen, color));
}
