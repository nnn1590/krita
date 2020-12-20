/*
 *  SPDX-FileCopyrightText: 2006, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2007 Emanuele Tamponi <emanuele@valinor.it>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KO_COMPOSITEOP_COPY2_H
#define KO_COMPOSITEOP_COPY2_H

#include "KoCompositeOpBase.h"

/**
 * Generic implementation of the COPY composite op which respects selection.
 *
 * Note: this composite op is necessary with the deform brush and should not
 * be hidden.
 */
template<class Traits>
class KoCompositeOpCopy2: public KoCompositeOpBase< Traits, KoCompositeOpCopy2<Traits> >
{
    typedef KoCompositeOpBase< Traits, KoCompositeOpCopy2<Traits> > base_class;
    typedef typename Traits::channels_type                          channels_type;

    static const qint32 channels_nb = Traits::channels_nb;
    static const qint32 alpha_pos   = Traits::alpha_pos;

public:
    KoCompositeOpCopy2(const KoColorSpace* cs)
        : base_class(cs, COMPOSITE_COPY, i18n("Copy"), KoCompositeOp::categoryMisc()) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha, channels_type maskAlpha,
                                                     channels_type opacity, const QBitArray& channelFlags) {
        using namespace Arithmetic;
        opacity = mul(maskAlpha, opacity);

        channels_type newAlpha = zeroValue<channels_type>();

        if (opacity == unitValue<channels_type>()) {
            if (!alphaLocked || srcAlpha != zeroValue<channels_type>()) {
                // don't blend if the color of the destination is undefined (has zero opacity)
                // copy the source channel instead
                for(qint32 i=0; i<channels_nb; ++i)
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i)))
                        dst[i] = src[i];
            }

            newAlpha = srcAlpha;

        } else if (opacity == zeroValue<channels_type>()) {

            newAlpha = dstAlpha;

        } else { // opacity 0...1

            if (!alphaLocked || srcAlpha != zeroValue<channels_type>()) {

                newAlpha = lerp(dstAlpha, srcAlpha, opacity);

                if (newAlpha == zeroValue<channels_type>()) {
                    return newAlpha;
                }

                // blend the color channels
                for(qint32 i=0; i<channels_nb; ++i) {
                    if(i != alpha_pos && (allChannelFlags || channelFlags.testBit(i))) {

                        // We use the most fundamental OVER algorithm here,
                        // which miltiplies, blends and then unmultiplies the
                        // channels

                        typedef typename KoColorSpaceMathsTraits<channels_type>::compositetype composite_type;

                        channels_type dstMult = mul(dst[i], dstAlpha);
                        channels_type srcMult = mul(src[i], srcAlpha);
                        channels_type blendedValue = lerp(dstMult, srcMult, opacity);

                        composite_type normedValue = KoColorSpaceMaths<channels_type>::divide(blendedValue, newAlpha);

                        dst[i] = KoColorSpaceMaths<channels_type>::clampAfterScale(normedValue);
                    }
                }
            }

        }

        return newAlpha;
    }
};

#endif // KO_COMPOSITEOP_COPY2_H
