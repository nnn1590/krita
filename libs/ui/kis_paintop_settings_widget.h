/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_PAINTOP_SETTINGS_WIDGET_H
#define KIS_PAINTOP_SETTINGS_WIDGET_H

#include <kritaui_export.h>
#include "kis_paintop_option.h"
#include <brushengine/kis_paintop_config_widget.h>

class KisPropertiesConfiguration;
class KisPaintOpConfigWidget;
class KisPaintopLodLimitations;

/**
 * A common widget for enabling/disabling and determining
 * the effect of tablet pressure, tilt and rotation and
 * other paintop settings.
 */
class KRITAUI_EXPORT KisPaintOpSettingsWidget : public KisPaintOpConfigWidget
{
    Q_OBJECT

public:

    KisPaintOpSettingsWidget(QWidget * parent = 0);

    ~KisPaintOpSettingsWidget() override;

    void addPaintOpOption(KisPaintOpOption * option, const QString &label);
    void addPaintOpOption(KisPaintOpOption * option, const QString &label, KisPaintOpOption::PaintopCategory category);
    void addPaintOpOption(KisPaintOpOption *option, const QString &label, QString category);

    /// Reimplemented
    void setConfiguration(const KisPropertiesConfigurationSP  config) override;

    /// Reimplemented
    void writeConfiguration(KisPropertiesConfigurationSP config) const override;

    KisPaintopLodLimitations lodLimitations() const override;

    ///Reimplemented, sets image on option widgets
    void setImage(KisImageWSP image) override;

    ///Reimplemented, sets node on option widgets
    void setNode(KisNodeWSP node) override;

    void setResourcesInterface(KisResourcesInterfaceSP resourcesInterface) override;
    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface) override;

private Q_SLOTS:

    void changePage(const QModelIndex&);
    void lockProperties(const QModelIndex& index);
    void slotLockPropertiesDrop();
    void slotLockPropertiesSave();
    void slotEntryChecked(const QModelIndex &index);

protected:
    virtual void notifyPageChanged();

private:
    
    struct Private;
    Private* const m_d;
    bool m_saveLockedOption;

};

#endif
