/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef KISTAGFILTERWIDGET_H
#define KISTAGFILTERWIDGET_H

#include <QWidget>
#include <KisTagModel.h>

class KisTagFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KisTagFilterWidget(KisTagModel* model, QWidget* parent);
    ~KisTagFilterWidget() override;
    void clear();

Q_SIGNALS:
    void filterTextChanged(const QString &filterText);
    void filterByTagChanged(const bool filterByTag);
private Q_SLOTS:
    void onTextChanged(const QString &lineEditText);
    void slotFilterByTagChanged(int filterByTag);
private:
    class Private;
    Private* const d;
};

#endif // KOTAGFILTERWIDGET_H
