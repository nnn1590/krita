/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2018 Emmet & Eoin O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_tool_utils.h>

#include <KoMixColorsOp.h>
#include <kis_group_layer.h>
#include <kis_transaction.h>
#include <kis_sequential_iterator.h>
#include <kis_properties_configuration.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include "kis_command_utils.h"
#include "kis_processing_applicator.h"

namespace KisToolUtils {

    bool pickColor(KoColor &out_color, KisPaintDeviceSP dev, const QPoint &pos,
                   KoColor const *const blendColor, int radius, int blend, bool pure)
    {
        KIS_ASSERT(dev);

        // Bugfix hack forcing pure on first sample to avoid wrong
        // format blendColor on newly initialized Krita.
        static bool firstTime = true;
        if (firstTime == true) {
            pure = true;
            firstTime = false;
        }

        const KoColorSpace *cs = dev->colorSpace();
        KoColor pickedColor(Qt::transparent, cs);

        // Sampling radius.
        if (!pure && radius > 1) {
            QVector<const quint8*> pixels;
            const int effectiveRadius = radius - 1;

            const QRect pickRect(pos.x() - effectiveRadius, pos.y() - effectiveRadius,
                                 2 * effectiveRadius + 1, 2 * effectiveRadius + 1);
            KisSequentialConstIterator it(dev, pickRect);

            const int radiusSq = pow2(effectiveRadius);

            while (it.nextPixel()) {
                const QPoint realPos(it.x(),  it.y());
                const QPoint pt = realPos - pos;
                if (pow2(pt.x()) + pow2(pt.y()) < radiusSq) {
                    pixels << it.oldRawData();
                }
            }

            const quint8 **cpixels = const_cast<const quint8**>(pixels.constData());
            cs->mixColorsOp()->mixColors(cpixels, pixels.size(), pickedColor.data());
        } else {
            dev->pixel(pos.x(), pos.y(), &pickedColor);
        }
        
        // Color blending.
        if (!pure && blendColor && blend < 100) {
            //Scale from 0..100% to 0..255 range for mixOp weights.
            quint8 blendScaled = static_cast<quint8>(blend * 2.55f);

            const quint8 *colors[2];
            colors[0] = blendColor->data();
            colors[1] = pickedColor.data();
            qint16 weights[2];
            weights[0] = 255 - blendScaled;
            weights[1] = blendScaled;

            const KoMixColorsOp *mixOp = dev->colorSpace()->mixColorsOp();
            mixOp->mixColors(colors, weights, 2, pickedColor.data());
        }

        pickedColor.convertTo(dev->compositionSourceColorSpace());

        bool validColorPicked = pickedColor.opacityU8() != OPACITY_TRANSPARENT_U8;

        if (validColorPicked) {
            out_color = pickedColor;
        }

        return validColorPicked;
    }

    KisNodeSP findNode(KisNodeSP node, const QPoint &point, bool wholeGroup, bool editableOnly)
    {
        KisNodeSP foundNode = 0;
        while (node) {
            KisLayerSP layer = qobject_cast<KisLayer*>(node.data());

            if (!layer || !layer->isEditable()) {
                node = node->prevSibling();
                continue;
            }

            KoColor color(layer->projection()->colorSpace());
            layer->projection()->pixel(point.x(), point.y(), &color);

            KisGroupLayerSP group = dynamic_cast<KisGroupLayer*>(layer.data());

            if ((group && group->passThroughMode()) ||  color.opacityU8() != OPACITY_TRANSPARENT_U8) {
                if (layer->inherits("KisGroupLayer") && (!editableOnly || layer->isEditable())) {
                    // if this is a group and the pixel is transparent, don't even enter it
                    foundNode = findNode(node->lastChild(), point, wholeGroup, editableOnly);
                }
                else {
                    foundNode = !wholeGroup ? node : node->parent();
                }

            }

            if (foundNode) break;

            node = node->prevSibling();
        }

        return foundNode;
    }

    bool clearImage(KisImageSP image, KisNodeSP node, KisSelectionSP selection)
    {
        if(node && node->hasEditablePaintDevice()) {
            KUndo2Command *cmd =
                new KisCommandUtils::LambdaCommand(kundo2_i18n("Clear"),
                    [node, selection] () {
                        KisPaintDeviceSP device = node->paintDevice();

                        KisTransaction transaction(kundo2_noi18n("internal-clear-command"), device);

                        QRect dirtyRect;
                        if (selection) {
                            dirtyRect = selection->selectedRect();
                            device->clearSelection(selection);
                        } else {
                            dirtyRect = device->extent();
                            device->clear();
                        }

                        device->setDirty(dirtyRect);
                        return transaction.endAndTake();
                    });
            KisProcessingApplicator::runSingleCommandStroke(image, cmd);
            return true;
        }
        return false;
    }

    const QString ColorPickerConfig::CONFIG_GROUP_NAME = "tool_color_picker";

    ColorPickerConfig::ColorPickerConfig()
        : toForegroundColor(true)
        , updateColor(true)
        , addColorToCurrentPalette(false)
        , normaliseValues(false)
        , sampleMerged(true)
        , radius(1)
        , blend(100)
    {
    }

    inline QString getConfigKey(bool defaultActivation) {
        return defaultActivation ?
            "ColorPickerDefaultActivation" : "ColorPickerTemporaryActivation";
    }

    void ColorPickerConfig::save(bool defaultActivation) const
    {
        KisPropertiesConfiguration props;
        props.setProperty("toForegroundColor", toForegroundColor);
        props.setProperty("updateColor", updateColor);
        props.setProperty("addPalette", addColorToCurrentPalette);
        props.setProperty("normaliseValues", normaliseValues);
        props.setProperty("sampleMerged", sampleMerged);
        props.setProperty("radius", radius);
        props.setProperty("blend", blend);

        KConfigGroup config =  KSharedConfig::openConfig()->group(CONFIG_GROUP_NAME);

        config.writeEntry(getConfigKey(defaultActivation), props.toXML());
    }

    void ColorPickerConfig::load(bool defaultActivation)
    {
        KisPropertiesConfiguration props;

        KConfigGroup config =  KSharedConfig::openConfig()->group(CONFIG_GROUP_NAME);
        props.fromXML(config.readEntry(getConfigKey(defaultActivation)));

        toForegroundColor = props.getBool("toForegroundColor", true);
        updateColor = props.getBool("updateColor", true);
        addColorToCurrentPalette = props.getBool("addPalette", false);
        normaliseValues = props.getBool("normaliseValues", false);
        sampleMerged = props.getBool("sampleMerged", !defaultActivation ? false : true);
        radius = props.getInt("radius", 1);
        blend = props.getInt("blend", 100);
    }
}
