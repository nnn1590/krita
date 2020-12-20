/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolChangesTracker.h"

#include "kis_global.h"
#include <QSharedPointer>

struct KisToolChangesTracker::Private {
    QList<KisToolChangesTrackerDataSP> undoStack;
};


KisToolChangesTracker::KisToolChangesTracker()
    : m_d(new Private)
{
}

KisToolChangesTracker::~KisToolChangesTracker()
{
}

void KisToolChangesTracker::commitConfig(KisToolChangesTrackerDataSP state)
{
    m_d->undoStack.append(state);
}

void KisToolChangesTracker::requestUndo()
{
    if (m_d->undoStack.isEmpty()) return;

    m_d->undoStack.removeLast();
    if (!m_d->undoStack.isEmpty()) {
        emit sigConfigChanged(m_d->undoStack.last());
    }
}

KisToolChangesTrackerDataSP KisToolChangesTracker::lastState() const
{
    return !m_d->undoStack.isEmpty() ? m_d->undoStack.last() : static_cast<QSharedPointer<KisToolChangesTrackerData>>(0);
}

void KisToolChangesTracker::reset()
{
    m_d->undoStack.clear();
}

bool KisToolChangesTracker::isEmpty() const
{
    return m_d->undoStack.isEmpty();
}
