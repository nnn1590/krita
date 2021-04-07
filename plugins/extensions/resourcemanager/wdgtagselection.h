/*
 *  Copyright (c) 2020 Agata Cacko cacko.azh@gmail.com
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef WDG_TAG_SELECTION_H
#define WDG_TAG_SELECTION_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QToolButton>
#include <QHBoxLayout>

#include <KisTag.h>
#include <KoResource.h>
#include <KisResourceModelProvider.h>
#include <KisTagModelProvider.h>

#include <KisTagSelectionWidget.h>

class KisWdgTagSelectionControllerOneResource : public QObject
{
    Q_OBJECT

public:
    KisWdgTagSelectionControllerOneResource(KisTagSelectionWidget* widget, bool editable);
    ~KisWdgTagSelectionControllerOneResource() override;

    void setResourceIds(QString resourceType, QList<int> resourceIds);

private Q_SLOTS:
    void slotRemoveTag(KoID tag);
    void slotAddTag(KoID tag);
    void slotCreateNewTag(QString tag);

private:
    void updateView();

private:
    KisTagSelectionWidget* m_tagSelectionWidget {0};
    bool m_editable {true};
    QList<int> m_resourceIds;
    QString m_resourceType {""};

    QSharedPointer<KisTagModel> m_tagModel;
    QSharedPointer<KisTagResourceModel> m_tagResourceModel;
};


class KisWdgTagSelectionControllerBundleTags : public QObject
{
    Q_OBJECT

public:
    KisWdgTagSelectionControllerBundleTags(KisTagSelectionWidget* widget, bool editable);
    ~KisWdgTagSelectionControllerBundleTags() override;

    QList<int> getSelectedTagIds() const;

    void updateView();
    void setResourceType(const QString& resourceType);


private Q_SLOTS:
    void slotRemoveTag(KoID tag);
    void slotAddTag(KoID tag);

private:

    struct TagResourceType
    {
        KisTagSP tag;
    };

private:
    KisTagSelectionWidget* m_tagSelectionWidget {0};
    bool m_editable {true};
    QString m_resourceType {""};

    QList<KoID> m_selectedTags;

    QMap<QString, QList<KoID>> m_selectedTagsByResourceType;

    QSharedPointer<KisTagModel> m_tagModel;
    QSharedPointer<KisTagResourceModel> m_tagResourceModel;
};


#endif // WDG_TAG_SELECTION_H
