/*
 *  Copyright (c) 2010 Lukáš Tvrdý lukast.dev@gmail.com
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
#include <QTest>

#include "kis_projection_benchmark.h"
#include "kis_benchmark_values.h"

#include <KoColor.h>

#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <KisPart.h>

void KisProjectionBenchmark::initTestCase()
{

}

void KisProjectionBenchmark::cleanupTestCase()
{
}


void KisProjectionBenchmark::benchmarkProjection()
{
    QBENCHMARK{
        KisDocument *doc = KisPart::instance()->createDocument();
        doc->loadNativeFormat(QString(FILES_DATA_DIR) + '/' + "load_test.kra");
        doc->image()->refreshGraph();
        doc->exportDocumentSync(QUrl::fromLocalFile(QString(FILES_OUTPUT_DIR) + '/' + "save_test.kra"), doc->mimeType());
        delete doc;
    }
}

void KisProjectionBenchmark::benchmarkLoading()
{
    QBENCHMARK{
        KisDocument *doc2 = KisPart::instance()->createDocument();
        doc2->loadNativeFormat(QString(FILES_DATA_DIR) + '/' + "load_test.kra");
        delete doc2;
    }
}


QTEST_MAIN(KisProjectionBenchmark)
