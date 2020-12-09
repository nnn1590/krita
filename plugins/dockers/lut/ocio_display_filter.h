/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef OCIO_DISPLAY_FILTER_H
#define OCIO_DISPLAY_FILTER_H

#include <kis_display_filter.h>
#include <OpenColorIO/OpenColorIO.h>
#include <OpenColorIO/OpenColorTransforms.h>
#include <QVector>
#include "kis_exposure_gamma_correction_interface.h"

namespace OCIO = OCIO_NAMESPACE;

enum OCIO_CHANNEL_SWIZZLE {
    LUMINANCE,
    RGBA,
    R,
    G,
    B,
    A
};

class OcioDisplayFilter : public KisDisplayFilter
{
    Q_OBJECT
public:
    explicit OcioDisplayFilter(KisExposureGammaCorrectionInterface *interface, QObject *parent = 0);
    ~OcioDisplayFilter();

    void filter(quint8 *pixels, quint32 numPixels);
    void approximateInverseTransformation(quint8 *pixels, quint32 numPixels);
    void approximateForwardTransformation(quint8 *pixels, quint32 numPixels);
    bool useInternalColorManagement() const;
    bool lockCurrentColorVisualRepresentation() const;
    void setLockCurrentColorVisualRepresentation(bool value);

    bool updateShader();
    template <class F>
    bool updateShaderImpl(F *f);

    KisExposureGammaCorrectionInterface *correctionInterface() const;

    virtual QString program() const;
    GLuint lutTexture() const;

    void updateProcessor();

    OCIO::ConstConfigRcPtr config;

    const char *inputColorSpaceName;
    const char *displayDevice;
    const char *view;
    const char *look;
    OCIO_CHANNEL_SWIZZLE swizzle;
    float exposure;
    float gamma;
    float blackPoint;
    float whitePoint;
    bool forceInternalColorManagement;

private:

    OCIO::ConstProcessorRcPtr m_processor;
    OCIO::ConstProcessorRcPtr m_revereseApproximationProcessor;
    OCIO::ConstProcessorRcPtr m_forwardApproximationProcessor;

    KisExposureGammaCorrectionInterface *m_interface;

    bool m_lockCurrentColorVisualRepresentation;

    QString m_program;
    GLuint m_lut3dTexID;
    QVector<float> m_lut3d;
    QString m_lut3dcacheid;
    QString m_shadercacheid;

    bool m_shaderDirty;
};

#endif // OCIO_DISPLAY_FILTER_H
