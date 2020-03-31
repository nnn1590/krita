/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "KisResourceServerProvider.h"

#include <QDir>
#include <QApplication>
#include <QGlobalStatic>

#include <kis_debug.h>

#include <KoResourcePaths.h>

#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <resources/KoPattern.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_workspace_resource.h>
#include <KisWindowLayoutResource.h>
#include <KisSessionResource.h>

#include <kis_psd_layer_style.h>

Q_GLOBAL_STATIC(KisResourceServerProvider, s_instance)

typedef KoResourceServer<KisPaintOpPreset> KisPaintOpPresetResourceServer;
typedef KoResourceServer<KisPSDLayerStyle> KisPSDLayerStyleServer;

KisResourceServerProvider::KisResourceServerProvider()
{
    m_paintOpPresetServer = new KisPaintOpPresetResourceServer(ResourceType::PaintOpPresets);
    m_workspaceServer = new KoResourceServer<KisWorkspaceResource>(ResourceType::Workspaces);
    m_windowLayoutServer = new KoResourceServer<KisWindowLayoutResource>(ResourceType::WindowLayouts);
    m_sessionServer = new KoResourceServer<KisSessionResource>(ResourceType::Sessions);
    m_layerStyleServer = new KisPSDLayerStyleServer(ResourceType::LayerStyles);
}

KisResourceServerProvider::~KisResourceServerProvider()
{
    delete m_paintOpPresetServer;
    delete m_workspaceServer;
    delete m_sessionServer;
    delete m_windowLayoutServer;
    delete m_layerStyleServer;
}

KisResourceServerProvider* KisResourceServerProvider::instance()
{
    return s_instance;
}


KisPaintOpPresetResourceServer* KisResourceServerProvider::paintOpPresetServer()
{
    return m_paintOpPresetServer;
}

KoResourceServer< KisWorkspaceResource >* KisResourceServerProvider::workspaceServer()
{
    return m_workspaceServer;
}

KoResourceServer< KisWindowLayoutResource >* KisResourceServerProvider::windowLayoutServer()
{
    return m_windowLayoutServer;
}

KoResourceServer< KisSessionResource >* KisResourceServerProvider::sessionServer()
{
    return m_sessionServer;
}

KoResourceServer<KisPSDLayerStyle> *KisResourceServerProvider::layerStyleServer()
{
    return m_layerStyleServer;
}

