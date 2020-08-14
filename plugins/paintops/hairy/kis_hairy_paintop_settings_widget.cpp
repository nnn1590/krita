/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_hairy_paintop_settings_widget.h"
#include "kis_hairy_paintop_settings.h"

#include "kis_hairy_ink_option.h"

#include <kis_paint_action_type_option.h>
#include "kis_hairy_bristle_option.h"
#include <kis_curve_option_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_compositeop_option.h>
#include <kis_brush_option_widget.h>

KisHairyPaintOpSettingsWidget:: KisHairyPaintOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    addPaintOpOption(new KisHairyBristleOption(), i18n("Bristle options"));
    addPaintOpOption(new KisHairyInkOption(), i18n("Ink depletion"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")), i18n("Rotation"));
    addPaintOpOption(new KisPaintActionTypeOption(), i18n("Painting Mode"));


    KisBrushOptionWidget *brushWidget = brushOptionWidget();
    QStringList hiddenOptions;
    hiddenOptions << "KisBrushChooser/lblSpacing"
                  << "KisBrushChooser/Spacing"
                  << "KisBrushChooser/ColorAsMask"
                  << "KisAutoBrushWidget/btnAntiAliasing"
                  << "KisAutoBrushWidget/grpFade"
                  << "KisAutoBrushWidget/lblDensity"
                  << "KisAutoBrushWidget/density"
                  << "KisAutoBrushWidget/lblSpacing"
                  << "KisAutoBrushWidget/spacingWidget"
                  << "KisAutoBrushWidget/lblRandomness"
                  << "KisAutoBrushWidget/inputRandomness"
                     ;
    brushWidget->hideOptions(hiddenOptions);
}

KisHairyPaintOpSettingsWidget::~ KisHairyPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisHairyPaintOpSettingsWidget::configuration() const
{
    KisHairyPaintOpSettings* config = new KisHairyPaintOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisHairyPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "hairybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
