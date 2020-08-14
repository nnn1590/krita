/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
 * Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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

#include <kis_assistant_tool.h>

#include <QPainter>
#include <QPainterPath>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStandardPaths>
#include <QFile>
#include <QLineF>


#include <kis_debug.h>
#include <klocalizedstring.h>
#include <KColorButton>
#include "kis_dom_utils.h"
#include <QMessageBox>

#include <KoIcon.h>
#include <KoFileDialog.h>
#include <KoViewConverter.h>
#include <KoPointerEvent.h>
#include <KoSnapGuide.h>

#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_icon.h>
#include <kis_abstract_perspective_grid.h>
#include <kis_painting_assistants_decoration.h>
#include "kis_global.h"
#include "VanishingPointAssistant.h"
#include "EditAssistantsCommand.h"
#include <kis_undo_adapter.h>

#include <math.h>

KisAssistantTool::KisAssistantTool(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::arrowCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas)),
      m_assistantDrag(0), m_newAssistant(0), m_optionsWidget(0)
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_assistanttool");
}

KisAssistantTool::~KisAssistantTool()
{
}

void KisAssistantTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{

    KisTool::activate(toolActivation, shapes);

    m_canvas->paintingAssistantsDecoration()->activateAssistantsEditor();
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();

    m_handleDrag = 0;
    m_internalMode = MODE_CREATION;
    m_assistantHelperYOffset = 10;


    m_handleSize = 17;
    m_canvas->paintingAssistantsDecoration()->setHandleSize(m_handleSize);


    if (m_optionsWidget) {
        m_canvas->paintingAssistantsDecoration()->deselectAssistant();
        updateToolOptionsUI();
    }

    m_canvas->updateCanvas();
}

void KisAssistantTool::deactivate()
{
    m_canvas->paintingAssistantsDecoration()->deactivateAssistantsEditor();
    m_canvas->updateCanvas();
    KisTool::deactivate();
}

void KisAssistantTool::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);
    m_origAssistantList = KisPaintingAssistant::cloneAssistantList(m_canvas->paintingAssistantsDecoration()->assistants());

    bool newAssistantAllowed = true;

    KisPaintingAssistantsDecorationSP canvasDecoration = m_canvas->paintingAssistantsDecoration();

    if (m_newAssistant) {
        m_internalMode = MODE_CREATION;
        *m_newAssistant->handles().back() = canvasDecoration->snapToGuide(event, QPointF(), false);
        if (m_newAssistant->handles().size() == m_newAssistant->numHandles()) {
            addAssistant();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(canvasDecoration->snapToGuide(event, QPointF(), false)), HandleType::NORMAL);
        }
        m_canvas->updateCanvas();
        return;
    }
    m_handleDrag = 0;
    double minDist = 81.0;


    QPointF mousePos = m_canvas->viewConverter()->documentToView(canvasDecoration->snapToGuide(event, QPointF(), false));//m_canvas->viewConverter()->documentToView(event->point);

    // syncs the assistant handles to the handles reference we store in this tool
    // they can get out of sync with the way the actions and paintevents occur
    // we probably need to stop storing a reference in m_handles and call the assistants directly
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();


    Q_FOREACH (KisPaintingAssistantSP assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {


        // find out which handle on all assistants is closest to the mouse position
        // vanishing points have "side handles", so make sure to include that
        {
            QList<KisPaintingAssistantHandleSP> allAssistantHandles;
            allAssistantHandles.append(assistant->handles());
            allAssistantHandles.append(assistant->sideHandles());

            Q_FOREACH (const KisPaintingAssistantHandleSP handle, allAssistantHandles) {

                double dist = KisPaintingAssistant::norm2(mousePos - m_canvas->viewConverter()->documentToView(*handle));
                if (dist < minDist) {
                    minDist = dist;
                    m_handleDrag = handle;

                    assistantSelected(assistant); // whatever handle is the closest contains the selected assistant
                }
            }
        }




        if(m_handleDrag && assistant->id() == "perspective") {
            // Look for the handle which was pressed


            if (m_handleDrag == assistant->topLeft()) {
                double dist = KisPaintingAssistant::norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_dragStart = QPointF(assistant->topRight().data()->x(),assistant->topRight().data()->y());
                m_internalMode = MODE_DRAGGING_NODE;
            } else if (m_handleDrag == assistant->topRight()) {
                double dist = KisPaintingAssistant::norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_internalMode = MODE_DRAGGING_NODE;
                m_dragStart = QPointF(assistant->topLeft().data()->x(),assistant->topLeft().data()->y());
            } else if (m_handleDrag == assistant->bottomLeft()) {
                double dist = KisPaintingAssistant::norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_internalMode = MODE_DRAGGING_NODE;
                m_dragStart = QPointF(assistant->bottomRight().data()->x(),assistant->bottomRight().data()->y());
            } else if (m_handleDrag == assistant->bottomRight()) {
                double dist = KisPaintingAssistant::norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_internalMode = MODE_DRAGGING_NODE;
                m_dragStart = QPointF(assistant->bottomLeft().data()->x(),assistant->bottomLeft().data()->y());
            } else if (m_handleDrag == assistant->leftMiddle()) {
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_dragStart = QPointF((assistant->bottomLeft().data()->x()+assistant->topLeft().data()->x())*0.5,
                                      (assistant->bottomLeft().data()->y()+assistant->topLeft().data()->y())*0.5);
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->topLeft().data()->x(),assistant->topLeft().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->bottomLeft().data()->x(),assistant->bottomLeft().data()->y());
                m_newAssistant = toQShared(KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant());
                m_newAssistant->addHandle(assistant->topLeft(), HandleType::NORMAL );
                m_newAssistant->addHandle(m_selectedNode1, HandleType::NORMAL);
                m_newAssistant->addHandle(m_selectedNode2, HandleType::NORMAL);
                m_newAssistant->addHandle(assistant->bottomLeft(), HandleType::NORMAL);
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            } else if (m_handleDrag == assistant->rightMiddle()) {
                m_dragStart = QPointF((assistant->topRight().data()->x()+assistant->bottomRight().data()->x())*0.5,
                                      (assistant->topRight().data()->y()+assistant->bottomRight().data()->y())*0.5);
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->topRight().data()->x(),assistant->topRight().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->bottomRight().data()->x(),assistant->bottomRight().data()->y());
                m_newAssistant = toQShared(KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant());
                m_newAssistant->addHandle(assistant->topRight(), HandleType::NORMAL);
                m_newAssistant->addHandle(m_selectedNode1, HandleType::NORMAL);
                m_newAssistant->addHandle(m_selectedNode2, HandleType::NORMAL);
                m_newAssistant->addHandle(assistant->bottomRight(), HandleType::NORMAL);
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            } else if (m_handleDrag == assistant->topMiddle()) {
                m_dragStart = QPointF((assistant->topLeft().data()->x()+assistant->topRight().data()->x())*0.5,
                                      (assistant->topLeft().data()->y()+assistant->topRight().data()->y())*0.5);
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->topLeft().data()->x(),assistant->topLeft().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->topRight().data()->x(),assistant->topRight().data()->y());
                m_newAssistant = toQShared(KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant());
                m_newAssistant->addHandle(m_selectedNode1, HandleType::NORMAL);
                m_newAssistant->addHandle(m_selectedNode2, HandleType::NORMAL);
                m_newAssistant->addHandle(assistant->topRight(), HandleType::NORMAL);
                m_newAssistant->addHandle(assistant->topLeft(), HandleType::NORMAL);
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            } else if (m_handleDrag == assistant->bottomMiddle()) {
                m_dragStart = QPointF((assistant->bottomLeft().data()->x()+assistant->bottomRight().data()->x())*0.5,
                                      (assistant->bottomLeft().data()->y()+assistant->bottomRight().data()->y())*0.5);
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->bottomLeft().data()->x(),assistant->bottomLeft().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->bottomRight().data()->x(),assistant->bottomRight().data()->y());
                m_newAssistant = toQShared(KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant());
                m_newAssistant->addHandle(assistant->bottomLeft(), HandleType::NORMAL);
                m_newAssistant->addHandle(assistant->bottomRight(), HandleType::NORMAL);
                m_newAssistant->addHandle(m_selectedNode2, HandleType::NORMAL);
                m_newAssistant->addHandle(m_selectedNode1, HandleType::NORMAL);
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            }
            m_snapIsRadial = false;
        }
        else if (m_handleDrag && assistant->handles().size()>1 && (assistant->id() == "ruler" ||
                                                                   assistant->id() == "parallel ruler" ||
                                                                   assistant->id() == "infinite ruler" ||
                                                                   assistant->id() == "spline")){
            if (m_handleDrag == assistant->handles()[0]) {
                m_dragStart = *assistant->handles()[1];
            } else if (m_handleDrag == assistant->handles()[1]) {
                m_dragStart = *assistant->handles()[0];
            } else if(assistant->handles().size()==4){
                if (m_handleDrag == assistant->handles()[2]) {
                    m_dragStart = *assistant->handles()[0];
                } else if (m_handleDrag == assistant->handles()[3]) {
                    m_dragStart = *assistant->handles()[1];
                }
            }
            m_snapIsRadial = false;
        } else if (m_handleDrag && assistant->handles().size()>2 && (assistant->id() == "ellipse" ||
                                    assistant->id() == "concentric ellipse" ||
                                    assistant->id() == "fisheye-point")){
            if (m_handleDrag == assistant->handles()[0]) {
                m_dragStart = *assistant->handles()[1];
                m_snapIsRadial = false;
            } else if (m_handleDrag == assistant->handles()[1]) {
                m_dragStart = *assistant->handles()[0];
                m_snapIsRadial = false;
            } else if (m_handleDrag == assistant->handles()[2]) {
                m_dragStart = assistant->getEditorPosition();
                m_radius = QLineF(m_dragStart, *assistant->handles()[0]);
                m_snapIsRadial = true;
            }

        } else {
            m_dragStart = assistant->getEditorPosition();
            m_snapIsRadial = false;
        }
    }

    if (m_handleDrag) {
        // TODO: Shift-press should now be handled using the alternate actions
        // if (event->modifiers() & Qt::ShiftModifier) {
        //     m_handleDrag->uncache();
        //     m_handleDrag = m_handleDrag->split()[0];
        //     m_handles = m_canvas->view()->paintingAssistantsDecoration()->handles();
        // }
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        return;
    }

    m_assistantDrag.clear();
    Q_FOREACH (KisPaintingAssistantSP assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {

        AssistantEditorData editorShared; // shared position data between assistant tool and decoration
        const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();

        // This code contains the click event behavior.
        QTransform initialTransform = converter->documentToWidgetTransform();
        QPointF actionsPosition = initialTransform.map(assistant->viewportConstrainedEditorPosition(converter, editorShared.boundingSize));

        // for UI editor widget controls with move, show, and delete -- disregard document transforms like rotating and mirroring.
        // otherwise the UI controls get awkward to use when they are at 45 degree angles or the order of controls gets flipped backwards
        QPointF uiMousePosition = initialTransform.map(canvasDecoration->snapToGuide(event, QPointF(), false));

        QPointF iconMovePosition(actionsPosition + editorShared.moveIconPosition);
        QPointF iconSnapPosition(actionsPosition + editorShared.snapIconPosition);
        QPointF iconDeletePosition(actionsPosition + editorShared.deleteIconPosition);

        QRectF deleteRect(iconDeletePosition, QSizeF(editorShared.deleteIconSize, editorShared.deleteIconSize));
        QRectF visibleRect(iconSnapPosition, QSizeF(editorShared.snapIconSize, editorShared.snapIconSize));
        QRectF moveRect(iconMovePosition, QSizeF(editorShared.moveIconSize, editorShared.moveIconSize));

        if (moveRect.contains(uiMousePosition)) {
            m_assistantDrag = assistant;
            m_cursorStart = event->point;
            m_currentAdjustment = QPointF();
            m_internalMode = MODE_EDITING;


            assistantSelected(assistant); // whatever handle is the closest contains the selected assistant

            return;
        }

        if (deleteRect.contains(uiMousePosition)) {
            removeAssistant(assistant);
            if(m_canvas->paintingAssistantsDecoration()->assistants().isEmpty()) {
                m_internalMode = MODE_CREATION;
            }
            else
                m_internalMode = MODE_EDITING;
            m_canvas->updateCanvas();
            return;
        }
        if (visibleRect.contains(uiMousePosition)) {
            newAssistantAllowed = false;
            assistant->setSnappingActive(!assistant->isSnappingActive()); // toggle
            assistant->uncache();//this updates the chache of the assistant, very important.

            assistantSelected(assistant); // whatever handle is the closest contains the selected assistant
        }
    }
    if (newAssistantAllowed==true){//don't make a new assistant when I'm just toogling visibility//
        QString key = m_options.availableAssistantsComboBox->model()->index( m_options.availableAssistantsComboBox->currentIndex(), 0 ).data(Qt::UserRole).toString();
        m_newAssistant = toQShared(KisPaintingAssistantFactoryRegistry::instance()->get(key)->createPaintingAssistant());
        m_internalMode = MODE_CREATION;
        m_newAssistant->addHandle(new KisPaintingAssistantHandle(canvasDecoration->snapToGuide(event, QPointF(), false)), HandleType::NORMAL);
        if (m_newAssistant->numHandles() <= 1) {
            addAssistant();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(canvasDecoration->snapToGuide(event, QPointF(), false)), HandleType::NORMAL);
        }
    }

    if (m_newAssistant) {
        m_newAssistant->setAssistantGlobalColorCache(m_canvas->paintingAssistantsDecoration()->globalAssistantsColor());
    }

    m_canvas->updateCanvas();
}

void KisAssistantTool::continuePrimaryAction(KoPointerEvent *event)
{
    KisPaintingAssistantsDecorationSP canvasDecoration = m_canvas->paintingAssistantsDecoration();

    if (m_handleDrag) {
        *m_handleDrag = event->point;

        KisPaintingAssistantSP selectedAssistant = m_canvas->paintingAssistantsDecoration()->selectedAssistant();

        if (!snap(event)) {
            *m_handleDrag = canvasDecoration->snapToGuide(event, QPointF(), false);
        }
        m_handleDrag->uncache();

        m_handleCombine = 0;
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            double minDist = 49.0;
            QPointF mousePos = m_canvas->viewConverter()->documentToView(event->point);
            Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
                if (handle == m_handleDrag)
                    continue;


                double dist = KisPaintingAssistant::norm2(mousePos - m_canvas->viewConverter()->documentToView(*handle));
                if (dist < minDist) {
                    minDist = dist;
                    m_handleCombine = handle;
                }
            }
        }
        m_canvas->updateCanvas();
    } else if (m_assistantDrag) {
        QPointF newAdjustment = canvasDecoration->snapToGuide(event, QPointF(), false) - m_cursorStart;
        if (event->modifiers() & Qt::ShiftModifier ) {
            newAdjustment = snapToClosestAxis(newAdjustment);
        }
        Q_FOREACH (KisPaintingAssistantHandleSP handle, m_assistantDrag->handles()) {
            *handle += (newAdjustment - m_currentAdjustment);
        }
        if (m_assistantDrag->id()== "vanishing point"){
            Q_FOREACH (KisPaintingAssistantHandleSP handle, m_assistantDrag->sideHandles()) {
                *handle += (newAdjustment - m_currentAdjustment);
            }
        }
        m_assistantDrag->uncache();
        m_currentAdjustment = newAdjustment;
        m_canvas->updateCanvas();

    } else {
        event->ignore();
    }

    bool wasHiglightedNode = m_higlightedNode != 0;
    QPointF mousep = m_canvas->viewConverter()->documentToView(event->point);
    QList <KisPaintingAssistantSP> pAssistant= m_canvas->paintingAssistantsDecoration()->assistants();

    Q_FOREACH (KisPaintingAssistantSP assistant, pAssistant) {
        if(assistant->id() == "perspective") {
            if ((m_higlightedNode = assistant->closestCornerHandleFromPoint(mousep))) {
                if (m_higlightedNode == m_selectedNode1 || m_higlightedNode == m_selectedNode2) {
                    m_higlightedNode = 0;
                } else {
                    m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                    break;
                }
            }
        }

        //this following bit sets the translations for the vanishing-point handles.
        if(m_handleDrag && assistant->id() == "vanishing point" && assistant->sideHandles().size()==4) {
            //for inner handles, the outer handle gets translated.
            if (m_handleDrag == assistant->sideHandles()[0]) {
                QLineF perspectiveline = QLineF(*assistant->handles()[0],
                                                *assistant->sideHandles()[0]);

                qreal length = QLineF(*assistant->sideHandles()[0],
                                      *assistant->sideHandles()[1]).length();

                if (length < 2.0){
                    length = 2.0;
                }

                length += perspectiveline.length();
                perspectiveline.setLength(length);
                *assistant->sideHandles()[1] = perspectiveline.p2();
            }
            else if (m_handleDrag == assistant->sideHandles()[2]){
                QLineF perspectiveline = QLineF(*assistant->handles()[0], *assistant->sideHandles()[2]);
                qreal length = QLineF(*assistant->sideHandles()[2], *assistant->sideHandles()[3]).length();

                if (length<2.0){
                    length=2.0;
                }

                length += perspectiveline.length();
                perspectiveline.setLength(length);
                *assistant->sideHandles()[3] = perspectiveline.p2();
            } // for outer handles, only the vanishing point is translated, but only if there's an intersection.
            else if (m_handleDrag == assistant->sideHandles()[1]|| m_handleDrag == assistant->sideHandles()[3]){
                QPointF vanishingpoint(0,0);
                QLineF perspectiveline = QLineF(*assistant->sideHandles()[0], *assistant->sideHandles()[1]);
                QLineF perspectiveline2 = QLineF(*assistant->sideHandles()[2], *assistant->sideHandles()[3]);

                if (QLineF(perspectiveline2).intersect(QLineF(perspectiveline), &vanishingpoint) != QLineF::NoIntersection){
                    *assistant->handles()[0] = vanishingpoint;
                }
            }// and for the vanishing point itself, only the outer handles get translated.
            else if (m_handleDrag == assistant->handles()[0]){
                QLineF perspectiveline = QLineF(*assistant->handles()[0], *assistant->sideHandles()[0]);
                QLineF perspectiveline2 = QLineF(*assistant->handles()[0], *assistant->sideHandles()[2]);
                qreal length =  QLineF(*assistant->sideHandles()[0], *assistant->sideHandles()[1]).length();
                qreal length2 = QLineF(*assistant->sideHandles()[2], *assistant->sideHandles()[3]).length();

                if (length < 2.0) {
                    length = 2.0;
                }

                if (length2 < 2.0) {
                    length2=2.0;
                }

                length += perspectiveline.length();
                length2 += perspectiveline2.length();
                perspectiveline.setLength(length);
                perspectiveline2.setLength(length2);
                *assistant->sideHandles()[1] = perspectiveline.p2();
                *assistant->sideHandles()[3] = perspectiveline2.p2();
            }

        }
    }
    if (wasHiglightedNode && !m_higlightedNode) {
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    }
}

void KisAssistantTool::endPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::HOVER_MODE);

    if (m_handleDrag || m_assistantDrag) {
        if (m_handleDrag) {
            if (!(event->modifiers() & Qt::ShiftModifier) && m_handleCombine) {
                m_handleCombine->mergeWith(m_handleDrag);
                m_handleCombine->uncache();
                m_handles = m_canvas->paintingAssistantsDecoration()->handles();
            }
            m_handleDrag = m_handleCombine = 0;
        } else {
            m_assistantDrag.clear();
        }
        dbgUI << "creating undo command...";
        KUndo2Command *command = new EditAssistantsCommand(m_canvas, m_origAssistantList, KisPaintingAssistant::cloneAssistantList(m_canvas->paintingAssistantsDecoration()->assistants()));
        m_canvas->viewManager()->undoAdapter()->addCommand(command);
        dbgUI << "done";
    } else if(m_internalMode == MODE_DRAGGING_TRANSLATING_TWONODES) {
        addAssistant();
        m_internalMode = MODE_CREATION;
    }
    else {
        event->ignore();
    }

    m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
}

void KisAssistantTool::addAssistant()
{
    m_canvas->paintingAssistantsDecoration()->addAssistant(m_newAssistant);

    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(m_newAssistant.data());
    if (grid) {
        m_canvas->viewManager()->canvasResourceProvider()->addPerspectiveGrid(grid);
    }

    QList<KisPaintingAssistantSP> assistants = m_canvas->paintingAssistantsDecoration()->assistants();
    KUndo2Command *addAssistantCmd = new EditAssistantsCommand(m_canvas, m_origAssistantList, KisPaintingAssistant::cloneAssistantList(assistants), EditAssistantsCommand::ADD, assistants.indexOf(m_newAssistant));
    m_canvas->viewManager()->undoAdapter()->addCommand(addAssistantCmd);

    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->paintingAssistantsDecoration()->setSelectedAssistant(m_newAssistant);
    updateToolOptionsUI(); // vanishing point assistant will get an extra option

    m_newAssistant.clear();
}

void KisAssistantTool::removeAssistant(KisPaintingAssistantSP assistant)
{
    QList<KisPaintingAssistantSP> assistants = m_canvas->paintingAssistantsDecoration()->assistants();

    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant.data());
    if (grid) {
        m_canvas->viewManager()->canvasResourceProvider()->removePerspectiveGrid(grid);
    }
    m_canvas->paintingAssistantsDecoration()->removeAssistant(assistant);

    KUndo2Command *removeAssistantCmd = new EditAssistantsCommand(m_canvas, m_origAssistantList, KisPaintingAssistant::cloneAssistantList(m_canvas->paintingAssistantsDecoration()->assistants()), EditAssistantsCommand::REMOVE, assistants.indexOf(assistant));
    m_canvas->viewManager()->undoAdapter()->addCommand(removeAssistantCmd);

    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->paintingAssistantsDecoration()->deselectAssistant();
    updateToolOptionsUI();
}

void KisAssistantTool::assistantSelected(KisPaintingAssistantSP assistant)
{
     m_canvas->paintingAssistantsDecoration()->setSelectedAssistant(assistant);
     updateToolOptionsUI();
}

void KisAssistantTool::updateToolOptionsUI()
{
     KisPaintingAssistantSP m_selectedAssistant =  m_canvas->paintingAssistantsDecoration()->selectedAssistant();

     bool hasActiveAssistant = m_selectedAssistant ? true : false;

     if (m_selectedAssistant) {
         bool isVanishingPointAssistant = m_selectedAssistant->id() == "vanishing point";
         m_options.vanishingPointAngleSpinbox->setVisible(isVanishingPointAssistant);

         if (isVanishingPointAssistant) {
             QSharedPointer <VanishingPointAssistant> assis = qSharedPointerCast<VanishingPointAssistant>(m_selectedAssistant);
             m_options.vanishingPointAngleSpinbox->setValue(assis->referenceLineDensity());
         }

         // load custom color settings from assistant (this happens when changing assistant
         m_options.useCustomAssistantColor->setChecked(m_selectedAssistant->useCustomColor());
         m_options.customAssistantColorButton->setColor(m_selectedAssistant->assistantCustomColor());


         double opacity = (double)m_selectedAssistant->assistantCustomColor().alpha()/(double)255.00 * (double)100.00 ;
         opacity = ceil(opacity); // helps keep the 0-100% slider from shifting

         m_options.customColorOpacitySlider->blockSignals(true);
         m_options.customColorOpacitySlider->setValue((double)opacity);
         m_options.customColorOpacitySlider->blockSignals(false);


     } else {
         m_options.vanishingPointAngleSpinbox->setVisible(false); //
     }

     // show/hide elements if an assistant is selected or not
      m_options.useCustomAssistantColor->setVisible(hasActiveAssistant);

      // hide custom color options if use custom color is not selected
      bool showCustomColorSettings = m_options.useCustomAssistantColor->isChecked() && hasActiveAssistant;
      m_options.customColorOpacitySlider->setVisible(showCustomColorSettings);
      m_options.customAssistantColorButton->setVisible(showCustomColorSettings);

      // disable global color settings if we are using the custom color
      m_options.assistantsGlobalOpacitySlider->setEnabled(!showCustomColorSettings);
      m_options.assistantsColor->setEnabled(!showCustomColorSettings);
      m_options.globalColorLabel->setEnabled(!showCustomColorSettings);

}

void KisAssistantTool::slotChangeVanishingPointAngle(double value)
{
    if ( m_canvas->paintingAssistantsDecoration()->assistants().length() == 0) {
        return;
    }

    // get the selected assistant and change the angle value
    KisPaintingAssistantSP m_selectedAssistant =  m_canvas->paintingAssistantsDecoration()->selectedAssistant();
    if (m_selectedAssistant) {
        bool isVanishingPointAssistant = m_selectedAssistant->id() == "vanishing point";

        if (isVanishingPointAssistant) {
            QSharedPointer <VanishingPointAssistant> assis = qSharedPointerCast<VanishingPointAssistant>(m_selectedAssistant);
            assis->setReferenceLineDensity((float)value);
        }
    }

    m_canvas->canvasWidget()->update();
}

void KisAssistantTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_newAssistant && m_internalMode == MODE_CREATION) {

        KisPaintingAssistantHandleSP new_handle = m_newAssistant->handles().back();
        if (!snap(event)) {
            *new_handle = event->point;
        }

    } else if (m_newAssistant && m_internalMode == MODE_DRAGGING_TRANSLATING_TWONODES) {
        QPointF translate = event->point - m_dragEnd;
        m_dragEnd = event->point;
        m_selectedNode1.data()->operator = (QPointF(m_selectedNode1.data()->x(),m_selectedNode1.data()->y()) + translate);
        m_selectedNode2.data()->operator = (QPointF(m_selectedNode2.data()->x(),m_selectedNode2.data()->y()) + translate);
    }

     m_canvas->updateCanvas();
}


void KisAssistantTool::keyPressEvent(QKeyEvent *event)
{
    // When the user is in the middle of creating a new
    // assistant the escape key can be used to cancel this process.
    if (event->key()==Qt::Key_Escape && (m_newAssistant)) {
        // Clear shared pointer to the assistant being created so
        // it gets cleaned-up
        m_newAssistant.clear();
        m_canvas->updateCanvas();
        event->accept();
    } else {
        event->ignore();
    }
}

void KisAssistantTool::paint(QPainter& _gc, const KoViewConverter &_converter)
{
    QRectF canvasSize = QRectF(QPointF(0, 0), QSizeF(m_canvas->image()->size()));

    // show special display while a new assistant is in the process of being created
    if (m_newAssistant) {

        QColor assistantColor = m_newAssistant->effectiveAssistantColor();
        assistantColor.setAlpha(80);

        m_newAssistant->drawAssistant(_gc, canvasSize, m_canvas->coordinatesConverter(), false, m_canvas, true, false);
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_newAssistant->handles()) {
            QPainterPath path;
            path.addEllipse(QRectF(_converter.documentToView(*handle) -  QPointF(m_handleSize * 0.5, m_handleSize * 0.5), QSizeF(m_handleSize, m_handleSize)));

            _gc.save();
            _gc.setPen(Qt::NoPen);
            _gc.setBrush(assistantColor);
            _gc.drawPath(path);
            _gc.restore();
        }
    }


    Q_FOREACH (KisPaintingAssistantSP assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {

        QColor assistantColor = assistant->effectiveAssistantColor();
        assistantColor.setAlpha(80);

        Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
            QRectF ellipse(_converter.documentToView(*handle) -  QPointF(m_handleSize * 0.5, m_handleSize * 0.5),
                           QSizeF(m_handleSize, m_handleSize));

            // render handles differently if it is the one being dragged.
            if (handle == m_handleDrag || handle == m_handleCombine) {
                QPen stroke(assistantColor, 4);
                _gc.save();
                _gc.setPen(stroke);
                _gc.setBrush(Qt::NoBrush);
                _gc.drawEllipse(ellipse);
                _gc.restore();
            }

        }
    }
}

void KisAssistantTool::removeAllAssistants()
{
    m_origAssistantList = m_canvas->paintingAssistantsDecoration()->assistants();

    m_canvas->viewManager()->canvasResourceProvider()->clearPerspectiveGrids();
    m_canvas->paintingAssistantsDecoration()->removeAll();

    KUndo2Command *removeAssistantCmd = new EditAssistantsCommand(m_canvas, m_origAssistantList, KisPaintingAssistant::cloneAssistantList(m_canvas->paintingAssistantsDecoration()->assistants()));
    m_canvas->viewManager()->undoAdapter()->addCommand(removeAssistantCmd);

    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->updateCanvas();

    m_canvas->paintingAssistantsDecoration()->deselectAssistant();
    updateToolOptionsUI();
}

void KisAssistantTool::loadAssistants()
{
    KoFileDialog dialog(m_canvas->viewManager()->mainWindow(), KoFileDialog::OpenFile, "OpenAssistant");
    dialog.setCaption(i18n("Select an Assistant"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(QStringList() << "application/x-krita-assistant", "application/x-krita-assistant");
    QString filename = dialog.filename();
    if (filename.isEmpty()) return;
    if (!QFileInfo(filename).exists()) return;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QByteArray data = file.readAll();
    QXmlStreamReader xml(data);
    QMap<int, KisPaintingAssistantHandleSP> handleMap;
    QMap<int, KisPaintingAssistantHandleSP> sideHandleMap;
    KisPaintingAssistantSP assistant;
    bool errors = false;
    while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xml.name() == "handle") {
                if (assistant && !xml.attributes().value("ref").isEmpty()) {
                    KisPaintingAssistantHandleSP handle = handleMap.value(xml.attributes().value("ref").toString().toInt());
                    if (handle) {
                       assistant->addHandle(handle, HandleType::NORMAL);
                    } else {
                        errors = true;
                    }
                } else {
                    QString strId = xml.attributes().value("id").toString(),
                            strX = xml.attributes().value("x").toString(),
                            strY = xml.attributes().value("y").toString();



                    if (!strId.isEmpty() && !strX.isEmpty() && !strY.isEmpty()) {
                        int id = strId.toInt();
                        double x = strX.toDouble(),
                                y = strY.toDouble();
                        if (!handleMap.contains(id)) {
                            handleMap.insert(id, new KisPaintingAssistantHandle(x, y));
                        } else {
                            errors = true;
                        }
                    } else {
                        errors = true;
                    }
                }
	    // for vanishing point assistant
            } else if (xml.name() == "sidehandle"){

	      // read in sidehandles
              if (!xml.attributes().value("id").isEmpty()) {
		  QString strId = xml.attributes().value("id").toString(),
			  strX = xml.attributes().value("x").toString(),
			  strY = xml.attributes().value("y").toString();
		  if (!strId.isEmpty() && !strX.isEmpty() && !strY.isEmpty()) {
		      int id = strId.toInt();
		      double x = strX.toDouble();
		      double y = strY.toDouble();
		      if (!sideHandleMap.contains(id)) {
			  sideHandleMap.insert(id, new KisPaintingAssistantHandle(x,y));
		      }}
              }
	      // addHandle to assistant
              if (!xml.attributes().value("ref").isEmpty() && assistant) {
		  KisPaintingAssistantHandleSP handle = sideHandleMap.value(xml.attributes().value("ref").toString().toInt());
		  if (handle) {
		      assistant->addHandle(handle, HandleType::SIDE);
		  }
	      }

            } else if (xml.name() == "assistant") {
                const KisPaintingAssistantFactory* factory = KisPaintingAssistantFactoryRegistry::instance()->get(xml.attributes().value("type").toString());

                if (factory) {
                    if (assistant) {
                        errors = true;
                        assistant.clear();
                    }
                    assistant = toQShared(factory->createPaintingAssistant());
                } else {
                    errors = true;
                }

                if (assistant) {
                    // load custom shared assistant properties
                    if (xml.attributes().hasAttribute("useCustomColor")) {
                        QStringRef useCustomColor = xml.attributes().value("useCustomColor");

                        bool usingColor = false;
                        if (useCustomColor.toString() == "1") {
                            usingColor = true;
                        }
                        assistant->setUseCustomColor(usingColor);
                    }

                    if ( xml.attributes().hasAttribute("useCustomColor")) {
                        QStringRef customColor = xml.attributes().value("customColor");
                        assistant->setAssistantCustomColor( KisDomUtils::qStringToQColor(customColor.toString()) );

                    }
                }
           }

            if (assistant) {
                assistant->loadCustomXml(&xml);
            }


           break;
        case QXmlStreamReader::EndElement:
            if (xml.name() == "assistant") {
                if (assistant) {
                    if (assistant->handles().size() == assistant->numHandles()) {
		        if (assistant->id() == "vanishing point" && sideHandleMap.empty()){
                        // Create side handles if the saved vp assistant doesn't have any.
                            QPointF pos = *assistant->handles()[0];
                            assistant->addHandle(new KisPaintingAssistantHandle(pos+QPointF(-70,0)), HandleType::SIDE);
                            assistant->addHandle(new KisPaintingAssistantHandle(pos+QPointF(-140,0)), HandleType::SIDE);
                            assistant->addHandle(new KisPaintingAssistantHandle(pos+QPointF(70,0)), HandleType::SIDE);
                            assistant->addHandle(new KisPaintingAssistantHandle(pos+QPointF(140,0)), HandleType::SIDE);
                        }
                        m_canvas->paintingAssistantsDecoration()->addAssistant(assistant);
                        KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant.data());
                        if (grid) {
                            m_canvas->viewManager()->canvasResourceProvider()->addPerspectiveGrid(grid);
                        }
                    } else {
                        errors = true;
                    }
                    assistant.clear();
                }
            }

            break;
        default:
            break;
        }

    }
    if (assistant) {
        errors = true;
        assistant.clear();
    }
    if (xml.hasError()) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), xml.errorString());
    }
    if (errors) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Errors were encountered. Not all assistants were successfully loaded."));
    }
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->updateCanvas();

}

void KisAssistantTool::saveAssistants()
{

    if (m_handles.isEmpty()) return;

    QByteArray data;
    QXmlStreamWriter xml(&data);
    xml.writeStartDocument();
    xml.writeStartElement("paintingassistant");
    xml.writeAttribute("color",
                       KisDomUtils::qColorToQString(
                           m_canvas->paintingAssistantsDecoration()->globalAssistantsColor())); // global color if no custom color used


    xml.writeStartElement("handles");
    QMap<KisPaintingAssistantHandleSP, int> handleMap;
    Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
        int id = handleMap.size();
        handleMap.insert(handle, id);
        xml.writeStartElement("handle");
        //xml.writeAttribute("type", handle->handleType());
        xml.writeAttribute("id", QString::number(id));
        xml.writeAttribute("x", QString::number(double(handle->x()), 'f', 3));
        xml.writeAttribute("y", QString::number(double(handle->y()), 'f', 3));
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeStartElement("sidehandles");
    QMap<KisPaintingAssistantHandleSP, int> sideHandleMap;
    Q_FOREACH (KisPaintingAssistantSP assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {
	Q_FOREACH (KisPaintingAssistantHandleSP handle, assistant->sideHandles()) {
	    int id = sideHandleMap.size();
	    sideHandleMap.insert(handle, id);
	    xml.writeStartElement("sidehandle");
	    xml.writeAttribute("id", QString::number(id));
	    xml.writeAttribute("x", QString::number(double(handle->x()), 'f', 3));
	    xml.writeAttribute("y", QString::number(double(handle->y()), 'f', 3));
	    xml.writeEndElement();
	}
    }
    xml.writeStartElement("assistants");


    Q_FOREACH (const KisPaintingAssistantSP assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {
        xml.writeStartElement("assistant");
        xml.writeAttribute("type", assistant->id());
        xml.writeAttribute("useCustomColor", QString::number(assistant->useCustomColor()));
        xml.writeAttribute("customColor",  KisDomUtils::qColorToQString(assistant->assistantCustomColor()));



        // custom assistant properties like angle density on vanishing point
        assistant->saveCustomXml(&xml);

        // handle information
        xml.writeStartElement("handles");
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->handles()) {
            xml.writeStartElement("handle");
            xml.writeAttribute("ref", QString::number(handleMap.value(handle)));
            xml.writeEndElement();
        }
        xml.writeEndElement();
        if (!sideHandleMap.empty()) {
	    xml.writeStartElement("sidehandles");
	    Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->sideHandles()) {
		xml.writeStartElement("sidehandle");
		xml.writeAttribute("ref", QString::number(sideHandleMap.value(handle)));
		xml.writeEndElement();
	    }
	    xml.writeEndElement();
        }
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    KoFileDialog dialog(m_canvas->viewManager()->mainWindow(), KoFileDialog::SaveFile, "OpenAssistant");
    dialog.setCaption(i18n("Save Assistant"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(QStringList() << "application/x-krita-assistant", "application/x-krita-assistant");
    QString filename = dialog.filename();
    if (filename.isEmpty()) return;

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(data);
}

QWidget *KisAssistantTool::createOptionWidget()
{
    if (!m_optionsWidget) {
        m_optionsWidget = new QWidget;
        m_options.setupUi(m_optionsWidget);

        // See https://bugs.kde.org/show_bug.cgi?id=316896
        QWidget *specialSpacer = new QWidget(m_optionsWidget);
        specialSpacer->setObjectName("SpecialSpacer");
        specialSpacer->setFixedSize(0, 0);
        m_optionsWidget->layout()->addWidget(specialSpacer);

        m_options.loadAssistantButton->setIcon(KisIconUtils::loadIcon("document-open"));
        m_options.saveAssistantButton->setIcon(KisIconUtils::loadIcon("document-save"));
        m_options.deleteAllAssistantsButton->setIcon(KisIconUtils::loadIcon("edit-delete"));

        QList<KoID> assistants;
        Q_FOREACH (const QString& key, KisPaintingAssistantFactoryRegistry::instance()->keys()) {
            QString name = KisPaintingAssistantFactoryRegistry::instance()->get(key)->name();
            assistants << KoID(key, name);
        }
        std::sort(assistants.begin(), assistants.end(), KoID::compareNames);
        Q_FOREACH(const KoID &id, assistants) {
            m_options.availableAssistantsComboBox->addItem(id.name(), id.id());
        }

        connect(m_options.saveAssistantButton, SIGNAL(clicked()), SLOT(saveAssistants()));
        connect(m_options.loadAssistantButton, SIGNAL(clicked()), SLOT(loadAssistants()));
        connect(m_options.deleteAllAssistantsButton, SIGNAL(clicked()), SLOT(removeAllAssistants()));

        connect(m_options.assistantsColor, SIGNAL(changed(QColor)), SLOT(slotGlobalAssistantsColorChanged(QColor)));
        connect(m_options.assistantsGlobalOpacitySlider, SIGNAL(valueChanged(int)), SLOT(slotGlobalAssistantOpacityChanged()));

        connect(m_options.vanishingPointAngleSpinbox, SIGNAL(valueChanged(double)), this, SLOT(slotChangeVanishingPointAngle(double)));

        // initialize UI elements with existing data if possible
        if (m_canvas && m_canvas->paintingAssistantsDecoration()) {
            const QColor color = m_canvas->paintingAssistantsDecoration()->globalAssistantsColor();

            QColor opaqueColor = color;
            opaqueColor.setAlpha(255);

            m_options.assistantsColor->setColor(opaqueColor);
            m_options.customAssistantColorButton->setColor(opaqueColor);
            m_options.assistantsGlobalOpacitySlider->setValue(color.alphaF() * 100.0);

        } else {
            m_options.assistantsColor->setColor(QColor(176, 176, 176, 255)); // grey default for all assistants
            m_options.assistantsGlobalOpacitySlider->setValue(100); // 100%
        }

        m_options.assistantsGlobalOpacitySlider->setPrefix(i18n("Opacity: "));
        m_options.assistantsGlobalOpacitySlider->setSuffix(" %");


        // custom color of selected assistant
        m_options.customColorOpacitySlider->setValue(100); // 100%
        m_options.customColorOpacitySlider->setPrefix(i18n("Opacity: "));
        m_options.customColorOpacitySlider->setSuffix(" %");

        connect(m_options.useCustomAssistantColor, SIGNAL(clicked(bool)), this, SLOT(slotUpdateCustomColor()));
        connect(m_options.customAssistantColorButton, SIGNAL(changed(QColor)), this, SLOT(slotUpdateCustomColor()));
        connect(m_options.customColorOpacitySlider, SIGNAL(valueChanged(int)), SLOT(slotCustomOpacityChanged()));

        m_options.vanishingPointAngleSpinbox->setPrefix(i18n("Density: "));
        m_options.vanishingPointAngleSpinbox->setSuffix(QChar(Qt::Key_degree));
        m_options.vanishingPointAngleSpinbox->setRange(1.0, 180.0);
        m_options.vanishingPointAngleSpinbox->setSingleStep(1.0);


        m_options.vanishingPointAngleSpinbox->setVisible(false);

    }

    updateToolOptionsUI();

    return m_optionsWidget;
}

void KisAssistantTool::slotGlobalAssistantsColorChanged(const QColor& setColor)
{
    // color and alpha are stored separately, so we need to merge the values before sending it on
    int oldAlpha = m_canvas->paintingAssistantsDecoration()->globalAssistantsColor().alpha();

    QColor newColor = setColor;
    newColor.setAlpha(oldAlpha);

    m_canvas->paintingAssistantsDecoration()->setGlobalAssistantsColor(newColor);

    m_canvas->paintingAssistantsDecoration()->uncache();
    m_canvas->canvasWidget()->update();
}

void KisAssistantTool::slotGlobalAssistantOpacityChanged()
{
    QColor newColor = m_canvas->paintingAssistantsDecoration()->globalAssistantsColor();
    qreal newOpacity = m_options.assistantsGlobalOpacitySlider->value() * 0.01 * 255.0;
    newColor.setAlpha(int(newOpacity));
    m_canvas->paintingAssistantsDecoration()->setGlobalAssistantsColor(newColor);

    m_canvas->paintingAssistantsDecoration()->uncache();
    m_canvas->canvasWidget()->update();
}

void KisAssistantTool::slotUpdateCustomColor()
{
    // get the selected assistant and change the angle value
    KisPaintingAssistantSP m_selectedAssistant =  m_canvas->paintingAssistantsDecoration()->selectedAssistant();
    if (m_selectedAssistant) {
        m_selectedAssistant->setUseCustomColor(m_options.useCustomAssistantColor->isChecked());

        // changing color doesn't keep alpha, so update that before we send it on
        QColor newColor = m_options.customAssistantColorButton->color();
        newColor.setAlpha(m_selectedAssistant->assistantCustomColor().alpha());

        m_selectedAssistant->setAssistantCustomColor(newColor);
        m_selectedAssistant->uncache();
    }

    updateToolOptionsUI();
    m_canvas->canvasWidget()->update();
}

void KisAssistantTool::slotCustomOpacityChanged()
{
    KisPaintingAssistantSP m_selectedAssistant =  m_canvas->paintingAssistantsDecoration()->selectedAssistant();
    if (m_selectedAssistant) {
        QColor newColor = m_selectedAssistant->assistantCustomColor();
        qreal newOpacity = m_options.customColorOpacitySlider->value() * 0.01 * 255.0;
        newColor.setAlpha(int(newOpacity));
        m_selectedAssistant->setAssistantCustomColor(newColor);
        m_selectedAssistant->uncache();
    }

    // this forces the canvas to refresh to see the changes immediately
    m_canvas->paintingAssistantsDecoration()->uncache();
    m_canvas->canvasWidget()->update();
}

void KisAssistantTool::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(action);
    setMode(KisTool::PAINT_MODE);

    if (m_newAssistant && m_internalMode == MODE_CREATION) {
        KisPaintingAssistantsDecorationSP canvasDecoration = m_canvas->paintingAssistantsDecoration();

        *m_newAssistant->handles().back() = event->point;

        if (!snap(event)) {
            *m_newAssistant->handles().back() = canvasDecoration->snapToGuide(event, QPointF(), false);
        }

        if (m_newAssistant->handles().size() == m_newAssistant->numHandles()) {
            addAssistant();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(canvasDecoration->snapToGuide(event, QPointF(), false)), HandleType::NORMAL);
        }
        m_canvas->updateCanvas();
        return;
    }
}

void KisAssistantTool::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(action);
    event->ignore();
}

void KisAssistantTool::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    Q_UNUSED(action);
    setMode(KisTool::HOVER_MODE);
    event->ignore();
    m_canvas->updateCanvas();
}

bool KisAssistantTool::snap(KoPointerEvent *event)
{
    if ((event->modifiers() & Qt::ShiftModifier) == false) {
        return false;
    } else {

        if (m_handleDrag) {
            if (m_snapIsRadial == true) {
                QLineF dragRadius = QLineF(m_dragStart, event->point);
                dragRadius.setLength(m_radius.length());
                *m_handleDrag = dragRadius.p2();
            } else {
                QPointF snap_point = snapToClosestAxis(event->point - m_dragStart);
                *m_handleDrag = m_dragStart + snap_point;
            }

        } else {
            if (m_newAssistant && m_internalMode == MODE_CREATION) {
                QList<KisPaintingAssistantHandleSP> handles = m_newAssistant->handles();
                KisPaintingAssistantHandleSP handle_snap = handles.back();
                // for any assistant, snap 2nd handle to x or y axis relative to first handle
                if (handles.size() == 2) {
                    QPointF snap_point = snapToClosestAxis(event->point - *handles[0]);
                    *handle_snap =  *handles[0] + snap_point;
                } else {
                    bool was_snapped = false;
                    if (m_newAssistant->id() == "spline") {
                        KisPaintingAssistantHandleSP start;
                        handles.size() == 3 ? start = handles[0] : start = handles[1];
                        QPointF snap_point = snapToClosestAxis(event->point - *start);
                        *handle_snap =  *start + snap_point;
                        was_snapped = true;
                    }

                    if (m_newAssistant->id() == "ellipse" ||
                        m_newAssistant->id() == "concentric ellipse" ||
                        m_newAssistant->id() == "fisheye-point") {
                        QPointF center = QLineF(*handles[0], *handles[1]).center();
                        QLineF radius = QLineF(center,*handles[0]);
                        QLineF dragRadius = QLineF(center, event->point);
                        dragRadius.setLength(radius.length());
                        *handle_snap = dragRadius.p2();
                        was_snapped = true;
                    }

                    if (m_newAssistant->id() == "perspective") {
                        KisPaintingAssistantHandleSP start;
                        handles.size() == 3 ? start = handles[1] : start = handles[2];
                        QPointF snap_point = snapToClosestAxis(event->point - *start);
                        *handle_snap =  *start + snap_point;
                        was_snapped = true;
                    }
                    return was_snapped;
                }
            }
        }
        return true;
    }
}
