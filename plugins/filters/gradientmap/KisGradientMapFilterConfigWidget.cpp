/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_signals_blocker.h>

#include "KisGradientMapFilterConfigWidget.h"
#include "KisGradientMapFilterConfiguration.h"


KisGradientMapFilterConfigWidget::KisGradientMapFilterConfigWidget(QWidget *parent, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
    , m_view(nullptr)
{
    m_ui.setupUi(this);

    m_gradientChangedCompressor = new KisSignalCompressor(50, KisSignalCompressor::FIRST_ACTIVE);
    
    m_ui.widgetGradientEditor->setContentsMargins(10, 10, 10, 10);
    m_ui.widgetGradientEditor->loadUISettings();
    
    connect(m_ui.widgetGradientEditor, SIGNAL(sigGradientChanged()), m_gradientChangedCompressor, SLOT(start()));
    connect(m_gradientChangedCompressor, SIGNAL(timeout()), this, SIGNAL(sigConfigurationItemChanged()));
    connect(m_ui.comboBoxColorMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);
    connect(m_ui.widgetDither, SIGNAL(sigConfigurationItemChanged()), this, SIGNAL(sigConfigurationItemChanged()));
}

KisGradientMapFilterConfigWidget::~KisGradientMapFilterConfigWidget()
{
    m_ui.widgetGradientEditor->saveUISettings();
}

KisPropertiesConfigurationSP KisGradientMapFilterConfigWidget::configuration() const
{
    KisGradientMapFilterConfiguration *config = new KisGradientMapFilterConfiguration(KisGlobalResourcesInterface::instance());
    
    KoAbstractGradientSP gradient = m_ui.widgetGradientEditor->gradient();
    if (gradient && m_view) {
        KoCanvasResourcesInterfaceSP canvasResourcesInterface =
            m_view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface();
        gradient->bakeVariableColors(canvasResourcesInterface);
    }
    config->setGradient(gradient);

    config->setColorMode(m_ui.comboBoxColorMode->currentIndex());
    m_ui.widgetDither->configuration(*config, "dither/");

    return config;
}

void KisGradientMapFilterConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisGradientMapFilterConfiguration *filterConfig =
        dynamic_cast<const KisGradientMapFilterConfiguration*>(config.data());
    Q_ASSERT(filterConfig);

    {
        KisSignalsBlocker signalsBlocker(this);

        m_ui.widgetGradientEditor->setGradient(filterConfig->gradient());
        m_ui.comboBoxColorMode->setCurrentIndex(filterConfig->colorMode());
        m_ui.widgetDither->setConfiguration(*filterConfig, "dither/");
    }

    emit sigConfigurationUpdated(); 
}

void KisGradientMapFilterConfigWidget::setView(KisViewManager *view)
{
    m_view = view;
    if (view) {
        KoCanvasResourcesInterfaceSP canvasResourcesInterface = m_view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface();
        m_ui.widgetGradientEditor->setCanvasResourcesInterface(canvasResourcesInterface);
    } else {
        m_ui.widgetGradientEditor->setCanvasResourcesInterface(nullptr);
    }
}
