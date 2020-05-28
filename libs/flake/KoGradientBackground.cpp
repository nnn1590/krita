/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoGradientBackground.h"
#include "KoFlake.h"
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>

#include <FlakeDebug.h>

#include <QSharedPointer>
#include <QBrush>
#include <QPainter>
#include <QSharedData>
#include <QPainterPath>

class KoGradientBackground::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , gradient(0)
    {}

    QGradient *gradient;
    QTransform matrix;
};

KoGradientBackground::KoGradientBackground(QGradient * gradient, const QTransform &matrix)
    : KoShapeBackground()
    , d(new Private)
{
    d->gradient = gradient;
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoGradientBackground::KoGradientBackground(const QGradient & gradient, const QTransform &matrix)
    : KoShapeBackground()
    , d(new Private)
{
    d->gradient = KoFlake::cloneGradient(&gradient);
    d->matrix = matrix;
    Q_ASSERT(d->gradient);
}

KoGradientBackground::~KoGradientBackground()
{
    delete d->gradient;
}

bool KoGradientBackground::compareTo(const KoShapeBackground *other) const
{
    const KoGradientBackground *otherGradient = dynamic_cast<const KoGradientBackground*>(other);

    return otherGradient &&
        d->matrix == otherGradient->d->matrix &&
        *d->gradient == *otherGradient->d->gradient;
}

void KoGradientBackground::setTransform(const QTransform &matrix)
{
    d->matrix = matrix;
}

QTransform KoGradientBackground::transform() const
{
    return d->matrix;
}

void KoGradientBackground::setGradient(const QGradient &gradient)
{
    delete d->gradient;

    d->gradient = KoFlake::cloneGradient(&gradient);
    Q_ASSERT(d->gradient);
}

const QGradient * KoGradientBackground::gradient() const
{
    return d->gradient;
}

void KoGradientBackground::paint(QPainter &painter, KoShapePaintingContext &/*context*/, const QPainterPath &fillPath) const
{
    if (!d->gradient) return;

    if (d->gradient->coordinateMode() == QGradient::ObjectBoundingMode) {

        /**
         * NOTE: important hack!
         *
         * Qt has different notation of QBrush::setTransform() in comparison
         * to what SVG defines. SVG defines gradientToUser matrix to be postmultiplied
         * by QBrush::transform(), but Qt does exactly reverse!
         *
         * That most probably has beed caused by the fact that Qt uses transposed
         * matrices and someone just mistyped the stuff long ago :(
         *
         * So here we basically emulate this feature by converting the gradient into
         * QGradient::LogicalMode and doing transformations manually.
         */

        const QRectF boundingRect = fillPath.boundingRect();
        QTransform gradientToUser(boundingRect.width(), 0, 0, boundingRect.height(),
                                  boundingRect.x(), boundingRect.y());

        // TODO: how about slicing the object?
        QGradient g = *d->gradient;
        g.setCoordinateMode(QGradient::LogicalMode);

        QBrush b(g);
        b.setTransform(d->matrix * gradientToUser);
        painter.setBrush(b);
    } else {
        QBrush b(*d->gradient);
        b.setTransform(d->matrix);
        painter.setBrush(b);
    }

    painter.drawPath(fillPath);
}
