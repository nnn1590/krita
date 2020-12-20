/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <brushengine/kis_paintop_settings.h>

#include <QImage>
#include <QColor>
#include <QPainterPath>
#include <QPointer>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>
#include <KoViewConverter.h>

#include "kis_paintop_preset.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paintop_registry.h"
#include "kis_timing_information.h"
#include <brushengine/kis_paint_information.h>
#include "kis_paintop_config_widget.h"
#include <brushengine/kis_paintop_preset.h>
#include "kis_paintop_settings_update_proxy.h"
#include <time.h>
#include <kis_types.h>
#include <kis_signals_blocker.h>

#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties_proxy.h>

#include "KisPaintopSettingsIds.h"
#include "kis_algebra_2d.h"
#include "kis_image_config.h"
#include <KoCanvasResourcesInterface.h>


struct Q_DECL_HIDDEN KisPaintOpSettings::Private {
    Private()
        : disableDirtyNotifications(false)
    {}

    Private(const Private &rhs)
        : settingsWidget(0),
          modelName(rhs.modelName),
          updateProxy(rhs.updateProxy),
          resourcesInterface(rhs.resourcesInterface),
          canvasResourcesInterface(rhs.canvasResourcesInterface),
          disableDirtyNotifications(false)
    {
    }

    QPointer<KisPaintOpConfigWidget> settingsWidget;
    QString modelName;
    QPointer<KisPaintopSettingsUpdateProxy> updateProxy;
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
    KisResourcesInterfaceSP resourcesInterface;
    KoCanvasResourcesInterfaceSP canvasResourcesInterface;

    bool disableDirtyNotifications;

    class DirtyNotificationsLocker {
    public:
        DirtyNotificationsLocker(KisPaintOpSettings::Private *d)
            : m_d(d),
              m_oldNotificationsState(d->disableDirtyNotifications)
        {
            m_d->disableDirtyNotifications = true;
        }

        ~DirtyNotificationsLocker() {
            m_d->disableDirtyNotifications = m_oldNotificationsState;
        }

    private:
        KisPaintOpSettings::Private *m_d;
        bool m_oldNotificationsState;
        Q_DISABLE_COPY(DirtyNotificationsLocker)
    };
};


KisPaintOpSettings::KisPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : d(new Private)
{
    d->updateProxy = 0;
    d->resourcesInterface = resourcesInterface;
}

KisPaintOpSettings::~KisPaintOpSettings()
{
}

KisPaintOpSettings::KisPaintOpSettings(const KisPaintOpSettings &rhs)
    : KisPropertiesConfiguration(rhs)
    , d(new Private(*rhs.d))
{
}

void KisPaintOpSettings::setOptionsWidget(KisPaintOpConfigWidget* widget)
{
    d->settingsWidget = widget;
}

void KisPaintOpSettings::setUpdateProxy(const QPointer<KisPaintopSettingsUpdateProxy> proxy)
{
    d->updateProxy = proxy;
}

QPointer<KisPaintopSettingsUpdateProxy> KisPaintOpSettings::updateProxy() const
{
    return d->updateProxy;
}

bool KisPaintOpSettings::mousePressEvent(const KisPaintInformation &paintInformation, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    Q_UNUSED(modifiers);
    Q_UNUSED(currentNode);
    setRandomOffset(paintInformation);
    return true; // ignore the event by default
}

bool KisPaintOpSettings::mouseReleaseEvent()
{
    return true; // ignore the event by default
}

void KisPaintOpSettings::setRandomOffset(const KisPaintInformation &paintInformation)
{
	bool disableDirtyBefore = d->disableDirtyNotifications;
	d->disableDirtyNotifications = true;
    if (getBool("Texture/Pattern/Enabled")) {
        if (getBool("Texture/Pattern/isRandomOffsetX")) {
            setProperty("Texture/Pattern/OffsetX",
                        paintInformation.randomSource()->generate(0, KisPropertiesConfiguration::getInt("Texture/Pattern/MaximumOffsetX")));
        }
        if (getBool("Texture/Pattern/isRandomOffsetY")) {
            setProperty("Texture/Pattern/OffsetY",
                        paintInformation.randomSource()->generate(0, KisPropertiesConfiguration::getInt("Texture/Pattern/MaximumOffsetY")));

        }
    }
	d->disableDirtyNotifications = disableDirtyBefore;
}

bool KisPaintOpSettings::hasMaskingSettings() const
{
    return getBool(KisPaintOpUtils::MaskingBrushEnabledTag, false);
}

KisPaintOpSettingsSP KisPaintOpSettings::createMaskingSettings() const
{
    if (!hasMaskingSettings()) return KisPaintOpSettingsSP();

    const KoID pixelBrushId(KisPaintOpUtils::MaskingBrushPaintOpId, QString());

    KisPaintOpSettingsSP maskingSettings = KisPaintOpRegistry::instance()->createSettings(pixelBrushId, resourcesInterface());
    this->getPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, maskingSettings);

    const bool useMasterSize = this->getBool(KisPaintOpUtils::MaskingBrushUseMasterSizeTag, true);
    if (useMasterSize) {
        /**
         * WARNING: cropping is a workaround for too big brushes due to
         * the proportional scaling using shift+drag gesture.
         *
         * See this bug: https://bugs.kde.org/show_bug.cgi?id=423572
         *
         * TODO:
         *
         * 1) Implement a warning notifying the user that his masking
         *    brush has been cropped
         *
         * 2) Make sure that the sliders in KisMaskingBrushOption have
         *    correct limits (right now they are limited by usual
         *    maximumBrushSize)
         */

        const qreal maxBrushSize = KisImageConfig(true).readEntry("maximumBrushSize", 1000);
        const qreal maxMaskingBrushSize = qMin(15000.0, 3.0 * maxBrushSize);

        const qreal masterSizeCoeff = getDouble(KisPaintOpUtils::MaskingBrushMasterSizeCoeffTag, 1.0);
        maskingSettings->setPaintOpSize(qMin(maxMaskingBrushSize, masterSizeCoeff * paintOpSize()));
    }

    return maskingSettings;
}

bool KisPaintOpSettings::hasPatternSettings() const
{
    return false;
}

QList<int> KisPaintOpSettings::requiredCanvasResources() const
{
    return {};
}

KoCanvasResourcesInterfaceSP KisPaintOpSettings::canvasResourcesInterface() const
{
    return d->canvasResourcesInterface;
}

void KisPaintOpSettings::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    d->canvasResourcesInterface = canvasResourcesInterface;
}

QString KisPaintOpSettings::maskingBrushCompositeOp() const
{
    return getString(KisPaintOpUtils::MaskingBrushCompositeOpTag, COMPOSITE_MULT);
}

KisResourcesInterfaceSP KisPaintOpSettings::resourcesInterface() const
{
    return d->resourcesInterface;
}

void KisPaintOpSettings::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    d->resourcesInterface = resourcesInterface;
}

KisPaintOpSettingsSP KisPaintOpSettings::clone() const
{
    QString paintopID = getString("paintop");
    if (paintopID.isEmpty())
        return 0;

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->createSettings(KoID(paintopID), resourcesInterface());
    QMapIterator<QString, QVariant> i(getProperties());
    while (i.hasNext()) {
        i.next();
        settings->setProperty(i.key(), QVariant(i.value()));
    }
    settings->setUpdateProxy(this->updateProxy());
    settings->setCanvasResourcesInterface(this->canvasResourcesInterface());
    return settings;
}

void KisPaintOpSettings::resetSettings(const QStringList &preserveProperties)
{
    QStringList allKeys = preserveProperties;
    allKeys << "paintop";

    QHash<QString, QVariant> preserved;
    Q_FOREACH (const QString &key, allKeys) {
        if (hasProperty(key)) {
            preserved[key] = getProperty(key);
        }
    }

    clearProperties();

    for (auto it = preserved.constBegin(); it != preserved.constEnd(); ++it) {
        setProperty(it.key(), it.value());
    }
}

void KisPaintOpSettings::activate()
{
}

void KisPaintOpSettings::setPaintOpOpacity(qreal value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("OpacityValue", value);
}

void KisPaintOpSettings::setPaintOpFlow(qreal value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("FlowValue", value);
}

void KisPaintOpSettings::setPaintOpCompositeOp(const QString &value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("CompositeOp", value);
}

qreal KisPaintOpSettings::paintOpOpacity()
{
    KisLockedPropertiesProxySP proxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this);

    return proxy->getDouble("OpacityValue", 1.0);
}

qreal KisPaintOpSettings::paintOpFlow()
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getDouble("FlowValue", 1.0);
}

qreal KisPaintOpSettings::paintOpPatternSize()
{
    KisLockedPropertiesProxySP proxy(
        KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getDouble("Texture/Pattern/Scale", 0.5);
}

QString KisPaintOpSettings::paintOpCompositeOp()
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getString("CompositeOp", COMPOSITE_OVER);
}

void KisPaintOpSettings::setEraserMode(bool value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("EraserMode", value);
}

bool KisPaintOpSettings::eraserMode()
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getBool("EraserMode", false);
}

QString KisPaintOpSettings::effectivePaintOpCompositeOp()
{
    return !eraserMode() ? paintOpCompositeOp() : COMPOSITE_ERASE;
}

qreal KisPaintOpSettings::savedEraserSize() const
{
    return getDouble("SavedEraserSize", 0.0);
}

void KisPaintOpSettings::setSavedEraserSize(qreal value)
{
    setProperty("SavedEraserSize", value);
    setPropertyNotSaved("SavedEraserSize");
}

qreal KisPaintOpSettings::savedBrushSize() const
{
    return getDouble("SavedBrushSize", 0.0);
}

void KisPaintOpSettings::setSavedBrushSize(qreal value)
{
    setProperty("SavedBrushSize", value);
    setPropertyNotSaved("SavedBrushSize");
}

qreal KisPaintOpSettings::savedEraserOpacity() const
{
    return getDouble("SavedEraserOpacity", 0.0);
}

void KisPaintOpSettings::setSavedEraserOpacity(qreal value)
{
    setProperty("SavedEraserOpacity", value);
    setPropertyNotSaved("SavedEraserOpacity");
}

qreal KisPaintOpSettings::savedBrushOpacity() const
{
    return getDouble("SavedBrushOpacity", 0.0);
}

void KisPaintOpSettings::setSavedBrushOpacity(qreal value)
{
    setProperty("SavedBrushOpacity", value);
    setPropertyNotSaved("SavedBrushOpacity");
}

QString KisPaintOpSettings::modelName() const
{
    return d->modelName;
}

void KisPaintOpSettings::setModelName(const QString & modelName)
{
    d->modelName = modelName;
}

KisPaintOpConfigWidget* KisPaintOpSettings::optionsWidget() const
{
    if (d->settingsWidget.isNull())
        return 0;

    return d->settingsWidget.data();
}

bool KisPaintOpSettings::isValid() const
{
    return true;
}

QString KisPaintOpSettings::indirectPaintingCompositeOp() const
{
    return COMPOSITE_ALPHA_DARKEN;
}

bool KisPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED, false);
}

qreal KisPaintOpSettings::airbrushInterval() const
{
    qreal rate = getDouble(AIRBRUSH_RATE, 1.0);
    if (rate == 0.0) {
        return LONG_TIME;
    }
    else {
        return 1000.0 / rate;
    }
}

bool KisPaintOpSettings::useSpacingUpdates() const
{
    return getBool(SPACING_USE_UPDATES, false);
}

bool KisPaintOpSettings::needsAsynchronousUpdates() const
{
    return false;
}

QPainterPath KisPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;
    if (mode.isVisible) {
        path = ellipseOutline(10, 10, 1.0, 0);

        if (mode.showTiltDecoration) {
            path.addPath(makeTiltIndicator(info, QPointF(0.0, 0.0), 0.0, 2.0));
        }

        path.translate(KisAlgebra2D::alignForZoom(info.pos(), alignForZoom));
    }

    return path;
}

QPainterPath KisPaintOpSettings::ellipseOutline(qreal width, qreal height, qreal scale, qreal rotation)
{
    QPainterPath path;
    QRectF ellipse(0, 0, width * scale, height * scale);
    ellipse.translate(-ellipse.center());
    path.addEllipse(ellipse);

    QTransform m;
    m.reset();
    m.rotate(rotation);
    path = m.map(path);
    return path;
}

QPainterPath KisPaintOpSettings::makeTiltIndicator(KisPaintInformation const& info,
                                                   QPointF const& start, qreal maxLength, qreal angle)
{
    if (maxLength == 0.0) maxLength = 50.0;
    maxLength = qMax(maxLength, 50.0);
    qreal const length = maxLength * (1 - info.tiltElevation(info, 60.0, 60.0, true));
    qreal const baseAngle = 360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0);

    QLineF guideLine = QLineF::fromPolar(length, baseAngle + angle);
    guideLine.translate(start);
    QPainterPath ret;
    ret.moveTo(guideLine.p1());
    ret.lineTo(guideLine.p2());
    guideLine.setAngle(baseAngle - angle);
    ret.lineTo(guideLine.p2());
    ret.lineTo(guideLine.p1());
    return ret;
}

void KisPaintOpSettings::setProperty(const QString & name, const QVariant & value)
{
    if (value != KisPropertiesConfiguration::getProperty(name) && !d->disableDirtyNotifications) {
        if (d->updateProxy) {
            d->updateProxy->setDirty(true);
        }
    }

    KisPropertiesConfiguration::setProperty(name, value);
    onPropertyChanged();
}


void KisPaintOpSettings::onPropertyChanged()
{
    if (d->updateProxy) {
        d->updateProxy->notifySettingsChanged();
    }
}

bool KisPaintOpSettings::isLodUserAllowed(const KisPropertiesConfigurationSP config)
{
    return config->getBool("lodUserAllowed", true);
}

void KisPaintOpSettings::setLodUserAllowed(KisPropertiesConfigurationSP config, bool value)
{
    config->setProperty("lodUserAllowed", value);
}

bool KisPaintOpSettings::lodSizeThresholdSupported() const
{
    return true;
}

qreal KisPaintOpSettings::lodSizeThreshold() const
{
    return getDouble("lodSizeThreshold", 100.0);
}

void KisPaintOpSettings::setLodSizeThreshold(qreal value)
{
    setProperty("lodSizeThreshold", value);
}

#include "kis_standard_uniform_properties_factory.h"

QList<KisUniformPaintOpPropertySP> KisPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
            listWeakToStrong(d->uniformProperties);


    if (props.isEmpty()) {
        using namespace KisStandardUniformPropertiesFactory;

        props.append(createProperty(opacity, settings, d->updateProxy));
        props.append(createProperty(size, settings, d->updateProxy));
        props.append(createProperty(flow, settings, d->updateProxy));

        d->uniformProperties = listStrongToWeak(props);
    }

    return props;
}
