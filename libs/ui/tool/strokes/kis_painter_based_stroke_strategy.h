/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINTER_BASED_STROKE_STRATEGY_H
#define __KIS_PAINTER_BASED_STROKE_STRATEGY_H

#include <QVector>

#include "KisRunnableBasedStrokeStrategy.h"
#include "kis_resources_snapshot.h"
#include "kis_selection.h"

class KisPainter;
class KisDistanceInformation;
class KisTransaction;
class KisFreehandStrokeInfo;
class KisMaskedFreehandStrokePainter;
class KisMaskingBrushRenderer;
class KisRunnableStrokeJobData;


class KRITAUI_EXPORT KisPainterBasedStrokeStrategy : public KisRunnableBasedStrokeStrategy
{
public:
    KisPainterBasedStrokeStrategy(const QString &id,
                                  const KUndo2MagicString &name,
                                  KisResourcesSnapshotSP resources,
                                  QVector<KisFreehandStrokeInfo*> strokeInfos, bool useMergeID = false);

    KisPainterBasedStrokeStrategy(const QString &id,
                                  const KUndo2MagicString &name,
                                  KisResourcesSnapshotSP resources,
                                  KisFreehandStrokeInfo *strokeInfo,bool useMergeID = false);

    ~KisPainterBasedStrokeStrategy();

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;

    void suspendStrokeCallback() override;
    void resumeStrokeCallback() override;

protected:
    KisNodeSP targetNode() const;
    KisPaintDeviceSP targetDevice() const;
    KisSelectionSP activeSelection() const;

    KisMaskedFreehandStrokePainter* maskedPainter(int strokeInfoId);
    int numMaskedPainters() const;

    void setUndoEnabled(bool value);

    /**
     * Return true if the descendant should execute a few more jobs before issuing setDirty()
     * call on the layer.
     *
     * If the returned value is true, then the stroke actually paints **not** on the
     * layer's paint device, but on some intermediate device owned by
     * KisPainterBasedStrokeStrategy and one should merge it first before asking the
     * update.
     *
     * The value can be true only when the stroke is declared to support masking brush!
     * \see supportsMaskingBrush()
     */
    bool needsMaskingUpdates() const;

    /**
     * Create a list of update jobs that should be run before issuing the setDirty()
     * call on the node
     *
     * \see needsMaskingUpdates()
     */
    QVector<KisRunnableStrokeJobData*> doMaskingBrushUpdates(const QVector<QRect> &rects);

protected:

    /**
     * The descendants may declare if this stroke should support auto-creation
     * of the masking brush. Default value: false
     */
    void setSupportsMaskingBrush(bool value);

    /**
     * Return if the stroke should auto-create a masking brush from the provided
     * paintop preset or not
     */
    bool supportsMaskingBrush() const;

protected:
    KisPainterBasedStrokeStrategy(const KisPainterBasedStrokeStrategy &rhs, int levelOfDetail);

private:
    void init();
    void initPainters(KisPaintDeviceSP targetDevice, KisPaintDeviceSP maskingDevice,
                      KisSelectionSP selection,
                      bool hasIndirectPainting,
                      const QString &indirectPaintingCompositeOp);
    void deletePainters();
    inline int timedID(const QString &id){
        return int(qHash(id));
    }

private:
    KisResourcesSnapshotSP m_resources;
    QVector<KisFreehandStrokeInfo*> m_strokeInfos;
    QVector<KisFreehandStrokeInfo*> m_maskStrokeInfos;
    QVector<KisMaskedFreehandStrokePainter*> m_maskedPainters;

    KisTransaction *m_transaction;

    QScopedPointer<KisMaskingBrushRenderer> m_maskingBrushRenderer;

    KisPaintDeviceSP m_targetDevice;
    KisSelectionSP m_activeSelection;
    bool m_useMergeID;

    bool m_supportsMaskingBrush;
};

#endif /* __KIS_PAINTER_BASED_STROKE_STRATEGY_H */
