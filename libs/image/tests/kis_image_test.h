/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_TESTER_H
#define KIS_IMAGE_TESTER_H

#include <QtTest>

class KisImageTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void layerTests();
    void benchmarkCreation();
    void testBlockLevelOfDetail();
    void testConvertImageColorSpace();
    void testAssignImageProfile();
    void testGlobalSelection();
    void testCloneImage();
    void testLayerComposition();

    void testFlattenLayer();
    void testMergeDown();
    void testMergeDownDestinationInheritsAlpha();
    void testMergeDownDestinationCustomCompositeOp();
    void testMergeDownDestinationSameCompositeOpLayerStyle();
    void testMergeDownDestinationSameCompositeOp();
    void testMergeDownMultipleFrames();

    void testMergeMultiple();
    void testMergeCrossColorSpace();

    void testMergeSelectionMasks();

    void testFlattenImage();

    void testFlattenPassThroughLayer();
    void testMergeTwoPassThroughLayers();

    void testMergePaintOverPassThroughLayer();
    void testMergePassThroughOverPaintLayer();

    void testPaintOverlayMask();
};

#endif

