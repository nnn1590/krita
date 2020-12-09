/* This file is part of the KDE project
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef SVGSYMBOLCOLLECTIONDOCKER_H
#define SVGSYMBOLCOLLECTIONDOCKER_H

#include <QDockWidget>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMap>
#include <QIcon>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>
#include <KisKineticScroller.h>

#include "ui_WdgSvgCollection.h"

class KoSvgSymbolCollectionResource;
class KisResourceModel;

class SvgCollectionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SvgCollectionModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDragActions() const override;
public:
    void setSvgSymbolCollectionResource(QSharedPointer<KoSvgSymbolCollectionResource> resource);
private:
    QSharedPointer<KoSvgSymbolCollectionResource> m_symbolCollection;
};


class SvgSymbolCollectionDockerFactory : public KoDockFactoryBase
{
public:
    SvgSymbolCollectionDockerFactory();

    QString id() const override;
    QDockWidget *createDockWidget() override;
    DockPosition defaultDockPosition() const override
    {
        return DockRight;
    }
};

class SvgSymbolCollectionDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:

    explicit SvgSymbolCollectionDocker(QWidget *parent = 0);
    ~SvgSymbolCollectionDocker();

    /// reimplemented
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

private Q_SLOTS:

    void collectionActivated(int index);
    void slotSetIconSize();

    void slotResourceModelAboutToBeReset();
    void slotResourceModelReset();


private:

    void clearModels();

    QScopedPointer<Ui_WdgSvgCollection> m_wdgSvgCollection;
    QMap<int, SvgCollectionModel*> m_collectionsModelsCache;
    QSlider* m_iconSizeSlider;

    KisResourceModel* m_resourceModel;
    int m_rememberedSvgCollectionId;
};

#endif
