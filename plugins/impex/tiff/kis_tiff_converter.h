/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_TIFF_CONVERTER_H_
#define _KIS_TIFF_CONVERTER_H_

#include <stdio.h>
#include <tiffio.h>

#include <QVector>

#include "kis_types.h"
#include "kis_global.h"
#include "kis_annotation.h"
#include <KisImportExportErrorCode.h>
class KisDocument;

struct KisTIFFOptions {
    quint16 compressionType = 0;
    quint16 predictor = 1;
    bool alpha = true;
    bool flatten = true;
    quint16 jpegQuality = 80;
    quint16 deflateCompress = 6;
    quint16 pixarLogCompress = 6;
    bool saveProfile = true;

    KisPropertiesConfigurationSP toProperties() const;
    void fromProperties(KisPropertiesConfigurationSP cfg);
};

class KisTIFFConverter : public QObject
{
    Q_OBJECT
public:
    KisTIFFConverter(KisDocument *doc);
    ~KisTIFFConverter() override;
public:
    KisImportExportErrorCode buildImage(const QString &filename);
    KisImportExportErrorCode buildFile(const QString &filename, KisImageSP layer, KisTIFFOptions);
    /** Retrieve the constructed image
    */
    KisImageSP image();
public Q_SLOTS:
    virtual void cancel();
private:
    KisImportExportErrorCode decode(const QString &filename);
    KisImportExportErrorCode readTIFFDirectory(TIFF* image);
private:
    KisImageSP m_image;
    KisDocument *m_doc;
    bool m_stop;
};

#endif
