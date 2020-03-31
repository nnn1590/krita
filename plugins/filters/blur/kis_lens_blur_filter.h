/*
 * This file is part of Krita
 *
 * Copyright (c) 2010 Edward Apap <schumifer@hotmail.com>
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


#ifndef KIS_LENS_BLUR_FILTER_H
#define KIS_LENS_BLUR_FILTER_H

#include "filter/kis_filter.h"
#include "ui_wdg_lens_blur.h"

#include <Eigen/Core>

class KisLensBlurFilter : public KisFilter
{
public:
    KisLensBlurFilter();
public:

    void processImpl(KisPaintDeviceSP src,
                     const QRect& size,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater
                     ) const override;
    static inline KoID id() {
        return KoID("lens blur", i18n("Lens Blur"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;

    static QSize getKernelHalfSize(const KisFilterConfigurationSP config, int lod);

    QRect neededRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;
    QRect changedRect(const QRect & rect, const KisFilterConfigurationSP _config, int lod) const override;

public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool useForMasks) const override;

private:
    static QPolygonF getIrisPolygon(const KisFilterConfigurationSP config, int lod);
};

#endif
