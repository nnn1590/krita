/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "widgets/kis_gradient_chooser.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>

#include <klocalizedstring.h>
#include <resources/KoAbstractGradient.h>
#include <KoResource.h>
#include <resources/KoSegmentGradient.h>
#include <KisResourceItemListView.h>
#include <KisKineticScroller.h>
#include <KoStopGradient.h>
#include <KoColorSpaceRegistry.h>
#include <KisResourceItemChooser.h>
#include <KoResourceServerProvider.h>
#include <kis_icon.h>
#include <kis_config.h>

#include "KisViewManager.h"
#include "kis_global.h"
#include "kis_autogradient.h"
#include "kis_canvas_resource_provider.h"
#include "kis_stopgradient_editor.h"

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>


KisCustomGradientDialog::KisCustomGradientDialog(KoAbstractGradientSP gradient, QWidget *parent, const char *name,
    KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KoDialog(parent, Qt::Dialog)
{
    setButtons(Ok|Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    setModal(false);

    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(reject()));

    KoStopGradientSP stopGradient = gradient.dynamicCast<KoStopGradient>();
    if (stopGradient) {
        m_page = new KisStopGradientEditor(stopGradient, this, "autogradient", i18n("Custom Stop Gradient"), canvasResourcesInterface);
    }
    else {
        KoSegmentGradientSP segmentedGradient = gradient.dynamicCast<KoSegmentGradient>();
        if (segmentedGradient) {
            m_page = new KisAutogradientEditor(segmentedGradient, this, "autogradient", i18n("Custom Segmented Gradient"), canvasResourcesInterface);
        }
    }
    setCaption(m_page->windowTitle());
    setMainWidget(m_page);
}

KisGradientChooser::KisGradientChooser(QWidget *parent, const char *name)
    : QFrame(parent)
{
    setObjectName(name);
    m_lbName = new QLabel();

    m_itemChooser = new KisResourceItemChooser(ResourceType::Gradients, false, this);

    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->itemView()->setViewMode(QListView::ListMode);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(update(KoResourceSP )));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SIGNAL(resourceSelected(KoResourceSP )));

    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);

    m_addGradient = new QToolButton(this);

    m_addGradient->setText(i18n("Add..."));
    m_addGradient->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(m_addGradient, SIGNAL(clicked()), this, SLOT(addStopGradient()));
    buttonLayout->addWidget(m_addGradient);

    QMenu *menuAddGradient = new QMenu(m_addGradient);

    QAction* addStopGradient = new QAction(i18n("Stop gradient"), this);
    connect(addStopGradient, SIGNAL(triggered(bool)), this, SLOT(addStopGradient()));
    menuAddGradient->addAction(addStopGradient);

    QAction* addSegmentedGradient = new QAction(i18n("Segmented gradient"), this);
    connect(addSegmentedGradient, SIGNAL(triggered(bool)), this, SLOT(addSegmentedGradient()));
    menuAddGradient->addAction(addSegmentedGradient);

    m_addGradient->setMenu(menuAddGradient);
    m_addGradient->setPopupMode(QToolButton::MenuButtonPopup);

    m_editGradient = new QPushButton();
    m_editGradient->setText(i18n("Edit..."));
    m_editGradient->setEnabled(false);
    connect(m_editGradient, SIGNAL(clicked()), this, SLOT(editGradient()));
    buttonLayout->addWidget(m_editGradient);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(m_itemChooser, 10);
    mainLayout->addWidget(buttonWidget);

    slotUpdateIcons();
}

KisGradientChooser::~KisGradientChooser()
{
}

void KisGradientChooser::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisGradientChooser::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}

KoResourceSP KisGradientChooser::currentResource()
{
    return m_itemChooser->currentResource();
}

void KisGradientChooser::setCurrentResource(KoResourceSP resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisGradientChooser::setCurrentItem(int row)
{
    m_itemChooser->setCurrentItem(row);
    if (currentResource())
        update(currentResource());
}

void KisGradientChooser::slotUpdateIcons()
{
    if (m_addGradient && m_editGradient) {
        m_addGradient->setIcon(KisIconUtils::loadIcon("list-add"));
        m_editGradient->setIcon(KisIconUtils::loadIcon("configure"));
    }
}

void KisGradientChooser::update(KoResourceSP resource)
{
    KoAbstractGradientSP gradient = resource.staticCast<KoAbstractGradient>();
    m_lbName->setText(gradient ? i18n(gradient->name().toUtf8().data()) : "");
    m_editGradient->setEnabled(true);
}

void KisGradientChooser::addStopGradient()
{
    KoStopGradientSP gradient(new KoStopGradient(""));

    QList<KoGradientStop> stops;
    stops << KoGradientStop(0.0, KoColor(QColor(250, 250, 0), KoColorSpaceRegistry::instance()->rgb8()), COLORSTOP)
        << KoGradientStop(1.0, KoColor(QColor(255, 0, 0, 255), KoColorSpaceRegistry::instance()->rgb8()), COLORSTOP);
    gradient->setType(QGradient::LinearGradient);
    gradient->setName(i18n("unnamed"));
    gradient->setStops(stops);
    addGradient(gradient);
}

void KisGradientChooser::addSegmentedGradient()
{
    KoSegmentGradientSP gradient(new KoSegmentGradient(""));
    gradient->createSegment(INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white);
    gradient->setName(i18n("unnamed"));
    addGradient(gradient);
}

void KisGradientChooser::addGradient(KoAbstractGradientSP gradient, bool editGradient)
{
    KoResourceServer<KoAbstractGradient> * rserver = KoResourceServerProvider::instance()->gradientServer();
    QString saveLocation = rserver->saveLocation();

    gradient->updateVariableColors(m_canvasResourcesInterface);

    KisCustomGradientDialog dialog(gradient, this, "KisCustomGradientDialog", m_canvasResourcesInterface);

    bool fileOverwriteAccepted = false;

    QString oldname = gradient->name();

    bool shouldSaveResource = true;

    while(!fileOverwriteAccepted) {
        if (dialog.exec() == KoDialog::Accepted) {

            if (gradient->name().isEmpty()) {
                shouldSaveResource = false;
                break;
            }

            if (editGradient && oldname == gradient->name()) {
                fileOverwriteAccepted = true;
                continue;
            }

            const QFileInfo fileInfo(saveLocation + gradient->name().split(" ").join("_") + gradient->defaultFileExtension());
            if (fileInfo.exists()) {
                int res = QMessageBox::warning(this, i18nc("@title:window", "Name Already Exists")
                                               , i18n("The name '%1' already exists, do you wish to overwrite it?", gradient->name())
                                               , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (res == QMessageBox::Yes) fileOverwriteAccepted = true;
            } else {
                fileOverwriteAccepted = true;
            }
        } else {
            shouldSaveResource = false;
            break;
        }
    }

    if (shouldSaveResource) {
        gradient->setFilename(gradient->name() + gradient->defaultFileExtension());
        gradient->setValid(true);
        rserver->addResource(gradient);
        // TODO: select the right gradient from the resource server. Right now this is not possible :(
        m_itemChooser->setCurrentItem(0);
    } else {
        // TODO: revert the changes made to the resource
    }
}

void KisGradientChooser::editGradient()
{
    addGradient(currentResource().staticCast<KoAbstractGradient>(), true);
}




