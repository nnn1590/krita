/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
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

#include "kis_effect_mask.h"

#include <KoIcon.h>
#include <kis_icon.h>
#include "kis_image.h"

KisEffectMask::KisEffectMask(KisImageWSP image, const QString &name)
        : KisMask(image, name)
{
}

KisEffectMask::~KisEffectMask()
{
}

KisEffectMask::KisEffectMask(const KisEffectMask& rhs)
        : KisMask(rhs)
{
}

QIcon KisEffectMask::icon() const
{
    return KisIconUtils::loadIcon("bookmarks");
}

