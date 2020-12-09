/*
 *  Copyright (c) 2004 C. Boemann <cbo@boemann.dk>
 *            (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_TILE_H_
#define KIS_TILE_H_

#include <QReadWriteLock>

#include <QMutex>
#include <QAtomicPointer>

#include <QRect>
#include <QStack>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "kis_tile_data.h"
#include "kis_tile_data_store.h"

//#define DEAD_TILES_SANITY_CHECK

class KisTile;
typedef KisSharedPtr<KisTile> KisTileSP;

class KisMementoManager;


/**
 * Provides abstraction to a tile.
 * + A tile contains a part of a PaintDevice,
 *   but only the individual pixels are accessible
 *   and that only via iterators.
 * + Actual tile data is stored in KisTileData that can be
 *   shared between many tiles
 */
class KRITAIMAGE_EXPORT KisTile : public KisShared
{
public:
    KisTile(qint32 col, qint32 row,
            KisTileData *defaultTileData, KisMementoManager* mm);
    KisTile(const KisTile& rhs, qint32 col, qint32 row, KisMementoManager* mm);
    KisTile(const KisTile& rhs, KisMementoManager* mm);
    KisTile(const KisTile& rhs);
    ~KisTile();

    /**
     * This method is called by the hash table when the tile is
     * disconnected from it. It means that from now on the tile is not
     * associated with any particular datamanager. All the users of
     * the tile (via shared pointers) may silently finish they work on
     * this tile and leave it. No result will be saved. Used for
     * threading purposes
     */
    void notifyDetachedFromDataManager();

    /**
     * Sometimes the tile gets replaced with another tile. In this case
     * we shouldn't notify memento manager that the tile has died. Just
     * forget the link to the manager and bury it in peace.
     */
    void notifyDeadWithoutDetaching();

    /**
     * Called by the hash table to notify that the tile has been attached
     * to the data manager.
     */
    void notifyAttachedToDataManager(KisMementoManager *mm);

public:

    void debugPrintInfo();
    void debugDumpTile();

    void lockForRead() const;
    void lockForWrite();
    void unlockForWrite();
    void unlockForRead() const;


    /* this allows us work directly on tile's data */
    inline quint8 *data() const {
        return m_tileData->data();
    }
    inline void setData(const quint8 *data) {
        m_tileData->setData(data);
    }

    inline qint32 row() const {
        return m_row;
    }
    inline qint32 col() const {
        return m_col;
    }

    inline QRect extent() const {
        return m_extent;
//QRect(m_col * KisTileData::WIDTH, m_row * KisTileData::HEIGHT,
//                     KisTileData::WIDTH, KisTileData::HEIGHT);
    }

    inline KisTileSP next() const {
        return m_nextTile;
    }

    void setNext(KisTileSP next) {
        m_nextTile = next;
    }

    inline qint32 pixelSize() const {
        /* don't lock here as pixelSize is constant */
        return m_tileData->pixelSize();
    }

    inline KisTileData*  tileData() const {
        return m_tileData;
    }

private:
    void init(qint32 col, qint32 row,
              KisTileData *defaultTileData, KisMementoManager* mm);

    inline void blockSwapping() const;
    inline void unblockSwapping() const;

    inline void safeReleaseOldTileData(KisTileData *td);

private:
    KisTileData *m_tileData;
    mutable QStack<KisTileData*> m_oldTileData;
    mutable volatile int m_lockCounter;

    qint32 m_col;
    qint32 m_row;

    /**
     * Added for faster retrieving by processors
     */
    QRect m_extent;

    /**
     * For KisTiledDataManager's hash table
     */
    KisTileSP m_nextTile;

    QAtomicPointer<KisMementoManager> m_mementoManager;

    /**
     * This is a special mutex for guarding copy-on-write
     * operations. We do not use lockless way here as it'll
     * create too much overhead for the most common operations
     * like "read the pointer of m_tileData".
     */
    QMutex m_COWMutex;

    /**
     * This lock is used to ensure no one will read the tile data
     * before it has been loaded from to the memory.
     */
    mutable QMutex m_swapBarrierLock;


#ifdef DEAD_TILES_SANITY_CHECK
    QAtomicInt m_sanityHasBeenDetached;
    QAtomicInt m_sanityIsDead;
    QAtomicInt m_sanityMMHasBeenInitializedManually;
    QAtomicInt m_sanityNumCOWHappened;
    QAtomicInt m_sanityLockedForWrite;
    mutable QAtomicInt m_sanityLockedForRead;

    void sanityCheckIsNotDestroyedYet();
    void sanityCheckIsNotLockedForWrite();
#endif

};

#endif // KIS_TILE_H_

