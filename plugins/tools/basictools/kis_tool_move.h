/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2003 Patrick Julien  <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_MOVE_H_
#define KIS_TOOL_MOVE_H_

#include <KisToolPaintFactoryBase.h>
#include <kis_types.h>
#include <kis_tool.h>
#include <flake/kis_node_shape.h>
#include <kis_icon.h>
#include <QKeySequence>
#include <QWidget>
#include <QGroupBox>
#include <QRadioButton>
#include "KisToolChangesTracker.h"
#include "kis_signal_compressor.h"
#include "kis_signal_auto_connection.h"
#include "KisAsyncronousStrokeUpdateHelper.h"

#include "kis_canvas2.h"


class KoCanvasBase;
class MoveToolOptionsWidget;
class KisDocument;

class KisToolMove : public KisTool
{
    Q_OBJECT
    Q_ENUMS(MoveToolMode);
public:
    KisToolMove(KoCanvasBase * canvas);
    ~KisToolMove() override;

    /**
     * @brief wantsAutoScroll
     * reimplemented from KoToolBase
     * there's an issue where autoscrolling with this tool never makes the
     * stroke end, so we return false here so that users don't get stuck with
     * the tool. See bug 362659
     * @return false
     */
    bool wantsAutoScroll() const override {
        return false;
    }

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;

public Q_SLOTS:
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;
    void requestUndoDuringStroke() override;

protected Q_SLOTS:
    void resetCursorStyle() override;

public:
    enum MoveToolMode {
        MoveSelectedLayer,
        MoveFirstLayer,
        MoveGroup
    };

    enum MoveDirection {
        Up,
        Down,
        Left,
        Right
    };

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action) override;
    void endAlternateAction(KoPointerEvent *event, AlternateAction action) override;

    void mouseMoveEvent(KoPointerEvent *event) override;

    void startAction(KoPointerEvent *event, MoveToolMode mode);
    void continueAction(KoPointerEvent *event);
    void endAction(KoPointerEvent *event);

    void paint(QPainter& gc, const KoViewConverter &converter) override;

    QWidget *createOptionWidget() override;
    void updateUIUnit(int newUnit);

    MoveToolMode moveToolMode() const;

    void setShowCoordinates(bool value);

public Q_SLOTS:
    void moveDiscrete(MoveDirection direction, bool big);

    void moveBySpinX(int newX);
    void moveBySpinY(int newY);

    void slotNodeChanged(const KisNodeList &nodes);
    void slotSelectionChanged();
    void commitChanges();

    void slotHandlesRectCalculated(const QRect &handlesRect);
    void slotStrokeStartedEmpty();
    void slotStrokePickedLayers(const KisNodeList &nodes);

Q_SIGNALS:
    void moveToolModeChanged();
    void moveInNewPosition(QPoint);

private:
    void drag(const QPoint& newPos);
    void cancelStroke();
    QPoint applyModifiers(Qt::KeyboardModifiers modifiers, QPoint pos);

    bool startStrokeImpl(MoveToolMode mode, const QPoint *pos);

    QPoint currentOffset() const;
    void notifyGuiAfterMove(bool showFloatingMessage = true);
    bool tryEndPreviousStroke(const KisNodeList &nodes);
    void requestHandlesRectUpdate();


private Q_SLOTS:
    void endStroke();
    void slotTrackerChangedConfig(KisToolChangesTrackerDataSP state);

    void slotMoveDiscreteLeft();
    void slotMoveDiscreteRight();
    void slotMoveDiscreteUp();
    void slotMoveDiscreteDown();
    void slotMoveDiscreteLeftMore();
    void slotMoveDiscreteRightMore();
    void slotMoveDiscreteUpMore();
    void slotMoveDiscreteDownMore();

private:

    MoveToolOptionsWidget* m_optionsWidget {0};
    QPoint m_dragStart; ///< Point where current cursor dragging began
    QPoint m_accumulatedOffset; ///< Total offset including multiple clicks, up/down/left/right keys, etc. added together

    KisStrokeId m_strokeId;

    KisNodeList m_currentlyProcessingNodes;
    bool m_currentlyUsingSelection = false;
    MoveToolMode m_currentMode = MoveSelectedLayer;

    int m_resolution;

    QAction *m_showCoordinatesAction {0};

    QPoint m_dragPos;
    QRect m_handlesRect;

    KisToolChangesTracker m_changesTracker;

    QPoint m_lastCursorPos;
    KisSignalCompressor m_updateCursorCompressor;
    KisSignalAutoConnectionsStore m_actionConnections;
    KisSignalAutoConnectionsStore m_canvasConnections;

    KisAsyncronousStrokeUpdateHelper m_asyncUpdateHelper;
};


class KisToolMoveFactory : public KisToolPaintFactoryBase
{

public:
    KisToolMoveFactory()
            : KisToolPaintFactoryBase("KritaTransform/KisToolMove") {
        setToolTip(i18n("Move Tool"));
        setSection(TOOL_TYPE_TRANSFORM);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(3);
        setIconName(koIconNameCStr("krita_tool_move"));
        setShortcut(QKeySequence(Qt::Key_T));
    }

    ~KisToolMoveFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolMove(canvas);
    }

    QList<QAction *> createActionsImpl() override;
};

#endif // KIS_TOOL_MOVE_H_

