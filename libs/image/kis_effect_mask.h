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
#ifndef _KIS_EFFECT_MASK_
#define _KIS_EFFECT_MASK_

#include "kis_mask.h"
/**
 * An effect mask is a single channel mask that applies a particular
 * effect to the layer the mask belongs to. It differs from an
 * adjustment layer in that it only works on its parent layer, while
 * adjustment layers work on all layers below it in its layer group.
 * The effect could be a filter, a transformation or anything else
 * that messes up pixels.
 */
class KRITAIMAGE_EXPORT KisEffectMask : public KisMask
{

    Q_OBJECT

public:

    /**
     * Create an empty effect mask.
     */
    KisEffectMask(KisImageWSP image, const QString &name);

    ~KisEffectMask() override;
    KisEffectMask(const KisEffectMask& rhs);

    QIcon icon() const override;

    using KisMask::apply;
};

#endif //_KIS_EFFECT_MASK_
