/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_layer_style_filter_projection_plane.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_layer_style_filter.h"
#include "kis_layer_style_filter_environment.h"
#include "kis_psd_layer_style.h"


#include "kis_painter.h"
#include "kis_multiple_projection.h"
#include "KisLayerStyleKnockoutBlower.h"


struct KisLayerStyleFilterProjectionPlane::Private
{
    Private(KisLayer *_sourceLayer)
        : sourceLayer(_sourceLayer),
          environment(new KisLayerStyleFilterEnvironment(_sourceLayer))
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(_sourceLayer);
    }

    Private(const Private &rhs, KisLayer *_sourceLayer, KisPSDLayerStyleSP clonedStyle)
        : sourceLayer(_sourceLayer),
          filter(rhs.filter ? rhs.filter->clone() : 0),
          style(clonedStyle),
          environment(new KisLayerStyleFilterEnvironment(_sourceLayer)),
          knockoutBlower(rhs.knockoutBlower),
          projection(rhs.projection)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(_sourceLayer);
    }


    KisLayer *sourceLayer;

    QScopedPointer<KisLayerStyleFilter> filter;
    KisPSDLayerStyleSP style;
    QScopedPointer<KisLayerStyleFilterEnvironment> environment;
    KisLayerStyleKnockoutBlower knockoutBlower;

    KisMultipleProjection projection;
};

KisLayerStyleFilterProjectionPlane::
KisLayerStyleFilterProjectionPlane(KisLayer *sourceLayer)
    : m_d(new Private(sourceLayer))
{
}

KisLayerStyleFilterProjectionPlane::KisLayerStyleFilterProjectionPlane(const KisLayerStyleFilterProjectionPlane &rhs, KisLayer *sourceLayer, KisPSDLayerStyleSP clonedStyle)
    : m_d(new Private(*rhs.m_d, sourceLayer, clonedStyle))
{
}

KisLayerStyleFilterProjectionPlane::~KisLayerStyleFilterProjectionPlane()
{
}

void KisLayerStyleFilterProjectionPlane::setStyle(KisLayerStyleFilter *filter, KisPSDLayerStyleSP style)
{
    m_d->filter.reset(filter);
    m_d->style = style;
}

QRect KisLayerStyleFilterProjectionPlane::recalculate(const QRect& rect, KisNodeSP filthyNode)
{
    Q_UNUSED(filthyNode);

    if (!m_d->sourceLayer || !m_d->filter) {
        warnKrita << "KisLayerStyleFilterProjectionPlane::recalculate(): [BUG] is not initialized";
        return QRect();
    }

    m_d->projection.clear(rect);
    m_d->filter->processDirectly(m_d->sourceLayer->projection(),
                                 &m_d->projection,
                                 &m_d->knockoutBlower,
                                 rect,
                                 m_d->style,
                                 m_d->environment.data());
    return rect;
}

void KisLayerStyleFilterProjectionPlane::apply(KisPainter *painter, const QRect &rect)
{
    m_d->projection.apply(painter->device(), rect, m_d->environment.data());
}

KisPaintDeviceList KisLayerStyleFilterProjectionPlane::getLodCapableDevices() const
{
    return m_d->projection.getLodCapableDevices();
}

bool KisLayerStyleFilterProjectionPlane::isEmpty() const
{
    return m_d->projection.isEmpty();
}

KisLayerStyleKnockoutBlower *KisLayerStyleFilterProjectionPlane::knockoutBlower() const
{
    return &m_d->knockoutBlower;
}

QRect KisLayerStyleFilterProjectionPlane::needRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    if (!m_d->sourceLayer || !m_d->filter) {
        warnKrita << "KisLayerStyleFilterProjectionPlane::needRect(): [BUG] is not initialized";
        return rect;
    }

    KIS_ASSERT_RECOVER_NOOP(pos == KisLayer::N_ABOVE_FILTHY);
    return m_d->filter->neededRect(rect, m_d->style, m_d->environment.data());
}

QRect KisLayerStyleFilterProjectionPlane::changeRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    if (!m_d->sourceLayer || !m_d->filter) {
        warnKrita << "KisLayerStyleFilterProjectionPlane::changeRect(): [BUG] is not initialized";
        return rect;
    }

    KIS_ASSERT_RECOVER_NOOP(pos == KisLayer::N_ABOVE_FILTHY);
    return m_d->filter->changedRect(rect, m_d->style, m_d->environment.data());
}

QRect KisLayerStyleFilterProjectionPlane::accessRect(const QRect &rect, KisLayer::PositionToFilthy pos) const
{
    return needRect(rect, pos);
}

QRect KisLayerStyleFilterProjectionPlane::needRectForOriginal(const QRect &rect) const
{
    return needRect(rect, KisLayer::N_ABOVE_FILTHY);
}

QRect KisLayerStyleFilterProjectionPlane::tightUserVisibleBounds() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->filter, QRect());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->sourceLayer, QRect());

    return m_d->filter->changedRect(m_d->sourceLayer->exactBounds(),
                                    m_d->style,
                                    m_d->environment.data());
}

