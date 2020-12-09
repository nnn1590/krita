/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_curve_channel_list_delegate.h"
#include "kis_animation_curve_channel_list_model.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

const int COLOR_CIRCLE_RADIUS = 3;

KisAnimationCurveChannelListDelegate::KisAnimationCurveChannelListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void KisAnimationCurveChannelListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    QPalette palette = QApplication::palette();
    painter->setBrush(palette.buttonText());
    QRect textRect = option.rect.adjusted(2*COLOR_CIRCLE_RADIUS + 4, 0, 0, 0);
    painter->drawText(textRect, index.data().toString(), QTextOption(Qt::AlignVCenter|Qt::AlignLeft));

    QVariant colorData = index.data(KisAnimationCurveChannelListModel::CurveColorRole);
    if (colorData.isValid()) {
        QColor color = colorData.value<QColor>();

        painter->setPen(QPen(color, 1));

        if (index.data(KisAnimationCurveChannelListModel::CurveVisibleRole).toBool()) {
            painter->setBrush(color);
        } else {
            painter->setBrush(QBrush());
        }

        QPoint center = QPoint(option.rect.left() + COLOR_CIRCLE_RADIUS, option.rect.top() + option.rect.height() / 2);
        painter->drawEllipse(center, COLOR_CIRCLE_RADIUS, COLOR_CIRCLE_RADIUS);
    }

    painter->restore();
}

QSize KisAnimationCurveChannelListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    size.setWidth(2*COLOR_CIRCLE_RADIUS + 4 + size.width());

    return size;
}

bool KisAnimationCurveChannelListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);

        if (me->button() == Qt::LeftButton) {
            QPoint localPos = me->pos() - option.rect.topLeft();

            if (localPos.x() >= 0 && localPos.x() <= 2*COLOR_CIRCLE_RADIUS) {
                bool visible = index.data(KisAnimationCurveChannelListModel::CurveVisibleRole).toBool();
                model->setData(index, !visible, KisAnimationCurveChannelListModel::CurveVisibleRole);

                return true;
            }
        }
    }

    return false;
}
