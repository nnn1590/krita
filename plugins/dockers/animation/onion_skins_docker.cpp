/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "onion_skins_docker.h"
#include "ui_onion_skins_docker.h"

#include <QSlider>
#include <QFrame>
#include <QGridLayout>
#include <QEvent>

#include "kis_icon_utils.h"
#include "kis_image_config.h"
#include "kis_onion_skin_compositor.h"
#include "kis_signals_blocker.h"
#include "kis_node_view_color_scheme.h"
#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include <KoColorSpaceRegistry.h>

#include "kis_equalizer_widget.h"
#include "kis_color_label_button.h"

OnionSkinsDocker::OnionSkinsDocker(QWidget *parent) :
    QDockWidget(i18n("Onion Skins"), parent),
    ui(new Ui::OnionSkinsDocker),
    m_updatesCompressor(300, KisSignalCompressor::FIRST_ACTIVE),
    m_toggleOnionSkinsAction(0)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    KisImageConfig config(true);
    ui->setupUi(mainWidget);

    mainWidget->setContentsMargins(10, 10, 10, 10);


    ui->doubleTintFactor->setMinimum(0);
    ui->doubleTintFactor->setMaximum(100);
    ui->doubleTintFactor->setPrefix(i18n("Tint: "));
    ui->doubleTintFactor->setSuffix(i18n("%"));

    ui->btnBackwardColor->setToolTip(i18n("Tint color for past frames"));
    ui->btnForwardColor->setToolTip(i18n("Tint color for future frames"));


    QVBoxLayout *layout = ui->slidersLayout;

    m_equalizerWidget = new KisEqualizerWidget(10, this);
    connect(m_equalizerWidget, SIGNAL(sigConfigChanged()), &m_updatesCompressor, SLOT(start()));
    layout->addWidget(m_equalizerWidget, 1);

    connect(ui->btnBackwardColor, SIGNAL(changed(KoColor)), &m_updatesCompressor, SLOT(start()));
    connect(ui->btnForwardColor, SIGNAL(changed(KoColor)), &m_updatesCompressor, SLOT(start()));
    connect(ui->doubleTintFactor, SIGNAL(valueChanged(qreal)), &m_updatesCompressor, SLOT(start()));

    connect(&m_updatesCompressor, SIGNAL(timeout()),
            SLOT(changed()));


    {
        const bool isShown = config.showAdditionalOnionSkinsSettings();
        ui->btnShowHide->setChecked(isShown);
        connect(ui->btnShowHide, SIGNAL(toggled(bool)), SLOT(slotShowAdditionalSettings(bool)));
        slotShowAdditionalSettings(isShown);
    }

    {
        KisNodeViewColorScheme scm;
        m_filterButtonGroup = new KisColorLabelFilterGroup(this);
        m_dragFilter = new KisColorLabelMouseDragFilter(this);
        m_filterButtonGroup->setExclusive(false);
        m_filterButtonGroup->setMinimumRequiredChecked(0);
        QWidget* filterButtonContainer = ui->colorFilterGroupbox;
        QLayout* filterButtonLayout = ui->filterButtons;
        filterButtonLayout->setSpacing(0);
        QVector<QColor> availableColors = scm.allColorLabels();
        QSet<int> viableColors;
        for (int i = 0; i < availableColors.count(); i++) {
            KisColorLabelButton* colorLabelButton = new KisColorLabelButton(availableColors[i], 24, filterButtonContainer);
            filterButtonLayout->addWidget(colorLabelButton);
            m_filterButtonGroup->addButton(colorLabelButton, i);
            colorLabelButton->installEventFilter(m_dragFilter);
            viableColors << i;
        }

        m_filterButtonGroup->setViableLabels(viableColors);

        connect(m_filterButtonGroup, SIGNAL(buttonToggled(int,bool)), this, SLOT(slotFilteredColorsChanged()));
        connect(ui->colorFilterGroupbox, SIGNAL(toggled(bool)), this, SLOT(slotFilteredColorsChanged()));
        connect(ui->resetFilter, SIGNAL(pressed()), m_filterButtonGroup, SLOT(reset()) );
    }

    loadSettings();
    KisOnionSkinCompositor::instance()->configChanged();

    // this mostly hides the checkboxes since no filtering is done by default
    slotFilteredColorsChanged();

    resize(sizeHint());
}


OnionSkinsDocker::~OnionSkinsDocker()
{
    delete ui;
}

void OnionSkinsDocker::setCanvas(KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
}

void OnionSkinsDocker::unsetCanvas()
{
}

void OnionSkinsDocker::setViewManager(KisViewManager *view)
{
    KisActionManager *actionManager = view->actionManager();

    m_toggleOnionSkinsAction = actionManager->createAction("toggle_onion_skin");
    connect(m_toggleOnionSkinsAction, SIGNAL(triggered()), SLOT(slotToggleOnionSkins()));

    slotUpdateIcons();
    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
}

void OnionSkinsDocker::slotToggleOnionSkins()
{
    m_equalizerWidget->toggleMasterSwitch();
}

void OnionSkinsDocker::slotFilteredColorsChanged()
{
    // what colors are selected to filter??
    QSet<int> selectedFilterColors = m_filterButtonGroup->getActiveLabels();

    // show all colors if the filter is off and ignore the checkboxes
    if(ui->colorFilterGroupbox->isChecked() == false) {
        selectedFilterColors.clear();
        selectedFilterColors << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8; // show everything
    }

    m_filterButtonGroup->setAllVisibility(ui->colorFilterGroupbox->isChecked());
    ui->resetFilter->setVisible(ui->colorFilterGroupbox->isChecked());

    // existing code
    KisOnionSkinCompositor::instance()->setColorLabelFilter(QList<int>::fromSet(selectedFilterColors));
    KisOnionSkinCompositor::instance()->configChanged();
}

void OnionSkinsDocker::slotUpdateIcons()
{
    if (m_toggleOnionSkinsAction) {
        m_toggleOnionSkinsAction->setIcon(KisIconUtils::loadIcon("onion_skin_options"));
    }
}

void OnionSkinsDocker::slotShowAdditionalSettings(bool value)
{
    ui->lblPrevColor->setVisible(value);
    ui->lblNextColor->setVisible(value);

    ui->btnBackwardColor->setVisible(value);
    ui->btnForwardColor->setVisible(value);

    ui->doubleTintFactor->setVisible(value);

    QIcon icon = KisIconUtils::loadIcon(value ? "arrow-down" : "arrow-up");
    ui->btnShowHide->setIcon(icon);

    KisImageConfig(false).setShowAdditionalOnionSkinsSettings(value);
}

void OnionSkinsDocker::changed()
{
    KisImageConfig config(false);

    KisEqualizerWidget::EqualizerValues v = m_equalizerWidget->getValues();
    config.setNumberOfOnionSkins(v.maxDistance);

    for (int i = -v.maxDistance; i <= v.maxDistance; i++) {
        config.setOnionSkinOpacity(i, v.value[i] * 255.0 / 100.0);
        config.setOnionSkinState(i, v.state[i]);
    }

    config.setOnionSkinTintFactor(ui->doubleTintFactor->value() * 255.0 / 100.0);
    config.setOnionSkinTintColorBackward(ui->btnBackwardColor->color().toQColor());
    config.setOnionSkinTintColorForward(ui->btnForwardColor->color().toQColor());

    KisOnionSkinCompositor::instance()->configChanged();
}

void OnionSkinsDocker::loadSettings()
{
    KisImageConfig config(true);

    KisSignalsBlocker b(ui->doubleTintFactor,
                        ui->btnBackwardColor,
                        ui->btnForwardColor,
                        m_equalizerWidget);

    ui->doubleTintFactor->setValue(qRound(config.onionSkinTintFactor() * 100.0 / 255));
    KoColor bcol(KoColorSpaceRegistry::instance()->rgb8());
    bcol.fromQColor(config.onionSkinTintColorBackward());
    ui->btnBackwardColor->setColor(bcol);
    bcol.fromQColor(config.onionSkinTintColorForward());
    ui->btnForwardColor->setColor(bcol);

    KisEqualizerWidget::EqualizerValues v;
    v.maxDistance = 10;

    for (int i = -v.maxDistance; i <= v.maxDistance; i++) {
        v.value.insert(i, qRound(config.onionSkinOpacity(i) * 100.0 / 255.0));
        v.state.insert(i, config.onionSkinState(i));
    }

    m_equalizerWidget->setValues(v);
}
