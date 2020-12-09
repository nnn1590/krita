/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2008
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_BRUSH_OPTION_H_
#define KIS_BRUSH_OPTION_H_

#include <kis_brush.h>
#include <KisPaintopPropertiesBase.h>
#include <kis_properties_configuration.h>
#include <kis_threaded_text_rendering_workaround.h>
#include <KisResourcesInterface.h>

#include <kritapaintop_export.h>

class PAINTOP_EXPORT KisBrushOptionProperties : public KisPaintopPropertiesCanvasResourcesBase
{
public:

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override;
    void readOptionSettingResourceImpl(const KisPropertiesConfiguration *setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface) override;
    QList<KoResourceSP> prepareLinkedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const override;
    QList<KoResourceSP> prepareEmbeddedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const override;

    KisBrushSP brush() const;
    void setBrush(KisBrushSP brush);

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
    static bool isTextBrush(const KisPropertiesConfiguration *setting);
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

private:
    KisBrushSP m_brush;
};

#endif
