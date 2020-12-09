/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_duplicateop_option.h"
#include <klocalizedstring.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgduplicateop.h"
#include <kis_image.h>

class KisDuplicateOpOptionsWidget: public QWidget, public Ui::DuplicateOpOptionsWidget
{
public:
    KisDuplicateOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
    KisImageWSP m_image;
protected:
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        //cbPerspective->setEnabled(m_image && m_image->perspectiveGrid() && m_image->perspectiveGrid()->countSubGrids() == 1);
        cbPerspective->setVisible(false); // XXX: Until perspective cloning works again!
    }
};


KisDuplicateOpOption::KisDuplicateOpOption()
    : KisPaintOpOption(KisPaintOpOption::COLOR, false)
{
    setObjectName("KisDuplicateOpOption");

    m_checkable = false;
    m_optionWidget = new KisDuplicateOpOptionsWidget();

    connect(m_optionWidget->cbHealing, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_optionWidget->cbPerspective, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_optionWidget->cbSourcePoint, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_optionWidget->cbResetSourcePoint, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_optionWidget->chkCloneProjection, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_optionWidget);
}


KisDuplicateOpOption::~KisDuplicateOpOption()
{
}

bool KisDuplicateOpOption::healing() const
{
    return m_optionWidget->cbHealing->isChecked();
}

void KisDuplicateOpOption::setHealing(bool healing)
{
    m_optionWidget->cbHealing->setChecked(healing);
}

bool KisDuplicateOpOption::correctPerspective() const
{
    return m_optionWidget->cbPerspective->isChecked();
}

void KisDuplicateOpOption::setPerspective(bool perspective)
{
    m_optionWidget->cbPerspective->setChecked(perspective);
}

bool KisDuplicateOpOption::moveSourcePoint() const
{
    return m_optionWidget->cbSourcePoint->isChecked();
}

void KisDuplicateOpOption::setMoveSourcePoint(bool move)
{
    m_optionWidget->cbSourcePoint->setChecked(move);
}

bool KisDuplicateOpOption::resetSourcePoint() const
{
    return m_optionWidget->cbResetSourcePoint->isChecked();
}

void KisDuplicateOpOption::setResetSourcePoint(bool reset)
{
    m_optionWidget->cbResetSourcePoint->setChecked(reset);
}

bool KisDuplicateOpOption::cloneFromProjection() const
{
    return m_optionWidget->chkCloneProjection->isChecked();
}

void KisDuplicateOpOption::setCloneFromProjection(bool cloneFromProjection)
{
    m_optionWidget->chkCloneProjection->setChecked(cloneFromProjection);
}

void KisDuplicateOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisDuplicateOptionProperties op;

    op.duplicate_healing = healing();
    op.duplicate_correct_perspective = correctPerspective();
    op.duplicate_move_source_point = moveSourcePoint();
    op.duplicate_reset_source_point = resetSourcePoint();
    op.duplicate_clone_from_projection = cloneFromProjection();

    op.writeOptionSetting(setting);
}

void KisDuplicateOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisDuplicateOptionProperties op;
    op.readOptionSetting(setting);

    m_optionWidget->cbHealing->setChecked(op.duplicate_healing);
    m_optionWidget->cbPerspective->setChecked(op.duplicate_correct_perspective);
    m_optionWidget->cbSourcePoint->setChecked(op.duplicate_move_source_point);
    m_optionWidget->cbResetSourcePoint->setChecked(op.duplicate_reset_source_point);
    m_optionWidget->chkCloneProjection->setChecked(op.duplicate_clone_from_projection);
}

void KisDuplicateOpOption::setImage(KisImageWSP image)
{
    m_optionWidget->m_image = image;
}
