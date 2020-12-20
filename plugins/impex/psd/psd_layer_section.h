/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_LAYER_SECTION_H
#define PSD_LAYER_SECTION_H

#include <QString>

class QIODevice;

#include <kis_types.h>

#include "psd_header.h"
#include "psd_layer_record.h"

class PSDLayerMaskSection
{

public:

    PSDLayerMaskSection(const PSDHeader& header);
    ~PSDLayerMaskSection();

    bool read(QIODevice* io);
    bool write(QIODevice* io, KisNodeSP rootLayer);

    QString error;

    // https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_21849
    quint64 layerMaskBlockSize {0}; // Length of the layer and mask information section

    // layer info: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_16000
    bool hasTransparency {false};

    qint16  nLayers {0}; // If layer count is a negative number, its absolute value is the number of layers and the first alpha channel contains the transparency data for the merged result.
    QVector<PSDLayerRecord*> layers;

    // mask info: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_17115
    struct GlobalLayerMaskInfo {
        quint16 overlayColorSpace {0};  // Overlay color space (undocumented).
        quint16 colorComponents[4] {0, 0, 0, 0}; // 4 * 2 byte color components
        quint16 opacity {0}; // Opacity. 0 = transparent, 100 = opaque.
        quint8  kind {0}; // Kind. 0 = Color selected--i.e. inverted; 1 = Color protected;128 = use value stored per layer. This value is preferred. The others are for backward compatibility with beta versions.
    };
    GlobalLayerMaskInfo globalLayerMaskInfo;
    PsdAdditionalLayerInfoBlock globalInfoSection;

private:
    bool readLayerInfoImpl(QIODevice* io);
    bool readImpl(QIODevice* io);
    void writeImpl(QIODevice* io, KisNodeSP rootLayer);
private:

    const PSDHeader m_header;

};

#endif // PSD_LAYER_SECTION_H
