/*
 *  This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MATH_TOOLBOX_H
#define KIS_MATH_TOOLBOX_H

#include <QObject>
#include <QRect>

#include <new>

#include <KoColorSpace.h>

#include "kis_types.h"
#include "kis_paint_device.h"

#ifdef _MSC_VER
#pragma warning(disable: 4290) // disable "C++ exception specification ignored" warning
#endif

#if !defined _MSC_VER
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

typedef double(*PtrToDouble)(const quint8*, int);
typedef void (*PtrFromDouble)(quint8*, int, double);
typedef void (*PtrFromDoubleCheckNull)(quint8*, int, double, bool*);

class KRITAIMAGE_EXPORT KisMathToolbox
{

public:

    struct KisFloatRepresentation {

        KisFloatRepresentation(uint nsize, uint ndepth)
            : coeffs(new float[nsize*nsize*ndepth])
            , size(nsize)
            , depth(ndepth) {
            // XXX: Valgrind shows that these are being used without being initialised.
            for (quint32 i = 0; i < nsize * nsize * ndepth; ++i) {
                coeffs[i] = 0;
            }
        }

        ~KisFloatRepresentation() {
            if (coeffs) delete[] coeffs;
        }

        float* coeffs;
        uint size;
        uint depth;
    };

    typedef KisFloatRepresentation KisWavelet;

    /**
     * This function initializes a wavelet structure
     * @param lay the layer that will be used for the transformation
     * @param rect the rectangular for transformation
     */
    inline KisWavelet* initWavelet(KisPaintDeviceSP lay, const QRect&);

    inline uint fastWaveletTotalSteps(const QRect&);

    /**
     * This function reconstruct the layer from the information of a wavelet
     * @param src layer from which the wavelet will be computed
     * @param rect the rectangular for reconstruction
     * @param buff if set to 0, the buffer will be initialized by the function,
     * you might want to give a buff to the function if you want to use the same buffer
     * in transformToWavelet and in untransformToWavelet, use initWavelet to initialize
     * the buffer
     */
    KisWavelet* fastWaveletTransformation(KisPaintDeviceSP src, const QRect&, KisWavelet* buff = 0);

    /**
     * This function reconstruct the layer from the information of a wavelet
     * @param dst layer on which the wavelet will be untransform
     * @param rect the rectangular for reconstruction
     * @param wav the wavelet
     * @param buff if set to 0, the buffer will be initialized by the function,
     * you might want to give a buff to the function if you want to use the same buffer
     * in transformToWavelet and in untransformToWavelet, use initWavelet to initialize
     * the buffer
     */
    void fastWaveletUntransformation(KisPaintDeviceSP dst, const QRect&, KisWavelet* wav, KisWavelet* buff = 0);

    bool getToDoubleChannelPtr(QList<KoChannelInfo *> cis, QVector<PtrToDouble>& f);
    bool getFromDoubleChannelPtr(QList<KoChannelInfo *> cis, QVector<PtrFromDouble>& f);
    bool getFromDoubleCheckNullChannelPtr(QList<KoChannelInfo *> cis, QVector<PtrFromDoubleCheckNull>& f);

    double minChannelValue(KoChannelInfo *);
    double maxChannelValue(KoChannelInfo *);

private:

    void wavetrans(KisWavelet* wav, KisWavelet* buff, uint halfsize);
    void waveuntrans(KisWavelet* wav, KisWavelet* buff, uint halfsize);

    /**
     * This function transform a paint device into a KisFloatRepresentation, this function is colorspace independent,
     * for Wavelet, Pyramid and FFT the data is always the exact value of the channel stored in a float.
     */
    void transformToFR(KisPaintDeviceSP src, KisFloatRepresentation*, const QRect&);

    /**
     * This function transform a KisFloatRepresentation into a paint device, this function is colorspace independent,
     * for Wavelet, Pyramid and FFT the data is always the exact value of the channel stored in a float.
     */
    void transformFromFR(KisPaintDeviceSP dst, KisFloatRepresentation*, const QRect&);

};

inline KisMathToolbox::KisWavelet* KisMathToolbox::initWavelet(KisPaintDeviceSP src, const QRect& rect)
{
    int size;
    int maxrectsize = (rect.height() < rect.width()) ? rect.width() : rect.height();
    for (size = 2; size < maxrectsize; size *= 2) ;
    qint32 depth = src->colorSpace()->colorChannelCount();
    return new KisWavelet(size, depth);
}

inline uint KisMathToolbox::fastWaveletTotalSteps(const QRect& rect)
{
    int size, steps;
    int maxrectsize = (rect.height() < rect.width()) ? rect.width() : rect.height();
    steps = 0;
    for (size = 2; size < maxrectsize; size *= 2) steps += size / 2; ;
    return steps;
}

#endif
