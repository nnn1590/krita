/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "kis_perspectivetransform_worker.h"

#include <QMatrix4x4>
#include <QTransform>
#include <QVector3D>
#include <QPolygonF>

#include <KoUpdater.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>

#include "kis_paint_device.h"
#include "kis_perspective_math.h"
#include "kis_random_accessor_ng.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"
#include <kis_iterator_ng.h>
#include "krita_utils.h"
#include "kis_progress_update_helper.h"
#include "kis_painter.h"
#include "kis_image.h"


KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QPointF center, double aX, double aY, double distance, KoUpdaterPtr progress)
        : m_dev(dev), m_progressUpdater(progress)

{
    QMatrix4x4 m;
    m.rotate(180. * aX / M_PI, QVector3D(1, 0, 0));
    m.rotate(180. * aY / M_PI, QVector3D(0, 1, 0));

    QTransform project = m.toTransform(distance);
    QTransform t = QTransform::fromTranslate(center.x(), center.y());

    QTransform forwardTransform = t.inverted() * project * t;

    init(forwardTransform);
}

KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, const QTransform &transform, KoUpdaterPtr progress)
    : m_dev(dev), m_progressUpdater(progress)
{
    init(transform);
}

void KisPerspectiveTransformWorker::fillParams(const QRectF &srcRect,
                                               const QRect &dstBaseClipRect,
                                               KisRegion *dstRegion,
                                               QPolygonF *dstClipPolygon)
{
    QPolygonF bounds = srcRect;
    QPolygonF newBounds = m_forwardTransform.map(bounds);

    newBounds = newBounds.intersected(QRectF(dstBaseClipRect));

    QPainterPath path;
    path.addPolygon(newBounds);
    *dstRegion = KritaUtils::splitPath(path);
    *dstClipPolygon = newBounds;
}

void KisPerspectiveTransformWorker::init(const QTransform &transform)
{
    m_isIdentity = transform.isIdentity();

    m_forwardTransform = transform;
    m_backwardTransform = transform.inverted();

    if (m_dev) {
        m_srcRect = m_dev->exactBounds();

        QPolygonF dstClipPolygonUnused;

        fillParams(m_srcRect,
                   m_dev->defaultBounds()->bounds(),
                   &m_dstRegion,
                   &dstClipPolygonUnused);
    }
}

KisPerspectiveTransformWorker::~KisPerspectiveTransformWorker()
{
}

void KisPerspectiveTransformWorker::setForwardTransform(const QTransform &transform)
{
    init(transform);
}

void KisPerspectiveTransformWorker::run()
{
    KIS_ASSERT_RECOVER_RETURN(m_dev);

    if (m_isIdentity) return;

    KisPaintDeviceSP cloneDevice = new KisPaintDevice(*m_dev.data());

    // Clear the destination device, since all the tiles are already
    // shared with cloneDevice
    m_dev->clear();

    KIS_ASSERT_RECOVER_NOOP(!m_isIdentity);

    KisProgressUpdateHelper progressHelper(m_progressUpdater, 100, m_dstRegion.rectCount());

    KisRandomSubAccessorSP srcAcc = cloneDevice->createRandomSubAccessor();
    KisRandomAccessorSP accessor = m_dev->createRandomAccessorNG();

    Q_FOREACH (const QRect &rect, m_dstRegion.rects()) {
        for (int y = rect.y(); y < rect.y() + rect.height(); ++y) {
            for (int x = rect.x(); x < rect.x() + rect.width(); ++x) {

                QPointF dstPoint(x, y);
                QPointF srcPoint = m_backwardTransform.map(dstPoint);

                if (m_srcRect.contains(srcPoint)) {
                    accessor->moveTo(dstPoint.x(), dstPoint.y());
                    srcAcc->moveTo(srcPoint.x(), srcPoint.y());
                    srcAcc->sampledOldRawData(accessor->rawData());
                }
            }
        }
        progressHelper.step();
    }
}

void KisPerspectiveTransformWorker::runPartialDst(KisPaintDeviceSP srcDev,
                                                  KisPaintDeviceSP dstDev,
                                                  const QRect &dstRect)
{

    QRectF srcClipRect = srcDev->exactBounds();
    if (srcClipRect.isEmpty()) return;

    if (m_isIdentity) {

        if (srcDev->defaultBounds()->wrapAroundMode()) {
            KisProgressUpdateHelper progressHelper(m_progressUpdater, 100, dstRect.height()/ srcClipRect.height());
            for (int y = dstRect.y(); y < dstRect.y() + dstRect.height(); y+=srcClipRect.height()) {
                for (int x = dstRect.x(); x < dstRect.x() + dstRect.width(); x+=srcClipRect.width()) {
                    KisPainter::copyAreaOptimizedOldData(QPoint(x, y), srcDev, dstDev, srcClipRect.toRect());
                }
                progressHelper.step();
            }
            return;
        } else {
            KisPainter::copyAreaOptimizedOldData(dstRect.topLeft(), srcDev, dstDev, dstRect);
            return;
        }
    }

    KisProgressUpdateHelper progressHelper(m_progressUpdater, 100, dstRect.height());

    KisRandomSubAccessorSP srcAcc = srcDev->createRandomSubAccessor();
    KisRandomAccessorSP accessor = dstDev->createRandomAccessorNG();

    for (int y = dstRect.y(); y < dstRect.y() + dstRect.height(); ++y) {
        for (int x = dstRect.x(); x < dstRect.x() + dstRect.width(); ++x) {

            QPointF dstPoint(x, y);
            QPointF srcPoint = m_backwardTransform.map(dstPoint);

            if (srcClipRect.contains(srcPoint) || srcDev->defaultBounds()->wrapAroundMode()) {
                accessor->moveTo(dstPoint.x(), dstPoint.y());
                srcAcc->moveTo(srcPoint.x(), srcPoint.y());
                srcAcc->sampledOldRawData(accessor->rawData());
            }
        }
        progressHelper.step();
    }

}

QTransform KisPerspectiveTransformWorker::forwardTransform() const
{
    return m_forwardTransform;
}

QTransform KisPerspectiveTransformWorker::backwardTransform() const
{
    return m_backwardTransform;
}
