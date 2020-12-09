/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EMBEDDED_PATTERN_MANAGER_H
#define __KIS_EMBEDDED_PATTERN_MANAGER_H

#include <kritapaintop_export.h>
#include <kis_properties_configuration.h>
#include <KoPattern.h>

class KoAbstractGradient;
class KisResourcesInterface;
using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

class PAINTOP_EXPORT KisEmbeddedPatternManager
{
public:
    static void saveEmbeddedPattern(KisPropertiesConfigurationSP setting, const KoPatternSP pattern);
    static KoPatternSP loadEmbeddedPattern(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);

    static KoPatternSP tryFetchPattern(const KisPropertiesConfigurationSP setting, KisResourcesInterfaceSP resourcesInterface);
private:
    struct Private;
};

#endif /* __KIS_EMBEDDED_PATTERN_MANAGER_H */
