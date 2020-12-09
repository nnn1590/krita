/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef IMAGESHAPE_FACTORY_H
#define IMAGESHAPE_FACTORY_H

// Calligra
#include <KoShapeFactoryBase.h>

class KoShape;

class ImageShapeFactory : public KoShapeFactoryBase
{

public:
    /// constructor
    ImageShapeFactory();
    ~ImageShapeFactory() override {}

    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;
    bool supports(const KoXmlElement &e, KoShapeLoadingContext &context) const override;
    QList<KoShapeConfigWidgetBase *> createShapeOptionPanels() override;
};

#endif
