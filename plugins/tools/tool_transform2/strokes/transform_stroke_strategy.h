/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TRANSFORM_STROKE_STRATEGY_H
#define __TRANSFORM_STROKE_STRATEGY_H

#include <QObject>
#include <QMutex>
#include <KoUpdater.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <kis_types.h>
#include "tool_transform_args.h"
#include <kis_processing_visitor.h>
#include <kritatooltransform_export.h>
#include <boost/optional.hpp>


class KisPostExecutionUndoAdapter;
class TransformTransactionProperties;
class KisUpdatesFacade;
class KisDecoratedNodeInterface;


class TransformStrokeStrategy : public QObject, public KisStrokeStrategyUndoCommandBased
{
    Q_OBJECT
public:
    struct TransformAllData : public KisStrokeJobData {
        TransformAllData(const ToolTransformArgs &_config)
            : KisStrokeJobData(SEQUENTIAL, NORMAL),
              config(_config) {}

        ToolTransformArgs config;
    };


    class TransformData : public KisStrokeJobData {
    public:
        enum Destination {
            PAINT_DEVICE,
            SELECTION,
        };

    public:
    TransformData(Destination _destination, const ToolTransformArgs &_config, KisNodeSP _node)
            : KisStrokeJobData(CONCURRENT, NORMAL),
            destination(_destination),
            config(_config),
            node(_node)
        {
        }

        Destination destination;
        ToolTransformArgs config;
        KisNodeSP node;
    };

    class ClearSelectionData : public KisStrokeJobData {
    public:
        ClearSelectionData(KisNodeSP _node)
            : KisStrokeJobData(SEQUENTIAL, NORMAL),
              node(_node)
        {
        }
        KisNodeSP node;
    };

    class PreparePreviewData : public KisStrokeJobData {
    public:
        PreparePreviewData()
            : KisStrokeJobData(BARRIER, NORMAL)
        {
        }
    };

public:
    TransformStrokeStrategy(ToolTransformArgs::TransformMode mode,
                            bool workRecursively,
                            const QString &filterId,
                            bool forceReset,
                            KisNodeSP rootNode,
                            KisSelectionSP selection,
                            KisStrokeUndoFacade *undoFacade, KisUpdatesFacade *updatesFacade);

    ~TransformStrokeStrategy() override;

    static bool shouldRestartStrokeOnModeChange(ToolTransformArgs::TransformMode oldMode,
                                                ToolTransformArgs::TransformMode newMode,
                                                KisNodeList processedNodes);

    void initStrokeCallback() override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;

    static bool fetchArgsFromCommand(const KUndo2Command *command, ToolTransformArgs *args, KisNodeSP *rootNode, KisNodeList *transformedNodes);

Q_SIGNALS:
    void sigTransactionGenerated(TransformTransactionProperties transaction, ToolTransformArgs args, void *cookie);
    void sigPreviewDeviceReady(KisPaintDeviceSP device);

protected:
    void postProcessToplevelCommand(KUndo2Command *command) override;

private:
    KoUpdaterPtr fetchUpdater(KisNodeSP node);

    void transformAndMergeDevice(const ToolTransformArgs &config,
                                 KisPaintDeviceSP src,
                                 KisPaintDeviceSP dst,
                                 KisProcessingVisitor::ProgressHelper *helper);
    void transformDevice(const ToolTransformArgs &config,
                         KisPaintDeviceSP device,
                         KisProcessingVisitor::ProgressHelper *helper);

    void clearSelection(KisPaintDeviceSP device);
    //void transformDevice(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisProcessingVisitor::ProgressHelper *helper);

    bool checkBelongsToSelection(KisPaintDeviceSP device) const;

    KisPaintDeviceSP createDeviceCache(KisPaintDeviceSP src);

    bool haveDeviceInCache(KisPaintDeviceSP src);
    void putDeviceCache(KisPaintDeviceSP src, KisPaintDeviceSP cache);
    KisPaintDeviceSP getDeviceCache(KisPaintDeviceSP src);

    QList<KisNodeSP> fetchNodesList(ToolTransformArgs::TransformMode mode, KisNodeSP root, bool recursive);
    ToolTransformArgs resetArgsForMode(ToolTransformArgs::TransformMode mode,
                                       const QString &filterId,
                                       const TransformTransactionProperties &transaction);
    bool tryInitArgsFromNode(KisNodeSP node, ToolTransformArgs *args);
    bool tryFetchArgsFromCommandAndUndo(ToolTransformArgs *args,
                                        ToolTransformArgs::TransformMode mode,
                                        KisNodeSP currentNode,
                                        KisNodeList selectedNodes, QVector<KisStrokeJobData *> *undoJobs);

    void finishStrokeImpl(bool applyTransform,
                          const ToolTransformArgs &args);

private:
    KisUpdatesFacade *m_updatesFacade;
    ToolTransformArgs::TransformMode m_mode;
    bool m_workRecursively;
    QString m_filterId;
    bool m_forceReset;

    KisSelectionSP m_selection;

    QMutex m_devicesCacheMutex;
    QHash<KisPaintDevice*, KisPaintDeviceSP> m_devicesCacheHash;

    KisTransformMaskSP writeToTransformMask;

    ToolTransformArgs m_initialTransformArgs;
    boost::optional<ToolTransformArgs> m_savedTransformArgs;
    KisNodeSP m_rootNode;
    KisNodeList m_processedNodes;
    QList<KisSelectionSP> m_deactivatedSelections;
    QList<KisNodeSP> m_hiddenProjectionLeaves;
    KisSelectionMaskSP m_deactivatedOverlaySelectionMask;
    QVector<KisDecoratedNodeInterface*> m_disabledDecoratedNodes;

    const KisSavedMacroCommand *m_overriddenCommand = 0;
    QVector<const KUndo2Command*> m_skippedWhileMergeCommands;

    bool m_finalizingActionsStarted = false;
};

#endif /* __TRANSFORM_STROKE_STRATEGY_H */
