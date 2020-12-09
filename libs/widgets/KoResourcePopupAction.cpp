/*
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * Copyright (C) 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoResourcePopupAction.h"

#include <KisResourceItemListView.h>
#include <KisResourceModel.h>
#include <KisResourceItemDelegate.h>
#include <KoResource.h>

#include <KoCheckerBoardPainter.h>
#include <KoShapeBackground.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoPattern.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>

#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QGradient>
#include <QToolButton>
#include <QRect>
#include <QWidgetAction>

class KoResourcePopupAction::Private
{
public:
    QMenu *menu = 0;
    KisResourceModel *model = 0;
    KisResourceItemListView *resourceList = 0;
    QSharedPointer<KoShapeBackground> background;
    KoImageCollection *imageCollection = 0;
    KoCheckerBoardPainter checkerPainter {4};
    KoCanvasResourcesInterfaceSP canvasResourcesInterface;
};

KoResourcePopupAction::KoResourcePopupAction(const QString &resourceType, KoCanvasResourcesInterfaceSP canvasResourcesInterface, QObject *parent)
    : QAction(parent)
    , d(new Private())
{
    d->canvasResourcesInterface = canvasResourcesInterface;

    d->menu = new QMenu();
    QWidget *widget = new QWidget();
    QWidgetAction *wdgAction = new QWidgetAction(this);

    d->resourceList = new KisResourceItemListView(widget);

    d->model = new KisResourceModel(resourceType, this);
    d->resourceList->setModel(d->model);
    d->resourceList->setItemDelegate(new KisResourceItemDelegate(widget));
    d->resourceList->setCurrentIndex(d->model->index(0, 0));
    if (resourceType==ResourceType::Gradients) {
        d->resourceList->setViewMode(QListView::ListMode);
    }
    indexChanged(d->resourceList->currentIndex());
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(d->resourceList);

    wdgAction->setDefaultWidget(widget);
    d->menu->addAction(wdgAction);
    setMenu(d->menu);
    new QHBoxLayout(d->menu);
    d->menu->layout()->addWidget(widget);
    d->menu->layout()->setMargin(0);

    connect(d->resourceList, SIGNAL(clicked(QModelIndex)), this, SLOT(indexChanged(QModelIndex)));

    updateIcon();
}

KoResourcePopupAction::~KoResourcePopupAction()
{
    /* Removing the actions here make them be deleted together with their default widget.
     * This happens only if the actions are QWidgetAction, and we know they are since
     * the only ones added are in KoResourcePopupAction constructor. */
    int i = 0;
    while(d->menu->actions().size() > 0) {
        d->menu->removeAction(d->menu->actions()[i]);
        ++i;
    }

    delete d->menu;
    delete d->imageCollection;
    delete d;
}

QSharedPointer<KoShapeBackground> KoResourcePopupAction::currentBackground() const
{
    return d->background;
}

void KoResourcePopupAction::setCurrentBackground(QSharedPointer<KoShapeBackground>  background)
{
    d->background = background;

    updateIcon();
}

void KoResourcePopupAction::setCurrentResource(KoResourceSP resource)
{
    QModelIndex index = d->model->indexForResource(resource);
    if (index.isValid()) {
        d->resourceList->setCurrentIndex(index);
        indexChanged(index);
    }
}

KoResourceSP KoResourcePopupAction::currentResource() const
{
    QModelIndex index = d->resourceList->currentIndex();
    if (!index.isValid()) return 0;

    KoResourceSP resource = d->model->resourceForIndex(index);
    return resource;
}

void KoResourcePopupAction::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    d->canvasResourcesInterface = canvasResourcesInterface;
}

void KoResourcePopupAction::indexChanged(const QModelIndex &modelIndex)
{
    if (! modelIndex.isValid()) {
        return;
    }

    d->menu->hide();

    KoResourceSP resource = d->model->resourceForIndex(modelIndex);

    if (resource) {
        KoAbstractGradientSP gradient = resource.dynamicCast<KoAbstractGradient>();
        KoPatternSP pattern = resource.dynamicCast<KoPattern>();
        if (gradient) {
            QGradient *qg = gradient->cloneAndBakeVariableColors(d->canvasResourcesInterface)->toQGradient();
            qg->setCoordinateMode(QGradient::ObjectBoundingMode);
            d->background = QSharedPointer<KoShapeBackground>(new KoGradientBackground(qg));
        } else if (pattern) {
            KoImageCollection *collection = new KoImageCollection();
            d->background = QSharedPointer<KoShapeBackground>(new KoPatternBackground(collection));
            qSharedPointerDynamicCast<KoPatternBackground>(d->background)->setPattern(pattern->pattern());
        }

        emit resourceSelected(d->background);

        updateIcon();
    }
}

void KoResourcePopupAction::updateIcon()
{
    QSize iconSize;
    QToolButton *toolButton = dynamic_cast<QToolButton*>(parentWidget());
    if (toolButton) {
        iconSize = QSize(toolButton->iconSize());
    } else {
        iconSize = QSize(16, 16);
    }

    // This must be a QImage, as drawing to a QPixmap outside the
    // UI thread will cause sporadic crashes.
    QImage pm = QImage(iconSize, QImage::Format_ARGB32_Premultiplied);

    pm.fill(Qt::transparent);

    QPainter p(&pm);
    QSharedPointer<KoGradientBackground> gradientBackground = qSharedPointerDynamicCast<KoGradientBackground>(d->background);
    QSharedPointer<KoPatternBackground> patternBackground = qSharedPointerDynamicCast<KoPatternBackground>(d->background);

    if (gradientBackground) {
        QRect innerRect(0, 0, iconSize.width(), iconSize.height());
        QLinearGradient paintGradient;
        paintGradient.setStops(gradientBackground->gradient()->stops());
        paintGradient.setStart(innerRect.topLeft());
        paintGradient.setFinalStop(innerRect.topRight());

        d->checkerPainter.paint(p, innerRect);
        p.fillRect(innerRect, QBrush(paintGradient));
    }
    else if (patternBackground) {
        d->checkerPainter.paint(p, QRect(QPoint(),iconSize));
        p.fillRect(0, 0, iconSize.width(), iconSize.height(), patternBackground->pattern());
    }

    p.end();

    setIcon(QIcon(QPixmap::fromImage(pm)));
}
