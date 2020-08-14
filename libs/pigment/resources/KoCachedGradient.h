/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2019 Miguel Lopez <reptillia39@live.com>
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


class KoCachedGradient : public KoAbstractGradient
{

public:
    KoCachedGradient() : KoAbstractGradient("")
    {
    }

    KoCachedGradient(const KoAbstractGradientSP gradient, qint32 steps, const KoColorSpace* cs)
        : KoAbstractGradient(gradient->filename())
    {
        setGradient(gradient, steps, cs);
    }

    ~KoCachedGradient() override {}

    KoResourceSP clone() const override {
        return KoResourceSP(new KoCachedGradient(m_subject, m_max + 1, m_colorSpace));
    }

    /**
    * Creates a QGradient from the gradient.
    * The resulting QGradient might differ from original gradient
    */
    QGradient* toQGradient() const override
    {
        return m_subject->toQGradient();
    }

    void setGradient(const KoAbstractGradientSP gradient, qint32 steps, const KoColorSpace* cs) {
        m_subject = gradient;
        m_max = steps - 1;
        m_colorSpace = cs;
        m_colors.clear();

        m_black = KoColor(cs);

        KoColor tmpColor(m_colorSpace);
        for (qint32 i = 0; i < steps; i++) {
            m_subject->colorAt(tmpColor, qreal(i) / m_max);
            m_colors << tmpColor;
        }
    }

    void setGradient(const KoAbstractGradientSP gradient, qint32 steps) {
        setGradient(gradient, steps, gradient->colorSpace());
    }

    /// gets the color data at position 0 <= t <= 1
    const quint8* cachedAt(qreal t) const
    {
        qint32 tInt = t * m_max + 0.5;
        if (m_colors.size() > tInt) {
            return m_colors[tInt].data();
        }
        else {
            return m_black.data();
        }
    }

    /// allow access to the actual color at position 0 <= t <= 1, instead of the cached color
    void colorAt(KoColor& color, qreal t) const override
    {
        m_subject->colorAt(color, t);
    }
    void setColorSpace(const KoColorSpace* colorSpace) 
    { 
        if (!m_colorSpace || *m_colorSpace != *colorSpace) {
            m_colorSpace = colorSpace;
            for (qint32 i = 0; i < m_colors.size(); i++) {
                m_colors[i].convertTo(m_colorSpace);
            }
        }
    }
    const KoColorSpace* colorSpace() const { return m_colorSpace; }

    KoAbstractGradientSP gradient() { return m_subject; }

    QByteArray generateMD5() const override { return QByteArray(); }

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override {
        return m_subject->loadFromDevice(dev, resourcesInterface);
    }

    QPair<QString, QString> resourceType() const override {
        return m_subject->resourceType();
    }

private:

    KoAbstractGradientSP m_subject;
    const KoColorSpace* m_colorSpace = 0;
    qint32 m_max = 0;
    QVector<KoColor> m_colors;
    KoColor m_black;
};
