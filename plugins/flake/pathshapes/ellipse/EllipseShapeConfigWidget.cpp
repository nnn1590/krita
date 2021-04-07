/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "EllipseShapeConfigWidget.h"
#include "EllipseShapeConfigCommand.h"
#include <klocalizedstring.h>
#include <kis_signals_blocker.h>
#include "kis_assert.h"

EllipseShapeConfigWidget::EllipseShapeConfigWidget()
    : m_ellipse(0)
{
    widget.setupUi(this);

    widget.ellipseType->clear();
    widget.ellipseType->addItem(i18n("Arc"));
    widget.ellipseType->addItem(i18n("Pie"));
    widget.ellipseType->addItem(i18n("Chord"));

    widget.startAngle->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);

    widget.endAngle->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);

    connect(widget.ellipseType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(propertyChanged()));
    connect(widget.startAngle, SIGNAL(angleChanged(qreal)), this, SIGNAL(propertyChanged()));
    connect(widget.endAngle, SIGNAL(angleChanged(qreal)), this, SIGNAL(propertyChanged()));
    connect(widget.closeEllipse, SIGNAL(clicked(bool)), this, SLOT(closeEllipse()));
}

void EllipseShapeConfigWidget::open(KoShape *shape)
{
    if (m_ellipse) {
        m_ellipse->removeShapeChangeListener(this);
    }

    m_ellipse = dynamic_cast<EllipseShape *>(shape);
    if (!m_ellipse) return;

    loadPropertiesFromShape(m_ellipse);

    m_ellipse->addShapeChangeListener(this);
}

void EllipseShapeConfigWidget::loadPropertiesFromShape(EllipseShape *shape)
{
    KisSignalsBlocker b(widget.ellipseType, widget.startAngle, widget.endAngle);

    widget.ellipseType->setCurrentIndex(shape->type());
    widget.startAngle->setAngle(shape->startAngle());
    widget.endAngle->setAngle(shape->endAngle());
}


void EllipseShapeConfigWidget::save()
{
    if (!m_ellipse) {
        return;
    }

    m_ellipse->setType(static_cast<EllipseShape::EllipseType>(widget.ellipseType->currentIndex()));
    m_ellipse->setStartAngle(widget.startAngle->angle());
    m_ellipse->setEndAngle(widget.endAngle->angle());
}

KUndo2Command *EllipseShapeConfigWidget::createCommand()
{
    if (!m_ellipse) {
        return 0;
    } else {
        EllipseShape::EllipseType type = static_cast<EllipseShape::EllipseType>(widget.ellipseType->currentIndex());
        return new EllipseShapeConfigCommand(m_ellipse, type, widget.startAngle->angle(), widget.endAngle->angle());
    }
}

void EllipseShapeConfigWidget::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_ellipse && shape == m_ellipse);

    if (type == KoShape::ParameterChanged) {
        open(m_ellipse);
    }
}

void EllipseShapeConfigWidget::closeEllipse()
{
    widget.startAngle->blockSignals(true);
    widget.endAngle->blockSignals(true);

    widget.startAngle->setAngle(0.0);
    widget.endAngle->setAngle(360.0);

    widget.startAngle->blockSignals(false);
    widget.endAngle->blockSignals(false);

    emit propertyChanged();
}
