/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#include <klocalizedstring.h>
#include <KoCompositeOp.h>
#include "kis_node.h"
#include "commands/kis_node_commands.h"
#include "kis_command_ids.h"

KisNodeCompositeOpCommand::KisNodeCompositeOpCommand(KisNodeSP node, const QString& oldCompositeOp,
        const QString& newCompositeOp) :
        KisNodeCommand(kundo2_i18n("Composition Mode Change"), node)
{
    m_oldCompositeOp = oldCompositeOp;
    m_newCompositeOp = newCompositeOp;
}

void KisNodeCompositeOpCommand::setCompositeOpImpl(const QString &compositeOp)
{
    /**
     * The node might have had "Destination Atop" blending
     * that changes extent of the layer
     */
    const QRect oldExtent = m_node->extent();
    m_node->setCompositeOpId(compositeOp);
    m_node->setDirty(oldExtent | m_node->extent());
}

void KisNodeCompositeOpCommand::redo()
{
    setCompositeOpImpl(m_newCompositeOp);
}

void KisNodeCompositeOpCommand::undo()
{
    setCompositeOpImpl(m_oldCompositeOp);
}

int KisNodeCompositeOpCommand::id() const
{
    return KisCommandUtils::ChangeNodeCompositeOpId;
}

bool KisNodeCompositeOpCommand::mergeWith(const KUndo2Command *command)
{
    const KisNodeCompositeOpCommand *other =
        dynamic_cast<const KisNodeCompositeOpCommand*>(command);

    if (other && other->m_node == m_node) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_newCompositeOp == other->m_oldCompositeOp);
        m_newCompositeOp = other->m_newCompositeOp;
        return true;
    }

    return false;
}

bool KisNodeCompositeOpCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisNodeCompositeOpCommand *other =
        dynamic_cast<const KisNodeCompositeOpCommand*>(command);

    return other && other->m_node == m_node;
}
