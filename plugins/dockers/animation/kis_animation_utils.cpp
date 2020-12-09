/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_animation_utils.h"

#include "kundo2command.h"
#include "kis_algebra_2d.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_global.h"
#include "kis_tool_utils.h"
#include "kis_image_animation_interface.h"
#include "kis_command_utils.h"
#include "kis_processing_applicator.h"
#include "kis_transaction.h"


namespace KisAnimationUtils {
    const QString lazyFrameCreationActionName = i18n("Auto Frame Mode");
    const QString dropFramesActionName = i18n("Drop Frames");

    const QString newLayerActionName = i18n("New Layer");
    const QString pinExistingLayerActionName = i18n("Pin Existing Layer");
    const QString removeLayerActionName = i18n("Remove Layer");

    const QString addTransformKeyframeActionName = i18n("Add transform keyframe");
    const QString removeTransformKeyframeActionName = i18n("Remove transform keyframe");

    KUndo2Command* createKeyframeCommand(KisImageSP image, KisNodeSP node, const QString &channelId, int time, bool copy, KUndo2Command *parentCommand) {
        KUndo2Command *cmd = new KisCommandUtils::LambdaCommand(
            copy ? kundo2_i18n("Copy Keyframe") :
                   kundo2_i18n("Add Keyframe"),
            parentCommand,

            [image, node, channelId, time, copy] () mutable -> KUndo2Command* {
                bool result = false;

                QScopedPointer<KUndo2Command> cmd(new KUndo2Command());

                KisKeyframeChannel *channel = node->getKeyframeChannel(channelId);
                quint8 originalOpacity = node->opacity();
                bool createdChannel = false;

                if (!channel) {
                    node->enableAnimation();
                    channel = node->getKeyframeChannel(channelId, true);
                    if (!channel) return nullptr;

                    createdChannel = true;
                }

                if (copy && channel->activeKeyframeAt(time)) {
                    if (!channel->keyframeAt(time)) {
                        channel->copyKeyframe(channel->activeKeyframeTime(time), time, cmd.data());
                        result = true;
                    }
                } else {
                    if (channel->keyframeAt(time) && !createdChannel) {
                        if (image->animationInterface()->currentTime() == time && channelId == KisKeyframeChannel::Raster.id()) {

                            //shortcut: clearing the image instead
                            KisPaintDeviceSP device = node->paintDevice();
                            if (device) {
                                const QRect dirtyRect = device->extent();

                                KisTransaction transaction(kundo2_i18n("Clear"), device, cmd.data());
                                device->clear();
                                (void) transaction.endAndTake(); // saved as 'parent'

                                node->setDirty(dirtyRect);

                                result = true;
                            }
                        }
                    } else {
                        channel->addKeyframe(time, cmd.data());
                        result = true;
                    }
                }

                // when a new opacity keyframe is created, the opacity is set to 0
                // this makes sure to use the opacity that was previously used
                // maybe there is a better way to do this
                node->setOpacity(originalOpacity);

                return result ? new KisCommandUtils::SkipFirstRedoWrapper(cmd.take()) : nullptr;
        });

        return cmd;
    }

    void createKeyframeLazy(KisImageSP image, KisNodeSP node, const QString &channelId, int time, bool copy)
    {
        KUndo2Command *cmd = createKeyframeCommand(image, node, channelId, time, copy);
        KisProcessingApplicator::runSingleCommandStroke(image, cmd,
                                                        KisStrokeJobData::BARRIER,
                                                        KisStrokeJobData::EXCLUSIVE);
    }

    void removeKeyframes(KisImageSP image, const FrameItemList &frames) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(!image->locked());

        KUndo2Command *cmd = new KisCommandUtils::LambdaCommand(
            kundo2_i18np("Remove Keyframe",
                         "Remove Keyframes",
                         frames.size()),

            [image, frames] () {
                bool result = false;

                QScopedPointer<KUndo2Command> cmd(new KUndo2Command());

                Q_FOREACH (const FrameItem &item, frames) {
                    const int time = item.time;
                    KisNodeSP node = item.node;
                    KisKeyframeChannel *channel = 0;
                    if (node) {
                        channel = node->getKeyframeChannel(item.channel);
                    }
                    if (!channel) continue;

                    KisKeyframeSP keyframe = channel->keyframeAt(time);
                    if (!keyframe) continue;

                    channel->removeKeyframe(time, cmd.data());

                    result = true;
                }

                return result ? new KisCommandUtils::SkipFirstRedoWrapper(cmd.take()) : 0;
        });

        KisProcessingApplicator::runSingleCommandStroke(image, cmd,
                                                        KisStrokeJobData::BARRIER,
                                                        KisStrokeJobData::EXCLUSIVE);
    }

    void removeKeyframe(KisImageSP image, KisNodeSP node, const QString &channel, int time) {
        QVector<FrameItem> frames;
        frames << FrameItem(node, channel, time);
        removeKeyframes(image, frames);
    }


    struct LessOperator {
        LessOperator(const QPoint &offset)
            : m_columnCoeff(-KisAlgebra2D::signPZ(offset.x())),
              m_rowCoeff(-1000000 * KisAlgebra2D::signZZ(offset.y()))
        {
        }

        bool operator()(const QModelIndex &lhs, const QModelIndex &rhs) {
            return
                m_columnCoeff * lhs.column() + m_rowCoeff * lhs.row() <
                m_columnCoeff * rhs.column() + m_rowCoeff * rhs.row();
        }

    private:
        int m_columnCoeff;
        int m_rowCoeff;
    };

    void sortPointsForSafeMove(QModelIndexList *points, const QPoint &offset)
    {
        std::sort(points->begin(), points->end(), LessOperator(offset));
    }

    bool supportsContentFrames(KisNodeSP node)
    {
        return node->inherits("KisPaintLayer") || node->inherits("KisFilterMask") || node->inherits("KisTransparencyMask") || node->inherits("KisSelectionBasedLayer");
    }

    void swapOneFrameItem(const FrameItem &src, const FrameItem &dst, KUndo2Command *parentCommand)
    {
        const int srcTime = src.time;
        KisNodeSP srcNode = src.node;
        KisKeyframeChannel *srcChannel = srcNode->getKeyframeChannel(src.channel);

        const int dstTime = dst.time;
        KisNodeSP dstNode = dst.node;
        KisKeyframeChannel *dstChannel = dstNode->getKeyframeChannel(dst.channel, true);

        if (srcNode == dstNode) {
            if (!srcChannel) return; // TODO: add warning!

            srcChannel->swapKeyframes(srcTime, dstTime, parentCommand);
        } else {
            if (!srcChannel || !dstChannel) return; // TODO: add warning!

            KisKeyframeChannel::swapKeyframes(srcChannel, srcTime, dstChannel, dstTime, parentCommand);
        }
    }

    void moveOneFrameItem(const FrameItem &src, const FrameItem &dst, bool copy, bool moveEmptyFrames, KUndo2Command *parentCommand)
    {
        const int srcTime = src.time;
        KisNodeSP srcNode = src.node;
        KisKeyframeChannel *srcChannel = srcNode->getKeyframeChannel(src.channel);

        const int dstTime = dst.time;
        KisNodeSP dstNode = dst.node;
        KisKeyframeChannel *dstChannel = dstNode->getKeyframeChannel(dst.channel, true);

        if (srcNode == dstNode) {
            if (!srcChannel) return; // TODO: add warning!

            if (srcChannel->keyframeAt(srcTime)) {
                if (copy) {
                    srcChannel->copyKeyframe(srcTime, dstTime, parentCommand);
                } else {
                    srcChannel->moveKeyframe(srcTime, dstTime, parentCommand);
                }
            } else {
                if (srcChannel->keyframeAt(dstTime) && moveEmptyFrames && !copy) {
                    //Destination is effectively replaced by an empty frame.
                    dstChannel->removeKeyframe(dstTime, parentCommand);
                }
            }
        } else {
            if (!srcChannel || !dstChannel) return; // TODO: add warning!

            KisKeyframeSP srcKeyframe = srcChannel->keyframeAt(srcTime);

            if (!srcKeyframe) return; // TODO: add warning!

            KisKeyframeChannel::copyKeyframe(srcChannel, srcTime, dstChannel, dstTime, parentCommand);

            if (!copy) {
                srcChannel->removeKeyframe(srcTime, parentCommand);
            }
        }
    }

    KUndo2Command* createMoveKeyframesCommand(const FrameItemList &srcFrames,
                                              const FrameItemList &dstFrames,
                                              bool copy,
                                              bool moveEmpty,
                                              KUndo2Command *parentCommand)
    {
        FrameMovePairList srcDstPairs;
        for (int i = 0; i < srcFrames.size(); i++) {
            srcDstPairs << std::make_pair(srcFrames[i], dstFrames[i]);
        }
        return createMoveKeyframesCommand(srcDstPairs, copy, moveEmpty, parentCommand);
    }

    KUndo2Command* createMoveKeyframesCommand(const FrameMovePairList &srcDstPairs,
                                              bool copy,
                                              bool moveEmptyFrames,
                                              KUndo2Command *parentCommand)
    {
        KUndo2Command *cmd = new KisCommandUtils::LambdaCommand(

            !copy ?
                kundo2_i18np("Move Keyframe",
                             "Move %1 Keyframes",
                             srcDstPairs.size()) :
                kundo2_i18np("Copy Keyframe",
                             "Copy %1 Keyframes",
                             srcDstPairs.size()),

            parentCommand,

            [srcDstPairs, copy, moveEmptyFrames] () -> KUndo2Command* {
                bool result = false;

                QScopedPointer<KUndo2Command> cmd(new KUndo2Command());

                using MoveChain = QList<FrameItem>;
                QHash<FrameItem, MoveChain> moveMap;
                Q_FOREACH (const FrameMovePair &pair, srcDstPairs) {
                    moveMap.insert(pair.first, {pair.second});
                }

                auto it = moveMap.begin();
                while (it != moveMap.end()) {
                    MoveChain &chain = it.value();
                    const FrameItem &previousFrame = chain.last();

                    auto tailIt = moveMap.find(previousFrame);

                    if (tailIt == it || tailIt == moveMap.end()) {
                        ++it;
                        continue;
                    }

                    chain.append(tailIt.value());
                    tailIt = moveMap.erase(tailIt);
                    // no incrementing! we are going to check the new tail now!
                }

                for (it = moveMap.begin(); it != moveMap.end(); ++it) {
                    MoveChain &chain = it.value();
                    chain.prepend(it.key());
                    KIS_SAFE_ASSERT_RECOVER(chain.size() > 1) { continue; }

                    bool isCycle = false;
                    if (chain.last() == chain.first()) {
                        isCycle = true;
                        chain.takeLast();
                    }

                    auto frameIt = chain.rbegin();

                    FrameItem dstItem = *frameIt++;

                    while (frameIt != chain.rend()) {
                        FrameItem srcItem = *frameIt++;

                        if (!isCycle) {
                            moveOneFrameItem(srcItem, dstItem, copy, moveEmptyFrames, cmd.data());
                        } else {
                            swapOneFrameItem(srcItem, dstItem, cmd.data());
                        }

                        dstItem = srcItem;
                        result = true;
                    }
                }

                return result ? new KisCommandUtils::SkipFirstRedoWrapper(cmd.take()) : nullptr;
        });

        return cmd;
    }

    KUndo2Command* createCloneKeyframesCommand(const FrameMovePairList &srcDstPairs,
                                              KUndo2Command *parentCommand)
    {
        return new KisCommandUtils::LambdaCommand(
                kundo2_i18np("Clone Keyframe",
                             "Clone %1 Keyframes",
                             srcDstPairs.size()),
                parentCommand,
                [srcDstPairs, parentCommand]() -> KUndo2Command*
        {
            QScopedPointer<KUndo2Command> cmd(new KUndo2Command());

            foreach (const FrameMovePair &move, srcDstPairs) {
                KisRasterKeyframeChannel *srcRasterChan = dynamic_cast<KisRasterKeyframeChannel*>(move.first.node->getKeyframeChannel(move.first.channel));
                KisRasterKeyframeChannel *dstRasterChan = dynamic_cast<KisRasterKeyframeChannel*>(move.second.node->getKeyframeChannel(move.second.channel));

                if (!srcRasterChan || !dstRasterChan) {
                    continue;
                }

                if (srcRasterChan == dstRasterChan) {
                    srcRasterChan->cloneKeyframe(move.first.time, move.second.time, cmd.data());
                } else {
                    KisKeyframeChannel::copyKeyframe(srcRasterChan, move.first.time, dstRasterChan, move.second.time, cmd.data());
                }
            }

            return cmd.take();
        });
    }

    void makeClonesUnique(KisImageSP image, const FrameItemList &frames)
    {
        KUndo2Command* cmd = new KisCommandUtils::LambdaCommand(
                    kundo2_i18n("Make clones Unique"),
                    [frames]() {
            QScopedPointer<KUndo2Command> cmd(new KUndo2Command());

            foreach (const FrameItem &frameItem, frames) {
                KisRasterKeyframeChannel *rasterChan = dynamic_cast<KisRasterKeyframeChannel*>(frameItem.node->getKeyframeChannel(frameItem.channel));
                if (!rasterChan) {
                    continue;
                }

                rasterChan->makeUnique(frameItem.time, cmd.data());
            }

            return cmd.take();
        });

        KisProcessingApplicator::runSingleCommandStroke(image, cmd,
                                                        KisStrokeJobData::BARRIER,
                                                        KisStrokeJobData::EXCLUSIVE);
    }

    QDebug operator<<(QDebug dbg, const FrameItem &item)
    {
        dbg.nospace() << "FrameItem(" << item.node->name() << ", " << item.channel << ", " << item.time << ")";
        return dbg.space();
    }
}

