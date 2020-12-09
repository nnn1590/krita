/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRequiredResourcesOperators.h"

#include <KisLocalStrokeResources.h>
#include <QApplication>
#include <QThread>


bool KisRequiredResourcesOperators::detail::isLocalResourcesStorage(KisResourcesInterfaceSP resourcesInterface)
{
    return resourcesInterface.dynamicCast<KisLocalStrokeResources>();
}

void KisRequiredResourcesOperators::detail::assertInGuiThread()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(QThread::currentThread() == qApp->thread());
}

KisResourcesInterfaceSP KisRequiredResourcesOperators::detail::createLocalResourcesStorage(const QList<KoResourceSP> &resources)
{
    return QSharedPointer<KisLocalStrokeResources>::create(resources);
}
