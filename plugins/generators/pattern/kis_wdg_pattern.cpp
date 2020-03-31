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

#include "kis_wdg_pattern.h"

#include <QLayout>
#include <QLabel>

#include <KoColor.h>
#include <resources/KoPattern.h>
#include <KisGlobalResourcesInterface.h>
#include <kis_generator_registry.h>

#include <filter/kis_filter_configuration.h>
#include <kis_pattern_chooser.h>
#include "ui_wdgpatternoptions.h"

KisWdgPattern::KisWdgPattern(QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgPatternOptions();
    m_widget->setupUi(this);
    m_widget->lblPattern->setVisible(false);
    m_widget->lblColor->setVisible(false);
    m_widget->bnColor->setVisible(false);
    connect(m_widget->patternChooser, SIGNAL(resourceSelected(KoResourceSP)), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgPattern::~KisWdgPattern()
{
    delete m_widget;
}


void KisWdgPattern::setConfiguration(const KisPropertiesConfigurationSP config)
{
    auto source = KisGlobalResourcesInterface::instance()->source<KoPattern>(ResourceType::Patterns);
    KoPatternSP pattern = source.resourceForName(config->getString("pattern", "Grid01.pat"));
    widget()->patternChooser->setCurrentPattern(pattern ? pattern : source.fallbackResource());
}

KisPropertiesConfigurationSP KisWdgPattern::configuration() const
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("pattern");
    KisFilterConfigurationSP config = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());

    QVariant v;
    v.setValue(widget()->patternChooser->currentResource()->name());
    config->setProperty("pattern", v);

    return config;
}

