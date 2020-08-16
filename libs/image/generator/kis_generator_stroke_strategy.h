/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include <QObject>
#include <QSharedPointer>
#include <kis_generator.h>
#include <kis_generator_layer.h>
#include <KisRunnableBasedStrokeStrategy.h>

class KisGeneratorStrokeStrategy: public QObject, public KisRunnableBasedStrokeStrategy
{
    Q_OBJECT
public:
    KisGeneratorStrokeStrategy();
    ~KisGeneratorStrokeStrategy() override;

    static QVector<KisStrokeJobData *> createJobsData(const KisGeneratorLayerSP layer, QSharedPointer<bool> cookie, const KisGeneratorSP f, const KisPaintDeviceSP dev, const QRegion &rc, const KisFilterConfigurationSP filterConfig);
};
