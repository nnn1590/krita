/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISRESOURCETAGGINGMANAGER_H
#define KISRESOURCETAGGINGMANAGER_H

#include <QObject>

#include <KoResource.h>
#include <KisTag.h>
#include <KisTagModel.h>

#include <kis_debug.h>

class QWidget;
class QStringList;
class QString;
class QPoint;

class KisTagFilterWidget;
class KisTagChooserWidget;
class KisTagFilterResourceProxyModel;


/**
 * @brief The KisResourceTaggingManager class is a helper class for KisResourceItemChooser for tagChooser and tagFilter widgets.
 *
 * It takes care of exchanging information about tags between KisTagChooserWidget and KisTagFilterWidget.
 * It makes sure that the correct tag is put in the resource model proxy that is used in the KisResourceItemChooser.
 * Historically it also managed a list of tags; now KisTagModel is taking care of it.
 *
 */
class KisResourceTaggingManager : public QObject
{
    Q_OBJECT

public:

    ///
    /// \brief KisResourceTaggingManager standard constructor of the KisResourceTaggingManager class
    /// \param resourceType resource type of the resources that will be dealt with
    /// \param model proxy model that is used to show only the resources tagged with a specific tag
    /// \param parent parent widget
    ///
    explicit KisResourceTaggingManager(QString resourceType, KisTagFilterResourceProxyModel *model, QWidget *parent);

    /// \brief ~KisResourceTaggingManager destructor
    ~KisResourceTaggingManager() override;

    ///
    /// \brief showTaggingBar method to show or hide the tag chooser bar and the tag filter
    /// \param show if true, the bars should be shown; if false, they should be hidden
    ///
    void showTaggingBar(bool show);
    ///
    /// \brief contextMenuRequested method to get the context menu
    /// \param currentResource current selected resource
    /// \param pos position of the mouse cursor where the context menu should be created
    ///
    void contextMenuRequested(KoResourceSP currentResource, QPoint pos);
    ///
    /// \brief tagFilterWidget method to get the tag filter widget
    /// \return tag filter widget
    ///
    KisTagFilterWidget *tagFilterWidget();
    ///
    /// \brief tagChooserWidget method to get the tag chooser widget
    /// \return tag chooser widget
    ///
    KisTagChooserWidget *tagChooserWidget();
    
private Q_SLOTS:
    ///
    /// \brief tagChooserIndexChanged slot for the signal that the tag chosen in the tags combobox changed
    /// \param tag the currently chosen tag
    ///
    /// It puts the current tag in the filter proxy model to get only the resources filtered out by the tag.
    ///
    void tagChooserIndexChanged(const KisTagSP tag);
    ///
    /// \brief tagSearchLineEditTextChanged slot for the signal that the text in the filter changed
    /// \param lineEditText the current text in the filter box
    ///
    /// It updates the filter in the filter proxy model.
    ///
    void tagSearchLineEditTextChanged(const QString &lineEditText);
    ///
    /// \brief slotFilterByTagChanged slot for the "filter by tag" checkbox being checked or unchecked by the user
    /// \param filterByTag current state of the checkbox
    ///
    /// It updates the filter in the filter proxy model to honor the "filter by tag" setting correctly.
    ///
    void slotFilterByTagChanged(const bool filterByTag);

private:

    class Private;
    Private* const d;
};


#endif // KORESOURCETAGGINGINTERFACE_H
