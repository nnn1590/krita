/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_option.h"
#include "kis_duplicateop_settings_widget.h"

#include <QDomElement>
#include <QDomDocument>

#include <KoPointerEvent.h>
#include <KoCompositeOpRegistry.h>

#include <kis_image.h>
#include <kis_brush_option_widget.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_darken_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_paint_action_type_option.h>
#include <kis_dom_utils.h>

KisDuplicateOpSettings::KisDuplicateOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface),
      m_isOffsetNotUptodate(false),
      m_duringPaintingStroke(false)
{
}

KisDuplicateOpSettings::~KisDuplicateOpSettings()
{
}

bool KisDuplicateOpSettings::paintIncremental()
{
    return false;
}

QString KisDuplicateOpSettings::indirectPaintingCompositeOp() const
{
    return COMPOSITE_COPY;
}

QPointF KisDuplicateOpSettings::offset() const
{
    return m_offset;
}

QPointF KisDuplicateOpSettings::position() const
{
    return m_position;
}

bool KisDuplicateOpSettings::mousePressEvent(const KisPaintInformation &info, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    bool ignoreEvent = true;

    if (modifiers & Qt::ControlModifier) {
        if (!m_sourceNode || !(modifiers & Qt::AltModifier)) {
            m_sourceNode = currentNode;
        }
        m_position = info.pos();
        m_isOffsetNotUptodate = true;
        ignoreEvent = false;
    }
    else {
        bool resetOrigin = getBool(DUPLICATE_RESET_SOURCE_POINT);
        if (m_isOffsetNotUptodate || resetOrigin) {
            m_offset = info.pos() - m_position;
            m_isOffsetNotUptodate = false;
        }
        m_duringPaintingStroke = true;
        ignoreEvent = true;
    }

    return ignoreEvent;
}

bool KisDuplicateOpSettings::mouseReleaseEvent()
{
    m_duringPaintingStroke = false;
    bool ignoreEvent = true;
    return ignoreEvent;
}


KisNodeWSP KisDuplicateOpSettings::sourceNode() const
{
    return m_sourceNode;
}

void KisDuplicateOpSettings::activate()
{
}

void KisDuplicateOpSettings::fromXML(const QDomElement& elt)
{
    // First, call the parent class fromXML to make sure all the
    // properties are saved to the map
    KisPaintOpSettings::fromXML(elt);

    m_offset.setX(KisDomUtils::toDouble(elt.attribute("OffsetX", "0.0")));
    m_offset.setY(KisDomUtils::toDouble(elt.attribute("OffsetY", "0.0")));
    m_isOffsetNotUptodate = false;
}

void KisDuplicateOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    // Then call the parent class fromXML
    KisPropertiesConfiguration::toXML(doc, rootElt);

    rootElt.setAttribute("OffsetX", QString::number(m_offset.x()));
    rootElt.setAttribute("OffsetY", QString::number(m_offset.y()));
}


KisPaintOpSettingsSP KisDuplicateOpSettings::clone() const
{
    KisPaintOpSettingsSP setting = KisBrushBasedPaintOpSettings::clone();
    KisDuplicateOpSettings* s = static_cast<KisDuplicateOpSettings*>(setting.data());
    s->m_offset = m_offset;
    s->m_isOffsetNotUptodate = m_isOffsetNotUptodate;
    s->m_position = m_position;
    s->m_sourceNode = m_sourceNode;
    s->m_duringPaintingStroke = m_duringPaintingStroke;

    return setting;
}

QPainterPath KisDuplicateOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;

    OutlineMode forcedMode = mode;

    if (!forcedMode.isVisible) {
        forcedMode.isVisible = true;
        forcedMode.forceCircle = true;
    }

    // clone tool should always show an outline
    path = KisBrushBasedPaintOpSettings::brushOutlineImpl(info, forcedMode, alignForZoom, 1.0);

    QPainterPath copy(path);
    QRectF rect2 = copy.boundingRect();
    bool shouldStayInOrigin = m_isOffsetNotUptodate // the clone brush right now waits for first stroke with a new origin, so stays at origin point
            || !getBool(DUPLICATE_MOVE_SOURCE_POINT) // the brush always use the same source point, so stays at origin point
            || (!m_duringPaintingStroke && getBool(DUPLICATE_RESET_SOURCE_POINT)); // during the stroke, with reset Origin selected, outline should stay at origin point

    if (shouldStayInOrigin) {
        copy.translate(m_position - info.pos());
    }
    else {
        copy.translate(-m_offset);
    }

    path.addPath(copy);

    qreal dx = rect2.width() / 4.0;
    qreal dy = rect2.height() / 4.0;
    rect2.adjust(dx, dy, -dx, -dy);

    path.moveTo(rect2.topLeft());
    path.lineTo(rect2.bottomRight());

    path.moveTo(rect2.topRight());
    path.lineTo(rect2.bottomLeft());

    return path;
}


#include <brushengine/kis_uniform_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisDuplicateOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
            listWeakToStrong(m_uniformProperties);

    if (props.isEmpty()) {
        {
            KisUniformPaintOpPropertyCallback *prop =
                    new KisUniformPaintOpPropertyCallback(
                        KisUniformPaintOpPropertyCallback::Bool,
                        "clone_healing",
                        i18n("Healing"),
                        settings, 0);

            prop->setReadCallback(
                        [](KisUniformPaintOpProperty *prop) {
                KisDuplicateOptionProperties option;
                option.readOptionSetting(prop->settings().data());

                prop->setValue(option.duplicate_healing);
            });
            prop->setWriteCallback(
                        [](KisUniformPaintOpProperty *prop) {
                KisDuplicateOptionProperties option;
                option.readOptionSetting(prop->settings().data());
                option.duplicate_healing = prop->value().toBool();
                option.writeOptionSetting(prop->settings().data());
            });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
        {
            KisUniformPaintOpPropertyCallback *prop =
                    new KisUniformPaintOpPropertyCallback(
                        KisUniformPaintOpPropertyCallback::Bool,
                        "clone_movesource",
                        i18n("Move Source"),
                        settings, 0);

            prop->setReadCallback(
                        [](KisUniformPaintOpProperty *prop) {
                KisDuplicateOptionProperties option;
                option.readOptionSetting(prop->settings().data());

                prop->setValue(option.duplicate_move_source_point);
            });
            prop->setWriteCallback(
                        [](KisUniformPaintOpProperty *prop) {
                KisDuplicateOptionProperties option;
                option.readOptionSetting(prop->settings().data());
                option.duplicate_move_source_point = prop->value().toBool();
                option.writeOptionSetting(prop->settings().data());
            });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties(settings) + props;
}


