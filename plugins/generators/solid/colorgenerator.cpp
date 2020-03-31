/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "colorgenerator.h"

#include <QPoint>

#include <kis_debug.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <kis_fill_painter.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <generator/kis_generator_registry.h>
#include <kis_global.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_color.h"
#include "ui_wdgcoloroptions.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaColorGeneratorFactory, "kritacolorgenerator.json", registerPlugin<KritaColorGenerator>();)

KritaColorGenerator::KritaColorGenerator(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisColorGenerator());
}

KritaColorGenerator::~KritaColorGenerator()
{
}

KisColorGenerator::KisColorGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Solid Color..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
}

KisFilterConfigurationSP KisColorGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    QVariant v;
    v.setValue(KoColor());
    config->setProperty("color", v);
    return config;
}

KisConfigWidget * KisColorGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgColor(parent);
}

void KisColorGenerator::generate(KisProcessingInformation dstInfo,
                                 const QSize& size,
                                 const KisFilterConfigurationSP config,
                                 KoUpdater* progressUpdater) const
{
    KisPaintDeviceSP dst = dstInfo.paintDevice();

    Q_ASSERT(!dst.isNull());
    Q_ASSERT(config);

    KoColor c;
    if (config) {
        c = config->getColor("color");


        KisFillPainter gc(dst);
        gc.setProgress(progressUpdater);
        gc.setChannelFlags(config->channelFlags());
        gc.setOpacity(100);
        gc.setSelection(dstInfo.selection());
        gc.fillRect(QRect(dstInfo.topLeft(), size), c);
        gc.end();
    }
}

#include "colorgenerator.moc"
