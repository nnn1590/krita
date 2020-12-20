/*
    SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef H_KIS_COLOR_H
#define H_KIS_COLOR_H

#include <QtGlobal>
#include <QColor>

#include <kis_display_color_converter.h>
#include <kis_arcs_constants.h>

class KisColor
{
public:
    enum Type { HSY, HSV, HSL, HSI };
    
    KisColor(KisDisplayColorConverter *converter = KisDisplayColorConverter::dumbConverterInstance(),
             Type type=HSY, qreal lR = DEFAULT_LUMA_R, qreal lG = DEFAULT_LUMA_G,
             qreal lB = DEFAULT_LUMA_B, qreal lGamma = DEFAULT_LUMA_GAMMA);

    KisColor(qreal hue, KisDisplayColorConverter *converter, Type type,
             qreal lR = DEFAULT_LUMA_R, qreal lG = DEFAULT_LUMA_G,
             qreal lB = DEFAULT_LUMA_B, qreal lGamma = DEFAULT_LUMA_GAMMA);

    KisColor(const QColor& color, KisDisplayColorConverter* converter, Type type=HSY,
             qreal lR = DEFAULT_LUMA_R, qreal lG = DEFAULT_LUMA_G,
             qreal lB = DEFAULT_LUMA_B, qreal lGamma = DEFAULT_LUMA_GAMMA);

    KisColor(Qt::GlobalColor color, KisDisplayColorConverter* converter, Type type=HSY,
             qreal lR = DEFAULT_LUMA_R, qreal lG = DEFAULT_LUMA_G,
             qreal lB = DEFAULT_LUMA_B, qreal lGamma = DEFAULT_LUMA_GAMMA);

    KisColor(const KisColor& color, KisDisplayColorConverter *converter, Type type = HSY,
             qreal lR = DEFAULT_LUMA_R, qreal lG = DEFAULT_LUMA_G,
             qreal lB = DEFAULT_LUMA_B, qreal lGamma = DEFAULT_LUMA_GAMMA);

    KisColor(const KoColor &color, KisDisplayColorConverter* converter, Type type=HSY,
             qreal lR = DEFAULT_LUMA_R, qreal lG = DEFAULT_LUMA_G,
             qreal lB = DEFAULT_LUMA_B, qreal lGamma = DEFAULT_LUMA_GAMMA);

    ~KisColor();
    
    inline Type getType()     const { return m_type;   }
    inline qreal getH() const { return m_hue; }
    inline qreal getS() const { return m_saturation; }
    inline qreal getX() const { return m_value; }
    inline qreal lumaR() const { return m_lumaR; }
    inline qreal lumaG() const { return m_lumaG; }
    inline qreal lumaB() const { return m_lumaB; }
    inline qreal lumaGamma() const { return m_lumaGamma; }

    inline void setH(qreal v) { m_hue = v; }
    inline void setS(qreal v) { m_saturation = v; }
    inline void setX(qreal v) { m_value = v; }
    
    QColor toQColor() const;
    KoColor toKoColor() const;
    void fromKoColor(const KoColor &color);

    inline void setHSX(qreal h, qreal s, qreal x) {
        m_hue = h;
        m_saturation = s;
        m_value = x;
    }
    
private:
    void initHSX(Type type, qreal h, qreal s, qreal x);
    
private:
    qreal m_hue;
    qreal m_value;
    qreal m_saturation;
    qreal m_lumaR;
    qreal m_lumaG;
    qreal m_lumaB;
    qreal m_lumaGamma;
    KisColor::Type m_type;
    KisDisplayColorConverter* m_colorConverter;
};

#endif // H_KIS_COLOR_H
