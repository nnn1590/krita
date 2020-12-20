/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_hud_properties_config.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_config.h"
#include "kis_dom_utils.h"


struct KisBrushHudPropertiesConfig::Private
{
    QDomDocument doc;
    QDomElement root;

    void readConfig();
    void writeConfig();

    QDomDocument createDocument();
};

KisBrushHudPropertiesConfig::KisBrushHudPropertiesConfig()
    : m_d(new Private)
{
    m_d->readConfig();
}


KisBrushHudPropertiesConfig::~KisBrushHudPropertiesConfig()
{
    m_d->writeConfig();
}

void KisBrushHudPropertiesConfig::setSelectedProperties(const QString &paintOpId, const QList<QString> &ids)
{
    QDomElement el = m_d->doc.createElement(paintOpId);

    KisDomUtils::saveValue(&el, "properties_list", ids);

    QDomElement oldEl = m_d->root.firstChildElement(paintOpId);
    if (!oldEl.isNull()) {
        m_d->root.replaceChild(el, oldEl);
    } else {
        m_d->root.appendChild(el);
    }
}

QList<QString> KisBrushHudPropertiesConfig::selectedProperties(const QString &paintOpId) const
{
    QList<QString> result;
    QDomElement el;
    QStringList errors;

    if (KisDomUtils::findOnlyElement(m_d->root, paintOpId, &el, &errors)) {
        KisDomUtils::loadValue(el, "properties_list", &result);
    }

    return result;
}

void KisBrushHudPropertiesConfig::Private::readConfig()
{
    KisConfig cfg(true);
    doc = QDomDocument();

    QString docContent = cfg.brushHudSetting();
    if (!docContent.isNull()) {
        doc.setContent(docContent);
        root = doc.firstChildElement("hud_properties");

        int version = -1;
        if (!KisDomUtils::loadValue(root, "version", &version) ||
            version != 1) {

            warnKrita << "Unknown Brush HUD XML document type or version!";
            doc = QDomDocument();
        }
    }

    if (doc.isNull()) {
        doc = QDomDocument("hud_properties");
        root = doc.createElement("hud_properties");
        doc.appendChild(root);

        KisDomUtils::saveValue(&root, "version", 1);
    }
}

void KisBrushHudPropertiesConfig::Private::writeConfig()
{
    KisConfig cfg(false);
    cfg.setBrushHudSetting(doc.toString());
}

void KisBrushHudPropertiesConfig::filterProperties(
    const QString &paintOpId,
    const QList<KisUniformPaintOpPropertySP> &allProperties,
    QList<KisUniformPaintOpPropertySP> *chosenProperties,
    QList<KisUniformPaintOpPropertySP> *skippedProperties) const {

    QList<QString> selectedIds = selectedProperties(paintOpId);
    *skippedProperties = allProperties;

    Q_FOREACH (const QString &id, selectedIds) {
        auto it = std::find_if(skippedProperties->begin(),
                               skippedProperties->end(),
                               [id] (KisUniformPaintOpPropertySP prop) {
                                   return prop->id() == id;
                               });

        if (it != skippedProperties->end()) {
            *chosenProperties << *it;
            it = skippedProperties->erase(it);
        } else {
            warnKrita << "Filtering HUD properties: property \"" << id << "\" does not exist!";
            ++it;
        }
    }
}

QDomDocument* KisBrushHudPropertiesConfig::testingGetDocument()
{
    return &m_d->doc;
}
