/*
*  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIS_HATCHING_PRESSURE_CROSSHATCHING_OPTION_H
#define KIS_HATCHING_PRESSURE_CROSSHATCHING_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>

/**
 * The pressure crosshatching option defines a curve that is used to
 * calculate the effect of pressure (or other parameters) on
 * crosshatching in the hatching brush
 */
class KisHatchingPressureCrosshatchingOption : public KisCurveOption
{
public:
    KisHatchingPressureCrosshatchingOption();
    double apply(const KisPaintInformation & info) const;
};

#endif
