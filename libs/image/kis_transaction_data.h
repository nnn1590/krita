/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_TRANSACTION_DATA_H_
#define KIS_TRANSACTION_DATA_H_

#include <kundo2command.h>
#include "kis_types.h"
#include <kritaimage_export.h>


/**
 * A tile based undo command.
 *
 * Ordinary KUndo2Command subclasses store parameters and apply the action in
 * the redo() command, however, Krita doesn't work like this. Undo replaces
 * the current tiles in a paint device with the old tiles, redo replaces them
 * again with the new tiles without actually executing the command that changed
 * the image data again.
 */
class KRITAIMAGE_EXPORT KisTransactionData : public KUndo2Command
{
public:
    KisTransactionData(const KUndo2MagicString& name, KisPaintDeviceSP device, bool resetSelectionOutlineCache, KUndo2Command* parent);
    ~KisTransactionData() override;

public:
    void redo() override;
    void undo() override;

    virtual void endTransaction();

protected:
    virtual void saveSelectionOutlineCache();
    virtual void restoreSelectionOutlineCache(bool undo);

private:
    void init(KisPaintDeviceSP device);
    void startUpdates();
    void possiblyNotifySelectionChanged();
    void possiblyResetOutlineCache();
    void possiblyFlattenSelection(KisPaintDeviceSP device);
    void doFlattenUndoRedo(bool undo);

private:
    class Private;
    Private * const m_d;
};

#endif /* KIS_TRANSACTION_DATA_H_ */

