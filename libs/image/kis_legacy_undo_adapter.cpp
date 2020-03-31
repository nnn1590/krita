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

#include "kis_legacy_undo_adapter.h"

#include "kis_image.h"


KisLegacyUndoAdapter::KisLegacyUndoAdapter(KisUndoStore *undoStore,
                                           KisImageWSP image)
    : KisUndoAdapter(undoStore, image.data()),
      m_image(image),
      m_macroCounter(0)
{
}

const KUndo2Command* KisLegacyUndoAdapter::presentCommand()
{
    return undoStore()->presentCommand();
}

void KisLegacyUndoAdapter::undoLastCommand()
{
    undoStore()->undoLastCommand();
}

void KisLegacyUndoAdapter::addCommand(KUndo2Command *command)
{
    if(!command) return;

    if(m_macroCounter) {
        undoStore()->addCommand(command);
    }
    else {
        m_image->barrierLock();
        undoStore()->addCommand(command);
        m_image->unlock();

        /// Sometimes legacy commands forget to emit sigImageModified() signal,
        /// it causes dockers to be updated in correctly. Let's help them.
        m_image->setModified();
    }
}

void KisLegacyUndoAdapter::beginMacro(const KUndo2MagicString& macroName)
{
    if(!m_macroCounter) {
        m_image->barrierLock();
    }

    m_macroCounter++;
    undoStore()->beginMacro(macroName);
}

void KisLegacyUndoAdapter::endMacro()
{
    m_macroCounter--;

    if(!m_macroCounter) {
        m_image->unlock();
    }
    undoStore()->endMacro();

    /// Sometimes legacy commands forget to emit sigImageModified() signal,
    /// it causes dockers to be updated in correctly. Let's help them.
    m_image->setModified();
}

