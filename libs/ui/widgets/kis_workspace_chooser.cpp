/* This file is part of the KDE project
 * Copyright (C) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_workspace_chooser.h"

#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QPushButton>
#include <QAction>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QListView>
#include <KisKineticScroller.h>

#include <klocalizedstring.h>

#include <KisResourceItemChooser.h>
#include <KisResourceItemListView.h>
#include <KoResource.h>
#include <KisResourceModel.h>
#include <KisResourceServerProvider.h>
#include <KoDockWidgetTitleBar.h>
#include <KisMainWindow.h>

#include "kis_workspace_resource.h"
#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "KisMainWindow.h"
#include "KisPart.h"
#include "KisWindowLayoutManager.h"
#include "dialogs/KisNewWindowLayoutDialog.h"
#include "kis_config.h"
#include <kis_icon.h>

class KisWorkspaceDelegate : public QAbstractItemDelegate
{
public:
    KisWorkspaceDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    ~KisWorkspaceDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }
};

void KisWorkspaceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (!index.isValid())
        return;

    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Active : QPalette::Disabled;
    QPalette::ColorRole cr = (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text;
    painter->setPen(option.palette.color(cg, cr));

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else {
        painter->fillRect(option.rect, option.palette.base());
    }

    QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString();
    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, name);
}

KisWorkspaceChooser::KisWorkspaceChooser(KisViewManager * view, QWidget* parent): QWidget(parent), m_view(view)
{
    m_layout = new QGridLayout(this);

    m_workspaceWidgets = createChooserWidgets(ResourceType::Workspaces, i18n("Workspaces"));
    m_windowLayoutWidgets = createChooserWidgets(ResourceType::WindowLayouts, i18n("Window layouts"));

    connect(m_workspaceWidgets.itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(workspaceSelected(KoResourceSP )));
    connect(m_workspaceWidgets.saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveWorkspace()));
    connect(m_workspaceWidgets.nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotUpdateWorkspaceSaveButton()));

    connect(m_windowLayoutWidgets.itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(windowLayoutSelected(KoResourceSP )));
    connect(m_windowLayoutWidgets.saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveWindowLayout()));
    connect(m_windowLayoutWidgets.nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotUpdateWindowLayoutSaveButton()));
}

KisWorkspaceChooser::ChooserWidgets KisWorkspaceChooser::createChooserWidgets(const QString &resourceType, const QString &title)
{
    ChooserWidgets widgets;

    QLabel *titleLabel = new QLabel(this);
    QFont titleFont;
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setText(title);

    m_workspaceSaveLocation = KisResourceServerProvider::instance()->workspaceServer()->saveLocation();
    m_windowLayoutSaveLocation = KisResourceServerProvider::instance()->windowLayoutServer()->saveLocation();

    widgets.itemChooser = new KisResourceItemChooser(resourceType, false, this);
    widgets.itemChooser->setItemDelegate(new KisWorkspaceDelegate(this));
    widgets.itemChooser->setFixedSize(250, 250);
    widgets.itemChooser->setRowHeight(30);
    widgets.itemChooser->itemView()->setViewMode(QListView::ListMode);
    widgets.itemChooser->showTaggingBar(false);
    widgets.saveButton = new QPushButton(i18n("Save"));

    widgets.nameEdit = new QLineEdit(this);
    widgets.nameEdit->setPlaceholderText(i18n("Insert name"));
    widgets.nameEdit->setClearButtonEnabled(true);

    int firstRow = m_layout->rowCount();
    m_layout->addWidget(titleLabel, firstRow, 0, 1, 2);
    m_layout->addWidget(widgets.itemChooser, firstRow + 1, 0, 1, 2);
    m_layout->addWidget(widgets.nameEdit, firstRow + 2, 0, 1, 1);
    m_layout->addWidget(widgets.saveButton, firstRow + 2, 1, 1, 1);

    return widgets;
}

KisWorkspaceChooser::~KisWorkspaceChooser()
{

}

void KisWorkspaceChooser::slotSaveWorkspace()
{
    if (!m_view->qtMainWindow()) {
        return;
    }

    KisWorkspaceResourceSP workspace(new KisWorkspaceResource(QString()));
    workspace->setDockerState(m_view->qtMainWindow()->saveState());
    workspace->setImage(m_view->mainWindow()->layoutThumbnail());
    m_view->canvasResourceProvider()->notifySavingWorkspace(workspace);
    workspace->setValid(true);

    QString name = m_workspaceWidgets.nameEdit->text();

    workspace->setName(name);
    workspace->setFilename(name.split(" ").join("_")+workspace->defaultFileExtension());

    KisResourceModel(ResourceType::Workspaces).addResource(workspace);
}

void KisWorkspaceChooser::slotUpdateWorkspaceSaveButton()
{
    if (QFileInfo(m_workspaceSaveLocation + m_workspaceWidgets.nameEdit->text().split(" ").join("_") + ".kws").exists()) {
        m_workspaceWidgets.saveButton->setIcon(KisIconUtils::loadIcon("warning"));
        m_workspaceWidgets.saveButton->setToolTip(i18n("File name already in use. Saving will overwrite the original Workspace."));
        //m_workspaceWidgets.saveButton->setText(i18n("Overwrite"));
    } else {
        m_workspaceWidgets.saveButton->setIcon(QIcon());
        m_workspaceWidgets.saveButton->setToolTip(i18n("Save current workspace."));
        //m_workspaceWidgets.saveButton->setText(i18n("Save"));
    }
}

void KisWorkspaceChooser::workspaceSelected(KoResourceSP resource)
{
    if (!m_view->qtMainWindow()) {
        return;
    }
    KisConfig cfg(false);
    cfg.writeEntry("CurrentWorkspace", resource->name());
    KisWorkspaceResourceSP workspace = resource.staticCast<KisWorkspaceResource>();
    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(m_view->qtMainWindow());
    mainWindow->restoreWorkspace(workspace->resourceId());
}

void KisWorkspaceChooser::slotSaveWindowLayout()
{
    KisMainWindow *thisWindow = qobject_cast<KisMainWindow*>(m_view->qtMainWindow());
    if (!thisWindow) return;

    KisNewWindowLayoutDialog dlg;
    dlg.setName(m_windowLayoutWidgets.nameEdit->text());
    dlg.exec();

    if (dlg.result() != QDialog::Accepted) return;

    QString name = dlg.name();
    bool showImageInAllWindows = dlg.showImageInAllWindows();
    bool primaryWorkspaceFollowsFocus = dlg.primaryWorkspaceFollowsFocus();

    KisWindowLayoutResourceSP layout = KisWindowLayoutResource::fromCurrentWindows(name, KisPart::instance()->mainWindows(), showImageInAllWindows, primaryWorkspaceFollowsFocus, thisWindow);
    layout->setValid(true);

    KisWindowLayoutManager::instance()->setShowImageInAllWindowsEnabled(showImageInAllWindows);
    KisWindowLayoutManager::instance()->setPrimaryWorkspaceFollowsFocus(primaryWorkspaceFollowsFocus, thisWindow->id());

    if (name.isEmpty()) {
        name = i18n("Window Layout");
    }

    layout->setName(name);
    layout->setFilename(name.split(" ").join("_") + layout->defaultFileExtension());
    KisResourceModel(ResourceType::WindowLayouts).addResource(layout);
}

void KisWorkspaceChooser::slotUpdateWindowLayoutSaveButton()
{
    if (QFileInfo(m_windowLayoutSaveLocation + m_windowLayoutWidgets.nameEdit->text().split(" ").join("_") + ".kwl").exists()) {
        m_windowLayoutWidgets.saveButton->setIcon(KisIconUtils::loadIcon("warning"));
        m_workspaceWidgets.saveButton->setToolTip(i18n("File name already in use. Saving will overwrite the original window layout."));
    } else {
        m_windowLayoutWidgets.saveButton->setIcon(QIcon());
        m_workspaceWidgets.saveButton->setToolTip(i18n("Save current window layout."));
    }
}

void KisWorkspaceChooser::windowLayoutSelected(KoResourceSP resource)
{
    KisWindowLayoutResourceSP layout = resource.dynamicCast<KisWindowLayoutResource>();
    layout->applyLayout();
}
