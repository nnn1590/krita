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

#include "move_stroke_test.h"

#include <QTest>

#include "stroke_testing_utils.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "strokes/move_stroke_strategy.h"


class MoveStrokeTester : public utils::StrokeTester
{
public:
    MoveStrokeTester()
        : StrokeTester("move", QSize(512, 512), "")
    {
    }

protected:
    using utils::StrokeTester::initImage;
    void initImage(KisImageWSP image, KisNodeSP activeNode) override {
        Q_UNUSED(image);

        QImage src(QString(FILES_DATA_DIR) + '/' + "carrot.png");
        activeNode->original()->convertFromQImage(src, 0);
    }

    KisStrokeStrategy* createStroke(KisResourcesSnapshotSP resources,
                                    KisImageWSP image) override {

        KisNodeSP node = resources->currentNode();
        return new MoveStrokeStrategy({node}, image.data(), image.data());
    }

    using utils::StrokeTester::addPaintingJobs;
    void addPaintingJobs(KisImageWSP image,
                         KisResourcesSnapshotSP resources) override {

        Q_UNUSED(resources);

        image->
            addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(100,100)));

        image->
            addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(50, 50)));

        for (int i = 0; i < 25; i++) {
            image->
                addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(50+i,50)));
            QTest::qSleep(1);
            image->
                addJob(strokeId(), new MoveStrokeStrategy::Data(QPoint(50+i,50+i)));

            QTest::qSleep(1);
        }
    }

private:

};


void MoveStrokeTest::testMoveStroke()
{
    MoveStrokeTester tester;
    tester.test();
}

QTEST_GUILESS_MAIN(MoveStrokeTest)
