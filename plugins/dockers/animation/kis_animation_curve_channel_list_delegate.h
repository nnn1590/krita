/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVE_CHANNEL_LIST_DELEGATE_H
#define _KIS_ANIMATION_CURVE_CHANNEL_LIST_DELEGATE_H

#include <qstyleditemdelegate.h>

class KisAnimationCurveChannelListDelegate : public QStyledItemDelegate
{
public:
    KisAnimationCurveChannelListDelegate(QObject *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif
