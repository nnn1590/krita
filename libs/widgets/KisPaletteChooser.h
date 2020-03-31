/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
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

#ifndef KISPALETTELISTWIDGET_H
#define KISPALETTELISTWIDGET_H

#include <QString>
#include <QWidget>
#include <ui_WdgPaletteListWidget.h>

#include "kritawidgets_export.h"

#include <KoColorSet.h>

class KoResource;


struct KisPaletteChooserPrivate;

class KRITAWIDGETS_EXPORT KisPaletteChooser : public QWidget
{
    Q_OBJECT
public:
    explicit KisPaletteChooser(QWidget *parent = nullptr);
    virtual ~KisPaletteChooser();

public:
    void setAllowModification(bool allowModification);

Q_SIGNALS:
    void sigPaletteSelected(KoColorSetSP);
    void sigAddPalette();
    void sigRemovePalette(KoColorSetSP);
    void sigImportPalette();
    void sigExportPalette(KoColorSetSP);

public Q_SLOTS:

private /* methods */:
    QString newPaletteFileName();

private Q_SLOTS:
    void slotPaletteResourceSelected(KoResourceSP);
    void slotAdd();
    void slotRemove();
    void slotImport();
    void slotExport();

private:
    QScopedPointer<Ui_WdgPaletteListWidget> m_ui;
    QScopedPointer<KisPaletteChooserPrivate> m_d;
};

#endif // KISPALETTELISTWIDGET_H
