/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisTagChooserWidget.h"

#include <QDebug>
#include <QToolButton>
#include <QGridLayout>
#include <QComboBox>

#include <klocalizedstring.h>
#include <KisSqueezedComboBox.h>

#include <KoIcon.h>

#include "KisResourceItemChooserContextMenu.h"
#include "KisTagToolButton.h"
#include "kis_debug.h"

class Q_DECL_HIDDEN KisTagChooserWidget::Private
{
public:
    QComboBox *comboBox;
    KisTagToolButton *tagToolButton;
    KisTagModel *model;
    KisTagSP rememberedTag;
};

KisTagChooserWidget::KisTagChooserWidget(KisTagModel *model, QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{

    d->comboBox = new QComboBox(this);

    d->comboBox->setToolTip(i18n("Tag"));
    d->comboBox->setSizePolicy(QSizePolicy::MinimumExpanding , QSizePolicy::Fixed);
    d->comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    model->sort(KisAllTagsModel::Name);
    d->comboBox->setModel(model);

    d->model = model;

    QGridLayout* comboLayout = new QGridLayout(this);

    comboLayout->addWidget(d->comboBox, 0, 0);

    d->tagToolButton = new KisTagToolButton(this);
    comboLayout->addWidget(d->tagToolButton, 0, 1);

    comboLayout->setSpacing(0);
    comboLayout->setMargin(0);
    comboLayout->setColumnStretch(0, 3);
    this->setEnabled(true);

    connect(d->comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(tagChanged(int)));

    connect(d->tagToolButton, SIGNAL(popupMenuAboutToShow()),
            this, SLOT (tagToolContextMenuAboutToShow()));

    connect(d->tagToolButton, SIGNAL(newTagRequested(QString)),
            this, SLOT(addTag(QString)));

    connect(d->tagToolButton, SIGNAL(deletionOfCurrentTagRequested()),
            this, SLOT(tagToolDeleteCurrentTag()));

    connect(d->tagToolButton, SIGNAL(renamingOfCurrentTagRequested(const QString&)),
            this, SLOT(tagToolRenameCurrentTag(const QString&)));

    connect(d->tagToolButton, SIGNAL(undeletionOfTagRequested(KisTagSP)),
            this, SLOT(tagToolUndeleteLastTag(KisTagSP)));

}

KisTagChooserWidget::~KisTagChooserWidget()
{
    delete d;
}

void KisTagChooserWidget::tagToolDeleteCurrentTag()
{
    KisTagSP currentTag = currentlySelectedTag();
    if (!currentTag.isNull() && currentTag->id() >= 0) {
        d->model->setTagInactive(currentTag);
        setCurrentIndex(0);
        d->tagToolButton->setUndeletionCandidate(currentTag);
        d->model->sort(KisAllTagsModel::Name);
    }
}

void KisTagChooserWidget::tagChanged(int tagIndex)
{
    if (tagIndex >= 0) {
        KisTagSP tag = currentlySelectedTag();
        d->tagToolButton->setCurrentTag(tag);
        d->model->sort(KisAllTagsModel::Name);
        emit sigTagChosen(tag);
    }
}

void KisTagChooserWidget::tagToolRenameCurrentTag(const QString& tagName)
{
    KisTagSP tag = currentlySelectedTag();
    bool canRenameCurrentTag = !tag.isNull();

    if (canRenameCurrentTag && !tagName.isEmpty()) {
        tag->setName(tagName);
        bool result = d->model->renameTag(tag);
        Q_ASSERT(result);
        d->model->sort(KisAllTagsModel::Name);
    }
}

void KisTagChooserWidget::tagToolUndeleteLastTag(KisTagSP tag)
{
    int previousIndex = d->comboBox->currentIndex();

    bool success = d->model->setTagActive(tag);
    setCurrentIndex(previousIndex);
    if (success) {
        d->tagToolButton->setUndeletionCandidate(KisTagSP());
        setCurrentItem(tag->name());
        d->model->sort(KisAllTagsModel::Name);
    }
}

void KisTagChooserWidget::setCurrentIndex(int index)
{
    d->comboBox->setCurrentIndex(index);
}

int KisTagChooserWidget::currentIndex() const
{
    return d->comboBox->currentIndex();
}

void KisTagChooserWidget::setCurrentItem(const QString &tag)
{
    for (int i = 0; i < d->model->rowCount(); i++) {
        QModelIndex index = d->model->index(i, 0);
        QString currentRowTag = d->model->data(index, Qt::UserRole + KisAllTagsModel::Name).toString();
        if (currentRowTag == tag) {
            setCurrentIndex(i);
        }
    }
}

void KisTagChooserWidget::addTag(const QString &tag)
{
    addTag(tag, 0);
}

void KisTagChooserWidget::addTag(const QString &tagName, KoResourceSP resource)
{
    d->model->addTag(tagName, {resource});
    d->model->sort(KisAllTagsModel::Name);
    setCurrentItem(tagName);
}

void KisTagChooserWidget::addTag(KisTagSP tag, KoResourceSP resource)
{
    d->model->addTag(tag, {resource});
    d->model->sort(KisAllTagsModel::Name);
    setCurrentItem(tag->name());
}

KisTagSP KisTagChooserWidget::currentlySelectedTag()
{
    int row = d->comboBox->currentIndex();
    if (row < 0) {
        return KisTagSP();
    }

    QModelIndex index = d->model->index(row, 0);
    KisTagSP tag =  d->model->tagForIndex(index);
    return tag;
}

void KisTagChooserWidget::tagToolContextMenuAboutToShow()
{
    /* only enable the save button if the selected tag set is editable */
    d->tagToolButton->readOnlyMode(currentlySelectedTag()->id() < 0);
}
