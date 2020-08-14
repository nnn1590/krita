/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PRESETHISTORY_DOCK_H_
#define _PRESETHISTORY_DOCK_H_

#include <QDockWidget>
#include <QPointer>
#include <KisKineticScroller.h>

#include <KoCanvasObserverBase.h>

#include <kis_canvas2.h>
#include <kis_types.h>

class QListWidget;
class QListWidgetItem;
class QActionGroup;

class PresetHistoryDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    enum HistoryDataRole {
        BrushPresetRole = Qt::UserRole,
        BubbleMarkerRole = Qt::UserRole + 1
    };

    enum DisplayOrder {
        Static = 0,
        MostRecent = 1,
        Bubbling = 2
    };

    PresetHistoryDock();
    QString observerName() override { return "PresetHistoryDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }
private Q_SLOTS:
    void presetSelected(QListWidgetItem* item);
    void canvasResourceChanged(int key, const QVariant& v);
    void slotSortingModeChanged(QAction *action);
    void slotContextMenuRequest(const QPoint &pos);
private:
    void sortPresets(int position);
    int bubblePreset(int position);
    void addPreset(KisPaintOpPresetSP preset);
private:
    QPointer<KisCanvas2> m_canvas;
    QListWidget *m_presetHistory;
    QAction *m_actionSortStatic;
    QAction *m_actionSortMostRecent;
    QAction *m_actionSortBubble;
    QActionGroup *m_sortingModes;
    DisplayOrder m_sorting {Static};
    bool m_block {false};
    bool m_initialized {false};
};


#endif
