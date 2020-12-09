/* This file is part of the KDE project
   Copyright (C) 2006, 2011 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007,2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOPATHSHAPE_H
#define KOPATHSHAPE_H

#include "kritaflake_export.h"

#include <QMetaType>
#include <QTransform>

#include "KoTosContainer.h"

#define KoPathShapeId "KoPathShape"

class KoPathSegment;
class KoPathPoint;
class KoPathShapePrivate;
class KoMarker;
class KisHandlePainterHelper;

typedef QPair<int, int> KoPathPointIndex;

/// a KoSubpath contains a path from a moveTo until a close or a new moveTo
typedef QList<KoPathPoint *> KoSubpath;
typedef QList<KoSubpath *> KoSubpathList;
/// The position of a path point within a path shape
/**
 * @brief This is the base for all graphical objects.
 *
 * All graphical objects are based on this object e.g. lines, rectangulars, pies
 * and so on.
 *
 * The KoPathShape uses KoPathPoint's to describe the path of the shape.
 *
 * Here a short example:
 * 3 points connected by a curveTo's described by the following svg:
 * M 100,200 C 100,100 250,100 250,200 C 250,200 400,300 400,200.
 *
 * This will be stored in 3 KoPathPoint's as
 * The first point contains in
 *       point 100,200
 *       controlPoint2 100,100
 * The second point contains in
 *       point 250,200
 *       controlPoint1 250,100
 *       controlPoint2 250,300
 * The third point contains in
 *       point 400,300
 *       controlPoint1 400,200
 *
 * Not the segments are stored but the points. Out of the points the segments are
 * generated. See the outline method. The reason for storing it like that is that
 * it is the points that are modified by the user and not the segments.
 */
class KRITAFLAKE_EXPORT KoPathShape : public KoTosContainer
{
public:
    /**
     * @brief constructor
     */
    KoPathShape();

    /**
     * @brief
     */
    ~KoPathShape() override;

    KoShape *cloneShape() const override;

    /// reimplemented
    void paint(QPainter &painter, KoShapePaintingContext &paintContext) const override;
    virtual void paintPoints(KisHandlePainterHelper &handlesHelper);

    /// reimplemented
    QRectF outlineRect() const override;
    /// reimplemented
    QPainterPath outline() const override;
    /// reimplemented
    QRectF boundingRect() const override;
    /// reimplemented
    QSizeF size() const override;

    QPainterPath pathStroke(const QPen &pen) const;
    /**
     * Resize the shape
     *
     * This makes sure that the pathshape will not be resized to 0 if the new size
     * is null as that makes it impossible to undo the change.
     *
     * All functions that overwrite this function should also use the resizeMatrix
     * function to get and use the same data in resizing.
     *
     * @see resizeMatrix()
     */
    void setSize(const QSizeF &size) override;

    /// reimplemented
    bool hitTest(const QPointF &position) const override;

    /// Removes all subpaths and their points from the path
    void clear();
    /**
     * @brief Starts a new Subpath
     *
     * Moves the pen to p and starts a new subpath.
     *
     * @return the newly created point
     */
    KoPathPoint *moveTo(const QPointF &p);

    /**
     * @brief Adds a new line segment
     *
     * Adds a straight line between the last point and the given point p.
     *
     * @return the newly created point
     */
    KoPathPoint *lineTo(const QPointF &p);

    /**
     * @brief Adds a new cubic Bezier curve segment.
     *
     * Adds a cubic Bezier curve between the last point and the given point p,
     * using the control points specified by c1 and c2.
     *
     * @param c1 control point1
     * @param c2 control point2
     * @param p the endpoint of this curve segment
     *
     * @return The newly created point
     */
    KoPathPoint *curveTo(const QPointF &c1, const QPointF &c2, const QPointF &p);

    /**
     * @brief Adds a new quadratic Bezier curve segment.
     *
     * Adds a quadratic Bezier curve between the last point and the given point p,
     * using the control point specified by c.
     *
     * @param c control point
     * @param p the endpoint of this curve segment
     *
     * @return The newly created point
     */
    KoPathPoint *curveTo(const QPointF &c, const QPointF &p);

    /**
     * @brief Add an arc.
     *
     * Adds an arc starting at the current point. The arc will be converted to bezier curves.
     *
     * @param rx x radius of the ellipse
     * @param ry y radius of the ellipse
     * @param startAngle the angle where the arc will be started
     * @param sweepAngle the length of the angle
     * TODO add param to have angle of the ellipse
     *
     * @return The newly created point
     */
    KoPathPoint *arcTo(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle);


    /**
     * @brief Closes the current subpath
     */
    void close();

    /**
     * @brief Closes the current subpath
     *
     * It tries to merge the last and first point of the subpath
     * to one point and then closes the subpath. If merging is not
     * possible as the two point are to far from each other a close
     * will be done.
     * TODO define a maximum distance between  the two points until this is working
     */
    void closeMerge();

    /**
     * @brief Normalizes the path data.
     *
     * The path points are transformed so that the top-left corner
     * of the bounding rect is at (0,0).
     * This should be called after adding points to the path or changing
     * positions of path points.
     * @return the offset by which the points are moved in shape coordinates.
     */
    virtual QPointF normalize();

    /**
     * @brief Returns the path points within the given rectangle.
     * @param rect the rectangle the requested points are in
     * @return list of points within the rectangle
     */
    QList<KoPathPoint*> pointsAt(const QRectF &rect) const;

    /**
     * @brief Returns the list of path segments within the given rectangle.
     * @param rect the rectangle the requested segments are in
     * @return list of segments within the rectangle
     */
    QList<KoPathSegment> segmentsAt(const QRectF &rect) const;

    /**
     * @brief Returns the path point index of a given path point
     *
     * @param point the point for which you want to get the index
     * @return path point index of the point if it exists
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex pathPointIndex(const KoPathPoint *point) const;

    /**
     * @brief Returns the path point specified by a path point index
     *
     * @param pointIndex index of the point to get
     *
     * @return KoPathPoint on success, 0 otherwise e.g. out of bounds
     */
    KoPathPoint *pointByIndex(const KoPathPointIndex &pointIndex) const;

    /**
     * @brief Returns the segment specified by a path point index
     *
     * A segment is defined by the point index of the first point in the segment.
     * A segment contains the defined point and its following point. If the subpath is
     * closed and the and the pointIndex point to the last point in the subpath, the
     * following point is the first point in the subpath.
     *
     * @param pointIndex index of the first point of the segment
     *
     * @return Segment containing both points of the segment or KoPathSegment( 0, 0 ) on error e.g. out of bounds
     */
    KoPathSegment segmentByIndex(const KoPathPointIndex &pointIndex) const;

    /**
     * @brief Returns the number of points in the path
     *
     * @return The number of points in the path
     */
    int pointCount() const;

    /**
     * @brief Returns the number of subpaths in the path
     *
     * @return The number of subpaths in the path
     */
    int subpathCount() const;

    /**
     * @brief Returns the number of points in a subpath
     *
     * @return The number of points in the subpath or -1 if subpath out of bounds
     */
    int subpathPointCount(int subpathIndex) const;

    /**
     * @brief Checks if a subpath is closed
     *
     * @param subpathIndex index of the subpath to check
     *
     * @return true when the subpath is closed, false otherwise
     */
    bool isClosedSubpath(int subpathIndex) const;

    /**
     * @brief Inserts a new point into the given subpath at the specified position
     *
     * This method keeps the subpath closed if it is closed, and open when it was
     * open. So it can change the properties of the point inserted.
     * You might need to update the point before/after to get the desired result
     * e.g. when you insert the point into a curve.
     *
     * @param point to insert
     * @param pointIndex index at which the point should be inserted
     *
     * @return true on success,
     *         false when pointIndex is out of bounds
     */
    bool insertPoint(KoPathPoint *point, const KoPathPointIndex &pointIndex);

    /**
     * @brief Removes a point from the path.
     *
     * Note that the ownership of the point will pass to the caller.
     *
     * @param pointIndex index of the point which should be removed
     *
     * @return The removed point on success,
     *         otherwise 0
     */
    KoPathPoint *removePoint(const KoPathPointIndex &pointIndex);

    /**
     * @brief Breaks the path after the point index
     *
     * The new subpath will be behind the one that was broken. The segment between
     * the given point and the one behind will be removed. If you want to split at
     * one point insert first a copy of the point behind it.
     * This does not work when the subpath is closed. Use openSubpath for this.
     * It does not break at the last position of a subpath or if there is only one
     * point in the subpath.
     *
     * @param pointIndex index of the point after which the path should be broken
     *
     * @return true if the subpath was broken, otherwise false
     */
    bool breakAfter(const KoPathPointIndex &pointIndex);

    /**
     * @brief Joins the given subpath with the following one
     *
     * Joins the given subpath with the following one by inserting a segment between
     * the two subpaths.
     * This does nothing if the specified subpath is the last subpath
     * or one of both subpaths is closed.
     *
     * @param subpathIndex index of the subpath being joined with the following subpath
     *
     * @return true if the subpath was joined, otherwise false
     */
    bool join(int subpathIndex);

    /**
     * @brief Moves the position of a subpath within a path
     *
     * @param oldSubpathIndex old index of the subpath
     * @param newSubpathIndex new index of the subpath
     *
     * @return true if the subpath was moved, otherwise false e.g. if an index is out of bounds
     */
    bool moveSubpath(int oldSubpathIndex, int newSubpathIndex);

    /**
     * @brief Opens a closed subpath
     *
     * The subpath is opened by removing the segment before the given point, making
     * the given point the new start point of the subpath.
     *
     * @param pointIndex the index of the point at which to open the closed subpath
     * @return the new position of the old first point in the subpath
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex openSubpath(const KoPathPointIndex &pointIndex);

    /**
     * @brief Close a open subpath
     *
     * The subpath is closed be inserting a segment between the start and end point, making
     * the given point the new start point of the subpath.
     *
     * @return the new position of the old first point in the subpath
     *         otherwise KoPathPointIndex( -1, -1 )
     */
    KoPathPointIndex closeSubpath(const KoPathPointIndex &pointIndex);

    /**
     * @brief Reverse subpath
     *
     * The last point becomes the first point and the first one becomes the last one.
     *
     * @param subpathIndex the index of the subpath to reverse
     */
    bool reverseSubpath(int subpathIndex);

    /**
     * @brief Removes subpath from the path
     * @param subpathIndex the index of the subpath to remove
     * @return the removed subpath on success, 0 otherwise.
     */
    KoSubpath *removeSubpath(int subpathIndex);

    /**
     * @brief Adds a subpath at the given index to the path
     * @param subpath the subpath to add
     * @param subpathIndex the index at which the new subpath should be inserted
     * @return true on success, false otherwise e.g. subpathIndex out of bounds
     */
    bool addSubpath(KoSubpath *subpath, int subpathIndex);

    /**
     * @brief Combines two path shapes by appending the data of the specified path.
     * @param path the path to combine with
     * @return index of the first segment inserted or -1 on failure
     */
    int combine(KoPathShape *path);

    /**
     * @brief Creates separate path shapes, one for each existing subpath.
     * @param separatedPaths the list which contains the separated path shapes
     * @return true if separating the path was successful, false otherwise
     */
    bool separate(QList<KoPathShape*> &separatedPaths);

    /**
     * Returns the specific path shape id.
     *
     * Path shape derived shapes have a different shape id which link them
     * to their respective shape factories. In most cases they do not have
     * a special tool for editing them.
     * This function returns the specific shape id for finding the shape
     * factory from KoShapeRegistry. The default KoPathShapeId is returned
     * from KoShape::shapeId() so that the generic path editing tool gets
     * activated when the shape is selected.
     *
     * @return the specific shape id
     */
    virtual QString pathShapeId() const;

    /// Returns a odf/svg string representation of the path data with the given matrix applied.
    QString toString(const QTransform &matrix = QTransform()) const;

    /// Returns the fill rule for the path object
    Qt::FillRule fillRule() const;

    /// Sets the fill rule to be used for painting the background
    void setFillRule(Qt::FillRule fillRule);

    /// Creates path shape from given QPainterPath
    static KoPathShape *createShapeFromPainterPath(const QPainterPath &path);

    void setMarker(KoMarker *marker, KoFlake::MarkerPosition pos);
    KoMarker* marker(KoFlake::MarkerPosition pos) const;
    bool hasMarkers() const;

    bool autoFillMarkers() const;
    void setAutoFillMarkers(bool value);

public:
    struct KRITAFLAKE_EXPORT PointSelectionChangeListener : public ShapeChangeListener {
        void notifyShapeChanged(ChangeType type, KoShape *shape) override;
        virtual void recommendPointSelectionChange(KoPathShape *shape, const QList<KoPathPointIndex> &newSelection) = 0;
        virtual void notifyPathPointsChanged(KoPathShape *shape) = 0;
    };

    void recommendPointSelectionChange(const QList<KoPathPointIndex> &newSelection);

protected:
    void notifyPointsChanged();

protected:
    /// constructor: to be used in cloneShape(), not in descendants!
    /// \internal
    /// XXX private?
    KoPathShape(const KoPathShape &rhs);

protected:

    /**
     * @brief Add an arc.
     *
     * Adds an arc starting at the current point. The arc will be converted to bezier curves.
     * @param rx x radius of the ellipse
     * @param ry y radius of the ellipse
     * @param startAngle the angle where the arc will be started
     * @param sweepAngle the length of the angle
     * TODO add param to have angle of the ellipse
     * @param offset to the first point in the arc
     * @param curvePoints a array which take the curve points, pass a 'QPointF curvePoins[12]';
     *
     * @return number of points created by the curve
     */
    int arcToCurve(qreal rx, qreal ry, qreal startAngle, qreal sweepAngle, const QPointF &offset, QPointF *curvePoints) const;

    /**
     * Get the resize matrix
     *
     * This makes sure that also if the newSize isNull that there will be a
     * very small size of 0.000001 pixels
     */
    QTransform resizeMatrix( const QSizeF &newSize ) const;

private:
    /// close-merges specified subpath
    void closeMergeSubpathPriv(KoSubpath *subpath);
    /// closes specified subpath
    void closeSubpathPriv(KoSubpath *subpath);
    void updateLastPriv(KoPathPoint **lastPoint);

protected:
    const KoSubpathList &subpaths() const;
    /// XXX: refactor this using setter?
    KoSubpathList &subpaths();
    void map(const QTransform &matrix);

private:
    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_METATYPE(KoPathShape*)

#endif /* KOPATHSHAPE_H */
