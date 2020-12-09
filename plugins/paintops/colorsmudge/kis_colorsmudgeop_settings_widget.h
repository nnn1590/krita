/*
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLORSMUDGEOP_SETTINGS_WIDGET_H_
#define KIS_COLORSMUDGEOP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>

class KisSmudgeOptionWidget;

class KisColorSmudgeOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{
    Q_OBJECT

public:
    KisColorSmudgeOpSettingsWidget(QWidget* parent = 0);
    ~KisColorSmudgeOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

protected:
    void notifyPageChanged() override;

private:
    KisSmudgeOptionWidget *m_smudgeOptionWidget;
};



#endif // KIS_COLORSMUDGEOP_SETTINGS_WIDGET_H_
