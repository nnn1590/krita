/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisMemoryStorage.h"

#include <QVector>
#include <QFileInfo>

#include <KisMimeDatabase.h>
#include <kis_debug.h>
#include <KisTag.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceStorage.h>
#include <QBuffer>
#include <KisGlobalResourcesInterface.h>
#include <kis_pointer_utils.h>
#include <KoMD5Generator.h>


struct StoredResource
{
    QDateTime timestamp;
    QSharedPointer<QByteArray> data;
};

class MemoryTagIterator : public KisResourceStorage::TagIterator
{
public:

    MemoryTagIterator(QVector<KisTagSP> tags, const QString &resourceType)
        : m_tags(tags)
        , m_resourceType(resourceType)
    {
    }

    bool hasNext() const override
    {
        return m_currentPosition < m_tags.size();
    }

    void next() override
    {
        const_cast<MemoryTagIterator*>(this)->m_currentPosition += 1;
    }

    QString url() const override { return tag() ? tag()->url() : QString(); }
    QString name() const override { return tag() ? tag()->name() : QString(); }
    QString comment() const override {return tag() ? tag()->comment() : QString(); }
    QString resourceType() const override { return  tag() ? tag()->resourceType() : resourceType(); }

    KisTagSP tag() const override
    {
        if (m_currentPosition >= m_tags.size()) return 0;
        return m_tags.at(m_currentPosition);

    }

private:

    int m_currentPosition {0};
    QVector<KisTagSP> m_tags;
    QString m_resourceType;
};


class MemoryItem : public KisResourceStorage::ResourceItem
{
public:
    ~MemoryItem() override {}
};


class KisMemoryStorage::Private {
public:
    QHash<QString, QHash<QString, StoredResource>> resourcesNew;
    QHash<QString, QVector<KisTagSP>> tags;
    QMap<QString, QVariant> metadata;
};


KisMemoryStorage::KisMemoryStorage(const QString &location)
    : KisStoragePlugin(location)
    , d(new Private)
{
}

KisMemoryStorage::~KisMemoryStorage()
{
}

KisMemoryStorage::KisMemoryStorage(const KisMemoryStorage &rhs)
    : KisStoragePlugin(rhs.location())
    , d(new Private)
{
    *this = rhs;
}

KisMemoryStorage &KisMemoryStorage::operator=(const KisMemoryStorage &rhs)
{
    if (this != &rhs) {
        d->resourcesNew = rhs.d->resourcesNew;

        Q_FOREACH(const QString &key, rhs.d->tags.keys()) {
            Q_FOREACH(const KisTagSP tag, rhs.d->tags[key]) {
                if (!d->tags.contains(key)) {
                    d->tags[key] = QVector<KisTagSP>();
                }
                d->tags[key] << tag->clone();
            }
        }
    }
    return *this;
}

bool KisMemoryStorage::addTag(const QString &resourceType, KisTagSP tag)
{
    if (!d->tags.contains(resourceType)) {
        d->tags[resourceType] = QVector<KisTagSP>();
    }
    if (!d->tags[resourceType].contains(tag)) {
        d->tags[resourceType].append(tag);
    }
    return true;
}

bool KisMemoryStorage::addResource(const QString &resourceType, KoResourceSP resource)
{
    QHash<QString, StoredResource> &typedResources =
        d->resourcesNew[resourceType];

    auto checkExists =
        [&typedResources] (const QString &filename) {
            return typedResources.contains(filename);
        };

    const QString newFilename =
        KisStorageVersioningHelper::chooseUniqueName(resource, 0, checkExists);

    if (newFilename.isEmpty()) return false;

    resource->setFilename(newFilename);

    StoredResource storedResource;
    storedResource.timestamp = QDateTime::currentDateTime();
    storedResource.data.reset(new QByteArray());
    QBuffer buffer(storedResource.data.data());
    buffer.open(QIODevice::WriteOnly);
    resource->saveToDevice(&buffer);
    buffer.close();

    typedResources.insert(newFilename, storedResource);

    return true;
}

KisResourceStorage::ResourceItem KisMemoryStorage::resourceItem(const QString &url)
{
    MemoryItem item;
    item.url = url;
    item.folder = QString();
    item.lastModified = QDateTime::fromMSecsSinceEpoch(0);
    return item;
}

bool KisMemoryStorage::loadVersionedResource(KoResourceSP resource)
{
    const QString resourceType = resource->resourceType().first;
    const QString resourceFilename = resource->filename();

    bool retval = false;

    if (d->resourcesNew.contains(resourceType) &&
        d->resourcesNew[resourceType].contains(resourceFilename)) {

        const StoredResource &storedResource =
            d->resourcesNew[resourceType][resourceFilename];

        QBuffer buffer(storedResource.data.data());
        buffer.open(QIODevice::ReadOnly);
        resource->loadFromDevice(&buffer, KisGlobalResourcesInterface::instance());
        retval = true;
    }

    return retval;
}

QByteArray KisMemoryStorage::resourceMd5(const QString &url)
{
    QStringList parts = url.split('/', QString::SkipEmptyParts);
    Q_ASSERT(parts.size() == 2);

    const QString resourceType = parts[0];
    const QString resourceFilename = parts[1];

    QByteArray result;

    if (d->resourcesNew.contains(resourceType) &&
        d->resourcesNew[resourceType].contains(resourceFilename)) {

        const StoredResource &storedResource =
            d->resourcesNew[resourceType][resourceFilename];

        result = KoMD5Generator::generateHash(*storedResource.data);
    }

    return result;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisMemoryStorage::resources(const QString &resourceType)
{
    QVector<VersionedResourceEntry> entries;


    QHash<QString, StoredResource> &typedResources =
        d->resourcesNew[resourceType];

    for (auto it = typedResources.begin(); it != typedResources.end(); ++it) {
        VersionedResourceEntry entry;
        entry.filename = it.key();
        entry.lastModified = it.value().timestamp;
        entry.tagList = {}; // TODO
        entry.resourceType = resourceType;
        entries.append(entry);

    }

    KisStorageVersioningHelper::detectFileVersions(entries);

    return toQShared(new KisVersionedStorageIterator(entries, this));
}

QSharedPointer<KisResourceStorage::TagIterator> KisMemoryStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new MemoryTagIterator(d->tags[resourceType], resourceType));
}

void KisMemoryStorage::setMetaData(const QString &key, const QVariant &value)
{
    d->metadata[key] = value;
}

QStringList KisMemoryStorage::metaDataKeys() const
{
    return QStringList() << KisResourceStorage::s_meta_name;
}

QVariant KisMemoryStorage::metaData(const QString &key) const
{
    QVariant r;
    if (d->metadata.contains(key)) {
        r = d->metadata[key];
    }
    return r;
}
