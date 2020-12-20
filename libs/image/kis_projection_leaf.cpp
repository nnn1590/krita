/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_projection_leaf.h"

#include <KoColorSpace.h>

#include "kis_layer.h"
#include "kis_image.h"
#include "kis_mask.h"
#include "kis_group_layer.h"
#include "kis_selection_mask.h"
#include "kis_adjustment_layer.h"

#include "krita_utils.h"

#include "kis_refresh_subtree_walker.h"
#include "kis_async_merger.h"
#include "kis_node_graph_listener.h"
#include "kis_clone_layer.h"


struct Q_DECL_HIDDEN KisProjectionLeaf::Private
{
    Private(KisNode *_node) : node(_node) {}

    KisNodeWSP node;
    bool isTemporaryHidden = false;

    static bool checkPassThrough(const KisNode *node) {
        const KisGroupLayer *group = qobject_cast<const KisGroupLayer*>(node);
        return group && group->passThroughMode();
    }

    static bool isSelectionMask(const KisNode *node) {
        return qobject_cast<const KisSelectionMask*>(node);
    }

    static KisNodeSP skipSelectionMasksForward(KisNodeSP node) {
        while (node && isSelectionMask(node)) {
            node = node->nextSibling();
        }
        return node;
    }

    static KisNodeSP skipSelectionMasksBackward(KisNodeSP node) {
        while (node && isSelectionMask(node)) {
            node = node->prevSibling();
        }
        return node;
    }

    bool checkParentPassThrough() {
        return node->parent() && checkPassThrough(node->parent());
    }

    bool checkThisPassThrough() {
        return checkPassThrough(node);
    }

    KisProjectionLeafSP overlayProjectionLeaf() const {
        return node && node->graphListener() && node->graphListener()->graphOverlayNode() ?
            node->graphListener()->graphOverlayNode()->projectionLeaf() : 0;
    }

    bool isTopmostNode() const {
        return !skipSelectionMasksForward(node->nextSibling()) &&
            node->parent() &&
            !node->parent()->parent();
    }

    KisNodeSP findRoot() const {
        KisNodeSP root = node;

        while (root->parent()) {
            root = root->parent();
        }

        return root;
    }

    void temporarySetPassThrough(bool value) {
        KisGroupLayer *group = qobject_cast<KisGroupLayer*>(node.data());
        if (!group) return;

        group->setPassThroughMode(value);
    }
};

KisProjectionLeaf::KisProjectionLeaf(KisNode *node)
    : m_d(new Private(node))
{
}

KisProjectionLeaf::~KisProjectionLeaf()
{
}

KisProjectionLeafSP KisProjectionLeaf::parent() const
{
    KisNodeSP node;

    if (Private::isSelectionMask(m_d->node)) {
        if (m_d->overlayProjectionLeaf() == this) {
            node = m_d->findRoot();
        }
    } else {
        node = m_d->node->parent();
    }

    while (node && Private::checkPassThrough(node)) {
        node = node->parent();
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}


KisProjectionLeafSP KisProjectionLeaf::firstChild() const
{
    KisNodeSP node;

    if (!m_d->checkThisPassThrough()) {
        node = m_d->node->firstChild();
        node = Private::skipSelectionMasksForward(node);
    }

    if (!node && isRoot()) {
        KisProjectionLeafSP overlayLeaf = m_d->overlayProjectionLeaf();
        if (overlayLeaf) {
            return overlayLeaf;
        }
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisProjectionLeafSP KisProjectionLeaf::lastChild() const
{
    KisNodeSP node;

    if (isRoot()) {
        KisProjectionLeafSP overlayLeaf = m_d->overlayProjectionLeaf();
        if (overlayLeaf) {
            return overlayLeaf;
        }
    }

    if (!m_d->checkThisPassThrough()) {
        node = m_d->node->lastChild();
        node = Private::skipSelectionMasksBackward(node);
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisProjectionLeafSP KisProjectionLeaf::prevSibling() const
{
    if (Private::isSelectionMask(m_d->node)) {
        KisProjectionLeafSP leaf;

        if (m_d->overlayProjectionLeaf() == this) {
            KisNodeSP node = m_d->findRoot()->lastChild();
            node = Private::skipSelectionMasksBackward(node);
            leaf = node->projectionLeaf();
        }

        return leaf;
    }

    KisNodeSP node;

    if (m_d->checkThisPassThrough()) {
        node = m_d->node->lastChild();
        node = Private::skipSelectionMasksBackward(node);
    }

    if (!node) {
        node = m_d->node->prevSibling();
        node = Private::skipSelectionMasksBackward(node);
    }

    const KisProjectionLeaf *leaf = this;
    while (!node && leaf->m_d->checkParentPassThrough()) {
        leaf = leaf->node()->parent()->projectionLeaf().data();
        node = leaf->node()->prevSibling();
        node = Private::skipSelectionMasksBackward(node);
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisProjectionLeafSP KisProjectionLeaf::nextSibling() const
{
    if (Private::isSelectionMask(m_d->node)) {
        return KisProjectionLeafSP();
    }

    KisProjectionLeafSP overlayLeaf = m_d->overlayProjectionLeaf();
    if (overlayLeaf && m_d->isTopmostNode()) {
        return overlayLeaf;
    }

    KisNodeSP node = m_d->node->nextSibling();
    node = Private::skipSelectionMasksForward(node);

    while (node && Private::checkPassThrough(node) && node->firstChild()) {
        node = node->firstChild();
        node = Private::skipSelectionMasksForward(node);
    }

    if (!node && m_d->checkParentPassThrough()) {
        node = m_d->node->parent();
        node = Private::skipSelectionMasksForward(node);
    }

    return node ? node->projectionLeaf() : KisProjectionLeafSP();
}

KisNodeSP KisProjectionLeaf::node() const
{
    return m_d->node;
}

KisAbstractProjectionPlaneSP KisProjectionLeaf::projectionPlane() const
{
    return m_d->node->projectionPlane();
}

bool KisProjectionLeaf::accept(KisNodeVisitor &visitor)
{
    return m_d->node->accept(visitor);
}

KisPaintDeviceSP KisProjectionLeaf::original()
{
    return m_d->node->original();
}

KisPaintDeviceSP KisProjectionLeaf::projection()
{
    return m_d->node->projection();
}

bool KisProjectionLeaf::isRoot() const
{
    return (bool)!m_d->node->parent();
}

bool KisProjectionLeaf::isLayer() const
{
    return (bool)qobject_cast<const KisLayer*>(m_d->node.data()) &&
        !m_d->node->isFakeNode();
}

bool KisProjectionLeaf::isMask() const
{
    return (bool)qobject_cast<const KisMask*>(m_d->node.data()) &&
        !m_d->node->isFakeNode();
}

bool KisProjectionLeaf::canHaveChildLayers() const
{
    return (bool)qobject_cast<const KisGroupLayer*>(m_d->node.data());
}

bool KisProjectionLeaf::dependsOnLowerNodes() const
{
    return (bool)qobject_cast<const KisAdjustmentLayer*>(m_d->node.data());
}

bool KisProjectionLeaf::visible() const
{
    if (m_d->isTemporaryHidden || isDroppedNode()) return false;

    // TODO: check opacity as well!

    bool hiddenByParentPassThrough = false;

    KisNodeSP node = m_d->node->parent();
    while (node && node->projectionLeaf()->m_d->checkThisPassThrough()) {
        hiddenByParentPassThrough |= !node->visible();
        node = node->parent();
    }

    return (m_d->node->visible(false) || m_d->node->isIsolatedRoot()) &&
        !m_d->checkThisPassThrough() &&
        !hiddenByParentPassThrough;
}

quint8 KisProjectionLeaf::opacity() const
{
    quint8 resultOpacity = m_d->node->opacity();

    if (m_d->checkParentPassThrough()) {
        quint8 parentOpacity = m_d->node->parent()->projectionLeaf()->opacity();

        resultOpacity = KritaUtils::mergeOpacity(resultOpacity, parentOpacity);
    }

    return resultOpacity;
}

QBitArray KisProjectionLeaf::channelFlags() const
{
    QBitArray channelFlags;

    KisLayer *layer = qobject_cast<KisLayer*>(m_d->node.data());
    if (!layer) return channelFlags;

    channelFlags = layer->channelFlags();

    if (m_d->checkParentPassThrough()) {
        QBitArray parentChannelFlags;

        if (*m_d->node->colorSpace() ==
            *m_d->node->parent()->colorSpace()) {

            KisLayer *parentLayer = qobject_cast<KisLayer*>(m_d->node->parent().data());
            parentChannelFlags = parentLayer->channelFlags();
        }

        channelFlags = KritaUtils::mergeChannelFlags(channelFlags, parentChannelFlags);
    }

    return channelFlags;
}

bool KisProjectionLeaf::isStillInGraph() const
{
    return (bool)m_d->node->graphListener();
}

bool KisProjectionLeaf::hasClones() const
{
    KisLayer *layer = qobject_cast<KisLayer*>(m_d->node.data());
    return layer ? layer->hasClones() : false;
}

bool KisProjectionLeaf::isDroppedNode() const
{
    return dropReason() != NodeAvailable;
}

KisProjectionLeaf::NodeDropReason KisProjectionLeaf::dropReason() const
{
    if (qobject_cast<KisMask*>(m_d->node.data()) &&
            m_d->checkParentPassThrough()) {

        return DropPassThroughMask;
    }

    KisCloneLayer *cloneLayer = qobject_cast<KisCloneLayer*>(m_d->node.data());
    if (cloneLayer && cloneLayer->copyFrom()) {
        KisProjectionLeafSP leaf = cloneLayer->copyFrom()->projectionLeaf();

        if (leaf->m_d->checkThisPassThrough()) {
            return DropPassThroughClone;
        }
    }

    return NodeAvailable;
}

bool KisProjectionLeaf::isOverlayProjectionLeaf() const
{
    return this == m_d->overlayProjectionLeaf();
}

void KisProjectionLeaf::setTemporaryHiddenFromRendering(bool value)
{
    m_d->isTemporaryHidden = value;
}

bool KisProjectionLeaf::isTemporaryHiddenFromRendering() const
{
    return m_d->isTemporaryHidden;
}

/**
 * This method is rather slow and dangerous. It should be executes in
 * exclusive environment only.
 */
void KisProjectionLeaf::explicitlyRegeneratePassThroughProjection()
{
    if (!m_d->checkThisPassThrough()) return;

    m_d->temporarySetPassThrough(false);

    const QRect updateRect = projection()->defaultBounds()->bounds();

    KisRefreshSubtreeWalker walker(updateRect);
    walker.collectRects(m_d->node, updateRect);

    KisAsyncMerger merger;
    merger.startMerge(walker);

    m_d->temporarySetPassThrough(true);
}
