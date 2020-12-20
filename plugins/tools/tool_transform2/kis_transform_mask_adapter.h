/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TRANSFORM_MASK_ADAPTER_H
#define __KIS_TRANSFORM_MASK_ADAPTER_H

#include <QScopedPointer>
#include "kis_transform_mask_params_interface.h"
#include "kritatooltransform_export.h"

class ToolTransformArgs;


class KRITATOOLTRANSFORM_EXPORT KisTransformMaskAdapter : public KisTransformMaskParamsInterface
{
public:
    KisTransformMaskAdapter(const ToolTransformArgs &args);
    ~KisTransformMaskAdapter() override;

    QTransform finalAffineTransform() const override;
    bool isAffine() const override;
    bool isHidden() const override;

    void transformDevice(KisNodeSP node, KisPaintDeviceSP src, KisPaintDeviceSP dst) const override;

    virtual const ToolTransformArgs& transformArgs() const;

    QString id() const override;
    void toXML(QDomElement *e) const override;
    static KisTransformMaskParamsInterfaceSP fromXML(const QDomElement &e);

    void translate(const QPointF &offset) override;

    QRect nonAffineChangeRect(const QRect &rc) override;
    QRect nonAffineNeedRect(const QRect &rc, const QRect &srcBounds) override;

    bool isAnimated() const;
    KisKeyframeChannel *getKeyframeChannel(const QString &id, KisDefaultBoundsBaseSP defaultBounds);
    void clearChangedFlag() override;
    bool hasChanged() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_TRANSFORM_MASK_ADAPTER_H */
