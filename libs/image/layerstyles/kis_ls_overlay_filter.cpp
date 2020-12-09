/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_ls_overlay_filter.h"

#include <cstdlib>

#include <QBitArray>

#include <resources/KoPattern.h>

#include <resources/KoAbstractGradient.h>

#include "psd.h"

#include "kis_convolution_kernel.h"
#include "kis_convolution_painter.h"
#include "kis_gaussian_kernel.h"

#include "kis_pixel_selection.h"
#include "kis_fill_painter.h"
#include "kis_gradient_painter.h"
#include "kis_iterator_ng.h"
#include "kis_random_accessor_ng.h"

#include "kis_psd_layer_style.h"
#include "kis_layer_style_filter_environment.h"

#include "kis_ls_utils.h"
#include "kis_multiple_projection.h"



KisLsOverlayFilter::KisLsOverlayFilter(Mode mode)
    : KisLayerStyleFilter(KoID("lsoverlay", i18n("Overlay (style)"))),
      m_mode(mode)
{
}

KisLsOverlayFilter::KisLsOverlayFilter(const KisLsOverlayFilter &rhs)
    : KisLayerStyleFilter(rhs),
      m_mode(rhs.m_mode)
{
}

KisLayerStyleFilter *KisLsOverlayFilter::clone() const
{
    return new KisLsOverlayFilter(*this);
}

void KisLsOverlayFilter::applyOverlay(KisPaintDeviceSP srcDevice,
                                      KisMultipleProjection *dst,
                                      const QRect &applyRect,
                                      const psd_layer_effects_overlay_base *config,
                                      KisLayerStyleFilterEnvironment *env) const
{
    if (applyRect.isEmpty()) return;

    const QString compositeOp = config->blendMode();
    const quint8 opacityU8 = quint8(qRound(255.0 / 100.0 * config->opacity()));

    KisPaintDeviceSP dstDevice = dst->getProjection(KisMultipleProjection::defaultProjectionId(),
                                                    compositeOp,
                                                    opacityU8,
                                                    QBitArray(),
                                                    srcDevice);

    KisLsUtils::fillOverlayDevice(dstDevice, applyRect, config, env);
}

const psd_layer_effects_overlay_base*
KisLsOverlayFilter::getOverlayStruct(KisPSDLayerStyleSP style) const
{
    const psd_layer_effects_overlay_base *config = 0;

    if (m_mode == Color) {
        config = style->colorOverlay();
    } else if (m_mode == Gradient) {
        config = style->gradientOverlay();
    } else if (m_mode == Pattern) {
        config = style->patternOverlay();
    }

    return config;
}

void KisLsOverlayFilter::processDirectly(KisPaintDeviceSP src,
                                         KisMultipleProjection *dst,
                                         KisLayerStyleKnockoutBlower *blower,
                                         const QRect &applyRect,
                                         KisPSDLayerStyleSP style,
                                         KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(env);
    Q_UNUSED(blower);
    KIS_ASSERT_RECOVER_RETURN(style);

    const psd_layer_effects_overlay_base *config = getOverlayStruct(style);
    if (!KisLsUtils::checkEffectEnabled(config, dst)) return;

    applyOverlay(src, dst, applyRect, config, env);
}

QRect KisLsOverlayFilter::neededRect(const QRect &rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(style);
    Q_UNUSED(env);
    return rect;
}

QRect KisLsOverlayFilter::changedRect(const QRect &rect, KisPSDLayerStyleSP style, KisLayerStyleFilterEnvironment *env) const
{
    Q_UNUSED(style);
    Q_UNUSED(env);
    return rect;
}
