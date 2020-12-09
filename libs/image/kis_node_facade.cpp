/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_node_facade.h"
#include "kis_node_graph_listener.h"
#include <kis_debug.h>

struct Q_DECL_HIDDEN KisNodeFacade::Private
{
public:
    KisNodeWSP root;
};

KisNodeFacade::KisNodeFacade()
        : m_d(new Private())
{
}

KisNodeFacade::KisNodeFacade(KisNodeSP root)
        : m_d(new Private())
{
    m_d->root = root;
}

KisNodeFacade::~KisNodeFacade()
{
}

void KisNodeFacade::setRoot(KisNodeSP root)
{
    m_d->root = root;
}

const KisNodeSP KisNodeFacade::root() const
{
    return m_d->root;
}

bool KisNodeFacade::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    dbgImage << "moveNode " << node << " " << parent << " " << aboveThis;
    if (!node) {
        dbgImage << "cannot move null node"; return false;
    }
    if (!parent)  {
        dbgImage << "cannot move to null parent"; return false;
    }
    if (node == parent)  {
        dbgImage << "cannot move self inside self"; return false;
    }
    if (node == aboveThis)  {
        dbgImage << "cannot move self above self"; return false;
    }
    if (parent == aboveThis)  {
        dbgImage << "cannot move above parent"; return false;
    }
    if (!node->parent())  {
        dbgImage << "node does not have a parent"; return false;
    }

    if (aboveThis && aboveThis->parent() != parent)  {
        dbgImage << "above this parent is not the parent"; return false;
    }

    int newIndex = aboveThis ? parent->index(aboveThis) + 1 : 0;
    return moveNode(node, parent, newIndex);
}

bool KisNodeFacade::moveNode(KisNodeSP node, KisNodeSP parent, quint32 newIndex)
{
    dbgImage << "moveNode " << node << " " << parent << " " << newIndex;
    int oldIndex = node->parent()->index(node);

    if (node->graphListener())
        node->graphListener()->aboutToMoveNode(node.data(), oldIndex, newIndex);
    KisNodeSP aboveThis = parent->at(newIndex - 1);
    if (aboveThis == node) return false;
    if (node->parent()) {
        if (!node->parent()->remove(node)) return false;
    }
    dbgImage << "moving node to " << newIndex;
    bool success = addNode(node, parent, aboveThis);
    if (node->graphListener())
        node->graphListener()->nodeHasBeenMoved(node.data(), oldIndex, newIndex);
    return success;
}


bool KisNodeFacade::addNode(KisNodeSP node, KisNodeSP parent)
{
    dbgImage << "Add node " << node << " to " << parent;
    if (!node) return false;
    if (!parent && !m_d->root) return false;

    if (parent)
        return parent->add(node, parent->lastChild());
    else
        return m_d->root->add(node, m_d->root->lastChild());
}

bool KisNodeFacade::addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    if (!node) return false;
    if (!parent) return false;

    return parent->add(node, aboveThis);
}

bool KisNodeFacade::addNode(KisNodeSP node,  KisNodeSP parent, quint32 index)
{
    if (!node) return false;
    if (!parent) return false;

    if (index == parent->childCount())
        return parent->add(node, parent->lastChild());
    else if (index != 0)
        return parent->add(node, parent->at(index));
    else
        return parent->add(node, 0);
}

bool KisNodeFacade::removeNode(KisNodeSP node)
{
    if (!node) return false;
    if (!node->parent()) return false;

    return node->parent()->remove(node);

}

bool KisNodeFacade::raiseNode(KisNodeSP node)
{
    if (!node) return false;
    if (!node->parent()) return false;

    if (node->nextSibling())
        return moveNode(node, node->parent(), node->nextSibling());
    else
        return true; // we're already at the top, but there is no
    // sense in complaining.

}

bool KisNodeFacade::lowerNode(KisNodeSP node)
{
    if (!node) return false;
    if (!node->parent()) return false;

    KisNodeSP parent = node->parent();
    KisNodeSP prevSibling = node->prevSibling();

    if (node->prevSibling()) {
        int prevIndex = parent->index(prevSibling);
        return moveNode(node, parent, prevIndex);
    } else {
        return true; // We're already at bottom, but there's no sense
        // in complaining
    }
}

bool KisNodeFacade::toTop(KisNodeSP node)
{
    if (!node) return false;
    if (!node->parent()) return false;
    if (node == node->parent()->lastChild()) return true;

    return moveNode(node, node->parent(), node->parent()->lastChild());

}

bool KisNodeFacade::toBottom(KisNodeSP node)
{
    if (!node) return false;
    if (!node->parent()) return false;

    KisNodeSP parent = node->parent();

    if (node == parent->firstChild()) return true;

    // Sets the parent of this node to 0
    if (!parent->remove(node)) return false;

    return parent->add(node, 0);

}
