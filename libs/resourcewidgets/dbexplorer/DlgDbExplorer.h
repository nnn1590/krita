/*
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef DLG_DBEXPLORER
#define DLG_DBEXPLORER

#include <KoDialog.h>

#include "ui_WdgDbExplorer.h"
//#include <KisTagFilterResourceProxyModel.h>

class KisResourceModel;
class KisTagModel;
class KisResourceTypeModel;
class KisTagFilterResourceProxyModel;


class WdgDbExplorer : public QWidget, public Ui::WdgDbExplorer
{
    Q_OBJECT

public:
    WdgDbExplorer(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgDbExplorer: public KoDialog
{
    Q_OBJECT
public:
    DlgDbExplorer(QWidget * parent = 0);
    ~DlgDbExplorer() override;

private Q_SLOTS:

    void slotTbResourceTypeSelected(int index);
    void slotTbResourceItemSelected();

    void slotRvResourceTypeSelected(int index);
    void slotRvTagSelected(int index);

    void slotResetTagModel(QModelIndex topLeft, QModelIndex bottomRight);

private:
    void updateTagModel(const QString& resourceType);

    WdgDbExplorer *m_page {0};

    KisTagModel *m_tagModel {0};
    KisResourceTypeModel *m_resourceTypeModel {0};
    KisTagFilterResourceProxyModel* m_filterProxyModel {0};
};

#endif // DLG_DBEXPLORER
