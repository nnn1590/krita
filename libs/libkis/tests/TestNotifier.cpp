/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestNotifier.h"
#include <QTest>

#include <Notifier.h>
#include <KisPart.h>
#include <Document.h>

#include  <sdk/tests/testui.h>

void TestNotifier::testNotifier()
{
    KisPart *part = KisPart::instance();

    Notifier *notifier = new Notifier();
    connect(notifier, SIGNAL(imageCreated(Document*)), SLOT(documentAdded(Document*)), Qt::DirectConnection);

    QVERIFY(!notifier->active());
    notifier->setActive(true);
    QVERIFY(notifier->active());

    KisDocument *doc = part->createDocument();
    part->addDocument(doc);

    QVERIFY(m_document);

}

void TestNotifier::documentAdded(Document *image)
{
    m_document = image;
}

KISTEST_MAIN(TestNotifier)

