/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PAINTOP_PRESET_H
#define KIS_PAINTOP_PRESET_H

#include <QPointer>

#include <KoResource.h>
#include "KoID.h"

#include "kis_types.h"
#include "kis_shared.h"
#include "kritaimage_export.h"
#include <brushengine/kis_uniform_paintop_property.h>
#include <kis_paintop_settings_update_proxy.h>

class KisPaintOpConfigWidget;

class KoCanvasResourcesInterface;
using KoCanvasResourcesInterfaceSP = QSharedPointer<KoCanvasResourcesInterface>;


class ProxyParent : public QObject
{
    Q_OBJECT
public:
    ProxyParent(KisPaintOpPreset *preset)
    {
        m_preset = preset;
    }

    KisPaintOpPreset *m_preset {0};
};

/**
 * A KisPaintOpPreset contains a particular set of settings
 * associated with a paintop, like brush, paintopsettings.
 * A new property in this class is to make it dirty. That means the
 * user can now temporarily save any tweaks in the Preset throughout
 * the session. The Dirty Preset setting/unsetting is handled by KisPaintOpPresetSettings
 */
class KRITAIMAGE_EXPORT KisPaintOpPreset : public KoResource
{
public:

    /**
     * @brief The UpdatedPostponer class
     * @see KisPaintopSettingsUpdateProxy::postponeSettingsChanges()
     */
    class KRITAIMAGE_EXPORT UpdatedPostponer{
    public:
        UpdatedPostponer(KisPaintOpPresetSP preset);

        ~UpdatedPostponer();

    private:
        QPointer<KisPaintopSettingsUpdateProxy> m_updateProxy;
    };

public:

    KisPaintOpPreset();

    KisPaintOpPreset(const QString& filename);

    ~KisPaintOpPreset() override;

    KisPaintOpPreset(const KisPaintOpPreset &rhs);
    KisPaintOpPreset &operator=(const KisPaintOpPreset &rhs) = delete;
    KoResourceSP clone() const override;

    /// set the id of the paintop plugin
    void setPaintOp(const KoID & paintOp);

    /// return the id of the paintop plugin
    KoID paintOp() const;

    /// replace the current settings object with the specified settings
    void setSettings(KisPaintOpSettingsSP settings);
    void setOriginalSettings(KisPaintOpSettingsSP originalSettings);

    /// return the settings that define this paintop preset
    KisPaintOpSettingsSP settings() const;
    KisPaintOpSettingsSP originalSettings() const;

    bool load(KisResourcesInterfaceSP resourcesInterface) override;
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;

    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::PaintOpPresets, "");
    }

    void toXML(QDomDocument& doc, QDomElement& elt) const;

    void fromXML(const QDomElement& elt, KisResourcesInterfaceSP resourcesInterface);

    bool removable() const {
        return true;
    }

    QString defaultFileExtension() const override {
        return ".kpp";
    }

    void setOptionsWidget(KisPaintOpConfigWidget *widget);

    QPointer<KisPaintopSettingsUpdateProxy> updateProxy() const;
    QPointer<KisPaintopSettingsUpdateProxy> updateProxyNoCreate() const;

    QList<KisUniformPaintOpPropertySP> uniformProperties();

    /**
     * @return true if this preset demands a secondary masked brush running
     *         alongside it
     */
    bool hasMaskingPreset() const;

    /**
     * @return a newly created preset of the masked brush that should be run
     *         alongside the current brush
     */
    KisPaintOpPresetSP createMaskingPreset() const;

    /**
     * @return resource interface that is used by KisPaintOpSettings object for
     * loading linked resources
     */
    KisResourcesInterfaceSP resourcesInterface() const;

    /**
     * Set resource interface that will be used by KisPaintOpSettings object for
     * loading linked resources
     */
    void setResourcesInterface(KisResourcesInterfaceSP resourcesInterface);

    /**
     * Returns canvas resources interface associated with the current preset.
     *
     * In contrast to resourcesInterface() the canvas resources interface may
     * be null, becuase the preset is created without any canvas resources.
     * The resources are assigned to the preset only when the ser starts to
     * paint with it.
     *
     * The preset has no default canvas resources interface, because canvas
     * resources are unique per-canvas, but the presets are unique per-
     * application. Therefore association between the preset and canvas
     * resources interface would be ambiguous.
     */
    KoCanvasResourcesInterfaceSP canvasResourcesInterface() const;

    /**
     * Sets canvas resources interface used for initializing the preset
     *
     * @see canvasResourcesInterface()
     */
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    /**
     * \see KisRequiredResourcesOperators::createLocalResourcesSnapshot
     */
    void createLocalResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    /**
     * \see KisRequiredResourcesOperators::hasLocalResourcesSnapshot
     */
    bool hasLocalResourcesSnapshot() const;

    /**
     * \see KisRequiredResourcesOperators::cloneWithResourcesSnapshot
     */
    KisPaintOpPresetSP cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface) const;


    QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

    QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override;

    QList<int> requiredCanvasResources() const override;

private:

    struct Private;
    Private * const d;
};

Q_DECLARE_METATYPE(KisPaintOpPresetSP)

#endif
