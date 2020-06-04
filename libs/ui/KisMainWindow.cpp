/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2006 David Faure <faure@kde.org>
   Copyright (C) 2007, 2009 Thomas zander <zander@kde.org>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KisMainWindow.h"

#include <KoConfig.h>

// qt includes
#include <QApplication>
#include <QByteArray>
#include <QCloseEvent>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDialog>
#include <QDockWidget>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QToolButton>
#include <KisSignalMapper.h>
#include <QTabBar>
#include <QMoveEvent>
#include <QUrl>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyleFactory>
#include <QMenu>
#include <QMenuBar>
#include <KisMimeDatabase.h>
#include <QMimeData>
#include <QStackedWidget>
#include <QProxyStyle>
#include <QScreen>
#include <QAction>
#include <QWindow>
#include <QScrollArea>

#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kis_debug.h>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <klocalizedstring.h>
#include <kaboutdata.h>
#include <kis_workspace_resource.h>
#include <input/kis_input_manager.h>
#include "kis_selection_manager.h"
#include "kis_icon_utils.h"
#include <krecentfilesaction.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kguiitem.h>
#include <kwindowconfig.h>
#include <kformat.h>

#include <KoResourcePaths.h>
#include <KoToolFactoryBase.h>
#include <KoToolRegistry.h>
#include "KoDockFactoryBase.h"
#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "KoFileDialog.h"
#include <kis_icon.h>
#include <KoToolManager.h>
#include <KoZoomController.h>
#include "KoToolDocker.h"
#include "KoToolBoxDocker_p.h"
#include <KoToolBoxFactory.h>
#include <KoDockRegistry.h>
#include <KoPluginLoader.h>
#include <KoColorSpaceEngine.h>
#include <KoUpdater.h>
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisResourceLoaderRegistry.h>
#include <KisResourceIterator.h>
#include <KisResourceTypes.h>
#include <KisResourceCacheDb.h>

#ifdef Q_OS_ANDROID
#include <KisAndroidFileManager.h>
#endif

#include <KisUsageLogger.h>
#include <brushengine/kis_paintop_settings.h>
#include "dialogs/kis_about_application.h"
#include "dialogs/kis_delayed_save_dialog.h"
#include "dialogs/kis_dlg_preferences.h"
#include "kis_action_manager.h"
#include "KisApplication.h"
#include "kis_canvas2.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_resource_provider.h"
#include "kis_clipboard.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_custom_image_widget.h"
#include <KisDocument.h>
#include "kis_group_layer.h"
#include "kis_image_from_clipboard_widget.h"
#include "kis_image.h"
#include <KisImportExportFilter.h>
#include "KisImportExportManager.h"
#include "kis_mainwindow_observer.h"
#include "kis_memory_statistics_server.h"
#include "kis_node.h"
#include "KisOpenPane.h"
#include "kis_paintop_box.h"
#include "KisPart.h"
#include "KisResourceServerProvider.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_statusbar.h"
#include "KisView.h"
#include "KisViewManager.h"
#include "thememanager.h"
#include "kis_animation_importer.h"
#include "dialogs/kis_dlg_import_image_sequence.h"
#include <KisImageConfigNotifier.h>
#include "KisWindowLayoutManager.h"
#include <KisUndoActionsUpdateManager.h>
#include "KisWelcomePageWidget.h"
#include <KritaVersionWrapper.h>
#include <kritaversion.h>
#include "KisCanvasWindow.h"
#include "kis_action.h"

#include <mutex>

class ToolDockerFactory : public KoDockFactoryBase
{
public:
    ToolDockerFactory() : KoDockFactoryBase() { }

    QString id() const override {
        return "sharedtooldocker";
    }

    QDockWidget* createDockWidget() override {
        KoToolDocker* dockWidget = new KoToolDocker();
        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};

class Q_DECL_HIDDEN KisMainWindow::Private
{
public:
    Private(KisMainWindow *parent, QUuid id)
        : q(parent)
        , id(id)
        , styleMenu(new KActionMenu(i18nc("@action:inmenu", "Styles"), parent))
        , dockWidgetMenu(new KActionMenu(i18nc("@action:inmenu", "&Dockers"), parent))
        , windowMenu(new KActionMenu(i18nc("@action:inmenu", "&Window"), parent))
        , documentMenu(new KActionMenu(i18nc("@action:inmenu", "New &View"), parent))
        , workspaceMenu(new KActionMenu(i18nc("@action:inmenu", "Wor&kspace"), parent))
        , welcomePage(new KisWelcomePageWidget(parent))
        , widgetStack(new QStackedWidget(parent))
        , mdiArea(new QMdiArea(parent))
        , windowMapper(new KisSignalMapper(parent))
        , documentMapper(new KisSignalMapper(parent))
    #ifdef Q_OS_ANDROID
        , fileManager(new KisAndroidFileManager(parent))
    #endif

    {
        if (id.isNull()) this->id = QUuid::createUuid();

        welcomeScroller = new QScrollArea();
        welcomeScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        welcomeScroller->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        welcomeScroller->setWidget(welcomePage);
        welcomeScroller->setWidgetResizable(true);

        widgetStack->addWidget(welcomeScroller);
        widgetStack->addWidget(mdiArea);
        mdiArea->setTabsMovable(true);
        mdiArea->setActivationOrder(QMdiArea::ActivationHistoryOrder);
    }

    ~Private() {
        qDeleteAll(toolbarList);
    }

    KisMainWindow *q {0};
    QUuid id;

    KisViewManager *viewManager {0};

    QPointer<KisView> activeView;

    QList<QAction *> toolbarList;

    bool firstTime {true};
    bool windowSizeDirty {false};
    bool readOnly {false};

    KisAction *showDocumentInfo {0};
    KisAction *saveAction {0};
    KisAction *saveActionAs {0};
    KisAction *importAnimation {0};
    KisAction *closeAll {0};
    KisAction *importFile {0};
    KisAction *exportFile {0};
    KisAction *undo {0};
    KisAction *redo {0};
    KisAction *newWindow {0};
    KisAction *close {0};
    KisAction *mdiCascade {0};
    KisAction *mdiTile {0};
    KisAction *mdiNextWindow {0};
    KisAction *mdiPreviousWindow {0};
    KisAction *toggleDockers {0};
    KisAction *resetConfigurations {0};
    KisAction *toggleDockerTitleBars {0};
    KisAction *toggleDetachCanvas {0};
    KisAction *fullScreenMode {0};
    KisAction *showSessionManager {0};

    KisAction *expandingSpacers[2];

    KActionMenu *styleMenu;
    QActionGroup* styleActions;
    QMap<QString, QAction*> actionMap;

    KActionMenu *dockWidgetMenu;
    KActionMenu *windowMenu;
    KActionMenu *documentMenu;
    KActionMenu *workspaceMenu;

    KHelpMenu *helpMenu  {0};

    KRecentFilesAction *recentFiles {0};
    KisResourceModel *workspacemodel {0};

    QScopedPointer<KisUndoActionsUpdateManager> undoActionsUpdateManager;

    QString lastExportLocation;

    QMap<QString, QDockWidget *> dockWidgetsMap;
    QByteArray dockerStateBeforeHiding;
    KoToolDocker *toolOptionsDocker {0};

    QCloseEvent *deferredClosingEvent {0};

    Digikam::ThemeManager *themeManager {0};

    QScrollArea *welcomeScroller {0};
    KisWelcomePageWidget *welcomePage {0};


    QStackedWidget *widgetStack {0};

    QMdiArea *mdiArea;
    QMdiSubWindow *activeSubWindow  {0};
    KisSignalMapper *windowMapper;
    KisSignalMapper *documentMapper;
    KisCanvasWindow *canvasWindow {0};

    QByteArray lastExportedFormat;
    QScopedPointer<KisSignalCompressorWithParam<int> > tabSwitchCompressor;
    QMutex savingEntryMutex;

    KConfigGroup windowStateConfig;

    QUuid workspaceBorrowedBy;
    KisSignalAutoConnectionsStore screenConnectionsStore;

#ifdef Q_OS_ANDROID
    KisAndroidFileManager *fileManager;
#endif

    KisActionManager * actionManager() {
        return viewManager->actionManager();
    }

    QTabBar* findTabBarHACK() {
        QObjectList objects = mdiArea->children();
        Q_FOREACH (QObject *object, objects) {
            QTabBar *bar = qobject_cast<QTabBar*>(object);
            if (bar) {
                return bar;
            }
        }
        return 0;
    }
};

KisMainWindow::KisMainWindow(QUuid uuid)
    : KXmlGuiWindow()
    , d(new Private(this, uuid))
{
    d->workspacemodel = KisResourceModelProvider::resourceModel(ResourceType::Workspaces);
    connect(d->workspacemodel, SIGNAL(afterResourcesLayoutReset()), this, SLOT(updateWindowMenu()));

    d->viewManager = new KisViewManager(this, actionCollection());
    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    d->themeManager = new Digikam::ThemeManager(group.readEntry("Theme", "Krita dark"), this);

    d->windowStateConfig = KSharedConfig::openConfig()->group("MainWindow");

    setStandardToolBarMenuEnabled(true);
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockNestingEnabled(true);

    qApp->setStartDragDistance(25);     // 25 px is a distance that works well for Tablet and Mouse events

#ifdef Q_OS_MACOS
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    connect(this, SIGNAL(restoringDone()), this, SLOT(forceDockTabFonts()));
    connect(this, SIGNAL(themeChanged()), d->viewManager, SLOT(updateIcons()));
    connect(KisPart::instance(), SIGNAL(documentClosed(QString)), SLOT(updateWindowMenu()));
    connect(KisPart::instance(), SIGNAL(documentOpened(QString)), SLOT(updateWindowMenu()));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(configChanged()));

    actionCollection()->addAssociatedWidget(this);
    KoPluginLoader::instance()->load("Krita/ViewPlugin", "Type == 'Service' and ([X-Krita-Version] == 28)", KoPluginLoader::PluginsConfig(), d->viewManager, false);

    // Load the per-application plugins (Right now, only Python) We do this only once, when the first mainwindow is being created.
    KoPluginLoader::instance()->load("Krita/ApplicationPlugin", "Type == 'Service' and ([X-Krita-Version] == 28)", KoPluginLoader::PluginsConfig(), qApp, true);

    KoToolBoxFactory toolBoxFactory;
    QDockWidget *toolbox = createDockWidget(&toolBoxFactory);
    toolbox->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    KisConfig cfg(true);
    if (cfg.toolOptionsInDocker()) {
        ToolDockerFactory toolDockerFactory;
        d->toolOptionsDocker = qobject_cast<KoToolDocker*>(createDockWidget(&toolDockerFactory));
        d->toolOptionsDocker->toggleViewAction()->setEnabled(true);
    }

    QMap<QString, QAction*> dockwidgetActions;
    dockwidgetActions[toolbox->toggleViewAction()->text()] = toolbox->toggleViewAction();
    Q_FOREACH (const QString & docker, KoDockRegistry::instance()->keys()) {
        KoDockFactoryBase *factory = KoDockRegistry::instance()->value(docker);
        QDockWidget *dw = createDockWidget(factory);
        dockwidgetActions[dw->toggleViewAction()->text()] = dw->toggleViewAction();
    }

    if (d->toolOptionsDocker) {
        dockwidgetActions[d->toolOptionsDocker->toggleViewAction()->text()] = d->toolOptionsDocker->toggleViewAction();
    }
    connect(KoToolManager::instance(), SIGNAL(toolOptionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)), this, SLOT(newOptionWidgets(KoCanvasController*,QList<QPointer<QWidget> >)));

    Q_FOREACH (QString title, dockwidgetActions.keys()) {
        d->dockWidgetMenu->addAction(dockwidgetActions[title]);
    }


    // Style menu actions
    d->styleActions = new QActionGroup(this);
    QAction * action;
    Q_FOREACH (QString styleName, QStyleFactory::keys()) {
        action = new QAction(styleName, d->styleActions);
        action->setCheckable(true);
        d->actionMap.insert(styleName, action);
        d->styleMenu->addAction(d->actionMap.value(styleName));
    }


    // select the config value, or the current style if that does not exist
    QString styleFromConfig = cfg.widgetStyle().toLower();
    QString styleToSelect = styleFromConfig == "" ? style()->objectName().toLower() : styleFromConfig;

    Q_FOREACH (auto key, d->actionMap.keys()) {
        if(key.toLower() == styleToSelect) { // does the key match selection
            d->actionMap.value(key)->setChecked(true);
        }
    }

    connect(d->styleActions, SIGNAL(triggered(QAction*)),
            this, SLOT(slotUpdateWidgetStyle()));




    Q_FOREACH (QDockWidget *wdg, dockWidgets()) {
        if ((wdg->features() & QDockWidget::DockWidgetClosable) == 0) {
            wdg->setVisible(true);
        }
    }

    Q_FOREACH (KoCanvasObserverBase* observer, canvasObservers()) {
        observer->setObservedCanvas(0);
        KisMainwindowObserver* mainwindowObserver = dynamic_cast<KisMainwindowObserver*>(observer);
        if (mainwindowObserver) {
            mainwindowObserver->setViewManager(d->viewManager);
        }
    }

    // Load all the actions from the tool plugins
    Q_FOREACH(KoToolFactoryBase *toolFactory, KoToolRegistry::instance()->values()) {
        toolFactory->createActions(actionCollection());
    }

    d->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->mdiArea->setTabPosition(QTabWidget::North);
    d->mdiArea->setTabsClosable(true);

    // Tab close button override
    // Windows just has a black X, and Ubuntu has a dark x that is hard to read
    // just switch this icon out for all OSs so it is easier to see
    d->mdiArea->setStyleSheet("QTabBar::close-button { image: url(:/pics/broken-preset.png) }");

    setCentralWidget(d->widgetStack);
    d->widgetStack->setCurrentIndex(0);

    connect(d->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivated()));
    connect(d->windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
    connect(d->documentMapper, SIGNAL(mapped(QObject*)), this, SLOT(newView(QObject*)));

    d->canvasWindow = new KisCanvasWindow(this);
    actionCollection()->addAssociatedWidget(d->canvasWindow);

    createActions();

    // the welcome screen needs to grab actions...so make sure this line goes after the createAction() so they exist
    d->welcomePage->setMainWindow(this);

    setAutoSaveSettings(d->windowStateConfig, false);

    subWindowActivated();
    updateWindowMenu();

    if (isHelpMenuEnabled() && !d->helpMenu) {
        // workaround for KHelpMenu (or rather KAboutData::applicationData()) internally
        // not using the Q*Application metadata ATM, which results e.g. in the bugreport wizard
        // not having the app version preset
        // fixed hopefully in KF5 5.22.0, patch pending
        QGuiApplication *app = qApp;
        KAboutData aboutData(app->applicationName(), app->applicationDisplayName(), app->applicationVersion());
        aboutData.setOrganizationDomain(app->organizationDomain().toUtf8());
        d->helpMenu = new KHelpMenu(this, aboutData, false);
        // workaround-less version:
        // d->helpMenu = new KHelpMenu(this, QString()/*unused*/, false);

        // The difference between using KActionCollection->addAction() is that
        // these actions do not get tied to the MainWindow.  What does this all do?
        KActionCollection *actions = d->viewManager->actionCollection();
        QAction *helpContentsAction = d->helpMenu->action(KHelpMenu::menuHelpContents);
        QAction *whatsThisAction = d->helpMenu->action(KHelpMenu::menuWhatsThis);
        QAction *reportBugAction = d->helpMenu->action(KHelpMenu::menuReportBug);
        QAction *switchLanguageAction = d->helpMenu->action(KHelpMenu::menuSwitchLanguage);
        QAction *aboutAppAction = d->helpMenu->action(KHelpMenu::menuAboutApp);
        QAction *aboutKdeAction = d->helpMenu->action(KHelpMenu::menuAboutKDE);

        if (helpContentsAction) {
            actions->addAction(helpContentsAction->objectName(), helpContentsAction);
        }
        if (whatsThisAction) {
            actions->addAction(whatsThisAction->objectName(), whatsThisAction);
        }
        if (reportBugAction) {
            actions->addAction(reportBugAction->objectName(), reportBugAction);
        }
        if (switchLanguageAction) {
            actions->addAction(switchLanguageAction->objectName(), switchLanguageAction);
        }
        if (aboutAppAction) {
            actions->addAction(aboutAppAction->objectName(), aboutAppAction);
        }
        if (aboutKdeAction) {
            actions->addAction(aboutKdeAction->objectName(), aboutKdeAction);
        }

        connect(d->helpMenu, SIGNAL(showAboutApplication()), SLOT(showAboutApplication()));
    }

    // KDE' libs 4''s help contents action is broken outside kde, for some reason... We can handle it just as easily ourselves
    QAction *helpAction = actionCollection()->action("help_contents");
    helpAction->disconnect();
    connect(helpAction, SIGNAL(triggered()), this, SLOT(showManual()));

#if 0
    //check for colliding shortcuts
    QSet<QKeySequence> existingShortcuts;
    Q_FOREACH (QAction* action, actionCollection()->actions()) {
        if(action->shortcut() == QKeySequence(0)) {
            continue;
        }
        dbgKrita << "shortcut " << action->text() << " " << action->shortcut();
        Q_ASSERT(!existingShortcuts.contains(action->shortcut()));
        existingShortcuts.insert(action->shortcut());
    }
#endif

    configChanged();

    // Make sure the python plugins create their actions in time
    KisPart::instance()->notifyMainWindowIsBeingCreated(this);

    // If we have customized the toolbars, load that first
    setLocalXMLFile(KoResourcePaths::locateLocal("data", "krita5.xmlgui"));
    setXMLFile(":/kxmlgui5/krita5.xmlgui");

    guiFactory()->addClient(this);
    connect(guiFactory(), SIGNAL(makingChanges(bool)), SLOT(slotXmlGuiMakingChanges(bool)));

    // Create and plug toolbar list for Settings menu
    QList<QAction *> toolbarList;
    Q_FOREACH (QWidget* it, guiFactory()->containers("ToolBar")) {
        KToolBar * toolBar = ::qobject_cast<KToolBar *>(it);
        if (toolBar) {
            toolBar->setMovable(KisConfig(true).readEntry<bool>("LockAllDockerPanels", false));
            if (toolBar->objectName() == "BrushesAndStuff") {
                toolBar->setEnabled(false);
            }

            KToggleAction* act = new KToggleAction(i18n("Show %1 Toolbar", toolBar->windowTitle()), this);
            actionCollection()->addAction(toolBar->objectName().toUtf8(), act);
            act->setCheckedState(KGuiItem(i18n("Hide %1 Toolbar", toolBar->windowTitle())));
            connect(act, SIGNAL(toggled(bool)), this, SLOT(slotToolbarToggled(bool)));
            act->setChecked(!toolBar->isHidden());
            toolbarList.append(act);
        } else {
            warnUI << "Toolbar list contains a " << it->metaObject()->className() << " which is not a toolbar!";
        }
    }

    KToolBar::setToolBarsLocked(KisConfig(true).readEntry<bool>("LockAllDockerPanels", false));
    plugActionList("toolbarlist", toolbarList);
    d->toolbarList = toolbarList;

    applyToolBarLayout();

    d->viewManager->updateGUI();
    d->viewManager->updateIcons();

    QTimer::singleShot(1000, this, SLOT(checkSanity()));

    {
        using namespace std::placeholders; // For _1 placeholder
        std::function<void (int)> callback(
                    std::bind(&KisMainWindow::switchTab, this, _1));

        d->tabSwitchCompressor.reset(
                    new KisSignalCompressorWithParam<int>(500, callback, KisSignalCompressor::FIRST_INACTIVE));
    }


    if (cfg.readEntry("CanvasOnlyActive", false)) {
        QString currentWorkspace = cfg.readEntry<QString>("CurrentWorkspace", "Default");
        KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();
        KisWorkspaceResourceSP workspace = rserver->resourceByName(currentWorkspace);
        if (workspace) {
            restoreWorkspace(workspace->resourceId());
        }
        cfg.writeEntry("CanvasOnlyActive", false);
        menuBar()->setVisible(true);
    }

    this->winId(); // Ensures the native window has been created.
    QWindow *window = this->windowHandle();
    connect(window, SIGNAL(screenChanged(QScreen *)), this, SLOT(windowScreenChanged(QScreen *)));

}

KisMainWindow::~KisMainWindow()
{
    //    Q_FOREACH (QAction *ac, actionCollection()->actions()) {
    //        QAction *action = qobject_cast<QAction*>(ac);
    //        if (action) {
    //        qDebug() << "<Action"
    //                 << "\n\tname=" << action->objectName()
    //                 << "\n\ticon=" << action->icon().name()
    //                 << "\n\ttext="  << action->text().replace("&", "&amp;")
    //                 << "\n\twhatsThis="  << action->whatsThis()
    //                 << "\n\ttoolTip="  << action->toolTip().replace("<html>", "").replace("</html>", "")
    //                 << "\n\ticonText="  << action->iconText().replace("&", "&amp;")
    //                 << "\n\tshortcut="  << action->shortcut().toString()
    //                 << "\n\tisCheckable="  << QString((action->isChecked() ? "true" : "false"))
    //                 << "\n\tstatusTip=" << action->statusTip()
    //                 << "\n/>\n"   ;
    //        }
    //        else {
    //            dbgKrita << "Got a non-qaction:" << ac->objectName();
    //        }
    //    }

    // The doc and view might still exist (this is the case when closing the window)
    KisPart::instance()->removeMainWindow(this);

    delete d->viewManager;
    delete d;

}

QUuid KisMainWindow::id() const {
    return d->id;
}

void KisMainWindow::addView(KisView *view, QMdiSubWindow *subWindow)
{
    if (d->activeView == view && !subWindow) return;

    if (d->activeView) {
        d->activeView->disconnect(this);
    }

    // register the newly created view in the input manager
    viewManager()->inputManager()->addTrackedCanvas(view->canvasBase());

    showView(view, subWindow);
    updateCaption();
    emit restoringDone();

    if (d->activeView) {
        connect(d->activeView, SIGNAL(titleModified(QString,bool)), SLOT(slotDocumentTitleModified()));
        connect(d->viewManager->statusBar(), SIGNAL(memoryStatusUpdated()), this, SLOT(updateCaption()));
    }
}

void KisMainWindow::notifyChildViewDestroyed(KisView *view)
{
    /**
     * If we are the last view of the window, Qt will not activate another tab
     * before destroying tab/window. In this case we should clear all the dangling
     * pointers manually by setting the current view to null
     */
    viewManager()->inputManager()->removeTrackedCanvas(view->canvasBase());
    if (view->canvasBase() == viewManager()->canvasBase()) {
        viewManager()->setCurrentView(0);
    }
}


void KisMainWindow::showView(KisView *imageView, QMdiSubWindow *subwin)
{
    if (imageView && activeView() != imageView) {
        // XXX: find a better way to initialize this!
        imageView->setViewManager(d->viewManager);

        imageView->canvasBase()->setFavoriteResourceManager(d->viewManager->paintOpBox()->favoriteResourcesManager());
        imageView->slotLoadingFinished();

        if (!subwin) {
            subwin = d->mdiArea->addSubWindow(imageView);
        } else {
            subwin->setWidget(imageView);
        }
        imageView->setSubWindow(subwin);
        subwin->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(subwin, SIGNAL(destroyed()), SLOT(updateWindowMenu()));

        KisConfig cfg(true);
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setWindowIcon(qApp->windowIcon());

#ifdef Q_OS_MACOS
        connect(subwin, SIGNAL(destroyed()), SLOT(updateSubwindowFlags()));
        updateSubwindowFlags();
#endif

        if (d->mdiArea->subWindowList().size() == 1) {
            imageView->showMaximized();
        }
        else {
            imageView->show();
        }

        /**
         * Hack alert!
         *
         * Here we explicitly request KoToolManager to emit all the tool
         * activation signals, to reinitialize the tool options docker.
         *
         * That is needed due to a design flaw we have in the
         * initialization procedure.  The tool in the KoToolManager is
         * initialized in KisView::setViewManager() calls, which
         * happens early enough. During this call the tool manager
         * requests KoCanvasControllerWidget to emit the signal to
         * update the widgets in the tool docker. *But* at that moment
         * of time the view is not yet connected to the main window,
         * because it happens in KisViewManager::setCurrentView a bit
         * later. This fact makes the widgets updating signals be lost
         * and never reach the tool docker.
         *
         * So here we just explicitly call the tool activation stub.
         */

        KoToolManager::instance()->initializeCurrentToolForCanvas();

        // No, no, no: do not try to call this _before_ the show() has
        // been called on the view; only when that has happened is the
        // opengl context active, and very bad things happen if we tell
        // the dockers to update themselves with a view if the opengl
        // context is not active.
        setActiveView(imageView);

        updateWindowMenu();
        updateCaption();
    }
}

void KisMainWindow::slotPreferences()
{
    QScopedPointer<KisDlgPreferences> dlgPreferences(new KisDlgPreferences(this));

    if (dlgPreferences->editPreferences()) {
        KisConfigNotifier::instance()->notifyConfigChanged();
        KisConfigNotifier::instance()->notifyPixelGridModeChanged();
        KisImageConfigNotifier::instance()->notifyConfigChanged();

        // XXX: should this be changed for the views in other windows as well?
        Q_FOREACH (QPointer<KisView> koview, KisPart::instance()->views()) {
            KisViewManager *view = qobject_cast<KisViewManager*>(koview);
            if (view) {
                // Update the settings for all nodes -- they don't query
                // KisConfig directly because they need the settings during
                // compositing, and they don't connect to the config notifier
                // because nodes are not QObjects (because only one base class
                // can be a QObject).
                KisNode* node = dynamic_cast<KisNode*>(view->image()->rootLayer().data());
                node->updateSettings();
            }

        }
        updateWindowMenu();

        d->viewManager->showHideScrollbars();
    }
}

void KisMainWindow::slotThemeChanged()
{
    // save theme changes instantly
    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    group.writeEntry("Theme", d->themeManager->currentThemeName());

    // reload action icons!
    Q_FOREACH (QAction *action, actionCollection()->actions()) {
        KisIconUtils::updateIcon(action);
    }
    if (d->mdiArea) {
        d->mdiArea->setPalette(qApp->palette());
        for (int i=0; i<d->mdiArea->subWindowList().size(); i++) {
            QMdiSubWindow *window = d->mdiArea->subWindowList().at(i);
            if (window) {
                window->setPalette(qApp->palette());
                KisView *view = qobject_cast<KisView*>(window->widget());
                if (view) {
                    view->slotThemeChanged(qApp->palette());
                }
            }
        }
    }

    emit themeChanged();
}

bool KisMainWindow::canvasDetached() const
{
    return centralWidget() != d->widgetStack;
}

void KisMainWindow::setCanvasDetached(bool detach)
{
    if (detach == canvasDetached()) return;

    QWidget *outgoingWidget = centralWidget() ? takeCentralWidget() : nullptr;
    QWidget *incomingWidget = d->canvasWindow->swapMainWidget(outgoingWidget);

    if (incomingWidget) {
        setCentralWidget(incomingWidget);
    }

    if (detach) {
        d->canvasWindow->show();
    } else {
        d->canvasWindow->hide();
    }
}

void KisMainWindow::slotFileSelected(QString path)
{
    QString url = path;
    if (!url.isEmpty()) {
        bool res = openDocument(QUrl::fromLocalFile(url), Import);
        if (!res) {
            warnKrita << "Loading" << url << "failed";
        }
    }
}

void KisMainWindow::slotEmptyFilePath()
{
    QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("The chosen file's location could not be found. Does it exist?"));
}

QWidget * KisMainWindow::canvasWindow() const
{
    return d->canvasWindow;
}

void KisMainWindow::setReadWrite(bool readwrite)
{
    d->saveAction->setEnabled(readwrite);
    d->importFile->setEnabled(readwrite);
    d->readOnly = !readwrite;
    updateCaption();
}

void KisMainWindow::addRecentURL(const QUrl &url, const QUrl &oldUrl)
{
    // Add entry to recent documents list
    // (call coming from KisDocument because it must work with cmd line, template dlg, file/open, etc.)
    if (!url.isEmpty()) {
        bool ok = true;
        if (url.isLocalFile()) {
            QString path = url.adjusted(QUrl::StripTrailingSlash).toLocalFile();
            const QStringList tmpDirs = KoResourcePaths::resourceDirs("tmp");
            for (QStringList::ConstIterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it) {
                if (path.contains(*it)) {
                    ok = false; // it's in the tmp resource
                }
            }

            const QStringList templateDirs = KoResourcePaths::findDirs("templates");
            for (QStringList::ConstIterator it = templateDirs.begin() ; ok && it != templateDirs.end() ; ++it) {
                if (path.contains(*it)) {
                    ok = false; // it's in the templates directory.
                    break;
                }
            }
        }
        if (ok) {
            if (!oldUrl.isEmpty()) {
                d->recentFiles->removeUrl(oldUrl);
            }
            d->recentFiles->addUrl(url);
        }
        saveRecentFiles();

    }
}

void KisMainWindow::saveRecentFiles()
{
    // Save list of recent files
    KSharedConfigPtr config =  KSharedConfig::openConfig();
    d->recentFiles->saveEntries(config->group("RecentFiles"));
    config->sync();

    // Tell all windows to reload their list, after saving
    // Doesn't work multi-process, but it's a start
    Q_FOREACH (KisMainWindow *mw, KisPart::instance()->mainWindows()) {
        if (mw != this) {
            mw->reloadRecentFileList();
        }
    }
}

QList<QUrl> KisMainWindow::recentFilesUrls()
{
    return d->recentFiles->urls();
}

void KisMainWindow::clearRecentFiles()
{
    d->recentFiles->clear();
    d->welcomePage->populateRecentDocuments();
}

void KisMainWindow::removeRecentUrl(const QUrl &url)
{
    d->recentFiles->removeUrl(url);
    KSharedConfigPtr config =  KSharedConfig::openConfig();
    d->recentFiles->saveEntries(config->group("RecentFiles"));
    config->sync();
}

void KisMainWindow::reloadRecentFileList()
{
    d->recentFiles->loadEntries(KSharedConfig::openConfig()->group("RecentFiles"));
}

void KisMainWindow::updateCaption()
{
    if (!d->mdiArea->activeSubWindow()) {
        updateCaption(QString(), false);
    }
    else if (d->activeView && d->activeView->document() && d->activeView->image()){
        KisDocument *doc = d->activeView->document();

        QString caption(doc->caption());

        caption = "RESOURCES REWRITE GOING ON " + caption;


        if (d->readOnly) {
            caption += " [" + i18n("Write Protected") + "] ";
        }

        if (doc->isRecovered()) {
            caption += " [" + i18n("Recovered") + "] ";
        }

        // show the file size for the document
        KisMemoryStatisticsServer::Statistics m_fileSizeStats = KisMemoryStatisticsServer::instance()->fetchMemoryStatistics(d->activeView ? d->activeView->image() : 0);

        if (m_fileSizeStats.imageSize) {
            caption += QString(" (").append( KFormat().formatByteSize(m_fileSizeStats.imageSize)).append( ")");
        }

        updateCaption(caption, doc->isModified());

        if (!doc->url().fileName().isEmpty()) {
            d->saveAction->setToolTip(i18n("Save as %1", doc->url().fileName()));
        }
        else {
            d->saveAction->setToolTip(i18n("Save"));
        }


    }

}

void KisMainWindow::updateCaption(const QString &caption, bool modified)
{
    QString versionString = KritaVersionWrapper::versionString(true);

    QString title = caption;
    if (!title.contains(QStringLiteral("[*]"))) { // append the placeholder so that the modified mechanism works
        title.append(QStringLiteral(" [*]"));
    }

    if (d->mdiArea->activeSubWindow()) {
#if defined(KRITA_ALPHA) || defined (KRITA_BETA) || defined (KRITA_RC)
        d->mdiArea->activeSubWindow()->setWindowTitle(QString("%1: %2").arg(versionString).arg(title));
#else
        d->mdiArea->activeSubWindow()->setWindowTitle(title);
#endif
        d->mdiArea->activeSubWindow()->setWindowModified(modified);
    }
    else {
#if defined(KRITA_ALPHA) || defined (KRITA_BETA) || defined (KRITA_RC)
    setWindowTitle(QString("%1: %2").arg(versionString).arg(title));
#else
    setWindowTitle(title);
#endif
    }
    setWindowModified(modified);


}


KisView *KisMainWindow::activeView() const
{
    if (d->activeView) {
        return d->activeView;
    }
    return 0;
}

bool KisMainWindow::openDocument(const QUrl &url, OpenFlags flags)
{
    if (!QFile(url.toLocalFile()).exists()) {
        if (!(flags & BatchMode)) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("The file %1 does not exist.", url.url()));
        }
        d->recentFiles->removeUrl(url); //remove the file from the recent-opened-file-list
        saveRecentFiles();
        return false;
    }
    return openDocumentInternal(url, flags);
}

bool KisMainWindow::openDocumentInternal(const QUrl &url, OpenFlags flags)
{
    if (!url.isLocalFile()) {
        qWarning() << "KisMainWindow::openDocumentInternal. Not a local file:" << url;
        return false;
    }

    KisDocument *newdoc = KisPart::instance()->createDocument();

    if (flags & BatchMode) {
        newdoc->setFileBatchMode(true);
    }

    d->firstTime = true;
    connect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newdoc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));

    KisDocument::OpenFlags openFlags = KisDocument::None;
    // XXX: Why this duplication of of OpenFlags...
    if (flags & RecoveryFile) {
        openFlags |= KisDocument::RecoveryFile;
    }

    bool openRet = !(flags & Import) ? newdoc->openUrl(url, openFlags) : newdoc->importDocument(url);

    if (!openRet) {
        delete newdoc;
        return false;
    }

    KisPart::instance()->addDocument(newdoc);

    if (!QFileInfo(url.toLocalFile()).isWritable()) {
        setReadWrite(false);
    }

    // Try to determine whether this was an unnamed autosave
    if (flags & RecoveryFile &&
            (   url.toLocalFile().startsWith(QDir::tempPath())
                || url.toLocalFile().startsWith(QDir::homePath())
                ) &&
            (      QFileInfo(url.toLocalFile()).fileName().startsWith(".krita")
                   || QFileInfo(url.toLocalFile()).fileName().startsWith("krita")
                   )
            )
    {
        QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (!QFileInfo(path).exists()) {
            path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
        newdoc->setUrl(QUrl::fromLocalFile( path + "/" + newdoc->objectName() + ".kra"));
    }

    return true;
}

void KisMainWindow::showDocument(KisDocument *document) {
    Q_FOREACH(QMdiSubWindow *subwindow, d->mdiArea->subWindowList()) {
        KisView *view = qobject_cast<KisView*>(subwindow->widget());
        KIS_SAFE_ASSERT_RECOVER_NOOP(view);

        if (view) {
            if (view->document() == document) {
                setActiveSubWindow(subwindow);
                return;
            }
        }
    }

    addViewAndNotifyLoadingCompleted(document);
}

KisView* KisMainWindow::addViewAndNotifyLoadingCompleted(KisDocument *document,
                                                         QMdiSubWindow *subWindow)
{
    showWelcomeScreen(false); // see workaround in function header

    KisView *view = KisPart::instance()->createView(document, d->viewManager, this);
    addView(view, subWindow);

    emit guiLoadingFinished();

    return view;
}

QStringList KisMainWindow::showOpenFileDialog(bool isImporting)
{
    KoFileDialog dialog(this, KoFileDialog::ImportFiles, "OpenDocument");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    dialog.setCaption(isImporting ? i18n("Import Images") : i18n("Open Images"));

    return dialog.filenames();
}

// Separate from openDocument to handle async loading (remote URLs)
void KisMainWindow::slotLoadCompleted()
{
    KisDocument *newdoc = qobject_cast<KisDocument*>(sender());
    if (newdoc && newdoc->image()) {
        addViewAndNotifyLoadingCompleted(newdoc);

        disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
        disconnect(newdoc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));

        emit loadCompleted();
    }
}

void KisMainWindow::slotLoadCanceled(const QString & errMsg)
{
    KisUsageLogger::log(QString("Loading canceled. Error:").arg(errMsg));
    if (!errMsg.isEmpty())   // empty when canceled by user
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), errMsg);
    // ... can't delete the document, it's the one who emitted the signal...

    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(doc, SIGNAL(canceled(QString)), this, SLOT(slotLoadCanceled(QString)));
}

void KisMainWindow::slotSaveCanceled(const QString &errMsg)
{
    KisUsageLogger::log(QString("Saving canceled. Error:").arg(errMsg));
    if (!errMsg.isEmpty()) {   // empty when canceled by user
        QMessageBox::critical(this, i18nc("@title:window", "Krita"), errMsg);
    }
    slotSaveCompleted();
}

void KisMainWindow::slotSaveCompleted()
{
    KisUsageLogger::log(QString("Saving Completed"));
    KisDocument* doc = qobject_cast<KisDocument*>(sender());
    Q_ASSERT(doc);
    disconnect(doc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    disconnect(doc, SIGNAL(canceled(QString)), this, SLOT(slotSaveCanceled(QString)));

    if (d->deferredClosingEvent) {
        KXmlGuiWindow::closeEvent(d->deferredClosingEvent);
    }
}

bool KisMainWindow::hackIsSaving() const
{
    StdLockableWrapper<QMutex> wrapper(&d->savingEntryMutex);
    std::unique_lock<StdLockableWrapper<QMutex>> l(wrapper, std::try_to_lock);
    return !l.owns_lock();
}

bool KisMainWindow::installBundle(const QString &fileName) const
{
    QFileInfo from(fileName);
    QFileInfo to(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bundles/" + from.fileName());
    if (to.exists()) {
        QFile::remove(to.canonicalFilePath());
    }
    return QFile::copy(fileName, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/bundles/" + from.fileName());
}

QImage KisMainWindow::layoutThumbnail()
{
    int size = 256;
    qreal scale = qreal(size)/qreal(qMax(geometry().width(), geometry().height()));
    QImage layoutThumbnail = QImage(qRound(geometry().width()*scale), qRound(geometry().height()*scale), QImage::Format_ARGB32);
    QPainter gc(&layoutThumbnail);
    gc.fillRect(0, 0, layoutThumbnail.width(), layoutThumbnail.height(), this->palette().dark());

    for (int childW = 0; childW< children().size(); childW++) {
        if (children().at(childW)->isWidgetType()) {
            QWidget *w = dynamic_cast<QWidget*>(children().at(childW));

            if (w->isVisible()) {
                QRect wRect = QRectF(w->geometry().x()*scale
                                     , w->geometry().y()*scale
                                     , w->geometry().width()*scale
                                     , w->geometry().height()*scale
                                     ).toRect();

                wRect = wRect.intersected(layoutThumbnail.rect().adjusted(-1, -1, -1, -1));

                gc.setBrush(this->palette().window());
                if (w == d->widgetStack) {
                    gc.setBrush(d->mdiArea->background());
                }
                gc.setPen(this->palette().windowText().color());
                gc.drawRect(wRect);
            }
        }
    }
    gc.end();
    return layoutThumbnail;
}

bool KisMainWindow::saveDocument(KisDocument *document, bool saveas, bool isExporting)
{
    if (!document) {
        return true;
    }

    /**
     * Make sure that we cannot enter this method twice!
     *
     * The lower level functions may call processEvents() so
     * double-entry is quite possible to achieve. Here we try to lock
     * the mutex, and if it is failed, just cancel saving.
     */
    StdLockableWrapper<QMutex> wrapper(&d->savingEntryMutex);
    std::unique_lock<StdLockableWrapper<QMutex>> l(wrapper, std::try_to_lock);
    if (!l.owns_lock()) return false;

    // no busy wait for saving because it is dangerous!
    KisDelayedSaveDialog dlg(document->image(), KisDelayedSaveDialog::SaveDialog, 0, this);
    dlg.blockIfImageIsBusy();

    if (dlg.result() == KisDelayedSaveDialog::Rejected) {
        return false;
    }
    else if (dlg.result() == KisDelayedSaveDialog::Ignored) {
        QMessageBox::critical(0,
                              i18nc("@title:window", "Krita"),
                              i18n("You are saving a file while the image is "
                                   "still rendering. The saved file may be "
                                   "incomplete or corrupted.\n\n"
                                   "Please select a location where the original "
                                   "file will not be overridden!"));


        saveas = true;
    }

    if (document->isRecovered()) {
        saveas = true;
    }

    if (document->url().isEmpty()) {
        saveas = true;
    }

    connect(document, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(document, SIGNAL(canceled(QString)), this, SLOT(slotSaveCanceled(QString)));

    QByteArray nativeFormat = document->nativeFormatMimeType();
    QByteArray oldMimeFormat = document->mimeType();

    QUrl suggestedURL = document->url();

    QStringList mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);

    mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export);
    if (!mimeFilter.contains(oldMimeFormat)) {
        dbgUI << "KisMainWindow::saveDocument no export filter for" << oldMimeFormat;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = QFileInfo(suggestedURL.toLocalFile()).completeBaseName();

        if (!suggestedFilename.isEmpty()) {  // ".kra" looks strange for a name
            suggestedFilename = suggestedFilename + "." + KisMimeDatabase::suffixesForMimeType(KIS_MIME_TYPE).first();
            suggestedURL = suggestedURL.adjusted(QUrl::RemoveFilename);
            suggestedURL.setPath(suggestedURL.path() + suggestedFilename);
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if (document->url().isEmpty() || isExporting || saveas) {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog dialog(this, KoFileDialog::SaveFile, "SaveAs");
        dialog.setCaption(isExporting ? i18n("Exporting") : i18n("Saving As"));

        //qDebug() << ">>>>>" << isExporting << d->lastExportLocation << d->lastExportedFormat << QString::fromLatin1(document->mimeType());

        if (isExporting && !d->lastExportLocation.isEmpty() && !d->lastExportLocation.contains(QDir::tempPath())) {

            // Use the location where we last exported to, if it's set, as the opening location for the file dialog
            QString proposedPath = QFileInfo(d->lastExportLocation).absolutePath();
            // If the document doesn't have a filename yet, use the title
            QString proposedFileName = suggestedURL.isEmpty() ? document->documentInfo()->aboutInfo("title") :  QFileInfo(suggestedURL.toLocalFile()).completeBaseName();
            // Use the last mimetype we exported to by default
            QString proposedMimeType =  d->lastExportedFormat.isEmpty() ? "" : d->lastExportedFormat;
            QString proposedExtension = KisMimeDatabase::suffixesForMimeType(proposedMimeType).first().remove("*,");

            // Set the default dir: this overrides the one loaded from the config file, since we're exporting and the lastExportLocation is not empty
            dialog.setDefaultDir(proposedPath + "/" + proposedFileName + "." + proposedExtension, true);
            dialog.setMimeTypeFilters(mimeFilter, proposedMimeType);
        }
        else {
            // Get the last used location for saving
            KConfigGroup group =  KSharedConfig::openConfig()->group("File Dialogs");
            QString proposedPath = group.readEntry("SaveAs", "");
            // if that is empty, get the last used location for loading
            if (proposedPath.isEmpty()) {
                proposedPath = group.readEntry("OpenDocument", "");
            }
            // If that is empty, too, use the Pictures location.
            if (proposedPath.isEmpty()) {
                proposedPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
            }
            // But only use that if the suggestedUrl, that is, the document's own url is empty, otherwise
            // open the location where the document currently is.
            dialog.setDefaultDir(suggestedURL.isEmpty() ? proposedPath : suggestedURL.toLocalFile(), true);

            // If exporting, default to all supported file types if user is exporting
            QByteArray default_mime_type = "";
            if (!isExporting) {
                // otherwise use the document's mimetype, or if that is empty, kra, which is the savest.
                default_mime_type = document->mimeType().isEmpty() ? nativeFormat : document->mimeType();
            }
            dialog.setMimeTypeFilters(mimeFilter, QString::fromLatin1(default_mime_type));
        }

        QUrl newURL = QUrl::fromUserInput(dialog.filename());

        if (newURL.isLocalFile()) {
            QString fn = newURL.toLocalFile();
            if (QFileInfo(fn).completeSuffix().isEmpty()) {
                fn.append(KisMimeDatabase::suffixesForMimeType(nativeFormat).first());
                newURL = QUrl::fromLocalFile(fn);
            }
        }

        if (document->documentInfo()->aboutInfo("title") == i18n("Unnamed")) {
            QString fn = newURL.toLocalFile();
            QFileInfo info(fn);
            document->documentInfo()->setAboutInfo("title", info.completeBaseName());
        }

        QByteArray outputFormat = nativeFormat;

        QString outputFormatString = KisMimeDatabase::mimeTypeForFile(newURL.toLocalFile(), false);
        outputFormat = outputFormatString.toLatin1();


        if (!isExporting) {
            justChangingFilterOptions = (newURL == document->url()) && (outputFormat == document->mimeType());
        }
        else {
            QString path = QFileInfo(d->lastExportLocation).absolutePath();
            QString filename = QFileInfo(document->url().toLocalFile()).completeBaseName();
            justChangingFilterOptions = (QFileInfo(newURL.toLocalFile()).absolutePath() == path)
                    && (QFileInfo(newURL.toLocalFile()).completeBaseName() == filename)
                    && (outputFormat == d->lastExportedFormat);
        }

        bool bOk = true;
        if (newURL.isEmpty()) {
            bOk = false;
        }

        if (bOk) {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions) {
                if (!document->isNativeFormat(outputFormat))
                    wantToSave = true;
            }

            if (wantToSave) {
                if (!isExporting) {  // Save As
                    ret = document->saveAs(newURL, outputFormat, true);
                    if (ret) {
                        dbgUI << "Successful Save As!";
                        KisPart::instance()->addRecentURLToAllMainWindows(newURL);
                        setReadWrite(true);
                    } else {
                        dbgUI << "Failed Save As!";

                    }
                }
                else { // Export
                    ret = document->exportDocument(newURL, outputFormat);

                    if (ret) {
                        d->lastExportLocation = newURL.toLocalFile();
                        d->lastExportedFormat = outputFormat;
                    }
                }

            }   // if (wantToSave)  {
            else
                ret = false;
        }   // if (bOk) {
        else
            ret = false;
    } else { // saving
        // We cannot "export" into the currently
        // opened document. We are not Gimp.
        KIS_ASSERT_RECOVER_NOOP(!isExporting);

        // be sure document has the correct outputMimeType!
        if (document->isModified()) {
            ret = document->save(true, 0);
        }

        if (!ret) {
            dbgUI << "Failed Save!";
        }
    }

    updateCaption();

    return ret;
}

void KisMainWindow::undo()
{
    if (activeView()) {
        activeView()->document()->undoStack()->undo();
    }
}

void KisMainWindow::redo()
{
    if (activeView()) {
        activeView()->document()->undoStack()->redo();
    }
}

void KisMainWindow::closeEvent(QCloseEvent *e)
{
    if (hackIsSaving()) {
        e->setAccepted(false);
        return;
    }

    if (!KisPart::instance()->closingSession()) {
        QAction *action= d->viewManager->actionCollection()->action("view_show_canvas_only");
        if ((action) && (action->isChecked())) {
            action->setChecked(false);
        }

        // Save session when last window is closed
        if (KisPart::instance()->mainwindowCount() == 1) {
            bool closeAllowed = KisPart::instance()->closeSession();

            if (!closeAllowed) {
                e->setAccepted(false);
                return;
            }
        }
    }

    d->mdiArea->closeAllSubWindows();

    QList<QMdiSubWindow*> childrenList = d->mdiArea->subWindowList();

    if (childrenList.isEmpty()) {
        d->deferredClosingEvent = e;
        saveWindowState(true);
        d->canvasWindow->close();
    } else {
        e->setAccepted(false);
    }
}

void KisMainWindow::saveWindowSettings()
{
    KSharedConfigPtr config =  KSharedConfig::openConfig();

    if (d->windowSizeDirty ) {
        dbgUI << "KisMainWindow::saveWindowSettings";
        KConfigGroup group = d->windowStateConfig;
        KWindowConfig::saveWindowSize(windowHandle(), group);
        config->sync();
        d->windowSizeDirty = false;
    }

    if (!d->activeView || d->activeView->document()) {

        // Save toolbar position into the config file of the app, under the doc's component name
        KConfigGroup group = d->windowStateConfig;
        saveMainWindowSettings(group);

        // Save collapsible state of dock widgets
        for (QMap<QString, QDockWidget*>::const_iterator i = d->dockWidgetsMap.constBegin();
             i != d->dockWidgetsMap.constEnd(); ++i) {
            if (i.value()->widget()) {
                KConfigGroup dockGroup = group.group(QString("DockWidget ") + i.key());
                dockGroup.writeEntry("Collapsed", i.value()->widget()->isHidden());
                dockGroup.writeEntry("Locked", i.value()->property("Locked").toBool());
                dockGroup.writeEntry("DockArea", (int) dockWidgetArea(i.value()));
                dockGroup.writeEntry("xPosition", (int) i.value()->widget()->x());
                dockGroup.writeEntry("yPosition", (int) i.value()->widget()->y());

                dockGroup.writeEntry("width", (int) i.value()->widget()->width());
                dockGroup.writeEntry("height", (int) i.value()->widget()->height());
            }
        }

    }

    KSharedConfig::openConfig()->sync();
    resetAutoSaveSettings(); // Don't let KMainWindow override the good stuff we wrote down

}

void KisMainWindow::resizeEvent(QResizeEvent * e)
{
    d->windowSizeDirty = true;
    KXmlGuiWindow::resizeEvent(e);
}

void KisMainWindow::setActiveView(KisView* view)
{
    d->activeView = view;
    updateCaption();

    if (d->undoActionsUpdateManager) {
        d->undoActionsUpdateManager->setCurrentDocument(view ? view->document() : 0);
    }

    d->viewManager->setCurrentView(view);

    KisWindowLayoutManager::instance()->activeDocumentChanged(view->document());
}

void KisMainWindow::dragMove(QDragMoveEvent * event)
{
    QTabBar *tabBar = d->findTabBarHACK();

    if (!tabBar && d->mdiArea->viewMode() == QMdiArea::TabbedView) {
        qWarning() << "WARNING!!! Cannot find QTabBar in the main window! Looks like Qt has changed behavior. Drag & Drop between multiple tabs might not work properly (tabs will not switch automatically)!";
    }

    if (tabBar && tabBar->isVisible()) {
        QPoint pos = tabBar->mapFromGlobal(mapToGlobal(event->pos()));
        if (tabBar->rect().contains(pos)) {
            const int tabIndex = tabBar->tabAt(pos);

            if (tabIndex >= 0 && tabBar->currentIndex() != tabIndex) {
                d->tabSwitchCompressor->start(tabIndex);
            }
        } else if (d->tabSwitchCompressor->isActive()) {
            d->tabSwitchCompressor->stop();
        }
    }
}

void KisMainWindow::dragLeave()
{
    if (d->tabSwitchCompressor->isActive()) {
        d->tabSwitchCompressor->stop();
    }
}


void KisMainWindow::switchTab(int index)
{
    QTabBar *tabBar = d->findTabBarHACK();
    if (!tabBar) return;

    tabBar->setCurrentIndex(index);
}

void KisMainWindow::showWelcomeScreen(bool show)
{
    d->widgetStack->setCurrentIndex(!show);
}

void KisMainWindow::slotFileNew()
{
    const QStringList mimeFilter = KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import);

    KisOpenPane *startupWidget = new KisOpenPane(this, mimeFilter, QStringLiteral("templates/"));
    startupWidget->setWindowModality(Qt::WindowModal);
    startupWidget->setWindowTitle(i18n("Create new document"));


    KisConfig cfg(true);

    int w = cfg.defImageWidth();
    int h = cfg.defImageHeight();
    const double resolution = cfg.defImageResolution();
    const QString colorModel = cfg.defColorModel();
    const QString colorDepth = cfg.defaultColorDepth();
    const QString colorProfile = cfg.defColorProfile();


    CustomDocumentWidgetItem item;
    item.widget = new KisCustomImageWidget(startupWidget,
                                           w,
                                           h,
                                           resolution,
                                           colorModel,
                                           colorDepth,
                                           colorProfile,
                                           i18n("Unnamed"));

    item.icon = "document-new";
    item.title = i18n("Custom Document");
    startupWidget->addCustomDocumentWidget(item.widget, item.title, "Custom Document", item.icon);

    QSize sz = KisClipboard::instance()->clipSize();
    if (sz.isValid() && sz.width() != 0 && sz.height() != 0) {
        w = sz.width();
        h = sz.height();
    }

    item.widget = new KisImageFromClipboard(startupWidget,
                                            w,
                                            h,
                                            resolution,
                                            colorModel,
                                            colorDepth,
                                            colorProfile,
                                            i18n("Unnamed"));

    item.title = i18n("Create from Clipboard");
    item.icon = "tab-new";

    startupWidget->addCustomDocumentWidget(item.widget, item.title, "Create from ClipBoard", item.icon);

    // calls deleteLater
    connect(startupWidget, SIGNAL(documentSelected(KisDocument*)), KisPart::instance(), SLOT(startCustomDocument(KisDocument*)));
    // calls deleteLater
    connect(startupWidget, SIGNAL(openTemplate(QUrl)), KisPart::instance(), SLOT(openTemplate(QUrl)));

    startupWidget->exec();

    // Cancel calls deleteLater...

}

void KisMainWindow::slotImportFile()
{
    dbgUI << "slotImportFile()";
    slotFileOpen(true);
}


void KisMainWindow::slotFileOpen(bool isImporting)
{
#ifndef Q_OS_ANDROID
    QStringList urls = showOpenFileDialog(isImporting);

    if (urls.isEmpty())
        return;

    Q_FOREACH (const QString& url, urls) {

        if (!url.isEmpty()) {
            OpenFlags flags = isImporting ? Import : None;
            bool res = openDocument(QUrl::fromLocalFile(url), flags);
            if (!res) {
                warnKrita << "Loading" << url << "failed";
            }
        }
    }
#else
    Q_UNUSED(isImporting)

    d->fileManager->openImportFile();
    connect(d->fileManager, SIGNAL(sigFileSelected(QString)), this, SLOT(slotFileSelected(QString)));
    connect(d->fileManager, SIGNAL(sigEmptyFilePath()), this, SLOT(slotEmptyFilePath()));
#endif
}

void KisMainWindow::slotFileOpenRecent(const QUrl &url)
{
    (void) openDocument(QUrl::fromLocalFile(url.toLocalFile()), None);
}

void KisMainWindow::slotFileSave()
{
    if (saveDocument(d->activeView->document(), false, false)) {
        emit documentSaved();
    }
}

void KisMainWindow::slotFileSaveAs()
{
    if (saveDocument(d->activeView->document(), true, false)) {
        emit documentSaved();
    }
}

void KisMainWindow::slotExportFile()
{
    if (saveDocument(d->activeView->document(), true, true)) {
        emit documentSaved();
    }
}

void KisMainWindow::slotShowSessionManager() {
    KisPart::instance()->showSessionManager();
}

KoCanvasResourceProvider *KisMainWindow::resourceManager() const
{
    return d->viewManager->canvasResourceProvider()->resourceManager();
}

int KisMainWindow::viewCount() const
{
    return d->mdiArea->subWindowList().size();
}

const KConfigGroup &KisMainWindow::windowStateConfig() const
{
    return d->windowStateConfig;
}

void KisMainWindow::saveWindowState(bool restoreNormalState)
{
    if (restoreNormalState) {
        QAction *showCanvasOnly = d->viewManager->actionCollection()->action("view_show_canvas_only");

        if (showCanvasOnly && showCanvasOnly->isChecked()) {
            showCanvasOnly->setChecked(false);
        }

        d->windowStateConfig.writeEntry("ko_geometry", saveGeometry().toBase64());
        d->windowStateConfig.writeEntry("State", saveState().toBase64());

        if (!d->dockerStateBeforeHiding.isEmpty()) {
            restoreState(d->dockerStateBeforeHiding);
        }

        statusBar()->setVisible(true);
        menuBar()->setVisible(true);

        saveWindowSettings();

    } else {
        saveMainWindowSettings(d->windowStateConfig);
    }

}

bool KisMainWindow::restoreWorkspaceState(const QByteArray &state)
{
    QByteArray oldState = saveState();

    // needed because otherwise the layout isn't correctly restored in some situations
    Q_FOREACH (QDockWidget *dock, dockWidgets()) {
        dock->toggleViewAction()->setEnabled(true);
        dock->hide();
    }

    bool success = KXmlGuiWindow::restoreState(state);

    if (!success) {
        KXmlGuiWindow::restoreState(oldState);
        return false;
    }

    return success;
}

bool KisMainWindow::restoreWorkspace(int workspaceId)
{

    KisWorkspaceResourceSP workspace =
            KisResourceModelProvider::resourceModel(ResourceType::Workspaces)
            ->resourceForId(workspaceId).dynamicCast<KisWorkspaceResource>();

    bool success = restoreWorkspaceState(workspace->dockerState());

    if (activeKisView()) {
        activeKisView()->resourceProvider()->notifyLoadingWorkspace(workspace);
    }

    return success;
}

QByteArray KisMainWindow::borrowWorkspace(KisMainWindow *other)
{
    QByteArray currentWorkspace = saveState();

    if (!d->workspaceBorrowedBy.isNull()) {
        if (other->id() == d->workspaceBorrowedBy) {
            // We're swapping our original workspace back
            d->workspaceBorrowedBy = QUuid();
            return currentWorkspace;
        } else {
            // Get our original workspace back before swapping with a third window
            KisMainWindow *borrower = KisPart::instance()->windowById(d->workspaceBorrowedBy);
            if (borrower) {
                QByteArray originalLayout = borrower->borrowWorkspace(this);
                borrower->restoreWorkspaceState(currentWorkspace);

                d->workspaceBorrowedBy = other->id();
                return originalLayout;
            }
        }
    }

    d->workspaceBorrowedBy = other->id();
    return currentWorkspace;
}

void KisMainWindow::swapWorkspaces(KisMainWindow *a, KisMainWindow *b)
{
    QByteArray workspaceA = a->borrowWorkspace(b);
    QByteArray workspaceB = b->borrowWorkspace(a);

    a->restoreWorkspaceState(workspaceB);
    b->restoreWorkspaceState(workspaceA);
}

KisViewManager *KisMainWindow::viewManager() const
{
    return d->viewManager;
}

void KisMainWindow::slotDocumentInfo()
{
    if (!d->activeView->document())
        return;

    KoDocumentInfo *docInfo = d->activeView->document()->documentInfo();

    if (!docInfo)
        return;

    KoDocumentInfoDlg *dlg = d->activeView->document()->createDocumentInfoDialog(this, docInfo);

    if (dlg->exec()) {
        if (dlg->isDocumentSaved()) {
            d->activeView->document()->setModified(false);
        } else {
            d->activeView->document()->setModified(true);
        }
        d->activeView->document()->setTitleModified();
    }

    delete dlg;
}

bool KisMainWindow::slotFileCloseAll()
{
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        if (subwin) {
            if(!subwin->close())
                return false;
        }
    }

    updateCaption();
    return true;
}

void KisMainWindow::slotFileQuit()
{
    // Do not close while KisMainWindow has the savingEntryMutex locked, bug409395.
    // After the background saving job is initiated, KisDocument blocks closing
    // while it saves itself.
    if (hackIsSaving()) {
        return;
    }
    KisPart::instance()->closeSession();
}

void KisMainWindow::importAnimation()
{
    if (!activeView()) return;

    KisDocument *document = activeView()->document();
    if (!document) return;

    KisDlgImportImageSequence dlg(this, document);

    if (dlg.exec() == QDialog::Accepted) {
        QStringList files = dlg.files();
        int firstFrame = dlg.firstFrame();
        int step = dlg.step();

        KoUpdaterPtr updater =
                !document->fileBatchMode() ? viewManager()->createUnthreadedUpdater(i18n("Import frames")) : 0;
        KisAnimationImporter importer(document->image(), updater);
        KisImportExportErrorCode status = importer.import(files, firstFrame, step);

        if (!status.isOk() && !status.isInternalError()) {
            QString msg = status.errorMessage();
            if (!msg.isEmpty())
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not finish import animation:\n%1", msg));
        }
        activeView()->canvasBase()->refetchDataFromImage();
    }
}

void KisMainWindow::slotConfigureToolbars()
{
    saveWindowState();
    KEditToolBar edit(factory(), this);
    connect(&edit, SIGNAL(newToolBarConfig()), this, SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
    applyToolBarLayout();
}

void KisMainWindow::slotResetConfigurations()
{
    KisApplication *kisApp = static_cast<KisApplication*>(qApp);
    kisApp->askresetConfig();
}

void KisMainWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(d->windowStateConfig);

    KXMLGUIFactory *factory = guiFactory();
    Q_UNUSED(factory);

    // Check if there's an active view
    if (!d->activeView)
        return;

    plugActionList("toolbarlist", d->toolbarList);
    applyToolBarLayout();
}

void KisMainWindow::slotToolbarToggled(bool toggle)
{
    //dbgUI <<"KisMainWindow::slotToolbarToggled" << sender()->name() <<" toggle=" << true;
    // The action (sender) and the toolbar have the same name
    KToolBar * bar = toolBar(sender()->objectName());
    if (bar) {
        if (toggle) {
            bar->show();
        }
        else {
            bar->hide();
        }

        if (d->activeView && d->activeView->document()) {
            saveWindowState();
        }
    } else
        warnUI << "slotToolbarToggled : Toolbar " << sender()->objectName() << " not found!";
}

void KisMainWindow::viewFullscreen(bool fullScreen)
{
    KisConfig cfg(false);
    cfg.setFullscreenMode(fullScreen);

    if (fullScreen) {
        setWindowState(windowState() | Qt::WindowFullScreen);   // set
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);   // reset
    }
    d->fullScreenMode->setChecked(isFullScreen());
}

void KisMainWindow::setMaxRecentItems(uint _number)
{
    d->recentFiles->setMaxItems(_number);
}

QDockWidget* KisMainWindow::createDockWidget(KoDockFactoryBase* factory)
{
    QDockWidget* dockWidget = 0;
    bool lockAllDockers = KisConfig(true).readEntry<bool>("LockAllDockerPanels", false);

    if (!d->dockWidgetsMap.contains(factory->id())) {
        dockWidget = factory->createDockWidget();

        // It is quite possible that a dock factory cannot create the dock; don't
        // do anything in that case.
        if (!dockWidget) {
            warnKrita << "Could not create docker for" << factory->id();
            return 0;
        }

        dockWidget->setFont(KoDockRegistry::dockFont());
        dockWidget->setObjectName(factory->id());
        dockWidget->setParent(this);
        if (lockAllDockers) {
            if (dockWidget->titleBarWidget()) {
                dockWidget->titleBarWidget()->setVisible(false);
            }
            dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }
        if (dockWidget->widget() && dockWidget->widget()->layout())
            dockWidget->widget()->layout()->setContentsMargins(1, 1, 1, 1);

        Qt::DockWidgetArea side = Qt::RightDockWidgetArea;
        bool visible = true;

        switch (factory->defaultDockPosition()) {
        case KoDockFactoryBase::DockTornOff:
            dockWidget->setFloating(true); // position nicely?
            break;
        case KoDockFactoryBase::DockTop:
            side = Qt::TopDockWidgetArea; break;
        case KoDockFactoryBase::DockLeft:
            side = Qt::LeftDockWidgetArea; break;
        case KoDockFactoryBase::DockBottom:
            side = Qt::BottomDockWidgetArea; break;
        case KoDockFactoryBase::DockRight:
            side = Qt::RightDockWidgetArea; break;
        case KoDockFactoryBase::DockMinimized:
        default:
            side = Qt::RightDockWidgetArea;
            visible = false;
        }

        KConfigGroup group = d->windowStateConfig.group("DockWidget " + factory->id());
        side = static_cast<Qt::DockWidgetArea>(group.readEntry("DockArea", static_cast<int>(side)));
        if (side == Qt::NoDockWidgetArea) side = Qt::RightDockWidgetArea;

        addDockWidget(side, dockWidget);
        if (!visible) {
            dockWidget->hide();
        }

        d->dockWidgetsMap.insert(factory->id(), dockWidget);
    }
    else {
        dockWidget = d->dockWidgetsMap[factory->id()];
    }

#ifdef Q_OS_MACOS
    dockWidget->setAttribute(Qt::WA_MacSmallSize, true);
#endif
    dockWidget->setFont(KoDockRegistry::dockFont());

    connect(dockWidget, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(forceDockTabFonts()));

    return dockWidget;
}

void KisMainWindow::forceDockTabFonts()
{
    Q_FOREACH (QObject *child, children()) {
        if (child->inherits("QTabBar")) {
            ((QTabBar *)child)->setFont(KoDockRegistry::dockFont());
        }
    }
}

void KisMainWindow::slotUpdateWidgetStyle()
{
     KisConfig cfg(true);
     QString themeFromConfig = cfg.widgetStyle();

     Q_FOREACH (auto key, d->actionMap.keys()) { // find checked style to save to config
         if(d->actionMap.value(key)->isChecked()) {
            cfg.setWidgetStyle(key);
            qApp->setStyle(key);
         }
     }
}

QList<QDockWidget*> KisMainWindow::dockWidgets() const
{
    return d->dockWidgetsMap.values();
}

QDockWidget* KisMainWindow::dockWidget(const QString &id)
{
    if (!d->dockWidgetsMap.contains(id)) return 0;
    return d->dockWidgetsMap[id];
}

QList<KoCanvasObserverBase*> KisMainWindow::canvasObservers() const
{
    QList<KoCanvasObserverBase*> observers;

    Q_FOREACH (QDockWidget *docker, dockWidgets()) {
        KoCanvasObserverBase *observer = dynamic_cast<KoCanvasObserverBase*>(docker);
        if (observer) {
            observers << observer;
        }
        else {
            warnKrita << docker << "is not a canvas observer";
        }
    }
    return observers;
}


void KisMainWindow::toggleDockersVisibility(bool visible)
{
    if (!visible) {
        d->dockerStateBeforeHiding = saveState();

        Q_FOREACH (QObject* widget, children()) {
            if (widget->inherits("QDockWidget")) {
                QDockWidget* dw = static_cast<QDockWidget*>(widget);
                if (dw->isVisible()) {
                    dw->hide();
                }
            }
        }
    }
    else {
        restoreState(d->dockerStateBeforeHiding);
    }
}

void KisMainWindow::slotDocumentTitleModified()
{
    updateCaption();
}


void KisMainWindow::subWindowActivated()
{
    bool enabled = (activeKisView() != 0);

    d->mdiCascade->setEnabled(enabled);
    d->mdiNextWindow->setEnabled(enabled);
    d->mdiPreviousWindow->setEnabled(enabled);
    d->mdiTile->setEnabled(enabled);
    d->close->setEnabled(enabled);
    d->closeAll->setEnabled(enabled);

    setActiveSubWindow(d->mdiArea->activeSubWindow());
    Q_FOREACH (QToolBar *tb, toolBars()) {
        if (tb->objectName() == "BrushesAndStuff") {
            tb->setEnabled(enabled);
        }
    }

    /**
     * Qt has a weirdness, it has hardcoded shortcuts added to an action
     * in the window menu. We need to reset the shortcuts for that menu
     * to nothing, otherwise the shortcuts cannot be made configurable.
     *
     * See: https://bugs.kde.org/show_bug.cgi?id=352205
     *      https://bugs.kde.org/show_bug.cgi?id=375524
     *      https://bugs.kde.org/show_bug.cgi?id=398729
     */
    QMdiSubWindow *subWindow = d->mdiArea->currentSubWindow();
    if (subWindow) {
        QMenu *menu = subWindow->systemMenu();
        if (menu && menu->actions().size() == 8) {
            Q_FOREACH (QAction *action, menu->actions()) {
                action->setShortcut(QKeySequence());

            }
            menu->actions().last()->deleteLater();
        }
    }

    updateCaption();
    d->actionManager()->updateGUI();
}

void KisMainWindow::windowFocused()
{
    /**
     * Notify selection manager so that it could update selection mask overlay
     */
    if (viewManager() && viewManager()->selectionManager()) {
        viewManager()->selectionManager()->selectionChanged();
    }

    KisPart *kisPart = KisPart::instance();
    KisWindowLayoutManager *layoutManager = KisWindowLayoutManager::instance();
    if (!layoutManager->primaryWorkspaceFollowsFocus()) return;

    QUuid primary = layoutManager->primaryWindowId();
    if (primary.isNull()) return;

    if (d->id == primary) {
        if (!d->workspaceBorrowedBy.isNull()) {
            KisMainWindow *borrower = kisPart->windowById(d->workspaceBorrowedBy);
            if (!borrower) return;
            swapWorkspaces(this, borrower);
        }
    } else {
        if (d->workspaceBorrowedBy == primary) return;

        KisMainWindow *primaryWindow = kisPart->windowById(primary);
        if (!primaryWindow) return;
        swapWorkspaces(this, primaryWindow);
    }
}


void KisMainWindow::updateWindowMenu()
{
    QMenu *menu = d->windowMenu->menu();
    menu->clear();

    menu->addAction(d->newWindow);
    menu->addAction(d->documentMenu);

    QMenu *docMenu = d->documentMenu->menu();
    docMenu->clear();

    QFontMetrics fontMetrics = docMenu->fontMetrics();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect geom = this->geometry();
    QPoint p(geom.width() / 2 + geom.left(), geom.height() / 2 + geom.top());
    QScreen *screen = qApp->screenAt(p);

    int fileStringWidth = 300;
    if (screen) {
        fileStringWidth = int(screen->availableGeometry().width() * .40f);
    }
#else
    int fileStringWidth = int(QApplication::desktop()->screenGeometry(this).width() * .40f);
#endif
    Q_FOREACH (QPointer<KisDocument> doc, KisPart::instance()->documents()) {
        if (doc) {
            QString title = fontMetrics.elidedText(doc->url().toDisplayString(QUrl::PreferLocalFile), Qt::ElideMiddle, fileStringWidth);
            if (title.isEmpty() && doc->image()) {
                title = doc->image()->objectName();
            }
            QAction *action = docMenu->addAction(title);
            action->setIcon(qApp->windowIcon());
            connect(action, SIGNAL(triggered()), d->documentMapper, SLOT(map()));
            d->documentMapper->setMapping(action, doc);
        }
    }

    menu->addAction(d->workspaceMenu);
    QMenu *workspaceMenu = d->workspaceMenu->menu();
    workspaceMenu->clear();

    KisResourceIterator resourceIterator(KisResourceModelProvider::resourceModel(ResourceType::Workspaces));
    KisMainWindow *m_this = this;

    while (resourceIterator.hasNext()) {
        KisResourceItemSP resource = resourceIterator.next();
        QAction *action = workspaceMenu->addAction(resource->name());
        connect(action, &QAction::triggered, this, [=]() {
            m_this->restoreWorkspace(resource->id());
        });
    }
    workspaceMenu->addSeparator();
    connect(workspaceMenu->addAction(i18nc("@action:inmenu", "&Import Workspace...")),
            &QAction::triggered,
            this,
            [&]()
    {
        QStringList mimeTypes = KisResourceLoaderRegistry::instance()->mimeTypes(ResourceType::Workspaces);
        KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
        dialog.setMimeTypeFilters(mimeTypes);
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        QString filename = dialog.filename();

        d->workspacemodel->importResourceFile(filename);
    });

    connect(workspaceMenu->addAction(i18nc("@action:inmenu", "&New Workspace...")),
            &QAction::triggered,
            [=]() {
        QString name;
        auto rserver = KisResourceServerProvider::instance()->workspaceServer();

        KisWorkspaceResourceSP workspace(new KisWorkspaceResource(""));
        workspace->setDockerState(m_this->saveState());
        d->viewManager->canvasResourceProvider()->notifySavingWorkspace(workspace);
        workspace->setValid(true);
        QString saveLocation = rserver->saveLocation();

        QFileInfo fileInfo(saveLocation + name + workspace->defaultFileExtension());
        bool fileOverWriteAccepted = false;

        while(!fileOverWriteAccepted) {
            name = QInputDialog::getText(this, i18nc("@title:window", "New Workspace..."),
                                                        i18nc("@label:textbox", "Name:"));
            if (name.isNull() || name.isEmpty()) {
                return;
            } else {
                fileInfo = QFileInfo(saveLocation + name.split(" ").join("_") + workspace->defaultFileExtension());
                if (fileInfo.exists()) {
                    int res = QMessageBox::warning(this, i18nc("@title:window", "Name Already Exists")
                                                                , i18n("The name '%1' already exists, do you wish to overwrite it?", name)
                                                                , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    if (res == QMessageBox::Yes) fileOverWriteAccepted = true;
                } else {
                    fileOverWriteAccepted = true;
                }
            }
        }

        workspace->setFilename(fileInfo.fileName());
        workspace->setName(name);
        rserver->addResource(workspace);
    });

    // TODO: What to do about delete?
    //    workspaceMenu->addAction(i18nc("@action:inmenu", "&Delete Workspace..."));

    menu->addSeparator();
    menu->addAction(d->close);
    menu->addAction(d->closeAll);
    if (d->mdiArea->viewMode() == QMdiArea::SubWindowView) {
        menu->addSeparator();
        menu->addAction(d->mdiTile);
        menu->addAction(d->mdiCascade);
    }
    menu->addSeparator();
    menu->addAction(d->mdiNextWindow);
    menu->addAction(d->mdiPreviousWindow);
    menu->addSeparator();

    QList<QMdiSubWindow *> windows = d->mdiArea->subWindowList();
    for (int i = 0; i < windows.size(); ++i) {
        QPointer<KisView>child = qobject_cast<KisView*>(windows.at(i)->widget());
        if (child && child->document()) {
            QString text;
            if (i < 9) {
                text = i18n("&%1 %2", i + 1, fontMetrics.elidedText(child->document()->url().toDisplayString(QUrl::PreferLocalFile), Qt::ElideMiddle, fileStringWidth));
            }
            else {
                text = i18n("%1 %2", i + 1, fontMetrics.elidedText(child->document()->url().toDisplayString(QUrl::PreferLocalFile), Qt::ElideMiddle, fileStringWidth));
            }

            QAction *action  = menu->addAction(text);
            action->setIcon(qApp->windowIcon());
            action->setCheckable(true);
            action->setChecked(child == activeKisView());
            connect(action, SIGNAL(triggered()), d->windowMapper, SLOT(map()));
            d->windowMapper->setMapping(action, windows.at(i));
        }
    }

    bool showMdiArea = windows.count( ) > 0;
    if (!showMdiArea) {
        showWelcomeScreen(true); // see workaround in function in header
        // keep the recent file list updated when going back to welcome screen
        reloadRecentFileList();
        d->welcomePage->populateRecentDocuments();
    }

    // enable/disable the toolbox docker if there are no documents open
    Q_FOREACH (QObject* widget, children()) {
        if (widget->inherits("QDockWidget")) {
            QDockWidget* dw = static_cast<QDockWidget*>(widget);

            if ( dw->objectName() == "ToolBox") {
                dw->setEnabled(showMdiArea);
            }
        }
    }

    updateCaption();
}

void KisMainWindow::updateSubwindowFlags()
{
    bool onlyOne = false;
    if (d->mdiArea->subWindowList().size() == 1 && d->mdiArea->viewMode() == QMdiArea::SubWindowView) {
        onlyOne = true;
    }
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        if (onlyOne) {
            subwin->setWindowFlags(subwin->windowFlags() | Qt::FramelessWindowHint);
            subwin->showMaximized();
        } else {
            subwin->setWindowFlags((subwin->windowFlags() | Qt::FramelessWindowHint) ^ Qt::FramelessWindowHint);
        }
    }
}

void KisMainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window) return;
    QMdiSubWindow *subwin = qobject_cast<QMdiSubWindow *>(window);
    //dbgKrita << "setActiveSubWindow();" << subwin << d->activeSubWindow;

    if (subwin && subwin != d->activeSubWindow) {
        KisView *view = qobject_cast<KisView *>(subwin->widget());
        //dbgKrita << "\t" << view << activeView();
        if (view && view != activeView()) {
            d->mdiArea->setActiveSubWindow(subwin);
            setActiveView(view);
        }
        d->activeSubWindow = subwin;
    }
    updateWindowMenu();
    d->actionManager()->updateGUI();
}

void KisMainWindow::configChanged()
{
    KisConfig cfg(true);
    QMdiArea::ViewMode viewMode = (QMdiArea::ViewMode)cfg.readEntry<int>("mdi_viewmode", (int)QMdiArea::TabbedView);
    d->mdiArea->setViewMode(viewMode);
    Q_FOREACH (QMdiSubWindow *subwin, d->mdiArea->subWindowList()) {
        subwin->setOption(QMdiSubWindow::RubberBandMove, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));
        subwin->setOption(QMdiSubWindow::RubberBandResize, cfg.readEntry<int>("mdi_rubberband", cfg.useOpenGL()));

        /**
         * Dirty workaround for a bug in Qt (checked on Qt 5.6.1):
         *
         * If you make a window "Show on top" and then switch to the tabbed mode
         * the window will continue to be painted in its initial "mid-screen"
         * position. It will persist here until you explicitly switch to its tab.
         */
        if (viewMode == QMdiArea::TabbedView) {
            Qt::WindowFlags oldFlags = subwin->windowFlags();
            Qt::WindowFlags flags = oldFlags;

            flags &= ~Qt::WindowStaysOnTopHint;
            flags &= ~Qt::WindowStaysOnBottomHint;

            if (flags != oldFlags) {
                subwin->setWindowFlags(flags);
                subwin->showMaximized();
            }
        }
    }
#ifdef Q_OS_MACOS
    updateSubwindowFlags();
#endif

    KConfigGroup group( KSharedConfig::openConfig(), "theme");
    d->themeManager->setCurrentTheme(group.readEntry("Theme", "Krita dark"));
    d->actionManager()->updateGUI();

    QString s = cfg.getMDIBackgroundColor();
    KoColor c = KoColor::fromXML(s);
    QBrush brush(c.toQColor());
    d->mdiArea->setBackground(brush);

    QString backgroundImage = cfg.getMDIBackgroundImage();
    if (backgroundImage != "") {
        QImage image(backgroundImage);
        QBrush brush(image);
        d->mdiArea->setBackground(brush);
    }

    d->mdiArea->update();
}

KisView* KisMainWindow::newView(QObject *document, QMdiSubWindow *subWindow)
{
    KisDocument *doc = qobject_cast<KisDocument*>(document);
    KisView *view = addViewAndNotifyLoadingCompleted(doc, subWindow);
    d->actionManager()->updateGUI();

    return view;
}

void KisMainWindow::newWindow()
{
    KisMainWindow *mainWindow = KisPart::instance()->createMainWindow();
    mainWindow->initializeGeometry();
    mainWindow->show();
}

void KisMainWindow::closeCurrentWindow()
{
    if (d->mdiArea->currentSubWindow()) {
        d->mdiArea->currentSubWindow()->close();
        d->actionManager()->updateGUI();
    }
}

void KisMainWindow::checkSanity()
{
    // print error if the lcms engine is not available
    if (!KoColorSpaceEngineRegistry::instance()->contains("icc")) {
        // need to wait 1 event since exiting here would not work.
        m_errorMessage = i18n("The Krita LittleCMS color management plugin is not installed. Krita will quit now.");
        m_dieOnError = true;
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
        return;
    }

    KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    if (rserver->resourceCount() == 0) {
        m_errorMessage = i18n("Krita cannot find any brush presets! Krita will quit now.");
        m_dieOnError = true;
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
        return;
    }
}

void KisMainWindow::showErrorAndDie()
{
    QMessageBox::critical(0, i18nc("@title:window", "Installation error"), m_errorMessage);
    if (m_dieOnError) {
        exit(10);
    }
}

void KisMainWindow::showAboutApplication()
{
    KisAboutApplication dlg(this);
    dlg.exec();
}

QPointer<KisView> KisMainWindow::activeKisView()
{
    if (!d->mdiArea) return 0;
    QMdiSubWindow *activeSubWindow = d->mdiArea->activeSubWindow();
    //dbgKrita << "activeKisView" << activeSubWindow;
    if (!activeSubWindow) return 0;
    return qobject_cast<KisView*>(activeSubWindow->widget());
}

void KisMainWindow::newOptionWidgets(KoCanvasController *controller, const QList<QPointer<QWidget> > &optionWidgetList)
{
    KIS_ASSERT_RECOVER_NOOP(controller == KoToolManager::instance()->activeCanvasController());
    bool isOurOwnView = false;

    Q_FOREACH (QPointer<KisView> view, KisPart::instance()->views()) {
        if (view && view->canvasController() == controller) {
            isOurOwnView = view->mainWindow() == this;
        }
    }

    if (!isOurOwnView) return;

    Q_FOREACH (QWidget *w, optionWidgetList) {
#ifdef Q_OS_MACOS
        w->setAttribute(Qt::WA_MacSmallSize, true);
#endif
        w->setFont(KoDockRegistry::dockFont());
    }

    if (d->toolOptionsDocker) {
        d->toolOptionsDocker->setOptionWidgets(optionWidgetList);
    }
    else {
        d->viewManager->paintOpBox()->newOptionWidgets(optionWidgetList);
    }
}

void KisMainWindow::createActions()
{
    KisActionManager *actionManager = d->actionManager();



    actionManager->createStandardAction(KStandardAction::New, this, SLOT(slotFileNew()));
    actionManager->createStandardAction(KStandardAction::Open, this, SLOT(slotFileOpen()));
    actionManager->createStandardAction(KStandardAction::Quit, this, SLOT(slotFileQuit()));
    actionManager->createStandardAction(KStandardAction::ConfigureToolbars, this, SLOT(slotConfigureToolbars()));
    d->fullScreenMode = actionManager->createStandardAction(KStandardAction::FullScreen, this, SLOT(viewFullscreen(bool)));

    d->recentFiles = KStandardAction::openRecent(this, SLOT(slotFileOpenRecent(QUrl)), actionCollection());
    connect(d->recentFiles, SIGNAL(recentListCleared()), this, SLOT(saveRecentFiles()));
    KSharedConfigPtr configPtr =  KSharedConfig::openConfig();
    d->recentFiles->loadEntries(configPtr->group("RecentFiles"));

    d->saveAction = actionManager->createStandardAction(KStandardAction::Save, this, SLOT(slotFileSave()));
    d->saveAction->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->saveActionAs = actionManager->createStandardAction(KStandardAction::SaveAs, this, SLOT(slotFileSaveAs()));
    d->saveActionAs->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undo = actionManager->createStandardAction(KStandardAction::Undo, this, SLOT(undo()));
    d->undo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->redo = actionManager->createStandardAction(KStandardAction::Redo, this, SLOT(redo()));
    d->redo->setActivationFlags(KisAction::ACTIVE_IMAGE);

    d->undoActionsUpdateManager.reset(new KisUndoActionsUpdateManager(d->undo, d->redo));
    d->undoActionsUpdateManager->setCurrentDocument(d->activeView ? d->activeView->document() : 0);

    d->importAnimation  = actionManager->createAction("file_import_animation");
    connect(d->importAnimation, SIGNAL(triggered()), this, SLOT(importAnimation()));

    d->closeAll = actionManager->createAction("file_close_all");
    connect(d->closeAll, SIGNAL(triggered()), this, SLOT(slotFileCloseAll()));

    d->importFile  = actionManager->createAction("file_import_file");
    connect(d->importFile, SIGNAL(triggered(bool)), this, SLOT(slotImportFile()));

    d->exportFile  = actionManager->createAction("file_export_file");
    connect(d->exportFile, SIGNAL(triggered(bool)), this, SLOT(slotExportFile()));

    /* The following entry opens the document information dialog.  Since the action is named so it
        intends to show data this entry should not have a trailing ellipses (...).  */
    d->showDocumentInfo  = actionManager->createAction("file_documentinfo");
    connect(d->showDocumentInfo, SIGNAL(triggered(bool)), this, SLOT(slotDocumentInfo()));


    d->themeManager->setThemeMenuAction(new KActionMenu(i18nc("@action:inmenu", "&Themes"), this));
    d->themeManager->registerThemeActions(actionCollection());
    connect(d->themeManager, SIGNAL(signalThemeChanged()), this, SLOT(slotThemeChanged()));


    connect(d->themeManager, SIGNAL(signalThemeChanged()), d->welcomePage, SLOT(slotUpdateThemeColors()));

    d->toggleDockers = actionManager->createAction("view_toggledockers");
    KisConfig(true).showDockers(true);
    d->toggleDockers->setChecked(true);
    connect(d->toggleDockers, SIGNAL(toggled(bool)), SLOT(toggleDockersVisibility(bool)));

    d->resetConfigurations  = actionManager->createAction("reset_configurations");
    connect(d->resetConfigurations, SIGNAL(triggered()), this, SLOT(slotResetConfigurations()));

    d->toggleDetachCanvas = actionManager->createAction("view_detached_canvas");
    d->toggleDetachCanvas->setChecked(false);
    connect(d->toggleDetachCanvas, SIGNAL(toggled(bool)), SLOT(setCanvasDetached(bool)));
    setCanvasDetached(false);

    actionCollection()->addAction("settings_dockers_menu", d->dockWidgetMenu);
    actionCollection()->addAction("window", d->windowMenu);

    actionCollection()->addAction("style_menu", d->styleMenu); // for widget styles: breeze, fusion, etc

    d->mdiCascade = actionManager->createAction("windows_cascade");
    connect(d->mdiCascade, SIGNAL(triggered()), d->mdiArea, SLOT(cascadeSubWindows()));

    d->mdiTile = actionManager->createAction("windows_tile");
    connect(d->mdiTile, SIGNAL(triggered()), d->mdiArea, SLOT(tileSubWindows()));

    d->mdiNextWindow = actionManager->createAction("windows_next");
    connect(d->mdiNextWindow, SIGNAL(triggered()), d->mdiArea, SLOT(activateNextSubWindow()));

    d->mdiPreviousWindow = actionManager->createAction("windows_previous");
    connect(d->mdiPreviousWindow, SIGNAL(triggered()), d->mdiArea, SLOT(activatePreviousSubWindow()));

    d->newWindow = actionManager->createAction("view_newwindow");
    connect(d->newWindow, SIGNAL(triggered(bool)), this, SLOT(newWindow()));

    d->close = actionManager->createStandardAction(KStandardAction::Close, this, SLOT(closeCurrentWindow()));

    d->showSessionManager = actionManager->createAction("file_sessions");
    connect(d->showSessionManager, SIGNAL(triggered(bool)), this, SLOT(slotShowSessionManager()));

    actionManager->createStandardAction(KStandardAction::Preferences, this, SLOT(slotPreferences()));

    for (int i = 0; i < 2; i++) {
        d->expandingSpacers[i] = new KisAction(i18n("Expanding Spacer"));
        d->expandingSpacers[i]->setDefaultWidget(new QWidget(this));
        d->expandingSpacers[i]->defaultWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        actionManager->addAction(QString("expanding_spacer_%1").arg(i), d->expandingSpacers[i]);
    }
}

void KisMainWindow::applyToolBarLayout()
{
    const bool isPlastiqueStyle = style()->objectName() == "plastique";

    Q_FOREACH (KToolBar *toolBar, toolBars()) {
        toolBar->layout()->setSpacing(4);
        if (isPlastiqueStyle) {
            toolBar->setContentsMargins(0, 0, 0, 2);
        }
        //Hide text for buttons with an icon in the toolbar
        Q_FOREACH (QAction *ac, toolBar->actions()){
            if (ac->icon().pixmap(QSize(1,1)).isNull() == false){
                ac->setPriority(QAction::LowPriority);
            }else {
                ac->setIcon(QIcon());
            }
        }
    }
}

void KisMainWindow::initializeGeometry()
{
    // if the user didn's specify the geometry on the command line (does anyone do that still?),
    // we first figure out some good default size and restore the x,y position. See bug 285804Z.
    KConfigGroup cfg = d->windowStateConfig;
    QByteArray geom = QByteArray::fromBase64(cfg.readEntry("ko_geometry", QByteArray()));
    if (!restoreGeometry(geom)) {
        const int scnum = QApplication::desktop()->screenNumber(parentWidget());
        QRect desk = QGuiApplication::screens().at(scnum)->availableVirtualGeometry();

        quint32 x = desk.x();
        quint32 y = desk.y();
        quint32 w = 0;
        quint32 h = 0;

        // Default size -- maximize on small screens, something useful on big screens
        const int deskWidth = desk.width();
        if (deskWidth > 1024) {
            // a nice width, and slightly less than total available
            // height to compensate for the window decs
            w = (deskWidth / 3) * 2;
            h = (desk.height() / 3) * 2;
        }
        else {
            w = desk.width();
            h = desk.height();
        }

        x += (desk.width() - w) / 2;
        y += (desk.height() - h) / 2;

        move(x,y);
        setGeometry(geometry().x(), geometry().y(), w, h);
    }
    d->fullScreenMode->setChecked(isFullScreen());
}

void KisMainWindow::showManual()
{
    QDesktopServices::openUrl(QUrl("https://docs.krita.org"));
}

void KisMainWindow::windowScreenChanged(QScreen *screen)
{
    emit screenChanged();
    d->screenConnectionsStore.clear();
    d->screenConnectionsStore.addConnection(screen, SIGNAL(physicalDotsPerInchChanged(qreal)),
                                            this, SIGNAL(screenChanged()));
}

void KisMainWindow::slotXmlGuiMakingChanges(bool finished)
{
    if (finished) {
        subWindowActivated();
    }
}



#include <moc_KisMainWindow.cpp>
