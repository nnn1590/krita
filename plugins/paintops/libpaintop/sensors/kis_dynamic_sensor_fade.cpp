/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_dynamic_sensor_fade.h"

#include <kis_debug.h>

#include <QDomElement>
#include "ui_SensorFadeConfiguration.h"

#include <brushengine/kis_paint_information.h>

static const int DEFAULT_LENGTH = 1000;

KisDynamicSensorFade::KisDynamicSensorFade()
    : KisDynamicSensor(FADE)
    , m_periodic(false)
{
    setLength(DEFAULT_LENGTH);
}

qreal KisDynamicSensorFade::value(const KisPaintInformation&  pi)
{
    if (pi.isHoveringMode()) return 1.0;

    const int currentValue =
        m_periodic ?
        pi.currentDabSeqNo() % m_length :
        qMin(pi.currentDabSeqNo(), m_length);

    return qreal(currentValue) / m_length;
}

void KisDynamicSensorFade::setPeriodic(bool periodic)
{
    m_periodic = periodic;
}

void KisDynamicSensorFade::setLength(int length)
{
    m_length = length;
}

QWidget* KisDynamicSensorFade::createConfigurationWidget(QWidget* parent, QWidget* ss)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorFadeConfiguration stc;
    stc.setupUi(wdg);
    stc.checkBoxRepeat->setChecked(m_periodic);

    stc.spinBoxLength->setSuffix(i18n(" px"));
    stc.spinBoxLength->setExponentRatio(3.0);

    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), SLOT(setPeriodic(bool)));
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), ss, SIGNAL(parametersChanged()));
    stc.spinBoxLength->setValue(m_length);
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), SLOT(setLength(int)));
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));
    return wdg;
}

void KisDynamicSensorFade::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("periodic", m_periodic);
    e.setAttribute("length", m_length);
}

void KisDynamicSensorFade::fromXML(const QDomElement& e)
{
    KisDynamicSensor::fromXML(e);
    m_periodic = e.attribute("periodic", "0").toInt();
    m_length = e.attribute("length", QString::number(DEFAULT_LENGTH)).toInt();
}

