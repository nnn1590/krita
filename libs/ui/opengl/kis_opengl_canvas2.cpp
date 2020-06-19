/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006-2013
 * Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
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

#define GL_GLEXT_PROTOTYPES

#include "opengl/kis_opengl_canvas2.h"
#include "opengl/kis_opengl_canvas2_p.h"

#include "kis_algebra_2d.h"
#include "opengl/kis_opengl_shader_loader.h"
#include "opengl/kis_opengl_canvas_debugger.h"
#include "canvas/kis_canvas2.h"
#include "canvas/kis_coordinates_converter.h"
#include "canvas/kis_display_filter.h"
#include "canvas/kis_display_color_converter.h"
#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_debug.h"

#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QPointer>
#include <QMatrix>
#include <QTransform>
#include <QThread>
#include <QFile>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QMessageBox>
#include "KisOpenGLModeProber.h"
#include <KoColorModelStandardIds.h>

#if !defined(Q_OS_MACOS) && !defined(HAS_ONLY_OPENGL_ES)
#include <QOpenGLFunctions_2_1>
#endif

#define NEAR_VAL -1000.0
#define FAR_VAL 1000.0

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

static bool OPENGL_SUCCESS = false;

struct KisOpenGLCanvas2::Private
{
public:
    ~Private() {
        delete displayShader;
        delete checkerShader;
        delete solidColorShader;
        delete overlayInvertedShader;
        Sync::deleteSync(glSyncObject);
    }

    bool canvasInitialized{false};

    KisOpenGLImageTexturesSP openGLImageTextures;

    KisOpenGLShaderLoader shaderLoader;
    KisShaderProgram *displayShader{0};
    KisShaderProgram *checkerShader{0};
    KisShaderProgram *solidColorShader{0};
    KisShaderProgram *overlayInvertedShader{0};

    QScopedPointer<QOpenGLFramebufferObject> canvasFBO;

    bool displayShaderCompiledWithDisplayFilterSupport{false};

    GLfloat checkSizeScale;
    bool scrollCheckers;

    QSharedPointer<KisDisplayFilter> displayFilter;
    KisOpenGL::FilterMode filterMode;
    bool proofingConfigIsUpdated=false;

    GLsync glSyncObject{0};

    bool wrapAroundMode{false};

    // Stores a quad for drawing the canvas
    QOpenGLVertexArrayObject quadVAO;
    QOpenGLBuffer quadBuffers[2];

    // Stores data for drawing tool outlines
    QOpenGLVertexArrayObject outlineVAO;
    QOpenGLBuffer lineVertexBuffer;
    QOpenGLBuffer lineTexCoordBuffer;

    QVector3D vertices[6];
    QVector2D texCoords[6];

#if !defined(Q_OS_MACOS) && !defined(HAS_ONLY_OPENGL_ES)
    QOpenGLFunctions_2_1 *glFn201;
#endif

    qreal pixelGridDrawingThreshold;
    bool pixelGridEnabled;
    QColor gridColor;
    QColor cursorColor;

    bool lodSwitchInProgress = false;

    int xToColWithWrapCompensation(int x, const QRect &imageRect) {
        int firstImageColumn = openGLImageTextures->xToCol(imageRect.left());
        int lastImageColumn = openGLImageTextures->xToCol(imageRect.right());

        int colsPerImage = lastImageColumn - firstImageColumn + 1;
        int numWraps = floor(qreal(x) / imageRect.width());
        int remainder = x - imageRect.width() * numWraps;

        return colsPerImage * numWraps + openGLImageTextures->xToCol(remainder);
    }

    int yToRowWithWrapCompensation(int y, const QRect &imageRect) {
        int firstImageRow = openGLImageTextures->yToRow(imageRect.top());
        int lastImageRow = openGLImageTextures->yToRow(imageRect.bottom());

        int rowsPerImage = lastImageRow - firstImageRow + 1;
        int numWraps = floor(qreal(y) / imageRect.height());
        int remainder = y - imageRect.height() * numWraps;

        return rowsPerImage * numWraps + openGLImageTextures->yToRow(remainder);
    }

};

KisOpenGLCanvas2::KisOpenGLCanvas2(KisCanvas2 *canvas,
                                   KisCoordinatesConverter *coordinatesConverter,
                                   QWidget *parent,
                                   KisImageWSP image,
                                   KisDisplayColorConverter *colorConverter)
    : QOpenGLWidget(parent)
    , KisCanvasWidgetBase(canvas, coordinatesConverter)
    , d(new Private())
{
    KisConfig cfg(false);
    cfg.setCanvasState("OPENGL_STARTED");

    d->openGLImageTextures =
            KisOpenGLImageTextures::getImageTextures(image,
                                                     colorConverter->openGLCanvasSurfaceProfile(),
                                                     colorConverter->renderingIntent(),
                                                     colorConverter->conversionFlags());

    connect(d->openGLImageTextures.data(),
            SIGNAL(sigShowFloatingMessage(QString, int, bool)),
            SLOT(slotShowFloatingMessage(QString, int, bool)));

    setAcceptDrops(true);
    setAutoFillBackground(false);

    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground, true);
#ifdef Q_OS_MACOS
    setAttribute(Qt::WA_AcceptTouchEvents, false);
#else
    setAttribute(Qt::WA_AcceptTouchEvents, true);
#endif
    setAttribute(Qt::WA_InputMethodEnabled, false);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    // we should make sure the texture doesn't have alpha channel,
    // otherwise blending will not work correctly.
    if (KisOpenGLModeProber::instance()->useHDRMode()) {
        setTextureFormat(GL_RGBA16F);
    } else {
        /**
         * When in pure OpenGL mode, the canvas surface will have alpha
         * channel. Therefore, if our canvas blending algorithm produces
         * semi-transparent pixels (and it does), then Krita window itself
         * will become transparent. Which is not good.
         *
         * In Angle mode, GL_RGB8 is not available (and the transparence effect
         * doesn't exist at all).
         */
        if (!KisOpenGL::hasOpenGLES()) {
            setTextureFormat(GL_RGB8);
        }
    }
#endif

    setDisplayFilterImpl(colorConverter->displayFilter(), true);

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    connect(KisConfigNotifier::instance(), SIGNAL(pixelGridModeChanged()), SLOT(slotPixelGridModeChanged()));
    slotConfigChanged();
    slotPixelGridModeChanged();
    cfg.writeEntry("canvasState", "OPENGL_SUCCESS");
}

KisOpenGLCanvas2::~KisOpenGLCanvas2()
{
    delete d;
}

void KisOpenGLCanvas2::setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter)
{
    setDisplayFilterImpl(displayFilter, false);
}

void KisOpenGLCanvas2::setDisplayFilterImpl(QSharedPointer<KisDisplayFilter> displayFilter, bool initializing)
{
    bool needsInternalColorManagement =
            !displayFilter || displayFilter->useInternalColorManagement();

    bool needsFullRefresh = d->openGLImageTextures->setInternalColorManagementActive(needsInternalColorManagement);

    d->displayFilter = displayFilter;

    if (!initializing && needsFullRefresh) {
        canvas()->startUpdateInPatches(canvas()->image()->bounds());
    }
    else if (!initializing)  {
        canvas()->updateCanvas();
    }
}

void KisOpenGLCanvas2::notifyImageColorSpaceChanged(const KoColorSpace *cs)
{
    // FIXME: on color space change the data is refetched multiple
    //        times by different actors!

    if (d->openGLImageTextures->setImageColorSpace(cs)) {
        canvas()->startUpdateInPatches(canvas()->image()->bounds());
    }
}

void KisOpenGLCanvas2::setWrapAroundViewingMode(bool value)
{
    d->wrapAroundMode = value;
    update();
}

inline void rectToVertices(QVector3D* vertices, const QRectF &rc)
{
    vertices[0] = QVector3D(rc.left(),  rc.bottom(), 0.f);
    vertices[1] = QVector3D(rc.left(),  rc.top(),    0.f);
    vertices[2] = QVector3D(rc.right(), rc.bottom(), 0.f);
    vertices[3] = QVector3D(rc.left(),  rc.top(), 0.f);
    vertices[4] = QVector3D(rc.right(), rc.top(), 0.f);
    vertices[5] = QVector3D(rc.right(), rc.bottom(),    0.f);
}

inline void rectToTexCoords(QVector2D* texCoords, const QRectF &rc)
{
    texCoords[0] = QVector2D(rc.left(), rc.bottom());
    texCoords[1] = QVector2D(rc.left(), rc.top());
    texCoords[2] = QVector2D(rc.right(), rc.bottom());
    texCoords[3] = QVector2D(rc.left(), rc.top());
    texCoords[4] = QVector2D(rc.right(), rc.top());
    texCoords[5] = QVector2D(rc.right(), rc.bottom());
}

void KisOpenGLCanvas2::initializeGL()
{
    KisOpenGL::initializeContext(context());
    initializeOpenGLFunctions();
#if !defined(Q_OS_MACOS) && !defined(HAS_ONLY_OPENGL_ES)
    if (!KisOpenGL::hasOpenGLES()) {
        d->glFn201 = context()->versionFunctions<QOpenGLFunctions_2_1>();
        if (!d->glFn201) {
            warnUI << "Cannot obtain QOpenGLFunctions_2_1, glLogicOp cannot be used";
        }
    } else {
        d->glFn201 = nullptr;
    }
#endif

    KisConfig cfg(true);
    d->openGLImageTextures->setProofingConfig(canvas()->proofingConfiguration());
    d->openGLImageTextures->initGL(context()->functions());
    d->openGLImageTextures->generateCheckerTexture(createCheckersImage(cfg.checkSize()));

    initializeShaders();

    // If we support OpenGL 3.2, then prepare our VAOs and VBOs for drawing
    if (KisOpenGL::hasOpenGL3()) {
        d->quadVAO.create();
        d->quadVAO.bind();

        glEnableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
        glEnableVertexAttribArray(PROGRAM_TEXCOORD_ATTRIBUTE);

        // Create the vertex buffer object, it has 6 vertices with 3 components
        d->quadBuffers[0].create();
        d->quadBuffers[0].setUsagePattern(QOpenGLBuffer::StaticDraw);
        d->quadBuffers[0].bind();
        d->quadBuffers[0].allocate(d->vertices, 6 * 3 * sizeof(float));
        glVertexAttribPointer(PROGRAM_VERTEX_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Create the texture buffer object, it has 6 texture coordinates with 2 components
        d->quadBuffers[1].create();
        d->quadBuffers[1].setUsagePattern(QOpenGLBuffer::StaticDraw);
        d->quadBuffers[1].bind();
        d->quadBuffers[1].allocate(d->texCoords, 6 * 2 * sizeof(float));
        glVertexAttribPointer(PROGRAM_TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Create the outline buffer, this buffer will store the outlines of
        // tools and will frequently change data
        d->outlineVAO.create();
        d->outlineVAO.bind();

        glEnableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
        glEnableVertexAttribArray(PROGRAM_TEXCOORD_ATTRIBUTE);

        // The outline buffer has a StreamDraw usage pattern, because it changes constantly
        d->lineVertexBuffer.create();
        d->lineVertexBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
        d->lineVertexBuffer.bind();
        glVertexAttribPointer(PROGRAM_VERTEX_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0, 0);

        d->lineTexCoordBuffer.create();
        d->lineTexCoordBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
        d->lineTexCoordBuffer.bind();
        glVertexAttribPointer(PROGRAM_TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, 0 ,0);
    }

    Sync::init(context());

    d->canvasInitialized = true;
}

/**
 * Loads all shaders and reports compilation problems
 */
void KisOpenGLCanvas2::initializeShaders()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->canvasInitialized);

    delete d->checkerShader;
    delete d->solidColorShader;
    delete d->overlayInvertedShader;
    d->checkerShader = 0;
    d->solidColorShader = 0;
    d->overlayInvertedShader = 0;

    try {
        d->checkerShader = d->shaderLoader.loadCheckerShader();
        d->solidColorShader = d->shaderLoader.loadSolidColorShader();
        d->overlayInvertedShader = d->shaderLoader.loadOverlayInvertedShader();
    } catch (const ShaderLoaderException &e) {
        reportFailedShaderCompilation(e.what());
    }

    initializeDisplayShader();
}

void KisOpenGLCanvas2::initializeDisplayShader()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->canvasInitialized);

    bool useHiQualityFiltering = d->filterMode == KisOpenGL::HighQualityFiltering;

    delete d->displayShader;
    d->displayShader = 0;

    try {
        d->displayShader = d->shaderLoader.loadDisplayShader(d->displayFilter, useHiQualityFiltering);
        d->displayShaderCompiledWithDisplayFilterSupport = d->displayFilter;
    } catch (const ShaderLoaderException &e) {
        reportFailedShaderCompilation(e.what());
    }
}

/**
 * Displays a message box telling the user that
 * shader compilation failed and turns off OpenGL.
 */
void KisOpenGLCanvas2::reportFailedShaderCompilation(const QString &context)
{
    KisConfig cfg(false);

    qDebug() << "Shader Compilation Failure: " << context;
    QMessageBox::critical(this, i18nc("@title:window", "Krita"),
                          i18n("Krita could not initialize the OpenGL canvas:\n\n%1\n\n Krita will disable OpenGL and close now.", context),
                          QMessageBox::Close);

    cfg.disableOpenGL();
    cfg.setCanvasState("OPENGL_FAILED");
}

void KisOpenGLCanvas2::resizeGL(int width, int height)
{
    // The given size is the widget size but here we actually want to give
    // KisCoordinatesConverter the viewport size aligned to device pixels.
    if (KisOpenGL::supportsRenderToFBO()) {
        d->canvasFBO.reset(new QOpenGLFramebufferObject(QSize(width * devicePixelRatioF(), height * devicePixelRatioF())));
    }
    coordinatesConverter()->setCanvasWidgetSize(widgetSizeAlignedToDevicePixel());
    paintGL();
}

void KisOpenGLCanvas2::paintGL()
{
    if (!OPENGL_SUCCESS) {
        KisConfig cfg(false);
        cfg.writeEntry("canvasState", "OPENGL_PAINT_STARTED");
    }

    KisOpenglCanvasDebugger::instance()->nofityPaintRequested();

    if (d->canvasFBO) {
        d->canvasFBO->bind();
    }

    renderCanvasGL();

    if (d->canvasFBO) {
        d->canvasFBO->release();
        QOpenGLFramebufferObject::blitFramebuffer(nullptr, d->canvasFBO.data(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        QOpenGLFramebufferObject::bindDefault();
    }

    if (d->glSyncObject) {
        Sync::deleteSync(d->glSyncObject);
    }
    d->glSyncObject = Sync::getSync();

    QPainter gc(this);
    renderDecorations(&gc);
    gc.end();

    if (!OPENGL_SUCCESS) {
        KisConfig cfg(false);
        cfg.writeEntry("canvasState", "OPENGL_SUCCESS");
        OPENGL_SUCCESS = true;
    }
}

void KisOpenGLCanvas2::paintToolOutline(const QPainterPath &path)
{
    if (!d->overlayInvertedShader->bind()) {
        return;
    }

    QSizeF widgetSize = widgetSizeAlignedToDevicePixel();

    // setup the mvp transformation
    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    // FIXME: It may be better to have the projection in device pixel, but
    //       this requires introducing a new coordinate system.
    projectionMatrix.ortho(0, widgetSize.width(), widgetSize.height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection & texture matrices
    QMatrix4x4 modelMatrix(coordinatesConverter()->flakeToWidgetTransform());
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->overlayInvertedShader->setUniformValue(d->overlayInvertedShader->location(Uniform::ModelViewProjection), modelMatrix);

    d->overlayInvertedShader->setUniformValue(
                d->overlayInvertedShader->location(Uniform::FragmentColor),
                QVector4D(d->cursorColor.redF(), d->cursorColor.greenF(), d->cursorColor.blueF(), 1.0f));

    // NOTE: Texture matrix transforms flake space -> widget space -> OpenGL UV texcoord space..
    const QMatrix4x4 widgetToFBOTexCoordTransform = KisAlgebra2D::mapToRectInverse(QRect(QPoint(0, this->height()),
                                                                                         QSize(this->width(), -1 * this->height())));
    const QMatrix4x4 textureMatrix = widgetToFBOTexCoordTransform *
        QMatrix4x4(coordinatesConverter()->flakeToWidgetTransform());

    d->overlayInvertedShader->setUniformValue(d->overlayInvertedShader->location(Uniform::TextureMatrix), textureMatrix);

    // For the legacy shader, we should use old fixed function
    // blending operations if available.
    if (!KisOpenGL::hasOpenGL3() && !KisOpenGL::hasOpenGLES()) {
        #ifndef HAS_ONLY_OPENGL_ES
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_COLOR_LOGIC_OP);

        #ifndef Q_OS_MACOS
        if (d->glFn201) {
            d->glFn201->glLogicOp(GL_XOR);
        }
        #else   // Q_OS_MACOS
        glLogicOp(GL_XOR);
        #endif  // Q_OS_MACOS

        #else   // HAS_ONLY_OPENGL_ES
        KIS_ASSERT_X(false, "KisOpenGLCanvas2::paintToolOutline",
                        "Unexpected KisOpenGL::hasOpenGLES returned false");
        #endif  // HAS_ONLY_OPENGL_ES
    }

    // Paint the tool outline
    if (KisOpenGL::hasOpenGL3()) {
        d->outlineVAO.bind();
        d->lineVertexBuffer.bind();
    }

    // Convert every disjointed subpath to a polygon and draw that polygon
    QList<QPolygonF> subPathPolygons = path.toSubpathPolygons();
    for (int polyIndex = 0; polyIndex < subPathPolygons.size(); polyIndex++) {
        const QPolygonF& polygon = subPathPolygons.at(polyIndex);

        QVector<QVector3D> vertices;
        QVector<QVector2D> texCoords;
        vertices.resize(polygon.count());
        texCoords.resize(polygon.count());

        for (int vertIndex = 0; vertIndex < polygon.count(); vertIndex++) {
            QPointF point = polygon.at(vertIndex);
            vertices[vertIndex].setX(point.x());
            vertices[vertIndex].setY(point.y());
            texCoords[vertIndex].setX(point.x());
            texCoords[vertIndex].setY(point.y());
        }
        if (KisOpenGL::hasOpenGL3()) {
            d->lineVertexBuffer.bind();
            d->lineVertexBuffer.allocate(vertices.constData(), 3 * vertices.size() * sizeof(float));
            d->lineTexCoordBuffer.bind();
            d->lineTexCoordBuffer.allocate(texCoords.constData(), 2 * texCoords.size() * sizeof(float));
        }
        else {
            d->overlayInvertedShader->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
            d->overlayInvertedShader->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, vertices.constData());
            d->overlayInvertedShader->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
            d->overlayInvertedShader->setAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE, texCoords.constData());
        }

        const bool usingLegacyShader = !((KisOpenGL::hasOpenGL3() || KisOpenGL::hasOpenGLES()) && KisOpenGL::supportsRenderToFBO());
        if (usingLegacyShader){
            glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
        } else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, d->canvasFBO->texture());

            glDrawArrays(GL_LINE_STRIP, 0, vertices.size());

            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    if (KisOpenGL::hasOpenGL3()) {
        d->lineVertexBuffer.release();
        d->outlineVAO.release();
    }

    if (!KisOpenGL::hasOpenGLES()) {
#ifndef HAS_ONLY_OPENGL_ES
        glDisable(GL_COLOR_LOGIC_OP);
#else
        KIS_ASSERT_X(false, "KisOpenGLCanvas2::paintToolOutline",
                "Unexpected KisOpenGL::hasOpenGLES returned false");
#endif
    } else {
        glDisable(GL_BLEND);
    }

    d->overlayInvertedShader->release();
}

bool KisOpenGLCanvas2::isBusy() const
{
    const bool isBusyStatus = Sync::syncStatus(d->glSyncObject) == Sync::Unsignaled;
    KisOpenglCanvasDebugger::instance()->nofitySyncStatus(isBusyStatus);

    return isBusyStatus;
}

void KisOpenGLCanvas2::setLodResetInProgress(bool value)
{
    d->lodSwitchInProgress = value;
}

void KisOpenGLCanvas2::drawCheckers()
{
    if (!d->checkerShader) {
        return;
    }

    KisCoordinatesConverter *converter = coordinatesConverter();
    QTransform textureTransform;
    QTransform modelTransform;
    QRectF textureRect;
    QRectF modelRect;

    QSizeF widgetSize = widgetSizeAlignedToDevicePixel();
    QRectF viewportRect = !d->wrapAroundMode ?
                converter->imageRectInViewportPixels() :
                converter->widgetToViewport(QRectF(0, 0, widgetSize.width(), widgetSize.height()));

    if (!canvas()->renderingLimit().isEmpty()) {
        const QRect vrect = converter->imageToViewport(canvas()->renderingLimit()).toAlignedRect();
        viewportRect &= vrect;
    }

    converter->getOpenGLCheckersInfo(viewportRect,
                                     &textureTransform, &modelTransform, &textureRect, &modelRect, d->scrollCheckers);

    textureTransform *= QTransform::fromScale(d->checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE,
                                              d->checkSizeScale / KisOpenGLImageTextures::BACKGROUND_TEXTURE_SIZE);

    if (!d->checkerShader->bind()) {
        qWarning() << "Could not bind checker shader";
        return;
    }

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    // FIXME: It may be better to have the projection in device pixel, but
    //       this requires introducing a new coordinate system.
    projectionMatrix.ortho(0, widgetSize.width(), widgetSize.height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(modelTransform);
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->checkerShader->setUniformValue(d->checkerShader->location(Uniform::ModelViewProjection), modelMatrix);

    QMatrix4x4 textureMatrix(textureTransform);
    d->checkerShader->setUniformValue(d->checkerShader->location(Uniform::TextureMatrix), textureMatrix);

    //Setup the geometry for rendering
    if (KisOpenGL::hasOpenGL3()) {
        rectToVertices(d->vertices, modelRect);
        d->quadBuffers[0].bind();
        d->quadBuffers[0].write(0, d->vertices, 3 * 6 * sizeof(float));

        rectToTexCoords(d->texCoords, textureRect);
        d->quadBuffers[1].bind();
        d->quadBuffers[1].write(0, d->texCoords, 2 * 6 * sizeof(float));
    }
    else {
        rectToVertices(d->vertices, modelRect);
        d->checkerShader->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
        d->checkerShader->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, d->vertices);

        rectToTexCoords(d->texCoords, textureRect);
        d->checkerShader->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
        d->checkerShader->setAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE, d->texCoords);
    }

    // render checkers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, d->openGLImageTextures->checkerTexture());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    d->checkerShader->release();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void KisOpenGLCanvas2::drawGrid()
{
    if (!d->solidColorShader->bind()) {
        return;
    }

    QSizeF widgetSize = widgetSizeAlignedToDevicePixel();

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    // FIXME: It may be better to have the projection in device pixel, but
    //       this requires introducing a new coordinate system.
    projectionMatrix.ortho(0, widgetSize.width(), widgetSize.height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(coordinatesConverter()->imageToWidgetTransform());
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->solidColorShader->setUniformValue(d->solidColorShader->location(Uniform::ModelViewProjection), modelMatrix);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    d->solidColorShader->setUniformValue(
                d->solidColorShader->location(Uniform::FragmentColor),
                QVector4D(d->gridColor.redF(), d->gridColor.greenF(), d->gridColor.blueF(), 0.5f));

    if (KisOpenGL::hasOpenGL3()) {
        d->outlineVAO.bind();
        d->lineVertexBuffer.bind();
    }

    QRectF widgetRect(0,0, widgetSize.width(), widgetSize.height());
    QRectF widgetRectInImagePixels = coordinatesConverter()->documentToImage(coordinatesConverter()->widgetToDocument(widgetRect));
    QRect wr = widgetRectInImagePixels.toAlignedRect();

    if (!d->wrapAroundMode) {
        wr &= d->openGLImageTextures->storedImageBounds();
    }

    QPoint topLeftCorner = wr.topLeft();
    QPoint bottomRightCorner = wr.bottomRight() + QPoint(1, 1);
    QVector<QVector3D> grid;

    for (int i = topLeftCorner.x(); i <= bottomRightCorner.x(); ++i) {
        grid.append(QVector3D(i, topLeftCorner.y(), 0));
        grid.append(QVector3D(i, bottomRightCorner.y(), 0));
    }
    for (int i = topLeftCorner.y(); i <= bottomRightCorner.y(); ++i) {
        grid.append(QVector3D(topLeftCorner.x(), i, 0));
        grid.append(QVector3D(bottomRightCorner.x(), i, 0));
    }

    if (KisOpenGL::hasOpenGL3()) {
        d->lineVertexBuffer.allocate(grid.constData(), 3 * grid.size() * sizeof(float));
    }
    else {
        d->solidColorShader->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
        d->solidColorShader->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, grid.constData());
    }

    glDrawArrays(GL_LINES, 0, grid.size());

    if (KisOpenGL::hasOpenGL3()) {
        d->lineVertexBuffer.release();
        d->outlineVAO.release();
    }

    d->solidColorShader->release();
    glDisable(GL_BLEND);
}

void KisOpenGLCanvas2::drawImage()
{
    if (!d->displayShader) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    KisCoordinatesConverter *converter = coordinatesConverter();

    d->displayShader->bind();

    QSizeF widgetSize = widgetSizeAlignedToDevicePixel();

    QMatrix4x4 projectionMatrix;
    projectionMatrix.setToIdentity();
    // FIXME: It may be better to have the projection in device pixel, but
    //       this requires introducing a new coordinate system.
    projectionMatrix.ortho(0, widgetSize.width(), widgetSize.height(), 0, NEAR_VAL, FAR_VAL);

    // Set view/projection matrices
    QMatrix4x4 modelMatrix(converter->imageToWidgetTransform());
    modelMatrix.optimize();
    modelMatrix = projectionMatrix * modelMatrix;
    d->displayShader->setUniformValue(d->displayShader->location(Uniform::ModelViewProjection), modelMatrix);

    QMatrix4x4 textureMatrix;
    textureMatrix.setToIdentity();
    d->displayShader->setUniformValue(d->displayShader->location(Uniform::TextureMatrix), textureMatrix);

    QRectF widgetRect(0,0, widgetSize.width(), widgetSize.height());
    QRectF widgetRectInImagePixels = converter->documentToImage(converter->widgetToDocument(widgetRect));

    const QRect renderingLimit = canvas()->renderingLimit();

    if (!renderingLimit.isEmpty()) {
        widgetRectInImagePixels &= renderingLimit;
    }

    qreal scaleX, scaleY;
    converter->imagePhysicalScale(&scaleX, &scaleY);

    d->displayShader->setUniformValue(d->displayShader->location(Uniform::ViewportScale), (GLfloat) scaleX);
    d->displayShader->setUniformValue(d->displayShader->location(Uniform::TexelSize), (GLfloat) d->openGLImageTextures->texelSize());

    QRect ir = d->openGLImageTextures->storedImageBounds();
    QRect wr = widgetRectInImagePixels.toAlignedRect();

    if (!d->wrapAroundMode) {
        // if we don't want to paint wrapping images, just limit the
        // processing area, and the code will handle all the rest
        wr &= ir;
    }

    int firstColumn = d->xToColWithWrapCompensation(wr.left(), ir);
    int lastColumn = d->xToColWithWrapCompensation(wr.right(), ir);
    int firstRow = d->yToRowWithWrapCompensation(wr.top(), ir);
    int lastRow = d->yToRowWithWrapCompensation(wr.bottom(), ir);

    int minColumn = d->openGLImageTextures->xToCol(ir.left());
    int maxColumn = d->openGLImageTextures->xToCol(ir.right());
    int minRow = d->openGLImageTextures->yToRow(ir.top());
    int maxRow = d->openGLImageTextures->yToRow(ir.bottom());

    int imageColumns = maxColumn - minColumn + 1;
    int imageRows = maxRow - minRow + 1;

    for (int col = firstColumn; col <= lastColumn; col++) {
        for (int row = firstRow; row <= lastRow; row++) {

            int effectiveCol = col;
            int effectiveRow = row;
            QPointF tileWrappingTranslation;

            if (effectiveCol > maxColumn || effectiveCol < minColumn) {
                int translationStep = floor(qreal(col) / imageColumns);
                int originCol = translationStep * imageColumns;
                effectiveCol = col - originCol;
                tileWrappingTranslation.rx() = translationStep * ir.width();
            }

            if (effectiveRow > maxRow || effectiveRow < minRow) {
                int translationStep = floor(qreal(row) / imageRows);
                int originRow = translationStep * imageRows;
                effectiveRow = row - originRow;
                tileWrappingTranslation.ry() = translationStep * ir.height();
            }

            KisTextureTile *tile =
                    d->openGLImageTextures->getTextureTileCR(effectiveCol, effectiveRow);

            if (!tile) {
                warnUI << "OpenGL: Trying to paint texture tile but it has not been created yet.";
                continue;
            }

            /*
             * We create a float rect here to workaround Qt's
             * "history reasons" in calculation of right()
             * and bottom() coordinates of integer rects.
             */

            QRectF textureRect;
            QRectF modelRect;

            if (renderingLimit.isEmpty()) {
                textureRect = tile->tileRectInTexturePixels();
                modelRect = tile->tileRectInImagePixels().translated(tileWrappingTranslation.x(), tileWrappingTranslation.y());
            } else {
                const QRect limitedTileRect = tile->tileRectInImagePixels() & renderingLimit;
                textureRect = tile->imageRectInTexturePixels(limitedTileRect);
                modelRect = limitedTileRect.translated(tileWrappingTranslation.x(), tileWrappingTranslation.y());
            }

            //Setup the geometry for rendering
            if (KisOpenGL::hasOpenGL3()) {
                rectToVertices(d->vertices, modelRect);
                d->quadBuffers[0].bind();
                d->quadBuffers[0].write(0, d->vertices, 3 * 6 * sizeof(float));

                rectToTexCoords(d->texCoords, textureRect);
                d->quadBuffers[1].bind();
                d->quadBuffers[1].write(0, d->texCoords, 2 * 6 * sizeof(float));
            }
            else {
                rectToVertices(d->vertices, modelRect);
                d->displayShader->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
                d->displayShader->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, d->vertices);

                rectToTexCoords(d->texCoords, textureRect);
                d->displayShader->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
                d->displayShader->setAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE, d->texCoords);
            }

            if (d->displayFilter) {
                glActiveTexture(GL_TEXTURE0 + 1);
                glBindTexture(GL_TEXTURE_3D, d->displayFilter->lutTexture());
                d->displayShader->setUniformValue(d->displayShader->location(Uniform::Texture1), 1);
            }

            glActiveTexture(GL_TEXTURE0);

            const int currentLodPlane = tile->bindToActiveTexture(d->lodSwitchInProgress);

            if (d->displayShader->location(Uniform::FixedLodLevel) >= 0) {
                d->displayShader->setUniformValue(d->displayShader->location(Uniform::FixedLodLevel),
                                                  (GLfloat) currentLodPlane);
            }

            if (currentLodPlane > 0) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            } else if (SCALE_MORE_OR_EQUAL_TO(scaleX, scaleY, 2.0)) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                switch(d->filterMode) {
                case KisOpenGL::NearestFilterMode:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    break;
                case KisOpenGL::BilinearFilterMode:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    break;
                case KisOpenGL::TrilinearFilterMode:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    break;
                case KisOpenGL::HighQualityFiltering:
                    if (SCALE_LESS_THAN(scaleX, scaleY, 0.5)) {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                    } else {
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    }
                    break;
                }
            }

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    d->displayShader->release();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_BLEND);
}

QSize KisOpenGLCanvas2::viewportDevicePixelSize() const
{
    // This is how QOpenGLCanvas sets the FBO and the viewport size. If
    // devicePixelRatioF() is non-integral, the result is truncated.
    int viewportWidth = static_cast<int>(width() * devicePixelRatioF());
    int viewportHeight = static_cast<int>(height() * devicePixelRatioF());
    return QSize(viewportWidth, viewportHeight);
}

QSizeF KisOpenGLCanvas2::widgetSizeAlignedToDevicePixel() const
{
    QSize viewportSize = viewportDevicePixelSize();
    qreal scaledWidth = viewportSize.width() / devicePixelRatioF();
    qreal scaledHeight = viewportSize.height() / devicePixelRatioF();
    return QSizeF(scaledWidth, scaledHeight);
}

void KisOpenGLCanvas2::slotConfigChanged()
{
    KisConfig cfg(true);
    d->checkSizeScale = KisOpenGLImageTextures::BACKGROUND_TEXTURE_CHECK_SIZE / static_cast<GLfloat>(cfg.checkSize());
    d->scrollCheckers = cfg.scrollCheckers();

    d->openGLImageTextures->generateCheckerTexture(createCheckersImage(cfg.checkSize()));
    d->openGLImageTextures->updateConfig(cfg.useOpenGLTextureBuffer(), cfg.numMipmapLevels());
    d->filterMode = (KisOpenGL::FilterMode) cfg.openGLFilteringMode();

    d->cursorColor = cfg.getCursorMainColor();

    notifyConfigChanged();
}

void KisOpenGLCanvas2::slotPixelGridModeChanged()
{
    KisConfig cfg(true);

    d->pixelGridDrawingThreshold = cfg.getPixelGridDrawingThreshold();
    d->pixelGridEnabled = cfg.pixelGridEnabled();
    d->gridColor = cfg.getPixelGridColor();

    update();
}

void KisOpenGLCanvas2::slotShowFloatingMessage(const QString &message, int timeout, bool priority)
{
    canvas()->imageView()->showFloatingMessage(message, QIcon(), timeout, priority ? KisFloatingMessage::High : KisFloatingMessage::Medium);
}

QVariant KisOpenGLCanvas2::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return processInputMethodQuery(query);
}

void KisOpenGLCanvas2::inputMethodEvent(QInputMethodEvent *event)
{
    processInputMethodEvent(event);
}

void KisOpenGLCanvas2::renderCanvasGL()
{
    {
        // Draw the border (that is, clear the whole widget to the border color)
        QColor widgetBackgroundColor = borderColor();

        const KoColorSpace *finalColorSpace =
               KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                            d->openGLImageTextures->updateInfoBuilder().destinationColorSpace()->colorDepthId().id(),
                                                            d->openGLImageTextures->monitorProfile());

        KoColor convertedBackgroudColor = KoColor(widgetBackgroundColor, KoColorSpaceRegistry::instance()->rgb8());
        convertedBackgroudColor.convertTo(finalColorSpace);

        QVector<float> channels = QVector<float>(4);
        convertedBackgroudColor.colorSpace()->normalisedChannelsValue(convertedBackgroudColor.data(), channels);
        // Data returned by KoRgbU8ColorSpace comes in the order: blue, green, red.
        glClearColor(channels[2], channels[1], channels[0], 1.0);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if ((d->displayFilter && d->displayFilter->updateShader()) ||
        (bool(d->displayFilter) != d->displayShaderCompiledWithDisplayFilterSupport)) {

        KIS_SAFE_ASSERT_RECOVER_NOOP(d->canvasInitialized);

        d->canvasInitialized = false; // TODO: check if actually needed?
        initializeDisplayShader();
        d->canvasInitialized = true;
    }

    if (KisOpenGL::hasOpenGL3()) {
        d->quadVAO.bind();
    }

    drawCheckers();
    drawImage();
    if ((coordinatesConverter()->effectiveZoom() > d->pixelGridDrawingThreshold - 0.00001) && d->pixelGridEnabled) {
        drawGrid();
    }
    if (KisOpenGL::hasOpenGL3()) {
        d->quadVAO.release();
    }
}

void KisOpenGLCanvas2::renderDecorations(QPainter *painter)
{
    QRect boundingRect = coordinatesConverter()->imageRectInWidgetPixels().toAlignedRect();
    drawDecorations(*painter, boundingRect);
}


void KisOpenGLCanvas2::setDisplayColorConverter(KisDisplayColorConverter *colorConverter)
{
    d->openGLImageTextures->setMonitorProfile(colorConverter->openGLCanvasSurfaceProfile(),
                                              colorConverter->renderingIntent(),
                                              colorConverter->conversionFlags());
}

void KisOpenGLCanvas2::channelSelectionChanged(const QBitArray &channelFlags)
{
    d->openGLImageTextures->setChannelFlags(channelFlags);
}


void KisOpenGLCanvas2::finishResizingImage(qint32 w, qint32 h)
{
    if (d->canvasInitialized) {
        d->openGLImageTextures->slotImageSizeChanged(w, h);
    }
}

KisUpdateInfoSP KisOpenGLCanvas2::startUpdateCanvasProjection(const QRect & rc, const QBitArray &channelFlags)
{
    d->openGLImageTextures->setChannelFlags(channelFlags);
    if (canvas()->proofingConfigUpdated()) {
        d->openGLImageTextures->setProofingConfig(canvas()->proofingConfiguration());
        canvas()->setProofingConfigUpdated(false);
    }
    return d->openGLImageTextures->updateCache(rc, d->openGLImageTextures->image());
}


QRect KisOpenGLCanvas2::updateCanvasProjection(KisUpdateInfoSP info)
{
    // See KisQPainterCanvas::updateCanvasProjection for more info
    bool isOpenGLUpdateInfo = dynamic_cast<KisOpenGLUpdateInfo*>(info.data());
    if (isOpenGLUpdateInfo) {
        d->openGLImageTextures->recalculateCache(info, d->lodSwitchInProgress);
    }
    return QRect(); // FIXME: Implement dirty rect for OpenGL
}

QVector<QRect> KisOpenGLCanvas2::updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects)
{
#ifdef Q_OS_MACOS
    /**
     * On OSX openGL different (shared) contexts have different execution queues.
     * It means that the textures uploading and their painting can be easily reordered.
     * To overcome the issue, we should ensure that the textures are uploaded in the
     * same openGL context as the painting is done.
     */

    QOpenGLContext *oldContext = QOpenGLContext::currentContext();
    QSurface *oldSurface = oldContext ? oldContext->surface() : 0;

    this->makeCurrent();
#endif

    QVector<QRect> result = KisCanvasWidgetBase::updateCanvasProjection(infoObjects);

#ifdef Q_OS_MACOS
    if (oldContext) {
        oldContext->makeCurrent(oldSurface);
    } else {
        this->doneCurrent();
    }
#endif

    return result;
}

bool KisOpenGLCanvas2::callFocusNextPrevChild(bool next)
{
    return focusNextPrevChild(next);
}

KisOpenGLImageTexturesSP KisOpenGLCanvas2::openGLImageTextures() const
{
    return d->openGLImageTextures;
}
