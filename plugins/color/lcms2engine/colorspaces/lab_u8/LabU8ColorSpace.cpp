/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "LabU8ColorSpace.h"

#include <QDomElement>

#include <klocalizedstring.h>

#include "../compositeops/KoCompositeOps.h"
#include <KoColorConversions.h>
#include <kis_dom_utils.h>

LabU8ColorSpace::LabU8ColorSpace(const QString &name, KoColorProfile *p) :
    LcmsColorSpace<KoLabU8Traits>(colorSpaceId(), name, TYPE_LABA_8, cmsSigLabData, p)
{
    addChannel(new KoChannelInfo(i18nc("Lightness value in Lab color model", "Lightness"), 0 * sizeof(quint8), 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(100, 100, 100)));
    addChannel(new KoChannelInfo(i18n("a*"),        1 * sizeof(quint8), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(150, 150, 150)));
    addChannel(new KoChannelInfo(i18n("b*"),        2 * sizeof(quint8), 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, sizeof(quint8), QColor(200, 200, 200)));
    addChannel(new KoChannelInfo(i18n("Alpha"),     3 * sizeof(quint8), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8, sizeof(quint8)));
    init();
    addStandardCompositeOps<KoLabU8Traits>(this);
}

bool LabU8ColorSpace::willDegrade(ColorSpaceIndependence /*independence*/) const
{
    return false;
}

KoColorSpace *LabU8ColorSpace::clone() const
{
    return new LabU8ColorSpace(name(), profile()->clone());
}

void LabU8ColorSpace::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    const KoLabU8Traits::Pixel *p = reinterpret_cast<const KoLabU8Traits::Pixel *>(pixel);
    QDomElement labElt = doc.createElement("Lab");

    double a, b;

    if (p->a <= KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB) {
        a = (p->a - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB) /
            (2.0 * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB));
    } else {
        a = 0.5 +
            (p->a - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB) /
                (2.0 * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::unitValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB));
    }

    if (p->b <= KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB) {
        b = (p->b - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB) /
            (2.0 * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB));
    } else {
        b = 0.5 +
            (p->b - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB) /
                (2.0 * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::unitValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB));
    }

    labElt.setAttribute("L", KisDomUtils::toString(KoColorSpaceMaths<KoLabU8Traits::channels_type, qreal>::scaleToA(p->L)));
    labElt.setAttribute("a", KisDomUtils::toString(a));
    labElt.setAttribute("b", KisDomUtils::toString(b));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void LabU8ColorSpace::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    KoLabU8Traits::Pixel *p = reinterpret_cast<KoLabU8Traits::Pixel *>(pixel);

    double a = KisDomUtils::toDouble(elt.attribute("a"));
    double b = KisDomUtils::toDouble(elt.attribute("b"));

    p->L = KoColorSpaceMaths<qreal, KoLabU8Traits::channels_type>::scaleToA(KisDomUtils::toDouble(elt.attribute("L")));

    if (a <= 0.5) {
        p->a =
            KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB + 2.0 * a * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB);
    } else {
        p->a = (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB +
                2.0 * (a - 0.5) * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::unitValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB));
    }

    if (b <= 0.5) {
        p->b =
            KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB + 2.0 * b * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::zeroValueAB);
    } else {
        p->b = (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB +
                2.0 * (b - 0.5) * (KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::unitValueAB - KoLabColorSpaceMathsTraits<KoLabU8Traits::channels_type>::halfValueAB));
    }

    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

void LabU8ColorSpace::toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const
{
    LabToLCH(channelValues[0],channelValues[1],channelValues[2], luma, sat, hue);
}

QVector <double> LabU8ColorSpace::fromHSY(qreal *hue, qreal *sat, qreal *luma) const
{
    QVector <double> channelValues(4);
    LCHToLab(*luma, *sat, *hue, &channelValues[0],&channelValues[1],&channelValues[2]);
    channelValues[3]=1.0;
    return channelValues;
}

void LabU8ColorSpace::toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const
{
    *y =channelValues[0];
    *u=channelValues[1];
    *v=channelValues[2];
}

QVector <double> LabU8ColorSpace::fromYUV(qreal *y, qreal *u, qreal *v) const
{
    QVector <double> channelValues(4);
    channelValues[0]=*y;
    channelValues[1]=*u;
    channelValues[2]=*v;
    channelValues[3]=1.0;
    return channelValues;
}

quint8 LabU8ColorSpace::scaleToU8(const quint8 *srcPixel, qint32 channelIndex) const
{
    typename ColorSpaceTraits::channels_type c = ColorSpaceTraits::nativeArray(srcPixel)[channelIndex];
    qreal b = 0;
    switch (channelIndex) {
    case ColorSpaceTraits::L_pos:
        b = ((qreal)c) / ColorSpaceTraits::math_trait::unitValueL;
        break;
    case ColorSpaceTraits::a_pos:
    case ColorSpaceTraits::b_pos:
        if (c <= ColorSpaceTraits::math_trait::halfValueAB) {
            b = ((qreal)c - ColorSpaceTraits::math_trait::zeroValueAB) / (2.0 * (ColorSpaceTraits::math_trait::halfValueAB - ColorSpaceTraits::math_trait::zeroValueAB));
        } else {
            b = 0.5 + ((qreal)c - ColorSpaceTraits::math_trait::halfValueAB) / (2.0 * (ColorSpaceTraits::math_trait::unitValueAB - ColorSpaceTraits::math_trait::halfValueAB));
        }
        break;
    default:
        b = ((qreal)c) / ColorSpaceTraits::math_trait::unitValue;
        break;
    }

    return KoColorSpaceMaths<qreal, quint8>::scaleToA(b);
}

void LabU8ColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const
{
    for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
        for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
            KoChannelInfo *channel = this->channels().at(channelIndex);
            qint32 channelSize = channel->size();
            if (channel->channelType() == KoChannelInfo::COLOR) {
                if (channelIndex == ColorSpaceTraits::L_pos) {
                    ColorSpaceTraits::channels_type c = ColorSpaceTraits::parent::nativeArray((src + (pixelIndex * ColorSpaceTraits::pixelSize)))[selectedChannelIndex];
                    switch (selectedChannelIndex) {
                    case ColorSpaceTraits::L_pos:
                        break;
                    case ColorSpaceTraits::a_pos:
                    case ColorSpaceTraits::b_pos:
                        if (c <= ColorSpaceTraits::math_trait::halfValueAB) {
                            c = ColorSpaceTraits::math_trait::unitValueL * (((qreal)c - ColorSpaceTraits::math_trait::zeroValueAB) / (2.0 * (ColorSpaceTraits::math_trait::halfValueAB - ColorSpaceTraits::math_trait::zeroValueAB)));
                        } else {
                            c = ColorSpaceTraits::math_trait::unitValueL * (0.5 + ((qreal)c - ColorSpaceTraits::math_trait::halfValueAB) / (2.0 * (ColorSpaceTraits::math_trait::unitValueAB - ColorSpaceTraits::math_trait::halfValueAB)));
                        }
                        break;
                    // As per KoChannelInfo alpha channels are [0..1]
                    default:
                        c = ColorSpaceTraits::math_trait::unitValueL * (qreal)c / ColorSpaceTraits::math_trait::unitValue;
                        break;
                    }
                    ColorSpaceTraits::parent::nativeArray(dst + (pixelIndex * ColorSpaceTraits::pixelSize))[channelIndex] = c;
                } else {
                    ColorSpaceTraits::parent::nativeArray(dst + (pixelIndex * ColorSpaceTraits::pixelSize))[channelIndex] = ColorSpaceTraits::math_trait::halfValueAB;
                }
            } else if (channel->channelType() == KoChannelInfo::ALPHA) {
                memcpy(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), channelSize);
            }
        }
    }
}

void LabU8ColorSpace::convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const
{
    for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
        for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
            KoChannelInfo *channel = this->channels().at(channelIndex);
            qint32 channelSize = channel->size();
            if (selectedChannels.testBit(channelIndex)) {
                memcpy(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize), channelSize);
            } else {
                ColorSpaceTraits::channels_type v;
                switch (channelIndex) {
                case ColorSpaceTraits::L_pos:
                    v = ColorSpaceTraits::math_trait::halfValueL;
                    break;
                case ColorSpaceTraits::a_pos:
                case ColorSpaceTraits::b_pos:
                    v = ColorSpaceTraits::math_trait::halfValueAB;
                    break;
                default:
                    v = ColorSpaceTraits::math_trait::zeroValue;
                    break;
                }
                reinterpret_cast<ColorSpaceTraits::channels_type *>(dst + (pixelIndex * ColorSpaceTraits::pixelSize) + (channelIndex * channelSize))[0] = v;
            }
        }
    }
}
