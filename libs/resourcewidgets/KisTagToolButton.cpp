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
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#include "KisTagToolButton.h"

#include <QToolButton>
#include <QGridLayout>

#include <klocalizedstring.h>

#include <KoIcon.h>

#include "KisResourceItemChooserContextMenu.h"
#include "kis_debug.h"

class KisTagToolButton::Private
{
public:
    QToolButton* tagToolButton;
    QAction* action_undeleteTag;
    QAction* action_deleteTag;
    KoLineEditAction* action_renameTag;
    KisTagSP undeleteCandidate;
};

KisTagToolButton::KisTagToolButton(QWidget* parent)
    :QWidget(parent), d(new Private())
{

    QGridLayout* buttonLayout = new QGridLayout(this);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);

    d->tagToolButton = new QToolButton(this);
    d->tagToolButton->setIcon(koIcon("bookmarks"));
    d->tagToolButton->setText(i18n("Tag"));
    d->tagToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->tagToolButton->setToolTip(i18nc("@info:tooltip", "<qt>Show the tag box options.</qt>"));
    d->tagToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tagToolButton->setEnabled(true);

    QMenu* popup = new QMenu(this);

    KoLineEditAction*  addTagAction = new KoLineEditAction(popup);
    addTagAction->setPlaceholderText(i18n("New tag"));
    addTagAction->setIcon(koIcon("document-new"));
    addTagAction->closeParentOnTrigger(true);
    popup->addAction(addTagAction);

    connect(addTagAction, SIGNAL(triggered(KisTagSP)),
            this, SIGNAL(newTagRequested(KisTagSP)));

    d->action_renameTag = new KoLineEditAction(popup);
    d->action_renameTag->setPlaceholderText(i18n("Rename tag"));
    d->action_renameTag->setIcon(koIcon("edit-rename"));
    d->action_renameTag->closeParentOnTrigger(true);
    popup->addAction(d->action_renameTag);

    connect(d->action_renameTag, SIGNAL(triggered(KisTagSP)),
            this, SIGNAL(renamingOfCurrentTagRequested(KisTagSP)));

    popup->addSeparator();

    d->action_deleteTag = new QAction(popup);
    d->action_deleteTag->setText(i18n("Delete this tag"));
    d->action_deleteTag->setIcon(koIcon("edit-delete"));
    popup->addAction(d->action_deleteTag);

    connect(d->action_deleteTag, SIGNAL(triggered()),
            this, SIGNAL(deletionOfCurrentTagRequested()));

    popup->addSeparator();

    d->action_undeleteTag = new QAction(popup);
    d->action_undeleteTag->setIcon(koIcon("edit-redo"));
    d->action_undeleteTag->setVisible(false);
    popup->addAction(d->action_undeleteTag);

    connect(d->action_undeleteTag, SIGNAL(triggered()),
            this, SLOT(onTagUndeleteClicked()));

    connect(popup, SIGNAL(aboutToShow()),
            this, SIGNAL(popupMenuAboutToShow()));

    d->tagToolButton->setMenu(popup);
    buttonLayout->addWidget(d->tagToolButton);
}

KisTagToolButton::~KisTagToolButton()
{
    delete d;
}

void KisTagToolButton::readOnlyMode(bool activate)
{
    activate = !activate;
    d->action_renameTag->setVisible(activate);
    d->action_deleteTag->setVisible(activate);
}

void KisTagToolButton::setUndeletionCandidate(const KisTagSP deletedTag)
{
    if (deletedTag.isNull() || deletedTag->name().isEmpty()) {
        d->action_undeleteTag->setVisible(false);
        return;
    } else {
        d->undeleteCandidate = deletedTag;
        d->action_undeleteTag->setText(i18n("Undelete") +" "+ deletedTag->name());
        d->action_undeleteTag->setVisible(true);
    }
}

void KisTagToolButton::onTagUndeleteClicked()
{
    emit undeletionOfTagRequested(d->undeleteCandidate);
}

