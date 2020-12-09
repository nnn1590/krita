/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KisTagFilterResourceProxyModel.h"

#include <QDebug>

#include <KisResourceModelProvider.h>
#include <KisResourceModel.h>
#include <KisTagResourceModel.h>
#include <KisTagModel.h>

#include <kis_debug.h>
#include <KisResourceSearchBoxFilter.h>

struct KisTagFilterResourceProxyModel::Private
{
    Private()
        : filter(new KisResourceSearchBoxFilter())
    {
    }

    QString resourceType;

    KisResourceModel *resourceModel {0}; // This is the source model if we are _not_ filtering by tag
    KisTagResourceModel *tagResourceModel {0}; // This is the source model if we are filtering by tag

    QScopedPointer<KisResourceSearchBoxFilter> filter;
    bool filterInCurrentTag {false};

    QMap<QString, QVariant> metaDataMapFilter;

};

KisTagFilterResourceProxyModel::KisTagFilterResourceProxyModel(const QString &resourceType, QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    d->resourceType = resourceType;
    d->resourceModel = new KisResourceModel(resourceType);
    d->tagResourceModel = new KisTagResourceModel(resourceType);

    setSourceModel(d->resourceModel);
}

KisTagFilterResourceProxyModel::~KisTagFilterResourceProxyModel()
{
    delete d->resourceModel;
    delete d->tagResourceModel;
    delete d;
}

void KisTagFilterResourceProxyModel::setResourceModel(KisResourceModel *resourceModel)
{
    d->resourceModel = resourceModel;
}

KoResourceSP KisTagFilterResourceProxyModel::resourceForIndex(QModelIndex index) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->resourceForIndex(mapToSource(index));
    }
    return 0;
}

QModelIndex KisTagFilterResourceProxyModel::indexForResource(KoResourceSP resource) const
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return mapFromSource(source->indexForResource(resource));
    }
    return QModelIndex();
}

bool KisTagFilterResourceProxyModel::setResourceInactive(const QModelIndex &index)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceInactive(mapToSource(index));
    }
    return false;
}

bool KisTagFilterResourceProxyModel::importResourceFile(const QString &filename)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->importResourceFile(filename);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::addResource(KoResourceSP resource, const QString &storageId)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->addResource(resource, storageId);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::updateResource(KoResourceSP resource)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->updateResource(resource);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::renameResource(KoResourceSP resource, const QString &name)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->renameResource(resource, name);
    }
    return false;
}

bool KisTagFilterResourceProxyModel::setResourceMetaData(KoResourceSP resource, QMap<QString, QVariant> metadata)
{
    KisAbstractResourceModel *source = dynamic_cast<KisAbstractResourceModel*>(sourceModel());
    if (source) {
        return source->setResourceMetaData(resource, metadata);
    }
    return false;
}

void KisTagFilterResourceProxyModel::setMetaDataFilter(QMap<QString, QVariant> metaDataMap)
{
    d->metaDataMapFilter = metaDataMap;
    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setTagFilter(const KisTagSP tag)
{
    if (!tag || tag->url() == "All") {
        d->tagResourceModel->setTagsFilter(QVector<KisTagSP>());
        setSourceModel(d->resourceModel);
        d->resourceModel->showOnlyUntaggedResources(false);
    }
    else {
        if (tag->url() == "All Untagged") {
            setSourceModel(d->resourceModel);
            d->resourceModel->showOnlyUntaggedResources(true);
        }
        else {
            setSourceModel(d->tagResourceModel);
            d->tagResourceModel->setTagsFilter(QVector<KisTagSP>() << tag);
        }
    }
    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setResourceFilter(const KoResourceSP resource)
{
    if (resource) {
        d->tagResourceModel->setResourcesFilter(QVector<KoResourceSP>() << resource);
        setSourceModel(d->tagResourceModel);
    }
    else {
        setSourceModel(d->resourceModel);
    }
    invalidateFilter();

}

void KisTagFilterResourceProxyModel::setSearchText(const QString& searchText)
{
    d->filter->setFilter(searchText);
    invalidateFilter();
}

void KisTagFilterResourceProxyModel::setFilterInCurrentTag(bool filterInCurrentTag)
{
    d->filterInCurrentTag = filterInCurrentTag;
    invalidateFilter();
}

bool KisTagFilterResourceProxyModel::tagResource(KisTagSP tag, KoResourceSP resource)
{
    return d->tagResourceModel->tagResource(tag, resource);
}

bool KisTagFilterResourceProxyModel::untagResource(const KisTagSP tag, const KoResourceSP resource)
{
    return d->tagResourceModel->untagResource(tag, resource);
}

bool KisTagFilterResourceProxyModel::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{
    return true;
}

bool KisTagFilterResourceProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // we don't need to check anything else because the user wants to search in all resources
    // but if the filter text is empty, we do need to filter by the current tag
    if (!d->filterInCurrentTag && d->filter->isEmpty() && d->metaDataMapFilter.isEmpty()) {
        return true;
    }

    // If there's a tag set to filter on, we use the tagResourceModel, so that already filters for the tag
    // Here, we only have to filter by the search string.
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    if (!idx.isValid()) {
        return false;
    }

    bool metaDataMatches = true;
    QMap<QString, QVariant> resourceMetaData = sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::MetaData).toMap();
    Q_FOREACH(const QString &key, d->metaDataMapFilter.keys()) {
        if (resourceMetaData.contains(key)) {
            metaDataMatches = (resourceMetaData[key] != d->metaDataMapFilter[key]);
        }
    }

    QString resourceName = sourceModel()->data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    bool resourceNameMatches = d->filter->matchesResource(resourceName);


    return (resourceNameMatches && metaDataMatches);
}

bool KisTagFilterResourceProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString nameLeft = sourceModel()->data(source_left, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    QString nameRight = sourceModel()->data(source_right, Qt::UserRole + KisAbstractResourceModel::Name).toString();
    return nameLeft < nameRight;
}

