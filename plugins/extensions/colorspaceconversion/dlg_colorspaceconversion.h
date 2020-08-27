    /*
 *  dlg_colorspaceconversion.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_COLORSPACECONVERSION
#define DLG_COLORSPACECONVERSION

#include <QButtonGroup>

#include <KoDialog.h>

#include <KoID.h>
#include "kis_types.h"

#include "ui_wdgconvertcolorspace.h"

class KoColorSpace;

class WdgConvertColorSpace : public QWidget, public Ui::WdgConvertColorSpace
{
    Q_OBJECT

public:
    WdgConvertColorSpace(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * XXX
 */
class DlgColorSpaceConversion: public KoDialog
{

    Q_OBJECT

public:

    DlgColorSpaceConversion(QWidget * parent = 0, const char* name = 0);
    ~DlgColorSpaceConversion() override;

    void setInitialColorSpace(const KoColorSpace *cs, KisImageSP entireImage);

    WdgConvertColorSpace * m_page;

    QButtonGroup m_intentButtonGroup;

public Q_SLOTS:
    void selectionChanged(bool);
    void okClicked();
    void slotColorSpaceChanged(const KoColorSpace *cs);

private:
    KisImageSP m_image;
};

#endif // DLG_COLORSPACECONVERSION
