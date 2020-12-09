/*
 * Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asccdl_filter.h"
#include "kis_wdg_asccdl.h"
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_color_transformation_configuration.h>
#include <qmath.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaASCCDLFactory,
                           "kritaasccdl.json",
                           registerPlugin<KritaASCCDL>();)


KritaASCCDL::KritaASCCDL(QObject *parent, const QVariantList &) : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisFilterASCCDL()));
}

KritaASCCDL::~KritaASCCDL()
{

}

KisFilterASCCDL::KisFilterASCCDL(): KisColorTransformationFilter(id(), FiltersCategoryAdjustId, i18n("&Slope, Offset, Power..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setSupportsThreading(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setShowConfigurationWidget(true);
}

KoColorTransformation *KisFilterASCCDL::createTransformation(const KoColorSpace *cs,
                                                             const KisFilterConfigurationSP config) const
{
    KoColor black(Qt::black, cs);
    KoColor white(Qt::white, cs);
    return new KisASCCDLTransformation(cs,
                                       config->getColor("slope", black),
                                       config->getColor("offset", white),
                                       config->getColor("power", black));
}

KisConfigWidget *KisFilterASCCDL::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool) const
{
    return new KisASCCDLConfigWidget(parent, dev->colorSpace());
}

bool KisFilterASCCDL::needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const
{
    KoColor black(Qt::black, cs);
    KoColor offset = config->getColor("offset", black);
    offset.convertTo(cs);
    if (cs->difference(black.data(), offset.data())>0) {
        return true;
    }
    return false;
}

KisFilterConfigurationSP KisFilterASCCDL::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    QVariant colorVariant("KoColor");
    KoColor black;
    black.fromQColor(QColor(Qt::black));
    KoColor white;
    white.fromQColor(QColor(Qt::white));
    colorVariant.setValue(white);
    config->setProperty( "slope", colorVariant);
    config->setProperty( "power", colorVariant);
    colorVariant.setValue(black);
    config->setProperty("offset", colorVariant);
    return config;
}

KisASCCDLTransformation::KisASCCDLTransformation(const KoColorSpace *cs, KoColor slope, KoColor offset, KoColor power)
{
    QVector<float> slopeN(cs->channelCount());
    slope.convertTo(cs);
    slope.colorSpace()->normalisedChannelsValue(slope.data(), slopeN);
    m_slope = slopeN;
    offset.convertTo(cs);
    QVector<float> offsetN(cs->channelCount());
    offset.colorSpace()->normalisedChannelsValue(offset.data(), offsetN);
    m_offset = offsetN;
    power.convertTo(cs);
    QVector<float> powerN(cs->channelCount());
    power.colorSpace()->normalisedChannelsValue(power.data(), powerN);
    m_power = powerN;
    m_cs = cs;
}

void KisASCCDLTransformation::transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
{
    QVector<float> normalised(m_cs->channelCount());
    const int pixelSize = m_cs->pixelSize();
    while (nPixels--) {
        m_cs->normalisedChannelsValue(src, normalised);

        for (uint c=0; c<m_cs->channelCount(); c++){
            if (m_cs->channels().at(c)->channelType()!=KoChannelInfo::ALPHA) {
                normalised[c] = qPow( (normalised.at(c)*m_slope.at(c))+m_offset.at(c), m_power.at(c));
            }
        }
        m_cs->fromNormalisedChannelsValue(dst, normalised);
        src += pixelSize;
        dst += pixelSize;
    }
}

#include "kis_asccdl_filter.moc"
