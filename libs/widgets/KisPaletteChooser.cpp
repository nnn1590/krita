/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPointer>
#include <QScopedPointer>
#include <QGridLayout>
#include <QSet>
#include <QStringList>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

#include <kis_icon.h>
#include <KoFileDialog.h>

#include <KisResourceModel.h>
#include <KisResourceItemListView.h>

#include <ui_WdgPaletteListWidget.h>
#include "KisPaletteChooser.h"
#include "KisPaletteChooser_p.h"

KisPaletteChooser::KisPaletteChooser(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_WdgPaletteListWidget)
    , m_d(new KisPaletteChooserPrivate(this))
{
    m_d->allowModification = false;

    m_d->actAdd.reset(new QAction(KisIconUtils::loadIcon("list-add"),
                                  i18n("Add a new palette")));
    m_d->actRemove.reset(new QAction(KisIconUtils::loadIcon("list-remove"),
                                     i18n("Remove current palette")));
    m_d->actImport.reset(new QAction(KisIconUtils::loadIcon("document-import"),
                                     i18n("Import a new palette from file")));
    m_d->actExport.reset(new QAction(KisIconUtils::loadIcon("document-export"),
                                     i18n("Export current palette to file")));
    m_ui->setupUi(this);
    m_ui->bnAdd->setDefaultAction(m_d->actAdd.data());
    m_ui->bnRemove->setDefaultAction(m_d->actRemove.data());
    m_ui->bnImport->setDefaultAction(m_d->actImport.data());
    m_ui->bnExport->setDefaultAction(m_d->actExport.data());

    m_ui->bnAdd->setEnabled(false);
    m_ui->bnRemove->setEnabled(false);
    m_ui->bnImport->setEnabled(false);
    m_ui->bnExport->setEnabled(false);

    connect(m_d->actAdd.data(), SIGNAL(triggered()), SLOT(slotAdd()));
    connect(m_d->actRemove.data(), SIGNAL(triggered()), SLOT(slotRemove()));
    connect(m_d->actImport.data(), SIGNAL(triggered()), SLOT(slotImport()));
    connect(m_d->actExport.data(), SIGNAL(triggered()), SLOT(slotExport()));

    m_d->itemChooser->setItemDelegate(m_d->delegate.data());
    m_d->itemChooser->setRowHeight(40);
    m_d->itemChooser->itemView()->setViewMode(QListView::ListMode);
    m_d->itemChooser->showButtons(false);
    m_d->itemChooser->showTaggingBar(true);
    QHBoxLayout *paletteLayout = new QHBoxLayout(m_ui->viewPalette);
    paletteLayout->addWidget(m_d->itemChooser.data());

    connect(m_d->itemChooser.data(), SIGNAL(resourceSelected(KoResourceSP )), SLOT(slotPaletteResourceSelected(KoResourceSP )));
}

KisPaletteChooser::~KisPaletteChooser()
{ }

void KisPaletteChooser::slotPaletteResourceSelected(KoResourceSP r)
{
    KoColorSetSP g = r.staticCast<KoColorSet>();
    emit sigPaletteSelected(g);
    if (!m_d->allowModification) { return; }
    if (g->isEditable()) {
        m_ui->bnRemove->setEnabled(true);
    } else {
        m_ui->bnRemove->setEnabled(false);
    }
}

void KisPaletteChooser::slotAdd()
{
    if (!m_d->allowModification) { return; }
    emit sigAddPalette();
    m_d->itemChooser->setCurrentItem(m_d->itemChooser->rowCount() - 1);
}

void KisPaletteChooser::slotRemove()
{
    if (!m_d->allowModification) { return; }
    if (m_d->itemChooser->currentResource()) {
        KoColorSetSP cs = m_d->itemChooser->currentResource().staticCast<KoColorSet>();
        emit sigRemovePalette(cs);
    }
    m_d->itemChooser->setCurrentItem(0);
}

void KisPaletteChooser::slotImport()
{
    if (!m_d->allowModification) { return; }
    emit sigImportPalette();
    m_d->itemChooser->setCurrentItem(m_d->itemChooser->rowCount() - 1);
}

void KisPaletteChooser::slotExport()
{
    if (!m_d->allowModification) { return; }
    emit sigExportPalette(m_d->itemChooser->currentResource().staticCast<KoColorSet>());
}

void KisPaletteChooser::setAllowModification(bool allowModification)
{
    m_d->allowModification = allowModification;
    m_ui->bnAdd->setEnabled(allowModification);
    m_ui->bnImport->setEnabled(allowModification);
    m_ui->bnExport->setEnabled(allowModification);
    KoColorSetSP cs = m_d->itemChooser->currentResource().staticCast<KoColorSet>();
    m_ui->bnRemove->setEnabled(allowModification && cs && cs->isEditable());
}

/************************* KisPaletteChooserPrivate **********************/

KisPaletteChooserPrivate::KisPaletteChooserPrivate(KisPaletteChooser *a_c)
    : c(a_c)
    , itemChooser(new KisResourceItemChooser(ResourceType::Palettes, false, a_c))
    , delegate(new Delegate(a_c))
{  }

KisPaletteChooserPrivate::~KisPaletteChooserPrivate()
{ }

/******************* KisPaletteChooserPrivate::Delegate ******************/

KisPaletteChooserPrivate::Delegate::Delegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{  }

KisPaletteChooserPrivate::Delegate::~Delegate()
{  }

void KisPaletteChooserPrivate::Delegate::paint(QPainter *painter,
                                                  const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    painter->save();
    if (!index.isValid())
        return;

    QImage preview = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
    QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString();

    QRect previewRect(option.rect.x() + 2,
                      option.rect.y() + 2,
                      option.rect.height() - 4,
                      option.rect.height() - 4);

    painter->drawImage(previewRect, preview);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->drawImage(previewRect, preview);
        painter->setPen(option.palette.highlightedText().color());
    } else {
        painter->setBrush(option.palette.text().color());
    }

    painter->drawText(option.rect.x() + previewRect.width() + 10,
                      option.rect.y() + painter->fontMetrics().ascent() + 5,
                      name);

    painter->restore();
}

inline QSize KisPaletteChooserPrivate::Delegate::sizeHint(const QStyleOptionViewItem & option,
                                                             const QModelIndex &) const
{
    return option.decorationSize;
}
