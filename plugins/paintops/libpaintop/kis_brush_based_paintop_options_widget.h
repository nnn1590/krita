/*
 *  Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_BRUSH_BASED_PAINTOP_OPTIONS_WIDGET_H
#define KIS_BRUSH_BASED_PAINTOP_OPTIONS_WIDGET_H

#include "kis_paintop_settings_widget.h"
#include "kis_types.h"
#include "kis_brush.h"
#include <kritapaintop_export.h>

class KisBrushOptionWidget;

class PAINTOP_EXPORT KisBrushBasedPaintopOptionWidget : public KisPaintOpSettingsWidget
{
public:
    KisBrushBasedPaintopOptionWidget(QWidget* parent = 0);
    ~KisBrushBasedPaintopOptionWidget() override;

    void setPrecisionEnabled(bool value);
    void setHSLBrushTipEnabled(bool value);

    KisBrushSP brush();

    bool presetIsValid() override;

protected:
    KisBrushOptionWidget *brushOptionWidget() const;

private:
    KisBrushOptionWidget *m_brushOption;
};

#endif // KIS_BRUSH_BASED_PAINTOP_OPTIONS_WIDGET_H
