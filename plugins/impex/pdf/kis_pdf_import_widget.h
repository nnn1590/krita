/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PDF_IMPORT_WIDGET_H
#define KIS_PDF_IMPORT_WIDGET_H

#include <QWidget>
#include <QList>
#include "ui_pdfimportwidgetbase.h"

namespace Poppler
{
class Document;
}

class KisPDFImportWidget : public QWidget, public Ui_PDFImportWidgetBase
{
    Q_OBJECT
public:
    KisPDFImportWidget(Poppler::Document* pdfDoc, QWidget * parent);

    ~KisPDFImportWidget();
public:
    inline QList<int>  pages() {
        return m_pages;
    }
private Q_SLOTS:
    void selectAllPages(bool v);
    void selectFirstPage(bool v);
    void selectSelectionOfPages(bool v);
    void updateSelectionOfPages();
    void updateResolution();
    void updateHRes();
    void updateHVer();
    void heightAspectRatio();
    void widthAspectRatio();
    void slotAspectChanged(bool keep);
    void updateMaxCanvasSize();
private:
    Poppler::Document* m_pdfDoc;
    QList<int> m_pages;
    double m_maxWidthInch, m_maxHeightInch;
    bool m_keepAspect;
};

#endif
