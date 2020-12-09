/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef KIS_COLOR_SELECTOR_CONTAINER_H
#define KIS_COLOR_SELECTOR_CONTAINER_H

#include <QWidget>
#include <QPointer>
#include <kis_canvas2.h>

class KisColorSelector;
class KisMyPaintShadeSelector;
class KisMinimalShadeSelector;
class QBoxLayout;
class QAction;
class KisGamutMaskToolbar;

class KisColorSelectorContainer : public QWidget
{
Q_OBJECT
public:
    explicit KisColorSelectorContainer(QWidget *parent = 0);
    void setCanvas(KisCanvas2* canvas);
    void unsetCanvas();
    bool doesAtleastOneDocumentExist();

    enum ShadeSelectorType{MyPaintSelector, MinimalSelector, NoSelector};

public Q_SLOTS:
    void slotUpdateIcons();

Q_SIGNALS:
    void openSettings();
    void settingsChanged();

protected Q_SLOTS:
    void updateSettings();

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    KisColorSelector* m_colorSelector;
    KisMyPaintShadeSelector* m_myPaintShadeSelector;
    KisMinimalShadeSelector* m_minimalShadeSelector;
    QWidget* m_shadeSelector;
    KisGamutMaskToolbar* m_gamutMaskToolbar;

    int m_onDockerResizeSetting;
    bool m_showColorSelector;

    QBoxLayout* m_widgetLayout;

    QAction * m_colorSelAction;
    QAction * m_mypaintAction;
    QAction * m_minimalAction;

    QPointer<KisCanvas2> m_canvas;
};

#endif // KIS_COLOR_SELECTOR_CONTAINER_H
