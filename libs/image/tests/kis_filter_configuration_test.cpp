/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_filter_configuration_test.h"
#include <QApplication>
#include <QTest>
#include <kis_debug.h>
#include <KoID.h>
#include <kis_paint_device.h>


#include "../filter/kis_filter_configuration.h"
#include "../filter/kis_filter_registry.h"
#include "../filter/kis_filter.h"

#include <KisGlobalResourcesInterface.h>

void KisFilterConfigurationTest::testCreation()
{
    KisFilterConfigurationSP  kfc = new KisFilterConfiguration("test", 1, KisGlobalResourcesInterface::instance());
    QVERIFY2(kfc != 0,  "Could not create test filter configuration");
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));

}

void KisFilterConfigurationTest::testRoundTrip()
{
    KisFilterConfigurationSP  kfc = new KisFilterConfiguration("test", 1, KisGlobalResourcesInterface::instance());
    QCOMPARE(kfc->version(), 1);
    QCOMPARE(kfc->name(), QString("test"));
    QString s = kfc->toXML();

    kfc = new KisFilterConfiguration("test2", 2, KisGlobalResourcesInterface::instance());
    kfc->fromXML(s);
    QCOMPARE(kfc->version(), 1);
}

void KisFilterConfigurationTest::testSetGetProperty()
{
    KisFilterConfigurationSP  kfc = new KisFilterConfiguration("test", 1, KisGlobalResourcesInterface::instance());
    kfc->setProperty("value1", 10);
    kfc->setProperty("value2", "foo");
    QCOMPARE(kfc->getInt("value1"), 10);
    QCOMPARE(kfc->getString("value2"), QString("foo"));
    QString s = kfc->toXML();

    kfc = new KisFilterConfiguration("test2", 2, KisGlobalResourcesInterface::instance());
    kfc->fromXML(s);
    QCOMPARE(kfc->getInt("value1"), 10);
    QCOMPARE(kfc->getString("value2"), QString("foo"));
}


QTEST_MAIN(KisFilterConfigurationTest)
