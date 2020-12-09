/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef LabF32ColorSpace_H_
#define LabF32ColorSpace_H_

#include "LcmsColorSpace.h"
#include "KoColorModelStandardIds.h"

// XXX: implement normalizedChannelValues?

struct KoLabF32Traits;

class LabF32ColorSpace : public LcmsColorSpace<KoLabF32Traits>
{
public:
    LabF32ColorSpace(const QString &name, KoColorProfile *p);

    bool willDegrade(ColorSpaceIndependence independence) const override;

    static QString colorSpaceId()
    {
        return QString("LABAF32");
    }

    KoID colorModelId() const override
    {
        return LABAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Float32BitsColorDepthID;
    }

    virtual KoColorSpace *clone() const;

    void colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const override;

    void colorFromXML(quint8* pixel, const QDomElement& elt) const override;
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;
    quint8 scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const override;
    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const qint32 selectedChannelIndex) const override;
    void convertChannelToVisualRepresentation(const quint8 *src, quint8 *dst, quint32 nPixels, const QBitArray selectedChannels) const override;

    bool hasHighDynamicRange() const override
    {
        return true;
    }
};

class LabF32ColorSpaceFactory : public LcmsColorSpaceFactory
{
public:
    LabF32ColorSpaceFactory()
        : LcmsColorSpaceFactory(TYPE_LabA_FLT, cmsSigLabData)
    {
    }

    bool userVisible() const override
    {
        return true;
    }

    QString id() const override
    {
        return LabF32ColorSpace::colorSpaceId();
    }

    QString name() const override
    {
        return QString("%1 (%2)").arg(LABAColorModelID.name()).arg(Float32BitsColorDepthID.name());
    }

    KoID colorModelId() const override
    {
        return LABAColorModelID;
    }

    KoID colorDepthId() const override
    {
        return Float32BitsColorDepthID;

    }
    int referenceDepth() const override
    {
        return 32;
    }

    KoColorSpace *createColorSpace(const KoColorProfile *p) const override
    {
        return new LabF32ColorSpace(name(), p->clone());
    }

    QString defaultProfile() const override
    {
        return "Lab identity built-in";
    }

    bool isHdr() const override
    {
        return true;
    }
};

#endif
