/*
 *  Copyright (c) 2013 Sahil Nagpal <nagpal.sahil01@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KIS_BURN_HIGHLIGHTS_ADJUSTMENT_H_
#define _KIS_BURN_HIGHLIGHTS_ADJUSTMENT_H_

#include "KoColorTransformationFactory.h"

class KisBurnHighlightsAdjustmentFactory : public KoColorTransformationFactory
{
public:

    KisBurnHighlightsAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;

};

#endif
