/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPALETTEMANAGER_H
#define KISPALETTEMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <KisSwatch.h>

#include <kritaui_export.h>

class KoColorSet;
class KisPaletteModel;
class KisViewManager;
class KisSwatchGroup;
class KisViewManager;

/**
 * @brief The PaletteEditor class
 * this class manipulates a KisPaletteModel using GUI elements and communicate
 * with KisDocument
 *
 * Changes made in this class won't be done to the palette if the palette is
 * read only (not editable, isEditable() == false)
 */
class KRITAUI_EXPORT KisPaletteEditor : public QObject
{
    Q_OBJECT
public:
    struct PaletteInfo;

public:
    explicit KisPaletteEditor(QObject *parent = 0);
    ~KisPaletteEditor();

    void setPaletteModel(KisPaletteModel *model);
    void setView(KisViewManager *view);

    void addPalette();
    void importPalette();
    void removePalette(KoColorSetSP );

    /**
     * @brief rowNumberOfGroup
     * @param oriName the original name of a group at the creation of the instance
     * @return newest row number of the group
     */
    int rowNumberOfGroup(const QString &oriName) const;
    /**
     * @brief oldNameFromNewName
     * @param newName the current name of a group
     * @return the name of the group at the creation of the instance
     */
    QString oldNameFromNewName(const QString &newName) const;
    /**
     * @brief duplicateExistsFilename
     * @param filename the name of the file
     * @param global if this filename is going to be used for a global palette
     * @return true if the a palette in the resource system that has filename
     * name already exists else false
     */
    bool duplicateExistsFilename(const QString &filename, bool global) const;
    QString relativePathFromSaveLocation() const;

    void rename(const QString &newName);
    void changeFilename(const QString &newName);
    void changeColCount(int);

    /**
     * @brief addGroup
     * @return new group's name if change accepted, empty string if cancelled
     */
    QString addGroup();
    /**
     * @brief removeGroup
     * @param name original group name
     * @return true if change accepted, false if cancelled
     */
    bool removeGroup(const QString &name);
    /**
     * @brief renameGroup
     * @param oldName
     * @return new name if change accepted, empty string if cancelled
     */
    QString renameGroup(const QString &oldName);
    void changeGroupRowCount(const QString &name, int newRowCount);
    void setGlobal(bool);
    void setReadOnly(bool);

    void setEntry(const KoColor &color, const QModelIndex &index);
    void removeEntry(const QModelIndex &index);
    void modifyEntry(const QModelIndex &index);
    void addEntry(const KoColor &color);

    bool isModified() const;

    /**
     * @brief getModifiedGroup
     * @param originalName name of the group at the creation of the instance
     * @return the modified group
     */
    const KisSwatchGroup &getModifiedGroup(const QString &originalName) const;

    /**
     * @brief updatePalette
     * MUST be called to make the changes into the resource server
     */
    void updatePalette();

private Q_SLOTS:
    void slotGroupNameChanged(const QString &newName);
    void slotPaletteChanged();
    void slotSetDocumentModified();

private:
    QString newGroupName() const;
    bool duplicateExistsGroupName(const QString &name) const;
    bool duplicateExistsOriginalGroupName(const QString &name) const;
    QString filenameFromPath(const QString &path) const;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPALETTEMANAGER_H
