/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISLAYERSTYLEKNOCKOUTBLOWER_H
#define KISLAYERSTYLEKNOCKOUTBLOWER_H

#include "kis_selection.h"
#include <QReadWriteLock>

class KisPainter;


class KRITAIMAGE_EXPORT KisLayerStyleKnockoutBlower
{
public:
    KisLayerStyleKnockoutBlower();
    KisLayerStyleKnockoutBlower(const KisLayerStyleKnockoutBlower &rhs);

    KisSelectionSP knockoutSelectionLazy();

    void setKnockoutSelection(KisSelectionSP selection);
    void resetKnockoutSelection();


    void apply(KisPainter *painter, KisPaintDeviceSP mergedStyle, const QRect &rect) const;
    bool isEmpty() const;

private:
    mutable QReadWriteLock m_lock;
    KisSelectionSP m_knockoutSelection;
};

#endif // KISLAYERSTYLEKNOCKOUTBLOWER_H
