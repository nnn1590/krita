/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman <geneing at gmail dot com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "histogramdockerwidget.h"

#include <QThread>
#include <QVector>
#include <limits>
#include <algorithm>
#include <QTime>
#include <QPainter>
#include <QPainterPath>
#include <functional>

#include "KoChannelInfo.h"
#include "kis_paint_device.h"
#include "KoColorSpace.h"
#include "kis_iterator_ng.h"
#include "kis_canvas2.h"

HistogramDockerWidget::HistogramDockerWidget(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QLabel(parent, f), m_colorSpace(0), m_smoothHistogram(true)
{
    setObjectName(name);
}

HistogramDockerWidget::~HistogramDockerWidget()
{

}

void HistogramDockerWidget::updateHistogram(KisCanvas2* canvas)
{
    if (canvas) {
        KisPaintDeviceSP paintDevice = canvas->image()->projection();
        QRect bounds = canvas->image()->bounds();

        // remember to save the color space to paint the histogram data!
        m_colorSpace = paintDevice->colorSpace();

        KisPaintDeviceSP m_devClone = new KisPaintDevice(paintDevice->colorSpace());

        m_devClone->makeCloneFrom(paintDevice, bounds);

        HistogramComputationThread *workerThread = new HistogramComputationThread(m_devClone, bounds);
        connect(workerThread, &HistogramComputationThread::resultReady, this, &HistogramDockerWidget::receiveNewHistogram);
        connect(workerThread, &HistogramComputationThread::finished, workerThread, &QObject::deleteLater);
        workerThread->start();
    } else {
        m_histogramData.clear();
        update();
    }
}

void HistogramDockerWidget::receiveNewHistogram(HistVector *histogramData)
{
    m_histogramData = *histogramData;
    update();
}

void HistogramDockerWidget::paintEvent(QPaintEvent *event)
{
    if (m_colorSpace && !m_histogramData.empty()) {
        int nBins = m_histogramData.at(0).size();
        const KoColorSpace* cs = m_colorSpace;

        QLabel::paintEvent(event);
        QPainter painter(this);
        painter.fillRect(0, 0, this->width(), this->height(), this->palette().dark().color());
        painter.setPen(this->palette().light().color());

        const int NGRID = 4;
        for (int i = 0; i <= NGRID; ++i) {
            painter.drawLine(this->width()*i / NGRID, 0., this->width()*i / NGRID, this->height());
            painter.drawLine(0., this->height()*i / NGRID, this->width(), this->height()*i / NGRID);
        }

        unsigned int nChannels = cs->channelCount();
        QList<KoChannelInfo *> channels = cs->channels();
        unsigned int highest = 0;
        //find the most populous bin in the histogram to scale it properly
        for (int chan = 0; chan < channels.size(); chan++) {
            if (channels.at(chan)->channelType() != KoChannelInfo::ALPHA) {
                std::vector<quint32> histogramTemp = m_histogramData.at(chan);
                //use 98th percentile, rather than max for better visual appearance
                int nthPercentile = 2 * histogramTemp.size() / 100;
                //unsigned int max = *std::max_element(m_histogramData.at(chan).begin(),m_histogramData.at(chan).end());
                std::nth_element(histogramTemp.begin(),
                                 histogramTemp.begin() + nthPercentile, histogramTemp.end(), std::greater<int>());
                unsigned int max = *(histogramTemp.begin() + nthPercentile);

                highest = std::max(max, highest);
            }
        }

        painter.setWindow(QRect(-1, 0, nBins + 1, highest));
        painter.setCompositionMode(QPainter::CompositionMode_Plus);

        for (int chan = 0; chan < (int)nChannels; chan++) {
            if (channels.at(chan)->channelType() != KoChannelInfo::ALPHA) {
                QColor color = channels.at(chan)->color();

                //special handling of grayscale color spaces. can't use color returned above.
                if(cs->colorChannelCount()==1){
                    color = QColor(Qt::gray);
                }

                QColor fill_color = color;
                fill_color.setAlphaF(.25);
                painter.setBrush(fill_color);
                QPen pen = QPen(color);
                pen.setWidth(0);
                painter.setPen(pen);

                if (m_smoothHistogram) {
                    QPainterPath path;
                    path.moveTo(QPointF(-1, highest));
                    for (qint32 i = 0; i < nBins; ++i) {
                        float v = std::max((float)highest - m_histogramData[chan][i], 0.f);
                        path.lineTo(QPointF(i, v));

                    }
                    path.lineTo(QPointF(nBins + 1, highest));
                    path.closeSubpath();
                    painter.drawPath(path);
                } else {
                    pen.setWidth(1);
                    painter.setPen(pen);
                    for (qint32 i = 0; i < nBins; ++i) {
                        float v = std::max((float)highest - m_histogramData[chan][i], 0.f);
                        painter.drawLine(QPointF(i, highest), QPointF(i, v));
                    }
                }
            }
        }
    }
}

void HistogramComputationThread::run()
{
    const KoColorSpace *cs = m_dev->colorSpace();
    quint32 channelCount = m_dev->channelCount();
    quint32 pixelSize = m_dev->pixelSize();

    quint32 imageSize = m_bounds.width() * m_bounds.height();
    quint32 nSkip = 1 + (imageSize >> 20); //for speed use about 1M pixels for computing histograms

    //allocate space for the histogram data
    bins.resize((int)channelCount);
    for (auto &bin : bins) {
        bin.resize(std::numeric_limits<quint8>::max() + 1);
    }

    QRect bounds = m_dev->exactBounds();
    if (bounds.isEmpty())
        return;

    quint32 toSkip = nSkip;

    KisSequentialConstIterator it(m_dev, m_dev->exactBounds());

    int numConseqPixels = it.nConseqPixels();
    while (it.nextPixels(numConseqPixels)) {

        numConseqPixels = it.nConseqPixels();
        const quint8* pixel = it.rawDataConst();
        for (int k = 0; k < numConseqPixels; ++k) {
            if (--toSkip == 0) {
                for (int chan = 0; chan < (int)channelCount; ++chan) {
                    bins[chan][cs->scaleToU8(pixel, chan)]++;
                }
                toSkip = nSkip;
            }
            pixel += pixelSize;
        }
    }

    emit resultReady(&bins);
}
