/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISOCIOCONFIGURATION_H
#define KISOCIOCONFIGURATION_H

#include <QString>

class KisOcioConfiguration
{
public:
    enum Mode {
        INTERNAL = 0,
        OCIO_CONFIG,
        OCIO_ENVIRONMENT
    };

public:
    Mode mode = INTERNAL;
    QString configurationPath;
    QString lutPath;
    QString inputColorSpace;
    QString displayDevice;
    QString displayView;
    QString look;
};

#endif // KISOCIOCONFIGURATION_H
