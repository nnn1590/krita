/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <utility>

#include <kpluginfactory.h>
#include <KoUpdater.h>
#include <kis_processing_information.h>
#include <KisSequentialIteratorProgress.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <KoCompositeOpRegistry.h>
#include <kis_selection.h>
#include <kis_painter.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>

#include "KisScreentoneGenerator.h"
#include "KisScreentoneConfigWidget.h"
#include "KisScreentoneBrightnessContrastFunctions.h"
#include "KisScreentoneScreentoneFunctions.h"
#include "KisScreentoneConfigDefaults.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaScreentoneGeneratorFactory, "KritaScreentoneGenerator.json", registerPlugin<KisScreentoneGeneratorHandle>();)

KisScreentoneGeneratorHandle::KisScreentoneGeneratorHandle(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisGeneratorRegistry::instance()->add(new KisScreentoneGenerator());

}

KisScreentoneGeneratorHandle::~KisScreentoneGeneratorHandle()
{
}

KisScreentoneGenerator::KisScreentoneGenerator() : KisGenerator(id(), KoID("basic"), i18n("&Screentone..."))
{
    setSupportsPainting(true);
}

void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisFilterConfigurationSP config,
                                      KoUpdater *progressUpdater) const
{
    const KisScreentonePatternType pattern =
        static_cast<KisScreentonePatternType>(
            config->getInt("pattern", KisScreentoneConfigDefaults::pattern())
        );
    const KisScreentoneShapeType shape =
        static_cast<KisScreentoneShapeType>(
            config->getInt("shape", KisScreentoneConfigDefaults::shape())
        );
    const KisScreentoneInterpolationType interpolation =
        static_cast<KisScreentoneInterpolationType>(
            config->getInt("interpolation", KisScreentoneConfigDefaults::interpolation())
        );

    if (pattern == KisScreentonePatternType_Dots) {
        if (shape == KisScreentoneShapeType_RoundDots) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsRoundLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsRoundSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_EllipseDots) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::DotsEllipseLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::DotsEllipseSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_DiamondDots) {
            KisScreentoneScreentoneFunctions::DotsDiamond screentoneFunction;
            generate(dst, size, config, progressUpdater, screentoneFunction);
        } else if (shape == KisScreentoneShapeType_SquareDots) {
            KisScreentoneScreentoneFunctions::DotsSquare screentoneFunction;
            generate(dst, size, config, progressUpdater, screentoneFunction);
        }
    } else if (pattern == KisScreentonePatternType_Lines) {
        if (shape == KisScreentoneShapeType_StraightLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesStraightLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesStraightSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_SineWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesSineWaveLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesSineWaveSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_TriangularWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesTriangularWaveLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesTriangularWaveSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_SawtoothWaveLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesSawToothWaveLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesSawToothWaveSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        } else if (shape == KisScreentoneShapeType_CurtainsLines) {
            if (interpolation == KisScreentoneInterpolationType_Linear) {
                KisScreentoneScreentoneFunctions::LinesCurtainsLinear screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            } else if (interpolation == KisScreentoneInterpolationType_Sinusoidal) {
                KisScreentoneScreentoneFunctions::LinesCurtainsSinusoidal screentoneFunction;
                generate(dst, size, config, progressUpdater, screentoneFunction);
            }
        }
    }
}

template <class ScreentoneFunction>
void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisFilterConfigurationSP config,
                                      KoUpdater *progressUpdater,
                                      const ScreentoneFunction &screentoneFunction) const
{
    const qreal brightness = config->getDouble("brightness", KisScreentoneConfigDefaults::brightness()) / 50. - 1.0;
    const qreal contrast = config->getDouble("contrast", KisScreentoneConfigDefaults::contrast()) / 50. - 1.0;

    const bool bypassBrightnessContrast = qFuzzyIsNull(brightness) && qFuzzyIsNull(contrast);

    if (bypassBrightnessContrast) {
        KisScreentoneBrightnessContrastFunctions::Identity brightnessContrastFunction;
        generate(dst, size, config, progressUpdater, screentoneFunction, brightnessContrastFunction);
    } else {
        KisScreentoneBrightnessContrastFunctions::BrightnessContrast brightnessContrastFunction(brightness, contrast);
        generate(dst, size, config, progressUpdater, screentoneFunction, brightnessContrastFunction);
    }
}

bool KisScreentoneGenerator::checkUpdaterInterruptedAndSetPercent(KoUpdater *progressUpdater, int percent) const
{
    // The updater is null so return false to keep going
    // with the computations
    if (!progressUpdater) {
        return false;
    }

    if (progressUpdater->interrupted()) {
        return true;
    }

    progressUpdater->setProgress(percent);
    return false;
}

template <class ScreentoneFunction, class BrightnessContrastFunction>
void KisScreentoneGenerator::generate(KisProcessingInformation dst,
                                      const QSize &size,
                                      const KisFilterConfigurationSP config,
                                      KoUpdater *progressUpdater,
                                      const ScreentoneFunction &screentoneFunction,
                                      const BrightnessContrastFunction &brightnessContrastFunction) const
{
    KisPaintDeviceSP device = dst.paintDevice();
    Q_ASSERT(!device.isNull());
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    checkUpdaterInterruptedAndSetPercent(progressUpdater, 0);
    
    const QRect bounds = QRect(dst.topLeft(), size);
    const KoColorSpace *colorSpace;
    if (device->colorSpace()->profile()->isLinear()) {
        colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    } else {
        colorSpace = device->colorSpace();
    }
    
    const qreal positionX = config->getDouble("position_x", KisScreentoneConfigDefaults::positionX());
    const qreal positionY = config->getDouble("position_y", KisScreentoneConfigDefaults::positionY());
    const bool kepSizeSquare = config->getBool("keep_size_square", KisScreentoneConfigDefaults::keepSizeSquare());
    const qreal sizeX = config->getDouble("size_x", KisScreentoneConfigDefaults::sizeX());
    // Ensure that the size y component is equal to the x component if keepSizeSquare is true
    const qreal sizeY = kepSizeSquare ? sizeX : config->getDouble("size_y", KisScreentoneConfigDefaults::sizeY());
    const qreal shearX = config->getDouble("shear_x", KisScreentoneConfigDefaults::shearX());
    const qreal shearY = config->getDouble("shear_y", KisScreentoneConfigDefaults::shearY());
    const qreal rotation = config->getDouble("rotation", KisScreentoneConfigDefaults::rotation());
    QTransform t;
    t.shear(shearX, shearY);
    t.scale(qIsNull(sizeX) ? 0.0 : 1.0 / sizeX, qIsNull(sizeY) ? 0.0 : 1.0 / sizeY);
    t.rotate(rotation);
    t.translate(positionX, positionY);

    KoColor foregroundColor = config->getColor("foreground_color", KisScreentoneConfigDefaults::foregroundColor());
    KoColor backgroundColor = config->getColor("background_color", KisScreentoneConfigDefaults::backgroundColor());
    qreal foregroundOpacity = config->getInt("foreground_opacity", KisScreentoneConfigDefaults::foregroundOpacity()) / 100.0;
    qreal backgroundOpacity = config->getInt("background_opacity", KisScreentoneConfigDefaults::backgroundOpacity()) / 100.0;
    foregroundColor.convertTo(colorSpace);
    backgroundColor.convertTo(colorSpace);
    foregroundColor.setOpacity(foregroundOpacity);
    backgroundColor.setOpacity(backgroundOpacity);

    KisPaintDeviceSP foregroundDevice = new KisPaintDevice(colorSpace, "screentone_generator_foreground_paint_device");
    KisPaintDeviceSP backgroundDevice;
    if (device->colorSpace()->profile()->isLinear()) {
        backgroundDevice = new KisPaintDevice(colorSpace, "screentone_generator_background_paint_device");
    } else {
        backgroundDevice = device;
    }

    foregroundDevice->fill(bounds, foregroundColor);
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 25);
    backgroundDevice->fill(bounds, backgroundColor);
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 50);

    KisSelectionSP selection = new KisSelection(device->defaultBounds());
    KisSequentialIterator it(selection->pixelSelection(), bounds);

    if (!config->getBool("invert", KisScreentoneConfigDefaults::invert())) {
        while (it.nextPixel()) {
            qreal x, y;
            t.map(it.x(), it.y(), &x, &y);

            qreal v = qBound(0.0, brightnessContrastFunction(screentoneFunction(x, y)), 1.0);
            *it.rawData() = 255 - static_cast<quint8>(qRound(v * 255.0));
        }
    } else {
        while (it.nextPixel()) {
            qreal x, y;
            t.map(it.x(), it.y(), &x, &y);

            qreal v = qBound(0.0, brightnessContrastFunction(screentoneFunction(x, y)), 1.0);
            *it.rawData() = static_cast<quint8>(qRound(v * 255.0));
        }
    }
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 25);

    {
        KisPainter gc(backgroundDevice, selection);
        gc.setCompositeOp(COMPOSITE_OVER);
        gc.bitBlt(bounds.topLeft(), foregroundDevice, bounds);
    }
    if (device->colorSpace()->profile()->isLinear()) {
        KisPainter gc(device);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(bounds.topLeft(), backgroundDevice, bounds);
    }
    checkUpdaterInterruptedAndSetPercent(progressUpdater, 100);
}

KisFilterConfigurationSP KisScreentoneGenerator::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    QVariant v;
    config->setProperty("pattern", KisScreentoneConfigDefaults::pattern());
    config->setProperty("shape", KisScreentoneConfigDefaults::shape());
    config->setProperty("interpolation", KisScreentoneConfigDefaults::interpolation());
    v.setValue(KisScreentoneConfigDefaults::foregroundColor());
    config->setProperty("foreground_color", v);
    config->setProperty("foreground_opacity", KisScreentoneConfigDefaults::foregroundOpacity());
    v.setValue(KisScreentoneConfigDefaults::backgroundColor());
    config->setProperty("background_color", v);
    config->setProperty("background_opacity", KisScreentoneConfigDefaults::backgroundOpacity());
    config->setProperty("invert", KisScreentoneConfigDefaults::invert());
    config->setProperty("brightness", KisScreentoneConfigDefaults::brightness());
    config->setProperty("contrast", KisScreentoneConfigDefaults::contrast());
    config->setProperty("position_x", KisScreentoneConfigDefaults::positionX());
    config->setProperty("position_y", KisScreentoneConfigDefaults::positionY());
    config->setProperty("size_x", KisScreentoneConfigDefaults::sizeX());
    config->setProperty("size_y", KisScreentoneConfigDefaults::sizeY());
    config->setProperty("keep_size_square", KisScreentoneConfigDefaults::keepSizeSquare());
    config->setProperty("shear_x", KisScreentoneConfigDefaults::shearX());
    config->setProperty("shear_y", KisScreentoneConfigDefaults::shearY());
    config->setProperty("rotation", KisScreentoneConfigDefaults::rotation());
    return config;
}

KisConfigWidget * KisScreentoneGenerator::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisScreentoneConfigWidget(parent);
}

#include "KisScreentoneGenerator.moc"

