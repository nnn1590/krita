/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_edge_detection_kernel.h"
#include "kis_global.h"
#include "kis_convolution_kernel.h"
#include <kis_convolution_painter.h>
#include <KoCompositeOpRegistry.h>
#include <QRect>
#include <KoColorSpace.h>
#include <kis_iterator_ng.h>
#include <QVector3D>

KisEdgeDetectionKernel::KisEdgeDetectionKernel()
{

}
/*
 * This code is very similar to the gaussian kernel code, except unlike the gaussian code,
 * edge-detection kernels DO use the diagonals.
 * Except for the simple mode. We implement the simple mode because it is an analog to
 * the old sobel filter.
 */

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> KisEdgeDetectionKernel::createHorizontalMatrix(qreal radius,
                                                                                                    FilterType type,
                                                                                                    bool reverse)
{
    int kernelSize = kernelSizeFromRadius(radius);
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, kernelSize);

    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    if (type==Prewit) {
        for (int x = 0; x < kernelSize; x++) {
            for (int y=0; y<kernelSize; y++) {
                qreal xDistance;
                if (reverse) {
                    xDistance = x - center;
                } else {
                    xDistance = center - x;
                }
                matrix(x, y) = xDistance;
            }
        }
    } else if(type==Simple) {
        matrix.resize(kernelSize, 1);
        for (int x = 0; x < kernelSize; x++) {
            qreal xDistance;
            if (reverse) {
                xDistance = x - center;
            } else {
                xDistance = center - x;
            }
            if (x==center) {
                matrix(x, 0) = 0;
            } else {
                matrix(x, 0) = (1/xDistance);
            }
        }
    } else {
        for (int x = 0; x < kernelSize; x++) {
            for (int y=0; y<kernelSize; y++) {
                if (x==center && y==center) {
                    matrix(x, y) = 0;
                } else {
                    qreal xD, yD;
                    if (reverse) {
                        xD = x - center;
                        yD = y - center;
                    } else {
                        xD = center - x;
                        yD = center - y;
                    }
                    matrix(x, y) = xD / (xD*xD + yD*yD);
                }
            }
        }
    }
    return matrix;
}

Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> KisEdgeDetectionKernel::createVerticalMatrix(qreal radius,
                                                                                                  FilterType type,
                                                                                                  bool reverse)
{
    int kernelSize = kernelSizeFromRadius(radius);

    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix(kernelSize, kernelSize);
    KIS_ASSERT_RECOVER_NOOP(kernelSize & 0x1);
    const int center = kernelSize / 2;

    if (type==Prewit) {
        for (int y = 0; y < kernelSize; y++) {
            for (int x=0; x<kernelSize; x++) {
                qreal yDistance;
                if (reverse) {
                    yDistance = y - center;
                } else {
                    yDistance = center - y;
                }
                matrix(x, y) = yDistance;
            }
        }
    } else if(type==Simple) {
        matrix.resize(1, kernelSize);
        for (int y = 0; y < kernelSize; y++) {
            qreal yDistance;
            if (reverse) {
                yDistance = y - center;
            } else {
                yDistance = center - y;
            }
            if (y==center) {
                matrix(0, y) = 0;
            } else {
                matrix(0, y) = (1/yDistance);
            }
        }
    } else {
        for (int y = 0; y < kernelSize; y++) {
            for (int x=0; x<kernelSize; x++) {
                if (x==center && y==center) {
                    matrix(x, y) = 0;
                } else {
                    qreal xD, yD;
                    if (reverse) {
                        xD = x - center;
                        yD = y - center;
                    } else {
                        xD = center - x;
                        yD = center - y;
                    }
                    matrix(x, y) = yD / (xD*xD + yD*yD);
                }
            }
        }
    }
    return matrix;
}

KisConvolutionKernelSP KisEdgeDetectionKernel::createHorizontalKernel(qreal radius,
                                                                      KisEdgeDetectionKernel::FilterType type,
                                                                      bool denormalize,
                                                                      bool reverse)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createHorizontalMatrix(radius, type, reverse);
    if (denormalize) {
        return KisConvolutionKernel::fromMatrix(matrix, 0.5, 1);
    } else {
        return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
    }
}

KisConvolutionKernelSP KisEdgeDetectionKernel::createVerticalKernel(qreal radius,
                                                                    KisEdgeDetectionKernel::FilterType type,
                                                                    bool denormalize,
                                                                    bool reverse)
{
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix = createVerticalMatrix(radius, type, reverse);
    if (denormalize) {
        return KisConvolutionKernel::fromMatrix(matrix, 0.5, 1);
    } else {
        return KisConvolutionKernel::fromMatrix(matrix, 0, matrix.sum());
    }
}

int KisEdgeDetectionKernel::kernelSizeFromRadius(qreal radius)
{
    return qMax((int)(2 * ceil(sigmaFromRadius(radius)) + 1), 3);
}

qreal KisEdgeDetectionKernel::sigmaFromRadius(qreal radius)
{
    return 0.3 * radius + 0.3;
}

void KisEdgeDetectionKernel::applyEdgeDetection(KisPaintDeviceSP device,
                                                const QRect &rect,
                                                qreal xRadius,
                                                qreal yRadius,
                                                KisEdgeDetectionKernel::FilterType type,
                                                const QBitArray &channelFlags,
                                                KoUpdater *progressUpdater,
                                                FilterOutput output,
                                                bool writeToAlpha)
{
    QPoint srcTopLeft = rect.topLeft();
    KisPainter finalPainter(device);
    finalPainter.setChannelFlags(channelFlags);
    finalPainter.setProgress(progressUpdater);
    if (output == pythagorean || output == radian) {
        KisPaintDeviceSP x_denormalised = new KisPaintDevice(device->colorSpace());
        KisPaintDeviceSP y_denormalised = new KisPaintDevice(device->colorSpace());

        x_denormalised->prepareClone(device);
        y_denormalised->prepareClone(device);


        KisConvolutionKernelSP kernelHorizLeftRight = KisEdgeDetectionKernel::createHorizontalKernel(xRadius, type);
        KisConvolutionKernelSP kernelVerticalTopBottom = KisEdgeDetectionKernel::createVerticalKernel(yRadius, type);

        KisConvolutionPainter horizPainterLR(x_denormalised);
        horizPainterLR.setChannelFlags(channelFlags);
        horizPainterLR.setProgress(progressUpdater);
        horizPainterLR.applyMatrix(kernelHorizLeftRight, device,
                                   srcTopLeft,
                                   srcTopLeft,
                                   rect.size(), BORDER_REPEAT);


        KisConvolutionPainter verticalPainterTB(y_denormalised);
        verticalPainterTB.setChannelFlags(channelFlags);
        verticalPainterTB.setProgress(progressUpdater);
        verticalPainterTB.applyMatrix(kernelVerticalTopBottom, device,
                                      srcTopLeft,
                                      srcTopLeft,
                                      rect.size(), BORDER_REPEAT);

        KisSequentialIterator yItterator(y_denormalised, rect);
        KisSequentialIterator xItterator(x_denormalised, rect);
        KisSequentialIterator finalIt(device, rect);
        const int pixelSize = device->colorSpace()->pixelSize();
        const int channels = device->colorSpace()->channelCount();
        const int alphaPos = device->colorSpace()->alphaPos();
        KIS_SAFE_ASSERT_RECOVER_RETURN(alphaPos >= 0);

        QVector<float> yNormalised(channels);
        QVector<float> xNormalised(channels);
        QVector<float> finalNorm(channels);

        while(yItterator.nextPixel() && xItterator.nextPixel() && finalIt.nextPixel()) {
            device->colorSpace()->normalisedChannelsValue(yItterator.rawData(), yNormalised);
            device->colorSpace()->normalisedChannelsValue(xItterator.rawData(), xNormalised);
            device->colorSpace()->normalisedChannelsValue(finalIt.rawData(), finalNorm);

            if (output == pythagorean) {
                for (int c = 0; c<channels; c++) {
                    finalNorm[c] = 2 * sqrt( ((xNormalised[c]-0.5)*(xNormalised[c]-0.5)) + ((yNormalised[c]-0.5)*(yNormalised[c]-0.5)));
                }
            } else { //radian
                for (int c = 0; c<channels; c++) {
                    finalNorm[c] = atan2(xNormalised[c]-0.5, yNormalised[c]-0.5);
                }
            }

            if (writeToAlpha) {
                KoColor col(finalIt.rawData(), device->colorSpace());
                qreal alpha = 0;

                for (int c = 0; c<(channels-1); c++) {
                    alpha = alpha+finalNorm[c];
                }

                alpha = qMin(alpha/(channels-1), col.opacityF());
                col.setOpacity(alpha);
                memcpy(finalIt.rawData(), col.data(), pixelSize);
            } else {
                quint8* f = finalIt.rawData();
                finalNorm[alphaPos] = 1.0;
                device->colorSpace()->fromNormalisedChannelsValue(f, finalNorm);
                memcpy(finalIt.rawData(), f, pixelSize);
            }

        }
    } else {
        KisConvolutionKernelSP kernel;
        bool denormalize = !writeToAlpha;
        if (output == xGrowth) {
            kernel = KisEdgeDetectionKernel::createHorizontalKernel(xRadius, type, denormalize);
        } else if (output == xFall) {
            kernel = KisEdgeDetectionKernel::createHorizontalKernel(xRadius, type, denormalize, true);
        } else if (output == yGrowth) {
            kernel = KisEdgeDetectionKernel::createVerticalKernel(yRadius, type, denormalize);
        } else { //yFall
            kernel = KisEdgeDetectionKernel::createVerticalKernel(yRadius, type, denormalize, true);
        }

        if (writeToAlpha) {
            KisPaintDeviceSP denormalised = new KisPaintDevice(device->colorSpace());
            denormalised->prepareClone(device);

            KisConvolutionPainter kernelP(denormalised);
            kernelP.setChannelFlags(channelFlags);
            kernelP.setProgress(progressUpdater);
            kernelP.applyMatrix(kernel, device,
                                srcTopLeft, srcTopLeft,
                                rect.size(), BORDER_REPEAT);
            KisSequentialIterator iterator(denormalised, rect);
            KisSequentialIterator finalIt(device, rect);
            const int pixelSize = device->colorSpace()->pixelSize();
            const int channels = device->colorSpace()->colorChannelCount();
            QVector<float> normalised(channels);
            while (iterator.nextPixel() && finalIt.nextPixel()) {
                device->colorSpace()->normalisedChannelsValue(iterator.rawData(), normalised);
                KoColor col(finalIt.rawData(), device->colorSpace());
                qreal alpha = 0;
                for (int c = 0; c<channels; c++) {
                    alpha = alpha+normalised[c];
                }
                alpha = qMin(alpha/channels, col.opacityF());
                col.setOpacity(alpha);
                memcpy(finalIt.rawData(), col.data(), pixelSize);

            }

        } else {
            KisConvolutionPainter kernelP(device);
            kernelP.setChannelFlags(channelFlags);
            kernelP.setProgress(progressUpdater);
            kernelP.applyMatrix(kernel, device,
                                srcTopLeft, srcTopLeft,
                                rect.size(), BORDER_REPEAT);

            KisSequentialIterator finalIt(device, rect);
            int numConseqPixels = finalIt.nConseqPixels();
            while (finalIt.nextPixels(numConseqPixels)) {
                numConseqPixels = finalIt.nConseqPixels();
                device->colorSpace()->setOpacity(finalIt.rawData(), 1.0, numConseqPixels);
            }
        }
    }
}

void KisEdgeDetectionKernel::convertToNormalMap(KisPaintDeviceSP device,
                                                const QRect &rect,
                                                qreal xRadius,
                                                qreal yRadius,
                                                KisEdgeDetectionKernel::FilterType type,
                                                int channelToConvert,
                                                QVector<int> channelOrder,
                                                QVector<bool> channelFlip,
                                                const QBitArray &channelFlags,
                                                KoUpdater *progressUpdater,
                                                boost::optional<bool> useFftw)
{
    QPoint srcTopLeft = rect.topLeft();
    KisPainter finalPainter(device);
    finalPainter.setChannelFlags(channelFlags);
    finalPainter.setProgress(progressUpdater);
    KisPaintDeviceSP x_denormalised = new KisPaintDevice(device->colorSpace());
    KisPaintDeviceSP y_denormalised = new KisPaintDevice(device->colorSpace());
    x_denormalised->prepareClone(device);
    y_denormalised->prepareClone(device);

    KisConvolutionKernelSP kernelHorizLeftRight = KisEdgeDetectionKernel::createHorizontalKernel(yRadius, type, true, !channelFlip[1]);
    KisConvolutionKernelSP kernelVerticalTopBottom = KisEdgeDetectionKernel::createVerticalKernel(xRadius, type, true, !channelFlip[0]);

    KisConvolutionPainter horizPainterLR(y_denormalised);

    if (useFftw) {
        horizPainterLR.setEnginePreference(*useFftw ? KisConvolutionPainter::FFTW : KisConvolutionPainter::SPATIAL);
    }

    horizPainterLR.setChannelFlags(channelFlags);
    horizPainterLR.setProgress(progressUpdater);
    horizPainterLR.applyMatrix(kernelHorizLeftRight, device,
                               srcTopLeft, srcTopLeft,
                               rect.size(), BORDER_REPEAT);


    KisConvolutionPainter verticalPainterTB(x_denormalised);

    if (useFftw) {
        verticalPainterTB.setEnginePreference(*useFftw ? KisConvolutionPainter::FFTW : KisConvolutionPainter::SPATIAL);
    }

    verticalPainterTB.setChannelFlags(channelFlags);
    verticalPainterTB.setProgress(progressUpdater);
    verticalPainterTB.applyMatrix(kernelVerticalTopBottom, device,
                                  srcTopLeft,
                                  srcTopLeft,
                                  rect.size(), BORDER_REPEAT);

    KisSequentialIterator yItterator(y_denormalised, rect);
    KisSequentialIterator xItterator(x_denormalised, rect);
    KisSequentialIterator finalIt(device, rect);
    const int pixelSize = device->colorSpace()->pixelSize();
    const int channels = device->colorSpace()->channelCount();
    const int alphaPos = device->colorSpace()->alphaPos();
    KIS_SAFE_ASSERT_RECOVER_RETURN(alphaPos >= 0);

    QVector<float> yNormalised(channels);
    QVector<float> xNormalised(channels);
    QVector<float> finalNorm(channels);

    while(yItterator.nextPixel() && xItterator.nextPixel() && finalIt.nextPixel()) {
        device->colorSpace()->normalisedChannelsValue(yItterator.rawData(), yNormalised);
        device->colorSpace()->normalisedChannelsValue(xItterator.rawData(), xNormalised);

        qreal z = 1.0;
        if (channelFlip[2]==true){
            z=-1.0;
        }
        QVector3D normal = QVector3D((xNormalised[channelToConvert]-0.5)*2, (yNormalised[channelToConvert]-0.5)*2, z);
        normal.normalize();
        finalNorm.fill(1.0);
        for (int c = 0; c<3; c++) {
            finalNorm[device->colorSpace()->channels().at(channelOrder[c])->displayPosition()] = (normal[channelOrder[c]]/2)+0.5;
        }

        finalNorm[alphaPos]= 1.0;

        quint8* pixel = finalIt.rawData();
        device->colorSpace()->fromNormalisedChannelsValue(pixel, finalNorm);
        memcpy(finalIt.rawData(), pixel, pixelSize);

    }
}
