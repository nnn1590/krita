/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOCOLORSPACEABSTRACT_H
#define KOCOLORSPACEABSTRACT_H

#include <QBitArray>
#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include "KoFallBackColorTransformation.h"
#include "KoLabDarkenColorTransformation.h"
#include "KoMixColorsOpImpl.h"

#include "KoConvolutionOpImpl.h"
#include "KoInvertColorTransformation.h"
#include "KoAlphaMaskApplicatorFactory.h"
#include "KoColorModelStandardIdsUtils.h"

/**
 * This in an implementation of KoColorSpace which can be used as a base for colorspaces with as many
 * different channels of the same type.
 *
 * The template parameters must be a class which inherits KoColorSpaceTrait (or a class with the same signature).
 *
 * SOMETYPE is the type of the channel for instance (quint8, quint32...),
 * SOMENBOFCHANNELS is the number of channels including the alpha channel
 * SOMEALPHAPOS is the position of the alpha channel in the pixel (can be equal to -1 if no alpha channel).
 */
template<class _CSTrait>
class KoColorSpaceAbstract : public KoColorSpace
{
public:
    typedef _CSTrait ColorSpaceTraits;

public:
    KoColorSpaceAbstract(const QString &id, const QString &name)
        : KoColorSpace(id, name, new KoMixColorsOpImpl< _CSTrait>(), new KoConvolutionOpImpl< _CSTrait>()),
          m_alphaMaskApplicator(KoAlphaMaskApplicatorFactory::create(colorDepthIdForChannelType<typename _CSTrait::channels_type>(), _CSTrait::channels_nb, _CSTrait::alpha_pos))
    {
    }

    quint32 colorChannelCount() const override {
        if (_CSTrait::alpha_pos == -1)
            return _CSTrait::channels_nb;
        else
            return _CSTrait::channels_nb - 1;
    }

    quint32 channelCount() const override {
        return _CSTrait::channels_nb;
    }

    quint32 alphaPos() const override {
        return _CSTrait::alpha_pos;
    }


    quint32 pixelSize() const override {
        return _CSTrait::pixelSize;
    }

    QString channelValueText(const quint8 *pixel, quint32 channelIndex) const override {
        return _CSTrait::channelValueText(pixel, channelIndex);
    }

    QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const override {
        return _CSTrait::normalisedChannelValueText(pixel, channelIndex);
    }

    void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const override {
        return _CSTrait::normalisedChannelsValue(pixel, channels);
    }

    void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const override {
        return _CSTrait::fromNormalisedChannelsValue(pixel, values);
    }

    quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const override {
        typename _CSTrait::channels_type c = _CSTrait::nativeArray(srcPixel)[channelIndex];
        return KoColorSpaceMaths<typename _CSTrait::channels_type, quint8>::scaleToA(c);
    }

    void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const override {
        _CSTrait::singleChannelPixel(dstPixel, srcPixel, channelIndex);
    }

    quint8 opacityU8(const quint8 * U8_pixel) const override {
        return _CSTrait::opacityU8(U8_pixel);
    }

    qreal opacityF(const quint8 * U8_pixel) const override {
        return _CSTrait::opacityF(U8_pixel);
    }

    void setOpacity(quint8 * pixels, quint8 alpha, qint32 nPixels) const override {
        _CSTrait::setOpacity(pixels, alpha, nPixels);
    }

    void setOpacity(quint8 * pixels, qreal alpha, qint32 nPixels) const override {
        _CSTrait::setOpacity(pixels, alpha, nPixels);
    }

    void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const override {
        _CSTrait::multiplyAlpha(pixels, alpha, nPixels);
    }

    void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const override {
        _CSTrait::applyAlphaU8Mask(pixels, alpha, nPixels);
    }

    void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const override {
        _CSTrait::applyInverseAlphaU8Mask(pixels, alpha, nPixels);
    }

    void applyAlphaNormedFloatMask(quint8 * pixels, const float * alpha, qint32 nPixels) const override {
        _CSTrait::applyAlphaNormedFloatMask(pixels, alpha, nPixels);
    }

    void applyInverseNormedFloatMask(quint8 * pixels, const float * alpha, qint32 nPixels) const override {
        m_alphaMaskApplicator->applyInverseNormedFloatMask(pixels, alpha, nPixels);
    }

    void fillInverseAlphaNormedFloatMaskWithColor(quint8 * pixels, const float * alpha, const quint8 *brushColor, qint32 nPixels) const override {
        m_alphaMaskApplicator->fillInverseAlphaNormedFloatMaskWithColor(pixels, alpha, brushColor, nPixels);
    }

    void fillGrayBrushWithColor(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const override {
        m_alphaMaskApplicator->fillGrayBrushWithColor(dst, brush, brushColor, nPixels);
    }

    quint8 intensity8(const quint8 * src) const override {
        QColor c;
        const_cast<KoColorSpaceAbstract<_CSTrait> *>(this)->toQColor(src, &c);
        return static_cast<quint8>(c.red() * 0.30 + c.green() * 0.59 + c.blue() * 0.11);
    }

    KoColorTransformation* createInvertTransformation() const override {
        return KoInvertColorTransformation::getTransformator(this);
    }

    KoColorTransformation *createDarkenAdjustment(qint32 shade, bool compensate, qreal compensation) const override {
        return new KoFallBackColorTransformation(this, KoColorSpaceRegistry::instance()->lab16(""), new KoLabDarkenColorTransformation<quint16>(shade, compensate, compensation, KoColorSpaceRegistry::instance()->lab16("")));
    }

    bool convertPixelsTo(const quint8 *src,
                                 quint8 *dst, const KoColorSpace *dstColorSpace,
                                 quint32 numPixels,
                                 KoColorConversionTransformation::Intent renderingIntent,
                                 KoColorConversionTransformation::ConversionFlags conversionFlags) const override
    {

        // check whether we have the same profile and color model, but only a different bit
        // depth; in that case we don't convert as such, but scale
        bool scaleOnly = false;

        // Note: getting the id() is really, really expensive, so only do that if
        // we are sure there is a difference between the colorspaces
        if (!(*this == *dstColorSpace)) {
            scaleOnly = dstColorSpace->colorModelId().id() == colorModelId().id() &&
                         dstColorSpace->colorDepthId().id() != colorDepthId().id() &&
                         dstColorSpace->profile()->name()   == profile()->name();
        }

        if (scaleOnly && dynamic_cast<const KoColorSpaceAbstract*>(dstColorSpace)) {
            typedef typename _CSTrait::channels_type channels_type;

            switch(dstColorSpace->channels()[0]->channelValueType())
            {
            case KoChannelInfo::UINT8:
                scalePixels<_CSTrait::pixelSize, 1, channels_type, quint8>(src, dst, numPixels);
                return true;
//             case KoChannelInfo::INT8:
//                 scalePixels<_CSTrait::pixelSize, 1, channels_type, qint8>(src, dst, numPixels);
//                 return true;
            case KoChannelInfo::UINT16:
                scalePixels<_CSTrait::pixelSize, 2, channels_type, quint16>(src, dst, numPixels);
                return true;
            case KoChannelInfo::INT16:
                scalePixels<_CSTrait::pixelSize, 2, channels_type, qint16>(src, dst, numPixels);
                return true;
            case KoChannelInfo::UINT32:
                scalePixels<_CSTrait::pixelSize, 4, channels_type, quint32>(src, dst, numPixels);
                return true;
            default:
                break;
            }
        }

        return KoColorSpace::convertPixelsTo(src, dst, dstColorSpace, numPixels, renderingIntent, conversionFlags);
    }

    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const override
    {
        qint32 selectedChannelPos = this->channels()[selectedChannelIndex]->pos();
        for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
            for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
                KoChannelInfo *channel = this->channels().at(channelIndex);
                qint32 channelSize = channel->size();
                if (channel->channelType() == KoChannelInfo::COLOR) {
                    memcpy(dst + (pixelIndex * _CSTrait::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * _CSTrait::pixelSize) + selectedChannelPos, channelSize);
                } else if (channel->channelType() == KoChannelInfo::ALPHA) {
                    memcpy(dst + (pixelIndex * _CSTrait::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * _CSTrait::pixelSize) + (channelIndex * channelSize), channelSize);
                }
            }
        }
    }

    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const override
    {
        for (uint pixelIndex = 0; pixelIndex < nPixels; ++pixelIndex) {
            for (uint channelIndex = 0; channelIndex < this->channelCount(); ++channelIndex) {
                KoChannelInfo *channel = this->channels().at(channelIndex);
                qint32 channelSize = channel->size();
                if (selectedChannels.testBit(channelIndex)) {
                    memcpy(dst + (pixelIndex * _CSTrait::pixelSize) + (channelIndex * channelSize), src + (pixelIndex * _CSTrait::pixelSize) + (channelIndex * channelSize), channelSize);
                } else {
                    reinterpret_cast<typename _CSTrait::channels_type *>(dst + (pixelIndex * _CSTrait::pixelSize) + (channelIndex * channelSize))[0] = _CSTrait::math_trait::zeroValue;
                }
            }
        }
    }

private:
    template<int srcPixelSize, int dstChannelSize, class TSrcChannel, class TDstChannel>
    void scalePixels(const quint8* src, quint8* dst, quint32 numPixels) const {
        qint32 dstPixelSize = dstChannelSize * _CSTrait::channels_nb;

        for(quint32 i=0; i<numPixels; ++i) {
            const TSrcChannel* srcPixel = reinterpret_cast<const TSrcChannel*>(src + i * srcPixelSize);
            TDstChannel*       dstPixel = reinterpret_cast<TDstChannel*>(dst + i * dstPixelSize);

            for(quint32 c=0; c<_CSTrait::channels_nb; ++c)
                dstPixel[c] = Arithmetic::scale<TDstChannel>(srcPixel[c]);
        }
    }

private:
    QScopedPointer<KoAlphaMaskApplicatorBase> m_alphaMaskApplicator;
};

#endif // KOCOLORSPACEABSTRACT_H
