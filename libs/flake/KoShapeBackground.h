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

#ifndef KOSHAPEBACKGROUND_H
#define KOSHAPEBACKGROUND_H

#include "kritaflake_export.h"

#include <QtGlobal>

class QSizeF;
class QPainter;
class QPainterPath;
class KoShapeSavingContext;
class KoShapePaintingContext;

/**
 * This is the base class for shape backgrounds.
 * Derived classes are used to paint the background of
 * a shape within a given painter path.
 */
class KRITAFLAKE_EXPORT KoShapeBackground
{
public:
    KoShapeBackground();
    virtual ~KoShapeBackground();

    /// Paints the background using the given fill path
    virtual void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const = 0;

    /// Returns if the background has some transparency.
    virtual bool hasTransparency() const;

    virtual bool compareTo(const KoShapeBackground *other) const = 0;

    virtual explicit operator bool() const { return true; }

};

#endif // KOSHAPEBACKGROUND_H
