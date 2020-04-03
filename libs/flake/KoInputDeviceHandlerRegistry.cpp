/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoInputDeviceHandlerRegistry.h"

#include <QGlobalStatic>

#include <KoPluginLoader.h>
#include "KoInputDeviceHandler.h"

Q_GLOBAL_STATIC(KoInputDeviceHandlerRegistry, s_instance)

KoInputDeviceHandlerRegistry::KoInputDeviceHandlerRegistry()
{
}

void KoInputDeviceHandlerRegistry::init()
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "DevicePlugins";
    config.blacklist = "DevicePluginsDisabled";
    config.group = "krita";
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Device"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"), config);

    Q_FOREACH (const QString & id, keys()) {
        KoInputDeviceHandler * d = value(id);
        if (d)
            d->start();
    }
}

KoInputDeviceHandlerRegistry::~KoInputDeviceHandlerRegistry()
{
    Q_FOREACH (const QString & id, keys()) {
        KoInputDeviceHandler * d = value(id);
        if (d) {
            d->stop();
        }
    }
    Q_FOREACH (const QString &id, keys()) {
        get(id)->deleteLater();
    }
    // just leak on exit -- we get into trouble for explicitly
    // deleting stuff from static objects, like registries
    //qDeleteAll(doubleEntries());
}

KoInputDeviceHandlerRegistry* KoInputDeviceHandlerRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}
