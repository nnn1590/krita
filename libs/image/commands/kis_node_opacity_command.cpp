/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <klocalizedstring.h>
#include "kis_node.h"
#include "commands/kis_node_opacity_command.h"
#include "kis_command_ids.h"

KisNodeOpacityCommand::KisNodeOpacityCommand(KisNodeSP node, quint8 oldOpacity, quint8 newOpacity) :
        KisNodeCommand(kundo2_i18n("Opacity Change"), node)
{
    m_oldOpacity = oldOpacity;
    m_newOpacity = newOpacity;
}

void KisNodeOpacityCommand::redo()
{
    m_node->setOpacity(m_newOpacity);
    m_node->setDirty();
}

void KisNodeOpacityCommand::undo()
{
    m_node->setOpacity(m_oldOpacity);
    m_node->setDirty();
}

int KisNodeOpacityCommand::id() const
{
    return KisCommandUtils::ChangeNodeOpacityId;
}

bool KisNodeOpacityCommand::mergeWith(const KUndo2Command *command)
{
    const KisNodeOpacityCommand *other =
        dynamic_cast<const KisNodeOpacityCommand*>(command);

    if (other && other->m_node == m_node) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_newOpacity == other->m_oldOpacity);
        m_newOpacity = other->m_newOpacity;
        return true;
    }

    return false;
}

bool KisNodeOpacityCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisNodeOpacityCommand *other =
        dynamic_cast<const KisNodeOpacityCommand*>(command);

    return other && other->m_node == m_node;
}
