/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_cache_populator.h"

#include <functional>

#include <QTimer>
#include <QMutex>
#include <QtConcurrent>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "KisPart.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_canvas2.h"
#include "kis_time_span.h"
#include "kis_animation_frame_cache.h"
#include "kis_update_info.h"
#include "kis_signal_auto_connection.h"
#include "kis_idle_watcher.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_keyframe_channel.h"

#include "KisAsyncAnimationCacheRenderer.h"
#include "dialogs/KisAsyncAnimationCacheRenderDialog.h"


struct KisAnimationCachePopulator::Private
{
    KisAnimationCachePopulator *q;
    KisPart *part;

    QTimer timer;

    /**
     * Counts up the number of subsequent times Krita has been detected idle.
     */
    int idleCounter;
    QStack<QPair<KisImageSP, int>> priorityFrames;
    static const int IDLE_COUNT_THRESHOLD = 4;
    static const int IDLE_CHECK_INTERVAL = 500;
    static const int BETWEEN_FRAMES_INTERVAL = 10;

    int requestedFrame;
    KisAnimationFrameCacheSP requestCache;
    KisOpenGLUpdateInfoSP requestInfo;
    KisSignalAutoConnectionsStore imageRequestConnections;

    QFutureWatcher<void> infoConversionWatcher;

    KisAsyncAnimationCacheRenderer regenerator;
    bool calculateAnimationCacheInBackground = true;



    enum State {
        NotWaitingForAnything,
        WaitingForIdle,
        WaitingForFrame,
        BetweenFrames
    };
    State state;

    Private(KisAnimationCachePopulator *_q, KisPart *_part)
        : q(_q),
          part(_part),
          idleCounter(0),
          priorityFrames(),
          requestedFrame(-1),
          state(WaitingForIdle)
    {
        timer.setSingleShot(true);
    }

    void timerTimeout() {
        switch (state) {
        case WaitingForIdle:
        case BetweenFrames:
            generateIfIdle();
            break;
        case WaitingForFrame:
            KIS_ASSERT_RECOVER_NOOP(0 && "WaitingForFrame cannot have a timeout. Just skip this message and report a bug");
            break;
        case NotWaitingForAnything:
            KIS_ASSERT_RECOVER_NOOP(0 && "NotWaitingForAnything cannot have a timeout. Just skip this message and report a bug");
            break;
        }
    }

    void generateIfIdle()
    {
        if (part->idleWatcher()->isIdle()) {
            idleCounter++;

            if (idleCounter >= IDLE_COUNT_THRESHOLD) {
                if (!tryRequestGeneration()) {
                    enterState(NotWaitingForAnything);
                }
                return;
            }
        } else {
            idleCounter = 0;
        }

        enterState(WaitingForIdle);
    }


    bool tryRequestGeneration()
    {
        if (!priorityFrames.isEmpty()) {
            KisImageSP image = priorityFrames.top().first;
            const int priorityFrame = priorityFrames.top().second;
            priorityFrames.pop();

            KisAnimationFrameCacheSP cache = KisAnimationFrameCache::cacheForImage(image);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(cache, false);

            bool requested = tryRequestGeneration(cache, KisTimeSpan(), priorityFrame);
            if (requested) return true;
        }

        // Prioritize the active document
        KisAnimationFrameCacheSP activeDocumentCache = KisAnimationFrameCacheSP(0);

        KisMainWindow *activeWindow = part->currentMainwindow();
        if (activeWindow && activeWindow->activeView()) {
            KisCanvas2 *activeCanvas = activeWindow->activeView()->canvasBase();

            if (activeCanvas && activeCanvas->frameCache()) {
                activeDocumentCache = activeCanvas->frameCache();

                // Let's skip frames affected by changes to the active node (on the active document)
                // This avoids constant invalidation and regeneration while drawing
                KisNodeSP activeNode = activeCanvas->viewManager()->nodeManager()->activeNode();
                KisTimeSpan skipRange;
                if (activeNode) {
                    const int currentTime = activeCanvas->currentImage()->animationInterface()->currentUITime();

                    if (!activeNode->keyframeChannels().isEmpty()) {
                        Q_FOREACH (const KisKeyframeChannel *channel, activeNode->keyframeChannels()) {
                            skipRange |= channel->affectedFrames(currentTime);
                        }
                    } else {
                        skipRange = KisTimeSpan::infinite(0);
                    }
                }

                bool requested = tryRequestGeneration(activeDocumentCache, skipRange, -1);
                if (requested) return true;
            }
        }

        QList<KisAnimationFrameCache*> caches = KisAnimationFrameCache::caches();
        KisAnimationFrameCache *cache;
        Q_FOREACH (cache, caches) {
            if (cache == activeDocumentCache.data()) {
                // We already handled this one...
                continue;
            }

            bool requested = tryRequestGeneration(cache, KisTimeSpan(), -1);
            if (requested) return true;
        }

        return false;
    }

    bool tryRequestGeneration(KisAnimationFrameCacheSP cache, KisTimeSpan skipRange, int priorityFrame)
    {
        KisImageSP image = cache->image();
        if (!image) return false;

        KisImageAnimationInterface *animation = image->animationInterface();
        KisTimeSpan currentRange = animation->fullClipRange();

        const int frame = priorityFrame >= 0 ? priorityFrame : KisAsyncAnimationCacheRenderDialog::calcFirstDirtyFrame(cache, currentRange, skipRange);

        if (frame >= 0) {
            return regenerate(cache, frame);
        }

        return false;
    }

    bool regenerate(KisAnimationFrameCacheSP cache, int frame)
    {
        if (state == WaitingForFrame) {
            // Already busy, deny request
            return false;
        }

        /**
         * We should enter the state before the frame is
         * requested. Otherwise the signal may come earlier than we
         * enter it.
         */
        enterState(WaitingForFrame);

        regenerator.setFrameCache(cache);

        // if we ever decide to add ROI to background cache
        // regeneration, it should be added here :)
        regenerator.startFrameRegeneration(cache->image(), frame);

        return true;
    }

    QString debugStateToString(State newState) {
        QString str = "<unknown>";

        switch (newState) {
        case WaitingForIdle:
            str = "WaitingForIdle";
            break;
        case WaitingForFrame:
            str = "WaitingForFrame";
            break;
        case NotWaitingForAnything:
            str = "NotWaitingForAnything";
            break;
        case BetweenFrames:
            str = "BetweenFrames";
            break;
        }

        return str;
    }

    void enterState(State newState)
    {
        //ENTER_FUNCTION() << debugStateToString(state) << "->" << debugStateToString(newState);

        state = newState;
        int timerTimeout = -1;

        switch (state) {
        case WaitingForIdle:
            timerTimeout = IDLE_CHECK_INTERVAL;
            break;
        case WaitingForFrame:
            // the timeout is handled by the regenerator now
            timerTimeout = -1;
            break;
        case NotWaitingForAnything:
            // frame conversion cannot be cancelled,
            // so there is no timeout
            timerTimeout = -1;
            break;
        case BetweenFrames:
            timerTimeout = BETWEEN_FRAMES_INTERVAL;
            break;
        }

        if (timerTimeout >= 0) {
            timer.start(timerTimeout);
        } else {
            timer.stop();
        }
    }
};

KisAnimationCachePopulator::KisAnimationCachePopulator(KisPart *part)
    : m_d(new Private(this, part))
{
    connect(&m_d->timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

    connect(&m_d->regenerator, SIGNAL(sigFrameCancelled(int)), SLOT(slotRegeneratorFrameCancelled()));
    connect(&m_d->regenerator, SIGNAL(sigFrameCompleted(int)), SLOT(slotRegeneratorFrameReady()));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();
}

KisAnimationCachePopulator::~KisAnimationCachePopulator()
{
    m_d->priorityFrames.clear();
}

bool KisAnimationCachePopulator::regenerate(KisAnimationFrameCacheSP cache, int frame)
{
    return m_d->regenerate(cache, frame);
}

void KisAnimationCachePopulator::requestRegenerationWithPriorityFrame(KisImageSP image, int frameIndex)
{
    if (!m_d->calculateAnimationCacheInBackground) return;

    m_d->priorityFrames.append(qMakePair(image, frameIndex));

    if (m_d->state == Private::NotWaitingForAnything) {
        m_d->generateIfIdle();
    }
}

void KisAnimationCachePopulator::slotTimer()
{
    m_d->timerTimeout();
}

void KisAnimationCachePopulator::slotRequestRegeneration()
{
    // skip if the user forbade background regeneration
    if (!m_d->calculateAnimationCacheInBackground) return;

    m_d->enterState(Private::WaitingForIdle);
}

void KisAnimationCachePopulator::slotRegeneratorFrameCancelled()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->state == Private::WaitingForFrame);
    m_d->enterState(Private::NotWaitingForAnything);
}

void KisAnimationCachePopulator::slotRegeneratorFrameReady()
{
    m_d->enterState(Private::BetweenFrames);
}

void KisAnimationCachePopulator::slotConfigChanged()
{
    KisConfig cfg(true);
    m_d->calculateAnimationCacheInBackground = cfg.calculateAnimationCacheInBackground();
    QTimer::singleShot(1000, this, SLOT(slotRequestRegeneration()));
}
