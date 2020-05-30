/*
 *  Copyright (c) 2020 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Scratchpad.h"
#include <KoResource.h>
#include <kis_config.h>
#include "kis_scratch_pad.h"
#include "Resource.h"
#include "View.h"
#include "Canvas.h"
#include <KoCanvasBase.h>
#include <kis_canvas2.h>

#include <QColor>

Scratchpad::Scratchpad(View *view, const QColor & defaultColor, QWidget *parent)
    : KisScratchPad(parent)
{
    KisScratchPad::setupScratchPad(view->view()->resourceProvider(), defaultColor);
    KisScratchPad::setMinimumSize(50, 50);
}

Scratchpad::~Scratchpad()
{
}

void Scratchpad::setModeManually(bool value)
{
    KisScratchPad::setModeManually(value);
}

void Scratchpad::setMode(QString modeType)
{
    KisScratchPad::setModeType(modeType);
}

void Scratchpad::loadScratchpad(QImage image)
{
    KisScratchPad::loadScratchpadImage(image);
}

QImage Scratchpad::copyScratchPadImage()
{
    return KisScratchPad::copyScratchpadImageData();
}

void Scratchpad::clear()
{
    // need ability to set color
    KisScratchPad::fillDefault();
}

void Scratchpad::setFillColor(QColor color)
{
    KisScratchPad::setFillColor(color);
}
