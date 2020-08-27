/*
 *  kis_cmb_composite.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include "kis_cmb_composite.h"

#include <KoCompositeOp.h>
#include <KoCompositeOpRegistry.h>

#include "kis_composite_ops_model.h"
#include "kis_categorized_item_delegate.h"
#include <kis_action.h>
#include <QWheelEvent>

//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpListWidget ------------------------------------------------------ //

KisCompositeOpListWidget::KisCompositeOpListWidget(QWidget* parent):
    KisCategorizedListView(parent),
    m_model(new KisSortedCompositeOpListModel(false, this))
{
    setModel(m_model);
    setItemDelegate(new KisCategorizedItemDelegate(this));
}

KisCompositeOpListWidget::~KisCompositeOpListWidget()
{
}

KoID KisCompositeOpListWidget::selectedCompositeOp() const {
    KoID op;

    if (m_model->entryAt(op, currentIndex())) {
        return op;
    }

    return KoCompositeOpRegistry::instance().getDefaultCompositeOp();
}

//////////////////////////////////////////////////////////////////////////////////////////
// ---- KisCompositeOpComboBox -------------------------------------------------------- //

KisCompositeOpComboBox::KisCompositeOpComboBox(QWidget* parent)
    : KisCompositeOpComboBox(false, parent)
{
}

KisCompositeOpComboBox::KisCompositeOpComboBox(bool limitToLayerStyles, QWidget* parent)
    : KisSqueezedComboBox(parent),
      m_model(new KisSortedCompositeOpListModel(limitToLayerStyles, this)),
      m_allowToHidePopup(true)
{
    m_view = new KisCategorizedListView();
    m_view->setCompositeBoxControl(true);

    setMaxVisibleItems(100);
    setSizeAdjustPolicy(AdjustToContents);
    m_view->setResizeMode(QListView::Adjust);

    setToolTip(i18n("Blending Mode"));

    setModel(m_model);
    setView(m_view);
    setItemDelegate(new KisCategorizedItemDelegate(this));

    connect(m_view, SIGNAL(sigCategoryToggled(QModelIndex,bool)), SLOT(slotCategoryToggled(QModelIndex,bool)));
    connect(m_view, SIGNAL(sigEntryChecked(QModelIndex)), SLOT(slotEntryChecked(QModelIndex)));

    selectCompositeOp(KoCompositeOpRegistry::instance().getDefaultCompositeOp());
}

KisCompositeOpComboBox::~KisCompositeOpComboBox()
{
    delete m_view;
}

QList<KisAction *> KisCompositeOpComboBox::createBlendmodeActions()
{
    QList<KisAction *> actions;

    KisAction *action = 0;
//    //    Cycle through blending modes
//    //    Shift + + (plus) or – (minus)
//    KisAction *action = new KisAction(i18n("Next Blending Mode"), this);
//    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Plus));
//    connect(action, SIGNAL(triggered()), SLOT(slotNextBlendingMode()));
//    m_actions << action;

//    action = new KisAction(i18n("Previous Blending Mode"), this);
//    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT +  Qt::Key_Minus));
//    connect(action, SIGNAL(triggered()), SLOT(slotPreviousBlendingMode()));
//    m_actions << action;

    //    Normal
    //    Shift + Alt + N
    action = new KisAction(i18n("Select Normal Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_N));
    connect(action, SIGNAL(triggered()), SLOT(slotNormal()));
    actions << action;

    //    Dissolve
    //    Shift + Alt + I
    action = new KisAction(i18n("Select Dissolve Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_I));
    connect(action, SIGNAL(triggered()), SLOT(slotDissolve()));
    actions << action;

    //    Behind (Brush tool only)
    //    Shift + Alt + Q
    action = new KisAction(i18n("Select Behind Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Q));
    connect(action, SIGNAL(triggered()), SLOT(slotBehind()));
    actions << action;

    //    Clear (Brush tool only)
    //    Shift + Alt + R
    action = new KisAction(i18n("Select Clear Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_R));
    connect(action, SIGNAL(triggered()), SLOT(slotClear()));
    actions << action;

    //    Darken
    //    Shift + Alt + K
    action = new KisAction(i18n("Select Darken Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_K));
    connect(action, SIGNAL(triggered()), SLOT(slotDarken()));
    actions << action;

    //    Multiply
    //    Shift + Alt + M
    action = new KisAction(i18n("Select Multiply Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_M));
    connect(action, SIGNAL(triggered()), SLOT(slotMultiply()));
    actions << action;

    //    Color Burn
    //    Shift + Alt + B
    action = new KisAction(i18n("Select Color Burn Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_B));
    connect(action, SIGNAL(triggered()), SLOT(slotColorBurn()));
    actions << action;

    //    Linear Burn
    //    Shift + Alt + A
    action = new KisAction(i18n("Select Linear Burn Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_A));
    connect(action, SIGNAL(triggered()), SLOT(slotLinearBurn()));
    actions << action;

    //    Lighten
    //    Shift + Alt + G
    action = new KisAction(i18n("Select Lighten Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_G));
    connect(action, SIGNAL(triggered()), SLOT(slotLighten()));
    actions << action;

    //    Screen
    //    Shift + Alt + S
    action = new KisAction(i18n("Select Screen Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_S));
    connect(action, SIGNAL(triggered()), SLOT(slotScreen()));
    actions << action;

    //    Color Dodge
    //    Shift + Alt + D
    action = new KisAction(i18n("Select Color Dodge Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_D));
    connect(action, SIGNAL(triggered()), SLOT(slotColorDodge()));
    actions << action;

    //    Linear Dodge
    //    Shift + Alt + W
    action = new KisAction(i18n("Select Linear Dodge Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_W));
    connect(action, SIGNAL(triggered()), SLOT(slotLinearDodge()));
    actions << action;

    //    Overlay
    //    Shift + Alt + O
    action = new KisAction(i18n("Select Overlay Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_O));
    connect(action, SIGNAL(triggered()), SLOT(slotOverlay()));
    actions << action;

    //    Hard Overlay
    //    Shift + Alt + P
    action = new KisAction(i18n("Select Hard Overlay Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_P));
    connect(action, SIGNAL(triggered()), SLOT(slotHardOverlay()));
    actions << action;

    //    Soft Light
    //    Shift + Alt + F
    action = new KisAction(i18n("Select Soft Light Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_F));
    connect(action, SIGNAL(triggered()), SLOT(slotSoftLight()));
    actions << action;

    //    Hard Light
    //    Shift + Alt + H
    action = new KisAction(i18n("Select Hard Light Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_H));
    connect(action, SIGNAL(triggered()), SLOT(slotHardLight()));
    actions << action;

    //    Vivid Light
    //    Shift + Alt + V
    action = new KisAction(i18n("Select Vivid Light Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_V));
    connect(action, SIGNAL(triggered()), SLOT(slotVividLight()));
    actions << action;

    //    Linear Light
    //    Shift + Alt + J
    action = new KisAction(i18n("Select Linear Light Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_J));
    connect(action, SIGNAL(triggered()), SLOT(slotLinearLight()));
    actions << action;

    //    Pin Light
    //    Shift + Alt + Z
    action = new KisAction(i18n("Select Pin Light Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Z));
    connect(action, SIGNAL(triggered()), SLOT(slotPinLight()));
    actions << action;

    //    Hard Mix
    //    Shift + Alt + L
    action = new KisAction(i18n("Select Hard Mix Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_L));
    connect(action, SIGNAL(triggered()), SLOT(slotHardMix()));
    actions << action;

    //    Difference
    //    Shift + Alt + E
    action = new KisAction(i18n("Select Difference Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_E));
    connect(action, SIGNAL(triggered()), SLOT(slotDifference()));
    actions << action;

    //    Exclusion
    //    Shift + Alt + X
    action = new KisAction(i18n("Select Exclusion Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_X));
    connect(action, SIGNAL(triggered()), SLOT(slotExclusion()));
    actions << action;

    //    Hue
    //    Shift + Alt + U
    action = new KisAction(i18n("Select Hue Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_U));
    connect(action, SIGNAL(triggered()), SLOT(slotHue()));
    actions << action;

    //    Saturation
    //    Shift + Alt + T
    action = new KisAction(i18n("Select Saturation Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_T));
    connect(action, SIGNAL(triggered()), SLOT(slotSaturation()));
    actions << action;

    //    Color
    //    Shift + Alt + C
    action = new KisAction(i18n("Select Color Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_C));
    connect(action, SIGNAL(triggered()), SLOT(slotColor()));
    actions << action;

    //    Luminosity
    //    Shift + Alt + Y
    action = new KisAction(i18n("Select Luminosity Blending Mode"), this);
    action->setDefaultShortcut(QKeySequence(Qt::SHIFT + Qt::ALT + Qt::Key_Y));
    connect(action, SIGNAL(triggered()), SLOT(slotLuminosity()));
    actions << action;

    return actions;
}

void KisCompositeOpComboBox::validate(const KoColorSpace *cs) {
    m_model->validate(cs);
}

void KisCompositeOpComboBox::selectCompositeOp(const KoID &op) {
    KoID currentOp;
    if (m_model->entryAt(currentOp, m_model->index(currentIndex(), 0)) &&
            currentOp == op) {

        return;
    }

    QModelIndex index = m_model->indexOf(op);

    setCurrentIndex(index.row());
    emit activated(index.row());
    emit activated(op.name());
}

KoID KisCompositeOpComboBox::selectedCompositeOp() const {
    KoID op;

    if (m_model->entryAt(op, m_model->index(currentIndex(), 0))) {
        return op;
    }
    return KoCompositeOpRegistry::instance().getDefaultCompositeOp();
}

void KisCompositeOpComboBox::slotCategoryToggled(const QModelIndex& index, bool toggled)
{
    Q_UNUSED(index);
    Q_UNUSED(toggled);

    //NOTE: this will (should) fit the size of the
    //      popup widget to the view
    //      don't know if this is expected behaviour
    //      on all supported platforms.
    //      There is nothing written about this in the docs.
    showPopup();
}

void KisCompositeOpComboBox::slotEntryChecked(const QModelIndex& index)
{
    Q_UNUSED(index);
    m_allowToHidePopup = false;
}

void KisCompositeOpComboBox::hidePopup()
{
    if (m_allowToHidePopup) {
        QComboBox::hidePopup();
    }
    else  {
        QComboBox::showPopup();
    }

    m_allowToHidePopup = true;
}

void KisCompositeOpComboBox::slotNextBlendingMode()
{
    if (currentIndex() < count()) {
        setCurrentIndex(currentIndex() + 1);
    }
}

void KisCompositeOpComboBox::slotPreviousBlendingMode()
{
    if (currentIndex() > 0) {
        setCurrentIndex(currentIndex() - 1);
    }
}

void KisCompositeOpComboBox::slotNormal()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_OVER));
}

void KisCompositeOpComboBox::slotDissolve()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DISSOLVE));
}

void KisCompositeOpComboBox::slotBehind()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_BEHIND));
}

void KisCompositeOpComboBox::slotClear()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_CLEAR));
}

void KisCompositeOpComboBox::slotDarken()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DARKEN));
}

void KisCompositeOpComboBox::slotMultiply()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_MULT));
}

void KisCompositeOpComboBox::slotColorBurn()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_BURN));
}

void KisCompositeOpComboBox::slotLinearBurn()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_BURN));
}

void KisCompositeOpComboBox::slotLighten()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LIGHTEN));
}

void KisCompositeOpComboBox::slotScreen()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_SCREEN));
}

void KisCompositeOpComboBox::slotColorDodge()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DODGE));
}

void KisCompositeOpComboBox::slotLinearDodge()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_DODGE));
}

void KisCompositeOpComboBox::slotOverlay()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_OVERLAY));
}

void KisCompositeOpComboBox::slotHardOverlay()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_OVERLAY));
}

void KisCompositeOpComboBox::slotSoftLight()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_SOFT_LIGHT_PHOTOSHOP));
}

void KisCompositeOpComboBox::slotHardLight()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_LIGHT));
}

void KisCompositeOpComboBox::slotVividLight()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_VIVID_LIGHT));
}

void KisCompositeOpComboBox::slotLinearLight()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LINEAR_LIGHT));
}

void KisCompositeOpComboBox::slotPinLight()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_PIN_LIGHT));
}

void KisCompositeOpComboBox::slotHardMix()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HARD_MIX_PHOTOSHOP));
}

void KisCompositeOpComboBox::slotDifference()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_DIFF));
}

void KisCompositeOpComboBox::slotExclusion()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_EXCLUSION));
}

void KisCompositeOpComboBox::slotHue()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_HUE));
}

void KisCompositeOpComboBox::slotSaturation()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_SATURATION));
}

void KisCompositeOpComboBox::slotColor()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_COLOR));
}

void KisCompositeOpComboBox::slotLuminosity()
{
    selectCompositeOp(KoCompositeOpRegistry::instance().getKoID(COMPOSITE_LUMINIZE));
}

void KisCompositeOpComboBox::wheelEvent(QWheelEvent *e)
{
    /**
     * This code is a copy of QComboBox::wheelEvent. It does the same thing,
     * except that it skips "Category" items, by checking m_model->entryAt()
     * on each step.
     */

    QStyleOptionComboBox opt;
    initStyleOption(&opt);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    if (style()->styleHint(QStyle::SH_ComboBox_AllowWheelScrolling, &opt, this)) {
#else
    if (1) {
#endif
        const int rowCount = count();
        int newIndex = currentIndex();

        QAbstractItemModel *model = this->model();
        KoID op;

        if (e->delta() > 0) {
            newIndex--;
            while ((newIndex >= 0) &&
                   (!(model->flags(model->index(newIndex, modelColumn(), rootModelIndex())) & Qt::ItemIsEnabled) ||
                    !m_model->entryAt(op, m_model->index(newIndex, modelColumn()))))

                newIndex--;
        } else if (e->delta() < 0) {
            newIndex++;
            while (newIndex < rowCount &&
                   (!(model->index(newIndex, modelColumn(), rootModelIndex()).flags() & Qt::ItemIsEnabled) ||
                    !m_model->entryAt(op, m_model->index(newIndex, modelColumn()))))

                newIndex++;
        }

        if (newIndex >= 0 && newIndex < rowCount && newIndex != currentIndex()) {
            setCurrentIndex(newIndex);

            emit activated(newIndex);
            if (m_model->entryAt(op, m_model->index(newIndex, 0))) {
                emit activated(op.name());
            }
        }
        e->accept();
    } else {
        KisSqueezedComboBox::wheelEvent(e);
    }
}

void KisCompositeOpComboBox::keyPressEvent(QKeyEvent *e)
{
    /**
     * This code is a copy of QComboBox::keyPressEvent. It does the same thing,
     * except that it skips "Category" items, by checking m_model->entryAt()
     * on each step.
     */

    enum Move { NoMove=0 , MoveUp , MoveDown , MoveFirst , MoveLast};

    Move move = NoMove;
    int newIndex = currentIndex();
    switch (e->key()) {
    case Qt::Key_Up:
        if (e->modifiers() & Qt::ControlModifier)
            break; // pass to line edit for auto completion
        Q_FALLTHROUGH();
    case Qt::Key_PageUp:
        move = MoveUp;
        break;
    case Qt::Key_Down:
        if (e->modifiers() & Qt::AltModifier) {
            showPopup();
            return;
        } else if (e->modifiers() & Qt::ControlModifier)
            break; // pass to line edit for auto completion
        Q_FALLTHROUGH();
    case Qt::Key_PageDown:
        move = MoveDown;
        break;
    case Qt::Key_Home:
        move = MoveFirst;
        break;
    case Qt::Key_End:
        move = MoveLast;
        break;
    case Qt::Key_F4:
        if (!e->modifiers()) {
            showPopup();
            return;
        }
        break;
    case Qt::Key_Space:
        showPopup();
        return;
    default:
        break;
    }

    const int rowCount = count();

    if (move != NoMove) {
        KoID op;

        e->accept();
        switch (move) {
        case MoveFirst:
            newIndex = -1;
            Q_FALLTHROUGH();
        case MoveDown:
            newIndex++;
            while (newIndex < rowCount &&
                   (!(model()->index(newIndex, modelColumn(), rootModelIndex()).flags() & Qt::ItemIsEnabled) ||
                    !m_model->entryAt(op, m_model->index(newIndex, modelColumn()))))
                newIndex++;
            break;
        case MoveLast:
            newIndex = rowCount;
            Q_FALLTHROUGH();
        case MoveUp:
            newIndex--;
            while ((newIndex >= 0) &&
                   (!(model()->flags(model()->index(newIndex, modelColumn(), rootModelIndex())) & Qt::ItemIsEnabled) ||
                    !m_model->entryAt(op, m_model->index(newIndex, modelColumn()))))
                newIndex--;
            break;
        default:
            e->ignore();
            break;
        }

        if (newIndex >= 0 && newIndex < rowCount && newIndex != currentIndex()) {
            setCurrentIndex(newIndex);
            emit activated(newIndex);

            if (m_model->entryAt(op, m_model->index(newIndex, 0))) {
                emit activated(op.name());
            }
        }
    } else {
        KisSqueezedComboBox::keyPressEvent(e);
    }
}

KisLayerStyleCompositeOpComboBox::KisLayerStyleCompositeOpComboBox(QWidget* parent)
    : KisCompositeOpComboBox(true, parent)
{
}
