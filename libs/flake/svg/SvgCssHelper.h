/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGCSSHELPER_H
#define SVGCSSHELPER_H

#include <QStringList>

#include <KoXmlReaderForward.h>

class SvgCssHelper
{
public:
    SvgCssHelper();
    ~SvgCssHelper();

    /// Parses css style sheet in given xml element
    void parseStylesheet(const KoXmlElement &);

    /**
     * Matches css styles to given xml element and returns them
     * @param element the element to match styles for
     * @return list of matching css styles sorted by priority
     */
    QStringList matchStyles(const KoXmlElement &element) const;

private:
    class Private;
    Private * const d;
};

#endif // SVGCSSHELPER_H
