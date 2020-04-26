/*
 *  dlg_rotateimage.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#ifndef DLG_ROTATEIMAGE
#define DLG_ROTATEIMAGE

#include <KoDialog.h>

#include "kis_global.h"

#include "ui_wdg_rotateimage.h"

enum enumRotationDirection {
    CLOCKWISE,
    COUNTERCLOCKWISE
};

class WdgRotateImage : public QWidget, public Ui::WdgRotateImage
{
    Q_OBJECT

public:
    WdgRotateImage(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgRotateImage: public KoDialog
{

    Q_OBJECT

public:

    DlgRotateImage(QWidget * parent = 0,
                   const char* name = 0);
    ~DlgRotateImage() override;

    void setAngle(quint32 w);
    double angle();

    void setDirection(enumRotationDirection direction);
    enumRotationDirection direction();

private Q_SLOTS:

    void okClicked();
    void resetPreview();
    void slotAngleValueChanged(double);

private:

    WdgRotateImage * m_page;
    double m_oldAngle {0.0};
    bool m_lock;

};

#endif // DLG_ROTATEIMAGE
