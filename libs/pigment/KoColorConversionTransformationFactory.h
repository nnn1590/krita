/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_CONVERSION_TRANSFORMATION_FACTORY_H_
#define _KO_COLOR_CONVERSION_TRANSFORMATION_FACTORY_H_

class KoColorSpace;
class KoColorConversionTransformation;

#include <QString>
#include <KoColorConversionTransformationAbstractFactory.h>

/**
 * Factory to create a color transformation between two colorsapces.
 */
class KRITAPIGMENT_EXPORT KoColorConversionTransformationFactory : public KoColorConversionTransformationAbstractFactory
{
public:
    /**
     * Create a color conversion transformation factory, that can create
     * KoColorConversionTransformation object between a source colorspace
     * and a destination colorspace.
     *
     * @param _srcModelId id for the source color model
     * @param _srcDepthId id for the source depth
     * @param _dstModelId id for the destination color model
     * @param _dstDepthId id for the destination depth
     * @param _srcProfile name of the source profile, or empty if any profile
     * @param _dstProfile name of the destination profile, or empty if any profile
     */
    KoColorConversionTransformationFactory(const QString &_srcModelId, const QString &_srcDepthId, const QString &_srcProfile, const QString &_dstModelId, const QString &_dstDepthId, const QString &_dstProfile);
    ~KoColorConversionTransformationFactory() override;
    /**
     * @return true if this factory creates a color conversion transformation which
     * conserve color information (typical color transformation that lose that information
     * is anything to grayscale).
     */
    virtual bool conserveColorInformation() const = 0;
    /**
     * @return true if this factory creates a color conversion transformation which
     * conserve the dynamic range of the color.
     */
    virtual bool conserveDynamicRange() const = 0;
public:
    /**
     * @return the id of the source color model
     */
    QString srcColorModelId() const;
    /**
     * @return the id of the source color depth
     */
    QString srcColorDepthId() const;
    /**
     * @return the name of the source profile (note that an empty name
     *         means all profiles can be used)
     */
    QString srcProfile() const;
    /**
     * @return the id of the destination color model
     */
    QString dstColorModelId() const;
    /**
     * @return the id of the destination color depth
     */
    QString dstColorDepthId() const;
    /**
     * @return the name of the destination profile (note that an empty name
     *         means all profiles can be used)
     */
    QString dstProfile() const;
protected:
    /**
     * @param srcCS source color space
     * @return true if the color space given as argument can be used as a source colorspace
     */
    bool canBeSource(const KoColorSpace* srcCS) const;
    /**
     * @param dstCS destination color space
     * @return true if the color space given as argument can be used as a destination colorspace
     */
    bool canBeDestination(const KoColorSpace* dstCS) const;
private:
    struct Private;
    Private* const d;
};

#endif
