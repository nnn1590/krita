/*
 *  Copyright (c) 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
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

#include "kis_heightmap_test.h"

#include <QTest>
#include <QCoreApplication>

#include  <sdk/tests/testui.h>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif


const QString HeightmapMimetype = "image/x-r8";


void KisHeightmapTest::testFiles()
{
    TestUtil::testFiles(QString(FILES_DATA_DIR) + "/sources", QStringList(), QString(), 1);
}



void KisHeightmapTest::testImportFromWriteonly()
{
    TestUtil::testImportFromWriteonly(QString(FILES_DATA_DIR), HeightmapMimetype);
}


void KisHeightmapTest::testExportToReadonly()
{
    TestUtil::testExportToReadonly(QString(FILES_DATA_DIR), HeightmapMimetype);
}


void KisHeightmapTest::testImportIncorrectFormat()
{
    TestUtil::testImportIncorrectFormat(QString(FILES_DATA_DIR), HeightmapMimetype);
}


KISTEST_MAIN(KisHeightmapTest)

