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

#include "kis_multinode_property_test.h"

#include <QTest>
#include <testutil.h>

#include <KoCompositeOpRegistry.h>

#include "kis_multinode_property.h"


void KisMultinodePropertyTest::test()
{
    TestUtil::MaskParent p;

    KisPaintLayerSP layer1 = p.layer;
    KisPaintLayerSP layer2 = new KisPaintLayer(p.image, "paint2", OPACITY_OPAQUE_U8);
    KisPaintLayerSP layer3 = new KisPaintLayer(p.image, "paint3", OPACITY_OPAQUE_U8);

    KisNodeList nodes;
    nodes << layer1;
    nodes << layer2;
    nodes << layer3;

    // Test uniform initial state
    {
        QScopedPointer<QCheckBox> box(new QCheckBox("test ignore"));
        KisMultinodeCompositeOpProperty prop(nodes);

        prop.connectIgnoreCheckBox(box.data());

        QCOMPARE(prop.isIgnored(), false);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), true);

        prop.setValue(COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);

        prop.setIgnored(true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), false);

        prop.setIgnored(false);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), false);
        QCOMPARE(box->isChecked(), true);
    }


    // Test non-uniform initial state
    layer1->setCompositeOpId(COMPOSITE_ALPHA_DARKEN);
    layer2->setCompositeOpId(COMPOSITE_OVER);
    layer3->setCompositeOpId(COMPOSITE_OVER);

    {
        QScopedPointer<QCheckBox> box(new QCheckBox("test ignore"));
        KisMultinodeCompositeOpProperty prop(nodes);

        prop.connectIgnoreCheckBox(box.data());

        QCOMPARE(prop.isIgnored(), true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), false);

        prop.setIgnored(false);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), true);

        prop.setValue(COMPOSITE_OVER);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_OVER);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), true);

        prop.setIgnored(true);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(prop.value(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(box->isEnabled(), true);
        QCOMPARE(box->isChecked(), false);
    }

    // Test undo-redo
    {
        QScopedPointer<QCheckBox> box(new QCheckBox("test ignore"));
        KisMultinodeCompositeOpProperty prop(nodes);

        prop.connectIgnoreCheckBox(box.data());

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);

        prop.setIgnored(false);

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);

        QScopedPointer<KUndo2Command> cmd(prop.createPostExecutionUndoCommand());

        cmd->undo();

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_OVER);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_OVER);

        cmd->redo();

        QCOMPARE(layer1->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer2->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
        QCOMPARE(layer3->compositeOpId(), COMPOSITE_ALPHA_DARKEN);
    }
}

QTEST_MAIN(KisMultinodePropertyTest)
