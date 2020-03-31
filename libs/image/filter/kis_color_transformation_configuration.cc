/*
 *  Copyright (c) 2015 Thorsten Zachmann <zachmann@kde.org>
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

#include "filter/kis_color_transformation_configuration.h"

#include <QMutexLocker>
#include <QMap>
#include <QThread>
#include "filter/kis_color_transformation_filter.h"

struct Q_DECL_HIDDEN KisColorTransformationConfiguration::Private {
    Private()
    {}

    ~Private()
    {
        qDeleteAll(colorTransformation);
    }

    // XXX: Threadlocal storage!!!
    QMap<QThread*, KoColorTransformation*> colorTransformation;
    QMutex mutex;
};

KisColorTransformationConfiguration::KisColorTransformationConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(name, version, resourcesInterface)
    , d(new Private())
{
}

KisColorTransformationConfiguration::KisColorTransformationConfiguration(const KisColorTransformationConfiguration &rhs)
    : KisFilterConfiguration(rhs)
    , d(new Private())
{
}

KisColorTransformationConfiguration::~KisColorTransformationConfiguration()
{
    delete d;
}

KisFilterConfigurationSP KisColorTransformationConfiguration::clone() const
{
    return new KisColorTransformationConfiguration(*this);
}

KoColorTransformation* KisColorTransformationConfiguration::colorTransformation(const KoColorSpace *cs, const KisColorTransformationFilter *filter) const
{
    QMutexLocker locker(&d->mutex);
    KoColorTransformation *transformation = d->colorTransformation.value(QThread::currentThread(), 0);
    if (!transformation) {
        KisFilterConfigurationSP config(const_cast<KisColorTransformationConfiguration*>(this));
        transformation = filter->createTransformation(cs, config);
        d->colorTransformation.insert(QThread::currentThread(), transformation);
    }
    locker.unlock();
    return transformation;
}
