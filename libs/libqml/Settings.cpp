/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "Settings.h"
#include <QApplication>


#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kis_paintop_preset.h>
#include "KisResourceServerProvider.h"

#include "Theme.h"
#include "PropertyContainer.h"
#include <kis_config.h>

class Settings::Private
{
public:
    Private() : temporaryFile(false), focusItem(0), theme(0){ }

    QString currentFile;
    bool temporaryFile;
    QQuickItem *focusItem;
    Theme* theme {0};
};

Settings::Settings( QObject* parent )
    : QObject( parent )
    , d( new Private )
{
}

Settings::~Settings()
{
    delete d;
}

void Settings::setTheme(Theme *theme)
{
    d->theme = theme;
    d->theme->setParent(this);
    connect(d->theme, SIGNAL(fontCacheRebuilt()), SIGNAL(themeChanged()));

}


QString Settings::currentFile() const
{
    return d->currentFile;
}

void Settings::setCurrentFile(const QString& fileName)
{
    qApp->processEvents();
    if (fileName != d->currentFile) {
        d->currentFile = fileName;
        emit currentFileChanged();
    }
}

bool Settings::isTemporaryFile() const
{
    return d->temporaryFile;
}

void Settings::setTemporaryFile(bool temp)
{
    if (temp != d->temporaryFile) {
        d->temporaryFile = temp;
        emit temporaryFileChanged();
    }
}

QQuickItem* Settings::focusItem()
{
    return d->focusItem;
}

void Settings::setFocusItem(QQuickItem* item)
{
    if (item != d->focusItem) {
        d->focusItem = item;
        emit focusItemChanged();
    }
}

QObject* Settings::theme() const
{
    return d->theme;
}

QString Settings::themeID() const
{
    if(d->theme)
        return d->theme->id();

    return QString();
}

void Settings::setThemeID(const QString& /*id*/)
{
//    if(!d->theme || id != d->theme->id()) {
//        if(d->theme) {
//            delete d->theme;
//            d->theme = 0;
//        }

//        d->theme = Theme::load(id, this);
//        KSharedConfig::openConfig()->group("General").writeEntry<QString>("theme", id);

//        emit themeChanged();
//    }
}

QObject* Settings::customImageSettings() const
{
    QObject* settings = new PropertyContainer("customImageSettings", qApp);
    KisConfig cfg(false);
    settings->setProperty("Width", cfg.defImageWidth());
    settings->setProperty("Height", cfg.defImageHeight());
    settings->setProperty("Resolution", qRound(cfg.defImageResolution() * 72)); // otherwise we end up with silly floating point numbers
    settings->setProperty("ColorModel", cfg.defColorModel());
    settings->setProperty("ColorDepth", cfg.defaultColorDepth());
    settings->setProperty("ColorProfile", cfg.defColorProfile());
    return settings;
}

