/*
 * Copyright (C) 2017 Jouni Pentikäinen <joupent@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <KoShapeCreateCommand.h>
#include <KoShapeDeleteCommand.h>
#include <KoKeepShapesSelectedCommand.h>
#include <KoSelection.h>
#include <kis_node_visitor.h>
#include <kis_processing_visitor.h>
#include <kis_shape_layer_canvas.h>

#include "KisReferenceImagesLayer.h"
#include "KisReferenceImage.h"
#include "KisDocument.h"

struct AddReferenceImagesCommand : KoShapeCreateCommand
{
    AddReferenceImagesCommand(KisDocument *document, KisSharedPtr<KisReferenceImagesLayer> layer, const QList<KoShape*> referenceImages, KUndo2Command *parent = nullptr)
        : KoShapeCreateCommand(layer->shapeController(), referenceImages, layer.data(), parent, kundo2_i18n("Add reference image"))
        , m_document(document)
        , m_layer(layer)
    {}

    void redo() override {
        auto layer = m_document->referenceImagesLayer();
        KIS_SAFE_ASSERT_RECOVER_NOOP(!layer || layer == m_layer);

        if (!layer) {
            m_document->setReferenceImagesLayer(m_layer, true);
        }

        KoShapeCreateCommand::redo();
    }

    void undo() override {
        KoShapeCreateCommand::undo();

        if (m_layer->shapeCount() == 0) {
            m_document->setReferenceImagesLayer(nullptr, true);
        }
    }

private:
    KisDocument *m_document;
    KisSharedPtr<KisReferenceImagesLayer> m_layer;
};

struct RemoveReferenceImagesCommand : KoShapeDeleteCommand
{
    RemoveReferenceImagesCommand(KisDocument *document, KisSharedPtr<KisReferenceImagesLayer> layer, QList<KoShape *> referenceImages, KUndo2Command *parent = nullptr)
        : KoShapeDeleteCommand(layer->shapeController(), referenceImages, parent)
        , m_document(document)
        , m_layer(layer)
    {}


    void redo() override {
        KoShapeDeleteCommand::redo();

        if (m_layer->shapeCount() == 0) {
            m_document->setReferenceImagesLayer(nullptr, true);
        }
    }

    void undo() override {
        auto layer = m_document->referenceImagesLayer();
        KIS_SAFE_ASSERT_RECOVER_NOOP(!layer || layer == m_layer);

        if (!layer) {
            m_document->setReferenceImagesLayer(m_layer, true);
        }

        KoShapeDeleteCommand::undo();
    }

private:
    KisDocument *m_document;
    KisSharedPtr<KisReferenceImagesLayer> m_layer;
};

class ReferenceImagesCanvas : public KisShapeLayerCanvasBase
{
public:
    ReferenceImagesCanvas(KisReferenceImagesLayer *parent, KisImageWSP image)
        : KisShapeLayerCanvasBase(parent, image)
        , m_layer(parent)
    {}

    void updateCanvas(const QRectF &rect) override
    {
        if (!m_layer->image() || m_isDestroying) {
            return;
        }

        QRectF r = m_viewConverter->documentToView(rect);
        m_layer->signalUpdate(r);
    }

    void forceRepaint() override
    {
        m_layer->signalUpdate(m_layer->boundingImageRect());
    }

    bool hasPendingUpdates() const override
    {
        return false;
    }

    void rerenderAfterBeingInvisible() override {}
    void resetCache() override {}
    void setImage(KisImageWSP image) override
    {
        m_viewConverter->setImage(image);
    }

private:
    KisReferenceImagesLayer *m_layer;
};

KisReferenceImagesLayer::KisReferenceImagesLayer(KoShapeControllerBase* shapeController, KisImageWSP image)
    : KisShapeLayer(shapeController, image, i18n("Reference images"), OPACITY_OPAQUE_U8, new ReferenceImagesCanvas(this, image))
{}

KisReferenceImagesLayer::KisReferenceImagesLayer(const KisReferenceImagesLayer &rhs)
    : KisShapeLayer(rhs, rhs.shapeController(), new ReferenceImagesCanvas(this, rhs.image()))
{}

KUndo2Command * KisReferenceImagesLayer::addReferenceImages(KisDocument *document, const QList<KoShape*> referenceImages)
{
    KisSharedPtr<KisReferenceImagesLayer> layer = document->referenceImagesLayer();
    if (!layer) {
        layer = new KisReferenceImagesLayer(document->shapeController(), document->image());
    }

    KUndo2Command *parentCommand = new KUndo2Command();

    new KoKeepShapesSelectedCommand(layer->shapeManager()->selection()->selectedShapes(), {}, layer->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::INITIALIZING, parentCommand);
    AddReferenceImagesCommand *cmd = new AddReferenceImagesCommand(document, layer, referenceImages, parentCommand);
    parentCommand->setText(cmd->text());
    new KoKeepShapesSelectedCommand({}, referenceImages, layer->selectedShapesProxy(), KisCommandUtils::FlipFlopCommand::State::FINALIZING, parentCommand);

    return parentCommand;
}

KUndo2Command * KisReferenceImagesLayer::removeReferenceImages(KisDocument *document, QList<KoShape*> referenceImages)
{
    return new RemoveReferenceImagesCommand(document, this, referenceImages);
}

QVector<KisReferenceImage*> KisReferenceImagesLayer::referenceImages() const
{
    QVector<KisReferenceImage*> references;

    Q_FOREACH(auto shape, shapes()) {
        KisReferenceImage *referenceImage = dynamic_cast<KisReferenceImage*>(shape);
        if (referenceImage) {
            references.append(referenceImage);
        }
    }
    return references;
}

void KisReferenceImagesLayer::paintReferences(QPainter &painter) {
    painter.setTransform(converter()->documentToView(), true);
    shapeManager()->paint(painter, false);
}

bool KisReferenceImagesLayer::allowAsChild(KisNodeSP) const
{
    return false;
}

bool KisReferenceImagesLayer::accept(KisNodeVisitor &visitor)
{
    return visitor.visit(this);
}

void KisReferenceImagesLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    visitor.visit(this, undoAdapter);
}

bool KisReferenceImagesLayer::isFakeNode() const
{
    return true;
}

KUndo2Command *KisReferenceImagesLayer::setProfile(const KoColorProfile *profile)
{
    // references should not be converted with the image
    Q_UNUSED(profile);
    return 0;
}

KUndo2Command *KisReferenceImagesLayer::convertTo(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags)
{
    // references should not be converted with the image
    Q_UNUSED(dstColorSpace);
    Q_UNUSED(renderingIntent);
    Q_UNUSED(conversionFlags);
    return 0;
}

void KisReferenceImagesLayer::signalUpdate(const QRectF &rect)
{
    emit sigUpdateCanvas(rect);
}

QRectF KisReferenceImagesLayer::boundingImageRect() const
{
    return converter()->documentToView(boundingRect());
}

QColor KisReferenceImagesLayer::getPixel(QPointF position) const
{
    const QPointF docPoint = converter()->viewToDocument(position);

    KoShape *shape = shapeManager()->shapeAt(docPoint);

    if (shape) {
        auto *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(reference, QColor());

        return reference->getPixel(docPoint);
    }

    return QColor();
}
