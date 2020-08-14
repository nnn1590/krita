/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
