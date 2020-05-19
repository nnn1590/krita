/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_filter_mask_test.h"
#include <QTest>

#include <KoColorSpaceRegistry.h>

#include "kis_selection.h"
#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "kis_filter_mask.h"
#include "filter/kis_filter_registry.h"
#include "kis_group_layer.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_types.h"
#include "kis_image.h"
#include <KisGlobalResourcesInterface.h>


#include "testutil.h"

#define IMAGE_WIDTH 1000
#define IMAGE_HEIGHT 1000

void KisFilterMaskTest::testCreation()
{
    KisFilterMaskSP mask = new KisFilterMask();
}

#define initImage(image, layer, device, mask) do {                      \
    image = new KisImage(0, IMAGE_WIDTH, IMAGE_HEIGHT, 0, "tests");     \
    layer = new KisPaintLayer(image, 0, 100, device);         \
    image->addNode(layer);                                              \
    image->addNode(mask, layer);                                        \
    } while(0)

void KisFilterMaskTest::testProjectionNotSelected()
{
    KisImageSP image;
    KisPaintLayerSP layer;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    KisFilterMaskSP mask = new KisFilterMask();
    mask->setFilter(kfc->cloneWithResourcesSnapshot());

    // Check basic apply(). Shouldn't do anything, since nothing is selected yet.
    KisPaintDeviceSP projection = new KisPaintDevice(cs);
    initImage(image, layer, projection, mask);

    projection->convertFromQImage(qimage, 0, 0, 0);
    mask->initSelection(layer);
    mask->createNodeProgressProxy();
    mask->select(qimage.rect(), MIN_SELECTED);

    mask->apply(projection, qimage.rect(), qimage.rect(), KisNode::N_FILTHY);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, qimage, projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtermasktest1.png");
        QFAIL(QString("Failed to create identical image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisFilterMaskTest::testProjectionSelected()
{
    KisImageSP image;
    KisPaintLayerSP layer;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    KisFilterMaskSP mask = new KisFilterMask();
    mask->setFilter(kfc->cloneWithResourcesSnapshot());
    mask->createNodeProgressProxy();

    KisPaintDeviceSP projection = new KisPaintDevice(cs);
    initImage(image, layer, projection, mask);
    projection->convertFromQImage(qimage, 0, 0, 0);

    mask->initSelection(layer);
    mask->select(qimage.rect(), MAX_SELECTED);
    mask->apply(projection, qimage.rect(), qimage.rect(), KisNode::N_FILTHY);
    QCOMPARE(mask->exactBounds(), QRect(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT));

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        projection->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtermasktest2.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }

}

QTEST_MAIN(KisFilterMaskTest)
