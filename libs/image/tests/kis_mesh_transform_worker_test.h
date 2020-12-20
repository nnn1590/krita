/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MESH_TRANSFORM_WORKER_TEST_H
#define __KIS_MESH_TRANSFORM_WORKER_TEST_H

#include <QtTest>

class KisMeshTransformWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIsCurveLinear();

    void testPointsQImage();
    void testGradient();

    void testMeshSubdivision();

    void testGlobalToLocal();

    void testDistanceToCurve();

    void testRemovePoint();

    void testIsIdentity();

    void testSerialization();

    void testIteratorConstness();
};

#endif /* __KIS_MESH_TRANSFORM_WORKER_TEST_H */
