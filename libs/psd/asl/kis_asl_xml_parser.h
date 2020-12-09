/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_XML_PARSER_H
#define __KIS_ASL_XML_PARSER_H

#include "kritapsd_export.h"

class QDomDocument;
class KisAslObjectCatcher;


class KRITAPSD_EXPORT KisAslXmlParser
{
public:
    void parseXML(const QDomDocument &doc, KisAslObjectCatcher &catcher);
};

#endif /* __KIS_ASL_XML_PARSER_H */
