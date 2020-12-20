/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISRESOURCELOADER_H
#define KISRESOURCELOADER_H

#include <QString>
#include <QStringList>
#include <QImage>
#include <QScopedPointer>
#include <QSharedPointer>

#include <KoResource.h>
#include <KoID.h>

#include <kritaresources_export.h>


/**
 * @brief The KisResourceLoader class is an abstract interface
 * class that must be implemented by actual resource classes and
 * registered with the KisResourceLoaderRegistry.
 */
class KRITARESOURCES_EXPORT KisResourceLoaderBase
{
public:

    KisResourceLoaderBase(const QString &resourceSubType, const QString &resourceType, const QString &name, const QStringList &mimetypes)
    {
        m_resourceSubType = resourceSubType;
        m_resourceType = resourceType;
        m_mimetypes = mimetypes;
        m_name = name;
    }

    virtual ~KisResourceLoaderBase()
    {
    }

    /**
     * @return a set of filters ("*.bla,*.foo") that is suitable for filtering
     * the contents of a directory.
     */
    QStringList filters() const;

    /**
     * @return the mimetypes this resource can load
     */
    QStringList mimetypes() const
    {
        return m_mimetypes;
    }

    /**
     * @return the folder in the resource storage where resources
     * of this type are located
     */
    QString resourceType() const
    {
        return m_resourceType;
    }

    QString resourceSubType() const
    {
        return id();
    }

    /// For registration in KisResourceLoaderRegistry
    QString id() const
    {
        return m_resourceSubType;
    }

    /// The user-friendly name of the category
    QString name() const
    {
        return m_name;
    }

    /**
     * Load this resource.
     * @return a resource if loading the resource succeeded, 0 otherwise
     */
    virtual KoResourceSP load(const QString &name, QIODevice &dev, KisResourcesInterfaceSP resourcesInterface) { Q_UNUSED(name); Q_UNUSED(dev); Q_UNUSED(resourcesInterface); return 0; }

private:
    QString m_resourceSubType;
    QString m_resourceType;
    QStringList m_mimetypes;
    QString m_name;

};

template<typename T>
class KisResourceLoader : public KisResourceLoaderBase {
public:
    KisResourceLoader(const QString &id, const QString &folder, const QString &name, const QStringList &mimetypes)
        : KisResourceLoaderBase(id, folder, name, mimetypes)
    {
    }

    KoResourceSP load(const QString &name, QIODevice &dev, KisResourcesInterfaceSP resourcesInterface) override
    {
        QSharedPointer<T> resource = QSharedPointer<T>::create(name);
        Q_ASSERT(dev.isOpen() && dev.isReadable());
        if (resource->loadFromDevice(&dev, resourcesInterface)) {
            return resource;
        }
        return 0;
    }
};



#endif // KISRESOURCELOADER_H
