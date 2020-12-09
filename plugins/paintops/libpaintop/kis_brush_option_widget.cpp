/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_brush_option_widget.h"
#include <klocalizedstring.h>

#include <kis_image.h>

#include "kis_brush_selection_widget.h"
#include "kis_brush.h"

KisBrushOptionWidget::KisBrushOptionWidget()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, true)
{
    m_checkable = false;
    m_brushSelectionWidget = new KisBrushSelectionWidget();
    connect(m_brushSelectionWidget, SIGNAL(sigPrecisionChanged()), SLOT(emitSettingChanged()));
    connect(m_brushSelectionWidget, SIGNAL(sigBrushChanged()), SLOT(brushChanged()));
    m_brushSelectionWidget->hide();
    setConfigurationPage(m_brushSelectionWidget);
    m_brushOption.setBrush(brush());

    setObjectName("KisBrushOptionWidget");
}

KisBrushSP KisBrushOptionWidget::brush() const
{
    return m_brushSelectionWidget->brush();
}

void KisBrushOptionWidget::setAutoBrush(bool on)
{
    m_brushSelectionWidget->setAutoBrush(on);
}

void KisBrushOptionWidget::setPredefinedBrushes(bool on)
{
    m_brushSelectionWidget->setPredefinedBrushes(on);
}

void KisBrushOptionWidget::setCustomBrush(bool on)
{
    m_brushSelectionWidget->setCustomBrush(on);
}

void KisBrushOptionWidget::setTextBrush(bool on)
{
    m_brushSelectionWidget->setTextBrush(on);
}

void KisBrushOptionWidget::setImage(KisImageWSP image)
{
    m_brushSelectionWidget->setImage(image);
}

void KisBrushOptionWidget::setPrecisionEnabled(bool value)
{
    m_brushSelectionWidget->setPrecisionEnabled(value);
}

void KisBrushOptionWidget::setHSLBrushTipEnabled(bool value)
{
    m_brushSelectionWidget->setHSLBrushTipEnabled(value);
}

void KisBrushOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    m_brushSelectionWidget->writeOptionSetting(settings);
    m_brushOption.writeOptionSetting(settings);
}

void KisBrushOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_brushSelectionWidget->readOptionSetting(setting);
    m_brushOption.readOptionSetting(setting, resourcesInterface(), canvasResourcesInterface());
    m_brushSelectionWidget->setCurrentBrush(m_brushOption.brush());
}

void KisBrushOptionWidget::lodLimitations(KisPaintopLodLimitations *l) const
{
    KisBrushSP brush = this->brush();
    brush->lodLimitations(l);
}

void KisBrushOptionWidget::brushChanged()
{
    m_brushOption.setBrush(brush());
    emitSettingChanged();
}

bool KisBrushOptionWidget::presetIsValid()
{
    return m_brushSelectionWidget->presetIsValid();
}

void KisBrushOptionWidget::hideOptions(const QStringList &options)
{
    m_brushSelectionWidget->hideOptions(options);
}

#include "moc_kis_brush_option_widget.cpp"
