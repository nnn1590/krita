/*
 * This file is part of Krita
 *
 * Copyright (c) 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_WDG_LENS_BLUR_H_
#define _KIS_WDG_LENS_BLUR_H_

#include <kis_config_widget.h>

class Ui_WdgLensBlur;

class KisWdgLensBlur : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgLensBlur(QWidget * parent);
    ~KisWdgLensBlur() override;
    inline const Ui_WdgLensBlur* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgLensBlur* m_widget;
    QMap<QString, QString> m_shapeTranslations;
};

#endif
