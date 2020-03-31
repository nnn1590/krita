/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
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


#ifndef KISGAMUTMASKCHOOSER_H
#define KISGAMUTMASKCHOOSER_H

#include <QWidget>

class KisResourceItemChooser;
#include <KoResource.h>
#include <resources/KoGamutMask.h>

class KisGamutMaskDelegate;

class KisGamutMaskChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisGamutMaskChooser(QWidget *parent = nullptr);
    ~KisGamutMaskChooser() override;

    enum ViewMode {
        THUMBNAIL, // Shows thumbnails
        DETAIL  // Shows thumbsnails with text next to it
    };

    void setCurrentResource(KoResourceSP resource);

protected:
    void resizeEvent(QResizeEvent* event) override;

Q_SIGNALS:
    void sigGamutMaskSelected(KoGamutMaskSP mask);

private Q_SLOTS:
    void resourceSelected(KoResourceSP resource);
    void slotSetModeThumbnail();
    void slotSetModeDetail();

private:
    void setViewMode(ViewMode mode);
    void updateViewSettings();
    KisResourceItemChooser* m_itemChooser;
    KisGamutMaskDelegate* m_delegate;
    ViewMode m_mode;
};

#endif // KISGAMUTMASKCHOOSER_H
