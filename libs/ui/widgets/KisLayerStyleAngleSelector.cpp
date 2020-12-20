/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisLayerStyleAngleSelector.h"

#include <QWidget>
#include <QDial>

#include <kis_signals_blocker.h>

KisLayerStyleAngleSelector::KisLayerStyleAngleSelector(QWidget *parent)
    : QWidget(parent)
    , m_enableGlobalLight(false)
{
    ui = new Ui_WdgKisLayerStyleAngleSelector();
    ui->setupUi(this);

    ui->chkUseGlobalLight->hide();

    connect(ui->dialAngle, SIGNAL(valueChanged(int)), SLOT(slotDialAngleChanged(int)));
    connect(ui->intAngle, SIGNAL(valueChanged(int)), SLOT(slotIntAngleChanged(int)));
}

int KisLayerStyleAngleSelector::value()
{
    return ui->intAngle->value();
}

void KisLayerStyleAngleSelector::setValue(int value)
{
    KisSignalsBlocker intB(ui->intAngle);
    KisSignalsBlocker dialB(ui->dialAngle);

    ui->intAngle->setValue(value);
    ui->dialAngle->setValue(value + m_dialValueShift);
}

void KisLayerStyleAngleSelector::enableGlobalLight(bool enable)
{
    m_enableGlobalLight = enable;

    if (enable) {
        ui->chkUseGlobalLight->show();
        connect(ui->chkUseGlobalLight, SIGNAL(toggled(bool)), SLOT(slotGlobalLightToggled()));
    } else {
        ui->chkUseGlobalLight->hide();
        disconnect(ui->chkUseGlobalLight, SIGNAL(toggled(bool)), this, SLOT(slotGlobalLightToggled()));
    }
}

bool KisLayerStyleAngleSelector::useGlobalLight()
{
    return m_enableGlobalLight && ui->chkUseGlobalLight->isChecked();
}

void KisLayerStyleAngleSelector::setUseGlobalLight(bool state)
{
    ui->chkUseGlobalLight->setChecked(state);
}

void KisLayerStyleAngleSelector::slotDialAngleChanged(int value)
{
    KisSignalsBlocker b(ui->intAngle);

    int normalizedValue = 0;
    if (value >= 270 && value <= 360) {
        // Due to the mismatch between the domain of the dial (0°,360°)
        // and the spinbox (-179°,180°), the shift in the third quadrant
        // of the dial is different
        normalizedValue = value - 360 - m_dialValueShift;
    } else {
        normalizedValue = value - m_dialValueShift;
    }

    ui->intAngle->setValue(normalizedValue);
    emit valueChanged(normalizedValue);
    emitChangeSignals();
}

void KisLayerStyleAngleSelector::slotIntAngleChanged(int value)
{
    KisSignalsBlocker b(ui->dialAngle);

    int angleDialValue = value + m_dialValueShift;
    ui->dialAngle->setValue(angleDialValue);

    emit valueChanged(value);
    emitChangeSignals();
}

void KisLayerStyleAngleSelector::slotGlobalLightToggled()
{
    emitChangeSignals();
}

void KisLayerStyleAngleSelector::emitChangeSignals()
{
    if (useGlobalLight()) {
        emit globalAngleChanged(value());
    }

    emit configChanged();
}
