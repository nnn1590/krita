/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __STROKE_TESTING_UTILS_H
#define __STROKE_TESTING_UTILS_H

#include <QString>
#include <KoCanvasResourceProvider.h>
#include "kis_node.h"
#include "kis_types.h"
#include "kis_stroke_strategy.h"
#include "kis_resources_snapshot.h"


class KisUndoStore;


namespace utils {

    KisImageSP createImage(KisUndoStore *undoStore, const QSize &imageSize);
    KoCanvasResourceProvider* createResourceManager(KisImageWSP image,
                                             KisNodeSP node = 0,
                                             const QString &presetFileName = "autobrush_300px.kpp");

    class StrokeTester
    {
    public:
        StrokeTester(const QString &name, const QSize &imageSize, const QString &presetFileName = "autobrush_300px.kpp");
        virtual ~StrokeTester();

        void testSimpleStroke();
        void test();
        void benchmark();
        void testSimpleStrokeNoVerification();

        void setNumIterations(int value);
        void setBaseFuzziness(int value);

        int lastStrokeTime() const;

    protected:
        KisStrokeId strokeId() {
            return m_strokeId;
        }

        virtual void modifyResourceManager(KoCanvasResourceProvider *manager,
                                           KisImageWSP image, int iteration);

        virtual void initImage(KisImageWSP image, KisNodeSP activeNode, int iteration);

        // overload
        virtual void modifyResourceManager(KoCanvasResourceProvider *manager,
                                           KisImageWSP image);

        // overload
        virtual void initImage(KisImageWSP image, KisNodeSP activeNode);
        virtual void beforeCheckingResult(KisImageWSP image, KisNodeSP activeNode);

        virtual KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                                KisImageWSP image) = 0;

        virtual void addPaintingJobs(KisImageWSP image,
                                     KisResourcesSnapshotSP resources,
                                     int iteration);

        // overload
        virtual void addPaintingJobs(KisImageWSP image,
                                     KisResourcesSnapshotSP resources);

    private:
        void testOneStroke(bool cancelled, bool indirectPainting,
                           bool externalLayer, bool testUpdates = false);

        QImage doStroke(bool cancelled,
                        bool externalLayer, bool testUpdates = false,
                        bool needQImage = true);

        QString formatTestName(const QString &baseName, bool cancelled,
                               bool indirectPainting, bool externalLayer);
        QString referenceFile(const QString &testName);
        QString dumpReferenceFile(const QString &testName);
        QString resultFile(const QString &testName);

    private:
        KisStrokeId m_strokeId;
        QString m_name;
        QSize m_imageSize;
        QString m_presetFilename;
        int m_numIterations;
        int m_baseFuzziness;
        int m_strokeTime = 0;
    };
}

#endif /* __STROKE_TESTING_UTILS_H */
