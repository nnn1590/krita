/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KIS_ANIMATION_CACHE_POPULATOR_H
#define KIS_ANIMATION_CACHE_POPULATOR_H

#include <QObject>
#include "kis_types.h"

class KisPart;

class KisAnimationCachePopulator : public QObject
{
    Q_OBJECT

public:
    KisAnimationCachePopulator(KisPart *part);
    ~KisAnimationCachePopulator() override;

    /**
     * Request generation of given frame. The request will
     * be ignored if the populator is already requesting a frame.
     * @return true if generation requested, false if busy
     */
    bool regenerate(KisAnimationFrameCacheSP cache, int frame);
    void requestRegenerationWithPriorityFrame(KisImageSP image, int frameIndex);

public Q_SLOTS:
    void slotRequestRegeneration();

private Q_SLOTS:
    void slotTimer();

    void slotRegeneratorFrameCancelled();
    void slotRegeneratorFrameReady();

    void slotConfigChanged();

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
