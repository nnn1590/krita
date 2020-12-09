/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTRESOURCEMODEL_H
#define TESTRESOURCEMODEL_H

#include <QObject>
#include <QtSql>

#include "KisResourceTypes.h"

class KisResourceLocator;
class TestResourceModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testRowCount();
    void testData();
    void testResourceForIndex();
    void testIndexFromResource();
    void testSetInactiveByIndex();
    void testImportResourceFile();
    void testAddResource();
    void testAddTemporaryResource();
    void testUpdateResource();
    void testResourceForId();
    void testResourceForName();
    void testResourceForFileName();
    void testResourceForMD5();
    void testRenameResource();
    void cleanupTestCase();
private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;
    const QString m_resourceType = ResourceType::PaintOpPresets;

};

#endif
