/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_standard_uniform_properties_factory.h"

#include "kis_slider_based_paintop_property.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_settings_update_proxy.h"
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

namespace KisStandardUniformPropertiesFactory {



KisUniformPaintOpPropertySP createProperty(const KoID &id,
                                           KisPaintOpSettingsRestrictedSP settings,
                                           KisPaintopSettingsUpdateProxy *updateProxy)
{
    return createProperty(id.id(), settings, updateProxy);
}

KisUniformPaintOpPropertySP createProperty(const QString &id,
                                           KisPaintOpSettingsRestrictedSP settings,
                                           KisPaintopSettingsUpdateProxy *updateProxy)
{
    KisUniformPaintOpPropertySP result;


    if (id == size.id()) {
        KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    "size",
                    i18n("Size"),
                    settings, 0);



        prop->setRange(0, KSharedConfig::openConfig()->group("").readEntry("maximumBrushSize", 1000));
        prop->setDecimals(2);
        prop->setSingleStep(1);
        prop->setExponentRatio(3.0);
        prop->setSuffix(i18n(" px"));

        prop->setReadCallback(
                    [](KisUniformPaintOpProperty *prop) {
            prop->setValue(prop->settings()->paintOpSize());
        });
        prop->setWriteCallback(
                    [](KisUniformPaintOpProperty *prop) {
            prop->settings()->setPaintOpSize(prop->value().toReal());
        });

        QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
        prop->requestReadValue();
        result = toQShared(prop);
    } else if (id == opacity.id()) {
        KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    opacity.id(),
                    opacity.name(),
                    settings, 0);

        prop->setRange(0.0, 1.0);
        prop->setSingleStep(0.01);

        prop->setReadCallback(
                    [](KisUniformPaintOpProperty *prop) {
            prop->setValue(prop->settings()->paintOpOpacity());
        });
        prop->setWriteCallback(
                    [](KisUniformPaintOpProperty *prop) {
            prop->settings()->setPaintOpOpacity(prop->value().toReal());
        });

        QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
        prop->requestReadValue();
        result = toQShared(prop);
    } else if (id == flow.id()) {
        KisDoubleSliderBasedPaintOpPropertyCallback *prop =
                new KisDoubleSliderBasedPaintOpPropertyCallback(
                    KisDoubleSliderBasedPaintOpPropertyCallback::Double,
                    flow.id(),
                    flow.name(),
                    settings, 0);

        prop->setRange(0.0, 1.0);
        prop->setSingleStep(0.01);

        prop->setReadCallback(
                    [](KisUniformPaintOpProperty *prop) {
            prop->setValue(prop->settings()->paintOpFlow());
        });
        prop->setWriteCallback(
                    [](KisUniformPaintOpProperty *prop) {
            prop->settings()->setPaintOpFlow(prop->value().toReal());
        });

        QObject::connect(updateProxy, SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
        prop->requestReadValue();
        result = toQShared(prop);
    } else if (id == angle.id()) {
        qFatal("Not implemented");
    } else if (id == spacing.id()) {
        qFatal("Not implemented");
    }

    if (!result) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "Unknown Uniform property id!");
    }

    return result;
}
}
