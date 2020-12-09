/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_UNIFORM_PAINT_OP_PROPERTY_H
#define __KIS_UNIFORM_PAINT_OP_PROPERTY_H

#include <QScopedPointer>
#include <QObject>

#include "kritaimage_export.h"
#include "kis_types.h"


class KRITAIMAGE_EXPORT KisUniformPaintOpProperty : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Int = 0,
        Double,
        Bool,
        Combo
    };

public:
    KisUniformPaintOpProperty(Type type,
                              const QString &id,
                              const QString &name,
                              KisPaintOpSettingsRestrictedSP settings,
                              QObject *parent);
    KisUniformPaintOpProperty(const QString &id,
                              const QString &name,
                              KisPaintOpSettingsRestrictedSP settings,
                              QObject *parent);
    ~KisUniformPaintOpProperty() override;

    QString id() const;
    QString name() const;
    Type type() const;

    QVariant value() const;

    QWidget *createPropertyWidget();

    KisPaintOpSettingsSP settings() const;

    virtual bool isVisible() const;

public Q_SLOTS:
    void setValue(const QVariant &value);
    void requestReadValue();

Q_SIGNALS:
    void valueChanged(const QVariant &value);

protected:
    virtual void readValueImpl();
    virtual void writeValueImpl();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

template<class T> class QSharedPointer;
template<class T> class QWeakPointer;
template<class T> class QList;

typedef QSharedPointer<KisUniformPaintOpProperty> KisUniformPaintOpPropertySP;
typedef QWeakPointer<KisUniformPaintOpProperty> KisUniformPaintOpPropertyWSP;

#include "kis_callback_based_paintop_property.h"
extern template class KRITAIMAGE_EXPORT KisCallbackBasedPaintopProperty<KisUniformPaintOpProperty>;
typedef KisCallbackBasedPaintopProperty<KisUniformPaintOpProperty> KisUniformPaintOpPropertyCallback;

#endif /* __KIS_UNIFORM_PAINT_OP_PROPERTY_H */
