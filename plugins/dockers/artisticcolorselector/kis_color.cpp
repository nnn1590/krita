/*
    SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KoColor.h>
#include "kis_color.h"
#include <kis_arcs_constants.h>


KisColor::KisColor(KisDisplayColorConverter* converter, Type type,
                   qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    m_colorConverter = converter;
    m_type = type;
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;
    m_lumaGamma = lGamma;

    initHSX(type, 0.0, 0.0, 0.0);
}

KisColor::KisColor(qreal hue, KisDisplayColorConverter* converter, Type type,
                   qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    m_colorConverter = converter;
    m_type = type;
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;
    m_lumaGamma = lGamma;

    initHSX(type, hue, 0.0, 0.0);
}

KisColor::KisColor(const QColor& color, KisDisplayColorConverter* converter, Type type,
                   qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    m_colorConverter = converter;
    m_type = type;
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;
    m_lumaGamma = lGamma;

    KoColor koColor = m_colorConverter->approximateFromRenderedQColor(color);
    fromKoColor(koColor);
}

KisColor::KisColor(Qt::GlobalColor color, KisDisplayColorConverter* converter,
                   Type type, qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    QColor c(color);
    m_colorConverter = converter;
    m_type = type;
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;
    m_lumaGamma = lGamma;

    KoColor koColor = m_colorConverter->approximateFromRenderedQColor(c);
    fromKoColor(koColor);
}

KisColor::KisColor(const KoColor &color, KisDisplayColorConverter* converter, KisColor::Type type,
                   qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    m_colorConverter = converter;
    m_type = type;
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;
    m_lumaGamma = lGamma;
    fromKoColor(color);
}

KisColor::KisColor(const KisColor& color, KisDisplayColorConverter* converter, KisColor::Type type,
                   qreal lR, qreal lG, qreal lB, qreal lGamma)
{
    m_colorConverter = converter;
    m_type = type;
    m_lumaR = lR;
    m_lumaG = lG;
    m_lumaB = lB;
    m_lumaGamma = lGamma;
    initHSX(type, color.getH(), color.getS(), color.getX());
}

KisColor::~KisColor()
{
}

QColor KisColor::toQColor() const
{
    return m_colorConverter->toQColor(toKoColor());
}

KoColor KisColor::toKoColor() const
{
    KoColor color;

    switch (m_type) {
    case HSV:
        color = m_colorConverter->fromHsvF(m_hue, m_saturation, m_value);
        break;
    case HSI:
        color = m_colorConverter->fromHsiF(m_hue, m_saturation, m_value);
        break;
    case HSL:
        color = m_colorConverter->fromHslF(m_hue, m_saturation, m_value);
        break;
    case HSY:
        color = m_colorConverter->fromHsyF(m_hue, m_saturation, m_value, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
        break;
    }

    return color;
}

void KisColor::fromKoColor(const KoColor& color)
{
    switch (m_type) {
    case HSV:
        m_colorConverter->getHsvF(color, &m_hue, &m_saturation, &m_value);
        break;
    case HSI:
        m_colorConverter->getHsiF(color, &m_hue, &m_saturation, &m_value);
        break;
    case HSL:
        m_colorConverter->getHslF(color, &m_hue, &m_saturation, &m_value);
        break;
    case HSY:
        m_colorConverter->getHsyF(color, &m_hue, &m_saturation, &m_value, m_lumaR, m_lumaG, m_lumaB, m_lumaGamma);
        break;
    }
}

void KisColor::initHSX(Type type, qreal h, qreal s, qreal x)
{
    m_type = type;
    m_hue = h;
    m_saturation = s;
    m_value = x;
}
