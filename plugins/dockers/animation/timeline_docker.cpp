/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "timeline_docker.h"

#include <QPointer>
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "QFormLayout"
#include "QLabel"
#include "QPushButton"
#include "QToolButton"
#include "QMenu"
#include "QWidgetAction"

#include "krita_utils.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include <KoIcon.h>
#include "KisViewManager.h"
#include "kis_paint_layer.h"
#include "KisDocument.h"
#include "kis_dummies_facade.h"
#include "kis_shape_controller.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "kis_animation_player.h"
#include "kis_animation_utils.h"
#include "kis_image_config.h"
#include "kis_keyframe_channel.h"
#include "kis_image.h"

#include "timeline_frames_model.h"
#include "timeline_frames_view.h"
#include "kis_time_span.h"
#include "kis_animation_frame_cache.h"
#include "kis_image_animation_interface.h"
#include "kis_signal_auto_connection.h"
#include "kis_node_manager.h"
#include "kis_transport_controls.h"
#include "kis_int_parse_spin_box.h"
#include "kis_slider_spin_box.h"
#include "kis_signals_blocker.h"

TimelineDockerTitleBar::TimelineDockerTitleBar(QWidget* parent) :
    KisUtilityTitleBar(new QLabel(i18n("Animation Timeline"), parent), parent)
{
    // Transport Controls...
    transport = new KisTransportControls(this);
    widgetAreaLayout->addWidget(transport);

    widgetAreaLayout->addSpacing(SPACING_UNIT);

    // Frame Register...
    frameRegister = new KisIntParseSpinBox(this);
    frameRegister->setToolTip(i18n("Frame register"));
    frameRegister->setPrefix("#  ");
    frameRegister->setRange(0, MAX_FRAMES);
    widgetAreaLayout->addWidget(frameRegister);

    widgetAreaLayout->addSpacing(SPACING_UNIT);

    {   // Frame ops...
        QWidget *widget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

        btnAddKeyframe = new QToolButton(this);
        layout->addWidget(btnAddKeyframe);

        btnDuplicateKeyframe = new QToolButton(this);
        layout->addWidget(btnDuplicateKeyframe);

        btnRemoveKeyframe = new QToolButton(this);
        layout->addWidget(btnRemoveKeyframe);

        widgetAreaLayout->addWidget(widget);
    }

    widgetAreaLayout->addSpacing(SPACING_UNIT);

    // Drop Frames..
    btnDropFrames = new QToolButton(this);
    widgetAreaLayout->addWidget(btnDropFrames);

    // Playback Speed..
    sbSpeed = new KisSliderSpinBox(this);
    sbSpeed->setRange(25, 200);
    sbSpeed->setSingleStep(5);
    sbSpeed->setValue(100);
    sbSpeed->setPrefix("Speed: ");
    sbSpeed->setSuffix(" %");
    sbSpeed->setToolTip(i18n("Preview playback speed"));
    widgetAreaLayout->addWidget(sbSpeed);

    widgetAreaLayout->addStretch();

    {   // Menus..
        QWidget *widget = new QWidget(this);

        QHBoxLayout *layout = new QHBoxLayout(widget);
        layout->setSpacing(0);
        layout->setContentsMargins(SPACING_UNIT,0,0,0);

        // Onion skins menu.
        btnOnionSkinsMenu = new QPushButton(KisIconUtils::loadIcon("onion_skin_options"), "", this);
        btnOnionSkinsMenu->setToolTip(i18n("Onion skins menu"));
        layout->addWidget(btnOnionSkinsMenu);

        // Audio menu..
        btnAudioMenu = new QPushButton(KisIconUtils::loadIcon("audio-none"), "", this);
        btnOnionSkinsMenu->setToolTip(i18n("Audio menu"));
        btnAudioMenu->hide(); // (NOTE: Hidden for now while audio features develop.)
        layout->addWidget(btnAudioMenu);

        {   // Settings menu..
            btnSettingsMenu = new QToolButton(this);
            btnSettingsMenu->setIcon(KisIconUtils::loadIcon("configure"));
            btnSettingsMenu->setToolTip(i18n("Animation settings menu"));

            QWidget *settingsMenuWidget = new QWidget(this);
            QHBoxLayout *settingsMenuLayout = new QHBoxLayout(settingsMenuWidget);

            QWidget *fields = new QWidget(settingsMenuWidget);
            QFormLayout *fieldsLayout = new QFormLayout(fields);

            sbStartFrame = new KisIntParseSpinBox(settingsMenuWidget);
            sbStartFrame->setMaximum(10000);
            fieldsLayout->addRow(i18n("Clip Start: "), sbStartFrame);

            sbEndFrame = new KisIntParseSpinBox(settingsMenuWidget);
            sbEndFrame->setMaximum(10000);
            fieldsLayout->addRow(i18n("Clip End: "), sbEndFrame);

            sbFrameRate = new KisIntParseSpinBox(settingsMenuWidget);
            sbFrameRate->setMinimum(0);
            sbFrameRate->setMaximum(180);
            fieldsLayout->addRow(i18n("Frame Rate: "), sbFrameRate);

            QWidget *buttons = new QWidget(settingsMenuWidget);
            QVBoxLayout *buttonsLayout = new QVBoxLayout(buttons);
            buttonsLayout->setAlignment(Qt::AlignTop);

            {   // AutoKey..
                // AutoKey Actions & Action Group..
                autoKeyBlank = new QAction(i18n("AutoKey Blank"), this);
                autoKeyBlank->setCheckable(true);
                autoKeyDuplicate = new QAction(i18n("AutoKey Duplicate"), this);
                autoKeyDuplicate->setCheckable(true);
                QActionGroup *autoKeyModes = new QActionGroup(this);
                autoKeyModes->addAction(autoKeyBlank);
                autoKeyModes->addAction(autoKeyDuplicate);
                autoKeyModes->setExclusive(true);

                connect(autoKeyModes, &QActionGroup::triggered, [this](QAction* modeAction){
                    if (!modeAction) return;
                    KisImageConfig  imageCfg(false);
                    if (modeAction == autoKeyBlank) {
                        imageCfg.setAutoKeyModeDuplicate(false);
                    } else if (modeAction == autoKeyDuplicate) {
                        imageCfg.setAutoKeyModeDuplicate(true);
                    }
                });

                // AutoKey Mode Menu..
                QMenu *autoKeyModeMenu = new QMenu(settingsMenuWidget);
                autoKeyModeMenu->addActions(autoKeyModes->actions());

                // AutoKey Button..
                btnAutoKey = new QToolButton(settingsMenuWidget);
                btnAutoKey->setMenu(autoKeyModeMenu);
                btnAutoKey->setPopupMode(QToolButton::MenuButtonPopup);
                buttonsLayout->addWidget(btnAutoKey);
            }

            settingsMenuLayout->addWidget(fields);
            settingsMenuLayout->addWidget(buttons);

            layout->addWidget(btnSettingsMenu);

            QMenu *settingsPopMenu = new QMenu(this);
            QWidgetAction *settingsMenuAction = new QWidgetAction(this);
            settingsMenuAction->setDefaultWidget(settingsMenuWidget);
            settingsPopMenu->addAction(settingsMenuAction);

            btnSettingsMenu->setPopupMode(QToolButton::InstantPopup);
            btnSettingsMenu->setMenu(settingsPopMenu);
        }

        widgetAreaLayout->addWidget(widget);
    }
}

struct TimelineDocker::Private
{
    Private(QWidget *parent)
        : framesModel(new TimelineFramesModel(parent)),
          framesView(new TimelineFramesView(parent)),
          titlebar(new TimelineDockerTitleBar(parent)),
          mainWindow(nullptr)
    {
        framesView->setModel(framesModel);
    }

    TimelineFramesModel *framesModel;
    TimelineFramesView *framesView;
    TimelineDockerTitleBar *titlebar;

    QPointer<KisCanvas2> canvas;

    KisSignalAutoConnectionsStore canvasConnections;
    KisMainWindow *mainWindow;
};

TimelineDocker::TimelineDocker()
    : QDockWidget(i18n("Animation Timeline"))
    , m_d(new Private(this))
{
    setWidget(m_d->framesView);
    setTitleBarWidget(m_d->titlebar);

    connect(m_d->titlebar->transport, SIGNAL(back()), this, SLOT(previousFrame()));
    connect(m_d->titlebar->transport, SIGNAL(stop()), this, SLOT(stop()));
    connect(m_d->titlebar->transport, SIGNAL(playPause()), this, SLOT(playPause()));
    connect(m_d->titlebar->transport, SIGNAL(forward()), this, SLOT(nextFrame()));

    connect(m_d->titlebar->frameRegister, SIGNAL(valueChanged(int)), SLOT(goToFrame(int)));
    connect(m_d->titlebar->sbStartFrame, SIGNAL(valueChanged(int)), SLOT(setStartFrame(int)));
    connect(m_d->titlebar->sbEndFrame, SIGNAL(valueChanged(int)), SLOT(setEndFrame(int)));
    connect(m_d->titlebar->sbFrameRate, SIGNAL(valueChanged(int)), SLOT(setFrameRate(int)));
    connect(m_d->titlebar->sbSpeed, SIGNAL(valueChanged(int)), SLOT(setPlaybackSpeed(int)));

    connect(m_d->titlebar->btnOnionSkinsMenu, &QPushButton::released, [this](){
        if (m_d->mainWindow) {
            QDockWidget *docker = m_d->mainWindow->dockWidget("OnionSkinsDocker");
            if (docker) {
                docker->setVisible(!docker->isVisible());
            }
        }
    });
}

TimelineDocker::~TimelineDocker()
{
}

struct NodeManagerInterface : TimelineFramesModel::NodeManipulationInterface
{
    NodeManagerInterface(KisNodeManager *manager) : m_manager(manager) {}

    KisLayerSP addPaintLayer() const override {
        return m_manager->createPaintLayer();
    }

    void removeNode(KisNodeSP node) const override {
        m_manager->removeSingleNode(node);
    }

    bool setNodeProperties(KisNodeSP node, KisImageSP image, KisBaseNode::PropertyList properties) const override
    {
        return m_manager->trySetNodeProperties(node, image, properties);
    }

private:
    KisNodeManager *m_manager;
};

void TimelineDocker::setCanvas(KoCanvasBase * canvas)
{
    if (m_d->canvas == canvas) return;

    if (m_d->framesView) {
        m_d->framesView->slotCanvasUpdate(canvas);
    }

    if (m_d->framesModel->hasConnectionToCanvas()) {
        m_d->canvasConnections.clear();
        m_d->framesModel->setDummiesFacade(0, 0, 0);
        m_d->framesModel->setFrameCache(0);
        m_d->framesModel->setAnimationPlayer(0);
        m_d->framesModel->setNodeManipulationInterface(0);
    }

    if (m_d->canvas) {
        m_d->canvas->disconnectCanvasObserver(this);
        m_d->canvas->animationPlayer()->disconnect(this);

        if(m_d->canvas->image()) {
            m_d->canvas->image()->animationInterface()->disconnect(this);
        }
    }

    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if(m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->framesModel->setDummiesFacade(kritaShapeController,
                                     m_d->canvas->image(),
                                     m_d->canvas->viewManager()->nodeManager()->nodeDisplayModeAdapter());

        updateFrameCache();

        {
            KisSignalsBlocker blocker(m_d->titlebar->sbStartFrame,
                                      m_d->titlebar->sbEndFrame,
                                      m_d->titlebar->sbFrameRate);

            KisImageAnimationInterface *animinterface = m_d->canvas->image()->animationInterface();
            m_d->titlebar->sbStartFrame->setValue(animinterface->fullClipRange().start());
            m_d->titlebar->sbEndFrame->setValue(animinterface->fullClipRange().end());
            m_d->titlebar->sbFrameRate->setValue(animinterface->framerate());
            m_d->titlebar->sbSpeed->setValue(100);
            m_d->titlebar->frameRegister->setValue(animinterface->currentTime());
        }


        m_d->framesModel->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->framesModel->setNodeManipulationInterface(
            new NodeManagerInterface(m_d->canvas->viewManager()->nodeManager()));

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->framesModel, SLOT(slotCurrentNodeChanged(KisNodeSP)));

        m_d->canvasConnections.addConnection(
            m_d->framesModel, SIGNAL(requestCurrentNodeChanged(KisNodeSP)),
            m_d->canvas->viewManager()->nodeManager(), SLOT(slotNonUiActivatedNode(KisNodeSP)));

        m_d->framesModel->slotCurrentNodeChanged(m_d->canvas->viewManager()->activeNode());

        m_d->canvasConnections.addConnection(
                    m_d->canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()),
                    this, SLOT(handleThemeChange()));

        m_d->canvasConnections.addConnection(
                    m_d->canvas, SIGNAL(sigCanvasEngineChanged()),
                    this, SLOT(updateFrameCache()));


        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigUiTimeChanged(int)), this, SLOT(updateFrameRegister()));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(updateFrameRegister()));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(updateFrameRegister()));

        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStateChanged(bool)), m_d->titlebar->frameRegister, SLOT(setDisabled(bool)));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStateChanged(bool)), m_d->titlebar->transport, SLOT(setPlaying(bool)));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStatisticsUpdated()), this, SLOT(updatePlaybackStatistics()));

        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFullClipRangeChanged()), SLOT(handleClipRangeChange()));
        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFramerateChanged()), SLOT(handleFrameRateChange()));
    }
}

void TimelineDocker::handleThemeChange()
{
    if (m_d->framesView) {
        m_d->framesView->slotUpdateIcons();
    }
}

void TimelineDocker::updateFrameCache()
{
    m_d->framesModel->setFrameCache(m_d->canvas->frameCache());
}

void TimelineDocker::updateFrameRegister()
{
    if (!m_d->canvas && !m_d->canvas->image()) {
        return;
    }

    const int frame = m_d->canvas->animationPlayer()->isPlaying() ?
                      m_d->canvas->animationPlayer()->visibleFrame() :
                      m_d->canvas->image()->animationInterface()->currentUITime();

    m_d->titlebar->frameRegister->setValue(frame);
}

void TimelineDocker::updatePlaybackStatistics()
{
    qreal effectiveFps = 0.0;
    qreal realFps = 0.0;
    qreal framesDropped = 0.0;
    bool isPlaying = false;

    KisAnimationPlayer *player = m_d->canvas &&  m_d->canvas->animationPlayer() ?  m_d->canvas->animationPlayer() : 0;
    if (player) {
        effectiveFps = player->effectiveFps();
        realFps = player->realFps();
        framesDropped = player->framesDroppedPortion();
        isPlaying = player->isPlaying();
    }

    KisConfig cfg(true);
    const bool shouldDropFrames = cfg.animationDropFrames();

    QAction *action = m_d->titlebar->btnDropFrames->defaultAction();
    const bool droppingFrames = shouldDropFrames && framesDropped > 0.05;
    action->setIcon(KisIconUtils::loadIcon(droppingFrames ? "droppedframes" : "dropframe"));

    QString actionText;
    if (!isPlaying) {
        actionText = QString("%1 (%2) \n%3")
            .arg(KisAnimationUtils::dropFramesActionName)
            .arg(KritaUtils::toLocalizedOnOff(shouldDropFrames))
            .arg(i18n("Enable to preserve playback timing."));
    } else {
        actionText = QString("%1 (%2)\n"
                       "%3\n"
                       "%4\n"
                       "%5")
            .arg(KisAnimationUtils::dropFramesActionName)
            .arg(KritaUtils::toLocalizedOnOff(shouldDropFrames))
            .arg(i18n("Effective FPS:\t%1", effectiveFps))
            .arg(i18n("Real FPS:\t%1", realFps))
            .arg(i18n("Frames dropped:\t%1\%", framesDropped * 100));
    }
    action->setText(actionText);
}

void TimelineDocker::unsetCanvas()
{
    setCanvas(0);
}

void TimelineDocker::setViewManager(KisViewManager *view)
{
    m_d->mainWindow = view->mainWindow();
    KisActionManager *actionManager = view->actionManager();
    m_d->framesView->setActionManager(actionManager);

    KisAction *action = 0;

    TimelineDockerTitleBar* titleBar = static_cast<TimelineDockerTitleBar*>(titleBarWidget());

    action = actionManager->actionByName("add_blank_frame");
    titleBar->btnAddKeyframe->setDefaultAction(action);

    action = actionManager->actionByName("add_duplicate_frame");
    titleBar->btnDuplicateKeyframe->setDefaultAction(action);

    action = actionManager->actionByName("remove_frames");
    titleBar->btnRemoveKeyframe->setDefaultAction(action);

    action = actionManager->createAction("toggle_playback");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(playPause()));

    action = actionManager->createAction("stop_playback");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(stop()));

    action = actionManager->createAction("previous_frame");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(previousFrame()));

    action = actionManager->createAction("next_frame");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(nextFrame()));

    action = actionManager->createAction("previous_keyframe");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(previousKeyframe()));

    action = actionManager->createAction("next_keyframe");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(nextKeyframe()));

    action = actionManager->createAction("previous_matching_keyframe");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(previousMatchingKeyframe()));

    action = actionManager->createAction("next_matching_keyframe");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(nextMatchingKeyframe()));

    action = actionManager->createAction("auto_key");
    m_d->titlebar->btnAutoKey->setDefaultAction(action);
    connect(action, SIGNAL(triggered(bool)), SLOT(setAutoKey(bool)));

    {
        KisImageConfig config(true);
        action->setChecked(config.autoKeyEnabled());
        action->setIcon(config.autoKeyEnabled() ? KisIconUtils::loadIcon("auto-key-on") : KisIconUtils::loadIcon("auto-key-off"));

        const bool autoKeyModeDuplicate = config.autoKeyModeDuplicate();
        m_d->titlebar->autoKeyBlank->setChecked(!autoKeyModeDuplicate);
        m_d->titlebar->autoKeyDuplicate->setChecked(autoKeyModeDuplicate);
    }

    action = actionManager->createAction("drop_frames");
    m_d->titlebar->btnDropFrames->setDefaultAction(action);
    connect(action, SIGNAL(triggered(bool)), SLOT(setDropFrames(bool)));

    {
        KisConfig config(true);
        action->setChecked(config.animationDropFrames());
    }
}

void TimelineDocker::playPause()
{
    if (!m_d->canvas) return;

    if (m_d->canvas->animationPlayer()->isPlaying()) {
        m_d->canvas->animationPlayer()->pause();
    } else {
        m_d->canvas->animationPlayer()->play();
    }
}

void TimelineDocker::stop()
{
    if (!m_d->canvas) return;

    if( m_d->canvas->animationPlayer()->isStopped()) {
        m_d->canvas->animationPlayer()->goToStartFrame();
    } else {
        m_d->canvas->animationPlayer()->stop();
        m_d->canvas->animationPlayer()->goToPlaybackOrigin();
    }
}

void TimelineDocker::previousFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = animInterface->currentUITime() - 1;

    if (frame < startFrame || frame >  endFrame) {
        frame = endFrame;
    }

    if (frame >= 0) {
        animInterface->requestTimeSwitchWithUndo(frame);
    }
}

void TimelineDocker::nextFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = animInterface->currentUITime() + 1;

    if (frame > endFrame || frame < startFrame ) {
        frame = startFrame;
    }

    animInterface->requestTimeSwitchWithUndo(frame);
}

void TimelineDocker::previousKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    int currentFrame = animInterface->currentUITime();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animInterface->requestTimeSwitchWithUndo(destinationTime);
    }
}

void TimelineDocker::nextKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    int currentTime = animation->currentUITime();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animation->requestTimeSwitchWithUndo(destinationTime);
    }
}

void TimelineDocker::previousMatchingKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();
    int time = animInterface->currentUITime();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = currentKeyframe ? currentKeyframe->colorLabel() : keyframes->keyframeAt(destinationTime)->colorLabel();
    while (keyframes->keyframeAt(destinationTime) &&
           (currentKeyframe == keyframes->keyframeAt(destinationTime) || keyframes->keyframeAt(destinationTime)->colorLabel() != desiredColor)) {
        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animInterface->requestTimeSwitchWithUndo(destinationTime);
    }

}

void TimelineDocker::nextMatchingKeyframe()
{
    if (!m_d->canvas) return;

    KisNodeSP node = m_d->canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    KisImageAnimationInterface *animation = m_d->canvas->image()->animationInterface();
    int time = animation->currentUITime();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = keyframes->keyframeAt(destinationTime)->colorLabel();

    while (keyframes->keyframeAt(destinationTime) &&
                (keyframes->keyframeAt(destinationTime) == keyframes->keyframeAt(time) ||
                 keyframes->keyframeAt(destinationTime)->colorLabel() != desiredColor)){
        destinationTime = keyframes->nextKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        animation->requestTimeSwitchWithUndo(destinationTime);
    }
}

void TimelineDocker::goToFrame(int frameIndex)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    if (m_d->canvas->animationPlayer()->isPlaying() ||
        frameIndex == animInterface->currentUITime()) {
        return;
    }

    animInterface->requestTimeSwitchWithUndo(frameIndex);
}

void TimelineDocker::setStartFrame(int frame)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    animInterface->setFullClipRangeStartTime(frame);
}

void TimelineDocker::setEndFrame(int frame)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    animInterface->setFullClipRangeEndTime(frame);
}

void TimelineDocker::setFrameRate(int framerate)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    animInterface->setFramerate(framerate);
}

void TimelineDocker::setPlaybackSpeed(int playbackSpeed)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    const float normalizedSpeed = playbackSpeed / 100.0;
    m_d->canvas->animationPlayer()->slotUpdatePlaybackSpeed(normalizedSpeed);
}

void TimelineDocker::setDropFrames(bool dropFrames)
{
    KisConfig cfg(false);
    if (dropFrames != cfg.animationDropFrames()) {
        cfg.setAnimationDropFrames(dropFrames);
        updatePlaybackStatistics();
    }
}

void TimelineDocker::setAutoKey(bool value)
{
    KisImageConfig cfg(false);
    if (value != cfg.autoKeyEnabled()) {
        cfg.setAutoKeyEnabled(value);
        const QIcon icon = cfg.autoKeyEnabled() ? KisIconUtils::loadIcon("auto-key-on") : KisIconUtils::loadIcon("auto-key-off");
        QAction* action = m_d->titlebar->btnAutoKey->defaultAction();
        action->setIcon(icon);
    }
}

void TimelineDocker::handleClipRangeChange()
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    m_d->titlebar->sbStartFrame->setValue(animInterface->fullClipRange().start());
    m_d->titlebar->sbEndFrame->setValue(animInterface->fullClipRange().end());
}

void TimelineDocker::handleFrameRateChange()
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    m_d->titlebar->sbFrameRate->setValue(animInterface->framerate());
}


