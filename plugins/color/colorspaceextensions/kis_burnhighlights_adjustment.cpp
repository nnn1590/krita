/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 2
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_burnhighlights_adjustment.h"
#include <KoConfig.h>

#include <kis_debug.h>
#include <klocalizedstring.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <KoColorConversions.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceTraits.h>
#include <KoColorTransformation.h>
#include <KoID.h>

template<typename _channel_type_, typename traits>
class KisBurnHighlightsAdjustment : public KoColorTransformation
{
    typedef traits RGBTrait;
    typedef typename RGBTrait::Pixel RGBPixel;

public:
    KisBurnHighlightsAdjustment(){}

 	void transform(const quint8 *srcU8, quint8 *dstU8, qint32 nPixels) const override
 	{
        const RGBPixel* src = reinterpret_cast<const RGBPixel*>(srcU8);
        RGBPixel* dst = reinterpret_cast<RGBPixel*>(dstU8);
        float value_red, value_green, value_blue;
        const float factor(1.0 - exposure * (0.33333));
        while(nPixels > 0) {

            value_red = factor * KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->red);
            value_green = factor * KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->green);
            value_blue = factor * KoColorSpaceMaths<_channel_type_, float>::scaleToA(src->blue);
            
            dst->red = KoColorSpaceMaths< float, _channel_type_>::scaleToA(value_red);
            dst->green = KoColorSpaceMaths< float, _channel_type_ >::scaleToA(value_green);
            dst->blue = KoColorSpaceMaths< float, _channel_type_>::scaleToA(value_blue);
            dst->alpha = src->alpha;
            
            --nPixels;
            ++src;
            ++dst;	
        }
    }

	QList<QString> parameters() const override
	{
        QList<QString> list;
        list << "exposure";
        return list;
	}

	int parameterId(const QString& name) const override
    {
        if (name == "exposure")
        return 0;
        return -1;
    }

    void setParameter(int id, const QVariant& parameter) override
    {
        switch(id)
        {
        case 0:
            exposure = parameter.toDouble();
            break;
        default:
            ;
        }
    }
private:

    float exposure {0.0f};
 };

 KisBurnHighlightsAdjustmentFactory::KisBurnHighlightsAdjustmentFactory()
    : KoColorTransformationFactory("BurnHighlights")
{
}

QList< QPair< KoID, KoID > > KisBurnHighlightsAdjustmentFactory::supportedModels() const
{
    QList< QPair< KoID, KoID > > l;
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer8BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Integer16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float16BitsColorDepthID));
    l.append(QPair< KoID, KoID >(RGBAColorModelID , Float32BitsColorDepthID));
    return l;
}

KoColorTransformation* KisBurnHighlightsAdjustmentFactory::createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const
{
    KoColorTransformation * adj;
    if (colorSpace->colorModelId() != RGBAColorModelID) {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisBurnHighlightsAdjustment::createTransformation";
        return 0;
    }
    if (colorSpace->colorDepthId() == Integer8BitsColorDepthID) {
        adj = new KisBurnHighlightsAdjustment< quint8, KoBgrTraits < quint8 > >();
    }
#ifdef HAVE_OPENEXR
    else if (colorSpace->colorDepthId() == Float16BitsColorDepthID) {
        adj = new KisBurnHighlightsAdjustment< half, KoRgbTraits < half > >();
    }
#endif
    else if (colorSpace->colorDepthId() == Integer16BitsColorDepthID) {
        adj = new KisBurnHighlightsAdjustment< quint16, KoBgrTraits < quint16 > >();
    } else if (colorSpace->colorDepthId() == Float32BitsColorDepthID) {
        adj = new KisBurnHighlightsAdjustment< float, KoRgbTraits < float > >();
    } else {
        dbgKrita << "Unsupported color space " << colorSpace->id() << " in KisBurnHighlightsAdjustment::createTransformation";
        return 0;
    }
    adj->setParameters(parameters);
    return adj;

}
