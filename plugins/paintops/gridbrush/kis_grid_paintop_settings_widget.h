/*
 * Copyright (c) 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRIDPAINTOP_SETTINGS_WIDGET_H_
#define KIS_GRIDPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class KisGridOpOption;
class KisGridShapeOption;
class KisColorOption;

class KisGridPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisGridPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisGridPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

public:
    KisGridOpOption *m_gridOption;
    KisGridShapeOption *m_gridShapeOption;
    KisColorOption *m_ColorOption;
};

#endif
