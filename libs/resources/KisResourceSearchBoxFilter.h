/*
 *  Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
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
#ifndef KIS_RESOURCE_SEARCH_BOX_FILTER_H
#define KIS_RESOURCE_SEARCH_BOX_FILTER_H


#include "kritaresources_export.h"

#include <QString>
#include <QScopedPointer>

/**
 * XXX: Apidox
 * 
 */
class KRITARESOURCES_EXPORT KisResourceSearchBoxFilter
{

public:

    KisResourceSearchBoxFilter();
    ~KisResourceSearchBoxFilter();
    void setFilter(const QString& filter);
    bool matchesResource(const QString& resourceName);
    bool isEmpty();

private:

    void initializeFilterData();
    void clearFilterData();

    class Private;
    QScopedPointer<Private> d;

};


#endif
