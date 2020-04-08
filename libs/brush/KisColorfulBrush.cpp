/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisColorfulBrush.h"


KisColorfulBrush::KisColorfulBrush(const QString &filename)
    : KisScalingSizeBrush(filename)
{
}

void KisColorfulBrush::setUseColorAsMask(bool useColorAsMask)
{
    /**
     * WARNING: There is a problem in the brush server, since it
     * returns not copies of brushes, but direct pointers to them. It
     * means that the brushes are shared among all the currently
     * present paintops, which might be a problem for e.g. Multihand
     * Brush Tool.
     *
     * Right now, all the instances of Multihand Brush Tool share the
     * same brush, so there is no problem in this sharing, unless we
     * reset the internal state of the brush on our way.
     */

    if (useColorAsMask != m_useColorAsMask) {
        m_useColorAsMask = useColorAsMask;
        resetBoundary();
        clearBrushPyramid();
    }
}

bool KisColorfulBrush::useColorAsMask() const
{
    return m_useColorAsMask;
}

#include <KoColorSpaceMaths.h>

QImage KisColorfulBrush::brushTipImage() const
{
    QImage image = KisBrush::brushTipImage();
    if (hasColor() && useColorAsMask()) {
        if (m_adjustmentMidPoint != 127 ||
            !qFuzzyIsNull(m_brightnessAdjustment) ||
            !qFuzzyIsNull(m_contrastAdjustment)) {

            const int half = KoColorSpaceMathsTraits<quint8>::halfValue;
            const int unit = KoColorSpaceMathsTraits<quint8>::unitValue;

            const qreal midX = m_adjustmentMidPoint;
            const qreal midY = m_brightnessAdjustment > 0 ?
                        KoColorSpaceMaths<qreal>::blend(unit, half, m_brightnessAdjustment) :
                        KoColorSpaceMaths<qreal>::blend(0, half, -m_brightnessAdjustment);

            qreal loA = 0.0;
            qreal hiA = 0.0;

            qreal loB = 0.0;
            qreal hiB = 255.0;

            if (!qFuzzyCompare(m_contrastAdjustment, 1.0)) {
                loA = midY / (1.0 - m_contrastAdjustment) / midX;
                hiA = (unit - midY) / (1.0 - m_contrastAdjustment) / (unit - midX);

                loB = midY - midX * loA;
                hiB = midY - midX * hiA;
            }

            for (int y = 0; y < image.height(); y++) {
                QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++) {
                    QRgb c = pixel[x];

                    int v = qGray(c);

                    if (v >= midX) {
                        v = qMin(unit, qRound(hiA * v + hiB));
                    } else {
                        v = qMax(0, qRound(loA * v + loB));
                    }

                    pixel[x] = qRgba(v, v, v, qAlpha(c));
                }
            }
        } else {
            for (int y = 0; y < image.height(); y++) {
                QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(y));
                for (int x = 0; x < image.width(); x++) {
                    QRgb c = pixel[x];

                    int v = qGray(c);
                    pixel[x] = qRgba(v, v, v, qAlpha(c));
                }
            }
        }
    }
    return image;
}

void KisColorfulBrush::setAdjustmentMidPoint(quint8 value)
{
    m_adjustmentMidPoint = value;
}

void KisColorfulBrush::setBrightnessAdjustment(qreal value)
{
    m_brightnessAdjustment = value;
}

void KisColorfulBrush::setContrastAdjustment(qreal value)
{
    m_contrastAdjustment = value;
}

quint8 KisColorfulBrush::adjustmentMidPoint() const
{
    return m_adjustmentMidPoint;
}

qreal KisColorfulBrush::brightnessAdjustment() const
{
    return m_brightnessAdjustment;
}

qreal KisColorfulBrush::contrastAdjustment() const
{
    return m_contrastAdjustment;
}

#include <QDomElement>

void KisColorfulBrush::toXML(QDomDocument& d, QDomElement& e) const
{
    e.setAttribute("ColorAsMask", QString::number((int)useColorAsMask()));
    e.setAttribute("AdjustmentMidPoint", QString::number(m_adjustmentMidPoint));
    e.setAttribute("BrightnessAdjustment", QString::number(m_brightnessAdjustment));
    e.setAttribute("ContrastAdjustment", QString::number(m_contrastAdjustment));
    KisBrush::toXML(d, e);
}
