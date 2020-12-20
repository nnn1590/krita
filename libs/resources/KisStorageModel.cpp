/*
 * SPDX-FileCopyrightText: 2019 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisStorageModel.h"

#include <QtSql>
#include <QElapsedTimer>
#include <KisResourceLocator.h>
#include <KisResourceModelProvider.h>

Q_GLOBAL_STATIC(KisStorageModel, s_instance)

struct KisStorageModel::Private {
    int cachedRowCount {-1};
    QList<QString> storages;
};

KisStorageModel::KisStorageModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
    connect(KisResourceLocator::instance(), SIGNAL(storageAdded(const QString&)), this, SLOT(addStorage(const QString&)));
    connect(KisResourceLocator::instance(), SIGNAL(storageRemoved(const QString&)), this, SLOT(removeStorage(const QString&)));

    QSqlQuery query;

    bool r = query.prepare("SELECT location\n"
                           "FROM   storages\n"
                           "ORDER BY id");
    if (!r) {
        qWarning() << "Could not prepare KisStorageModel query" << query.lastError();
    }

    r = query.exec();

    if (!r) {
        qWarning() << "Could not execute KisStorageModel query" << query.lastError();
    }

    while (query.next()) {
        d->storages << query.value(0).toString();
    }
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
    return d->storages.size();
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

    QString location = d->storages.at(index.row());

    QSqlQuery query;

    bool r = query.prepare("SELECT storages.id as id\n"
                           ",      storage_types.name as storage_type\n"
                           ",      location\n"
                           ",      timestamp\n"
                           ",      pre_installed\n"
                           ",      active\n"
                           ",      thumbnail\n"
                           "FROM   storages\n"
                           ",      storage_types\n"
                           "WHERE  storages.storage_type_id = storage_types.id\n"
                           "AND    location = :location");

    if (!r) {
        qWarning() << "Could not prepare KisStorageModel data query" << query.lastError();
        return v;
    }

    query.bindValue(":location", location);

    r = query.exec();

    if (!r) {
        qWarning() << "Could not execute KisStorageModel data query" << query.lastError() << query.boundValues();
        return v;
    }

    if (!query.first()) {
        qWarning() << "KisStorageModel data query did not return anything";
        return v;
    }

    if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() == Active) {
        return query.value("active");
    }
    else
    {
        switch(role) {
        case Qt::DisplayRole:
        {
            switch(index.column()) {
            case Id:
                return query.value("id");
            case StorageType:
                return query.value("storage_type");
            case Location:
                return query.value("location");
            case TimeStamp:
                return query.value("timestamp");
            case PreInstalled:
                return query.value("pre_installed");
            case Active:
                return query.value("active");
            case Thumbnail:
            {
                QByteArray ba = query.value("thumbnail").toByteArray();
                QBuffer buf(&ba);
                buf.open(QBuffer::ReadOnly);
                QImage img;
                img.load(&buf, "PNG");
                return QVariant::fromValue<QImage>(img);
            }
            case DisplayName:
            {
                QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(query.value("location").toString());
                QVariant name = query.value("location");
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
                QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(query.value("location").toString());
                return r;
            }
            default:
                return v;
            }
        }
        case Qt::UserRole + Id:
            return query.value("id");
        case Qt::UserRole + DisplayName:
        {
            QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(query.value("location").toString());
            QVariant name = query.value("location");
            if (r.contains(KisResourceStorage::s_meta_name) && !r[KisResourceStorage::s_meta_name].isNull()) {
                name = r[KisResourceStorage::s_meta_name];
            }
            else if (r.contains(KisResourceStorage::s_meta_title) && !r[KisResourceStorage::s_meta_title].isNull()) {
                name = r[KisResourceStorage::s_meta_title];
            }
            return name;
        }
        case Qt::UserRole + StorageType:
            return query.value("storage_type");
        case Qt::UserRole + Location:
            return query.value("location");
        case Qt::UserRole + TimeStamp:
            return query.value("timestamp");
        case Qt::UserRole + PreInstalled:
            return query.value("pre_installed");
        case Qt::UserRole + Active:
            return query.value("active");
        case Qt::UserRole + Thumbnail:
        {
            QByteArray ba = query.value("thumbnail").toByteArray();
            QBuffer buf(&ba);
            buf.open(QBuffer::ReadOnly);
            QImage img;
            img.load(&buf, "PNG");
            return QVariant::fromValue<QImage>(img);
        }
        case Qt::UserRole + MetaData:
        {
            QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForStorage(query.value("location").toString());
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
            QSqlQuery query;
            bool r = query.prepare("UPDATE storages\n"
                                   "SET    active = :active\n"
                                   "WHERE  id = :id\n");
            query.bindValue(":active", value);
            query.bindValue(":id", index.data(Qt::UserRole + Id));

            if (!r) {
                qWarning() << "Could not prepare KisStorageModel update query" << query.lastError();
                return false;
            }

            r = query.exec();

            if (!r) {
                qWarning() << "Could not execute KisStorageModel update query" << query.lastError();
                return false;
            }

        }
    }
    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags KisStorageModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

KisResourceStorageSP KisStorageModel::storageForIndex(const QModelIndex &index) const
{

    if (!index.isValid()) return 0;
    if (index.row() > rowCount()) return 0;
    if (index.column() > (int)MetaData) return 0;

    QString location = d->storages.at(index.row());

    return KisResourceLocator::instance()->storageByLocation(KisResourceLocator::instance()->makeStorageLocationAbsolute(location));
}

QVariant KisStorageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant v = QVariant();
    if (role != Qt::DisplayRole) {
        return v;
    }
    if (orientation == Qt::Horizontal) {
        switch(section) {
        case Id:
            return i18n("Id");
        case StorageType:
            return i18n("Type");
        case Location:
            return i18n("Location");
        case TimeStamp:
            return i18n("Creation Date");
        case PreInstalled:
            return i18n("Preinstalled");
        case Active:
            return i18n("Active");
        case Thumbnail:
            return i18n("Thumbnail");
        case DisplayName:
            return i18n("Name");
        default:
            v = QString::number(section);
        }
        return v;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void KisStorageModel::addStorage(const QString &location)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    d->storages.append(location);
    endInsertRows();
}

void KisStorageModel::removeStorage(const QString &location)
{
    int index = d->storages.indexOf(location);
    beginRemoveRows(QModelIndex(), index, index);
    d->storages.removeAt(index);
    endRemoveRows();
}


