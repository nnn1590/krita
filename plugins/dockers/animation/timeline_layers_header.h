/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#ifndef __TIMELINE_LAYERS_HEADER_H
#define __TIMELINE_LAYERS_HEADER_H

#include <QHeaderView>
#include <QProxyStyle>

#include <QScopedPointer>


class TimelineLayersHeader : public QHeaderView
{
    Q_OBJECT

public:
    TimelineLayersHeader(QWidget *parent);
    TimelineLayersHeader();
    ~TimelineLayersHeader() override;

protected:
    void paintSection(QPainter *painter, const QRect &rect, int layerIndex) const override;
    QSize sectionSizeFromContents(int layerIndex) const override;
    bool viewportEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override {mousePressEvent(event);}

Q_SIGNALS:
    void sigRequestContextMenu(const QPoint &pos);

private:
    struct Private;
    const QScopedPointer<Private> m_d;


};

#endif /* __TIMELINE_LAYERS_HEADER_H */
