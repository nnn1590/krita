/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_stopgradient_editor.h"
#include <QPainter>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPoint>
#include <QMenu>

#include <KoColorSpace.h>
#include <resources/KoStopGradient.h>

#include "kis_debug.h"

#include <kis_icon_utils.h>

/****************************** KisStopGradientEditor ******************************/

KisStopGradientEditor::KisStopGradientEditor(QWidget *parent)
    : QWidget(parent),
      m_gradient(0)
{
    setupUi(this);

    connect(gradientSlider, SIGNAL(sigSelectedStop(int)), this, SLOT(stopChanged(int)));
    connect(nameedit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));
    connect(colorButton, SIGNAL(changed(KoColor)), SLOT(colorChanged(KoColor)));

    opacitySlider->setPrefix(i18n("Opacity: "));
    opacitySlider->setRange(0.0, 1.0, 2);
    connect(opacitySlider, SIGNAL(valueChanged(qreal)), this, SLOT(opacityChanged(qreal)));


    buttonReverse->setIcon(KisIconUtils::loadIcon("transform_icons_mirror_x"));
    buttonReverse->setToolTip(i18n("Flip Gradient"));
    KisIconUtils::updateIcon(buttonReverse);
    connect(buttonReverse, SIGNAL(pressed()), SLOT(reverse()));

    buttonReverseSecond->setIcon(KisIconUtils::loadIcon("transform_icons_mirror_x"));
    buttonReverseSecond->setToolTip(i18n("Flip Gradient"));
    KisIconUtils::updateIcon(buttonReverseSecond);
    connect(buttonReverseSecond, SIGNAL(clicked()), SLOT(reverse()));

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    setCompactMode(false);

    setGradient(0);
    stopChanged(-1);
}

KisStopGradientEditor::KisStopGradientEditor(KoStopGradientSP gradient, QWidget *parent, const char* name, const QString& caption)
    : KisStopGradientEditor(parent)
{
    setObjectName(name);
    setWindowTitle(caption);
    setGradient(gradient);
}

void KisStopGradientEditor::setCompactMode(bool value)
{
    lblName->setVisible(!value);
    buttonReverse->setVisible(!value);
    nameedit->setVisible(!value);

    buttonReverseSecond->setVisible(value);
}

void KisStopGradientEditor::setGradient(KoStopGradientSP gradient)
{
    m_gradient = gradient;
    setEnabled(m_gradient);

    if (m_gradient) {
        gradientSlider->setGradientResource(m_gradient);
        nameedit->setText(gradient->name());
        stopChanged(gradientSlider->selectedStop());
    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::notifyGlobalColorChanged(const KoColor &color)
{
    if (colorButton->isEnabled() &&
        color != colorButton->color()) {

        colorButton->setColor(color);
    }
}

boost::optional<KoColor> KisStopGradientEditor::currentActiveStopColor() const
{
    if (!colorButton->isEnabled()) return boost::none;
    return colorButton->color();
}

void KisStopGradientEditor::stopChanged(int stop)
{
    if (!m_gradient) return;

    const bool hasStopSelected = stop >= 0;

    opacitySlider->setEnabled(hasStopSelected);
    colorButton->setEnabled(hasStopSelected);
    stopLabel->setEnabled(hasStopSelected);

    if (hasStopSelected) {
        KoColor color = m_gradient->stops()[stop].second;
        opacitySlider->setValue(color.opacityF());
   
        color.setOpacity(1.0);
        colorButton->setColor(color);
    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::colorChanged(const KoColor& color)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();

    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].first;
    
    KoColor c(color, stops[currentStop].second.colorSpace());
    c.setOpacity(stops[currentStop].second.opacityU8());
    
    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, c));
    
    m_gradient->setStops(stops);
    gradientSlider->update();

    emit sigGradientChanged();
}

void KisStopGradientEditor::opacityChanged(qreal value)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();

    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].first;
    
    KoColor c = stops[currentStop].second;
    c.setOpacity(value);
    
    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, c));
    
    m_gradient->setStops(stops);
    gradientSlider->update();

    emit sigGradientChanged();
}


void KisStopGradientEditor::nameChanged()
{
    if (!m_gradient) return;

    m_gradient->setName(nameedit->text());
    emit sigGradientChanged();
}

void KisStopGradientEditor::reverse()
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    QList<KoGradientStop> reversedStops;
    for(const KoGradientStop& stop : stops) {
        reversedStops.push_front(KoGradientStop(1 - stop.first, stop.second));
    }
    m_gradient->setStops(reversedStops);
    gradientSlider->setSelectedStop(stops.size() - 1 - gradientSlider->selectedStop());

    emit sigGradientChanged();
}

void KisStopGradientEditor::sortByValue( SortFlags flags = SORT_ASCENDING )
{
    if (!m_gradient) return;

    bool ascending = (flags & SORT_ASCENDING) > 0;
    bool evenDistribution = (flags & EVEN_DISTRIBUTION) > 0;

    QList<KoGradientStop> stops = m_gradient->stops();
    const int stopCount = stops.size();

    QList<KoGradientStop> sortedStops;
    std::sort(stops.begin(), stops.end(), KoGradientStopValueSort());

    int stopIndex = 0;
    for (const KoGradientStop& stop : stops) {
        const float value = evenDistribution ? (float)stopIndex / (float)(stopCount - 1) : stop.second.toQColor().valueF();
        const float position = ascending ? value : 1.f - value;

        if (ascending) {
            sortedStops.push_back(KoGradientStop(position, stop.second));
        } else {
            sortedStops.push_front(KoGradientStop(position, stop.second));
        }

        stopIndex++;
    }

    m_gradient->setStops(sortedStops);
    gradientSlider->setSelectedStop(stopCount - 1);
    gradientSlider->update();

    emit sigGradientChanged();
}

void KisStopGradientEditor::showContextMenu(const QPoint &origin)
{
    QMenu contextMenu(i18n("Options"), this);

    QAction reverseValues(i18n("Reverse Values"), this);
    connect(&reverseValues, &QAction::triggered, this, &KisStopGradientEditor::reverse);

    QAction sortAscendingValues(i18n("Sort by Value"), this);
    connect(&sortAscendingValues, &QAction::triggered, this, [this]{ this->sortByValue(SORT_ASCENDING); } );
    QAction sortAscendingDistributed(i18n("Sort by Value (Even Distribution)"), this);
    connect(&sortAscendingDistributed, &QAction::triggered, this, [this]{ this->sortByValue(SORT_ASCENDING | EVEN_DISTRIBUTION);} );

    contextMenu.addAction(&reverseValues);
    contextMenu.addSeparator();
    contextMenu.addAction(&sortAscendingValues);
    contextMenu.addAction(&sortAscendingDistributed);

    contextMenu.exec(mapToGlobal(origin));
}

