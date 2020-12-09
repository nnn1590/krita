/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_hatching_options.h"
#include <brushengine/kis_paintop_lod_limitations.h>

#include "ui_wdghatchingoptions.h"

class KisHatchingOptionsWidget: public QWidget, public Ui::WdgHatchingOptions
{
public:
    KisHatchingOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        QString degree = QChar(Qt::Key_degree);
        QString px = i18n(" px");

        //setRange(minimum, maximum, decimals)

        angleKisDoubleSliderSpinBox     -> setRange(-90.0, 90.0, 1);
        separationKisDoubleSliderSpinBox-> setRange(1.0, 30.0, 1);
        thicknessKisDoubleSliderSpinBox -> setRange(1.0, 30.0, 1);
        originXKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);
        originYKisDoubleSliderSpinBox   -> setRange(-300, 300, 0);

        angleKisDoubleSliderSpinBox     -> setValue(-60);
        separationKisDoubleSliderSpinBox-> setValue(6);
        thicknessKisDoubleSliderSpinBox -> setValue(1);
        originXKisDoubleSliderSpinBox   -> setValue(50);
        originYKisDoubleSliderSpinBox   -> setValue(50);

        angleKisDoubleSliderSpinBox     -> setSuffix(degree);
        separationKisDoubleSliderSpinBox-> setSuffix(px);
        thicknessKisDoubleSliderSpinBox -> setSuffix(px);
        originXKisDoubleSliderSpinBox   -> setSuffix(px);
        originYKisDoubleSliderSpinBox   -> setSuffix(px);
    }
};

KisHatchingOptions::KisHatchingOptions()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisHatchingOptions");

    m_checkable = false;
    m_options = new KisHatchingOptionsWidget();

    connect(m_options->angleKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->separationKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->thicknessKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->originXKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->originYKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));

    connect(m_options->noCrosshatchingRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->perpendicularRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->minusThenPlusRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->plusThenMinusRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->moirePatternRadioButton, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));

    connect(m_options->separationIntervalSpinBox, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisHatchingOptions::~KisHatchingOptions()
{
}

void KisHatchingOptions::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    HatchingOption op;

    op.angle = m_options->angleKisDoubleSliderSpinBox->value();
    op.separation = m_options->separationKisDoubleSliderSpinBox->value();
    op.thickness = m_options->thicknessKisDoubleSliderSpinBox->value();
    op.origin_x = m_options->originXKisDoubleSliderSpinBox->value();
    op.origin_y = m_options->originYKisDoubleSliderSpinBox->value();

    op.bool_nocrosshatching = m_options->noCrosshatchingRadioButton->isChecked();
    op.bool_perpendicular = m_options->perpendicularRadioButton->isChecked();
    op.bool_minusthenplus = m_options->minusThenPlusRadioButton->isChecked();
    op.bool_plusthenminus = m_options->plusThenMinusRadioButton->isChecked();
    op.bool_moirepattern = m_options->moirePatternRadioButton->isChecked();

    op.separationintervals = m_options->separationIntervalSpinBox->value();

    op.writeOptionSetting(setting);
}

void KisHatchingOptions::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    HatchingOption op;
    op.readOptionSetting(setting);

    m_options->angleKisDoubleSliderSpinBox->setValue(op.angle);
    m_options->separationKisDoubleSliderSpinBox->setValue(op.separation);
    m_options->thicknessKisDoubleSliderSpinBox->setValue(op.thickness);
    m_options->originXKisDoubleSliderSpinBox->setValue(op.origin_x);
    m_options->originYKisDoubleSliderSpinBox->setValue(op.origin_y);

    m_options->noCrosshatchingRadioButton->setChecked(op.bool_nocrosshatching);
    m_options->perpendicularRadioButton->setChecked(op.bool_perpendicular);
    m_options->minusThenPlusRadioButton->setChecked(op.bool_minusthenplus);
    m_options->plusThenMinusRadioButton->setChecked(op.bool_plusthenminus);
    m_options->moirePatternRadioButton->setChecked(op.bool_moirepattern);

    m_options->separationIntervalSpinBox->setValue(op.separationintervals);
}

void KisHatchingOptions::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->limitations << KoID("hatching-brush", i18nc("PaintOp instant preview limitation", "Hatching Brush (heavy aliasing in preview mode)"));
}

