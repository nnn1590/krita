/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PREDEFINED_BRUSH_FACTORY_H
#define __KIS_PREDEFINED_BRUSH_FACTORY_H

#include <QString>
#include <QDomElement>

#include "kis_brush_factory.h"
#include "kis_brush.h"


class KisPredefinedBrushFactory : public KisBrushFactory
{
public:
    KisPredefinedBrushFactory(const QString &brushType);

    QString id() const override;
    KisBrushSP createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface) override;

private:
    const QString m_id;
};

#endif /* __KIS_PREDEFINED_BRUSH_FACTORY_H */
