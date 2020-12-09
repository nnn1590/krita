/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_FILLLAYER_H
#define LIBKIS_FILLLAYER_H

#include <QObject>
#include "Node.h"
#include <InfoObject.h>
#include <Selection.h>

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"
/**
 * @brief The FillLayer class
 * A fill layer is much like a filter layer in that it takes a name
 * and filter. It however specializes in filters that fill the whole canvas,
 * such as a pattern or full color fill.
 */
class KRITALIBKIS_EXPORT FillLayer : public Node
{
    Q_OBJECT
    Q_DISABLE_COPY(FillLayer)

public:
    /**
     * @brief FillLayer Create a new fill layer with the given generator plugin
     * @param image the image this fill layer will belong to
     * @param name "pattern" or "color"
     * @param filterConfig a configuration object appropriate to the given generator plugin
     *
     * For a "pattern" fill layer, the InfoObject can contain a single "pattern" parameter with
     * the name of a pattern as known to the resource system: "pattern" = "Cross01.pat".
     *
     * For a "color" fill layer, the InfoObject can contain a single "color" parameter with
     * a QColor, a string that QColor can parse (see https://doc.qt.io/qt-5/qcolor.html#setNamedColor)
     * or an XML description of the color, which can be derived from a @see ManagedColor.
     *
     * @param selection a selection object, can be empty
     * @param parent
     */
    explicit FillLayer(KisImageSP image, QString name, KisFilterConfigurationSP filterConfig, Selection &selection, QObject *parent = 0);
    explicit FillLayer(KisGeneratorLayerSP layer, QObject *parent = 0);
    ~FillLayer() override;
public Q_SLOTS:

    /**
     * @brief type Krita has several types of nodes, split in layers and masks. Group
     * layers can contain other layers, any layer can contain masks.
     *
     * @return The type of the node. Valid types are:
     * <ul>
     *  <li>paintlayer
     *  <li>grouplayer
     *  <li>filelayer
     *  <li>filterlayer
     *  <li>filllayer
     *  <li>clonelayer
     *  <li>vectorlayer
     *  <li>transparencymask
     *  <li>filtermask
     *  <li>transformmask
     *  <li>selectionmask
     *  <li>colorizemask
     * </ul>
     *
     * If the Node object isn't wrapping a valid Krita layer or mask object, and
     * empty string is returned.
     */
    virtual QString type() const override;

    /**
     * @brief setGenerator set the given generator for this fill layer
     * @param generatorName "pattern" or "color"
     * @param filterConfig a configuration object appropriate to the given generator plugin
     * @return true if the generator was correctly created and set on the layer
     */
    bool setGenerator(const QString &generatorName, InfoObject *filterConfig);
    QString generatorName();

    InfoObject *filterConfig();
};

#endif // LIBKIS_FILLLAYER_H
