/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LIBKIS_PALETTE_H
#define LIBKIS_PALETTE_H

#include <QObject>
#include <QList>

#include "kritalibkis_export.h"
#include "libkis.h"
#include "Resource.h"
#include "KoColorSet.h"
#include <Swatch.h>

class ManagedColor;


/**
 * @brief The Palette class
 * Palette is a resource object that stores organised color data.
 * It's purpose is to allow artists to save colors and store them.
 *
 * An example for printing all the palettes and the entries:
 *
 * @code
import sys
from krita import *

resources = Application.resources("palette")

for (k, v) in resources.items():
    print(k)
    palette = Palette(v)
    for x in range(palette.numberOfEntries()):
        entry = palette.colorSetEntryByIndex(x)
        c = palette.colorForEntry(entry);
        print(x, entry.name(), entry.id(), entry.spotColor(), c.toQString())
 * @endcode
 */

class KRITALIBKIS_EXPORT Palette : public QObject
{
public:
    Palette(Resource *resource);
    ~Palette() override;

    /**
     * @brief numberOfEntries
     * @return
     */
    int numberOfEntries() const;

    /**
     * @brief columnCount
     * @return the amount of columns this palette is set to use.
     */
    int columnCount();
    /**
     * @brief setColumnCount
     * Set the amount of columns this palette should use.
     */
    void setColumnCount(int columns);
    /**
     * @brief comment
     * @return the comment or description associated with the palette.
     */
    QString comment();
    /**
     * @brief setComment
     * set the comment or description associated with the palette.
     * @param comment
     */
    void setComment(QString comment);
    /**
     * @brief groupNames
     * @return the list of group names. This is list is in the order these groups are in the file.
     */
    QStringList groupNames() const;
    /**
     * @brief addGroup
     * @param name of the new group
     * @return whether adding the group was successful.
     */
    bool addGroup(QString name);
    /**
     * @brief removeGroup
     * @param name the name of the group to remove.
     * @param keepColors whether or not to delete all the colors inside, or to move them to the default group.
     * @return
     */
    bool removeGroup(QString name, bool keepColors = true);

    /**
     * @brief colorsCountTotal
     * @return the total amount of entries in the whole group
     */
    int colorsCountTotal();

    /**
     * @brief colorSetEntryByIndex
     * get the colorsetEntry from the global index.
     * @param index the global index
     * @return the colorset entry
     */
    Swatch *colorSetEntryByIndex(int index);
    /**
     * @brief colorSetEntryFromGroup
     * @param index index in the group.
     * @param groupName the name of the group to get the color from.
     * @return the colorsetentry.
     */
    Swatch *colorSetEntryFromGroup(int index, const QString &groupName);

    /**
     * @brief addEntry
     * add an entry to a group. Gets appended to the end.
     * @param entry the entry
     * @param groupName the name of the group to add to.
     */
    void addEntry(Swatch entry, QString groupName = QString());
    /**
     * @brief removeEntry
     * remove the entry at @p index from the group @p groupName.
     */
    void removeEntry(int index, const QString &groupName);

    /**
     * @brief changeGroupName
     * change the group name.
     * @param oldGroupName the old groupname to change.
     * @param newGroupName the new name to change it into.
     * @return whether successful. Reasons for failure include not knowing have oldGroupName
     */
    bool changeGroupName(QString oldGroupName, QString newGroupName);
    /**
     * @brief moveGroup
     * move the group to before groupNameInsertBefore.
     * @param groupName group to move.
     * @param groupNameInsertBefore group to inset before.
     * @return whether successful. Reasons for failure include either group not existing.
     */
    bool moveGroup(const QString &groupName, const QString &groupNameInsertBefore = QString());

    /**
     * @brief save
     * save the palette
     * @return whether it was successful.
     */
    bool save();

private:
    friend class PaletteView;
    struct Private;
    Private *const d;

    /**
     * @brief colorSet
     * @return gives qa KoColorSet object back
     */
    KoColorSetSP colorSet();

};

#endif // LIBKIS_PALETTE_H
