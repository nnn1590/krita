/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPEREGISTRY_H
#define KOSHAPEREGISTRY_H

#include <KoGenericRegistry.h>
#include <KoShapeFactoryBase.h>
#include <KoXmlReaderForward.h>

#include "kritaflake_export.h"

class KoShape;
class KoShapeLoadingContext;


/**
 * This singleton class keeps a register of all available flake shapes,
 * or rather, of the factories that applications can use to create flake
 * shape objects.
 */
class KRITAFLAKE_EXPORT KoShapeRegistry : public KoGenericRegistry<KoShapeFactoryBase*>
{
public:
    KoShapeRegistry();
    ~KoShapeRegistry() override;

    /**
     * Return an instance of the KoShapeRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoShapeRegistry *instance();

    /**
     * Add shape factory for a shape that is not a plugin
     * This can be used also if you want to have a shape only in one application
     *
     * @param factory The factory of the shape
     */
    void addFactory(KoShapeFactoryBase *factory);

    /**
     * Use the element to find out which flake plugin can load it, and
     * returns the loaded shape. The element expected is one of
     * 'draw:line', 'draw:frame' / etc.
     *
     * @returns the shape or 0 if no shape could be created. The shape may have as its parent
     *    set a layer which was previously created and stored in the context.
     * @see KoShapeLoadingContext::layer()
     */
    KoShape *createShapeFromXML(const KoXmlElement &element, KoShapeLoadingContext &context) const;

    /**
     * Returns a list of shape factories supporting the specified xml element.
     * @param nameSpace the namespace of the xml element, see KoXmlNS for valid namespaces
     * @param elementName the tag name of the element
     * @return the list of shape factories supporting the specified xml element
     */
    QList<KoShapeFactoryBase*> factoriesForElement(const QString &nameSpace, const QString &elementName);

private:
    KoShapeRegistry(const KoShapeRegistry&);
    KoShapeRegistry operator=(const KoShapeRegistry&);

    class Private;
    Private * const d;
};

#endif
