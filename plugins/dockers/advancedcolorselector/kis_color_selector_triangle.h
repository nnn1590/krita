/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef KIS_COLOR_SELECTOR_TRIANGLE_H
#define KIS_COLOR_SELECTOR_TRIANGLE_H

#include "kis_color_selector_component.h"
#include "kis_paint_device.h"
#include <QSize>
#include <QImage>

namespace Acs {
    class PixelCacheRenderer;
}

class KisColorSelectorTriangle : public KisColorSelectorComponent
{
    Q_OBJECT
public:
    explicit KisColorSelectorTriangle(KisColorSelector* parent);
    void setColor(const KoColor &color) override;

protected:
    void paint(QPainter*) override;
    KoColor selectColor(int x, int y) override;
    bool containsPointInComponentCoords(int x, int y) const override;

private:
    friend class Acs::PixelCacheRenderer;
    KoColor colorAt(float x, float y) const;

private:
    int triangleWidth() const;
    int triangleHeight() const;
    void updatePixelCache(qreal devicePixelRatioF);
    QPoint widgetToTriangleCoordinates(const QPoint& point) const;
    QPoint triangleToWidgetCoordinates(const QPoint& point) const;

private:
    QImage m_renderedPixelCache;
    KisPaintDeviceSP m_realPixelCache;
    QSize m_cachedSize;
    QPointF m_lastClickPos;
    qreal m_cacheDevicePixelRatioF {1.0};
};

#endif // KIS_COLOR_SELECTOR_TRIANGLE_H
