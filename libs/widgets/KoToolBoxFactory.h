/*
 * Copyright (c) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOTOOLBOXFACTORY_H
#define KOTOOLBOXFACTORY_H

#include <KoDockFactoryBase.h>
#include "kritawidgets_export.h"

#include <QString>
#include <QDockWidget>


/**
 * Factory class to create a new KoToolBox that contains the buttons
 * to activate tools.
 */
class KRITAWIDGETS_EXPORT KoToolBoxFactory : public KoDockFactoryBase
{
public:
    explicit KoToolBoxFactory();
    ~KoToolBoxFactory() override;

    QString id() const override;
    KoDockFactoryBase::DockPosition defaultDockPosition() const override;
    QDockWidget* createDockWidget() override;
    bool isCollapsable() const override { return false; }
};

#endif
