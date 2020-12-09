/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_layer_record.h"

#include <QtEndian>

#include <QIODevice>
#include <QBuffer>
#include <QDataStream>
#include <QStringList>
#include <KoColor.h>


#include <kis_debug.h>
#include <kis_node.h>
#include "kis_iterator_ng.h"
#include <kis_paint_layer.h>

#include "psd_utils.h"
#include "psd_header.h"
#include "compression.h"

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceRegistry.h>

#include <asl/kis_offset_keeper.h>
#include <asl/kis_asl_writer_utils.h>
#include <asl/kis_asl_reader_utils.h>

#include "psd_pixel_utils.h"
#include <kundo2command.h>


// Just for pretty debug messages
QString channelIdToChannelType(int channelId, psd_color_mode colormode)
{
    switch(channelId) {
    case -3:
        return "Real User Supplied Layer Mask (when both a user mask and a vector mask are present";
    case -2:
        return "User Supplied Layer Mask";
    case -1:
        return "Transparency mask";
    case  0:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return "gray";
        case RGB:
        case RGB48:
            return "red";
        case Lab:
        case Lab48:
            return "L";
        case CMYK:
        case CMYK64:
            return "cyan";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    case 1:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("WARNING bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return QString("WARNING: %1").arg(channelId);
        case RGB:
        case RGB48:
            return "green";
        case Lab:
        case Lab48:
            return "a";
        case CMYK:
        case CMYK64:
            return "Magenta";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    case 2:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("WARNING bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return QString("WARNING: %1").arg(channelId);
        case RGB:
        case RGB48:
            return "blue";
        case Lab:
        case Lab48:
            return "b";
        case CMYK:
        case CMYK64:
            return "yellow";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    case 3:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("WARNING bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return QString("WARNING: %1").arg(channelId);
        case RGB:
        case RGB48:
            return QString("alpha: %1").arg(channelId);
        case Lab:
        case Lab48:
            return QString("alpha: %1").arg(channelId);
        case CMYK:
        case CMYK64:
            return "Key";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    default:
        return QString("unknown: %1").arg(channelId);
    };

}

PSDLayerRecord::PSDLayerRecord(const PSDHeader& header)
    : top(0)
    , left(0)
    , bottom(0)
    , right(0)
    , nChannels(0)
    , opacity(0)
    , clipping(0)
    , transparencyProtected(false)
    , visible(true)
    , irrelevant(false)
    , layerName("UNINITIALIZED")
    , infoBlocks(header)
    , m_transparencyMaskSizeOffset(0)
    , m_header(header)
{
}

bool PSDLayerRecord::read(QIODevice* io)
{
    dbgFile << "Going to read layer record. Pos:" << io->pos();

    if (!psdread(io, &top)  ||
            !psdread(io, &left) ||
            !psdread(io, &bottom) ||
            !psdread(io, &right) ||
            !psdread(io, &nChannels)) {

        error = "could not read layer record";
        return false;
    }

    dbgFile << "\ttop" << top << "left" << left << "bottom" << bottom << "right" << right << "number of channels" << nChannels;

    Q_ASSERT(top <= bottom);
    Q_ASSERT(left <= right);
    Q_ASSERT(nChannels > 0);

    if (nChannels < 1) {
        error = QString("Not enough channels. Got: %1").arg(nChannels);
        return false;
    }

    if (nChannels > MAX_CHANNELS) {
        error = QString("Too many channels. Got: %1").arg(nChannels);
        return false;
    }

    for (int i = 0; i < nChannels; ++i) {

        if (io->atEnd()) {
            error = "Could not read enough data for channels";
            return false;
        }

        ChannelInfo* info = new ChannelInfo;

        if (!psdread(io, &info->channelId)) {
            error = "could not read channel id";
            delete info;
            return false;
        }
        bool r;
        if (m_header.version == 1) {
            quint32 channelDataLength;
            r = psdread(io, &channelDataLength);
            info->channelDataLength = (quint64)channelDataLength;
        }
        else {
            r = psdread(io, &info->channelDataLength);
        }
        if (!r) {
            error = "Could not read length for channel data";
            delete info;
            return false;
        }

        dbgFile << "\tchannel" << i << "id"
                << channelIdToChannelType(info->channelId, m_header.colormode)
                << "length" << info->channelDataLength
                << "start" << info->channelDataStart
                << "offset" << info->channelOffset
                << "channelInfoPosition" << info->channelInfoPosition;

        channelInfoRecords << info;

    }

    if (!psd_read_blendmode(io, blendModeKey)) {
        error = QString("Could not read blend mode key. Got: %1").arg(blendModeKey);
        return false;
    }

    dbgFile << "\tBlend mode" << blendModeKey << "pos" << io->pos();

    if (!psdread(io, &opacity)) {
        error = "Could not read opacity";
        return false;
    }

    dbgFile << "\tOpacity" << opacity << io->pos();

    if (!psdread(io, &clipping)) {
        error = "Could not read clipping";
        return false;
    }

    dbgFile << "\tclipping" << clipping << io->pos();

    quint8 flags;
    if (!psdread(io, &flags)) {
        error = "Could not read flags";
        return false;
    }
    dbgFile << "\tflags" << flags << io->pos();

    transparencyProtected = flags & 1 ? true : false;

    dbgFile << "\ttransparency protected" << transparencyProtected;

    visible = flags & 2 ? false : true;

    dbgFile << "\tvisible" << visible;

    if (flags & 8) {
        irrelevant = flags & 16 ? true : false;
    }
    else {
        irrelevant = false;
    }

    dbgFile << "\tirrelevant" << irrelevant;

    dbgFile << "\tfiller at " << io->pos();

    quint8 filler;
    if (!psdread(io, &filler) || filler != 0) {
        error = "Could not read padding";
        return false;
    }

    dbgFile << "\tGoing to read extra data length" << io->pos();

    quint32 extraDataLength;
    if (!psdread(io, &extraDataLength) || io->bytesAvailable() < extraDataLength) {
        error = QString("Could not read extra layer data: %1 at pos %2").arg(extraDataLength).arg(io->pos());
        return false;
    }

    dbgFile << "\tExtra data length" << extraDataLength;

    if (extraDataLength > 0) {

        dbgFile << "Going to read extra data field. Bytes available: "
                << io->bytesAvailable()
                << "pos" << io->pos();

                
        // See https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_22582
        quint32 layerMaskLength = 1; // invalid...
        if (!psdread(io, &layerMaskLength) ||
                io->bytesAvailable() < layerMaskLength ||
                !(layerMaskLength == 0 || layerMaskLength == 20 || layerMaskLength == 36)) {
            error = QString("Could not read layer mask length: %1").arg(layerMaskLength);
            return false;
        }

        memset(&layerMask, 0, sizeof(LayerMaskData));

        if (layerMaskLength == 20 || layerMaskLength == 36) {
            if (!psdread(io, &layerMask.top)  ||
                    !psdread(io, &layerMask.left) ||
                    !psdread(io, &layerMask.bottom) ||
                    !psdread(io, &layerMask.right) ||
                    !psdread(io, &layerMask.defaultColor) ||
                    !psdread(io, &flags)) {

                error = "could not read mask record";
                return false;
            }
        }
        if (layerMaskLength == 20) {
            quint16 padding;
            if (!psdread(io, &padding)) {
                error = "Could not read layer mask padding";
                return false;
            }
        }

        // If it's 36, that is, bit four of the flags is set, we also need to read the 'real' flags, background and rectangle
        if (layerMaskLength == 36 ) {
            if (!psdread(io, &flags) ||
                    !psdread(io, &layerMask.defaultColor) ||
                    !psdread(io, &layerMask.top)  ||
                    !psdread(io, &layerMask.left) ||
                    !psdread(io, &layerMask.bottom) ||
                    !psdread(io, &layerMask.top)) {

                error = "could not read 'real' mask record";
                return false;
            }
        }

        layerMask.positionedRelativeToLayer = flags & 1 ? true : false;
        layerMask.disabled = flags & 2 ? true : false;
        layerMask.invertLayerMaskWhenBlending = flags & 4 ? true : false;

        dbgFile << "\tRead layer mask/adjustment layer data. Length of block:"
                << layerMaskLength << "pos" << io->pos();

        // layer blending thingies
        quint32 blendingDataLength;
        if (!psdread(io, &blendingDataLength) || io->bytesAvailable() < blendingDataLength) {
            error = "Could not read extra blending data.";
            return false;
        }

        //dbgFile << "blending block data length" << blendingDataLength << ", pos" << io->pos();

        blendingRanges.data = io->read(blendingDataLength);
        if ((quint32)blendingRanges.data.size() != blendingDataLength) {
            error = QString("Got %1 bytes for the blending range block, needed %2").arg(blendingRanges.data.size(), blendingDataLength);
        }
        /*
        // XXX: reading this block correctly failed, I have more channel ranges than I'd expected.

        if (!psdread(io, &blendingRanges.blackValues[0]) ||
            !psdread(io, &blendingRanges.blackValues[1]) ||
            !psdread(io, &blendingRanges.whiteValues[0]) ||
            !psdread(io, &blendingRanges.whiteValues[1]) ||
            !psdread(io, &blendingRanges.compositeGrayBlendDestinationRange)) {

            error = "Could not read blending black/white values";
            return false;
        }

        for (int i = 0; i < nChannels; ++i) {
            quint32 src;
            quint32 dst;
            if (!psdread(io, &src) || !psdread(io, &dst)) {
                error = QString("could not read src/dst range for channel %1").arg(i);
                return false;
            }
            dbgFile << "\tread range " << src << "to" << dst << "for channel" << i;
            blendingRanges.sourceDestinationRanges << QPair<quint32, quint32>(src, dst);
        }
        */
        dbgFile << "\tGoing to read layer name at" << io->pos();
        quint8 layerNameLength;
        if (!psdread(io, &layerNameLength)) {
            error = "Could not read layer name length";
            return false;
        }

        dbgFile << "\tlayer name length unpadded" << layerNameLength << "pos" << io->pos();
        layerNameLength = ((layerNameLength + 1 + 3) & ~0x03) - 1;

        dbgFile << "\tlayer name length padded" << layerNameLength << "pos" << io->pos();
        layerName = io->read(layerNameLength);
        dbgFile << "\tlayer name" << layerName << io->pos();

        if (!infoBlocks.read(io)) {
            error = infoBlocks.error;
            return false;
        }

        if (infoBlocks.keys.contains("luni") && !infoBlocks.unicodeLayerName.isEmpty()) {
            layerName = infoBlocks.unicodeLayerName;
        }

    }

    return valid();
}

void PSDLayerRecord::write(QIODevice* io,
                           KisPaintDeviceSP layerContentDevice,
                           KisNodeSP onlyTransparencyMask,
                           const QRect &maskRect,
                           psd_section_type sectionType,
                           const QDomDocument &stylesXmlDoc,
                           bool useLfxsLayerStyleFormat)
{
    dbgFile << "writing layer info record" << "at" << io->pos();

    m_layerContentDevice = layerContentDevice;
    m_onlyTransparencyMask = onlyTransparencyMask;
    m_onlyTransparencyMaskRect = maskRect;

    dbgFile << "saving layer record for " << layerName << "at pos" << io->pos();
    dbgFile << "\ttop" << top << "left" << left << "bottom" << bottom << "right" << right << "number of channels" << nChannels;
    Q_ASSERT(left <= right);
    Q_ASSERT(top <= bottom);
    Q_ASSERT(nChannels > 0);

    try {
        const QRect layerRect(left, top, right - left, bottom - top);
        KisAslWriterUtils::writeRect(layerRect, io);

        {
            quint16 realNumberOfChannels = nChannels + bool(m_onlyTransparencyMask);
            SAFE_WRITE_EX(io, realNumberOfChannels);
        }

        Q_FOREACH (ChannelInfo *channel, channelInfoRecords) {
            SAFE_WRITE_EX(io, (quint16)channel->channelId);

            channel->channelInfoPosition = io->pos();

            // to be filled in when we know how big channel block is
            const quint32 fakeChannelSize = 0;
            SAFE_WRITE_EX(io, fakeChannelSize);
        }

        if (m_onlyTransparencyMask) {
            const quint16 userSuppliedMaskChannelId = -2;
            SAFE_WRITE_EX(io, userSuppliedMaskChannelId);

            m_transparencyMaskSizeOffset = io->pos();

            const quint32 fakeTransparencyMaskSize = 0;
            SAFE_WRITE_EX(io, fakeTransparencyMaskSize);
        }

        // blend mode
        dbgFile  << ppVar(blendModeKey) << ppVar(io->pos());

        KisAslWriterUtils::writeFixedString("8BIM", io);
        KisAslWriterUtils::writeFixedString(blendModeKey, io);

        SAFE_WRITE_EX(io, opacity);
        SAFE_WRITE_EX(io, clipping); // unused

        // visibility and protection
        quint8 flags = 0;
        if (transparencyProtected) flags |= 1;
        if (!visible) flags |= 2;
        if (irrelevant) {
            flags |= (1 << 3) | (1 << 4);
        }

        SAFE_WRITE_EX(io, flags);

        {
            quint8 padding = 0;
            SAFE_WRITE_EX(io, padding);
        }

        {
            // extra fields with their own length tag
            KisAslWriterUtils::OffsetStreamPusher<quint32> extraDataSizeTag(io);

            if (m_onlyTransparencyMask) {
                {
                    const quint32 layerMaskDataSize = 20; // support simple case only
                    SAFE_WRITE_EX(io, layerMaskDataSize);
                }

                KisAslWriterUtils::writeRect(m_onlyTransparencyMaskRect, io);

                {
                    // NOTE: in PSD the default color of the mask is stored in 1 byte value!
                    //       Even when the mask is actually 16/32 bit! I have no idea how it is
                    //       actually treated in this case.
                    KIS_ASSERT_RECOVER_NOOP(m_onlyTransparencyMask->paintDevice()->pixelSize() == 1);
                    const quint8 defaultPixel = *m_onlyTransparencyMask->paintDevice()->defaultPixel().data();
                    SAFE_WRITE_EX(io, defaultPixel);
                }

                {
                    const quint8 maskFlags = 0; // nothing serious
                    SAFE_WRITE_EX(io, maskFlags);

                    const quint16 padding = 0; // 2-byte padding
                    SAFE_WRITE_EX(io, padding);
                }
            } else {
                const quint32 nullLayerMaskDataSize = 0;
                SAFE_WRITE_EX(io, nullLayerMaskDataSize);
            }

            {
                // blending ranges are not implemented yet
                const quint32 nullBlendingRangesSize = 0;
                SAFE_WRITE_EX(io, nullBlendingRangesSize);
            }

            // layer name: Pascal string, padded to a multiple of 4 bytes.
            psdwrite_pascalstring(io, layerName, 4);


            PsdAdditionalLayerInfoBlock additionalInfoBlock(m_header);

            // write 'luni' data block
            additionalInfoBlock.writeLuniBlockEx(io, layerName);

            // write 'lsct' data block
            if (sectionType != psd_other) {
                additionalInfoBlock.writeLsctBlockEx(io, sectionType, isPassThrough, blendModeKey);
            }

            // write 'lfx2' data block
            if (!stylesXmlDoc.isNull()) {
                additionalInfoBlock.writeLfx2BlockEx(io, stylesXmlDoc, useLfxsLayerStyleFormat);
            }
        }
    } catch (KisAslWriterUtils::ASLWriteException &e) {
        throw KisAslWriterUtils::ASLWriteException(PREPEND_METHOD(e.what()));
    }
}

KisPaintDeviceSP PSDLayerRecord::convertMaskDeviceIfNeeded(KisPaintDeviceSP dev)
{
    KisPaintDeviceSP result = dev;

    if (m_header.channelDepth == 16) {
        result = new KisPaintDevice(*dev);
        result->convertTo(KoColorSpaceRegistry::instance()->alpha16());
    } else if (m_header.channelDepth == 32) {
        result = new KisPaintDevice(*dev);
        result->convertTo(KoColorSpaceRegistry::instance()->alpha32f());
    }
    return result;
}

void PSDLayerRecord::writeTransparencyMaskPixelData(QIODevice *io)
{
    if (m_onlyTransparencyMask) {
        KisPaintDeviceSP device = convertMaskDeviceIfNeeded(m_onlyTransparencyMask->paintDevice());

        QByteArray buffer(device->pixelSize() * m_onlyTransparencyMaskRect.width() * m_onlyTransparencyMaskRect.height(), 0);
        device->readBytes((quint8*)buffer.data(), m_onlyTransparencyMaskRect);

        PsdPixelUtils::writeChannelDataRLE(io, (quint8*)buffer.data(), device->pixelSize(), m_onlyTransparencyMaskRect, m_transparencyMaskSizeOffset, -1, true);
    }
}

void PSDLayerRecord::writePixelData(QIODevice *io)
{
    try {
        writePixelDataImpl(io);
    }  catch (KisAslWriterUtils::ASLWriteException &e) {
        throw KisAslWriterUtils::ASLWriteException(PREPEND_METHOD(e.what()));
    }
}

void PSDLayerRecord::writePixelDataImpl(QIODevice *io)
{
    dbgFile << "writing pixel data for layer" << layerName << "at" << io->pos();

    KisPaintDeviceSP dev = m_layerContentDevice;
    const QRect rc(left, top, right - left, bottom - top);

    if (rc.isEmpty()) {
        dbgFile << "Layer is empty! Writing placeholder information.";

        for (int i = 0; i < nChannels; i++) {
            const ChannelInfo *channelInfo = channelInfoRecords[i];
            KisAslWriterUtils::OffsetStreamPusher<quint32> channelBlockSizeExternalTag(io, 0, channelInfo->channelInfoPosition);
            SAFE_WRITE_EX(io, (quint16)Compression::Uncompressed);
        }

        writeTransparencyMaskPixelData(io);

        return;
    }

    // now write all the channels in display order
    dbgFile << "layer" << layerName;

    const int channelSize = m_header.channelDepth / 8;
    const psd_color_mode colorMode = m_header.colormode;

    QVector<PsdPixelUtils::ChannelWritingInfo> writingInfoList;
    Q_FOREACH (const ChannelInfo *channelInfo, channelInfoRecords) {
        writingInfoList <<
            PsdPixelUtils::ChannelWritingInfo(channelInfo->channelId,
                                              channelInfo->channelInfoPosition);
    }

    PsdPixelUtils::writePixelDataCommon(io, dev, rc, colorMode, channelSize, true, true, writingInfoList);
    writeTransparencyMaskPixelData(io);
}

bool PSDLayerRecord::valid()
{
    // XXX: check validity!
    return true;
}

bool PSDLayerRecord::readPixelData(QIODevice *io, KisPaintDeviceSP device)
{
    dbgFile << "Reading pixel data for layer" << layerName << "pos" << io->pos();

    const int channelSize = m_header.channelDepth / 8;
    const QRect layerRect = QRect(left,
                                  top,
                                  right - left,
                                  bottom - top);

    try {
        PsdPixelUtils::readChannels(io, device, m_header.colormode, channelSize, layerRect, channelInfoRecords);
    } catch (KisAslReaderUtils::ASLParseException &e) {
        device->clear();
        error = e.what();
        return false;
    }

    return true;
}

QRect PSDLayerRecord::channelRect(ChannelInfo *channel) const
{
    QRect result;

    if (channel->channelId < -1) {
        result = QRect(layerMask.left,
                       layerMask.top,
                       layerMask.right - layerMask.left,
                       layerMask.bottom - layerMask.top);
    } else {
        result = QRect(left,
                       top,
                       right - left,
                       bottom - top);
    }

    return result;
}


bool PSDLayerRecord::readMask(QIODevice *io, KisPaintDeviceSP dev, ChannelInfo *channelInfo)
{
    KIS_ASSERT_RECOVER(channelInfo->channelId < -1) { return false; }

    dbgFile << "Going to read" << channelIdToChannelType(channelInfo->channelId, m_header.colormode) << "mask";

    QRect maskRect = channelRect(channelInfo);
    if (maskRect.isEmpty()) {
        dbgFile << "Empty Channel";
        return true;
    }

    // the device must be a pixel selection
    KIS_ASSERT_RECOVER(dev->pixelSize() == 1) { return false; }

    dev->setDefaultPixel(KoColor(&layerMask.defaultColor, dev->colorSpace()));

    const int pixelSize =
        m_header.channelDepth == 16 ? 2 :
        m_header.channelDepth == 32 ? 4 : 1;

    QVector<ChannelInfo*> infoRecords;
    infoRecords << channelInfo;
    PsdPixelUtils::readAlphaMaskChannels(io, dev, pixelSize, maskRect, infoRecords);

    return true;
}

QDebug operator<<(QDebug dbg, const PSDLayerRecord &layer)
{
#ifndef NODEBUG
    dbg.nospace() << "valid: " << const_cast<PSDLayerRecord*>(&layer)->valid();
    dbg.nospace() << ", name: " << layer.layerName;
    dbg.nospace() << ", top: " << layer.top;
    dbg.nospace() << ", left:" << layer.left;
    dbg.nospace() << ", bottom: " << layer.bottom;
    dbg.nospace() << ", right: " << layer.right;
    dbg.nospace() << ", number of channels: " << layer.nChannels;
    dbg.nospace() << ", blendModeKey: " << layer.blendModeKey;
    dbg.nospace() << ", opacity: " << layer.opacity;
    dbg.nospace() << ", clipping: " << layer.clipping;
    dbg.nospace() << ", transparency protected: " << layer.transparencyProtected;
    dbg.nospace() << ", visible: " << layer.visible;
    dbg.nospace() << ", irrelevant: " << layer.irrelevant << "\n";
    Q_FOREACH (const ChannelInfo* channel, layer.channelInfoRecords) {
        dbg.space() << channel;
    }
#endif
    return dbg.nospace();
}


QDebug operator<<(QDebug dbg, const ChannelInfo &channel)
{
#ifndef NODEBUG
    dbg.nospace() << "\tChannel type" << channel.channelId
                  << "size: " << channel.channelDataLength
                  << "compression type" << channel.compressionType << "\n";
#endif
    return dbg.nospace();
}
