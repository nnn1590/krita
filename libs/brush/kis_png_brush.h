/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_PNG_BRUSH_
#define KIS_PNG_BRUSH_

#include "KisColorfulBrush.h"

class BRUSH_EXPORT  KisPngBrush : public KisColorfulBrush
{
public:
    /// Construct brush to load filename later as brush
    KisPngBrush(const QString& filename);
    KisPngBrush(const KisPngBrush &rhs);
    KoResourceSP clone() const override;
    KisPngBrush &operator=(const KisPngBrush &rhs) = delete;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice *dev) const override;

    QString defaultFileExtension() const override;
    void toXML(QDomDocument& d, QDomElement& e) const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Brushes, ResourceSubType::PngBrushes);
    }

};

#endif
