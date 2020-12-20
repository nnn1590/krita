/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2006 Martin Pfeiffer <hubipete@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __koDocumentInfoDlg_h__
#define __koDocumentInfoDlg_h__

#include <kpagedialog.h>

class KoDocumentInfo;
class KPageWidgetItem;
class KoPageWidgetItem;

#include "kritaui_export.h"

/**
 * @short The dialog that shows information about the document
 * @author Simon Hausmann <hausmann@kde.org>
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @see KoDocumentInfo
 *
 * This dialog is invoked by KoMainWindow and shows the content
 * of the given KoDocumentInfo class. It consists of several pages,
 * one showing general information about the document and an other
 * showing information about the author.
 * This dialog implements only things that are stored in the OASIS
 * meta.xml file and therefore available through the KoDocumentInfo
 * class.
 * The widgets shown in the tabs are koDocumentInfoAboutWidget and
 * koDocumentInfoAuthorWidget. This class here is derived from
 * KPageDialog and uses the face type Tabbed.
 */

class KRITAUI_EXPORT KoDocumentInfoDlg : public KPageDialog
{
    Q_OBJECT

public:
    /**
     * The constructor
     * @param parent a pointer to the parent widget
     * @param docInfo a pointer to the shown KoDocumentInfo
     */
    KoDocumentInfoDlg(QWidget *parent, KoDocumentInfo* docInfo);

    /** The destructor */
    ~KoDocumentInfoDlg() override;

    QList<KPageWidgetItem*> pages() const;

    /** Returns true if the document was saved when the dialog was closed */
    bool isDocumentSaved();

    /** Sets all fields to read-only mode. Used by the property dialog. */
    void setReadOnly(bool ro);

    void addPageItem(KoPageWidgetItem *item);

public Q_SLOTS:  // QDialog API
    void accept() override;

protected:  // QWidget API
    void hideEvent(QHideEvent * event) override;

private Q_SLOTS:
    /** Connected with clicked() from pbReset - Reset parts of the metadata */
    void slotResetMetaData();

Q_SIGNALS:
    void saveRequested();

private:
    /** Sets up the aboutWidget and fills the widgets with content */
    void initAboutTab();
    /** Sets up the authorWidget and fills the widgets with content */
    void initAuthorTab();
    /** Saves the changed data back to the KoDocumentInfo class */
    void saveAboutData();

    void updateEditingTime();

    class KoDocumentInfoDlgPrivate;
    KoDocumentInfoDlgPrivate * const d;
};

#endif
