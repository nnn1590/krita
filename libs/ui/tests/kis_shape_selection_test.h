/*
 *  Copyright (c) 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SHAPE_SELETION_TEST_H
#define KIS_SHAPE_SELETION_TEST_H

#include <QtTest>

class KisShapeSelectionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testAddChild();

    void testUndoFlattening();

    void testHistoryOnFlattening();
};

#endif

