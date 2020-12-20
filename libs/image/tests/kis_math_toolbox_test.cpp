/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_math_toolbox_test.h"

#include <QTest>
#include "kis_math_toolbox.h"

void KisMathToolboxTest::testCreation()
{
    KisMathToolbox tb;
    Q_UNUSED(tb);
}


QTEST_MAIN(KisMathToolboxTest)
