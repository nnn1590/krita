/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QTreeView>
#include <QSplitter>
#include <QToolBar>
#include <QScroller>

#include "kis_animation_curve_docker.h"
#include "kis_animation_curves_model.h"
#include "kis_animation_curves_view.h"
#include "kis_animation_curve_channel_list_model.h"
#include "kis_animation_curve_channel_list_delegate.h"

#include "kis_keyframe_channel.h"

#include "kis_image_animation_interface.h"
#include "kis_animation_utils.h"

#include "KisDocument.h"
#include "kis_canvas2.h"
#include "kis_shape_controller.h"
#include "kis_signal_auto_connection.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_animation_frame_cache.h"
#include "klocalizedstring.h"
#include "kis_icon_utils.h"
#include "kis_action_manager.h"
#include "kis_action.h"

#include "ui_wdg_animation_curves.h"

struct KisAnimationCurveDocker::Private
{
    Private(QWidget *parent)
        : curvesWidget()
        , curvesModel(new KisAnimationCurvesModel(parent))
    {
        channelListModel = new KisAnimationCurveChannelListModel(curvesModel, parent);
    }

    Ui_WdgAnimationCurves curvesWidget;

    KisAnimationCurvesModel *curvesModel;
    KisAnimationCurveChannelListModel *channelListModel;

    QPointer<KisCanvas2> canvas;
    KisSignalAutoConnectionsStore canvasConnections;
};

KisAnimationCurveDocker::KisAnimationCurveDocker()
    : QDockWidget(i18n("Animation Curves"))
    , m_d(new Private(this))
{
    QWidget *mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_d->curvesWidget.setupUi(mainWidget);

    KisAnimationCurvesView *curvesView = m_d->curvesWidget.curvesView;
    QTreeView *channelListView = m_d->curvesWidget.channelListView;
    KisAnimationCurveChannelListDelegate *listDelegate = new KisAnimationCurveChannelListDelegate(channelListView);

    curvesView->setModel(m_d->curvesModel);
    curvesView->setZoomButtons(m_d->curvesWidget.btnHorizontalZoom, m_d->curvesWidget.btnVerticalZoom);
    channelListView->setModel(m_d->channelListModel);
    channelListView->setItemDelegate(listDelegate);
    channelListView->setHeaderHidden(true);

    m_d->curvesWidget.splitter->setStretchFactor(0, 1);
    m_d->curvesWidget.splitter->setStretchFactor(1, 4);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(channelListView);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    connect(m_d->channelListModel, &KisAnimationCurveChannelListModel::rowsInserted,
            this, &KisAnimationCurveDocker::slotListRowsInserted);

    connect(m_d->curvesWidget.btnConstantInterpolation, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::applyConstantMode);
    connect(m_d->curvesWidget.btnLinearInterpolation, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::applyLinearMode);
    connect(m_d->curvesWidget.btnBezierInterpolation, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::applyBezierMode);
    connect(m_d->curvesWidget.btnSmooth, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::applySmoothMode);
    connect(m_d->curvesWidget.btnSharp, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::applySharpMode);
    connect(m_d->curvesWidget.btnAddKeyframe, &QToolButton::clicked,
            this, &KisAnimationCurveDocker::slotAddAllEnabledKeys);
    connect(m_d->curvesWidget.btnRemoveKeyframes, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::removeKeyframes);
    connect(m_d->curvesWidget.btnZoomToFit, &QToolButton::clicked,
            curvesView, &KisAnimationCurvesView::zoomToFit);
}

KisAnimationCurveDocker::~KisAnimationCurveDocker()
{}

void KisAnimationCurveDocker::setCanvas(KoCanvasBase *canvas)
{
    if (canvas && m_d->canvas == canvas) return;

    if (m_d->canvas) {
        m_d->canvasConnections.clear();
        m_d->canvas->disconnectCanvasObserver(this);
        m_d->channelListModel->selectedNodesChanged(KisNodeList());
    }

    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if (m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->channelListModel->setDummiesFacade(kritaShapeController);

        m_d->curvesModel->setImage(m_d->canvas->image());
        m_d->curvesModel->setFrameCache(m_d->canvas->frameCache());
        m_d->curvesModel->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigUiNeedChangeSelectedNodes(KisNodeList)),
            m_d->channelListModel, SLOT(selectedNodesChanged(KisNodeList))
        );

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            this, SLOT(slotNodeActivated(KisNodeSP))
        );

        m_d->channelListModel->clear();
        m_d->channelListModel->selectedNodesChanged(m_d->canvas->viewManager()->nodeManager()->selectedNodes());

    }
}

void KisAnimationCurveDocker::unsetCanvas()
{
    setCanvas(0);
}

void KisAnimationCurveDocker::setViewManager(KisViewManager *kisview)
{
    connect(kisview->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
    slotUpdateIcons();

    KisActionManager* manager = kisview->actionManager();

    KisAction* action = manager->createAction("insert_opacity_keyframe");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotAddOpacityKey()));
    action = manager->createAction("remove_opacity_keyframe");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotRemoveOpacityKey()));

}

void KisAnimationCurveDocker::addKeyframe(const QString &channel)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_d->canvas->image()->animationInterface()->currentTime();

    KisAnimationUtils::createKeyframeLazy(m_d->canvas->image(), node, channel, time, true);
}

void KisAnimationCurveDocker::removeKeyframe(const QString &channel)
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_d->canvas->image()->animationInterface()->currentTime();
    KisAnimationUtils::removeKeyframe(m_d->canvas->image(), node, channel, time);
}

void KisAnimationCurveDocker::slotScrollerStateChanged(QScroller::State state)
{
    KisKineticScroller::updateCursor(m_d->curvesWidget.channelListView, state);
}

void KisAnimationCurveDocker::slotNodeActivated(KisNodeSP node)
{
    if (!node) return;
    m_d->curvesWidget.btnAddKeyframe->setEnabled(node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id()));
}

void KisAnimationCurveDocker::slotUpdateIcons()
{
    m_d->curvesWidget.btnConstantInterpolation->setIcon(KisIconUtils::loadIcon("interpolation_constant"));
    m_d->curvesWidget.btnLinearInterpolation->setIcon(KisIconUtils::loadIcon("interpolation_linear"));
    m_d->curvesWidget.btnBezierInterpolation->setIcon(KisIconUtils::loadIcon("interpolation_bezier"));
    m_d->curvesWidget.btnSmooth->setIcon(KisIconUtils::loadIcon("interpolation_smooth"));
    m_d->curvesWidget.btnSharp->setIcon(KisIconUtils::loadIcon("interpolation_sharp"));

    m_d->curvesWidget.btnHorizontalZoom->setIcon(KisIconUtils::loadIcon("zoom-horizontal"));
    m_d->curvesWidget.btnVerticalZoom->setIcon(KisIconUtils::loadIcon("zoom-vertical"));
    m_d->curvesWidget.btnZoomToFit->setIcon(KisIconUtils::loadIcon("zoom-fit"));

    m_d->curvesWidget.btnAddKeyframe->setIcon(KisIconUtils::loadIcon("keyframe-add"));
    m_d->curvesWidget.btnRemoveKeyframes->setIcon(KisIconUtils::loadIcon("keyframe-remove"));
}

void KisAnimationCurveDocker::slotAddAllEnabledKeys()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas && m_d->canvas->viewManager());
    // remember current node's opacity and set it once we create a new opacity keyframe
    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(node);


    /* Once we have more than one supported scalar key value,
     * we should add a dropdown check-box set of actions that can
     * enable and disable keys. For now, since opacity is the only
     * key officially supported, we will just presume opacity. */
    if (node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id())) {
        addKeyframe(KisKeyframeChannel::Opacity.id());
    }
}

void KisAnimationCurveDocker::slotAddOpacityKey()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas && m_d->canvas->viewManager());

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(node);

    if (node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id())) {
        addKeyframe(KisKeyframeChannel::Opacity.id());
    }
}

void KisAnimationCurveDocker::slotRemoveOpacityKey()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->canvas && m_d->canvas->viewManager());

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    KIS_SAFE_ASSERT_RECOVER_RETURN(node);
    if (node->supportsKeyframeChannel(KisKeyframeChannel::Opacity.id())) {
        removeKeyframe(KisKeyframeChannel::Opacity.id());
    }
}

void KisAnimationCurveDocker::slotListRowsInserted(const QModelIndex &parentIndex, int first, int last)
{
    // Auto-expand nodes on the tree

    for (int r=first; r<=last; r++) {
        QModelIndex index = m_d->channelListModel->index(r, 0, parentIndex);
        m_d->curvesWidget.channelListView->expand(index);
    }
}
