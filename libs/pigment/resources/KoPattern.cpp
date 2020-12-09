/*  This file is part of the KDE project

    Copyright (c) 2000 Matthias Elter <elter@kde.org>
    Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <resources/KoPattern.h>

#include <sys/types.h>
#include <QtEndian>

#include <limits.h>
#include <stdlib.h>

#include <QFileInfo>
#include <QDir>
#include <QPoint>
#include <QSize>
#include <QImage>
#include <QMap>
#include <QFile>
#include <QBuffer>
#include <QTextStream>

#include <DebugPigment.h>
#include <klocalizedstring.h>

namespace
{
struct GimpPatternHeader {
    quint32 header_size;  /*  header_size = sizeof (PatternHeader) + brush name  */
    quint32 version;      /*  pattern file version #  */
    quint32 width;        /*  width of pattern */
    quint32 height;       /*  height of pattern  */
    quint32 bytes;        /*  depth of pattern in bytes : 1, 2, 3 or 4*/
    quint32 magic_number; /*  GIMP brush magic number  */
};

// Yes! This is _NOT_ what my pat.txt file says. It's really not 'GIMP', but 'GPAT'
quint32 const GimpPatternMagic = (('G' << 24) + ('P' << 16) + ('A' << 8) + ('T' << 0));
}


KoPattern::KoPattern(const QString& file)
    : KoResource(file)
{
}

KoPattern::KoPattern(const QImage &image, const QString &name, const QString &folderName)
    : KoResource(QString())
{
    setPatternImage(image);
    setName(name);

    QFileInfo fileInfo(folderName + '/' + name + defaultFileExtension());

    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(folderName + '/' +
                         name + QString::number(i) + defaultFileExtension());
        i++;
    }

    setFilename(fileInfo.filePath());
}


KoPattern::~KoPattern()
{
}

KoPattern::KoPattern(const KoPattern &rhs)
    : KoResource(rhs),
      m_pattern(rhs.m_pattern),
      m_md5(rhs.m_md5)
{
}

KoResourceSP KoPattern::clone() const
{
    return KoResourceSP(new KoPattern(*this));
}

bool KoPattern::loadPatFromDevice(QIODevice *dev)
{
    QByteArray data = dev->readAll();
    return init(data);
}

bool KoPattern::savePatToDevice(QIODevice* dev) const
{
    // Header: header_size (24+name length),version,width,height,colordepth of brush,magic,name
    // depth: 1 = greyscale, 2 = greyscale + A, 3 = RGB, 4 = RGBA
    // magic = "GPAT", as a single uint32, the docs are wrong here!
    // name is UTF-8 (\0-terminated! The docs say nothing about this!)
    // _All_ data in network order, it seems! (not mentioned in gimp-2.2.8/devel-docs/pat.txt!!)
    // We only save RGBA at the moment
    // Version is 1 for now...

    GimpPatternHeader ph;
    QByteArray utf8Name = name().toUtf8();
    char const* name = utf8Name.data();
    int nameLength = qstrlen(name);

    ph.header_size = qToBigEndian((quint32)sizeof(GimpPatternHeader) + nameLength + 1); // trailing 0
    ph.version = qToBigEndian((quint32)1);
    ph.width = qToBigEndian((quint32)width());
    ph.height = qToBigEndian((quint32)height());
    ph.bytes = qToBigEndian((quint32)4);
    ph.magic_number = qToBigEndian((quint32)GimpPatternMagic);

    QByteArray bytes = QByteArray::fromRawData(reinterpret_cast<char*>(&ph), sizeof(GimpPatternHeader));
    int wrote = dev->write(bytes);
    bytes.clear();

    if (wrote == -1)
        return false;

    wrote = dev->write(name, nameLength + 1); // Trailing 0 apparently!
    if (wrote == -1)
        return false;

    int k = 0;
    bytes.resize(width() * height() * 4);
    for (qint32 y = 0; y < height(); ++y) {
        for (qint32 x = 0; x < width(); ++x) {
            // RGBA only
            QRgb pixel = m_pattern.pixel(x, y);
            bytes[k++] = static_cast<char>(qRed(pixel));
            bytes[k++] = static_cast<char>(qGreen(pixel));
            bytes[k++] = static_cast<char>(qBlue(pixel));
            bytes[k++] = static_cast<char>(qAlpha(pixel));
        }
    }

    wrote = dev->write(bytes);
    if (wrote == -1)
        return false;

    KoResource::saveToDevice(dev);

    return true;
}

bool KoPattern::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    Q_UNUSED(resourcesInterface);

    QString fileExtension;
    int index = filename().lastIndexOf('.');

    if (index != -1)
        fileExtension = filename().mid(index + 1).toLower();

    bool result;

    if (fileExtension == "pat") {
        result = loadPatFromDevice(dev);
    }
    else {
        QImage image;
        // Workaround for some OS (Debian, Ubuntu), where loading directly from the QIODevice
        // fails with "libpng error: IDAT: CRC error"
        QByteArray data = dev->readAll();
        QBuffer buffer(&data);
        result = image.load(&buffer, fileExtension.toUpper().toLatin1());
        setPatternImage(image);
    }

    return result;

}

bool KoPattern::saveToDevice(QIODevice *dev) const
{
    QString fileExtension;
    int index = filename().lastIndexOf('.');

    if (index != -1)
        fileExtension = filename().mid(index + 1).toLower();

    bool result = false;

    if (fileExtension == "pat") {
        result = savePatToDevice(dev);
    }
    else {
        result = m_pattern.save(dev, fileExtension.toUpper().toLatin1());
    }

    return result && KoResource::saveToDevice(dev);
}

bool KoPattern::init(QByteArray& bytes)
{
    int dataSize = bytes.size();
    const char* data = bytes.constData();

    // load Gimp patterns
    GimpPatternHeader bh;
    qint32 k;
    char* name;

    if ((int)sizeof(GimpPatternHeader) > dataSize) {
        return false;
    }

    memcpy(&bh, data, sizeof(GimpPatternHeader));
    bh.header_size = qFromBigEndian(bh.header_size);
    bh.version = qFromBigEndian(bh.version);
    bh.width = qFromBigEndian(bh.width);
    bh.height = qFromBigEndian(bh.height);
    bh.bytes = qFromBigEndian(bh.bytes);
    bh.magic_number = qFromBigEndian(bh.magic_number);

    if ((int)bh.header_size > dataSize || bh.header_size == 0) {
        return false;
    }
    int size = bh.header_size - sizeof(GimpPatternHeader);
    name = new char[size];
    memcpy(name, data + sizeof(GimpPatternHeader), size);

    if (name[size - 1]) {
        delete[] name;
        return false;
    }

    // size -1 so we don't add the end 0 to the QString...
    setName(QString::fromLatin1(name, size -1));
    delete[] name;

    if (bh.width == 0 || bh.height == 0) {
        return false;
    }

    QImage::Format imageFormat;

    if (bh.bytes == 1 || bh.bytes == 3) {
        imageFormat = QImage::Format_RGB32;
    } else {
        imageFormat = QImage::Format_ARGB32;
    }

    QImage pattern = QImage(bh.width, bh.height, imageFormat);
    if (pattern.isNull()) {
        return false;
    }
    k = bh.header_size;

    if (bh.bytes == 1) {
        // Grayscale
        qint32 val;
        for (quint32 y = 0; y < bh.height; ++y) {
            QRgb* pixels = reinterpret_cast<QRgb*>( pattern.scanLine(y) );
            for (quint32 x = 0; x < bh.width; ++x, ++k) {
                if (k > dataSize) {
                    qWarning() << "failed to load grayscale pattern" << filename();
                    return false;
                }

                val = data[k];
                pixels[x] = qRgb(val, val, val);
            }
        }
        // It was grayscale, so make the pattern as small as possible
        // by converting it to Indexed8
        pattern = pattern.convertToFormat(QImage::Format_Indexed8);
    }
    else if (bh.bytes == 2) {
        // Grayscale + A
        qint32 val;
        qint32 alpha;
        for (quint32 y = 0; y < bh.height; ++y) {
            QRgb* pixels = reinterpret_cast<QRgb*>( pattern.scanLine(y) );
            for (quint32 x = 0; x < bh.width; ++x, ++k) {
                if (k + 2 > dataSize) {
                    qWarning() << "failed to load grayscale +_ alpha pattern" << filename();
                    return false;
                }

                val = data[k];
                alpha = data[k++];
                pixels[x] = qRgba(val, val, val, alpha);
            }
        }
    }
    else if (bh.bytes == 3) {
        // RGB without alpha
        for (quint32 y = 0; y < bh.height; ++y) {
            QRgb* pixels = reinterpret_cast<QRgb*>( pattern.scanLine(y) );
            for (quint32 x = 0; x < bh.width; ++x) {
                if (k + 3 > dataSize) {
                    qWarning() << "failed to load RGB pattern" << filename();
                    return false;
                }
                pixels[x] = qRgb(data[k],
                                 data[k + 1],
                                 data[k + 2]);
                k += 3;
            }
        }
    } else if (bh.bytes == 4) {
        // Has alpha
        for (quint32 y = 0; y < bh.height; ++y) {
            QRgb* pixels = reinterpret_cast<QRgb*>( pattern.scanLine(y) );
            for (quint32 x = 0; x < bh.width; ++x) {
                if (k + 4 > dataSize) {
                    qWarning() << "failed to load RGB + Alpha pattern" << filename();
                    return false;
                }

                pixels[x] = qRgba(data[k],
                                  data[k + 1],
                                  data[k + 2],
                                  data[k + 3]);
                k += 4;
            }
        }
    } else {
        return false;
    }

    if (pattern.isNull()) {
        return false;
    }

    setPatternImage(pattern);
    setValid(true);

    return true;
}

qint32 KoPattern::width() const
{
    return m_pattern.width();
}

qint32 KoPattern::height() const
{
    return m_pattern.height();
}

void KoPattern::setPatternImage(const QImage& image)
{
    m_pattern = image;
    checkForAlpha(image);
    setImage(image);
    setValid(true);
}


QString KoPattern::defaultFileExtension() const
{
    return QString(".pat");
}


QImage KoPattern::pattern() const
{
    return m_pattern;
}

void KoPattern::checkForAlpha(const QImage& image) {
    m_hasAlpha = false;
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            if (qAlpha(image.pixel(x, y)) != 255) {
                m_hasAlpha = true;
                break;
            }
        }
    }
}

bool KoPattern::hasAlpha() {
    return m_hasAlpha;
}