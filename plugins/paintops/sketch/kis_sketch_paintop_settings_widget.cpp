/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_sketch_paintop_settings_widget.h"

#include "kis_sketchop_option.h"
#include "kis_sketch_paintop_settings.h"

#include <kis_curve_option_widget.h>
#include <kis_pressure_opacity_option.h>
#include <kis_paintop_settings_widget.h>
#include <kis_paint_action_type_option.h>
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_compositeop_option.h>

#include <QDomDocument>
#include <QDomElement>
#include <kis_pressure_rotation_option.h>
#include "kis_density_option.h"
#include "kis_linewidth_option.h"
#include "kis_offset_scale_option.h"

KisSketchPaintOpSettingsWidget::KisSketchPaintOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    m_sketchOption =  new KisSketchOpOption();

    addPaintOpOption(m_sketchOption, i18n("Brush size"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")), i18n("Opacity"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")), i18n("Size"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")), i18n("Rotation"));
    addPaintOpOption(new KisCurveOptionWidget(new KisLineWidthOption()  , i18n("0%"), i18n("100%")), i18n("Line width"));
    addPaintOpOption(new KisCurveOptionWidget(new KisOffsetScaleOption(), i18n("0%"), i18n("100%")), i18n("Offset scale"));
    addPaintOpOption(new KisCurveOptionWidget(new KisDensityOption(), i18n("0%"), i18n("100%")), i18n("Density"));
    addPaintOpOption(new KisAirbrushOptionWidget(false, false), i18n("Airbrush"));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRateOption(), i18n("0%"), i18n("100%")), i18n("Rate"));

    m_paintActionType = new KisPaintActionTypeOption();
    KisPropertiesConfigurationSP defaultSetting = new KisPropertiesConfiguration();
    defaultSetting->setProperty("PaintOpAction", BUILDUP);
    m_paintActionType->readOptionSetting(defaultSetting);

    addPaintOpOption(m_paintActionType, i18n("Painting Mode"));

    KisPropertiesConfigurationSP reconfigurationCourier = configuration();
    QDomDocument xMLAnalyzer;
    xMLAnalyzer.setContent(reconfigurationCourier->getString("brush_definition"));

    QDomElement firstTag = xMLAnalyzer.documentElement();
    QDomElement firstTagsChild = firstTag.elementsByTagName("MaskGenerator").item(0).toElement();

    firstTagsChild.attributeNode("diameter").setValue("128");

    reconfigurationCourier->setProperty("brush_definition", xMLAnalyzer.toString());
    setConfiguration(reconfigurationCourier);
}

KisSketchPaintOpSettingsWidget::~ KisSketchPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisSketchPaintOpSettingsWidget::configuration() const
{
    KisSketchPaintOpSettingsSP config = new KisSketchPaintOpSettings(resourcesInterface());
    config->setOptionsWidget(const_cast<KisSketchPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "sketchbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

