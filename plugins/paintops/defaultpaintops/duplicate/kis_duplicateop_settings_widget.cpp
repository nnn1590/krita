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

#include "kis_duplicateop_settings_widget.h"
#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_option.h"

#include <kis_image.h>
#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_curve_option_widget.h>
#include <kis_compositeop_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"
#include <brushengine/kis_paintop_lod_limitations.h>

KisDuplicateOpSettingsWidget::KisDuplicateOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);

    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")), i18n("Rotation"));
    addPaintOpOption(new KisPressureMirrorOptionWidget(), i18n("Mirror"));
    addPaintOpOption(new KisDuplicateOpOption(), i18n("Painting Mode"));
    addPaintOpOption(new KisTextureOption(), i18n("Pattern"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")), i18n("Strength"));

}

KisDuplicateOpSettingsWidget::~KisDuplicateOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisDuplicateOpSettingsWidget::configuration() const
{
    KisDuplicateOpSettings *config = new KisDuplicateOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisDuplicateOpSettingsWidget*>(this));
    config->setProperty("paintop", "duplicate"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

KisPaintopLodLimitations KisDuplicateOpSettingsWidget::lodLimitations() const
{
    KisPaintopLodLimitations l = KisBrushBasedPaintopOptionWidget::lodLimitations();
    l.blockers << KoID("clone-brush", i18nc("PaintOp instant preview limitation", "Clone Brush (temporarily disabled)"));
    return l;
}
