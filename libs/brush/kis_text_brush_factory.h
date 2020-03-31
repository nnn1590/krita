/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_TEXT_BRUSH_FACTORY
#define KIS_TEXT_BRUSH_FACTORY

#include <QString>
#include <QDomElement>
#include "kis_brush_factory.h"
#include "kis_brush.h"

/**
 * A brush factory can create a new brush instance based
 * on a properties object that contains a serialized representation
 * of the object.
 */
class BRUSH_EXPORT KisTextBrushFactory : public KisBrushFactory
{

public:

    KisTextBrushFactory() {}
    ~KisTextBrushFactory() override {}

    QString id() const override {
        return "kis_text_brush";
    }


    /**
     * Create a new brush from the given data or return an existing KisBrush
     * object. If this call leads to the creation of a resource, it should be
     * added to the resource provider, too.
     */
    KisBrushSP createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface) override;


};

#endif
