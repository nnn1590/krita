/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISANIMATIONRENDERINGBENCHMARK_H
#define KISANIMATIONRENDERINGBENCHMARK_H

#include <QtTest>

class KisAnimationRenderingBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
   void testCacheRendering();
};

#endif // KISANIMATIONRENDERINGBENCHMARK_H
