
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

#include "kis_hatching_pressure_separation_option.h"
#include <kis_pressure_opacity_option.h>

#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>


KisHatchingPressureSeparationOption::KisHatchingPressureSeparationOption()
    : KisCurveOption("Separation", KisPaintOpOption::GENERAL, true)
{
}

double KisHatchingPressureSeparationOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 0.5;
    return computeSizeLikeValue(info);
}
