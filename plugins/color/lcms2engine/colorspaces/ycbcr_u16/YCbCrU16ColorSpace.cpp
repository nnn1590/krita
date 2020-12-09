/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "YCbCrU16ColorSpace.h"
#include <QDomElement>

#include <QDebug>
#include <klocalizedstring.h>

#include "compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

YCbCrU16ColorSpace::YCbCrU16ColorSpace(const QString &name, KoColorProfile *p)
    : LcmsColorSpace<KoYCbCrU16Traits>(colorSpaceId(), name, TYPE_YCbCrA_16, cmsSigXYZData, p)
{
    addChannel(new KoChannelInfo(i18n("Y"),     KoYCbCrU16Traits::Y_pos     * sizeof(quint16), KoYCbCrU16Traits::Y_pos,     KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::cyan));
    addChannel(new KoChannelInfo(i18n("Cb"),    KoYCbCrU16Traits::Cb_pos    * sizeof(quint16), KoYCbCrU16Traits::Cb_pos,    KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::magenta));
    addChannel(new KoChannelInfo(i18n("Cr"),    KoYCbCrU16Traits::Cr_pos    * sizeof(quint16), KoYCbCrU16Traits::Cr_pos,    KoChannelInfo::COLOR, KoChannelInfo::UINT16, sizeof(quint16), Qt::yellow));
    addChannel(new KoChannelInfo(i18n("Alpha"), KoYCbCrU16Traits::alpha_pos * sizeof(quint16), KoYCbCrU16Traits::alpha_pos, KoChannelInfo::ALPHA, KoChannelInfo::UINT16, sizeof(quint16)));

    init();

    addStandardCompositeOps<KoYCbCrU16Traits>(this);
}

bool YCbCrU16ColorSpace::willDegrade(ColorSpaceIndependence independence) const
{
    if (independence == TO_RGBA8) {
        return true;
    } else {
        return false;
    }
}

KoColorSpace *YCbCrU16ColorSpace::clone() const
{
    return new YCbCrU16ColorSpace(name(), profile()->clone());
}

void YCbCrU16ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoYCbCrU16Traits::Pixel *p = reinterpret_cast<const KoYCbCrU16Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("YCbCr");
    labElt.setAttribute("Y",  KisDomUtils::toString(KoColorSpaceMaths< KoYCbCrU16Traits::channels_type, qreal>::scaleToA(p->Y)));
    labElt.setAttribute("Cb", KisDomUtils::toString(KoColorSpaceMaths< KoYCbCrU16Traits::channels_type, qreal>::scaleToA(p->Cb)));
    labElt.setAttribute("Cr", KisDomUtils::toString(KoColorSpaceMaths< KoYCbCrU16Traits::channels_type, qreal>::scaleToA(p->Cr)));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void YCbCrU16ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoYCbCrU16Traits::Pixel *p = reinterpret_cast<KoYCbCrU16Traits::Pixel *>(pixel);
    p->Y = KoColorSpaceMaths< qreal, KoYCbCrU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("Y")));
    p->Cb = KoColorSpaceMaths< qreal, KoYCbCrU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("Cb")));
    p->Cr = KoColorSpaceMaths< qreal, KoYCbCrU16Traits::channels_type >::scaleToA(KisDomUtils::toDouble(elt.attribute("Cr")));
    p->alpha = KoColorSpaceMathsTraits<quint16>::max;
}

void YCbCrU16ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> YCbCrU16ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}
void YCbCrU16ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *u=channelValues[1];
    *v=channelValues[2];
}

QVector <double> YCbCrU16ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*u;
    channelValues[2]=*v;
    channelValues[3]=1.0;
    return channelValues;
}
