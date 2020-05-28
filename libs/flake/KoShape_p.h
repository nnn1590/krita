/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KOSHAPEPRIVATE_H
#define KOSHAPEPRIVATE_H

#include "KoShape.h"

#include <QPoint>
#include <QPaintDevice>
#include <QTransform>
#include <QScopedPointer>
#include <QSharedData>

#include <KoClipMask.h>

class KoShapeManager;


class KoShape::SharedData : public QSharedData
{
public:
    explicit SharedData();
    virtual ~SharedData();

    explicit SharedData(const SharedData &rhs);


public:
    // Members

    mutable QSizeF size; // size in pt
    QString shapeId;
    QString name; ///< the shapes names

    QTransform localMatrix; ///< the shapes local transformation matrix

    QScopedPointer<KoShapeUserData> userData;
    QSharedPointer<KoShapeStrokeModel> stroke; ///< points to a stroke, or 0 if there is no stroke
    QSharedPointer<KoShapeBackground> fill; ///< Stands for the background color / fill etc.
    bool inheritBackground = false;
    bool inheritStroke = false;
    KoShapeShadow * shadow; ///< the current shape shadow
    // XXX: change this to instance instead of pointer
    QScopedPointer<KoClipPath> clipPath; ///< the current clip path
    QScopedPointer<KoClipMask> clipMask; ///< the current clip mask
    QMap<QString, QString> additionalAttributes;
    QMap<QByteArray, QString> additionalStyleAttributes;
    KoFilterEffectStack *filterEffectStack; ///< stack of filter effects applied to the shape
    qreal transparency; ///< the shapes transparency
    QString hyperLink; //hyperlink for this shape

    int zIndex : 16; // keep maxZIndex in sync!
    int runThrough : 16;
    int visible : 1;
    int printable : 1;
    int geometryProtected : 1;
    int keepAspect : 1;
    int selectable : 1;
    int protectContent : 1;

    KoShape::TextRunAroundSide textRunAroundSide;
    qreal textRunAroundDistanceLeft;
    qreal textRunAroundDistanceTop;
    qreal textRunAroundDistanceRight;
    qreal textRunAroundDistanceBottom;
    qreal textRunAroundThreshold;
    KoShape::TextRunAroundContour textRunAroundContour;
};

class KoShape::Private
{
public:
    KoShapeContainer *parent;
    QSet<KoShapeManager *> shapeManagers;
    QSet<KoShape *> toolDelegates;
    QList<KoShape*> dependees; ///< list of shape dependent on this shape
    QList<KoShape::ShapeChangeListener*> listeners;
};

#endif
