/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
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
#ifndef KIS_VISUAL_TRIANGLE_SELECTOR_SHAPE_H
#define KIS_VISUAL_TRIANGLE_SELECTOR_SHAPE_H

#include "KisVisualColorSelectorShape.h"

class KisVisualTriangleSelectorShape : public KisVisualColorSelectorShape
{
    Q_OBJECT
public:
    explicit KisVisualTriangleSelectorShape(QWidget *parent,
                                            Dimensions dimension,
                                            const KoColorSpace *cs,
                                            int channel1, int channel2,
                                            const KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(),
                                            int margin = 5
            );
    ~KisVisualTriangleSelectorShape() override;

    void setBorderWidth(int /*width*/) override;

    /**
     * @brief getSpaceForSquare
     * @param geom the full widget rectangle
     * @return rectangle with enough space for second widget
     */
    QRect getSpaceForSquare(QRect geom) override;
    QRect getSpaceForCircle(QRect geom) override;
    QRect getSpaceForTriangle(QRect geom) override;

protected:
    QImage renderAlphaMask() const override;

private:

    QPointF convertShapeCoordinateToWidgetCoordinate(QPointF coordinate) const override;
    QPointF convertWidgetCoordinateToShapeCoordinate(QPointF coordinate) const override;

    QRegion getMaskMap() override;
    void drawCursor() override;

    int m_margin { 5 };
};
#endif
