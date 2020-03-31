/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "Filter.h"

#include <KoCanvasResourceProvider.h>

#include <kis_canvas_resource_provider.h>
#include <kis_filter.h>
#include <kis_properties_configuration.h>
#include <kis_filter_configuration.h>
#include <kis_filter_manager.h>
#include <kis_filter_registry.h>
#include <KisPart.h>
#include <KisView.h>

#include <strokes/kis_filter_stroke_strategy.h>
#include <krita_utils.h>
#include <KisGlobalResourcesInterface.h>

#include "Krita.h"
#include "Document.h"
#include "InfoObject.h"
#include "Node.h"

struct Filter::Private {
    Private() {}
    QString name;
    InfoObject *configuration {0};
};

Filter::Filter()
    : QObject(0)
    , d(new Private)
{
}

Filter::~Filter()
{
    delete d->configuration;
    delete d;
}

bool Filter::operator==(const Filter &other) const
{
    return (d->name == other.d->name
            && d->configuration == other.d->configuration);
}

bool Filter::operator!=(const Filter &other) const
{
    return !(operator==(other));
}


QString Filter::name() const
{
    return d->name;
}

void Filter::setName(const QString &name)
{
    d->name = name;
    delete d->configuration;

    KisFilterSP filter = KisFilterRegistry::instance()->value(d->name);
    d->configuration = new InfoObject(filter->defaultConfiguration(KisGlobalResourcesInterface::instance()));
}

InfoObject* Filter::configuration() const
{
    return d->configuration;
}

void Filter::setConfiguration(InfoObject* value)
{
    d->configuration = value;
}

bool Filter::apply(Node *node, int x, int y, int w, int h)
{
    if (node->locked()) return false;

    KisFilterSP filter = KisFilterRegistry::instance()->value(d->name);
    if (!filter) return false;

    KisPaintDeviceSP dev = node->paintDevice();
    if (!dev) return false;

    QRect applyRect = QRect(x, y, w, h);
    KisFilterConfigurationSP config = static_cast<KisFilterConfiguration*>(d->configuration->configuration().data());
    filter->process(dev, applyRect, config);
    return true;
}

bool Filter::startFilter(Node *node, int x, int y, int w, int h)
{
    if (node->locked()) return false;

    KisFilterSP filter = KisFilterRegistry::instance()->value(d->name);
    if (!filter) return false;

    KisImageWSP image = node->image();
    if (!image) return false;

    KisFilterConfigurationSP filterConfig = static_cast<KisFilterConfiguration*>(d->configuration->configuration().data());

    image->waitForDone();
    QRect initialApplyRect = QRect(x, y, w, h);

    QRect applyRect = initialApplyRect;

    KisPaintDeviceSP paintDevice = node->paintDevice();
    if (paintDevice && filter->needsTransparentPixels(filterConfig.data(), paintDevice->colorSpace())) {
        applyRect |= image->bounds();
    }

    KisResourcesSnapshotSP resources = new KisResourcesSnapshot(image, node->node());

    Document *document = Krita::instance()->activeDocument();
    if (document && KisPart::instance()->viewCount(document->document()) > 0) {
        Q_FOREACH (QPointer<KisView> view, KisPart::instance()->views()) {
            if (view && view->document() == document->document()) {
                resources = new KisResourcesSnapshot(image, node->node(), view->resourceProvider()->resourceManager());
                break;
            }
        }
    }
    delete document;

    KisStrokeId currentStrokeId = image->startStroke(new KisFilterStrokeStrategy(filter,
                                                                                 KisFilterConfigurationSP(filterConfig),
                                                                                 resources));

    QRect processRect = filter->changedRect(applyRect, filterConfig.data(), 0);
    processRect &= image->bounds();

    if (filter->supportsThreading()) {
        QSize size = KritaUtils::optimalPatchSize();
        QVector<QRect> rects = KritaUtils::splitRectIntoPatches(processRect, size);
        Q_FOREACH (const QRect &rc, rects) {
            image->addJob(currentStrokeId, new KisFilterStrokeStrategy::Data(rc, true));
        }
    } else {
        image->addJob(currentStrokeId, new KisFilterStrokeStrategy::Data(processRect, false));
    }

    image->endStroke(currentStrokeId);
    image->waitForDone();

    return true;
}

KisFilterConfigurationSP Filter::filterConfig()
{
    KisFilterConfigurationSP config = KisFilterRegistry::instance()->get(d->name)->factoryConfiguration(KisGlobalResourcesInterface::instance());
    Q_FOREACH(const QString property, d->configuration->properties().keys()) {
        config->setProperty(property, d->configuration->property(property));
    }
    return config;
}
