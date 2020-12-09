/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisGLImageF16.h"

#include <QByteArray>
#include <QSize>

struct KisGLImageF16::Private : public QSharedData
{
    QSize size;
    QByteArray data;
};



KisGLImageF16::KisGLImageF16()
    : m_d(new Private)
{
}

KisGLImageF16::KisGLImageF16(const QSize &size, bool clearPixels)
    : m_d(new Private)
{
    resize(size, clearPixels);
}

KisGLImageF16::KisGLImageF16(int width, int height, bool clearPixels)
    : KisGLImageF16(QSize(width, height), clearPixels)
{
}

KisGLImageF16::KisGLImageF16(const KisGLImageF16 &rhs)
    : m_d(rhs.m_d)
{
}

KisGLImageF16 &KisGLImageF16::operator=(const KisGLImageF16 &rhs)
{
    m_d = rhs.m_d;
    return *this;
}

bool operator==(const KisGLImageF16 &lhs, const KisGLImageF16 &rhs)
{
    return lhs.m_d == rhs.m_d;
}

KisGLImageF16::~KisGLImageF16()
{
}

void KisGLImageF16::clearPixels()
{
    if (!m_d->data.isEmpty()) {
        m_d->data.fill(0);
    }
}

void KisGLImageF16::resize(const QSize &size, bool clearPixels)
{
    const int pixelSize = 2 * 4;

    m_d->size = size;
    m_d->data.resize(size.width() * size.height() * pixelSize);

    if (clearPixels) {
        m_d->data.fill(0);
    }
}

const half *KisGLImageF16::constData() const
{
    Q_ASSERT(!m_d->data.isNull());
    return reinterpret_cast<const half*>(m_d->data.data());
}

half *KisGLImageF16::data()
{
    m_d->data.detach();
    Q_ASSERT(!m_d->data.isNull());

    return reinterpret_cast<half*>(m_d->data.data());
}

QSize KisGLImageF16::size() const
{
    return m_d->size;
}

int KisGLImageF16::width() const
{
    return m_d->size.width();
}

int KisGLImageF16::height() const
{
    return m_d->size.height();
}

bool KisGLImageF16::isNull() const
{
    return m_d->data.isNull();
}
