/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISBASEOPTION_H
#define KISBASEOPTION_H

#include <kis_types.h>
#include <kritaui_export.h>

class KoResource;
using KoResourceSP = QSharedPointer<KoResource>;

class KisResourcesInterface;
using KisResourcesInterfaceSP = QSharedPointer<KisResourcesInterface>;

/**
 * This is a special base class for all the options that load/save
 * settings into a properties objects and do *not* store the properties
 * themselves. A KisPaintOpOption derived class generates a QWidget for
 * its configuration page. This cannot be created from a KisPaintO[
 *
 * This class adapts your option to allow its easy use with
 * both raw pointers and shared pointers.
 *
 * Motivation:
 * In quite a lot of places we call some options from the KisPaintOpSettings
 * class itself with patting 'this' as a parameter into
 * read/writeOptionSetting(). Conversion of 'this' into a shared pointer is
 * extremely dangerous, and, ideally, should be prohibited. We cannot prohibit
 * it atm, but we still can create a special interface for accepting raw pointers,
 * which will be used automatically, when 'this' is passed.
 */

class KRITAUI_EXPORT KisPaintopPropertiesResourcesBase
{
public:
    virtual ~KisPaintopPropertiesResourcesBase();

    void readOptionSetting(KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface);
    void writeOptionSetting(KisPropertiesConfigurationSP setting) const;

    void readOptionSetting(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface);
    void writeOptionSetting(KisPropertiesConfiguration *settings) const;

    QList<KoResourceSP> prepareLinkedResources(const KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface) const;
    QList<KoResourceSP> prepareLinkedResources(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const;

    QList<KoResourceSP> prepareEmbeddedResources(const KisPropertiesConfigurationSP settings, KisResourcesInterfaceSP resourcesInterface) const;
    QList<KoResourceSP> prepareEmbeddedResources(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const;


protected:
    virtual void readOptionSettingResourceImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) = 0;
    virtual void writeOptionSettingImpl(KisPropertiesConfiguration *settings) const = 0;
    virtual QList<KoResourceSP> prepareLinkedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const = 0;
    virtual QList<KoResourceSP> prepareEmbeddedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const = 0;
};

class KRITAUI_EXPORT KisPaintopPropertiesBase : public KisPaintopPropertiesResourcesBase
{
public:
    void readOptionSetting(KisPropertiesConfigurationSP settings);
    void readOptionSetting(const KisPropertiesConfiguration *settings);

private:
    void readOptionSettingResourceImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) override final;
    QList<KoResourceSP> prepareLinkedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const override final;
    QList<KoResourceSP> prepareEmbeddedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const override final;

protected:
    virtual void readOptionSettingImpl(const KisPropertiesConfiguration *settings) = 0;
};

#endif // KISBASEOPTION_H
