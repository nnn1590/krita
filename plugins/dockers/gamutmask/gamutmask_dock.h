/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
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

#ifndef H_GAMUT_MASK_DOCK_H
#define H_GAMUT_MASK_DOCK_H

#include <QDockWidget>
#include <QPointer>
#include <QRegExpValidator>
#include <QMessageBox>

#include <KoCanvasObserverBase.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerObserver.h>
#include <resources/KoGamutMask.h>
#include <KisDocument.h>
#include <KisView.h>
#include <kis_types.h>
#include <KisResourceItemChooser.h>

#include <kis_mainwindow_observer.h>

class KisCanvasResourceProvider;
class QButtonGroup;
class QMenu;

struct GamutMaskChooserUI;

class GamutMaskDock: public QDockWidget, public KisMainwindowObserver, public KoResourceServerObserver<KoGamutMask>
{
    Q_OBJECT

public:
    GamutMaskDock();
    ~GamutMaskDock() override;
    QString observerName() override { return "GamutMaskDock"; }
    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public: // KoResourceServerObserver
    void unsetResourceServer() override;
    void resourceAdded(KoGamutMaskSP /*resource*/) override {}
    void removingResource(KoGamutMaskSP resource) override;
    void resourceChanged(KoGamutMaskSP resource) override;
    void syncTaggedResourceView() override {}
    void syncTagAddition(const QString&) override {}
    void syncTagRemoval(const QString&) override {}

Q_SIGNALS:
    void sigGamutMaskSet(KoGamutMaskSP mask);
    void sigGamutMaskChanged(KoGamutMaskSP mask);
    void sigGamutMaskUnset();
    void sigGamutMaskPreviewUpdate();

private Q_SLOTS:
    void slotGamutMaskEdit();
    void slotGamutMaskSave();
    void slotGamutMaskCancelEdit();
    void slotGamutMaskSelected(KoGamutMaskSP mask);
    void slotGamutMaskPreview();
    void slotGamutMaskCreateNew();
    void slotGamutMaskDuplicate();
    void slotGamutMaskDelete();

    void slotDocumentRemoved(QString filename);
    void slotViewChanged();
    void slotDocumentSaved();

private:
    void closeMaskDocument();
    bool openMaskEditor();
    void cancelMaskEdit();
    void selectMask(KoGamutMaskSP mask, bool notifyItemChooser = true);
    bool saveSelectedMaskResource();
    void deleteMask();
    int getUserFeedback(QString text, QString informativeText = "",
                        QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::StandardButton defaultButton = QMessageBox::Yes,
                        QMessageBox::Icon severity = QMessageBox::Warning);

    int saveOrCancel(QMessageBox::StandardButton defaultAction = QMessageBox::Save);

    KoGamutMaskSP createMaskResource(KoGamutMaskSP sourceMask, QString newTitle);

    QPair<QString, QFileInfo> resolveMaskTitle(QString suggestedTitle);

    QList<KoShape*> getShapesFromLayer();
    KisShapeLayerSP getShapeLayer();

    KisCanvasResourceProvider* m_resourceProvider;

    bool m_selfClosingTemplate;
    bool m_externalTemplateClose;
    bool m_creatingNewMask;
    bool m_templatePrevSaved;
    bool m_selfSelectingMask;

    GamutMaskChooserUI* m_dockerUI;
    KisResourceItemChooser* m_maskChooser;

    KoGamutMaskSP m_selectedMask;
    QRegExpValidator* m_maskTitleValidator;

    KisDocument* m_maskDocument;
    KisView* m_view;
};


#endif // H_GAMUT_MASK_DOCK_H
