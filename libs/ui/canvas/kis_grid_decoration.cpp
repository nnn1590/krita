/*
 * This file is part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_grid_decoration.h"

#include <QPainter>
#include <QPen>
#include <QtMath>
#include <klocalizedstring.h>

#include <KoUnit.h>

#include "kis_grid_config.h"
#include "kis_coordinates_converter.h"

struct KisGridDecoration::Private
{
    KisGridConfig config;
};

KisGridDecoration::KisGridDecoration(KisView* parent)
    : KisCanvasDecoration("grid", parent),
      m_d(new Private)
{
    setPriority(0);
}

KisGridDecoration::~KisGridDecoration()
{

}

void KisGridDecoration::setGridConfig(const KisGridConfig &config)
{
    m_d->config = config;
}

void KisGridDecoration::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    if (!m_d->config.showGrid()) return;

    Q_UNUSED(canvas);

    QTransform transform = converter->imageToWidgetTransform();

    const qreal scale = KoUnit::approxTransformScale(transform);
    const int minWidgetSize = 3;
    const int effectiveSize = qMin(m_d->config.spacing().x(), m_d->config.spacing().y());

    int scaleCoeff = 1;
    quint32 subdivision = m_d->config.subdivision();

    while (qFloor(scale * scaleCoeff * effectiveSize) <= minWidgetSize) {
        if (subdivision > 1) {
            scaleCoeff = subdivision;
            subdivision = 1;
        } else {
            scaleCoeff *= 2;
        }

        if (scaleCoeff > 32768) {
            qWarning() << "WARNING: Grid Scale Coeff is too high! That is surely a bug!";
            return;
        }
    }

    const QPen mainPen = m_d->config.penMain();
    const QPen subdivisionPen = m_d->config.penSubdivision();

    gc.save();
    gc.setTransform(transform);
    gc.setRenderHints(QPainter::Antialiasing, false);
    gc.setRenderHints(QPainter::HighQualityAntialiasing, false);

    qreal x1, y1, x2, y2;
    QRectF imageRect =
        converter->documentToImage(updateArea) &
        converter->imageRectInImagePixels();
    imageRect.getCoords(&x1, &y1, &x2, &y2);


    // for angles. This will later be a combobox to select different types of options
    // also add options to hide specific lines (vertical, horizonta, angle 1, etc
    KisGridConfig::GridType gridType = m_d->config.gridType();

    if (gridType == KisGridConfig::GRID_RECTANGULAR) {

        {
            // vertical lines
            const int offset = m_d->config.offset().x();
            const int step = scaleCoeff * m_d->config.spacing().x();
            const int lineIndexFirst = qCeil((x1 - offset) / step);
            const int lineIndexLast = qFloor((x2 - offset) / step);

            for (int i = lineIndexFirst; i <= lineIndexLast; i++) {
                int w = offset + i * step;

                gc.setPen(i % subdivision == 0 ? mainPen : subdivisionPen);
                gc.drawLine(QPointF(w, y1),QPointF(w, y2));
            }
        }

        {
            // horizontal lines
            const int offset = m_d->config.offset().y();
            const int step = scaleCoeff * m_d->config.spacing().y();
            const int lineIndexFirst = qCeil((y1 - offset) / step);
            const int lineIndexLast = qFloor((y2 - offset) / step);

            for (int i = lineIndexFirst; i <= lineIndexLast; i++) {
                int w = offset + i * step;

                gc.setPen(i % subdivision == 0 ? mainPen : subdivisionPen);
                gc.drawLine(QPointF(x1, w),QPointF(x2, w));
            }
        }
    }

    if (gridType == KisGridConfig::GRID_ISOMETRIC)  {

        // get true coordinates, not just the updateArea
        QRectF trueImageRect = converter->imageRectInImagePixels();
        trueImageRect.getCoords(&x1, &y1, &x2, &y2);

        const int offset = m_d->config.offset().x();
        const int offsetY = m_d->config.offset().y();
        const int step = scaleCoeff * m_d->config.spacing().y();
        const int lineIndexFirst = qCeil((y1 - offset) / step);
        const int cellSpacing = m_d->config.cellSpacing();

        gc.setClipping(true);
        gc.setClipRect(imageRect, Qt::IntersectClip);





        // left angle
        {
            int gridXAngle = m_d->config.angleLeft();
            int bottomRightOfImageY = y2; // this should be the height of the image
            int finalY = 0;


            // figure out the spacing based off the angle. The spacing needs to be perpendicular to the angle,
            // so we need to do a bit of trig to get the correct spacing.
            float correctedAngleSpacing = cellSpacing;
            if (gridXAngle > 0) {
                correctedAngleSpacing = cellSpacing / qCos( qDegreesToRadians((float)gridXAngle));

            }

            int counter = qFloor((-(offset + offsetY))/(qreal)correctedAngleSpacing);

            while (finalY < bottomRightOfImageY) {

                int w = (counter * correctedAngleSpacing) + offsetY + offset;
                gc.setPen(mainPen);

                // calculate where the ending point will be based off the angle
                int startingY = w;
                int horizontalDistance = x2;

                int length2 = qTan( qDegreesToRadians((float)gridXAngle)) * x2; // qTan takes radians, so convert first before sending it

                finalY = startingY - length2;
                gc.drawLine(QPointF(x1, w),QPointF(horizontalDistance, finalY));

                counter = counter +1;
            }
        }


        // right angle (almost the same thing, except starting the lines on the right side)
        {
            int gridXAngle = m_d->config.angleRight(); // TODO: add another angle property
            int bottomLeftOfImageY = y2;


            // figure out the spacing based off the angle
            float correctedAngleSpacing = cellSpacing;
            if (gridXAngle > 0) {
                correctedAngleSpacing = cellSpacing / qCos( qDegreesToRadians((float)gridXAngle));
            }

            int horizontalDistance = x2; // distance is the same (width of the image)
            int length2 = qTan( qDegreesToRadians((float)gridXAngle)) * horizontalDistance; // qTan takes radians, so convert first before sending it

            // let's get x, y of the line that starts in the top right corder
            int yLower = 0;
            int yHigher = yLower - length2;

            int yLeftFirst = qCeil(qreal(yHigher)/correctedAngleSpacing)*correctedAngleSpacing;
            int additionalOffset = yLeftFirst - yHigher;
            int finalY = 0;
            int counter = qFloor((-(offsetY - offset))/(qreal)correctedAngleSpacing);

            while (finalY < bottomLeftOfImageY) {

                int w = (counter * correctedAngleSpacing) + offsetY - offset + additionalOffset;
                gc.setPen(mainPen);

                // calculate where the ending point will be based off the angle
                int startingY = w;

                finalY = startingY - length2;
                gc.drawLine(QPointF(x2, w),QPointF(0, finalY));

                counter = counter +1;
            }
        }


    }

    gc.restore();
}
