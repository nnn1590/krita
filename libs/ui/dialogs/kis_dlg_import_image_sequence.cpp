/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_import_image_sequence.h"

#include "KisDocument.h"
#include "KisMainWindow.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "KisImportExportManager.h"
#include "KoFileDialog.h"
#include <QStandardPaths>
#include <QRegExp>


class KisDlgImportImageSequence::ListItem : QListWidgetItem {

public:
    ListItem(const QString &text, QListWidget *view, QCollator *collator)
        : QListWidgetItem(text, view)
        , collator(collator)
    {}

    bool operator <(const QListWidgetItem &other) const override
    {
        if (collator->numericMode()) {
            const QRegExp rx(QLatin1Literal("[^0-9]+"));
            QStringList ours = text().split(rx, QString::SkipEmptyParts);
            QStringList theirs = other.text().split(rx, QString::SkipEmptyParts);

            // Let's compare the last numbers -- they are most likely to be the serial numbers
            if (ours.size() > 0 && theirs.size() > 0) {
                return (ours.last().toInt() < theirs.last().toInt());
            }

        }
        return collator->compare(this->text(), other.text()) < 0;
    }

private:
    QCollator *collator;
};

KisDlgImportImageSequence::KisDlgImportImageSequence(KisMainWindow *mainWindow, KisDocument *document) :
    KoDialog(mainWindow),
    m_mainWindow(mainWindow),
    m_document(document)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget * page = new QWidget(this);
    m_ui.setupUi(page);
    setMainWidget(page);

    enableButtonOk(false);

    m_ui.cmbOrder->addItem(i18n("Ascending"), Ascending);
    m_ui.cmbOrder->addItem(i18n("Descending"), Descending);
    m_ui.cmbOrder->setCurrentIndex(0);

    m_ui.cmbSortMode->addItem(i18n("Alphabetical"), Natural);
    m_ui.cmbSortMode->addItem(i18n("Numerical"), Numerical);
    m_ui.cmbSortMode->setCurrentIndex(1);

    m_ui.lstFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_ui.btnAddImages, &QAbstractButton::clicked, this, &KisDlgImportImageSequence::slotAddFiles);
    connect(m_ui.btnRemove, &QAbstractButton::clicked, this, &KisDlgImportImageSequence::slotRemoveFiles);
    connect(m_ui.spinStep, SIGNAL(valueChanged(int)), this, SLOT(slotSkipChanged(int)));
    connect(m_ui.cmbOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrderOptionsChanged(int)));
    connect(m_ui.cmbSortMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrderOptionsChanged(int)));

    // cold initialization of the controls
    slotSkipChanged(m_ui.spinStep->value());
    slotOrderOptionsChanged(m_ui.cmbOrder->currentIndex());
    slotOrderOptionsChanged(m_ui.cmbSortMode->currentIndex());
}

QStringList KisDlgImportImageSequence::files()
{
    QStringList list;

    for (int i=0; i < m_ui.lstFiles->count(); i++) {
        list.append(m_ui.lstFiles->item(i)->text());
    }

    return list;
}

int KisDlgImportImageSequence::firstFrame()
{
    return m_ui.spinFirstFrame->value();
}

int KisDlgImportImageSequence::step()
{
    return m_ui.spinStep->value();
}

void KisDlgImportImageSequence::slotAddFiles()
{
    QStringList urls = showOpenFileDialog();

    if (!urls.isEmpty()) {
        Q_FOREACH(QString url, urls) {
            new ListItem(url, m_ui.lstFiles, &m_collator);
        }

        sortFileList();
    }

    enableButtonOk(m_ui.lstFiles->count() > 0);
}

QStringList KisDlgImportImageSequence::showOpenFileDialog()
{
    KoFileDialog dialog(this, KoFileDialog::ImportFiles, "OpenDocument");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    dialog.setCaption(i18n("Import Images"));

    return dialog.filenames();
}



void KisDlgImportImageSequence::slotRemoveFiles()
{
    QList<QListWidgetItem*> selected = m_ui.lstFiles->selectedItems();

    Q_FOREACH(QListWidgetItem *item, selected) {
        delete item;
    }

    enableButtonOk(m_ui.lstFiles->count() > 0);
}

void KisDlgImportImageSequence::slotSkipChanged(int)
{
    int documentFps = m_document->image()->animationInterface()->framerate();
    float sourceFps = 1.0f * documentFps / m_ui.spinStep->value();

    m_ui.lblFramerate->setText(i18n("Source fps: %1", sourceFps));
}

void KisDlgImportImageSequence::slotOrderOptionsChanged(int)
{
    sortFileList();
}

void KisDlgImportImageSequence::sortFileList()
{
    int order = m_ui.cmbOrder->currentData().toInt();
    bool numeric = m_ui.cmbSortMode->currentData().toInt() == Numerical;

    m_collator.setNumericMode(numeric);
    m_ui.lstFiles->sortItems((order == Ascending) ? Qt::AscendingOrder : Qt::DescendingOrder);
}
