/*
 *  Copyright (c) 2017 Alvin Wong <alvinhochun@gmail.com>
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisOpenGLModeProber.h"

#include <config-hdr.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QWindow>

#include <QGlobalStatic>
Q_GLOBAL_STATIC(KisOpenGLModeProber, s_instance)


KisOpenGLModeProber::KisOpenGLModeProber()
{
}

KisOpenGLModeProber::~KisOpenGLModeProber()
{

}

KisOpenGLModeProber *KisOpenGLModeProber::instance()
{
    return s_instance;
}

bool KisOpenGLModeProber::useHDRMode() const
{
    return isFormatHDR(QSurfaceFormat::defaultFormat());
}

QSurfaceFormat KisOpenGLModeProber::surfaceformatInUse() const
{
    // TODO: use information provided by KisOpenGL instead
    QOpenGLContext *sharedContext = QOpenGLContext::globalShareContext();
    QSurfaceFormat format = sharedContext ? sharedContext->format() : QSurfaceFormat::defaultFormat();
    return format;
}

const KoColorProfile *KisOpenGLModeProber::rootSurfaceColorProfile() const
{
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)

    const KisSurfaceColorSpace surfaceColorSpace = surfaceformatInUse().colorSpace();
    if (surfaceColorSpace == KisSurfaceColorSpace::sRGBColorSpace) {
        // use the default one!
#ifdef HAVE_HDR
    } else if (surfaceColorSpace == KisSurfaceColorSpace::scRGBColorSpace) {
        profile = KoColorSpaceRegistry::instance()->p709G10Profile();
    } else if (surfaceColorSpace == KisSurfaceColorSpace::bt2020PQColorSpace) {
        profile = KoColorSpaceRegistry::instance()->p2020PQProfile();
#endif
    }

#endif

    return profile;
}

namespace {
struct AppAttributeSetter
{
    AppAttributeSetter(Qt::ApplicationAttribute attribute, bool useOpenGLES)
        : m_attribute(attribute),
          m_oldValue(QCoreApplication::testAttribute(attribute))
    {
        QCoreApplication::setAttribute(attribute, useOpenGLES);
    }

    ~AppAttributeSetter() {
        QCoreApplication::setAttribute(m_attribute, m_oldValue);
    }

private:
    Qt::ApplicationAttribute m_attribute;
    bool m_oldValue = false;
};

struct SurfaceFormatSetter
{
    SurfaceFormatSetter(const QSurfaceFormat &format)
        : m_oldFormat(QSurfaceFormat::defaultFormat())
    {
        QSurfaceFormat::setDefaultFormat(format);
    }

    ~SurfaceFormatSetter() {
        QSurfaceFormat::setDefaultFormat(m_oldFormat);
    }

private:
    QSurfaceFormat m_oldFormat;
};


#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
QString qEnvironmentVariable(const char *varName) {
    return qgetenv(varName);
}
#endif

struct EnvironmentSetter
{
    EnvironmentSetter(const QLatin1String &env, const QString &value)
        : m_env(env)
    {
        if (qEnvironmentVariableIsEmpty(m_env.latin1())) {
            m_oldValue = qgetenv(env.latin1());
        }
        if (!value.isEmpty()) {
            qputenv(env.latin1(), value.toLatin1());
        } else {
            qunsetenv(env.latin1());
        }
    }

    ~EnvironmentSetter() {
        if (m_oldValue) {
            qputenv(m_env.latin1(), (*m_oldValue).toLatin1());
        } else {
            qunsetenv(m_env.latin1());
        }
    }

private:
    const QLatin1String m_env;
    boost::optional<QString> m_oldValue;
};

}

boost::optional<KisOpenGLModeProber::Result>
KisOpenGLModeProber::probeFormat(const KisOpenGL::RendererConfig &rendererConfig,
                                 bool adjustGlobalState)
{
    const QSurfaceFormat &format = rendererConfig.format;

    QScopedPointer<AppAttributeSetter> sharedContextSetter;
    QScopedPointer<AppAttributeSetter> glSetter;
    QScopedPointer<AppAttributeSetter> glesSetter;
    QScopedPointer<SurfaceFormatSetter> formatSetter;
    QScopedPointer<EnvironmentSetter> rendererSetter;
    QScopedPointer<QGuiApplication> application;

    int argc = 1;
    QByteArray probeAppName("krita");
    char *argv = probeAppName.data();



    if (adjustGlobalState) {
        sharedContextSetter.reset(new AppAttributeSetter(Qt::AA_ShareOpenGLContexts, false));

        if (format.renderableType() != QSurfaceFormat::DefaultRenderableType) {
            glSetter.reset(new AppAttributeSetter(Qt::AA_UseDesktopOpenGL, format.renderableType() != QSurfaceFormat::OpenGLES));
            glesSetter.reset(new AppAttributeSetter(Qt::AA_UseOpenGLES, format.renderableType() == QSurfaceFormat::OpenGLES));
        }

        rendererSetter.reset(new EnvironmentSetter(QLatin1String("QT_ANGLE_PLATFORM"), angleRendererToString(rendererConfig.angleRenderer)));
        formatSetter.reset(new SurfaceFormatSetter(format));

        QGuiApplication::setDesktopSettingsAware(false);
        application.reset(new QGuiApplication(argc, &argv));
        QGuiApplication::setDesktopSettingsAware(true);
    }

    QWindow surface;
    surface.setFormat(format);
    surface.setSurfaceType(QSurface::OpenGLSurface);
    surface.create();
    QOpenGLContext context;
    context.setFormat(format);


    if (!context.create()) {
        dbgOpenGL << "OpenGL context cannot be created";
        return boost::none;
    }
    if (!context.isValid()) {
        dbgOpenGL << "OpenGL context is not valid while checking Qt's OpenGL status";
        return boost::none;
    }
    if (!context.makeCurrent(&surface)) {
        dbgOpenGL << "OpenGL context cannot be made current";
        return boost::none;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    if (!fuzzyCompareColorSpaces(context.format().colorSpace(), format.colorSpace())) {
        dbgOpenGL << "Failed to create an OpenGL context with requested color space. Requested:" << format.colorSpace() << "Actual:" << context.format().colorSpace();
        return boost::none;
    }
#endif

    return Result(context);
}

bool KisOpenGLModeProber::fuzzyCompareColorSpaces(const KisSurfaceColorSpace &lhs, const KisSurfaceColorSpace &rhs)
{
    return lhs == rhs ||
        ((lhs == KisSurfaceColorSpace::DefaultColorSpace ||
          lhs == KisSurfaceColorSpace::sRGBColorSpace) &&
         (rhs == KisSurfaceColorSpace::DefaultColorSpace ||
          rhs == KisSurfaceColorSpace::sRGBColorSpace));
}

void KisOpenGLModeProber::initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                                      QSurfaceFormat *format)
{
#ifdef HAVE_HDR
    if (config == KisConfig::BT2020_PQ) {

        format->setRedBufferSize(10);
        format->setGreenBufferSize(10);
        format->setBlueBufferSize(10);
        format->setAlphaBufferSize(2);
        format->setColorSpace(KisSurfaceColorSpace::bt2020PQColorSpace);
    } else if (config == KisConfig::BT709_G10) {
        format->setRedBufferSize(16);
        format->setGreenBufferSize(16);
        format->setBlueBufferSize(16);
        format->setAlphaBufferSize(16);
        format->setColorSpace(KisSurfaceColorSpace::scRGBColorSpace);
    } else
#else
    if (config == KisConfig::BT2020_PQ) {
        qWarning() << "WARNING: Bt.2020 PQ surface type is not supported by this build of Krita";
    } else if (config == KisConfig::BT709_G10) {
        qWarning() << "WARNING: scRGB surface type is not supported by this build of Krita";
    }
#endif

    {
        format->setRedBufferSize(8);
        format->setGreenBufferSize(8);
        format->setBlueBufferSize(8);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        format->setAlphaBufferSize(8);
#else
        format->setAlphaBufferSize(0);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        // TODO: check if we can use real sRGB space here
        format->setColorSpace(KisSurfaceColorSpace::DefaultColorSpace);
#endif
    }
}

bool KisOpenGLModeProber::isFormatHDR(const QSurfaceFormat &format)
{
#ifdef HAVE_HDR

    bool isBt2020PQ =
        format.colorSpace() == KisSurfaceColorSpace::bt2020PQColorSpace &&
        format.redBufferSize() == 10 &&
        format.greenBufferSize() == 10 &&
        format.blueBufferSize() == 10 &&
        format.alphaBufferSize() == 2;

    bool isBt709G10 =
        format.colorSpace() == KisSurfaceColorSpace::scRGBColorSpace &&
        format.redBufferSize() == 16 &&
        format.greenBufferSize() == 16 &&
        format.blueBufferSize() == 16 &&
        format.alphaBufferSize() == 16;

    return isBt2020PQ || isBt709G10;
#else
    Q_UNUSED(format);
    return false;
#endif
}

QString KisOpenGLModeProber::angleRendererToString(KisOpenGL::AngleRenderer renderer)
{
    QString value;

    switch (renderer) {
    case KisOpenGL::AngleRendererDefault:
        break;
    case KisOpenGL::AngleRendererD3d9:
        value = "d3d9";
        break;
    case KisOpenGL::AngleRendererD3d11:
        value = "d3d11";
        break;
    case KisOpenGL::AngleRendererD3d11Warp:
        value = "warp";
        break;
    };

    return value;
}

KisOpenGLModeProber::Result::Result(QOpenGLContext &context) {
    if (!context.isValid()) {
        return;
    }

    QOpenGLFunctions *funcs = context.functions(); // funcs is ready to be used

    m_rendererString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER)));
    m_driverVersionString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION)));
    m_vendorString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VENDOR)));
    m_shadingLanguageString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_SHADING_LANGUAGE_VERSION)));
    m_glMajorVersion = context.format().majorVersion();
    m_glMinorVersion = context.format().minorVersion();
    m_supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);
    m_isOpenGLES = context.isOpenGLES();
    m_format = context.format();
    m_supportsFBO = context.functions()->hasOpenGLFeature(QOpenGLFunctions::Framebuffers);
}
