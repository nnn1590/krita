/*
 *  kis_tool_fill.h - part of Krayon^Krita
 *
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef KIS_TOOL_FILL_H_
#define KIS_TOOL_FILL_H_

#include "kis_tool_paint.h"
#include <flake/kis_node_shape.h>
#include <KoIcon.h>
#include <kis_icon.h>
#include <QPoint>
#include <QList>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kis_signal_compressor.h>
#include <kis_signal_auto_connection.h>


class QWidget;
class QCheckBox;
class KisSliderSpinBox;
class KoCanvasBase;
class KisColorFilterCombo;
class KisDummiesFacadeBase;

class KisToolFill : public KisToolPaint
{

    Q_OBJECT

public:

    KisToolFill(KoCanvasBase * canvas);
    ~KisToolFill() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    QWidget * createOptionWidget() override;

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;
    void slotSetUseFastMode(bool);
    void slotSetThreshold(int);
    void slotSetUsePattern(bool);
    void slotSetFillSelection(bool);
    void slotSetSizemod(int);
    void slotSetFeather(int);
    void slotSetSampleLayers(int index);
    void slotSetSelectedColorLabels();

protected Q_SLOTS:
    void resetCursorStyle() override;
    void slotUpdateAvailableColorLabels();

protected:
    bool wantsAutoScroll() const override { return false; }
private:
    void updateGUI();
    QString sampleLayerModeToUserString(QString sampleLayersModeId);
    void setCmbSampleLayersMode(QString sampleLayersModeId);

    void activateConnectionsToImage();
    void deactivateConnectionsToImage();


private:

    QString SAMPLE_LAYERS_MODE_CURRENT = {"currentLayer"};
    QString SAMPLE_LAYERS_MODE_ALL = {"allLayers"};
    QString SAMPLE_LAYERS_MODE_COLOR_LABELED = {"colorLabeledLayers"};


private:
    Qt::KeyboardModifiers keysAtStart;
    int m_feather;
    int m_sizemod;
    QPoint m_startPos;
    int m_threshold;
    bool m_usePattern;
    bool m_fillOnlySelection;
    QString m_sampleLayersMode;
    QList<int> m_selectedColors;

    bool m_widgetsInitialized {false};

    QCheckBox *m_useFastMode;
    KisSliderSpinBox *m_slThreshold;
    KisSliderSpinBox *m_sizemodWidget;
    KisSliderSpinBox *m_featherWidget;
    QCheckBox *m_checkUsePattern;
    QCheckBox *m_checkFillSelection;
    QComboBox *m_cmbSampleLayersMode;
    KisColorFilterCombo *m_cmbSelectedLabels;
    KisSignalCompressor m_colorLabelCompressor;
    KisDummiesFacadeBase* m_dummiesFacade;
    KisSignalAutoConnectionsStore m_imageConnections;

    KConfigGroup m_configGroup;
};


#include "KisToolPaintFactoryBase.h"

class KisToolFillFactory : public KisToolPaintFactoryBase
{

public:
    KisToolFillFactory()
            : KisToolPaintFactoryBase("KritaFill/KisToolFill") {
        setToolTip(i18n("Fill Tool"));
        setSection(TOOL_TYPE_FILL);
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_color_fill"));
        setShortcut( QKeySequence( Qt::Key_F ) );
        setPriority(14);
    }

    ~KisToolFillFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolFill(canvas);
    }

};

#endif //__filltool_h__

