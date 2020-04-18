/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_algebra_2d_test.h"

#include <QTest>

#include "kis_algebra_2d.h"
#include "kis_debug.h"

namespace KisAlgebra2D {

}

void KisAlgebra2DTest::testHalfPlane()
{
    {
        QPointF a(10,10);
        QPointF b(5,5);

        KisAlgebra2D::RightHalfPlane p(a, b);

        QVERIFY(p.value(QPointF(7, 5)) > 0);
        QVERIFY(p.value(QPointF(3, 5)) < 0);
        QVERIFY(p.value(QPointF(3, 3)) == 0);
    }

    {
        QPointF a(10,10);
        QPointF b(15,10);

        KisAlgebra2D::RightHalfPlane p(a, b);
        QCOMPARE(p.value(QPointF(3, 5)), -5.0);
        QCOMPARE(p.value(QPointF(500, 15)), 5.0);
        QCOMPARE(p.value(QPointF(1000, 10)), 0.0);

        QCOMPARE(p.valueSq(QPointF(3, 5)), -25.0);
        QCOMPARE(p.valueSq(QPointF(500, 15)), 25.0);
        QCOMPARE(p.valueSq(QPointF(1000, 10)), 0.0);

        QCOMPARE(p.pos(QPointF(3, 5)), -1);
        QCOMPARE(p.pos(QPointF(500, 15)), 1);
        QCOMPARE(p.pos(QPointF(1000, 10)), 0);
    }
}

void KisAlgebra2DTest::testOuterCircle()
{
    QPointF a(10,10);
    KisAlgebra2D::OuterCircle p(a, 5);

    QVERIFY(p.value(QPointF(3, 5)) > 0);
    QVERIFY(p.value(QPointF(7, 7)) < 0);
    QVERIFY(p.value(QPointF(10, 5)) == 0);

    QCOMPARE(p.value(QPointF(10, 12)), -3.0);
    QCOMPARE(p.value(QPointF(10, 15)), 0.0);
    QCOMPARE(p.value(QPointF(10, 17)), 2.0);
}

void KisAlgebra2DTest::testQuadraticEquation()
{
    int result = 0;
    qreal x1 = 0;
    qreal x2 = 0;

    result = KisAlgebra2D::quadraticEquation(3, -11, 6, &x1, &x2);

    QCOMPARE(result, 2);
    QCOMPARE(x2, 2.0 / 3.0);
    QCOMPARE(x1, 3.0);

    result = KisAlgebra2D::quadraticEquation(9, -12, 4, &x1, &x2);

    QCOMPARE(result, 1);
    QCOMPARE(x1, 2.0 / 3.0);

    result = KisAlgebra2D::quadraticEquation(9, -1, 4, &x1, &x2);
    QCOMPARE(result, 0);
}

void KisAlgebra2DTest::testIntersections()
{
    QVector<QPointF> points;

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(20,10), 5.0);

    QCOMPARE(points.size(), 1);
    QCOMPARE(points[0], QPointF(15, 10));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(18,10), 5.0);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPointF(14, 13));
    QCOMPARE(points[1], QPointF(14, 7));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(10,20), 5.0);

    QCOMPARE(points.size(), 1);
    QCOMPARE(points[0], QPointF(10, 15));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(10,18), 5.0);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPointF(7, 14));
    QCOMPARE(points[1], QPointF(13, 14));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(17,17), 5.0);

    QCOMPARE(points.size(), 2);
    QCOMPARE(points[0], QPointF(13, 14));
    QCOMPARE(points[1], QPointF(14, 13));

    points = KisAlgebra2D::intersectTwoCircles(QPointF(10,10), 5.0,
                                               QPointF(10,100), 5.0);

    QCOMPARE(points.size(), 0);
}

void KisAlgebra2DTest::testWeirdIntersections()
{
    QVector<QPointF> points;

    QPointF c1 = QPointF(5369.14,3537.98);
    QPointF c2 = QPointF(5370.24,3536.71);
    qreal r1 = 8.5;
    qreal r2 = 10;

    points = KisAlgebra2D::intersectTwoCircles(c1, r1, c2, r2);

    QCOMPARE(points.size(), 2);
    //QCOMPARE(points[0], QPointF(15, 10));
}

void KisAlgebra2DTest::testMatrixDecomposition1()
{
    QTransform Sh;
    Sh.shear(0.2, 0);
    QTransform R;
    R.rotate(30);
    const QTransform t0 =
        QTransform::fromScale(0.5, -0.6) *
        R * Sh *
        QTransform::fromTranslate(100, 200);

    KisAlgebra2D::DecomposedMatix matrix(t0);

    QCOMPARE(matrix.isValid(), true);
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(matrix.transform(), t0, 1e-4));
}

void KisAlgebra2DTest::testMatrixDecomposition2()
{
    QPolygonF poly;
    poly << QPointF(-0.3,-0.3);
    poly << QPointF(1,0);
    poly << QPointF(0.8,0.8);
    poly << QPointF(0,1);

    QTransform t0;
    bool valid = QTransform::squareToQuad(poly, t0);
    QVERIFY(valid);

    KisAlgebra2D::DecomposedMatix matrix(t0);

    QCOMPARE(matrix.isValid(), true);
    QVERIFY(KisAlgebra2D::fuzzyMatrixCompare(matrix.transform(), t0, 1e-4));
}



void KisAlgebra2DTest::testDivisionWithFloor()
{
    QBENCHMARK {

    for (int a = -1000; a < 1000; a++) {
        for (int b = -1000; b < 1000; b++) {
            if (b == 0) continue;

            int refValue = qFloor(qreal(a) / b);
            int value = KisAlgebra2D::divideFloor(a, b);

            if (refValue != value) {
                qDebug() << ppVar(a) << ppVar(b);

                QCOMPARE(value, refValue);
            }
        }
    }
    }
}

#include <QPainter>
#include <QPainterPath>

void KisAlgebra2DTest::testDrawEllipse()
{
    QImage image(QSize(300,300), QImage::Format_ARGB32);
    image.fill(255);

    QPainter gc(&image);

    QTransform rot;
    rot.rotate(-30);

    QTransform shear;
    shear.shear(0.5, 0.3);

    const QTransform transform =
        rot * QTransform::fromTranslate(10, 30) * shear * QTransform::fromTranslate(150, 150);

    const qreal a = 100;
    const qreal b = 50;

    gc.setTransform(transform);
    gc.setPen(Qt::black);
    gc.drawEllipse(QPointF(0,0), a, b);

    gc.setPen(Qt::blue);
    gc.drawEllipse(QPointF(a, 0), 3, 3);

    gc.setPen(Qt::red);
    gc.drawEllipse(QPointF(0, b), 3, 3);

    QPointF newAxes;
    QTransform newTransform;

    std::tie(newAxes, newTransform) = KisAlgebra2D::transformEllipse(QPointF(a, b), transform);

    gc.setOpacity(50);
    gc.resetTransform();
    gc.setTransform(newTransform);
    gc.setPen(QPen(Qt::blue, 2));
    gc.drawEllipse(QPointF(0,0), newAxes.x(), newAxes.y());

    gc.setPen(QPen(Qt::green, 2));
    gc.drawEllipse(QPointF(newAxes.x(), 0), 5, 5);

    gc.setPen(Qt::yellow);
    gc.drawEllipse(QPointF(0, newAxes.y()), 5, 5);

    image.save("ellipse_result.png");
}

void KisAlgebra2DTest::testNullRectProcessing()
{
    QPainterPath line;
    line.moveTo(10,10);
    line.lineTo(110,10);

    const QRectF lineRect(line.boundingRect());
    qDebug() << ppVar(lineRect) << ppVar(lineRect.isValid()) << ppVar(lineRect.isNull());



    // test relative operations
    const QPointF relPoint = KisAlgebra2D::absoluteToRelative(QPointF(30, 10), lineRect);
    QCOMPARE(relPoint, QPointF(0.2, 0.0));

    const QPointF absPoint = KisAlgebra2D::relativeToAbsolute(relPoint, lineRect);
    QCOMPARE(absPoint, QPointF(30.0, 10.0));

    const QTransform relTransform = KisAlgebra2D::mapToRect(lineRect);
    QCOMPARE(relTransform.map(relPoint), absPoint);

    // test relative isotropic operations
    QCOMPARE(KisAlgebra2D::absoluteToRelative(
                 KisAlgebra2D::relativeToAbsolute(0.2, lineRect),
                 lineRect),
             0.2);

    /// test transformations

    // translate
    QCOMPARE(QTransform::fromTranslate(10, 20).mapRect(lineRect), QRect(20, 30, 100, 0));

    // scale
    QCOMPARE(QTransform::fromScale(2.0, 2.0).mapRect(lineRect), QRect(20, 20, 200, 0));

    // rotate
    QTransform rot;
    rot.rotate(90);
    QCOMPARE(rot.mapRect(lineRect), QRect(-10, 10, 0, 100));

    // shear-x
    QTransform shearX;
    shearX.shear(2.0, 0.0);
    QCOMPARE(shearX.mapRect(lineRect), QRect(30, 10, 100, 0));

    // shear-y
    QTransform shearY;
    shearY.shear(0.0, 2.0);
    QCOMPARE(shearY.mapRect(lineRect), QRect(10, 30, 100, 200));

    /// binary operations

    QCOMPARE(QRectF() | lineRect, lineRect);
    QCOMPARE(QRectF(10, 10, 40, 40) | lineRect, QRectF(10, 10, 100, 40));

    QCOMPARE(QRectF(20, 20, 40, 40) & lineRect, QRectF());

    QEXPECT_FAIL("", "Qt's konjunstion operator doesn't work with line-rects", Continue);
    QCOMPARE(QRectF(10, 10, 40, 40) & lineRect, QRectF(10, 10, 40, 0));

    /// QPolygon's bounding rect

    QCOMPARE(QPolygonF(lineRect).boundingRect(), lineRect);
}

QTEST_MAIN(KisAlgebra2DTest)
