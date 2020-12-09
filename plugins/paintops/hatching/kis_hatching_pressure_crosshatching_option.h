/*
*  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
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
