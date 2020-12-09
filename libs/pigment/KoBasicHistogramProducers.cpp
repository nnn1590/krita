/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KoBasicHistogramProducers.h"

#include <QString>
#include <klocalizedstring.h>

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

// #include "Ko_global.h"
#include "KoIntegerMaths.h"
#include "KoChannelInfo.h"

static const KoColorSpace* m_labCs = 0;


KoBasicHistogramProducer::KoBasicHistogramProducer(const KoID& id, int channelCount, int nrOfBins)
    : m_channels(channelCount)
    , m_nrOfBins(nrOfBins)
    , m_colorSpace(0)
    , m_id(id)
{
    m_bins.resize(m_channels);
    for (int i = 0; i < m_channels; i++)
        m_bins[i].resize(m_nrOfBins);
    m_outLeft.resize(m_channels);
    m_outRight.resize(m_channels);
    m_count = 0;
    m_from = 0.0;
    m_width = 1.0;
}

KoBasicHistogramProducer::KoBasicHistogramProducer(const KoID& id, int nrOfBins, const KoColorSpace *cs)
    : m_nrOfBins(nrOfBins),
      m_colorSpace(cs),
      m_id(id)
{
    Q_ASSERT(cs);
    m_channels = cs->channelCount();

    m_bins.resize(m_channels);
    for (int i = 0; i < m_channels; i++)
        m_bins[i].resize(m_nrOfBins);
    m_outLeft.resize(m_channels);
    m_outRight.resize(m_channels);
    m_count = 0;
    m_from = 0.0;
    m_width = 1.0;
}


void KoBasicHistogramProducer::clear()
{
    m_count = 0;
    for (int i = 0; i < m_channels; i++) {
        for (int j = 0; j < m_nrOfBins; j++) {
            m_bins[i][j] = 0;
        }
        m_outRight[i] = 0;
        m_outLeft[i] = 0;
    }
}

void KoBasicHistogramProducer::makeExternalToInternal()
{
    // This function assumes that the pixel is has no 'gaps'. That is to say: if we start
    // at byte 0, we can get to the end of the pixel by adding consecutive size()s of
    // the channels
    QList<KoChannelInfo *> c = channels();
    uint count = c.count();
    int currentPos = 0;

    for (uint i = 0; i < count; i++) {
        for (uint j = 0; j < count; j++) {
            if (c[j]->pos() == currentPos) {
                m_external.append(j);
                break;
            }
        }
        currentPos += c.at(m_external.at(m_external.count() - 1))->size();
    }
}

// ------------ U8 ---------------------

KoBasicU8HistogramProducer::KoBasicU8HistogramProducer(const KoID& id, const KoColorSpace *cs)
    : KoBasicHistogramProducer(id, 256, cs)
{
}

QString KoBasicU8HistogramProducer::positionToString(qreal pos) const
{
    return QString("%1").arg(static_cast<quint8>(pos * UINT8_MAX));
}

void KoBasicU8HistogramProducer::addRegionToBin(const quint8 * pixels, const quint8 * selectionMask, quint32 nPixels, const KoColorSpace *cs)
{
    quint32 dstPixelSize = m_colorSpace->pixelSize();
    quint8 *dstPixels = new quint8[nPixels * dstPixelSize];
    cs->convertPixelsTo(pixels, dstPixels, m_colorSpace, nPixels, KoColorConversionTransformation::IntentAbsoluteColorimetric, KoColorConversionTransformation::Empty);

    if (selectionMask) {
        quint8 *dst = dstPixels;
        while (nPixels > 0) {
            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8)) {

                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    m_bins[i][m_colorSpace->scaleToU8(dst,i)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            selectionMask++;
            nPixels--;
        }
    } else {
        quint8 *dst = dstPixels;
        while (nPixels > 0) {
            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8)) {

                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    m_bins[i][m_colorSpace->scaleToU8(dst,i)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            nPixels--;
        }
    }
}

// ------------ U16 ---------------------

KoBasicU16HistogramProducer::KoBasicU16HistogramProducer(const KoID& id, const KoColorSpace *cs)
    : KoBasicHistogramProducer(id, 256, cs)
{
}

QString KoBasicU16HistogramProducer::positionToString(qreal pos) const
{
    return QString("%1").arg(static_cast<quint8>(pos * UINT8_MAX));
}

qreal KoBasicU16HistogramProducer::maximalZoom() const
{
    return 1.0 / 255.0;
}

void KoBasicU16HistogramProducer::addRegionToBin(const quint8 * pixels, const quint8 * selectionMask, quint32 nPixels, const KoColorSpace *cs)
{
    // The view
    quint16 from = static_cast<quint16>(m_from * UINT16_MAX);
    quint16 width = static_cast<quint16>(m_width * UINT16_MAX + 0.5); // We include the end
    quint16 to = from + width;
    qreal factor = 255.0 / width;

    quint32 dstPixelSize = m_colorSpace->pixelSize();
    quint8 *dstPixels = new quint8[nPixels * dstPixelSize];
    cs->convertPixelsTo(pixels, dstPixels, m_colorSpace, nPixels, KoColorConversionTransformation::IntentAbsoluteColorimetric, KoColorConversionTransformation::Empty);
    quint8 *dst = dstPixels;
    QVector<float> channels(m_colorSpace->channelCount());

    if (selectionMask) {
        while (nPixels > 0) {
            if (!((m_skipUnselected && *selectionMask == 0) || (m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8))) {
                m_colorSpace->normalisedChannelsValue(dst,channels);
                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    quint16 value = channels[i]*UINT16_MAX;
                    if (value > to)
                        m_outRight[i]++;
                    else if (value < from)
                        m_outLeft[i]++;
                    else
                        m_bins[i][static_cast<quint8>((value - from) * factor)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            selectionMask++;
            nPixels--;
        }
    } else {
        while (nPixels > 0) {
            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8)) {
                m_colorSpace->normalisedChannelsValue(dst,channels);
                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    quint16 value = channels[i]*UINT16_MAX;

                    if (value > to)
                        m_outRight[i]++;
                    else if (value < from)
                        m_outLeft[i]++;
                    else
                        m_bins[i][static_cast<quint8>((value - from) * factor)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            nPixels--;
        }
    }
}

// ------------ Float32 ---------------------
KoBasicF32HistogramProducer::KoBasicF32HistogramProducer(const KoID& id, const KoColorSpace *cs)
    : KoBasicHistogramProducer(id, 256, cs)
{
}

QString KoBasicF32HistogramProducer::positionToString(qreal pos) const
{
    return QString("%1").arg(static_cast<float>(pos)); // XXX I doubt this is correct!
}

qreal KoBasicF32HistogramProducer::maximalZoom() const
{
    // XXX What _is_ the maximal zoom here? I don't think there is one with floats, so this seems a fine compromise for the moment
    return 1.0 / 255.0;
}

void KoBasicF32HistogramProducer::addRegionToBin(const quint8 * pixels, const quint8 * selectionMask, quint32 nPixels, const KoColorSpace *cs)
{
    // The view
    float from = static_cast<float>(m_from);
    float width = static_cast<float>(m_width);
    float to = from + width;
    float factor = 255.0 / width;

    quint32 dstPixelSize = m_colorSpace->pixelSize();
    quint8 *dstPixels = new quint8[nPixels * dstPixelSize];
    cs->convertPixelsTo(pixels, dstPixels, m_colorSpace, nPixels, KoColorConversionTransformation::IntentAbsoluteColorimetric, KoColorConversionTransformation::Empty);
    quint8 *dst = dstPixels;
    QVector<float> channels(m_colorSpace->channelCount());

    if (selectionMask) {
        while (nPixels > 0) {
            if (!((m_skipUnselected && *selectionMask == 0) || (m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8))) {
                m_colorSpace->normalisedChannelsValue(dst,channels);
                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    float value = channels[i];
                    if (value > to)
                        m_outRight[i]++;
                    else if (value < from)
                        m_outLeft[i]++;
                    else
                        m_bins[i][static_cast<quint8>((value - from) * factor)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            selectionMask++;
            nPixels--;

        }
    } else {
        while (nPixels > 0) {
            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8)) {
                m_colorSpace->normalisedChannelsValue(dst,channels);
                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    float value = channels[i];
                    if (value > to)
                        m_outRight[i]++;
                    else if (value < from)
                        m_outLeft[i]++;
                    else
                        m_bins[i][static_cast<quint8>((value - from) * factor)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            nPixels--;

        }
    }
}

#ifdef HAVE_OPENEXR
// ------------ Float16 Half ---------------------
KoBasicF16HalfHistogramProducer::KoBasicF16HalfHistogramProducer(const KoID& id,
                                                                 const KoColorSpace *cs)
    : KoBasicHistogramProducer(id, 256, cs)
{
}

QString KoBasicF16HalfHistogramProducer::positionToString(qreal pos) const
{
    return QString("%1").arg(static_cast<float>(pos)); // XXX I doubt this is correct!
}

qreal KoBasicF16HalfHistogramProducer::maximalZoom() const
{
    // XXX What _is_ the maximal zoom here? I don't think there is one with floats, so this seems a fine compromise for the moment
    return 1.0 / 255.0;
}

void KoBasicF16HalfHistogramProducer::addRegionToBin(const quint8 * pixels, const quint8 * selectionMask, quint32 nPixels, const KoColorSpace *cs)
{
    // The view
    float from = static_cast<float>(m_from);
    float width = static_cast<float>(m_width);
    float to = from + width;
    float factor = 255.0 / width;

    quint32 dstPixelSize = m_colorSpace->pixelSize();
    quint8 *dstPixels = new quint8[nPixels * dstPixelSize];
    cs->convertPixelsTo(pixels, dstPixels, m_colorSpace, nPixels, KoColorConversionTransformation::IntentAbsoluteColorimetric, KoColorConversionTransformation::Empty);
    quint8 *dst = dstPixels;
    QVector<float> channels(m_colorSpace->channelCount());

    if (selectionMask) {
        while (nPixels > 0) {
            if (!((m_skipUnselected  && *selectionMask == 0) || (m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8))) {
                m_colorSpace->normalisedChannelsValue(dst,channels);
                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    float value = channels[i];
                    if (value > to)
                        m_outRight[i]++;
                    else if (value < from)
                        m_outLeft[i]++;
                    else
                        m_bins[i][static_cast<quint8>((value - from) * factor)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            selectionMask++;
            nPixels--;
        }
    } else {
        while (nPixels > 0) {
            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8)) {
                m_colorSpace->normalisedChannelsValue(dst,channels);
                for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
                    float value = channels[i];
                    if (value > to)
                        m_outRight[i]++;
                    else if (value < from)
                        m_outLeft[i]++;
                    else
                        m_bins[i][static_cast<quint8>((value - from) * factor)]++;
                }
                m_count++;
            }
            dst += dstPixelSize;
            nPixels--;
        }
    }
}
#endif

// ------------ Generic RGB ---------------------
KoGenericRGBHistogramProducer::KoGenericRGBHistogramProducer()
    : KoBasicHistogramProducer(KoID("GENRGBHISTO", i18n("Generic RGB Histogram")), 3, 256)
{
    /* we set 0 as colorspece, because we are not based on a specific colorspace. This
       is no problem for the superclass since we override channels() */
    m_channelsList.append(new KoChannelInfo(i18n("R"), 0, 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255, 0, 0)));
    m_channelsList.append(new KoChannelInfo(i18n("G"), 1, 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 255, 0)));
    m_channelsList.append(new KoChannelInfo(i18n("B"), 2, 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 0, 255)));
}

QList<KoChannelInfo *> KoGenericRGBHistogramProducer::channels()
{
    return m_channelsList;
}

QString KoGenericRGBHistogramProducer::positionToString(qreal pos) const
{
    return QString("%1").arg(static_cast<quint8>(pos * UINT8_MAX));
}

qreal KoGenericRGBHistogramProducer::maximalZoom() const
{
    return 1.0;
}


void KoGenericRGBHistogramProducer::addRegionToBin(const quint8 * pixels, const quint8 * selectionMask, quint32 nPixels, const KoColorSpace *cs)
{
    for (int i = 0; i < m_channels; i++) {
        m_outRight[i] = 0;
        m_outLeft[i] = 0;
    }

    QColor c;
    qint32 pSize = cs->pixelSize();
    if (selectionMask) {
        while (nPixels > 0) {
            if (!((m_skipUnselected  && *selectionMask == 0) || (m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8))) {
                cs->toQColor(pixels, &c);
                m_bins[0][c.red()]++;
                m_bins[1][c.green()]++;
                m_bins[2][c.blue()]++;

                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }

    } else {
        while (nPixels > 0) {

            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8)) {
                cs->toQColor(pixels, &c);
                m_bins[0][c.red()]++;
                m_bins[1][c.green()]++;
                m_bins[2][c.blue()]++;

                m_count++;
            }
            pixels += pSize;
            nPixels--;
        }
    }
}

KoGenericRGBHistogramProducerFactory::KoGenericRGBHistogramProducerFactory()
    : KoHistogramProducerFactory(KoID("GENRGBHISTO", i18n("Generic RGB Histogram")))
{
}

// ------------ Generic L*a*b* ---------------------
KoGenericLabHistogramProducer::KoGenericLabHistogramProducer()
    : KoBasicHistogramProducer(KoID("GENLABHISTO", i18n("L*a*b* Histogram")), 3, 256)
{
    /* we set 0 as colorspace, because we are not based on a specific colorspace. This
       is no problem for the superclass since we override channels() */
    m_channelsList.append(new KoChannelInfo(i18n("L*"), 0, 0, KoChannelInfo::COLOR, KoChannelInfo::UINT8));
    m_channelsList.append(new KoChannelInfo(i18n("a*"), 1, 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8));
    m_channelsList.append(new KoChannelInfo(i18n("b*"), 2, 2, KoChannelInfo::COLOR, KoChannelInfo::UINT8));

    if (!m_labCs) {
        m_labCs = KoColorSpaceRegistry::instance()->lab16();
    }
    m_colorSpace = m_labCs;
}
KoGenericLabHistogramProducer::~KoGenericLabHistogramProducer()
{
    delete m_channelsList[0];
    delete m_channelsList[1];
    delete m_channelsList[2];
}

QList<KoChannelInfo *> KoGenericLabHistogramProducer::channels()
{
    return m_channelsList;
}

QString KoGenericLabHistogramProducer::positionToString(qreal pos) const
{
    return QString("%1").arg(static_cast<quint16>(pos * UINT16_MAX));
}

qreal KoGenericLabHistogramProducer::maximalZoom() const
{
    return 1.0;
}


void KoGenericLabHistogramProducer::addRegionToBin(const quint8 *pixels, const quint8 *selectionMask, quint32 nPixels,  const KoColorSpace *cs)
{
    for (int i = 0; i < m_channels; i++) {
        m_outRight[i] = 0;
        m_outLeft[i] = 0;
    }

    qint32 dstPixelSize = m_colorSpace->pixelSize();

    quint8 *dstPixels = new quint8[nPixels * dstPixelSize];
    cs->convertPixelsTo(pixels, dstPixels, m_colorSpace, nPixels, KoColorConversionTransformation::IntentAbsoluteColorimetric, KoColorConversionTransformation::Empty);

    qint32 pSize = cs->pixelSize();

    if (selectionMask) {
        while (nPixels > 0) {
            if (!((m_skipUnselected  && *selectionMask == 0) || (m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8))) {
                m_count++;
            }
            pixels += pSize;
            selectionMask++;
            nPixels--;
        }
    } else {
        quint8 *dst = dstPixels;
        while (nPixels > 0) {
            if (!(m_skipTransparent && cs->opacityU8(pixels) == OPACITY_TRANSPARENT_U8))  {

                m_bins[0][m_colorSpace->scaleToU8(dst, 0)]++;
                m_bins[1][m_colorSpace->scaleToU8(dst, 1)]++;
                m_bins[2][m_colorSpace->scaleToU8(dst, 2)]++;

                m_count++;
            }
            dst+= dstPixelSize;
            nPixels--;
        }
    }
    delete[] dstPixels;
}

KoGenericLabHistogramProducerFactory::KoGenericLabHistogramProducerFactory()
    : KoHistogramProducerFactory(KoID("GENLABHISTO", i18n("Generic L*a*b* Histogram")))
{
}
