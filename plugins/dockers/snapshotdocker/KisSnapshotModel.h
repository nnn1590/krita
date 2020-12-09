/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SNAPSHOT_MODEL_H_
#define KIS_SNAPSHOT_MODEL_H_

#include <QAbstractListModel>
#include <QScopedPointer>
#include <QPointer>

#include <kis_canvas2.h>

class KisSnapshotModel : public QAbstractListModel
{
public:
    KisSnapshotModel();
    ~KisSnapshotModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    // this function is re-implemented to make the items editable
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void setCanvas(QPointer<KisCanvas2> canvas);

public Q_SLOTS:
    bool slotCreateSnapshot();
    bool slotRemoveSnapshot(const QModelIndex &index);
    bool slotSwitchToSnapshot(const QModelIndex &index);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KIS_SNAPSHOT_MODEL_H_
