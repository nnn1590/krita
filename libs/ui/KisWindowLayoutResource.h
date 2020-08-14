/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KISWINDOWLAYOUTRESOURCE_H
#define KISWINDOWLAYOUTRESOURCE_H

#include <KoResource.h>
#include <KisMainWindow.h>

class KisWindowLayoutResource;
typedef QSharedPointer<KisWindowLayoutResource> KisWindowLayoutResourceSP;

#include "kritaui_export.h"

class KRITAUI_EXPORT KisWindowLayoutResource : public KoResource
{
public:
    explicit KisWindowLayoutResource(const QString &filename);
    ~KisWindowLayoutResource() override;
    KisWindowLayoutResource(const KisWindowLayoutResource &rhs);
    KisWindowLayoutResource &operator=(const KisWindowLayoutResource &rhs) = delete;
    KoResourceSP clone() const override;

    static KisWindowLayoutResourceSP fromCurrentWindows (
            const QString &filename, const QList<QPointer<KisMainWindow>> &mainWindows,
            bool showImageInAllWindows,
            bool primaryWorkspaceFollowsFocus,
            KisMainWindow *primaryWindow
            );

    void applyLayout();

    bool saveToDevice(QIODevice *dev) const override;
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;


    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::WindowLayouts, "");
    }

    QString defaultFileExtension() const override;

protected:
    void setWindows(const QList<QPointer<KisMainWindow>> &mainWindows);

    virtual void saveXml(QDomDocument &doc, QDomElement &root) const;

    virtual void loadXml(const QDomElement &root) const;

private:
    struct Private;

    QScopedPointer<Private> d;
};


#endif
