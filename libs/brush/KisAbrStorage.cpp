/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisAbrStorage.h"
#include "KisResourceStorage.h"

#include <QFileInfo>

struct KisAbrStorageStaticRegistrar {
    KisAbrStorageStaticRegistrar() {
        KisStoragePluginRegistry::instance()->addStoragePluginFactory(KisResourceStorage::StorageType::AdobeBrushLibrary, new KisStoragePluginFactory<KisAbrStorage>());
    }
};
static KisAbrStorageStaticRegistrar s_registrar;


class AbrTagIterator : public KisResourceStorage::TagIterator
{
public:
    AbrTagIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {}

    bool hasNext() const override {return false; }
    void next() override {}

    QString url() const override { return QString(); }
    QString name() const override { return QString(); }
    QString comment() const override {return QString(); }
    KisTagSP tag() const override { return 0; }
private:

    QString m_location;
    QString m_resourceType;
};

class AbrIterator : public KisResourceStorage::ResourceIterator
{
public:
    KisAbrBrushCollectionSP m_brushCollection;
    QSharedPointer<QMap<QString, KisAbrBrushSP>> m_brushesMap;
    QMap<QString, KisAbrBrushSP>::const_iterator m_brushCollectionIterator;
    KisAbrBrushSP m_currentResource;
    bool isLoaded;
    QString m_currentUrl;


    AbrIterator(KisAbrBrushCollectionSP brushCollection)
        : m_brushCollection(brushCollection)
        , isLoaded(false)
    {
    }

    bool hasNext() const override
    {
        if (!isLoaded) {
            bool success = m_brushCollection->load();
            Q_UNUSED(success); // brush collection will be empty
            const_cast<AbrIterator*>(this)->m_brushesMap = m_brushCollection->brushesMap();
            const_cast<AbrIterator*>(this)->m_brushCollectionIterator = m_brushesMap->constBegin();
            const_cast<AbrIterator*>(this)->isLoaded = true;
        }

        if (m_brushCollectionIterator == m_brushesMap->constEnd()) {
            return false;
        }


        QMap<QString, KisAbrBrushSP>::const_iterator nextIteration = m_brushCollectionIterator;
        nextIteration++;
        bool hasNext = (nextIteration != m_brushesMap->constEnd());
        return hasNext;
    }

    void next() override
    {
        m_brushCollectionIterator++;
        m_currentResource = m_brushCollectionIterator.value();
        m_currentUrl = m_brushCollectionIterator.key();
    }

    QString url() const override { return m_currentUrl; }
    QString type() const override { return ResourceType::Brushes; }
    QDateTime lastModified() const override { return m_brushCollection->lastModified(); }

    KoResourceSP resource() const override
    {
        return m_currentResource;
    }
};

KisAbrStorage::KisAbrStorage(const QString &location)
    : KisStoragePlugin(location)
    , m_brushCollection(new KisAbrBrushCollection(location))
{
}

KisAbrStorage::~KisAbrStorage()
{

}

KisResourceStorage::ResourceItem KisAbrStorage::resourceItem(const QString &url)
{
    KisResourceStorage::ResourceItem item;
    item.url = url;
    // last "_" with index is the suffix added by abr_collection
    int indexOfUnderscore = url.lastIndexOf("_");
    QString filenameUrl = url;
    // filenameUrl contains the name of the collection (filename without .abr, brush name without index)
    filenameUrl.remove(indexOfUnderscore, url.length() - indexOfUnderscore);
    item.folder = filenameUrl;
    item.resourceType = ResourceType::Brushes;
    item.lastModified = QFileInfo(m_brushCollection->filename()).lastModified();
    return item;
}


KoResourceSP KisAbrStorage::resource(const QString &url)
{
    if (!m_brushCollection->isLoaded()) {
        m_brushCollection->load();
    }
    return m_brushCollection->brushByName(url);
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisAbrStorage::resources(const QString &/*resourceType*/)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new AbrIterator(m_brushCollection));
}

QSharedPointer<KisResourceStorage::TagIterator> KisAbrStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new AbrTagIterator(location(), resourceType));
}
