/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CATEGORIES_MAPPER_TEST_H
#define __KIS_CATEGORIES_MAPPER_TEST_H

#include <QtTest>

#include "kis_categories_mapper.h"
#include <QTest>

class KisCategoriesMapperTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAddRemoveCategories();
    void testAddRemoveEntries();
    void testRemoveNonEmptyCategories();
    void testChangingItem();
};

#endif /* __KIS_CATEGORIES_MAPPER_TEST_H */
