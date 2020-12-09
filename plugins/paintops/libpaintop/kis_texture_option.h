/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2012
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_TEXTURE_OPTION_H
#define KIS_TEXTURE_OPTION_H

#include <kritapaintop_export.h>

#include <kis_paint_device.h>
#include <kis_types.h>
#include "kis_paintop_option.h"
#include "kis_pressure_texture_strength_option.h"
#include <resources/KoAbstractGradient.h>
#include <resources/KoCachedGradient.h>

#include "KisTextureMaskInfo.h"

#include <QRect>

class KisTextureChooser;
class KoPattern;
class KoResource;
class KisPropertiesConfiguration;
class KisPaintopLodLimitations;
class KisResourcesInterface;

class PAINTOP_EXPORT KisTextureOption : public KisPaintOpOption
{
    Q_OBJECT
public:

    explicit KisTextureOption();
    ~KisTextureOption() override;

public Q_SLOTS:

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

private Q_SLOTS:

    void resetGUI(KoResourceSP ); /// called when a new pattern is selected


private:
    /// UI Widget that stores all the texture options
    KisTextureChooser* m_textureOptions;

};

class PAINTOP_EXPORT KisTextureProperties
{
public:
    KisTextureProperties(int levelOfDetail);

    enum TexturingMode {
        MULTIPLY,
        SUBTRACT,
        LIGHTNESS,
        GRADIENT
    };

    bool m_enabled;

    /**
     * @brief apply combine the texture map with the dab
     * @param dab the colored, final representation of the dab, after mirroring and everything.
     * @param offset the position of the dab on the image. used to calculate the position of the mask pattern
     * @param info the paint information
     */
    void apply(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation & info);
    void fillProperties(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);
    QList<KoResourceSP> prepareEmbeddedResources(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);
    bool applyingGradient() const;

    static bool applyingGradient(const KisPropertiesConfiguration *settings);

private:

    void applyLightness(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info);
    void applyGradient(KisFixedPaintDeviceSP dab, const QPoint& offset, const KisPaintInformation& info);

private:

    int m_offsetX;
    int m_offsetY;
    TexturingMode m_texturingMode;
    KoAbstractGradientSP m_gradient;
    KoCachedGradient m_cachedGradient;

    int m_levelOfDetail;

private:
    KisPressureTextureStrengthOption m_strengthOption;
    KisTextureMaskInfoSP m_maskInfo;
};

#endif // KIS_TEXTURE_OPTION_H
