/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_FILTER_REGISTRY_H_
#define KIS_FILTER_REGISTRY_H_

#include <QObject>

#include "kis_filter.h"
#include "kis_types.h"
#include "KoGenericRegistry.h"

#include <kritaimage_export.h>

class QString;
class KisFilterConfiguration;

class KRITAIMAGE_EXPORT KisFilterRegistry : public QObject, public KoGenericRegistry<KisFilterSP>
{

    Q_OBJECT

public:

    ~KisFilterRegistry() override;

    static KisFilterRegistry* instance();
    void add(KisFilterSP item);
    void add(const QString &id, KisFilterSP item);

Q_SIGNALS:

    void filterAdded(QString id);

private:

    KisFilterRegistry(QObject *parent);
    KisFilterRegistry(const KisFilterRegistry&);
    KisFilterRegistry operator=(const KisFilterRegistry&);

};

#endif // KIS_FILTERSPACE_REGISTRY_H_
