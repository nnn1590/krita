/*
 * This file is part of Krita
 *
 * Copyright (c) 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_motion_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdg_motion_blur.h"

KisWdgMotionBlur::KisWdgMotionBlur(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgMotionBlur();
    m_widget->setupUi(this);

    connect(m_widget->blurAngleSlider, SIGNAL(valueChanged(int)), SLOT(angleSliderChanged(int)));
    connect(m_widget->blurAngleDial, SIGNAL(valueChanged(int)), SLOT(angleDialChanged(int)));

    connect(m_widget->blurAngleSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->blurLength, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgMotionBlur::~KisWdgMotionBlur()
{
    delete m_widget;
}

KisPropertiesConfigurationSP KisWdgMotionBlur::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("motion blur", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("blurAngle", m_widget->blurAngleSlider->value());
    config->setProperty("blurLength", m_widget->blurLength->value());
    return config;
}

void KisWdgMotionBlur::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("blurAngle", value)) {
        m_widget->blurAngleSlider->setValue(value.toInt());
    }
    if (config->getProperty("blurLength", value)) {
        m_widget->blurLength->setValue(value.toInt());
    }
}

void KisWdgMotionBlur::angleSliderChanged(int v)
{
    int absoluteValue = -v + 270;
    if (absoluteValue < 0) {
        absoluteValue += 360;
    }
    else if (absoluteValue > 360) {
        absoluteValue = absoluteValue - 360;
    }

    m_widget->blurAngleDial->setValue(absoluteValue);
}

void KisWdgMotionBlur::angleDialChanged(int v)
{
    int absoluteValue = v - 270;
    if (absoluteValue < 0) {
        absoluteValue = 360 + absoluteValue;
    }
    absoluteValue = -absoluteValue;
    if (absoluteValue < 0) {
        absoluteValue += 360;
    }

    m_widget->blurAngleSlider->setValue(absoluteValue);
}

