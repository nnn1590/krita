/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_BGR_COLORSPACE_TRAITS_H_
#define _KO_BGR_COLORSPACE_TRAITS_H_

/** 
 * Base class for bgr traits, it provides some convenient functions to
 * access BGR channels through an explicit API.
 */
template<typename _channels_type_>
struct KoBgrTraits : public KoColorSpaceTrait<_channels_type_, 4, 3> {
    typedef _channels_type_ channels_type;
    typedef KoColorSpaceTrait<_channels_type_, 4, 3> parent;
    static const qint32 red_pos = 2;
    static const qint32 green_pos = 1;
    static const qint32 blue_pos = 0;
    /**
     * An BGR pixel
     */
    struct Pixel {
        channels_type blue;
        channels_type green;
        channels_type red;
        channels_type alpha;
    };

    /// @return the red component
    inline static channels_type red(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[red_pos];
    }
    /// Set the red component
    inline static void setRed(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[red_pos] = nv;
    }
    /// @return the green component
    inline static channels_type green(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[green_pos];
    }
    /// Set the green component
    inline static void setGreen(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[green_pos] = nv;
    }
    /// @return the blue component
    inline static channels_type blue(quint8* data) {
        channels_type* d = parent::nativeArray(data);
        return d[blue_pos];
    }
    /// Set the blue component
    inline static void setBlue(quint8* data, channels_type nv) {
        channels_type* d = parent::nativeArray(data);
        d[blue_pos] = nv;
    }
};


struct KoBgrU8Traits : public KoBgrTraits<quint8> {
};

struct KoBgrU16Traits : public KoBgrTraits<quint16> {
};

struct KoBgrU32Traits : public KoBgrTraits<quint32> {
};

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>

struct KoBgrF16Traits : public KoBgrTraits<half> {
};

#endif

struct KoBgrF32Traits : public KoBgrTraits<float> {
};

struct KoBgrF64Traits : public KoBgrTraits<double> {
};


#endif
