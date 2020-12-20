/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QWidget>
#include "KisGamutMaskToolbar.h"
#include <kis_icon_utils.h>
#include <kis_canvas_resource_provider.h>
#include <kis_signals_blocker.h>

KisGamutMaskToolbar::KisGamutMaskToolbar(QWidget* parent) : QWidget(parent)
  , m_selectedMask(nullptr)
  , m_selfUpdate(false)
{
    m_ui.reset(new Ui_wdgGamutMaskToolbar());
    m_ui->setupUi(this);

    m_iconMaskOff = KisIconUtils::loadIcon("gamut-mask-off");
    m_iconMaskOn = KisIconUtils::loadIcon("gamut-mask-on");

    m_textNoMask = i18n("Select a mask in \"Gamut Masks\" docker");
    m_textMaskDisabled = i18n("Mask is disabled");

    m_ui->bnToggleMask->setChecked(false);
    m_ui->bnToggleMask->setIcon(m_iconMaskOff);

    m_ui->rotationSlider->setRange(0, 360);
    m_ui->rotationSlider->setPrefix(i18n("Rotation: "));
    m_ui->rotationSlider->setSuffix("°");
    m_ui->rotationSlider->setFastSliderStep(30); // TODO: test for usability
    m_ui->rotationSlider->hide();

    // gamut mask connections
    connect(m_ui->bnToggleMask, SIGNAL(toggled(bool)), SLOT(slotGamutMaskToggle(bool)));
    connect(m_ui->rotationSlider, SIGNAL(valueChanged(int)), SLOT(slotGamutMaskRotate(int)));
}

void KisGamutMaskToolbar::connectMaskSignals(KisCanvasResourceProvider* resourceProvider)
{
    connect(resourceProvider, SIGNAL(sigGamutMaskChanged(KoGamutMaskSP)),
            this, SLOT(slotGamutMaskSet(KoGamutMaskSP)), Qt::UniqueConnection);

    connect(resourceProvider, SIGNAL(sigGamutMaskUnset()),
            this, SLOT(slotGamutMaskUnset()), Qt::UniqueConnection);

    connect(this, SIGNAL(sigGamutMaskChanged(KoGamutMaskSP)),
            resourceProvider, SLOT(slotGamutMaskActivated(KoGamutMaskSP)), Qt::UniqueConnection);

    connect(this, SIGNAL(sigGamutMaskDeactivated()),
            resourceProvider, SLOT(slotGamutMaskDeactivate()), Qt::UniqueConnection);

    connect(resourceProvider, SIGNAL(sigGamutMaskDeactivated()),
            this, SLOT(slotGamutMaskDeactivate()), Qt::UniqueConnection);

}

void KisGamutMaskToolbar::slotGamutMaskSet(KoGamutMaskSP mask)
{
    if (!mask) {
        return;
    }

    if (m_selfUpdate) {
        return;
    }

    m_selectedMask = mask;

    if (m_selectedMask) {
        slotGamutMaskToggle(true);
    } else {
        slotGamutMaskToggle(false);
    }
}

void KisGamutMaskToolbar::slotGamutMaskUnset()
{
    m_ui->rotationSlider->hide();
    m_ui->labelMaskName->show();
    m_ui->labelMaskName->setText(m_textNoMask);
}

void KisGamutMaskToolbar::slotGamutMaskDeactivate()
{
    if (m_selfUpdate) {
        return;
    }

    slotGamutMaskToggle(false);
}

void KisGamutMaskToolbar::slotGamutMaskToggle(bool state)
{
    bool b = (!m_selectedMask) ? false : state;

    m_ui->bnToggleMask->setChecked(b);

    if (b == true) {
        m_ui->bnToggleMask->setIcon(m_iconMaskOn);
        m_ui->labelMaskName->hide();
        m_ui->rotationSlider->show();

        m_ui->rotationSlider->blockSignals(true);
        m_ui->rotationSlider->setValue(m_selectedMask->rotation());
        m_ui->rotationSlider->blockSignals(false);

        m_selfUpdate = true;
        emit sigGamutMaskChanged(m_selectedMask);
        m_selfUpdate = false;

    } else {
        m_ui->bnToggleMask->setIcon(m_iconMaskOff);
        m_ui->rotationSlider->hide();
        m_ui->labelMaskName->show();
        m_ui->labelMaskName->setText(m_textMaskDisabled);

        m_selfUpdate = true;
        emit sigGamutMaskDeactivated();
        m_selfUpdate = false;
    }
}

void KisGamutMaskToolbar::slotGamutMaskRotate(int angle)
{
    if (!m_selectedMask) {
        return;
    }

    m_selectedMask->setRotation(angle);
    emit sigGamutMaskChanged(m_selectedMask);
}
