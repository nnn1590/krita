/*
 *  Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H
#define KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H

#include <QString>
#include <KisImportExportErrorCode.h>

class KRITAUI_EXPORT KisImportExportAdditionalChecks
{

public:

    static bool isFileWritable(QString filepath);
    static bool isFileReadable(QString filepath);
    static bool doesFileExist(QString filepath);
};




#endif // #ifndef KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H
