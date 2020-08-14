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

#include "kis_brush_based_paintop_options_widget.h"

#include "kis_brush_option_widget.h"
#include <klocalizedstring.h>

KisBrushBasedPaintopOptionWidget::KisBrushBasedPaintopOptionWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_brushOption = new KisBrushOptionWidget();
    addPaintOpOption(m_brushOption, i18n("Brush Tip"));
}

KisBrushBasedPaintopOptionWidget::~KisBrushBasedPaintopOptionWidget()
{
}

void KisBrushBasedPaintopOptionWidget::setPrecisionEnabled(bool value)
{
    m_brushOption->setPrecisionEnabled(value);
}

void KisBrushBasedPaintopOptionWidget::setHSLBrushTipEnabled(bool value)
{
    m_brushOption->setHSLBrushTipEnabled(value);
}

KisBrushSP KisBrushBasedPaintopOptionWidget::brush()
{
    return m_brushOption->brush();
}

bool KisBrushBasedPaintopOptionWidget::presetIsValid()
{
    return m_brushOption->presetIsValid();
}

KisBrushOptionWidget *KisBrushBasedPaintopOptionWidget::brushOptionWidget() const
{
    return m_brushOption;
}
