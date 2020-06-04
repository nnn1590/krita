/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 * Copyright (C) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisApplication.h"

#include <stdlib.h>
#ifdef Q_OS_WIN
#include <windows.h>
#include <tchar.h>
#endif

#ifdef Q_OS_MACOS
#include "osx.h"
#endif

#include <QStandardPaths>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QStringList>
#include <QStyle>
#include <QStyleFactory>
#include <QSysInfo>
#include <QTimer>
#include <QWidget>
#include <QImageReader>
#include <QImageWriter>
#include <QThread>

#include <klocalizedstring.h>
#include <kdesktopfile.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <KoDockRegistry.h>
#include <KoToolRegistry.h>
#include <KoColorSpaceRegistry.h>
#include <KoPluginLoader.h>
#include <KoShapeRegistry.h>
#include <KoDpi.h>
#include "KoConfig.h"
#include <KoResourcePaths.h>
#include <KisMimeDatabase.h>
#include "thememanager.h"
#include "KisDocument.h"
#include "KisMainWindow.h"
#include "KisAutoSaveRecoveryDialog.h"
#include "KisPart.h"
#include <kis_icon.h>
#include "kis_splash_screen.h"
#include "kis_config.h"
#include "flake/kis_shape_selection.h"
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <generator/kis_generator.h>
#include <brushengine/kis_paintop_registry.h>
#include <kis_meta_data_io_backend.h>
#include "kisexiv2/kis_exiv2.h"
#include "KisApplicationArguments.h"
#include <kis_debug.h>
#include "kis_action_registry.h"
#include <KoResourceServer.h>
#include <KisResourceServerProvider.h>
#include <KoResourceServerProvider.h>
#include "kis_image_barrier_locker.h"
#include "opengl/kis_opengl.h"
#include "kis_spin_box_unit_manager.h"
#include "kis_document_aware_spin_box_unit_manager.h"
#include "KisViewManager.h"
#include <KisUsageLogger.h>

#include <KritaVersionWrapper.h>
#include <dialogs/KisSessionManagerDialog.h>

#include <KisResourceCacheDb.h>
#include <KisResourceLocator.h>
#include <KisResourceLoader.h>
#include <KisResourceLoaderRegistry.h>

#include <kis_gbr_brush.h>
#include <kis_png_brush.h>
#include <kis_svg_brush.h>
#include <kis_imagepipe_brush.h>
#include <KoColorSet.h>
#include <KoSegmentGradient.h>
#include <KoStopGradient.h>
#include <KoPattern.h>
#include <kis_workspace_resource.h>
#include <KisSessionResource.h>
#include <resources/KoSvgSymbolCollectionResource.h>

#include "widgets/KisScreenColorPicker.h"
#include "KisDlgInternalColorSelector.h"

#include <dialogs/KisAsyncAnimationFramesSaveDialog.h>
#include <kis_image_animation_interface.h>
#include "kis_file_layer.h"
#include "kis_group_layer.h"
#include "kis_node_commands_adapter.h"

#include <kis_psd_layer_style.h>

namespace {
const QTime appStartTime(QTime::currentTime());
}

class KisApplication::Private
{
public:
    Private() {}
    QPointer<KisSplashScreen> splashScreen;
    KisAutoSaveRecoveryDialog *autosaveDialog {0};
    QPointer<KisMainWindow> mainWindow; // The first mainwindow we create on startup
    bool batchRun {false};
    QVector<QByteArray> earlyRemoteArguments;
};

class KisApplication::ResetStarting
{
public:
    ResetStarting(KisSplashScreen *splash, int fileCount)
        : m_splash(splash)
        , m_fileCount(fileCount)
    {
    }

    ~ResetStarting()  {

        if (m_splash) {
            m_splash->hide();
        }
    }

    QPointer<KisSplashScreen> m_splash;
    int m_fileCount;
};


KisApplication::KisApplication(const QString &key, int &argc, char **argv)
    : QtSingleApplication(key, argc, argv)
    , d(new Private)
{
#ifdef Q_OS_MACOS
    setMouseCoalescingEnabled(false);
#endif

    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath());

    setApplicationDisplayName("Krita");
    setApplicationName("krita");
    // Note: Qt docs suggest we set this, but if we do, we get resource paths of the form of krita/krita, which is weird.
    //    setOrganizationName("krita");
    setOrganizationDomain("krita.org");

    QString version = KritaVersionWrapper::versionString(true);
    setApplicationVersion(version);
    setWindowIcon(KisIconUtils::loadIcon("krita"));



    if (qgetenv("KRITA_NO_STYLE_OVERRIDE").isEmpty()) {
        QStringList styles = QStringList() << "breeze" << "fusion" << "plastique";
        if (!styles.contains(style()->objectName().toLower())) {
            Q_FOREACH (const QString & style, styles) {
                if (!setStyle(style)) {
                    qDebug() << "No" << style << "available.";
                }
                else {
                    qDebug() << "Set style" << style;
                    break;
                }
            }
        }

        // if style is set from config, try to load that
        KisConfig cfg(true);
        QString widgetStyleFromConfig = cfg.widgetStyle();
        if(widgetStyleFromConfig != "") {
            qApp->setStyle(widgetStyleFromConfig);
        }

    }
    else {
        qDebug() << "Style override disabled, using" << style()->objectName();
    }


}

#if defined(Q_OS_WIN) && defined(ENV32BIT)
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL isWow64()
{
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
                GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

    if(0 != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}
#endif

void KisApplication::initializeGlobals(const KisApplicationArguments &args)
{
    int dpiX = args.dpiX();
    int dpiY = args.dpiY();
    if (dpiX > 0 && dpiY > 0) {
        KoDpi::setDPI(dpiX, dpiY);
    }
}

void KisApplication::addResourceTypes()
{
    //    qDebug() << "addResourceTypes();";
    // All Krita's resource types
    KoResourcePaths::addResourceType("markers", "data", "/styles/");
    KoResourcePaths::addResourceType("kis_pics", "data", "/pics/");
    KoResourcePaths::addResourceType("kis_images", "data", "/images/");
    KoResourcePaths::addResourceType("metadata_schema", "data", "/metadata/schemas/");
    KoResourcePaths::addResourceType(ResourceType::Brushes, "data", "/brushes/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("kis_taskset", "data", "/taskset/");
    KoResourcePaths::addResourceType("gmic_definitions", "data", "/gmic/");
    KoResourcePaths::addResourceType("kis_resourcebundles", "data", "/bundles/");
    KoResourcePaths::addResourceType("kis_defaultpresets", "data", "/defaultpresets/");
    KoResourcePaths::addResourceType(ResourceType::PaintOpPresets, "data", "/paintoppresets/");
    KoResourcePaths::addResourceType(ResourceType::Workspaces, "data", "/workspaces/");
    KoResourcePaths::addResourceType(ResourceType::WindowLayouts, "data", "/windowlayouts/");
    KoResourcePaths::addResourceType(ResourceType::Sessions, "data", "/sessions/");
    KoResourcePaths::addResourceType("psd_layer_style_collections", "data", "/asl");
    KoResourcePaths::addResourceType(ResourceType::Patterns, "data", "/patterns/", true);
    KoResourcePaths::addResourceType(ResourceType::Gradients, "data", "/gradients/");
    KoResourcePaths::addResourceType(ResourceType::Gradients, "data", "/gradients/", true);
    KoResourcePaths::addResourceType(ResourceType::Palettes, "data", "/palettes/", true);
    KoResourcePaths::addResourceType("kis_shortcuts", "data", "/shortcuts/");
    KoResourcePaths::addResourceType("kis_actions", "data", "/actions");
    KoResourcePaths::addResourceType("kis_actions", "data", "/pykrita");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/color/icc");
    KoResourcePaths::addResourceType("icc_profiles", "data", "/profiles/");
    KoResourcePaths::addResourceType(ResourceType::FilterEffects, "data", "/effects/");
    KoResourcePaths::addResourceType("tags", "data", "/tags/");
    KoResourcePaths::addResourceType("templates", "data", "/templates");
    KoResourcePaths::addResourceType("pythonscripts", "data", "/pykrita");
    KoResourcePaths::addResourceType(ResourceType::Symbols, "data", "/symbols");
    KoResourcePaths::addResourceType("preset_icons", "data", "/preset_icons");
    KoResourcePaths::addResourceType(ResourceType::GamutMasks, "data", "/gamutmasks/", true);

    //    // Extra directories to look for create resources. (Does anyone actually use that anymore?)
    //    KoResourcePaths::addResourceDir(ResourceType::Gradients, "/usr/share/create/gradients/gimp");
    //    KoResourcePaths::addResourceDir(ResourceType::Gradients, QDir::homePath() + QString("/.create/gradients/gimp"));
    //    KoResourcePaths::addResourceDir(ResourceType::Patterns, "/usr/share/create/patterns/gimp");
    //    KoResourcePaths::addResourceDir(ResourceType::Patterns, QDir::homePath() + QString("/.create/patterns/gimp"));
    //    KoResourcePaths::addResourceDir(ResourceType::Brushes, "/usr/share/create/brushes/gimp");
    //    KoResourcePaths::addResourceDir(ResourceType::Brushes, QDir::homePath() + QString("/.create/brushes/gimp"));
    //    KoResourcePaths::addResourceDir(ResourceType::Palettes, "/usr/share/create/swatches");
    //    KoResourcePaths::addResourceDir(ResourceType::Palettes, QDir::homePath() + QString("/.create/swatches"));

    // Make directories for all resources we can save, and tags
    QDir d;
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tags/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/asl/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bundles/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/brushes/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/gradients/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/paintoppresets/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/palettes/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/patterns/");
    // between 4.2.x and 4.3.0 there was a change from 'taskset' to 'tasksets'
    // so to make older resource folders compatible with the new version, let's rename the folder
    // so no tasksets are lost.
    if (d.exists(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/taskset/")) {
        d.rename(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/taskset/",
                 QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasksets/");
    }
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tasksets/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/workspaces/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/input/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/pykrita/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/symbols/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/color-schemes/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/preset_icons/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/preset_icons/tool_icons/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/preset_icons/emblem_icons/");
    d.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/gamutmasks/");
}

bool KisApplication::registerResources()
{
    KisResourceLoaderRegistry *reg = KisResourceLoaderRegistry::instance();

    reg->add(new KisResourceLoader<KisPaintOpPreset>(ResourceType::PaintOpPresets, ResourceType::PaintOpPresets, i18n("Brush presets"), QStringList() << "application/x-krita-paintoppreset"));

    reg->add(new KisResourceLoader<KisGbrBrush>(ResourceSubType::GbrBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush"));
    reg->add(new KisResourceLoader<KisImagePipeBrush>(ResourceSubType::GihBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/x-gimp-brush-animated"));
    reg->add(new KisResourceLoader<KisSvgBrush>(ResourceSubType::SvgBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<KisPngBrush>(ResourceSubType::PngBrushes, ResourceType::Brushes, i18n("Brush tips"), QStringList() << "image/png"));

    reg->add(new KisResourceLoader<KoSegmentGradient>(ResourceSubType::SegmentedGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-gimp-gradient"));
    reg->add(new KisResourceLoader<KoStopGradient>(ResourceSubType::StopGradients, ResourceType::Gradients, i18n("Gradients"), QStringList() << "application/x-karbon-gradient" << "image/svg+xml"));

    reg->add(new KisResourceLoader<KoColorSet>(ResourceType::Palettes, ResourceType::Palettes, i18n("Palettes"),
                                     QStringList() << KisMimeDatabase::mimeTypeForSuffix("kpl")
                                               << KisMimeDatabase::mimeTypeForSuffix("gpl")
                                               << KisMimeDatabase::mimeTypeForSuffix("pal")
                                               << KisMimeDatabase::mimeTypeForSuffix("act")
                                               << KisMimeDatabase::mimeTypeForSuffix("aco")
                                               << KisMimeDatabase::mimeTypeForSuffix("css")
                                               << KisMimeDatabase::mimeTypeForSuffix("colors")
                                               << KisMimeDatabase::mimeTypeForSuffix("xml")
                                               << KisMimeDatabase::mimeTypeForSuffix("sbz")));

    QList<QByteArray> src = QImageReader::supportedMimeTypes();
    QStringList allImageMimes;
    Q_FOREACH(const QByteArray ba, src) {
        if (QImageWriter::supportedMimeTypes().contains(ba)) {
            allImageMimes << QString::fromUtf8(ba);
        }
    }
    allImageMimes << KisMimeDatabase::mimeTypeForSuffix("pat");

    reg->add(new KisResourceLoader<KoPattern>(ResourceType::Patterns, ResourceType::Patterns, i18n("Patterns"), allImageMimes));
    reg->add(new KisResourceLoader<KisWorkspaceResource>(ResourceType::Workspaces, ResourceType::Workspaces, i18n("Workspaces"), QStringList() << "application/x-krita-workspace"));
    reg->add(new KisResourceLoader<KoSvgSymbolCollectionResource>(ResourceType::Symbols, ResourceType::Symbols, i18n("SVG symbol libraries"), QStringList() << "image/svg+xml"));
    reg->add(new KisResourceLoader<KisWindowLayoutResource>(ResourceType::WindowLayouts, ResourceType::WindowLayouts, i18n("Window layouts"), QStringList() << "application/x-krita-windowlayout"));
    reg->add(new KisResourceLoader<KisSessionResource>(ResourceType::Sessions, ResourceType::Sessions, i18n("Sessions"), QStringList() << "application/x-krita-session"));
    reg->add(new KisResourceLoader<KoGamutMask>(ResourceType::GamutMasks, ResourceType::GamutMasks, i18n("Gamut masks"), QStringList() << "application/x-krita-gamutmasks"));

    reg->add(new KisResourceLoader<KisPSDLayerStyle>(ResourceType::LayerStyles,
                                                     ResourceType::LayerStyles,
                                                     ResourceType::LayerStyles,
                                                     QStringList() << "application/x-photoshop-style"));

    if (!KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita: Fatal error"), i18n("%1\n\nKrita will quit now.", KisResourceCacheDb::lastError()));
        //return false;
    }

    KisResourceLocator::LocatorError r = KisResourceLocator::instance()->initialize(KoResourcePaths::getApplicationRoot() + "/share/krita");
    connect(KisResourceLocator::instance(), SIGNAL(progressMessage(const QString&)), this, SLOT(setSplashScreenLoadingText(const QString&)));
    if (r != KisResourceLocator::LocatorError::Ok ) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita: Fatal error"), KisResourceLocator::instance()->errorMessages().join('\n') + i18n("\n\nKrita will quit now."));
        //return false;
    }

    return true;
}

void KisApplication::loadPlugins()
{
    //    qDebug() << "loadPlugins();";

    KoShapeRegistry* r = KoShapeRegistry::instance();
    r->add(new KisShapeSelectionFactory());
    KoColorSpaceRegistry::instance();
    KisActionRegistry::instance();
    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
    KisPaintOpRegistry::instance();
    KoToolRegistry::instance();
    KoDockRegistry::instance();
}

void KisApplication::loadGuiPlugins()
{
    // XXX_EXIV: make the exiv io backends real plugins
    setSplashScreenLoadingText(i18n("Loading Plugins Exiv/IO..."));
    processEvents();
    //    qDebug() << "loading exiv2";
    KisExiv2::initialize();
}

bool KisApplication::start(const KisApplicationArguments &args)
{
    KisConfig cfg(false);

#if defined(Q_OS_WIN)
#ifdef ENV32BIT

    if (isWow64() && !cfg.readEntry("WarnedAbout32Bits", false)) {
        QMessageBox::information(0,
                                 i18nc("@title:window", "Krita: Warning"),
                                 i18n("You are running a 32 bits build on a 64 bits Windows.\n"
                                      "This is not recommended.\n"
                                      "Please download and install the x64 build instead."));
        cfg.writeEntry("WarnedAbout32Bits", true);

    }
#endif
#endif

    QString opengl = cfg.canvasState();
    if (opengl == "OPENGL_NOT_TRIED" ) {
        cfg.setCanvasState("TRY_OPENGL");
    }
    else if (opengl != "OPENGL_SUCCESS" && opengl != "TRY_OPENGL") {
        cfg.setCanvasState("OPENGL_FAILED");
    }

    setSplashScreenLoadingText(i18n("Initializing Globals"));
    processEvents();
    initializeGlobals(args);

    const bool doNewImage = args.doNewImage();
    const bool doTemplate = args.doTemplate();
    const bool exportAs = args.exportAs();
    const bool exportSequence = args.exportSequence();
    const QString exportFileName = args.exportFileName();

    d->batchRun = (exportAs || exportSequence || !exportFileName.isEmpty());
    const bool needsMainWindow = (!exportAs && !exportSequence);
    // only show the mainWindow when no command-line mode option is passed
    bool showmainWindow = (!exportAs && !exportSequence); // would be !batchRun;

    const bool showSplashScreen = !d->batchRun && qEnvironmentVariableIsEmpty("NOSPLASH");
    if (showSplashScreen && d->splashScreen) {
        d->splashScreen->show();
        d->splashScreen->repaint();
        processEvents();
    }

    KConfigGroup group(KSharedConfig::openConfig(), "theme");
    Digikam::ThemeManager themeManager;
    themeManager.setCurrentTheme(group.readEntry("Theme", "Krita dark"));

    ResetStarting resetStarting(d->splashScreen, args.filenames().count()); // remove the splash when done
    Q_UNUSED(resetStarting);

    // Make sure we can save resources and tags
    setSplashScreenLoadingText(i18n("Adding resource types"));
    processEvents();
    addResourceTypes();

    // Load the plugins
    loadPlugins();

    // Load all resources
    if (!registerResources()) {
        return false;
    }

    // Load the gui plugins
    loadGuiPlugins();

    KisPart *kisPart = KisPart::instance();
    if (needsMainWindow) {
        // show a mainWindow asap, if we want that
        setSplashScreenLoadingText(i18n("Loading Main Window..."));
        processEvents();


        bool sessionNeeded = true;
        auto sessionMode = cfg.sessionOnStartup();

        if (!args.session().isEmpty()) {
            sessionNeeded = !kisPart->restoreSession(args.session());
        } else if (sessionMode == KisConfig::SOS_ShowSessionManager) {
            showmainWindow = false;
            sessionNeeded = false;
            kisPart->showSessionManager();
        } else if (sessionMode == KisConfig::SOS_PreviousSession) {
            KConfigGroup sessionCfg = KSharedConfig::openConfig()->group("session");
            const QString &sessionName = sessionCfg.readEntry("previousSession");

            sessionNeeded = !kisPart->restoreSession(sessionName);
        }

        if (sessionNeeded) {
            kisPart->startBlankSession();
        }

        if (!args.windowLayout().isEmpty()) {
            KoResourceServer<KisWindowLayoutResource> * rserver = KisResourceServerProvider::instance()->windowLayoutServer();
            KisWindowLayoutResourceSP windowLayout = rserver->resourceByName(args.windowLayout());
            if (windowLayout) {
                windowLayout->applyLayout();
            }
        }

        if (showmainWindow) {
            d->mainWindow = kisPart->currentMainwindow();

            if (!args.workspace().isEmpty()) {
                KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();
                KisWorkspaceResourceSP workspace = rserver->resourceByName(args.workspace());
                if (workspace) {
                    d->mainWindow->restoreWorkspace(workspace->resourceId());
                }
            }

            if (args.canvasOnly()) {
                d->mainWindow->viewManager()->switchCanvasOnly(true);
            }

            if (args.fullScreen()) {
                d->mainWindow->showFullScreen();
            }
        } else {
            d->mainWindow = kisPart->createMainWindow();
        }
    }
    short int numberOfOpenDocuments = 0; // number of documents open

    // Check for autosave files that can be restored, if we're not running a batchrun (test)
    if (!d->batchRun) {
        checkAutosaveFiles();
    }

    setSplashScreenLoadingText(QString()); // done loading, so clear out label
    processEvents();

    //configure the unit manager
    KisSpinBoxUnitManagerFactory::setDefaultUnitManagerBuilder(new KisDocumentAwareSpinBoxUnitManagerBuilder());
    connect(this, &KisApplication::aboutToQuit, &KisSpinBoxUnitManagerFactory::clearUnitManagerBuilder); //ensure the builder is destroyed when the application leave.
    //the new syntax slot syntax allow to connect to a non q_object static method.

    // Create a new image, if needed
    if (doNewImage) {
        KisDocument *doc = args.createDocumentFromArguments();
        if (doc) {
            kisPart->addDocument(doc);
            d->mainWindow->addViewAndNotifyLoadingCompleted(doc);
        }
    }

    // Get the command line arguments which we have to parse
    int argsCount = args.filenames().count();
    if (argsCount > 0) {
        // Loop through arguments
        for (int argNumber = 0; argNumber < argsCount; argNumber++) {
            QString fileName = args.filenames().at(argNumber);
            // are we just trying to open a template?
            if (doTemplate) {
                // called in mix with batch options? ignore and silently skip
                if (d->batchRun) {
                    continue;
                }
                if (createNewDocFromTemplate(fileName, d->mainWindow)) {
                    ++numberOfOpenDocuments;
                }
                // now try to load
            }
            else {
                if (exportAs) {
                    QString outputMimetype = KisMimeDatabase::mimeTypeForFile(exportFileName, false);
                    if (outputMimetype == "application/octetstream") {
                        dbgKrita << i18n("Mimetype not found, try using the -mimetype option") << endl;
                        return false;
                    }

                    KisDocument *doc = kisPart->createDocument();
                    doc->setFileBatchMode(d->batchRun);
                    bool result = doc->openUrl(QUrl::fromLocalFile(fileName));

                    if (!result) {
                        errKrita << "Could not load " << fileName << ":" << doc->errorMessage();
                        QTimer::singleShot(0, this, SLOT(quit()));
                        return false;
                    }

                    if (exportFileName.isEmpty()) {
                        errKrita << "Export destination is not specified for" << fileName << "Please specify export destination with --export-filename option";
                        QTimer::singleShot(0, this, SLOT(quit()));
                        return false;
                    }

                    qApp->processEvents(); // For vector layers to be updated

                    doc->setFileBatchMode(true);
                    if (!doc->exportDocumentSync(QUrl::fromLocalFile(exportFileName), outputMimetype.toLatin1())) {
                        errKrita << "Could not export " << fileName << "to" << exportFileName << ":" << doc->errorMessage();
                    }
                    QTimer::singleShot(0, this, SLOT(quit()));
                    return true;
                }
                else if (exportSequence) {
                    KisDocument *doc = kisPart->createDocument();
                    doc->setFileBatchMode(d->batchRun);
                    doc->openUrl(QUrl::fromLocalFile(fileName));
                    qApp->processEvents(); // For vector layers to be updated
                    
                    if (!doc->image()->animationInterface()->hasAnimation()) {
                        errKrita << "This file has no animation." << endl;
                        QTimer::singleShot(0, this, SLOT(quit()));
                        return false;
                    }

                    doc->setFileBatchMode(true);
                    int sequenceStart = 0;

                    KisAsyncAnimationFramesSaveDialog exporter(doc->image(),
                                               doc->image()->animationInterface()->fullClipRange(),
                                               exportFileName,
                                               sequenceStart,
                                               false,
                                               0);
                    exporter.setBatchMode(d->batchRun);
                    KisAsyncAnimationFramesSaveDialog::Result result =
                        exporter.regenerateRange(0);
                    if (result == KisAsyncAnimationFramesSaveDialog::RenderFailed) {
                        errKrita << i18n("Failed to render animation frames!") << endl;
                    }
                    QTimer::singleShot(0, this, SLOT(quit()));
                    return true;
                }
                else if (d->mainWindow) {
                    if (fileName.endsWith(".bundle")) {
                        d->mainWindow->installBundle(fileName);
                    }
                    else {
                        KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;

                        if (d->mainWindow->openDocument(QUrl::fromLocalFile(fileName), flags)) {
                            // Normal case, success
                            numberOfOpenDocuments++;
                        }
                    }
                }
            }
        }
    }

    //add an image as file-layer
    if (!args.fileLayer().isEmpty()){
        if (d->mainWindow->viewManager()->image()){
            KisFileLayer *fileLayer = new KisFileLayer(d->mainWindow->viewManager()->image(), "",
                                                    args.fileLayer(), KisFileLayer::None,
                                                    d->mainWindow->viewManager()->image()->nextLayerName(), OPACITY_OPAQUE_U8);
            QFileInfo fi(fileLayer->path());
            if (fi.exists()){
                KisNodeCommandsAdapter adapter(d->mainWindow->viewManager());
                adapter.addNode(fileLayer, d->mainWindow->viewManager()->activeNode()->parent(),
                                    d->mainWindow->viewManager()->activeNode());
            }
            else{
                QMessageBox::warning(nullptr, i18nc("@title:window", "Krita:Warning"),
                                            i18n("Cannot add %1 as a file layer: the file does not exist.", fileLayer->path()));
            }
        }
        else if (this->isRunning()){
            QMessageBox::warning(nullptr, i18nc("@title:window", "Krita:Warning"),
                                i18n("Cannot add the file layer: no document is open.\n\n"
"You can create a new document using the --new-image option, or you can open an existing file.\n\n"
"If you instead want to add the file layer to a document in an already running instance of Krita, check the \"Allow only one instance of Krita\" checkbox in the settings (Settings -> General -> Window)."));
        }
        else {
            QMessageBox::warning(nullptr, i18nc("@title:window", "Krita: Warning"),
                                i18n("Cannot add the file layer: no document is open.\n"
                                     "You can either create a new file using the --new-image option, or you can open an existing file."));
        }
    }

    // fixes BUG:369308  - Krita crashing on splash screen when loading.
    // trying to open a file before Krita has loaded can cause it to hang and crash
    if (d->splashScreen) {
        d->splashScreen->displayLinks(true);
        d->splashScreen->displayRecentFiles(true);
    }

    Q_FOREACH(const QByteArray &message, d->earlyRemoteArguments) {
        executeRemoteArguments(message, d->mainWindow);
    }

    KisUsageLogger::writeSysInfo(KisUsageLogger::screenInformation());


    // not calling this before since the program will quit there.
    return true;
}

KisApplication::~KisApplication()
{
    KisResourceCacheDb::deleteTemporaryResources();
}

void KisApplication::setSplashScreen(QWidget *splashScreen)
{
    d->splashScreen = qobject_cast<KisSplashScreen*>(splashScreen);
}

void KisApplication::setSplashScreenLoadingText(const QString &textToLoad)
{
    if (d->splashScreen) {
        d->splashScreen->setLoadingText(textToLoad);
        d->splashScreen->repaint();
    }
}

void KisApplication::hideSplashScreen()
{
    if (d->splashScreen) {
        // hide the splashscreen to see the dialog
        d->splashScreen->hide();
    }
}


bool KisApplication::notify(QObject *receiver, QEvent *event)
{
    try {
        return QApplication::notify(receiver, event);
    } catch (std::exception &e) {
        qWarning("Error %s sending event %i to object %s",
                 e.what(), event->type(), qPrintable(receiver->objectName()));
    } catch (...) {
        qWarning("Error <unknown> sending event %i to object %s",
                 event->type(), qPrintable(receiver->objectName()));
    }
    return false;
}


void KisApplication::executeRemoteArguments(QByteArray message, KisMainWindow *mainWindow)
{
    KisApplicationArguments args = KisApplicationArguments::deserialize(message);
    const bool doTemplate = args.doTemplate();
    const bool doNewImage = args.doNewImage();
    const int argsCount = args.filenames().count();
    bool documentCreated = false;

    // Create a new image, if needed
    if (doNewImage) {
        KisDocument *doc = args.createDocumentFromArguments();
        if (doc) {
            KisPart::instance()->addDocument(doc);
            d->mainWindow->addViewAndNotifyLoadingCompleted(doc);
        }
    }
    if (argsCount > 0) {
        // Loop through arguments
        for (int argNumber = 0; argNumber < argsCount; ++argNumber) {
            QString filename = args.filenames().at(argNumber);
            // are we just trying to open a template?
            if (doTemplate) {
                documentCreated |= createNewDocFromTemplate(filename, mainWindow);
            }
            else if (QFile(filename).exists()) {
                KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
                documentCreated |= mainWindow->openDocument(QUrl::fromLocalFile(filename), flags);
            }
        }
    }

    //add an image as file-layer if called in another process and singleApplication is enabled
    if (!args.fileLayer().isEmpty()){
        if (argsCount > 0  && !documentCreated){
            //arg was passed but document was not created so don't add the file layer.
            QMessageBox::warning(mainWindow, i18nc("@title:window", "Krita:Warning"),
                                            i18n("Couldn't open file %1",args.filenames().at(argsCount - 1)));
        }
        else if (mainWindow->viewManager()->image()){
            KisFileLayer *fileLayer = new KisFileLayer(mainWindow->viewManager()->image(), "",
                                                    args.fileLayer(), KisFileLayer::None,
                                                    mainWindow->viewManager()->image()->nextLayerName(), OPACITY_OPAQUE_U8);
            QFileInfo fi(fileLayer->path());
            if (fi.exists()){
                KisNodeCommandsAdapter adapter(d->mainWindow->viewManager());
                adapter.addNode(fileLayer, d->mainWindow->viewManager()->activeNode()->parent(),
                                    d->mainWindow->viewManager()->activeNode());
            }
            else{
                QMessageBox::warning(mainWindow, i18nc("@title:window", "Krita:Warning"),
                                            i18n("Cannot add %1 as a file layer: the file does not exist.", fileLayer->path()));
            }
        }
        else {
            QMessageBox::warning(mainWindow, i18nc("@title:window", "Krita:Warning"),
                                            i18n("Cannot add the file layer: no document is open."));
        }
    }
}


void KisApplication::remoteArguments(QByteArray message, QObject *socket)
{
    Q_UNUSED(socket);

    // check if we have any mainwindow
    KisMainWindow *mw = qobject_cast<KisMainWindow*>(qApp->activeWindow());

    if (!mw && KisPart::instance()->mainWindows().size() > 0) {
        mw = KisPart::instance()->mainWindows().first();
    }

    if (!mw) {
        d->earlyRemoteArguments << message;
        return;
    }
    executeRemoteArguments(message, mw);
}

void KisApplication::fileOpenRequested(const QString &url)
{
    KisMainWindow *mainWindow = KisPart::instance()->mainWindows().first();
    if (mainWindow) {
        KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
        mainWindow->openDocument(QUrl::fromLocalFile(url), flags);
    }
}


void KisApplication::checkAutosaveFiles()
{
    if (d->batchRun) return;

#ifdef Q_OS_WIN
    QDir dir = QDir::temp();
#else
    QDir dir = QDir::home();
#endif

    // Check for autosave files from a previous run. There can be several, and
    // we want to offer a restore for every one. Including a nice thumbnail!

    // Hidden autosave files
    QStringList filters = QStringList() << QString(".krita-*-*-autosave.kra");

    // all autosave files for our application
    QStringList autosaveFiles = dir.entryList(filters, QDir::Files | QDir::Hidden);

    // Visible autosave files
    filters = QStringList() << QString("krita-*-*-autosave.kra");
    autosaveFiles += dir.entryList(filters, QDir::Files);

    // Allow the user to make their selection
    if (autosaveFiles.size() > 0) {
        if (d->splashScreen) {
            // hide the splashscreen to see the dialog
            d->splashScreen->hide();
        }
        d->autosaveDialog = new KisAutoSaveRecoveryDialog(autosaveFiles, activeWindow());
        QDialog::DialogCode result = (QDialog::DialogCode) d->autosaveDialog->exec();

        if (result == QDialog::Accepted) {
            QStringList filesToRecover = d->autosaveDialog->recoverableFiles();
            Q_FOREACH (const QString &autosaveFile, autosaveFiles) {
                if (!filesToRecover.contains(autosaveFile)) {
                    KisUsageLogger::log(QString("Removing autosave file %1").arg(dir.absolutePath() + "/" + autosaveFile));
                    QFile::remove(dir.absolutePath() + "/" + autosaveFile);
                }
            }
            autosaveFiles = filesToRecover;
        } else {
            autosaveFiles.clear();
        }

        if (autosaveFiles.size() > 0) {
            QList<QUrl> autosaveUrls;
            Q_FOREACH (const QString &autoSaveFile, autosaveFiles) {
                const QUrl url = QUrl::fromLocalFile(dir.absolutePath() + QLatin1Char('/') + autoSaveFile);
                autosaveUrls << url;
            }
            if (d->mainWindow) {
                Q_FOREACH (const QUrl &url, autosaveUrls) {
                    KisMainWindow::OpenFlags flags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
                    d->mainWindow->openDocument(url, flags | KisMainWindow::RecoveryFile);
                }
            }
        }
        // cleanup
        delete d->autosaveDialog;
        d->autosaveDialog = nullptr;
    }
}

bool KisApplication::createNewDocFromTemplate(const QString &fileName, KisMainWindow *mainWindow)
{
    QString templatePath;

    const QUrl templateUrl = QUrl::fromLocalFile(fileName);
    if (QFile::exists(fileName)) {
        templatePath = templateUrl.toLocalFile();
        dbgUI << "using full path...";
    }
    else {
        QString desktopName(fileName);
        const QString templatesResourcePath =  QStringLiteral("templates/");

        QStringList paths = KoResourcePaths::findAllResources("data", templatesResourcePath + "*/" + desktopName);
        if (paths.isEmpty()) {
            paths = KoResourcePaths::findAllResources("data", templatesResourcePath + desktopName);
        }

        if (paths.isEmpty()) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("No template found for: %1", desktopName));
        } else if (paths.count() > 1) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("Too many templates found for: %1", desktopName));
        } else {
            templatePath = paths.at(0);
        }
    }

    if (!templatePath.isEmpty()) {
        QUrl templateBase;
        templateBase.setPath(templatePath);
        KDesktopFile templateInfo(templatePath);

        QString templateName = templateInfo.readUrl();
        QUrl templateURL;
        templateURL.setPath(templateBase.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path() + '/' + templateName);
        if (templateURL.scheme().isEmpty()) {
            templateURL.setScheme("file");
        }

        KisMainWindow::OpenFlags batchFlags = d->batchRun ? KisMainWindow::BatchMode : KisMainWindow::None;
        if (mainWindow->openDocument(templateURL, KisMainWindow::Import | batchFlags)) {
            dbgUI << "Template loaded...";
            return true;
        }
        else {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"),
                                  i18n("Template %1 failed to load.", templateURL.toDisplayString()));
        }
    }

    return false;
}

void KisApplication::resetConfig()
{
    KIS_ASSERT_RECOVER_RETURN(qApp->thread() == QThread::currentThread());

    KSharedConfigPtr config =  KSharedConfig::openConfig();
    config->markAsClean();
    
    // find user settings file
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString kritarcPath = configPath + QStringLiteral("/kritarc");
    
    QFile kritarcFile(kritarcPath);
    
    if (kritarcFile.exists()) {
        if (kritarcFile.open(QFile::ReadWrite)) {
            QString backupKritarcPath = kritarcPath + QStringLiteral(".backup");
    
            QFile backupKritarcFile(backupKritarcPath);
    
            if (backupKritarcFile.exists()) {
                backupKritarcFile.remove();
            }

            QMessageBox::information(0,
                                 i18nc("@title:window", "Krita"),
                                 i18n("Krita configurations reset!\n\n"
                                      "Backup file was created at: %1\n\n"
                                      "Restart Krita for changes to take effect.",
                                      backupKritarcPath),
                                 QMessageBox::Ok, QMessageBox::Ok);

            // clear file
            kritarcFile.rename(backupKritarcPath);

            kritarcFile.close();
        }
        else {
            QMessageBox::warning(0,
                                 i18nc("@title:window", "Krita"),
                                 i18n("Failed to clear %1\n\n"
                                      "Please make sure no other program is using the file and try again.",
                                      kritarcPath),
                                 QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    // reload from disk; with the user file settings cleared,
    // this should load any default configuration files shipping with the program
    config->reparseConfiguration();
    config->sync();

    // Restore to default workspace
    KConfigGroup cfg = KSharedConfig::openConfig()->group("MainWindow");

    QString currentWorkspace = cfg.readEntry<QString>("CurrentWorkspace", "Default");
    KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();
    KisWorkspaceResourceSP workspace = rserver->resourceByName(currentWorkspace);

    if (workspace) {
        d->mainWindow->restoreWorkspace(workspace->resourceId());
    }
}

void KisApplication::askresetConfig()
{
    bool ok = QMessageBox::question(0,
                                    i18nc("@title:window", "Krita"),
                                    i18n("Do you want to clear the settings file?"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
    if (ok) {
        resetConfig();
    }
}
