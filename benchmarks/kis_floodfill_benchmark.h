/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_FLOODFILL_BENCHMARK_H
#define KIS_FLOODFILL_BENCHMARK_H

#include <KoColor.h>

#include <kis_types.h>
#include <QtTest>
#include <kis_paint_device.h>

class KoColor;

class KisFloodFillBenchmark : public QObject
{
    Q_OBJECT
private:
    const KoColorSpace * m_colorSpace;
    KoColor m_color;
    KisPaintDeviceSP m_deviceStandardFloodFill;
    KisPaintDeviceSP m_deviceWithSelectionAsBoundary;
    KisPaintDeviceSP m_deviceWithoutSelectionAsBoundary;
    KisPaintDeviceSP m_existingSelection;
    int m_startX;
    int m_startY;
    
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    
    void benchmarkFlood();
    void benchmarkFloodWithoutSelectionAsBoundary();
    void benchmarkFloodWithSelectionAsBoundary();

    
    
    
};

#endif
