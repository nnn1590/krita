/*
 * Copyright (c) 2019 boud <boud@valdyas.org>
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
#include "KisStorageModel.h"

#include <QtSql>
#include <QElapsedTimer>
#include <KisResourceLocator.h>
#include <KisResourceModelProvider.h>

Q_GLOBAL_STATIC(KisStorageModel, s_instance)

struct KisStorageModel::Private {
    int cachedRowCount {-1};
    QSqlQuery query;
};

KisStorageModel::KisStorageModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    prepareQuery();
    connect(KisResourceLocator::instance(), SIGNAL(storageAdded()), this, SLOT(resetQuery()));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved()), this, SLOT(resetQuery()));
}

KisStorageModel *KisStorageModel::instance()
{
    return s_instance;
}

KisStorageModel::~KisStorageModel()
{
}

int KisStorageModel::rowCount(const QModelIndex & /*parent*/) const
{
    if (d->cachedRowCount < 0) {
        QSqlQuery q;
        q.prepare("SELECT count(*)\n"
                  "FROM   storages\n");
        q.exec();
        q.first();

        const_cast<KisStorageModel*>(this)->d->cachedRowCount = q.value(0).toInt();
    }
    return d->cachedRowCount;
}

int KisStorageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 8;
}

QVariant KisStorageModel::data(const QModelIndex &index, int role) const
{
    QVariant v;

    if (!index.isValid()) return v;
    if (index.row() > rowCount()) return v;
    if (index.column() > (int)MetaData) return v;

    bool pos = d->query.seek(index.row());

    if (pos) {
        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                return d->query.value("id");
            case StorageType:
                return d->query.value("storage_type");
            case Location:
                return d->query.value("location");
            case TimeStamp:
               return d->query.value("timestamp");
            case PreInstalled:
                return d->query.value("pre_installed");
            case Active:
                return d->query.value("active");
            case Thumbnail:
            {
                QByteArray ba = d->query.value("thumbnail").toByteArray();
                QBuffer buf(&ba);
                buf.open(QBuffer::ReadOnly);
                QImage img;
                img.load(&buf, "PNG");
                return QVariant::fromValue<QImage>(img);
            }
            case DisplayName:
            {
                QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(d->query.value("location").toString());
                QVariant name = d->query.value("location");
                if (r.contains(KisResourceStorage::s_meta_name) && !r[KisResourceStorage::s_meta_name].isNull()) {
                    name = r[KisResourceStorage::s_meta_name];
                }
                else if (r.contains(KisResourceStorage::s_meta_title) && !r[KisResourceStorage::s_meta_title].isNull()) {
                    name = r[KisResourceStorage::s_meta_title];
                }
                return name;
            }
            case Qt::UserRole + MetaData:
            {
                QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(d->query.value("location").toString());
                return r;
            }
            default:
                return v;
            }
        }
        case Qt::UserRole + Id:
            return d->query.value("id");
        case Qt::UserRole + DisplayName:
        {
            QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(d->query.value("location").toString());
            QVariant name = d->query.value("location");
            if (r.contains(KisResourceStorage::s_meta_name) && !r[KisResourceStorage::s_meta_name].isNull()) {
                name = r[KisResourceStorage::s_meta_name];
            }
            else if (r.contains(KisResourceStorage::s_meta_title) && !r[KisResourceStorage::s_meta_title].isNull()) {
                name = r[KisResourceStorage::s_meta_title];
            }
            return name;
        }
        case Qt::UserRole + StorageType:
            return d->query.value("storage_type");
        case Qt::UserRole + Location:
            return d->query.value("location");
        case Qt::UserRole + TimeStamp:
           return d->query.value("timestamp");
        case Qt::UserRole + PreInstalled:
            return d->query.value("pre_installed");
        case Qt::UserRole + Active:
            return d->query.value("active");
        case Qt::UserRole + Thumbnail:
        {
            QByteArray ba = d->query.value("thumbnail").toByteArray();
            QBuffer buf(&ba);
            buf.open(QBuffer::ReadOnly);
            QImage img;
            img.load(&buf, "PNG");
            return QVariant::fromValue<QImage>(img);
        }
        case Qt::UserRole + MetaData:
        {
            QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(d->query.value("location").toString());
            return r;
        }

        default:
            ;
        }
    }
    return v;

}

bool KisStorageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid()) {
        if (role == Qt::CheckStateRole) {
            QSqlQuery q;
            bool r = q.prepare("UPDATE storages\n"
                               "SET    active = :active\n"
                               "WHERE  id = :id\n");
            q.bindValue(":active", value);
            q.bindValue(":id", index.data(Qt::UserRole + Id));
            if (!r) {
                qWarning() << "Could not prepare KisStorageModel update query" << d->query.lastError();
                return false;
            }
            r = q.exec();
            if (!r) {
                qWarning() << "Could not execute KisStorageModel update query" << d->query.lastError();
                return false;
            }

        }
    }
    QAbstractTableModel::setData(index, value, role);
    KisResourceModelProvider::resetAllModels();
    return prepareQuery();
}

Qt::ItemFlags KisStorageModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

KisResourceStorageSP KisStorageModel::storageForIndex(const QModelIndex &index) const
{
    return KisResourceLocator::instance()->storageByLocation(KisResourceLocator::instance()->makeStorageLocationAbsolute(index.data(Qt::UserRole + Location).toString()));
}

QVariant KisStorageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant v = QVariant();
    if (role != Qt::DisplayRole) {
        return v;
    }
    if (orientation == Qt::Horizontal) {
        switch(section) {
        case 0:
            v = i18n("Id");
            break;
        case 1:
            v = i18n("Type");
            break;
        case 2:
            v = i18n("Location");
            break;
        case 3:
            v = i18n("Creation Date");
            break;
        case 4:
            v = i18n("Preinstalled");
            break;
        case 5:
            v = i18n("Active");
            break;
        case 6:
            v = i18n("Thumbnail");
            break;
        case 7:
            v = i18n("Name");
            break;
        default:
            v = QString::number(section);
        }
        return v;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

bool KisStorageModel::resetQuery()
{
    QElapsedTimer t;
    t.start();

    beginResetModel();
    bool r = d->query.exec();
    if (!r) {
        qWarning() << "Could not select storages" << d->query.lastError() << d->query.boundValues();
    }
    d->cachedRowCount = -1;

    endResetModel();
    qDebug() << "KisStorageModel::resetQuery took" << t.elapsed() << "ms";

    return r;
}

bool KisStorageModel::prepareQuery()
{
    beginResetModel();
    bool r = d->query.prepare("SELECT storages.id as id\n"
                              ",      storage_types.name as storage_type\n"
                              ",      location\n"
                              ",      timestamp\n"
                              ",      pre_installed\n"
                              ",      active\n"
                              ",      thumbnail\n"
                              "FROM   storages\n"
                              ",      storage_types\n"
                              "WHERE  storages.storage_type_id = storage_types.id\n");
    if (!r) {
        qWarning() << "Could not prepare KisStorageModel query" << d->query.lastError();
    }
    r = d->query.exec();
    if (!r) {
        qWarning() << "Could not execute KisStorageModel query" << d->query.lastError();
    }
    d->cachedRowCount = -1;
    endResetModel();
    return r;
}
