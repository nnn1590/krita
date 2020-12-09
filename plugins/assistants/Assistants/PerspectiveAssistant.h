/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PERSPECTIVE_ASSISTANT_H_
#define _PERSPECTIVE_ASSISTANT_H_

#include "kis_abstract_perspective_grid.h"
#include "kis_painting_assistant.h"
#include <QObject>
#include <QPolygonF>
#include <QLineF>
#include <QTransform>

class PerspectiveAssistant : public KisAbstractPerspectiveGrid, public KisPaintingAssistant
{
    Q_OBJECT
public:
    PerspectiveAssistant(QObject * parent = 0);
    KisPaintingAssistantSP clone(QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap) const override;

    QPointF adjustPosition(const QPointF& point, const QPointF& strokeBegin) override;
    void setAdjustedBrushPosition(const QPointF position) override;
    void setFollowBrushPosition(bool follow) override;
    void endStroke() override;

    QPointF getEditorPosition() const override;
    int numHandles() const override { return 4; }
    void drawAssistant(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter* converter, bool cached = true,KisCanvas2* canvas=0, bool assistantVisible=true, bool previewVisible=true) override;

    bool contains(const QPointF& point) const override;
    qreal distance(const QPointF& point) const override;


    bool isAssistantComplete() const override;

protected:
    void drawCache(QPainter& gc, const KisCoordinatesConverter *converter,  bool assistantVisible=true) override;
private:
    QPointF project(const QPointF& pt, const QPointF& strokeBegin);
    // creates the convex hull, returns false if it's not a quadrilateral
    bool quad(QPolygonF& out) const;
    // finds the transform from perspective coordinates (a unit square) to the document
    bool getTransform(QPolygonF& polyOut, QTransform& transformOut) const;
    explicit PerspectiveAssistant(const PerspectiveAssistant &rhs, QMap<KisPaintingAssistantHandleSP, KisPaintingAssistantHandleSP> &handleMap);

    // which direction to snap to (in transformed coordinates)
    QLineF m_snapLine;
    // cached information
    mutable QTransform m_cachedTransform;
    mutable QPolygonF m_cachedPolygon;
    mutable QPointF m_cachedPoints[4];
    mutable bool m_cacheValid;
    // Needed to make sure that when we are in the middle of a brush stroke, the
    // guides follow the brush position, not the cursor position.
    bool m_followBrushPosition;
    bool m_adjustedPositionValid;
    QPointF m_adjustedBrushPosition;
};

class PerspectiveAssistantFactory : public KisPaintingAssistantFactory
{
public:
    PerspectiveAssistantFactory();
    ~PerspectiveAssistantFactory() override;
    QString id() const override;
    QString name() const override;
    KisPaintingAssistant* createPaintingAssistant() const override;
};

#endif
