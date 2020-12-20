/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_model_index_converter.h"

#include "kis_selection_mask.h"
#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "kis_node_model.h"
#include "kis_node_manager.h"
#include "KisReferenceImagesLayer.h"
#include "KisDecorationsWrapperLayer.h"


KisModelIndexConverter::KisModelIndexConverter(KisDummiesFacadeBase *dummiesFacade,
                                               KisNodeModel *model,
                                               bool showGlobalSelection)
    : m_dummiesFacade(dummiesFacade),
      m_model(model),
      m_showGlobalSelection(showGlobalSelection)
{
}

inline bool KisModelIndexConverter::checkDummyType(KisNodeDummy *dummy)
{
    return dummy->isGUIVisible(m_showGlobalSelection);
}

inline bool KisModelIndexConverter::checkDummyMetaObjectType(const QString &type)
{
    const QString selectionMaskType = KisSelectionMask::staticMetaObject.className();
    const QString referencesLayerType = KisReferenceImagesLayer::staticMetaObject.className();
    const QString decorationsLayerType = KisDecorationsWrapperLayer::staticMetaObject.className();

    return (type != selectionMaskType || m_showGlobalSelection) &&
            type != referencesLayerType &&
            type != decorationsLayerType;
}

KisNodeDummy* KisModelIndexConverter::dummyFromRow(int row, QModelIndex parent)
{

    KisNodeDummy *parentDummy = parent.isValid() ?
        dummyFromIndex(parent) : m_dummiesFacade->rootDummy();

    if(!parentDummy) return 0;

    KisNodeDummy *resultDummy = 0;

    // a child of the root node
    if(!parentDummy->parent()) {
        KisNodeDummy *currentDummy = parentDummy->lastChild();
        while(currentDummy) {
            if(checkDummyType(currentDummy)) {
                if(!row) {
                    resultDummy = currentDummy;
                    break;
                }
                row--;
            }
            currentDummy = currentDummy->prevSibling();
        }
    }
    // a child of other layer
    else {
        int rowCount = parentDummy->childCount();
        int index = rowCount - row - 1;
        resultDummy = parentDummy->at(index);
    }


    return resultDummy;
}

KisNodeDummy* KisModelIndexConverter::dummyFromIndex(QModelIndex index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.internalPointer());
    return static_cast<KisNodeDummy*>(index.internalPointer());
}

QModelIndex KisModelIndexConverter::indexFromDummy(KisNodeDummy *dummy)
{
    Q_ASSERT(dummy);
    KisNodeDummy *parentDummy = dummy->parent();

    // a root node
    if(!parentDummy) return QModelIndex();

    int row = 0;

    // a child of the root node
    if(!parentDummy->parent()) {
        if(!checkDummyType(dummy)) return QModelIndex();

        KisNodeDummy *currentDummy = parentDummy->lastChild();
        while(currentDummy && currentDummy != dummy) {
            if(checkDummyType(currentDummy)) {
                row++;
            }
            currentDummy = currentDummy->prevSibling();
        }
    }
    // a child of other layer
    else {
        int rowCount = parentDummy->childCount();
        int index = parentDummy->indexOf(dummy);
        row = rowCount - index - 1;
    }

    return m_model->createIndex(row, 0, (void*)dummy);
}

bool KisModelIndexConverter::indexFromAddedDummy(KisNodeDummy *parentDummy,
                                                 int index,
                                                 const QString &newNodeMetaObjectType,
                                                 QModelIndex &parentIndex,
                                                 int &row)
{
    // adding a root node
    if(!parentDummy) {
        Q_ASSERT(!index);
        return false;
    }

    // adding a child of the root node
    if(!parentDummy->parent()) {
        if(!checkDummyMetaObjectType(newNodeMetaObjectType)) {
            return false;
        }

        row = 0;

        parentIndex = QModelIndex();
        KisNodeDummy *dummy = parentDummy->lastChild();
        int toScan = parentDummy->childCount() - index;
        while(dummy && toScan > 0) {
            if(checkDummyType(dummy)) {
                row++;
            }
            dummy = dummy->prevSibling();
            toScan--;
        }
    }
    // everything else
    else {
        parentIndex = indexFromDummy(parentDummy);
        int rowCount = parentDummy->childCount();
        row = rowCount - index;
    }

    return true;
}

int KisModelIndexConverter::rowCount(QModelIndex parent)
{
    KisNodeDummy *dummy = parent.isValid() ?
        dummyFromIndex(parent) : m_dummiesFacade->rootDummy();

    // a root node (hidden)
    if(!dummy) return 0;

    int numChildren = 0;

    // children of the root node
    if(!dummy->parent()) {
        KisNodeDummy *currentDummy = dummy->lastChild();
        while(currentDummy) {
            if(checkDummyType(currentDummy)) {
                numChildren++;
            }

            currentDummy = currentDummy->prevSibling();
        }
    }
    // children of other nodes
    else {
        numChildren = dummy->childCount();
    }

    return numChildren;
}
