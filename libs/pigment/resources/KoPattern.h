/*
    Copyright (c) 2000 Matthias Elter  <elter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOPATTERN_H
#define KOPATTERN_H

#include <KoResource.h>
#include <kritapigment_export.h>

#include <QMetaType>
#include <QSharedPointer>

class KoPattern;
typedef QSharedPointer<KoPattern> KoPatternSP;


/// Write API docs here
class KRITAPIGMENT_EXPORT KoPattern : public KoResource
{
public:

    /**
     * Creates a new KoPattern object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    explicit KoPattern(const QString &filename);
    KoPattern(const QImage &image, const QString &name, const QString &folderName);
    ~KoPattern() override;

    KoPattern(const KoPattern &rhs);
    KoPattern& operator=(const KoPattern& rhs) = delete;
    KoResourceSP clone() const override;


public:

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice* dev) const override;

    bool loadPatFromDevice(QIODevice *dev);
    bool savePatToDevice(QIODevice* dev) const;

    qint32 width() const;
    qint32 height() const;

    QString defaultFileExtension() const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Patterns, "");
    }

    /**
     * @brief pattern the actual pattern image
     * @return a valid QImage. There are no guarantees to the image format.
     */
    QImage pattern() const;

    bool hasAlpha();

private:

    bool init(QByteArray& data);
    void setPatternImage(const QImage& image);
    void checkForAlpha(const QImage& image);

private:
    QImage m_pattern;
    bool m_hasAlpha = false;
    mutable QByteArray m_md5;
};

Q_DECLARE_METATYPE(KoPattern*)

Q_DECLARE_METATYPE(QSharedPointer<KoPattern>)

#endif // KOPATTERN_H

