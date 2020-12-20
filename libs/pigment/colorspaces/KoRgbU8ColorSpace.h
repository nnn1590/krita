/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KORGBU8COLORSPACE_H
#define KORGBU8COLORSPACE_H

#include <QColor>

#include "KoSimpleColorSpace.h"
#include "KoSimpleColorSpaceFactory.h"
#include "KoColorModelStandardIds.h"

struct KoBgrU8Traits;

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a color common to the mask. The default color is white.
 */
class KoRgbU8ColorSpace : public KoSimpleColorSpace<KoBgrU8Traits>
{

public:

    KoRgbU8ColorSpace();

    ~KoRgbU8ColorSpace() override;

    static QString colorSpaceId();

    virtual KoColorSpace* clone() const;

    void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const override;

    void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const override;
    
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;

    void fillGrayBrushWithColorAndLightnessOverlay(quint8 *dst, const QRgb *brush, quint8 *brushColor, qint32 nPixels) const override;
    void fillGrayBrushWithColorAndLightnessWithStrength(quint8* dst, const QRgb* brush, quint8* brushColor, qreal strength, qint32 nPixels) const override;
};

class KoRgbU8ColorSpaceFactory : public KoSimpleColorSpaceFactory
{

public:
    KoRgbU8ColorSpaceFactory()
            : KoSimpleColorSpaceFactory("RGBA",
                                        i18n("RGB (8-bit integer/channel, unmanaged)"),
                                        true,
                                        RGBAColorModelID,
                                        Integer8BitsColorDepthID) {
    }

    KoColorSpace *createColorSpace(const KoColorProfile *) const override {
        return new KoRgbU8ColorSpace();
    }

};

#endif
