/*
 *  SPDX-FileCopyrightText: 2019 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisClipboardUtil.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QList>
#include <QSet>
#include <QPair>
#include <QDebug>

namespace KisClipboardUtil {

struct ClipboardImageFormat
{
    QSet<QString> mimeTypes;
    QString format;
};

QImage getImageFromClipboard()
{
    static const QList<ClipboardImageFormat> supportedFormats = {
            {{"image/png"}, "PNG"},
            {{"image/tiff"}, "TIFF"},
            {{"image/bmp", "image/x-bmp", "image/x-MS-bmp", "image/x-win-bitmap"}, "BMP"},
            {{"image/jpeg"}, "JPG"}
    };

    QClipboard *clipboard = QApplication::clipboard();

    QImage image;
    QSet<QString> clipboardMimeTypes;

    Q_FOREACH(const QString &format, clipboard->mimeData()->formats()) {
        clipboardMimeTypes << format;
    }

    Q_FOREACH (const ClipboardImageFormat &item, supportedFormats) {
        const QSet<QString> &intersection = item.mimeTypes & clipboardMimeTypes;
        if (intersection.isEmpty()) {
            continue;
        }

        const QString &format = *intersection.constBegin();
        const QByteArray &imageData = clipboard->mimeData()->data(format);
        if (imageData.isEmpty()) {
            continue;
        }

        if (image.loadFromData(imageData, item.format.toLatin1())) {
            break;
        }
    }

    if (image.isNull()) {
        image = clipboard->image();
    }

    return image;
}

}
