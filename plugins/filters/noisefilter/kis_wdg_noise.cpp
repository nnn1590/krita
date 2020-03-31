/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_wdg_noise.h"


#include <QLayout>

#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdgnoiseoptions.h"

KisWdgNoise::KisWdgNoise(KisFilter* /*nfilter*/, QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgNoiseOptions();
    m_widget->setupUi(this);

    connect(widget()->intLevel, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intOpacity, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    m_seedThreshold = rand();
    m_seedRed = rand();
    m_seedGreen = rand();
    m_seedBlue = rand();
}

KisWdgNoise::~KisWdgNoise()
{
    delete m_widget;
}

void KisWdgNoise::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("level", value)) {
        widget()->intLevel->setValue(value.toUInt());
    }
    if (config->getProperty("opacity", value)) {
        widget()->intOpacity->setValue(value.toUInt());
    }
}

KisPropertiesConfigurationSP KisWdgNoise::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("noise", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("level", this->widget()->intLevel->value());
    config->setProperty("opacity", this->widget()->intOpacity->value());
    config->setProperty("seedThreshold", m_seedThreshold);
    config->setProperty("seedRed", m_seedRed);
    config->setProperty("seedGreen", m_seedGreen);
    config->setProperty("seedBlue", m_seedBlue);
    return config;
}


