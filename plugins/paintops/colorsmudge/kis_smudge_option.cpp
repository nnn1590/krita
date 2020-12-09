/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_smudge_option.h"

#include <klocalizedstring.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>

KisSmudgeOption::KisSmudgeOption():
    KisRateOption("SmudgeRate", KisPaintOpOption::GENERAL, true),
    m_mode(SMEARING_MODE)
{
    setValueRange(0.01, 1.0);
}

void KisSmudgeOption::apply(KisPainter& painter, const KisPaintInformation& info, qreal scaleMin, qreal scaleMax, qreal multiplicator) const
{
    if (!isChecked()) {
        painter.setOpacity((quint8)(scaleMax * 255.0));
        return;
    }

    qreal value = computeSizeLikeValue(info);

    qreal  rate    = scaleMin + (scaleMax - scaleMin) * multiplicator * value; // scale m_rate into the range scaleMin - scaleMax
    quint8 opacity = qBound(OPACITY_TRANSPARENT_U8, (quint8)(rate * 255.0), OPACITY_OPAQUE_U8);

    painter.setOpacity(opacity);
}

void KisSmudgeOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisRateOption::writeOptionSetting(setting);
    setting->setProperty(name() + "Mode", m_mode);
    setting->setProperty(name() + "SmearAlpha", m_smearAlpha);
}

void KisSmudgeOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisRateOption::readOptionSetting(setting);

    m_mode = (Mode)setting->getInt(name() + "Mode", SMEARING_MODE);
    m_smearAlpha = setting->getBool(name() + "SmearAlpha", true);
}

bool KisSmudgeOption::getSmearAlpha() const
{
    return m_smearAlpha;
}

void KisSmudgeOption::setSmearAlpha(bool smearAlpha)
{
    m_smearAlpha = smearAlpha;
}
