/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PROCESSING_APPLICATOR_H
#define __KIS_PROCESSING_APPLICATOR_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include "kis_stroke_job_strategy.h"
#include "KisImageSignals.h"
#include "kundo2magicstring.h"
#include "kundo2commandextradata.h"


class KRITAIMAGE_EXPORT KisProcessingApplicator
{
public:
    enum ProcessingFlag {
        NONE = 0x0,
        RECURSIVE = 0x1,
        NO_UI_UPDATES = 0x2,
        SUPPORTS_WRAPAROUND_MODE = 0x4,
        NO_IMAGE_UPDATES = 0x8
    };

    Q_DECLARE_FLAGS(ProcessingFlags, ProcessingFlag)

public:
    KisProcessingApplicator(KisImageWSP image,
                            KisNodeSP node,
                            ProcessingFlags flags = NONE,
                            KisImageSignalVector emitSignals = KisImageSignalVector(),
                            const KUndo2MagicString &name = KUndo2MagicString(),
                            KUndo2CommandExtraData *extraData = 0,
                            int macroId = -1);

    ~KisProcessingApplicator();

    void applyVisitor(KisProcessingVisitorSP visitor,
                      KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    void applyCommand(KUndo2Command *command,
                      KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                      KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    void applyVisitorAllFrames(KisProcessingVisitorSP visitor,
                               KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                               KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    /**
     * This method emits all the final update signals of the stroke
     * without actually ending the stroke. This can be used for
     * long-running strokes which are kept open to implement preview
     * of the actions.
     *
     * WARNING: you cannot add new commands/processings after the
     * final signals has been emitted. You should either call end() or
     * cancel().
     */
    void explicitlyEmitFinalSignals();

    void end();
    void cancel();

    /**
     * Returns the underlying stroke.
     * Useful if you want to inject additional jobs in the applicator.
     */
    const KisStrokeId getStroke() const;

    /**
     * @brief runSingleCommandStroke creates a stroke and runs \p cmd in it.
     *        The text() field fo \p cmd is used as a title of the stroke.
     * @param image the image to run the stroke on
     * @param cmd the command to be executed
     * @param sequentiality sequentiality property of the command being executed (see strokes documentation)
     * @param exclusivity sequentiality property of the command being executed (see strokes documentation)
     */
    static void runSingleCommandStroke(KisImageSP image,
                                       KUndo2Command *cmd,
                                       KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                                       KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

private:
    void visitRecursively(KisNodeSP node,
                          KisProcessingVisitorSP visitor,
                          KisStrokeJobData::Sequentiality sequentiality,
                          KisStrokeJobData::Exclusivity exclusivity);

private:
    KisImageWSP m_image;
    KisNodeSP m_node;
    ProcessingFlags m_flags;
    KisImageSignalVector m_emitSignals;
    KisStrokeId m_strokeId;
    bool m_finalSignalsEmitted;
    QSharedPointer<bool> m_sharedAllFramesToken;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisProcessingApplicator::ProcessingFlags)


#endif /* __KIS_PROCESSING_APPLICATOR_H */
