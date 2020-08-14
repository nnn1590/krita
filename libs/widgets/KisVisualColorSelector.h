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
#ifndef KIS_VISUAL_COLOR_SELECTOR_H
#define KIS_VISUAL_COLOR_SELECTOR_H

#include <QWidget>
#include <QScopedPointer>

#include <KoColor.h>

#include "KisColorSelectorConfiguration.h"
#include "KisColorSelectorInterface.h"
#include "kritawidgets_export.h"

class KoColorSpace;
class KoColorDisplayRendererInterface;

/**
 * @brief The KisVisualColorSelector class
 *
 * This gives a color selector box that draws gradients and everything.
 *
 * Unlike other color selectors, this one draws the full gamut of the given
 * colorspace.
 */
class KRITAWIDGETS_EXPORT KisVisualColorSelector : public KisColorSelectorInterface
{
    Q_OBJECT
public:
    enum ColorModel { None, Channel, HSV, HSL, HSI, HSY, YUV };

    explicit KisVisualColorSelector(QWidget *parent = 0);
    ~KisVisualColorSelector() override;

    /**
     * @brief setConfig
     * @param forceCircular
     * Force circular is for space where you only have room for a circular selector.
     * @param forceSelfUpdate
     * ignored, can possibly be removed from parent class now
     */
    void setConfig(bool forceCircular, bool forceSelfUpdate) override;
    void setAcceptTabletEvents(bool on);
    KoColor getCurrentColor() const override;
    QVector4D getChannelValues() const;
    ColorModel getColorModel() const;
    bool isHSXModel() const;
    KoColor convertShapeCoordsToKoColor(const QVector4D &coordinates) const;
    QVector4D convertKoColorToShapeCoordinates(KoColor c) const;

public Q_SLOTS:

    void slotSetColor(const KoColor &c) override;
    void slotSetColorSpace(const KoColorSpace *cs) override;
    void slotSetHSX(const QVector3D &hsx);
    void configurationChanged();
    void setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer) override;

private Q_SLOTS:
    void slotCursorMoved(QPointF pos);
    void slotDisplayConfigurationChanged();
    void slotRebuildSelectors();

Q_SIGNALS:
    /**
     * @brief sigColorModelChanged is emitted whenever the selector's color model changes.
     *
     * This is mostly relevant for configuration changes where the same RGB model
     * gets represented in a different way like HSV, HSL etc. so the values of
     * sigHSXChanged() change meaning.
     *
     * @see getColorModel()
     */
    void sigColorModelChanged();
    void sigHSXChanged(const QVector3D &hsx);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;

};

#endif
