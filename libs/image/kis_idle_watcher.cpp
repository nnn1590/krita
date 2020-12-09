/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_idle_watcher.h"

#include <QTimer>
#include "kis_image.h"
#include "kis_signal_auto_connection.h"
#include "kis_signal_compressor.h"


struct KisIdleWatcher::Private
{
    static const int IDLE_CHECK_PERIOD = 200; /* ms */
    static const int IDLE_CHECK_COUNT = 4; /* ticks */

    Private(int delay, KisIdleWatcher *q)
        : imageModifiedCompressor(delay,
                                  KisSignalCompressor::POSTPONE, q),
          idleCheckCounter(0)
    {
        idleCheckTimer.setSingleShot(true);
        idleCheckTimer.setInterval(IDLE_CHECK_PERIOD);
    }

    KisSignalAutoConnectionsStore connectionsStore;
    QVector<KisImageWSP> trackedImages;

    KisSignalCompressor imageModifiedCompressor;

    QTimer idleCheckTimer;
    int idleCheckCounter;
};

KisIdleWatcher::KisIdleWatcher(int delay, QObject *parent)
    : QObject(parent), m_d(new Private(delay, this))
{
    connect(&m_d->imageModifiedCompressor, SIGNAL(timeout()), SLOT(startIdleCheck()));
    connect(&m_d->idleCheckTimer, SIGNAL(timeout()), SLOT(slotIdleCheckTick()));
}

KisIdleWatcher::~KisIdleWatcher()
{
}

bool KisIdleWatcher::isIdle() const
{
    bool idle = true;

    Q_FOREACH (KisImageSP image, m_d->trackedImages) {
        if (!image) continue;

        if (!image->isIdle()) {
            idle = false;
            break;
        }
    }

    return idle;
}

void KisIdleWatcher::setTrackedImages(const QVector<KisImageSP> &images)
{
    m_d->connectionsStore.clear();
    m_d->trackedImages.clear();

    Q_FOREACH (KisImageSP image, images) {
        if (image) {
            m_d->trackedImages << image;
            m_d->connectionsStore.addConnection(image, SIGNAL(sigImageModified()),
                                                this, SLOT(slotImageModified()));

            m_d->connectionsStore.addConnection(image, SIGNAL(sigIsolatedModeChanged()),
                                                this, SLOT(slotImageModified()));
        }
    }
}

void KisIdleWatcher::setTrackedImage(KisImageSP image)
{
    QVector<KisImageSP> images;
    images << image;
    setTrackedImages(images);
}

void KisIdleWatcher::slotImageModified()
{
    stopIdleCheck();
    m_d->imageModifiedCompressor.start();
}

void KisIdleWatcher::startIdleCheck()
{
    m_d->idleCheckCounter = 0;
    m_d->idleCheckTimer.start();
}

void KisIdleWatcher::stopIdleCheck()
{
    m_d->idleCheckTimer.stop();
    m_d->idleCheckCounter = 0;
}

void KisIdleWatcher::slotIdleCheckTick()
{
    if (isIdle()) {
        if (m_d->idleCheckCounter >= Private::IDLE_CHECK_COUNT) {
            stopIdleCheck();
            if (!m_d->trackedImages.isEmpty()) {
                emit startedIdleMode();
            }
        } else {
            m_d->idleCheckCounter++;
            m_d->idleCheckTimer.start();
        }
    } else {
        slotImageModified();
    }
}
