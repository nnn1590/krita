/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "kritasketchplugin.h"

#include "ColorSelectorItem.h"
#include "CurveEditorItem.h"
#include "DocumentListModel.h"
#include "ColorImageProvider.h"
#include "FiltersCategoryModel.h"
#include "LayerModel.h"
#include "LayerCompositeDetails.h"
#include "PaletteColorsModel.h"
#include "RecentImagesModel.h"
#include "RecentImageImageProvider.h"
#include "RecentFileManager.h"
#include "IconImageProvider.h"
#include "KisMultiFeedRSSModel.h"
#include "FileSystemModel.h"
#include "CompositeOpModel.h"
#include "KeyboardModel.h"
#include "ColorModelModel.h"
#include "ColorDepthModel.h"
#include "ColorProfileModel.h"
#include <TemplatesModel.h>
#include "Theme.h"


#include "Constants.h"
#include "Settings.h"
#include "SimpleTouchArea.h"
#include "ToolManager.h"
#include "KritaNamespace.h"
#include "PanelConfiguration.h"
#include "DocumentManager.h"
#include "kis_clipboard.h"
#include "KisSketchView.h"

#include <QQmlEngine>
#include <QQmlContext>


static QObject *provideConstantsObject(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new Constants;
}

static QObject *provideKritaNamespaceObject(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new KritaNamespace;
}

static QObject *provideKritaRssModelObject(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    MultiFeedRssModel *rssModel = new MultiFeedRssModel;
    rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));

    return rssModel;
}


void KritaSketchPlugin::registerTypes(const char* uri)
{
    Q_UNUSED(uri);
    Q_ASSERT(uri == QLatin1String("org.krita.sketch"));
    qmlRegisterType<SimpleTouchArea>("org.krita.sketch", 1, 0, "SimpleTouchArea");
    qmlRegisterType<ColorSelectorItem>("org.krita.sketch", 1, 0, "ColorSelectorItem");
    qmlRegisterType<CurveEditorItem>("org.krita.sketch", 1, 0, "CurveEditorItem");
    qmlRegisterType<DocumentListModel>("org.krita.sketch", 1, 0, "DocumentListModel");
    qmlRegisterType<PaletteColorsModel>("org.krita.sketch", 1, 0, "PaletteColorsModel");
    qmlRegisterType<KisSketchView>("org.krita.sketch", 1, 0, "SketchView");
    qmlRegisterType<LayerModel>("org.krita.sketch", 1, 0, "LayerModel");
    qmlRegisterType<FiltersCategoryModel>("org.krita.sketch", 1, 0, "FiltersCategoryModel");
    qmlRegisterType<RecentImagesModel>("org.krita.sketch", 1, 0, "RecentImagesModel");
    qmlRegisterType<FileSystemModel>("org.krita.sketch", 1, 0, "FileSystemModel");
    qmlRegisterType<ToolManager>("org.krita.sketch", 1, 0, "ToolManager");
    qmlRegisterType<CompositeOpModel>("org.krita.sketch", 1, 0, "CompositeOpModel");
    qmlRegisterType<PanelConfiguration>("org.krita.sketch", 1, 0, "PanelConfiguration");
    qmlRegisterType<KeyboardModel>("org.krita.sketch", 1, 0, "KeyboardModel");
    qmlRegisterType<ColorModelModel>("org.krita.sketch", 1, 0, "ColorModelModel");
    qmlRegisterType<ColorDepthModel>("org.krita.sketch", 1, 0, "ColorDepthModel");
    qmlRegisterType<ColorProfileModel>("org.krita.sketch", 1, 0, "ColorProfileModel");
    qmlRegisterType<Theme>("org.krita.sketch", 1, 0, "Theme");
    qmlRegisterType<TemplatesModel>("org.krita.sketch", 1, 0, "TemplatesModel");

    qmlRegisterSingletonType<Constants>("org.krita.sketch", 1, 0, "Constants", provideConstantsObject);
    qmlRegisterSingletonType<KritaNamespace>("org.krita.sketch", 1, 0, "Krita", provideKritaNamespaceObject);
    qmlRegisterSingletonType<MultiFeedRssModel>("org.krita.sketch", 1, 0, "KritaFeedRssModel", provideKritaRssModelObject);

    qmlRegisterUncreatableType<LayerCompositeDetails>("org.krita.sketch", 1, 0, "LayerCompositeDetails", "This type is returned by the LayerModel class");
}

void KritaSketchPlugin::initializeEngine(QQmlEngine* engine, const char* uri)
{
    Q_UNUSED(uri);
    Q_ASSERT(uri == QLatin1String("org.krita.sketch"));

    engine->addImageProvider(QLatin1String("color"), new ColorImageProvider);
    engine->addImageProvider(QLatin1String("recentimage"), new RecentImageImageProvider);
    engine->addImageProvider(QLatin1String("icon"), new IconImageProvider);

    RecentFileManager *recentFileManager = DocumentManager::instance()->recentFileManager();
    engine->rootContext()->setContextProperty("RecentFileManager", recentFileManager);
    engine->rootContext()->setContextProperty("KisClipBoard", KisClipboard::instance());
    engine->rootContext()->setContextProperty("QMLEngine", engine);
    // This would be a problem, but doesn't seem to be used...
//    engine->rootContext()->setContextProperty("View", d->view);
}

