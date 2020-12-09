/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KARBONCALLIGRAPHICSHAPE_H
#define KARBONCALLIGRAPHICSHAPE_H

#include <KoParameterShape.h>
#include <QDebug>

#define KarbonCalligraphicShapeId "KarbonCalligraphicShape"

class KarbonCalligraphicPoint
{
public:
    KarbonCalligraphicPoint(const QPointF &point, qreal angle, qreal width)
        : m_point(point), m_angle(angle), m_width(width) {}

    KarbonCalligraphicPoint(const KarbonCalligraphicPoint &rhs) = default;
    KarbonCalligraphicPoint() = default;

    bool operator==(const KarbonCalligraphicPoint &rhs) const {
        return m_point == rhs.m_point &&
            qFuzzyCompare(m_angle, rhs.m_angle) &&
            qFuzzyCompare(m_width, rhs.m_width);
    }

    QPointF point() const
    {
        return m_point;
    }
    qreal angle() const
    {
        return m_angle;
    }
    qreal width() const
    {
        return m_width;
    }

    void setPoint(const QPointF &point)
    {
        m_point = point;
    }
    void setAngle(qreal angle)
    {
        m_angle = angle;
    }

private:
    QPointF m_point; // in shape coordinates
    qreal m_angle = 0.0;
    qreal m_width = 0.0;
};

inline QDebug operator<<(QDebug dbg, const KarbonCalligraphicPoint &pt)
{
    dbg.nospace() << "(" << pt.point() << ", a: " << pt.angle() << ", w: " << pt.width() << ")";
    return dbg.space();
}

/*class KarbonCalligraphicShape::Point
{
public:
    KoPainterPath(KoPathPoint *point) : m_prev(point), m_next(0) {}

    // calculates the effective point
    QPointF point() {
        if (m_next = 0)
            return m_prev.point();

        // m_next != 0
        qDebug() << "not implemented yet!!!!";
        return QPointF();
    }

private:
    KoPainterPath m_prev;
    KoPainterPath m_next;
    qreal m_percentage;
};*/

// the indexes of the path will be similar to:
//        7--6--5--4   <- pointCount() / 2
// start  |        |   end    ==> (direction of the stroke)
//        0--1--2--3
class KarbonCalligraphicShape : public KoParameterShape
{
public:
    explicit KarbonCalligraphicShape(qreal caps = 0.0);
    ~KarbonCalligraphicShape() override;

    KoShape* cloneShape() const override;

    void appendPoint(const QPointF &p1, qreal angle, qreal width);
    void appendPointToPath(const KarbonCalligraphicPoint &p);

    // returns the bounding rect of what needs to be repainted
    // after new points are added
    const QRectF lastPieceBoundingRect();

    void setSize(const QSizeF &newSize) override;
    //virtual QPointF normalize();

    QPointF normalize() override;

    void simplifyPath();

    void simplifyGuidePath();

    // reimplemented
    QString pathShapeId() const override;

protected:
    // reimplemented
    void moveHandleAction(int handleId,
                          const QPointF &point,
                          Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;

    // reimplemented
    void updatePath(const QSizeF &size) override;

private:
    KarbonCalligraphicShape(const KarbonCalligraphicShape &rhs);

    // auxiliary function that actually inserts the points
    // without doing any additional checks
    // the points should be given in canvas coordinates
    void appendPointsToPathAux(const QPointF &p1, const QPointF &p2);

    // function to detect a flip, given the points being inserted
    bool flipDetected(const QPointF &p1, const QPointF &p2);

    void smoothLastPoints();
    void smoothPoint(const int index);

    // determine whether the points given are in counterclockwise order or not
    // returns +1 if they are, -1 if they are given in clockwise order
    // and 0 if they form a degenerate triangle
    static int ccw(const QPointF &p1, const QPointF &p2, const QPointF &p3);

    //
    void addCap(int index1, int index2, int pointIndex, bool inverted = false);

    struct Private;
    QSharedDataPointer<Private> s;
};

#endif // KARBONCALLIGRAPHICSHAPE_H

