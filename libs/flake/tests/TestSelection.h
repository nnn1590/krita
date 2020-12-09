/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TestSelection_H
#define TestSelection_H

#include <QObject>

class TestSelection : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    // tests
    void testSelectedShapes();
    void testSize();
};

#endif
