/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisResourceQueryMapper.h"

#include <QString>
#include <QVariant>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QSqlError>

#include "KisResourceModel.h"
#include "KisResourceLocator.h"
#include "KisResourceModelProvider.h"
#include "KisTag.h"

QVariant KisResourceQueryMapper::variantFromResourceQuery(const QSqlQuery &query, int column, int role)
{
    const QString resourceType = query.value("resource_type").toString();
    KisResourceModel resourceModel(resourceType);

    switch(role) {
    case Qt::DisplayRole:
    {
        switch(column) {
        case KisAbstractResourceModel::Id:
            return query.value("id");
        case KisAbstractResourceModel::StorageId:
            return query.value("storage_id");
        case KisAbstractResourceModel::Name:
            return query.value("name");
        case KisAbstractResourceModel::Filename:
            return query.value("filename");
        case KisAbstractResourceModel::Tooltip:
            return query.value("tooltip");
        case KisAbstractResourceModel::Thumbnail:
        {
            QByteArray ba = query.value("thumbnail").toByteArray();
            QBuffer buf(&ba);
            buf.open(QBuffer::ReadOnly);
            QImage img;
            img.load(&buf, "PNG");
            return QVariant::fromValue<QImage>(img);
        }
        case KisAbstractResourceModel::Status:
            return query.value("status");
        case KisAbstractResourceModel::Location:
            return query.value("location");
        case KisAbstractResourceModel::ResourceType:
            return query.value("resource_type");
        case KisAbstractResourceModel::ResourceActive:
            return query.value("resource_active");
        case KisAbstractResourceModel::StorageActive:
            return query.value("storage_active");
        default:
            ;
        };
        Q_FALLTHROUGH();
    }
    case Qt::DecorationRole:
    {
        if (column == KisAbstractResourceModel::Thumbnail) {
            QByteArray ba = query.value("thumbnail").toByteArray();
            Q_ASSERT(!ba.isEmpty());
            QBuffer buf(&ba);
            buf.open(QBuffer::ReadOnly);
            QImage img;
            img.load(&buf, "PNG");
            Q_ASSERT(!img.isNull());
            return QVariant::fromValue<QImage>(img);
        }
        return QVariant();
    }
    case Qt::ToolTipRole:
        Q_FALLTHROUGH();
    case Qt::StatusTipRole:
        Q_FALLTHROUGH();
    case Qt::WhatsThisRole:
        return query.value("tooltip");
    case Qt::UserRole + KisAbstractResourceModel::Id:
        return query.value("id");
    case Qt::UserRole + KisAbstractResourceModel::StorageId:
        return query.value("storage_id");
    case Qt::UserRole + KisAbstractResourceModel::Name:
        return query.value("name");
    case Qt::UserRole + KisAbstractResourceModel::Filename:
        return query.value("filename");
    case Qt::UserRole + KisAbstractResourceModel::Tooltip:
        return query.value("tooltip");
    case Qt::UserRole + KisAbstractResourceModel::Thumbnail:
    {
        QByteArray ba = query.value("thumbnail").toByteArray();
        QBuffer buf(&ba);
        buf.open(QBuffer::ReadOnly);
        QImage img;
        img.load(&buf, "PNG");
        return QVariant::fromValue<QImage>(img);
    }
    case Qt::UserRole + KisAbstractResourceModel::Status:
        return query.value("status");
    case Qt::UserRole + KisAbstractResourceModel::Location:
        return query.value("location");
    case Qt::UserRole + KisAbstractResourceModel::ResourceType:
        return query.value("resource_type");
    case Qt::UserRole + KisAbstractResourceModel::Tags:
    {
        QStringList tagNames;
        Q_FOREACH(const KisTagSP tag, resourceModel.tagsForResource(query.value("id").toInt())) {
            tagNames << tag->name();
        }
        return tagNames;
    }
    case Qt::UserRole + KisAbstractResourceModel::Dirty:
    {
        QString storageLocation = query.value("location").toString();
        QString filename = query.value("filename").toString();

        // An uncached resource has not been loaded, so it cannot be dirty
        if (!KisResourceLocator::instance()->resourceCached(storageLocation, resourceType, filename)) {
            return false;
        }
        else {
            // Now we have to check the resource, but that's cheap since it's been loaded in any case
            KoResourceSP resource = resourceModel.resourceForId(query.value("id").toInt());
            return resource->isDirty();
        }
    }
    case Qt::UserRole + KisAbstractResourceModel::MetaData:
    {
        QMap<QString, QVariant> r = KisResourceLocator::instance()->metaDataForResource(query.value("id").toInt());
        return r;
    }
    case Qt::UserRole + KisAbstractResourceModel::ResourceActive:
    {
        return query.value("resource_active");
    }
    case Qt::UserRole + KisAbstractResourceModel::StorageActive:
    {
        return query.value("storage_active");
    }
    default:
        ;
    }

    return QVariant();
}

QVariant KisResourceQueryMapper::variantFromResourceQueryById(int resourceId, int column, int role)
{
    QVariant v;

    QSqlQuery q;
    if (!q.prepare("SELECT resources.id\n"
              ",      resources.storage_id\n"
              ",      resources.name\n"
              ",      resources.filename\n"
              ",      resources.tooltip\n"
              ",      resources.thumbnail\n"
              ",      resources.status\n"
              ",      storages.location\n"
              ",      resources.version\n"
              ",      resource_types.name as resource_type\n"
              ",      resources.status as resource_active\n"
              ",      storages.active as storage_active\n"
              "FROM   resources\n"
              ",      resource_types\n"
              ",      storages\n"
             "WHERE  resources.id = :resource_id"))
    {
        qWarning() << "Could not prepare variantFromResourceQueryById query" << q.lastError();
        return v;
    }

    q.bindValue(":resource_id", resourceId);

    if (!q.exec()) {
        qWarning() << "Could not execute variantFromResourceQueryById query" << q.lastError() << q.boundValues();
        return v;
    }

    if (!q.first()) {
        qWarning() << "Could not find resource with id" << resourceId;
    }

    v = variantFromResourceQuery(q, column, role);

    return v;
}
