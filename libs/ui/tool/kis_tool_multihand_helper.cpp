/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_multihand_helper.h"

#include <QTransform>

#include "kis_painter.h"
#include <strokes/KisFreehandStrokeInfo.h>
#include "kis_algebra_2d.h"

struct KisToolMultihandHelper::Private
{
    QVector<QTransform> transformations;
};

KisToolMultihandHelper::KisToolMultihandHelper(KisPaintingInformationBuilder *infoBuilder,
                                               KoCanvasResourceProvider *resourceManager,
                                               const KUndo2MagicString &transactionText)
    : KisToolFreehandHelper(infoBuilder, resourceManager, transactionText)
    , d(new Private)
{
}

KisToolMultihandHelper::~KisToolMultihandHelper()
{
    delete d;
}

void KisToolMultihandHelper::setupTransformations(const QVector<QTransform> &transformations)
{
    d->transformations = transformations;
}

void KisToolMultihandHelper::createPainters(QVector<KisFreehandStrokeInfo*> &strokeInfos,
                                            const KisDistanceInformation &startDist)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];
        KisDistanceInitInfo __startDistInfo(transform.map(startDist.lastPosition()),
                                            startDist.lastDrawingAngle(),
                                            startDist.getSpacingInterval(),
                                            startDist.getTimingUpdateInterval(),
                                            0);

        KisDistanceInformation __startDist = __startDistInfo.makeDistInfo();
        strokeInfos << new KisFreehandStrokeInfo(__startDist);
    }
}

void adjustPointInformationRotation(KisPaintInformation &pi, const QTransform &t)
{
    KisAlgebra2D::DecomposedMatix d(t);

    qreal rotation = d.angle;
    const bool mirrorX = KisAlgebra2D::signPZ(d.scaleX) < 0;
    const bool mirrorY = KisAlgebra2D::signPZ(d.scaleY) < 0;

    pi.setCanvasMirroredH(pi.canvasMirroredH() ^ mirrorX);
    pi.setCanvasMirroredV(pi.canvasMirroredV() ^ mirrorY);

    if (pi.canvasMirroredH()!= pi.canvasMirroredV()) {
        rotation = normalizeAngleDegrees(360.0 - rotation);
    }

    pi.setCanvasRotation(normalizeAngleDegrees(pi.canvasRotation() - rotation));
}


void KisToolMultihandHelper::paintAt(const KisPaintInformation &pi)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];
        KisPaintInformation __pi = pi;
        __pi.setPos(transform.map(__pi.pos()));
        adjustPointInformationRotation(__pi, transform);
        paintAt(i, __pi);
    }
}

void KisToolMultihandHelper::paintLine(const KisPaintInformation &pi1,
                                       const KisPaintInformation &pi2)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];

        KisPaintInformation __pi1 = pi1;
        KisPaintInformation __pi2 = pi2;
        __pi1.setPos(transform.map(__pi1.pos()));
        __pi2.setPos(transform.map(__pi2.pos()));

        adjustPointInformationRotation(__pi1, transform);
        adjustPointInformationRotation(__pi2, transform);

        paintLine(i, __pi1, __pi2);
    }
}

void KisToolMultihandHelper::paintBezierCurve(const KisPaintInformation &pi1,
                                              const QPointF &control1,
                                              const QPointF &control2,
                                              const KisPaintInformation &pi2)
{
    for (int i = 0; i < d->transformations.size(); i++) {
        const QTransform &transform = d->transformations[i];

        KisPaintInformation __pi1 = pi1;
        KisPaintInformation __pi2 = pi2;
        __pi1.setPos(transform.map(__pi1.pos()));
        __pi2.setPos(transform.map(__pi2.pos()));

        adjustPointInformationRotation(__pi1, transform);
        adjustPointInformationRotation(__pi2, transform);

        QPointF __control1 = transform.map(control1);
        QPointF __control2 = transform.map(control2);

        paintBezierCurve(i, __pi1, __control1, __control2, __pi2);
    }
}
