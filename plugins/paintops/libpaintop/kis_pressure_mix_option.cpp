/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_mix_option.h"
#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>

KisPressureMixOption::KisPressureMixOption()
    : KisCurveOption("Mix", KisPaintOpOption::COLOR, false)
{
}


double KisPressureMixOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}
