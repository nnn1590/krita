/*
 *  Copyright (c) 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "zoomcontroller_test.h"

#include <QTest>
#include <QCoreApplication>

#include <kactioncollection.h>
#include <WidgetsDebug.h>

#include "KoCanvasControllerWidget.h"
#include "KoZoomHandler.h"
#include "KoZoomController.h"

void zoomcontroller_test::testApi()
{
    KoZoomHandler zoomHandler;
    KoZoomController zoomController(new KoCanvasControllerWidget(0, 0), &zoomHandler, new KActionCollection(this));
    Q_UNUSED(zoomController);

}

QTEST_MAIN(zoomcontroller_test)
