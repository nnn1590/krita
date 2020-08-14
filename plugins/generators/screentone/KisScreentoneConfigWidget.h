/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISSCREENTONECONFIGWIDGET_H
#define KISSCREENTONECONFIGWIDGET_H

#include <kis_config_widget.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "ui_KisScreentoneConfigWidget.h"

class Ui_WdgScreentoneOptions;

class KisScreentoneConfigWidget : public KisConfigWidget
{
    Q_OBJECT
public:
    KisScreentoneConfigWidget(QWidget* parent = 0, const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8());
    ~KisScreentoneConfigWidget() override;
public:
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;

private:
    Ui_ScreentoneConfigWidget m_ui;
    const KoColorSpace *m_colorSpace;

    void setupPatternComboBox();
    void setupShapeComboBox();
    void setupInterpolationComboBox();

private Q_SLOTS:
    void slot_comboBoxPattern_currentIndexChanged(int);
    void slot_comboBoxShape_currentIndexChanged(int);
    void slot_sliderSizeX_valueChanged(qreal value);
    void slot_sliderSizeY_valueChanged(qreal value);
    void slot_buttonKeepSizeSquare_keepAspectRatioChanged(bool keep);

};

#endif
