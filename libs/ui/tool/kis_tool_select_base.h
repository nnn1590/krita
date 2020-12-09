/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISTOOLSELECTBASE_H
#define KISTOOLSELECTBASE_H

#include "KoPointerEvent.h"
#include "kis_tool.h"
#include "kis_canvas2.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selection_tool_config_widget_helper.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "kis_selection_modifier_mapper.h"
#include "strokes/move_stroke_strategy.h"
#include "kis_image.h"
#include "kis_cursor.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_signal_auto_connection.h"
#include "kis_selection_tool_helper.h"
#include "kis_assert.h"

/**
 * This is a basic template to create selection tools from basic path based drawing tools.
 * The template overrides the ability to execute alternate actions correctly.
 * The default behavior for the modifier keys is as follows:
 *
 * Shift: add to selection
 * Alt: subtract from selection
 * Shift+Alt: intersect current selection
 * Ctrl: replace selection
 *
 * The mapping itself is done in KisSelectionModifierMapper.
 *
 * Certain tools also use modifier keys to alter their behavior, e.g. forcing square proportions with the rectangle tool.
 * The template enables the following rules for forwarding keys:

 * 1) Any modifier keys held *when the tool is first activated* will determine
 * the new selection method. This is recorded in m_selectionActionAlternate. A
 * value of m_selectionActionAlternate = SELECTION_DEFAULT means no modifier was
 * being pressed when the tool was activated.
 *
 * 2) If the underlying tool *does not take modifier keys*, pressing modifier
 * keys in the middle of a stroke will change the selection method. This is
 * recorded in m_selectionAction. A value of SELECTION_DEFAULT means no modifier
 * is being pressed. Applies to the lasso tool and polygon tool.
 *
 * 3) If the underlying tool *takes modifier keys,* they will always be
 * forwarded to the underlying tool, and it is not possible to change the
 * selection method in the middle of a stroke.
 */

template <class BaseClass>
class KisToolSelectBase : public BaseClass
{

public:

    KisToolSelectBase(KoCanvasBase* canvas, const QString toolName)
        : BaseClass(canvas)
        , m_widgetHelper(toolName)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
    }

    KisToolSelectBase(KoCanvasBase* canvas, const QCursor cursor, const QString toolName)
        : BaseClass(canvas, cursor)
        , m_widgetHelper(toolName)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
    }

    KisToolSelectBase(KoCanvasBase* canvas, QCursor cursor, QString toolName, KoToolBase *delegateTool)
        : BaseClass(canvas, cursor, delegateTool)
        , m_widgetHelper(toolName)
        , m_selectionActionAlternate(SELECTION_DEFAULT)
    {
        KisSelectionModifierMapper::instance();
    }

    enum SampleLayersMode
    {
        SampleAllLayers,
        SampleCurrentLayer,
        SampleColorLabeledLayers,
    };

    void updateActionShortcutToolTips() {
        KisSelectionOptions *widget = m_widgetHelper.optionWidget();
        if (widget) {
            widget->updateActionButtonToolTip(
                SELECTION_REPLACE,
                this->action("selection_tool_mode_replace")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_ADD,
                this->action("selection_tool_mode_add")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_SUBTRACT,
                this->action("selection_tool_mode_subtract")->shortcut());
            widget->updateActionButtonToolTip(
                SELECTION_INTERSECT,
                this->action("selection_tool_mode_intersect")->shortcut());
        }
    }

    void activate(KoToolBase::ToolActivation activation, const QSet<KoShape*> &shapes)
    {
        BaseClass::activate(activation, shapes);

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_replace"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotReplaceModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_add"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotAddModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_subtract"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotSubtractModeRequested()));

        m_modeConnections.addUniqueConnection(
            this->action("selection_tool_mode_intersect"), SIGNAL(triggered()),
            &m_widgetHelper, SLOT(slotIntersectModeRequested()));

        updateActionShortcutToolTips();

        if (m_widgetHelper.optionWidget()) {

            m_widgetHelper.optionWidget()->activateConnectionToImage();

            if (isPixelOnly()) {
                m_widgetHelper.optionWidget()->enablePixelOnlySelectionMode();
            }
            m_widgetHelper.optionWidget()->setColorLabelsEnabled(usesColorLabels());
        }
    }

    void deactivate()
    {
        BaseClass::deactivate();
        m_modeConnections.clear();
        if (m_widgetHelper.optionWidget()) {
            m_widgetHelper.optionWidget()->deactivateConnectionToImage();
        }
    }

    QWidget* createOptionWidget()
    {
        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        Q_ASSERT(canvas);

        m_widgetHelper.createOptionWidget(canvas, this->toolId());
        this->connect(this, SIGNAL(isActiveChanged(bool)), &m_widgetHelper, SLOT(slotToolActivatedChanged(bool)));
        this->connect(&m_widgetHelper, SIGNAL(selectionActionChanged(int)), this, SLOT(resetCursorStyle()));

        updateActionShortcutToolTips();
        if (m_widgetHelper.optionWidget()) {
            if (isPixelOnly()) {
                m_widgetHelper.optionWidget()->enablePixelOnlySelectionMode();
            }
            m_widgetHelper.optionWidget()->setColorLabelsEnabled(usesColorLabels());
        }

        return m_widgetHelper.optionWidget();
    }

    SelectionMode selectionMode() const
    {
        return m_widgetHelper.selectionMode();
    }

    SelectionAction selectionAction() const
    {
        if (alternateSelectionAction() == SELECTION_DEFAULT) {
            return m_widgetHelper.selectionAction();
        }
        return alternateSelectionAction();
    }

    bool antiAliasSelection() const
    {
        return m_widgetHelper.antiAliasSelection();
    }

    QList<int> colorLabelsSelected() const
    {
        return m_widgetHelper.colorLabelsSelected();
    }

    SampleLayersMode sampleLayersMode() const
    {
        QString layersMode = m_widgetHelper.sampleLayersMode();
        if (layersMode == m_widgetHelper.optionWidget()->SAMPLE_LAYERS_MODE_ALL) {
            return SampleAllLayers;
        } else if (layersMode == m_widgetHelper.optionWidget()->SAMPLE_LAYERS_MODE_CURRENT) {
            return SampleCurrentLayer;
        } else if (layersMode == m_widgetHelper.optionWidget()->SAMPLE_LAYERS_MODE_COLOR_LABELED) {
            return SampleColorLabeledLayers;
        }
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(true, SampleAllLayers);
        return SampleAllLayers;
    }

    SelectionAction alternateSelectionAction() const
    {
        return m_selectionActionAlternate;
    }

    KisSelectionOptions* selectionOptionWidget()
    {
        return m_widgetHelper.optionWidget();
    }

    virtual void setAlternateSelectionAction(SelectionAction action)
    {
        m_selectionActionAlternate = action;
        dbgKrita << "Changing to selection action" << m_selectionActionAlternate;
    }

    void activateAlternateAction(KisTool::AlternateAction action)
    {
        Q_UNUSED(action);
        BaseClass::activatePrimaryAction();
    }

    void deactivateAlternateAction(KisTool::AlternateAction action)
    {
        Q_UNUSED(action);
        BaseClass::deactivatePrimaryAction();
    }

    void beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        beginPrimaryAction(event);
    }

    void continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        continuePrimaryAction(event);
    }

    void endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action) {
        Q_UNUSED(action);
        endPrimaryAction(event);
    }

    KisNodeSP locateSelectionMaskUnderCursor(const QPointF &pos, Qt::KeyboardModifiers modifiers) {
        if (modifiers != Qt::NoModifier) return 0;

        KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas, 0);

        KisSelectionSP selection = canvas->viewManager()->selection();
        if (selection &&
            selection->outlineCacheValid()) {

            const qreal handleRadius = qreal(this->handleRadius()) / canvas->coordinatesConverter()->effectiveZoom();
            QPainterPath samplePath;
            samplePath.addEllipse(pos, handleRadius, handleRadius);

            const QPainterPath selectionPath = selection->outlineCache();

            if (selectionPath.intersects(samplePath) && !selectionPath.contains(samplePath)) {
                KisNodeSP parent = selection->parentNode();
                if (parent && parent->isEditable()) {
                    return parent;
                }
            }
        }

        return 0;
    }

    void keyPressEvent(QKeyEvent *event) {
        if (this->mode() != KisTool::PAINT_MODE) {
            setAlternateSelectionAction(KisSelectionModifierMapper::map(event->modifiers()));
            this->resetCursorStyle();
        }
        BaseClass::keyPressEvent(event);
    }

    void keyReleaseEvent(QKeyEvent *event) {
        if (this->mode() != KisTool::PAINT_MODE) {
            setAlternateSelectionAction(KisSelectionModifierMapper::map(event->modifiers()));
            this->resetCursorStyle();
        }
        BaseClass::keyPressEvent(event);
    }

    void mouseMoveEvent(KoPointerEvent *event) {
        if (!this->hasUserInteractionRunning() &&
           (m_moveStrokeId || this->mode() != KisTool::PAINT_MODE)) {

            const QPointF pos = this->convertToPixelCoord(event->point);
            KisNodeSP selectionMask = locateSelectionMaskUnderCursor(pos, event->modifiers());
            if (selectionMask) {
                this->useCursor(KisCursor::moveSelectionCursor());
            } else {
                setAlternateSelectionAction(KisSelectionModifierMapper::map(event->modifiers()));
                this->resetCursorStyle();
            }
        }

        BaseClass::mouseMoveEvent(event);
    }


    virtual void beginPrimaryAction(KoPointerEvent *event)
    {
        if (!this->hasUserInteractionRunning()) {
            const QPointF pos = this->convertToPixelCoord(event->point);
            KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(this->canvas());
            KIS_SAFE_ASSERT_RECOVER_RETURN(canvas);

            KisNodeSP selectionMask = locateSelectionMaskUnderCursor(pos, event->modifiers());
            if (selectionMask) {
                KisStrokeStrategy *strategy = new MoveStrokeStrategy({selectionMask}, this->image().data(), this->image().data());
                m_moveStrokeId = this->image()->startStroke(strategy);
                m_dragStartPos = pos;
                m_didMove = true;
                return;
            }
        }
        m_didMove = false;
        keysAtStart = event->modifiers();

        setAlternateSelectionAction(KisSelectionModifierMapper::map(keysAtStart));
        if (alternateSelectionAction() != SELECTION_DEFAULT) {
            BaseClass::listenToModifiers(false);
        }
        BaseClass::beginPrimaryAction(event);
    }

    virtual void continuePrimaryAction(KoPointerEvent *event)
    {
        if (m_moveStrokeId) {
            const QPointF pos = this->convertToPixelCoord(event->point);
            const QPoint offset((pos - m_dragStartPos).toPoint());

            this->image()->addJob(m_moveStrokeId, new MoveStrokeStrategy::Data(offset));
            return;
        }


        //If modifier keys have changed, tell the base tool it can start capturing modifiers
        if ((keysAtStart != event->modifiers()) && !BaseClass::listeningToModifiers()) {
            BaseClass::listenToModifiers(true);
        }

        //Always defer to the base class if it signals it is capturing modifier keys
        if (!BaseClass::listeningToModifiers()) {
            setAlternateSelectionAction(KisSelectionModifierMapper::map(event->modifiers()));
        }

        BaseClass::continuePrimaryAction(event);
    }

    void endPrimaryAction(KoPointerEvent *event)
    {
        if (m_moveStrokeId) {
            this->image()->endStroke(m_moveStrokeId);

            m_moveStrokeId.clear();
            return;
        }


        keysAtStart = Qt::NoModifier; //reset this with each action
        BaseClass::endPrimaryAction(event);
    }

    bool selectionDragInProgress() const {
        return m_moveStrokeId;
    }

    bool selectionDidMove() const {
        return m_didMove;
    }

    QMenu* popupActionsMenu() {
        KisCanvas2 * kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(kisCanvas, 0);

        return KisSelectionToolHelper::getSelectionContextMenu(kisCanvas);
    }


protected:
    using BaseClass::canvas;
    KisSelectionToolConfigWidgetHelper m_widgetHelper;
    SelectionAction m_selectionActionAlternate;

    virtual bool isPixelOnly() const {
        return false;
    }

    virtual bool usesColorLabels() const {
        return false;
    }

private:
    Qt::KeyboardModifiers keysAtStart;

    QPointF m_dragStartPos;
    KisStrokeId m_moveStrokeId;
    bool m_didMove = false;

    KisSignalAutoConnectionsStore m_modeConnections;
};

struct FakeBaseTool : KisTool
{
    FakeBaseTool(KoCanvasBase* canvas)
        : KisTool(canvas, QCursor())
    {
    }

    FakeBaseTool(KoCanvasBase* canvas, const QString &toolName)
        : KisTool(canvas, QCursor())
    {
        Q_UNUSED(toolName);
    }

    FakeBaseTool(KoCanvasBase* canvas, const QCursor &cursor)
        : KisTool(canvas, cursor)
    {
    }

    bool hasUserInteractionRunning() const {
        return false;
    }

};


typedef KisToolSelectBase<FakeBaseTool> KisToolSelect;


#endif // KISTOOLSELECTBASE_H
