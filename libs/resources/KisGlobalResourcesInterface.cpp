/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGlobalResourcesInterface.h"

#include <QGlobalStatic>
#include <KisResourceModel.h>

#include <kis_debug.h>

#include <QBasicMutex>


namespace {
class GlobalResourcesSource : public KisResourcesInterface::ResourceSourceAdapter
{
public:
    GlobalResourcesSource(KisResourceModel *model)
        : m_model(model)
    {}

    ~GlobalResourcesSource() override
    {
        delete m_model;
    }


    KoResourceSP resourceForFilename(const QString& filename) const override {
        return m_model->resourceForFilename(filename);
    }
    KoResourceSP resourceForName(const QString& name) const override {
        return m_model->resourceForName(name);
    }
    KoResourceSP resourceForMD5(const QByteArray& md5) const override {
        return m_model->resourceForMD5(md5);
    }
    KoResourceSP fallbackResource() const override {
        return m_model->rowCount() > 0 ? m_model->resourceForIndex(m_model->index(0, 0)) : KoResourceSP();
    }

private:
    KisResourceModel *m_model;
};
}

KisResourcesInterfaceSP KisGlobalResourcesInterface::instance()
{
    // TODO: implement general macro like Q_GLOBAL_STATIC()

    static QBasicAtomicInt guard = Q_BASIC_ATOMIC_INITIALIZER(QtGlobalStatic::Uninitialized);
    static KisResourcesInterfaceSP d;
    static QBasicMutex mutex;
    int x = guard.loadAcquire();
    if (Q_UNLIKELY(x >= QtGlobalStatic::Uninitialized)) {
        QMutexLocker locker(&mutex);
        if (guard.load() == QtGlobalStatic::Uninitialized) {
            d.reset(new KisGlobalResourcesInterface());
            static struct Cleanup {
                ~Cleanup() {
                    d.reset();
                    guard.store(QtGlobalStatic::Destroyed);
                }
            } cleanup;
            guard.storeRelease(QtGlobalStatic::Initialized);
        }
    }

    return d;
}

KisResourcesInterface::ResourceSourceAdapter *KisGlobalResourcesInterface::createSourceImpl(const QString &type) const
{
    KisResourcesInterface::ResourceSourceAdapter *source =
        new GlobalResourcesSource(new KisResourceModel(type));

    KIS_ASSERT(source);
    return source;
}
