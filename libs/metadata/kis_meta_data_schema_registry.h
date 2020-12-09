/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_SCHEMA_REGISTRY_H_
#define _KIS_META_DATA_SCHEMA_REGISTRY_H_

#include <kritametadata_export.h>

class QString;
class QDebug;

namespace KisMetaData
{

class Schema;

class KRITAMETADATA_EXPORT SchemaRegistry
{
    struct Private;
    SchemaRegistry();
    ~SchemaRegistry();
public:
    /**
     * Creates a new schema.
     * @param uri the name of the schema
     * @param prefix the namespace prefix used for this schema
     * @return the schema associated with the uri (it can return 0, if no schema exist
     * for the uri, but the prefix was already used, and it can be an already existing
     * schema if the uri was already included)
     */
    const KisMetaData::Schema* create(const QString & uri, const QString & prefix);
    /**
     * @return the schema for this uri
     */
    const Schema* schemaFromUri(const QString & uri) const;
    /**
     * @return the schema for this prefix
     */
    const Schema* schemaFromPrefix(const QString & prefix) const;
    /**
     * Return an instance of the SchemaRegistry.
     * Creates an instance if that has never happened before and returns
     * the singleton instance.
     * Initialize it with default schemas.
     */
    static KisMetaData::SchemaRegistry* instance();
private:
    Private* const d;
};
}

KRITAMETADATA_EXPORT QDebug operator<<(QDebug debug, const KisMetaData::Schema &c);

#endif
