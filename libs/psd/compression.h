/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <QByteArray>
#include "kritapsd_export.h"

class KRITAPSD_EXPORT Compression
{
public:

    enum CompressionType {
        Uncompressed = 0,
        RLE,
        ZIP,
        ZIPWithPrediction,
        Unknown
    };

    static QByteArray uncompress(quint32 unpacked_len, QByteArray bytes, CompressionType compressionType);
    static QByteArray compress(QByteArray bytes, CompressionType compressionType);
};

#endif // PSD_COMPRESSION_H
