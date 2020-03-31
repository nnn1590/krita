/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_FILTEROP_H_
#define KIS_FILTEROP_H_

#include "kis_brush_based_paintop.h"
#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>

class KisFilterConfiguration;
class KisFilterOpSettings;
class KisPaintInformation;
class KisPainter;

class KisFilterOp : public KisBrushBasedPaintOp
{

public:

    KisFilterOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisFilterOp() override;

    static QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);
    static QList<KoResourceSP> prepareEmbeddedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:

    KisPaintDeviceSP m_tmpDevice;
    KisPressureSizeOption m_sizeOption;
    KisPressureRotationOption m_rotationOption;
    KisFilterSP m_filter;
    KisFilterConfigurationSP m_filterConfiguration;
    bool m_smudgeMode;
};

#endif // KIS_FILTEROP_H_
