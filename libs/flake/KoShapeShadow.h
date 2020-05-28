/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPESHADOW_H
#define KOSHAPESHADOW_H

#include "kritaflake_export.h"

#include <QtGlobal>

class KoShape;
class KoShapeSavingContext;
class QPainter;
class QPointF;
class QColor;
struct KoInsets;

class KRITAFLAKE_EXPORT KoShapeShadow
{
public:
    KoShapeShadow();
    ~KoShapeShadow();

    KoShapeShadow(const KoShapeShadow &rhs);
    KoShapeShadow& operator=(const KoShapeShadow &rhs);

    /**
     * Paints the shadow of the shape.
     * @param shape the shape to paint around
     * @param painter the painter to paint shadows to canvas
     * @param converter to convert between internal and view coordinates.
     */
    void paint(KoShape *shape, QPainter &painter);

    /**
     * Sets the shadow offset from the topleft corner of the shape
     * @param offset the shadow offset
     */
    void setOffset(const QPointF &offset);

    /// Returns the shadow offset
    QPointF offset() const;

    /**
     * Sets the shadow color, including the shadow opacity.
     * @param color the shadow color and opacity
     */
    void setColor(const QColor &color);

    /// Returns the shadow color including opacity
    QColor color() const;

    /**
     * Sets the shadow blur radius of the shape
     * @param blur the shadow blur radius
     */
    void setBlur(qreal blur);

    /// Returns the shadow blur radius
    qreal blur() const;

    /// Sets the shadow visibility
    void setVisible(bool visible);

    /// Returns if shadow is visible
    bool isVisible() const;

    /// Fills the insets object with the space the shadow takes around a shape
    void insets(KoInsets &insets) const;

    /**
     * Increments the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool ref();
    /**
     * Decrements the use-value.
     * Returns true if the new value is non-zero, false otherwise.
     */
    bool deref();
    /// Return the usage count
    int useCount() const;

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPESHADOW_H
