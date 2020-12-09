/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2016 L. E. Segovia <amy@amyspark.me>

    SPDX-License-Identifier: LGPL-2.1-or-later

 */
#ifndef KOCOLORSET
#define KOCOLORSET

#include <QObject>
#include <QColor>
#include <QVector>
#include <QScopedPointer>
#include <QSharedPointer>

#include <KoResource.h>
#include "KoColor.h"
#include "KisSwatch.h"
#include "KisSwatchGroup.h"

/**
 * Also called palette.
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class KRITAPIGMENT_EXPORT KoColorSet :public KoResource
{
public:
    static const QString GLOBAL_GROUP_NAME;
    static const QString KPL_VERSION_ATTR;
    static const QString KPL_GROUP_ROW_COUNT_ATTR;
    static const QString KPL_PALETTE_COLUMN_COUNT_ATTR;
    static const QString KPL_PALETTE_NAME_ATTR;
    static const QString KPL_PALETTE_COMMENT_ATTR;
    static const QString KPL_PALETTE_FILENAME_ATTR;
    static const QString KPL_PALETTE_READONLY_ATTR;
    static const QString KPL_COLOR_MODEL_ID_ATTR;
    static const QString KPL_COLOR_DEPTH_ID_ATTR;
    static const QString KPL_GROUP_NAME_ATTR;
    static const QString KPL_SWATCH_ROW_ATTR;
    static const QString KPL_SWATCH_COL_ATTR;
    static const QString KPL_SWATCH_NAME_ATTR;
    static const QString KPL_SWATCH_SPOT_ATTR;
    static const QString KPL_SWATCH_ID_ATTR;
    static const QString KPL_SWATCH_BITDEPTH_ATTR;
    static const QString KPL_PALETTE_PROFILE_TAG;
    static const QString KPL_SWATCH_POS_TAG;
    static const QString KPL_SWATCH_TAG;
    static const QString KPL_GROUP_TAG;
    static const QString KPL_PALETTE_TAG;

public:
    enum PaletteType {
        UNKNOWN = 0,
        GPL,                // GIMP
        RIFF_PAL,           // RIFF
        ACT,                // Photoshop binary
        PSP_PAL,            // PaintShop Pro
        ACO,                // Photoshop Swatches
        XML,                // XML palette (Scribus)
        KPL,                // KoColor-based XML palette
        SBZ                 // SwatchBooker
    };


    /**
     * Load a color set from a file. This can be a Gimp
     * palette, a RIFF palette, a Photoshop palette,
     * a Krita palette,
     * a Scribus palette or a SwatchBooker palette.
     */
    explicit KoColorSet(const QString &filename = QString());

    KoColorSet(const KoColorSet& rhs);

    ~KoColorSet() override;

    KoColorSet &operator=(const KoColorSet &rhs) = delete;

    KoResourceSP clone() const override;

    bool load(KisResourcesInterfaceSP resourcesInterface) override;
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;
    QString defaultFileExtension() const override;
    QPair<QString, QString> resourceType() const override
    {
        return QPair<QString, QString>(ResourceType::Palettes, "");
    }

    void setColumnCount(int columns);
    int columnCount() const;

    void setComment(QString comment);
    QString comment();

    int rowCount() const;
    quint32 colorCount() const;

    PaletteType paletteType() const;
    void setPaletteType(PaletteType paletteType);

    bool isEditable() const;
    void setIsEditable(bool isEditable);

    QByteArray toByteArray() const;
    bool fromByteArray(QByteArray &data, KisResourcesInterfaceSP resourcesInterface);

    /**
     * @brief Add a color to the palette.
     * @param c the swatch
     * @param groupName color to add the group to. If empty, it will be added to the unsorted.
     */
    void add(const KisSwatch &, const QString &groupName = GLOBAL_GROUP_NAME);
    void setEntry(const KisSwatch &e, int x, int y, const QString &groupName = GLOBAL_GROUP_NAME);

    /**
     * @brief getColorGlobal
     * A function for getting a color based on a global index. Useful for iterating through all color entries.
     * @param x the global x index over the whole palette.
     * @param y the global y index over the whole palette.
     * @return the entry.
     */
    KisSwatch getColorGlobal(quint32 x, quint32 y) const;

    /**
     * @brief getColorGroup
     * A function for getting the color from a specific group.
     * @param x the x index over the group.
     * @param y the y index over the group.
     * @param groupName the name of the group, will give unsorted when not defined.
     * @return the entry
     */
    KisSwatch getColorGroup(quint32 x, quint32 y, QString groupName);

    /**
     * @brief getGroupNames
     * @return returns a list of group names, excluding the unsorted group.
     */
    QStringList getGroupNames() const;

    /**
     * @brief getGroup
     * @param name
     * @return the group with the name given; global group if no parameter is given
     * null pointer if not found.
     */
    KisSwatchGroup *getGroup(const QString &name);
    KisSwatchGroup *getGlobalGroup();

    bool changeGroupName(const QString &oldGroupName, const QString &newGroupName);


    /**
     * @brief addGroup
     * Adds a new group.
     * @param groupName the name of the new group. When not specified, this will fail.
     * @return whether thegroup was made.
     */
    bool addGroup(const QString &groupName);

    /**
     * @brief moveGroup
     * Move a group in the internal stringlist.
     * @param groupName the groupname to move.
     * @param groupNameInsertBefore the groupname to insert before. Empty means it will be added to the end.
     * @return
     */
    bool moveGroup(const QString &groupName, const QString &groupNameInsertBefore = GLOBAL_GROUP_NAME);
    /**
     * @brief removeGroup
     * Remove a group from the KoColorSet
     * @param groupName the name of the group you want to remove.
     * @param keepColors Whether you wish to keep the colorsetentries. These will be added to the unsorted.
     * @return whether it could find the group to remove.
     */
    bool removeGroup(const QString &groupName, bool keepColors = true);

    void clear();

    /**
     * @brief getIndexClosestColor
     * function that matches the color to all colors in the colorset, and returns the index
     * of the closest match.
     * @param compare the color you wish to compare.
     * @param useGivenColorSpace whether to use the color space of the color given
     * when the two colors' colorspaces don't match. Else it'll use the entry's colorspace.
     * @return returns the int of the closest match.
     */
    KisSwatchGroup::SwatchInfo getClosestColorInfo(KoColor compare, bool useGivenColorSpace = true);

private:
    class Private;
    const QScopedPointer<Private> d;

};

typedef QSharedPointer<KoColorSet> KoColorSetSP;

#endif // KOCOLORSET
