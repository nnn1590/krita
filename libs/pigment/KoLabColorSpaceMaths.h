/*
 *  Copyright (c) 2006,2007,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2017,2020 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KOLABCOLORSPACEMATHS_H_
#define KOLABCOLORSPACEMATHS_H_

#include <cmath>
#include <limits>

#include "kritapigment_export.h"
#include <KoIntegerMaths.h>
#include "KoChannelInfo.h"
#include "KoLut.h"

#include <KoColorSpaceMaths.h>

#undef _T

/**
 * This is an empty mainWindow that needs to be "specialized" for each possible
 * numerical type (quint8, quint16...).
 *
 * It needs to defines some static constant fields :
 * - zeroValue : the zero for this numerical type
 * - unitValue : the maximum value of the normal dynamic range
 * - max : the maximum value
 * - min : the minimum value
 * - epsilon : a value close to zero but different of zero
 * - bits : the bit depth
 *
 * And some types :
 * - compositetype the type used for composite operations (usually one with
 *   a higher bit depth)
 *
 * This class is specialized to handle the floating point bounds of the Lab color space.
 */

template<typename _T>
class KoLabColorSpaceMathsTraits
{
public:
};

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<quint8> : public KoColorSpaceMathsTraits<quint8>
{
public:
    static const quint8 zeroValueL = 0;
    static const quint8 unitValueL = 0x00FF;
    static const quint8 halfValueL = 0x00FF / 2;
    static const quint8 zeroValueAB = 0;
    static const quint8 unitValueAB = 0x00FF;
    static const quint8 halfValueAB = 0x0080;
};

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<quint16> : public KoColorSpaceMathsTraits<quint16>
{
public:
    static const quint16 zeroValueL = 0;
    static const quint16 unitValueL = 0xFFFF;
    static const quint16 halfValueL = 0xFFFF / 2;
    static const quint16 zeroValueAB = 0;
    static const quint16 unitValueAB = 0xFFFF;
    static const quint16 halfValueAB = 0x8080;
};

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<qint16> : public KoColorSpaceMathsTraits<qint16>
{
public:
    static const qint16 zeroValueL = 0;
    static const qint16 unitValueL = 32767;
    static const qint16 halfValueL = 32767 / 2;
    static const qint16 zeroValueAB = 0;
    static const qint16 unitValueAB = 32767;
    static const qint16 halfValueAB = 19549;
};

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<quint32> : public KoColorSpaceMathsTraits<quint32>
{
public:
    static const quint32 zeroValueL = 0;
    static const quint32 unitValueL = 0xFFFFFFFF;
    static const quint32 halfValueL = 0xFFFFFFFF / 2;
    static const quint32 zeroValueAB = 0;
    static const quint32 unitValueAB = 0xFFFFFFFF;
    static const quint32 halfValueAB = 0x80808080;
};

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<half> : public KoColorSpaceMathsTraits<half>
{
public:
    static const half zeroValueL;
    static const half unitValueL;
    static const half halfValueL;
    static const half zeroValueAB;
    static const half unitValueAB;
    static const half halfValueAB;
};
#endif

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<float> : public KoColorSpaceMathsTraits<float>
{
public:
    static const float zeroValueL;
    static const float unitValueL;
    static const float halfValueL;
    static const float zeroValueAB;
    static const float unitValueAB;
    static const float halfValueAB;
};

template<>
class KRITAPIGMENT_EXPORT KoLabColorSpaceMathsTraits<double> : public KoColorSpaceMathsTraits<double>
{
public:
    static const double zeroValueL;
    static const double unitValueL;
    static const double halfValueL;
    static const double zeroValueAB;
    static const double unitValueAB;
    static const double halfValueAB;
};

//template<typename _T_>
//struct KoIntegerToFloat {
//  inline float operator()(_T_ f) const
//  {
//    return f / float(KoColorSpaceMathsTraits<_T_>::max);
//  }
//};

//struct KoLuts {

//  static KRITAPIGMENT_EXPORT const Ko::FullLut< KoIntegerToFloat<quint16>, float, quint16> Uint16ToFloat;
//  static KRITAPIGMENT_EXPORT const Ko::FullLut< KoIntegerToFloat<quint8>, float, quint8> Uint8ToFloat;
//};

///**
// * This class defines some elementary operations used by various color
// * space. It's intended to be generic, but some specialization exists
// * either for optimization or just for being buildable.
// *
// * @param _T some numerical type with an existing trait
// * @param _Tdst some other numerical type with an existing trait, it is
// *              only needed if different of _T
// */
//template < typename _T, typename _Tdst = _T >
//class KoColorSpaceMaths
//{
//    typedef KoColorSpaceMathsTraits<_T> traits;
//    typedef typename traits::compositetype src_compositetype;
//    typedef typename KoColorSpaceMathsTraits<_Tdst>::compositetype dst_compositetype;

//public:
//    inline static _Tdst multiply(_T a, _Tdst b) {
//        return (dst_compositetype(a)*b) /  KoColorSpaceMathsTraits<_Tdst>::unitValue;
//    }

//    inline static _Tdst multiply(_T a, _Tdst b, _Tdst c) {
//        return (dst_compositetype(a)*b*c) / (dst_compositetype(KoColorSpaceMathsTraits<_Tdst>::unitValue) * KoColorSpaceMathsTraits<_T>::unitValue);
//    }

//    /**
//     * Division : (a * MAX ) / b
//     * @param a
//     * @param b
//     */
//    inline static dst_compositetype divide(_T a, _Tdst b) {
//        return (dst_compositetype(a) *  KoColorSpaceMathsTraits<_Tdst>::unitValue) / b;
//    }

//    /**
//     * Inversion : unitValue - a
//     * @param a
//     */
//    inline static _T invert(_T a) {
//        return traits::unitValue - a;
//    }

//    /**
//     * Blending : (a * alpha) + b * (1 - alpha)
//     * @param a
//     * @param b
//     * @param alpha
//     */
//    inline static _T blend(_T a, _T b, _T alpha) {
//        src_compositetype c = ((src_compositetype(a) - b) * alpha) / traits::unitValue;
//        return c + b;
//    }

//    /**
//     * This function will scale a value of type _T to fit into a _Tdst.
//     */
//    inline static _Tdst scaleToA(_T a) {
//        return _Tdst(dst_compositetype(a) * KoColorSpaceMathsTraits<_Tdst>::unitValue / KoColorSpaceMathsTraits<_T>::unitValue);
//    }

//    inline static dst_compositetype clamp(dst_compositetype val) {
//        return qBound<dst_compositetype>(KoColorSpaceMathsTraits<_Tdst>::min, val, KoColorSpaceMathsTraits<_Tdst>::max);
//    }

//    /**
//     * Clamps the composite type on higher border only. That is a fast path
//     * for scale-only transformations
//     */
//    inline static _Tdst clampAfterScale(dst_compositetype val) {
//        return qMin<dst_compositetype>(val, KoColorSpaceMathsTraits<_Tdst>::max);
//    }
//};

////------------------------------ double specialization ------------------------------//
//template<>
//inline quint8 KoColorSpaceMaths<double, quint8>::scaleToA(double a)
//{
//    double v = a * 255;
//    return float2int(CLAMP(v, 0, 255));
//}

//template<>
//inline double KoColorSpaceMaths<quint8, double>::scaleToA(quint8 a)
//{
//    return KoLuts::Uint8ToFloat(a);
//}

//template<>
//inline quint16 KoColorSpaceMaths<double, quint16>::scaleToA(double a)
//{
//    double v = a * 0xFFFF;
//    return float2int(CLAMP(v, 0, 0xFFFF));
//}

//template<>
//inline double KoColorSpaceMaths<quint16, double>::scaleToA(quint16 a)
//{
//    return KoLuts::Uint16ToFloat(a);
//}

//template<>
//inline double KoColorSpaceMaths<double>::clamp(double a)
//{
//    return a;
//}

////------------------------------ float specialization ------------------------------//

//template<>
//inline float KoColorSpaceMaths<double, float>::scaleToA(double a)
//{
//    return (float)a;
//}

//template<>
//inline double KoColorSpaceMaths<float, double>::scaleToA(float a)
//{
//    return a;
//}

//template<>
//inline quint16 KoColorSpaceMaths<float, quint16>::scaleToA(float a)
//{
//    float v = a * 0xFFFF;
//    return (quint16)float2int(CLAMP(v, 0, 0xFFFF));
//}

//template<>
//inline float KoColorSpaceMaths<quint16, float>::scaleToA(quint16 a)
//{
//    return KoLuts::Uint16ToFloat(a);
//}

//template<>
//inline quint8 KoColorSpaceMaths<float, quint8>::scaleToA(float a)
//{
//    float v = a * 255;
//    return (quint8)float2int(CLAMP(v, 0, 255));
//}

//template<>
//inline float KoColorSpaceMaths<quint8, float>::scaleToA(quint8 a)
//{
//    return KoLuts::Uint8ToFloat(a);
//}

//template<>
//inline float KoColorSpaceMaths<float>::blend(float a, float b, float alpha)
//{
//    return (a - b) * alpha + b;
//}

//template<>
//inline double KoColorSpaceMaths<float>::clamp(double a)
//{
//    return a;
//}

////------------------------------ half specialization ------------------------------//

//#ifdef HAVE_OPENEXR

//template<>
//inline half KoColorSpaceMaths<double, half>::scaleToA(double a)
//{
//    return (half)a;
//}

//template<>
//inline double KoColorSpaceMaths<half, double>::scaleToA(half a)
//{
//    return a;
//}

//template<>
//inline float KoColorSpaceMaths<half, float>::scaleToA(half a)
//{
//    return a;
//}

//template<>
//inline half KoColorSpaceMaths<float, half>::scaleToA(float a)
//{
//    return (half) a;
//}

//template<>
//inline quint8 KoColorSpaceMaths<half, quint8>::scaleToA(half a)
//{
//    half v = a * 255;
//    return (quint8)(CLAMP(v, 0, 255));
//}

//template<>
//inline half KoColorSpaceMaths<quint8, half>::scaleToA(quint8 a)
//{
//    return a *(1.0 / 255.0);
//}
//template<>
//inline quint16 KoColorSpaceMaths<half, quint16>::scaleToA(half a)
//{
//    double v = a * 0xFFFF;
//    return (quint16)(CLAMP(v, 0, 0xFFFF));
//}

//template<>
//inline half KoColorSpaceMaths<quint16, half>::scaleToA(quint16 a)
//{
//    return a *(1.0 / 0xFFFF);
//}

//template<>
//inline half KoColorSpaceMaths<half, half>::scaleToA(half a)
//{
//    return a;
//}

//template<>
//inline half KoColorSpaceMaths<half>::blend(half a, half b, half alpha)
//{
//    return (a - b) * alpha + b;
//}

//template<>
//inline double KoColorSpaceMaths<half>::clamp(double a)
//{
//    return a;
//}


//#endif

////------------------------------ quint8 specialization ------------------------------//

//template<>
//inline quint8 KoColorSpaceMaths<quint8>::multiply(quint8 a, quint8 b)
//{
//    return (quint8)UINT8_MULT(a, b);
//}


//template<>
//inline quint8 KoColorSpaceMaths<quint8>::multiply(quint8 a, quint8 b, quint8 c)
//{
//    return (quint8)UINT8_MULT3(a, b, c);
//}

//template<>
//inline KoColorSpaceMathsTraits<quint8>::compositetype
//KoColorSpaceMaths<quint8>::divide(quint8 a, quint8 b)
//{
//    return UINT8_DIVIDE(a, b);
//}

//template<>
//inline quint8 KoColorSpaceMaths<quint8>::invert(quint8 a)
//{
//    return ~a;
//}

//template<>
//inline quint8 KoColorSpaceMaths<quint8>::blend(quint8 a, quint8 b, quint8 c)
//{
//    return UINT8_BLEND(a, b, c);
//}

////------------------------------ quint16 specialization ------------------------------//

//template<>
//inline quint16 KoColorSpaceMaths<quint16>::multiply(quint16 a, quint16 b)
//{
//    return (quint16)UINT16_MULT(a, b);
//}

//template<>
//inline KoColorSpaceMathsTraits<quint16>::compositetype
//KoColorSpaceMaths<quint16>::divide(quint16 a, quint16 b)
//{
//    return UINT16_DIVIDE(a, b);
//}

//template<>
//inline quint16 KoColorSpaceMaths<quint16>::invert(quint16 a)
//{
//    return ~a;
//}

////------------------------------ various specialization ------------------------------//


//// TODO: use more functions from KoIntegersMaths to do the computation

///// This specialization is needed because the default implementation won't work when scaling up
//template<>
//inline quint16 KoColorSpaceMaths<quint8, quint16>::scaleToA(quint8 a)
//{
//    return UINT8_TO_UINT16(a);
//}

//template<>
//inline quint8 KoColorSpaceMaths<quint16, quint8>::scaleToA(quint16 a)
//{
//    return UINT16_TO_UINT8(a);
//}


//// Due to once again a bug in gcc, there is the need for those specialized functions:

//template<>
//inline quint8 KoColorSpaceMaths<quint8, quint8>::scaleToA(quint8 a)
//{
//    return a;
//}

//template<>
//inline quint16 KoColorSpaceMaths<quint16, quint16>::scaleToA(quint16 a)
//{
//    return a;
//}

//template<>
//inline float KoColorSpaceMaths<float, float>::scaleToA(float a)
//{
//    return a;
//}

//namespace Arithmetic
//{
//    const static qreal pi = 3.14159265358979323846;

//    template<class T>
//    inline T mul(T a, T b) { return KoColorSpaceMaths<T>::multiply(a, b); }

//    template<class T>
//    inline T mul(T a, T b, T c) { return KoColorSpaceMaths<T>::multiply(a, b, c); }

////     template<class T>
////     inline T mul(T a, T b) {
////         typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
////         return T(composite_type(a) * b / KoColorSpaceMathsTraits<T>::unitValue);
////     }
////
////     template<class T>
////     inline T mul(T a, T b, T c) {
////         typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
////         return T((composite_type(a) * b * c) / (composite_type(KoColorSpaceMathsTraits<T>::unitValue) * KoColorSpaceMathsTraits<T>::unitValue));
////     }

//    template<class T>
//    inline T inv(T a) { return KoColorSpaceMaths<T>::invert(a); }

//    template<class T>
//    inline T lerp(T a, T b, T alpha) { return KoColorSpaceMaths<T>::blend(b, a, alpha); }

//    template<class TRet, class T>
//    inline TRet scale(T a) { return KoColorSpaceMaths<T,TRet>::scaleToA(a); }

//    template<class T>
//    inline typename KoColorSpaceMathsTraits<T>::compositetype
//    div(T a, T b) { return KoColorSpaceMaths<T>::divide(a, b); }

//    template<class T>
//    inline T clamp(typename KoColorSpaceMathsTraits<T>::compositetype a) {
//        return KoColorSpaceMaths<T>::clamp(a);
//    }

//    template<class T>
//    inline T min(T a, T b, T c) {
//        b = (a < b) ? a : b;
//        return (b < c) ? b : c;
//    }

//    template<class T>
//    inline T max(T a, T b, T c) {
//        b = (a > b) ? a : b;
//        return (b > c) ? b : c;
//    }

//    template<class T>
//    inline T zeroValue() { return KoColorSpaceMathsTraits<T>::zeroValue; }

//    template<class T>
//    inline T halfValue() { return KoColorSpaceMathsTraits<T>::halfValue; }

//    template<class T>
//    inline T unitValue() { return KoColorSpaceMathsTraits<T>::unitValue; }

//    template<class T>
//    inline T unionShapeOpacity(T a, T b) {
//        typedef typename KoColorSpaceMathsTraits<T>::compositetype composite_type;
//        return T(composite_type(a) + b - mul(a,b));
//    }

//    template<class T>
//    inline T blend(T src, T srcAlpha, T dst, T dstAlpha, T cfValue) {
//        return mul(inv(srcAlpha), dstAlpha, dst) + mul(inv(dstAlpha), srcAlpha, src) + mul(dstAlpha, srcAlpha, cfValue);
//    }
//}

//struct HSYType
//{
//    template<class TReal>
//    inline static TReal getLightness(TReal r, TReal g, TReal b) {
//        return TReal(0.299)*r + TReal(0.587)*g + TReal(0.114)*b;
//    }

//    template<class TReal>
//    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
//        return Arithmetic::max(r,g,b) - Arithmetic::min(r,g,b);
//    }
//};

//struct HSIType
//{
//    template<class TReal>
//    inline static TReal getLightness(TReal r, TReal g, TReal b) {
//        return (r + g + b) * TReal(0.33333333333333333333); // (r + g + b) / 3.0
//    }

//    template<class TReal>
//    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
//        TReal max    = Arithmetic::max(r, g, b);
//        TReal min    = Arithmetic::min(r, g, b);
//        TReal chroma = max - min;

//        return (chroma > std::numeric_limits<TReal>::epsilon()) ?
//            (TReal(1.0) - min / getLightness(r, g, b)) : TReal(0.0);
//    }
//};

//struct HSLType
//{
//    template<class TReal>
//    inline static TReal getLightness(TReal r, TReal g, TReal b) {
//        TReal max = Arithmetic::max(r, g, b);
//        TReal min = Arithmetic::min(r, g, b);
//        return (max + min) * TReal(0.5);
//    }

//    template<class TReal>
//    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
//        TReal max    = Arithmetic::max(r, g, b);
//        TReal min    = Arithmetic::min(r, g, b);
//        TReal chroma = max - min;
//        TReal light  = (max + min) * TReal(0.5);
//        TReal div    = TReal(1.0) - std::abs(TReal(2.0)*light - TReal(1.0));

//        if(div > std::numeric_limits<TReal>::epsilon())
//            return chroma / div;

//        return TReal(1.0);
//    }
//};

//struct HSVType
//{
//    template<class TReal>
//    inline static TReal getLightness(TReal r, TReal g, TReal b) {
//        return Arithmetic::max(r,g,b);
//    }

//    template<class TReal>
//    inline static TReal getSaturation(TReal r, TReal g, TReal b) {
//        TReal max = Arithmetic::max(r, g, b);
//        TReal min = Arithmetic::min(r, g, b);
//        return (max == TReal(0.0)) ? TReal(0.0) : (max - min) / max;
//    }
//};

//template<class TReal>
//TReal getHue(TReal r, TReal g, TReal b) {
//    TReal min    = Arithmetic::min(r, g, b);
//    TReal max    = Arithmetic::max(r, g, b);
//    TReal chroma = max - min;

//    TReal hue = TReal(-1.0);

//    if(chroma > std::numeric_limits<TReal>::epsilon()) {

////         return atan2(TReal(2.0)*r - g - b, TReal(1.73205080756887729353)*(g - b));

//        if(max == r) // between yellow and magenta
//            hue = (g - b) / chroma;
//        else if(max == g) // between cyan and yellow
//            hue = TReal(2.0) + (b - r) / chroma;
//        else if(max == b) // between magenta and cyan
//            hue = TReal(4.0) + (r - g) / chroma;

//        if(hue < -std::numeric_limits<TReal>::epsilon())
//            hue += TReal(6.0);

//        hue /= TReal(6.0);
//    }

////     hue = (r == max) ? (b-g) : (g == max) ? TReal(2.0)+(r-b) : TReal(4.0)+(g-r);

//    return hue;
//}

//template<class TReal>
//void getRGB(TReal& r, TReal& g, TReal& b, TReal hue) {
//    // 0 red    -> (1,0,0)
//    // 1 yellow -> (1,1,0)
//    // 2 green  -> (0,1,0)
//    // 3 cyan   -> (0,1,1)
//    // 4 blue   -> (0,0,1)
//    // 5 maenta -> (1,0,1)
//    // 6 red    -> (1,0,0)

//    if(hue < -std::numeric_limits<TReal>::epsilon()) {
//        r = g = b = TReal(0.0);
//        return;
//    }

//    int   i = int(hue * TReal(6.0));
//    TReal x = hue * TReal(6.0) - i;
//    TReal y = TReal(1.0) - x;

//    switch(i % 6){
//        case 0: { r=TReal(1.0), g=x         , b=TReal(0.0); } break;
//        case 1: { r=y         , g=TReal(1.0), b=TReal(0.0); } break;
//        case 2: { r=TReal(0.0), g=TReal(1.0), b=x         ; } break;
//        case 3: { r=TReal(0.0), g=y         , b=TReal(1.0); } break;
//        case 4: { r=x         , g=TReal(0.0), b=TReal(1.0); } break;
//        case 5: { r=TReal(1.0), g=TReal(0.0), b=y         ; } break;
//    }
//}

//template<class HSXType, class TReal>
//inline static TReal getLightness(TReal r, TReal g, TReal b) {
//    return HSXType::getLightness(r, g, b);
//}

//template<class HSXType, class TReal>
//inline void addLightness(TReal& r, TReal& g, TReal& b, TReal light)
//{
//    using namespace Arithmetic;

//    r += light;
//    g += light;
//    b += light;

//    TReal l = HSXType::getLightness(r, g, b);
//    TReal n = min(r, g, b);
//    TReal x = max(r, g, b);

//    if(n < TReal(0.0)) {
//        TReal iln = TReal(1.0) / (l-n);
//        r = l + ((r-l) * l) * iln;
//        g = l + ((g-l) * l) * iln;
//        b = l + ((b-l) * l) * iln;
//    }

//    if(x > TReal(1.0) && (x-l) > std::numeric_limits<TReal>::epsilon()) {
//        TReal il  = TReal(1.0) - l;
//        TReal ixl = TReal(1.0) / (x - l);
//        r = l + ((r-l) * il) * ixl;
//        g = l + ((g-l) * il) * ixl;
//        b = l + ((b-l) * il) * ixl;
//    }
//}

//template<class HSXType, class TReal>
//inline void setLightness(TReal& r, TReal& g, TReal& b, TReal light)
//{
//    addLightness<HSXType>(r,g,b, light - HSXType::getLightness(r,g,b));
//}

//template<class HSXType, class TReal>
//inline static TReal getSaturation(TReal r, TReal g, TReal b) {
//    return HSXType::getSaturation(r, g, b);
//}

//template<class HSXType, class TReal>
//inline void setSaturation(TReal& r, TReal& g, TReal& b, TReal sat)
//{
//    int   min    = 0;
//    int   mid    = 1;
//    int   max    = 2;
//    TReal rgb[3] = {r, g, b};

//    if(rgb[mid] < rgb[min]) {
//        int tmp = min;
//        min = mid;
//        mid = tmp;
//    }

//    if(rgb[max] < rgb[mid]) {
//        int tmp = mid;
//        mid = max;
//        max = tmp;
//    }

//    if(rgb[mid] < rgb[min]) {
//        int tmp = min;
//        min = mid;
//        mid = tmp;
//    }

//    if((rgb[max] - rgb[min]) > TReal(0.0)) {
//        rgb[mid] = ((rgb[mid]-rgb[min]) * sat) / (rgb[max]-rgb[min]);
//        rgb[max] = sat;
//        rgb[min] = TReal(0.0);

//        r = rgb[0];
//        g = rgb[1];
//        b = rgb[2];
//    }
//    else r = g = b = TReal(0.0);
//}

#endif
