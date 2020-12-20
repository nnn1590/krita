/*
 * SPDX-FileCopyrightText: 2007, 2010 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_canvas_widget_base.h"

#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QMenu>

#include <KoShapeManager.h>
#include <KoToolManager.h>
#include <KoViewConverter.h>
#include <KoToolProxy.h>
#include <KoCanvasController.h>
#include <KoShape.h>
#include <KoSelection.h>
#include <KoShapePaintingContext.h>

#include "kis_coordinates_converter.h"
#include "kis_canvas_decoration.h"
#include "kis_config.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_selection_manager.h"
#include "KisDocument.h"
#include "kis_update_info.h"
#include "KisQPainterStateSaver.h"


struct KisCanvasWidgetBase::Private
{
public:
    Private(KisCanvas2 *newCanvas, KisCoordinatesConverter *newCoordinatesConverter)
        : canvas(newCanvas)
        , coordinatesConverter(newCoordinatesConverter)
        , viewConverter(newCanvas->viewConverter())
        , toolProxy(newCanvas->toolProxy())
        , ignorenextMouseEventExceptRightMiddleClick(0)
        , borderColor(Qt::gray)
    {}

    QList<KisCanvasDecorationSP> decorations;
    KisCanvas2 * canvas;
    KisCoordinatesConverter *coordinatesConverter;
    const KoViewConverter * viewConverter;
    KoToolProxy * toolProxy;
    QTimer blockMouseEvent;

    bool ignorenextMouseEventExceptRightMiddleClick; // HACK work around Qt bug not sending tablet right/dblclick https://bugreports.qt.io/browse/QTBUG-8598
    QColor borderColor;
};

KisCanvasWidgetBase::KisCanvasWidgetBase(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter)
    : m_d(new Private(canvas, coordinatesConverter))
{
    m_d->blockMouseEvent.setSingleShot(true);
}

KisCanvasWidgetBase::~KisCanvasWidgetBase()
{
    /**
     * Clear all the attached decoration. Otherwise they might decide
     * to process some events or signals after the canvas has been
     * destroyed
     */
    //5qDeleteAll(m_d->decorations);
    m_d->decorations.clear();

    delete m_d;
}

void KisCanvasWidgetBase::drawDecorations(QPainter & gc, const QRect &updateWidgetRect) const
{
    if (!m_d->canvas) {
        dbgFile<<"canvas doesn't exist, in canvas widget base!";
        return;
    }
    gc.save();

    // Setup the painter to take care of the offset; all that the
    // classes that do painting need to keep track of is resolution
    gc.setRenderHint(QPainter::Antialiasing);
    gc.setRenderHint(QPainter::TextAntialiasing);

    // This option does not do anything anymore with Qt4.6, so don't re-enable it since it seems to break display
    // https://lists.qt-project.org/pipermail/qt-interest-old/2009-December/017078.html
    // gc.setRenderHint(QPainter::HighQualityAntialiasing);

    gc.setRenderHint(QPainter::SmoothPixmapTransform);

    {
        KisQPainterStateSaver paintShapesState(&gc);
        gc.setTransform(m_d->coordinatesConverter->documentToWidgetTransform());

        // Paint the shapes (other than the layers)
        m_d->canvas->globalShapeManager()->paint(gc, false);

    }

    // ask the decorations to paint themselves
    // decorations are painted in "widget" coordinate system
    Q_FOREACH (KisCanvasDecorationSP deco, m_d->decorations) {
        if (deco->visible()) {
            deco->paint(gc, m_d->coordinatesConverter->widgetToDocument(updateWidgetRect), m_d->coordinatesConverter,m_d->canvas);
        }
    }

    {
        KisQPainterStateSaver paintDecorationsState(&gc);
        gc.setTransform(m_d->coordinatesConverter->flakeToWidgetTransform());

        // - some tools do not restore gc, but that is not important here
        toolProxy()->paint(gc, *m_d->viewConverter);
    }

    gc.restore();
}

void KisCanvasWidgetBase::addDecoration(KisCanvasDecorationSP deco)
{
    m_d->decorations.push_back(deco);
    std::stable_sort(m_d->decorations.begin(), m_d->decorations.end(), KisCanvasDecoration::comparePriority);
}

void KisCanvasWidgetBase::removeDecoration(const QString &id)
{
    for (auto it = m_d->decorations.begin(); it != m_d->decorations.end(); ++it) {
        if ((*it)->id() == id) {
            it = m_d->decorations.erase(it);
            break;
        }
    }
}

KisCanvasDecorationSP KisCanvasWidgetBase::decoration(const QString& id) const
{
    Q_FOREACH (KisCanvasDecorationSP deco, m_d->decorations) {
        if (deco->id() == id) {
            return deco;
        }
    }
    return 0;
}

void KisCanvasWidgetBase::setDecorations(const QList<KisCanvasDecorationSP > &decorations)
{
    m_d->decorations=decorations;
    std::stable_sort(m_d->decorations.begin(), m_d->decorations.end(), KisCanvasDecoration::comparePriority);
}

QList<KisCanvasDecorationSP > KisCanvasWidgetBase::decorations() const
{
    return m_d->decorations;
}

void KisCanvasWidgetBase::setWrapAroundViewingMode(bool value)
{
    Q_UNUSED(value);
}

QImage KisCanvasWidgetBase::createCheckersImage(qint32 checkSize)
{
    KisConfig cfg(true);

    if(checkSize < 0)
        checkSize = cfg.checkSize();

    QColor checkColor1 = cfg.checkersColor1();
    QColor checkColor2 = cfg.checkersColor2();

    QImage tile(checkSize * 2, checkSize * 2, QImage::Format_RGB32);
    QPainter pt(&tile);
    pt.fillRect(tile.rect(), checkColor2);
    pt.fillRect(0, 0, checkSize, checkSize, checkColor1);
    pt.fillRect(checkSize, checkSize, checkSize, checkSize, checkColor1);
    pt.end();

    return tile;
}

void KisCanvasWidgetBase::notifyConfigChanged()
{
    KisConfig cfg(true);
    m_d->borderColor = cfg.canvasBorderColor();
}

QColor KisCanvasWidgetBase::borderColor() const
{
    return m_d->borderColor;
}

KisCanvas2 *KisCanvasWidgetBase::canvas() const
{
    return m_d->canvas;
}

KisCoordinatesConverter* KisCanvasWidgetBase::coordinatesConverter() const
{
    return m_d->coordinatesConverter;
}

QVector<QRect> KisCanvasWidgetBase::updateCanvasProjection(const QVector<KisUpdateInfoSP> &infoObjects)
{
    QVector<QRect> dirtyViewRects;

    Q_FOREACH (KisUpdateInfoSP info, infoObjects) {
        dirtyViewRects << this->updateCanvasProjection(info);
    }

    return dirtyViewRects;
}

KoToolProxy *KisCanvasWidgetBase::toolProxy() const
{
    return m_d->toolProxy;
}

QVariant KisCanvasWidgetBase::processInputMethodQuery(Qt::InputMethodQuery query) const
{
    if (query == Qt::ImMicroFocus) {
        QRectF rect = m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter).toRectF();
        return m_d->coordinatesConverter->flakeToWidget(rect);
    }
    return m_d->toolProxy->inputMethodQuery(query, *m_d->viewConverter);
}

void KisCanvasWidgetBase::processInputMethodEvent(QInputMethodEvent *event)
{
    m_d->toolProxy->inputMethodEvent(event);
}
