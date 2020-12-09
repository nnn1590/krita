/*
 * resourcemanager.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "resourcemanager.h"

#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QThread>

#include <QMessageBox>
#include <QGlobalStatic>
#include <QStandardPaths>

#include <klocalizedstring.h>
#include <KoResourcePaths.h>
#include <kpluginfactory.h>

#include <KoFileDialog.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>
#include <kis_action.h>
#include <KisViewManager.h>
#include <KisResourceServerProvider.h>
#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <KisBrushServerProvider.h>
#include <kis_paintop_settings.h>
#include <KisPaintopSettingsIds.h>
#include <krita_container_utils.h>

#include "config-seexpr.h"

#include "dlg_bundle_manager.h"
#include "dlg_create_bundle.h"

class ResourceManager::Private {

public:

    Private()
    {
        brushServer = KisBrushServerProvider::instance()->brushServer();
        paintopServer = KisResourceServerProvider::instance()->paintOpPresetServer();
        gradientServer = KoResourceServerProvider::instance()->gradientServer();
        patternServer = KoResourceServerProvider::instance()->patternServer();
        paletteServer = KoResourceServerProvider::instance()->paletteServer();
        workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
        gamutMaskServer = KoResourceServerProvider::instance()->gamutMaskServer();
#if defined HAVE_SEEXPR
        seExprScriptServer = KoResourceServerProvider::instance() ->seExprScriptServer();
#endif
    }

    KoResourceServer<KisBrush>* brushServer;
    KisPaintOpPresetResourceServer * paintopServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoPattern> *patternServer;
    KoResourceServer<KoColorSet>* paletteServer;
    KoResourceServer<KisWorkspaceResource>* workspaceServer;
    KoResourceServer<KoGamutMask>* gamutMaskServer;
#if defined HAVE_SEEXPR
    KoResourceServer<KisSeExprScript>* seExprScriptServer;
#endif
};

K_PLUGIN_FACTORY_WITH_JSON(ResourceManagerFactory, "kritaresourcemanager.json", registerPlugin<ResourceManager>();)

ResourceManager::ResourceManager(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
    , d(new Private())
{
    KisAction *action = new KisAction(i18n("Import Bundles..."), this);
    addAction("import_bundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportBundles()));

    action = new KisAction(i18n("Import Brushes..."), this);
    addAction("import_brushes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportBrushes()));

    action = new KisAction(i18n("Import Gradients..."), this);
    addAction("import_gradients", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportGradients()));

    action = new KisAction(i18n("Import Palettes..."), this);
    addAction("import_palettes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportPalettes()));

    action = new KisAction(i18n("Import Patterns..."), this);
    addAction("import_patterns", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportPatterns()));

    action = new KisAction(i18n("Import Presets..."), this);
    addAction("import_presets", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportPresets()));

    action = new KisAction(i18n("Import Workspaces..."), this);
    addAction("import_workspaces", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportWorkspaces()));

    action = new KisAction(i18n("Create Resource Bundle..."), this);
    addAction("create_bundle", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCreateBundle()));

    action = new KisAction(i18n("Manage Resources..."), this);
    addAction("manage_bundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageBundles()));

}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::slotCreateBundle()
{
    DlgCreateBundle dlgCreateBundle;
    if (dlgCreateBundle.exec() != QDialog::Accepted) {
        return;
    }
    saveBundle(dlgCreateBundle);
}

KoResourceBundleSP ResourceManager::saveBundle(const DlgCreateBundle &dlgCreateBundle)
{
    QString bundlePath =  dlgCreateBundle.saveLocation() + "/" + dlgCreateBundle.bundleName() + ".bundle";

    KoResourceBundleSP newBundle(new KoResourceBundle(bundlePath));

    newBundle->setMetaData("name", dlgCreateBundle.bundleName());
    newBundle->setMetaData("author", dlgCreateBundle.authorName());
    newBundle->setMetaData("email", dlgCreateBundle.email());
    newBundle->setMetaData("license", dlgCreateBundle.license());
    newBundle->setMetaData("website", dlgCreateBundle.website());
    newBundle->setMetaData("description", dlgCreateBundle.description());
    newBundle->setThumbnail(dlgCreateBundle.previewImage());

    /*
    QStringList res = dlgCreateBundle.selectedBrushes();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP res = d->brushServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::Brushes, res->filename(), d->brushServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedGradients();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP res = d->gradientServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::Gradients, res->filename(), d->gradientServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPalettes();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP res = d->paletteServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::Palettes, res->filename(), d->paletteServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPatterns();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP res = d->patternServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::Patterns, res->filename(), d->patternServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPresets();
    Q_FOREACH (const QString &r, res) {
        KisPaintOpPresetSP preset = d->paintopServer->resourceByFilename(r);
        KoResourceSP res = preset;
        newBundle->addResource(ResourceType::PaintOpPresets, res->filename(), d->paintopServer->assignedTagsList(res), res->md5());
        KisPaintOpSettingsSP settings = preset->settings();

        QStringList requiredFiles = settings->getStringList(KisPaintOpUtils::RequiredBrushFilesListTag);
        requiredFiles << settings->getString(KisPaintOpUtils::RequiredBrushFileTag);
        KritaUtils::makeContainerUnique(requiredFiles);

        Q_FOREACH (const QString &brushFile, requiredFiles) {
            KisBrushSP brush = d->brushServer->resourceByFilename(brushFile);
            if (brush) {
                newBundle->addResource(ResourceType::Brushes, brushFile, d->brushServer->assignedTagsList(brush), brush->md5());
            } else {
                qWarning() << "There is no brush with name" << brushFile;
            }
        }
    }

    res = dlgCreateBundle.selectedWorkspaces();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP res = d->workspaceServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::Workspaces, res->filename(), d->workspaceServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedGamutMasks();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP res = d->gamutMaskServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::GamutMasks, res->filename(), d->gamutMaskServer->assignedTagsList(res), res->md5());
    }

#if defined HAVE_SEEXPR
    res = dlgCreateBundle.selectedSeExprScripts();
    Q_FOREACH (const QString &r, res) {
        KoResourceSP *res = d->seExprScriptServer->resourceByFilename(r);
        newBundle->addResource(ResourceType::SeExprScripts, res->filename(), d->gamutMaskServer->assignedTagsList(res), res->md5());
    }
#endif
    */

    newBundle->setMetaData("fileName", bundlePath);
    newBundle->setMetaData("created", QDateTime::currentDateTime().toOffsetFromUtc(0).toString(Qt::ISODate));

    if (!newBundle->save()) {
        QMessageBox::critical(viewManager()->mainWindow(), i18nc("@title:window", "Krita"), i18n("Could not create the new bundle."));
    }
    else {
//        if (QDir(KoResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation()) != QDir(QFileInfo(bundlePath).path())) {
//            newBundle->setFilename(KoResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + "/" + dlgCreateBundle.bundleName() + ".bundle");
//        }
//        if (KoResourceBundleServerProvider::instance()->resourceBundleServer()->resourceByName(newBundle->name())) {
//            KoResourceBundleServerProvider::instance()->resourceBundleServer()->removeResourceFromServer(
//                        KoResourceBundleServerProvider::instance()->resourceBundleServer()->resourceByName(newBundle->name()));
//        }
//        KoResourceBundleServerProvider::instance()->resourceBundleServer()->addResource(newBundle, true);
        newBundle->load();
    }

    return newBundle;
}

void ResourceManager::slotManageBundles()
{
    QPointer<DlgBundleManager> dlg = new DlgBundleManager();
    dlg->exec();
}

QStringList ResourceManager::importResources(const QString &title, const QStringList &mimes) const
{
    KoFileDialog dialog(viewManager()->mainWindow(), KoFileDialog::OpenFiles, "krita_resources");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setCaption(title);
    dialog.setMimeTypeFilters(mimes);
    return dialog.filenames();
}

void ResourceManager::slotImportBrushes()
{
    QStringList resources = importResources(i18n("Import Brushes"), QStringList() << "image/x-gimp-brush"
                                   << "image/x-gimp-x-gimp-brush-animated"
                                   << "image/x-adobe-brushlibrary"
                                   << "image/png"
                                   << "image/svg+xml");
    Q_FOREACH (const QString &res, resources) {
        d->brushServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportPresets()
{
    QStringList resources = importResources(i18n("Import Presets"), QStringList() << "application/x-krita-paintoppreset");
    Q_FOREACH (const QString &res, resources) {
        d->paintopServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportGradients()
{
    QStringList resources = importResources(i18n("Import Gradients"), QStringList() << "image/svg+xml"
                                   << "application/x-gimp-gradient");
    Q_FOREACH (const QString &res, resources) {
        d->gradientServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportBundles()
{
//    QStringList resources = importResources(i18n("Import Bundles"), QStringList() << "application/x-krita-bundle");
//    Q_FOREACH (const QString &res, resources) {
//        KoResourceBundleSP bundle = KoResourceBundleServerProvider::instance()->resourceBundleServer()->createResource(res);
//        bundle->load();
//        if (bundle->valid()) {
//            if (!bundle->install()) {
//                QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not install the resources for bundle %1.", res));
//            }
//        }
//        else {
//            QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not load bundle %1.", res));
//        }

//        QFileInfo fi(res);
//        QString newFilename = KoResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + fi.baseName() + bundle->defaultFileExtension();
//        QFileInfo fileInfo(newFilename);

//        int i = 1;
//        while (fileInfo.exists()) {
//            fileInfo.setFile(KoResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + fi.baseName() + QString("%1").arg(i) + bundle->defaultFileExtension());
//            i++;
//        }
//        bundle->setFilename(fileInfo.filePath());
//        QFile::copy(res, newFilename);
//        KoResourceBundleServerProvider::instance()->resourceBundleServer()->addResource(bundle, false);
//    }
}

void ResourceManager::slotImportPatterns()
{
    QStringList resources = importResources(i18n("Import Patterns"), QStringList() << "image/png"
                                   << "image/svg+xml"
                                   << "application/x-gimp-pattern"
                                   << "image/jpeg"
                                   << "image/tiff"
                                   << "image/bmp"
                                   << "image/xpg");
    Q_FOREACH (const QString &res, resources) {
        d->patternServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportPalettes()
{
    QStringList resources = importResources(i18n("Import Palettes"), QStringList() << "image/x-gimp-color-palette");
    Q_FOREACH (const QString &res, resources) {
        d->paletteServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportWorkspaces()
{
    QStringList resources = importResources(i18n("Import Workspaces"), QStringList() << "application/x-krita-workspace");
    Q_FOREACH (const QString &res, resources) {
        d->workspaceServer->importResourceFile(res);
    }
}

#include "resourcemanager.moc"
