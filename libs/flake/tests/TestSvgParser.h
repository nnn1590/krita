/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TESTSVGPARSER_H
#define TESTSVGPARSER_H

#include <QtTest>

class TestSvgParser : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testUnitPx();
    void testUnitPxResolution();
    void testUnitPt();
    void testUnitIn();
    void testUnitPercentInitial();
    void testScalingViewport();
    void testScalingViewportKeepMeet1();
    void testScalingViewportKeepMeet2();
    void testScalingViewportKeepMeetAlign();
    void testScalingViewportKeepSlice1();
    void testScalingViewportKeepSlice2();
    void testScalingViewportResolution();
    void testScalingViewportPercentInternal();
    void testParsePreserveAspectRatio();
    void testParseTransform();

    void testScalingViewportTransform();
    void testTransformNesting();
    void testTransformNestingGroups();
    void testTransformRotation1();
    void testTransformRotation2();

    void testRenderStrokeNone();
    void testRenderStrokeColorName();
    void testRenderStrokeColorHex3();
    void testRenderStrokeColorHex6();
    void testRenderStrokeColorRgbValues();
    void testRenderStrokeColorRgbPercent();
    void testRenderStrokeColorCurrent();
    void testRenderStrokeColorNonexistentIri();

    void testRenderStrokeWidth();
    void testRenderStrokeZeroWidth();
    void testRenderStrokeOpacity();

    void testRenderStrokeJointRound();
    void testRenderStrokeLinecap();
    void testRenderStrokeMiterLimit();

    void testRenderStrokeDashArrayEven();
    void testRenderStrokeDashArrayEvenOffset();
    void testRenderStrokeDashArrayOdd();
    void testRenderStrokeDashArrayRelative();


    void testRenderFillDefault();
    void testRenderFillRuleNonZero();
    void testRenderFillRuleEvenOdd();
    void testRenderFillOpacity();

    void testRenderDisplayAttribute();
    void testRenderVisibilityAttribute();

    void testRenderVisibilityInheritance();
    void testRenderDisplayInheritance();

    void testRenderStrokeWithInlineStyle();

    void testIccColor();
    void testRenderFillLinearGradientRelativePercent();
    void testRenderFillLinearGradientRelativePortion();
    void testRenderFillLinearGradientUserCoord();
    void testRenderFillLinearGradientStopPortion();
    void testRenderFillLinearGradientTransform();
    void testRenderFillLinearGradientTransformUserCoord();
    void testRenderFillLinearGradientRotatedShape();
    void testRenderFillLinearGradientRotatedShapeUserCoord();

    void testRenderFillRadialGradient();
    void testRenderFillRadialGradientUserCoord();

    void testRenderFillLinearGradientUserCoordPercent();

    void testRenderStrokeLinearGradient();

    void testRenderMeshGradient_bilinear_1by1_UserCoord();
    void testRenderMeshGradient_bicubic_1by1_UserCoord();
    void testRenderMeshGradient_bilinear_2by2_UserCoord();
    void testRenderMeshGradient_bicubic_2by2_UserCoord();
    void testRenderMeshGradient_bilinear_1by1_Obb();
    void testRenderMeshGradient_bicubic_2by2_Obb();
    void testRenderMeshGradient_MeshTransform_UserCoord();
    void testRenderMeshGradient_ShapeTransform_UserCoord();
    void testRenderMeshGradient_transparent();
    void testRenderMeshGradient_reversed();
    void testRenderMeshGradient_MeshTransform_Obb();
    void testRenderMeshGradient_ShapeTransform_Obb();

    void testManualRenderPattern_ContentUser_RefObb();
    void testManualRenderPattern_ContentObb_RefObb();
    void testManualRenderPattern_ContentUser_RefUser();

    void testManualRenderPattern_ContentObb_RefObb_Transform_Rotate();

    void testManualRenderPattern_ContentView_RefObb();

    void testManualRenderPattern_ContentView_RefUser();

    void testRenderPattern_r_User_c_User();
    void testRenderPattern_InfiniteRecursionWhenInherited();
    void testRenderPattern_r_User_c_View();
    void testRenderPattern_r_User_c_Obb();

    void testRenderPattern_r_User_c_View_Rotated();
    void testRenderPattern_r_Obb_c_View_Rotated();

    void testKoClipPathRendering();
    void testKoClipPathRelativeRendering();

    void testRenderClipPath_User();
    void testRenderClipPath_Obb();
    void testRenderClipPath_Obb_Transform();

    void testRenderClipMask_Obb();
    void testRenderClipMaskOnGroup_Obb();
    void testRenderClipMask_User_Clip_Obb();
    void testRenderClipMask_User_Clip_User();

    void testRenderImage_AspectDefault();
    void testRenderImage_AspectNone();
    void testRenderImage_AspectMeet();

    void testRectShapeRoundUniformX();
    void testRectShapeRoundUniformY();
    void testRectShapeRoundXY();
    void testRectShapeRoundXYOverflow();

    void testCircleShape();
    void testEllipseShape();
    void testLineShape();
    void testPolylineShape();
    void testPolygonShape();

    void testPathShape();

    void testDefsHidden();
    void testDefsUseInheritance();
    void testUseWithoutDefs();

    void testMarkersAutoOrientation();
    void testMarkersAutoOrientationScaled();
    void testMarkersAutoOrientationScaledUserCoordinates();
    void testMarkersCustomOrientation();

    void testMarkersDifferent();

    void testGradientRecoveringTransform();
    void testMarkersOnClosedPath();
    void testMarkersAngularUnits();

    void testSodipodiArcShape();
    void testSodipodiArcShapeOpen();
    void testKritaChordShape();
    void testSodipodiChordShape();

    void testMarkersFillAsShape();
private:

};

#endif // TESTSVGPARSER_H
