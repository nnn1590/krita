/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SELECTION_ACTION_FACTORIES_H
#define __KIS_SELECTION_ACTION_FACTORIES_H

#include "KisNoParameterActionFactory.h"
#include "operations/kis_operation.h"
#include "operations/kis_operation_configuration.h"
#include "operations/kis_filter_selection_operation.h"
#include "dialogs/kis_dlg_stroke_selection_properties.h"


struct KRITAUI_EXPORT KisSelectAllActionFactory : public KisNoParameterActionFactory {
    KisSelectAllActionFactory() : KisNoParameterActionFactory("select-all-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisDeselectActionFactory : public KisNoParameterActionFactory {
    KisDeselectActionFactory() : KisNoParameterActionFactory("deselect-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisReselectActionFactory : public KisNoParameterActionFactory {
    KisReselectActionFactory() : KisNoParameterActionFactory("reselect-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisFillActionFactory : public KisOperation
{
    KisFillActionFactory() : KisOperation("fill-ui-action") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override {
        run(config.getString("fill-source", "fg"), view);
    }
    /**
     * \p fillColor may be one of three variants:
     * - "fg" --- foreground color
     * - "bg" --- background color
     * - "pattern" --- current pattern
     */
    void run(const QString &fillSource, KisViewManager *view);
};

struct KRITAUI_EXPORT KisClearActionFactory : public KisNoParameterActionFactory {
    KisClearActionFactory() : KisNoParameterActionFactory("clear-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisImageResizeToSelectionActionFactory : public KisNoParameterActionFactory {
    KisImageResizeToSelectionActionFactory() : KisNoParameterActionFactory("resize-to-selection-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisCutCopyActionFactory : public KisOperation {
    KisCutCopyActionFactory() : KisOperation("cut-copy-ui-action") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override {
        run(config.getBool("will-cut", false), config.getBool("use-sharp-clip", false), view);
    }

    void run(bool willCut, bool makeSharpClip, KisViewManager *view);
};

struct KRITAUI_EXPORT KisCopyMergedActionFactory : public KisNoParameterActionFactory {
    KisCopyMergedActionFactory() : KisNoParameterActionFactory("copy-merged-ui-action") {}
    void run(KisViewManager *view) override;
};

struct KisInvertSelectionOperation : public KisFilterSelectionOperation {
    KisInvertSelectionOperation() : KisFilterSelectionOperation("invertselection") {}
    void runFromXML(KisViewManager *view, const KisOperationConfiguration &config) override;
};

struct KRITAUI_EXPORT KisSelectionToVectorActionFactory : public KisNoParameterActionFactory {
    KisSelectionToVectorActionFactory() : KisNoParameterActionFactory("selection-to-vector") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisSelectionToRasterActionFactory : public KisNoParameterActionFactory {
    KisSelectionToRasterActionFactory() : KisNoParameterActionFactory("selection-to-raster") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisShapesToVectorSelectionActionFactory : public KisNoParameterActionFactory {
    KisShapesToVectorSelectionActionFactory() : KisNoParameterActionFactory("shapes-to-vector-selection") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisSelectionToShapeActionFactory : public KisNoParameterActionFactory {
    KisSelectionToShapeActionFactory() : KisNoParameterActionFactory("selection-to-shape-action") {}
    void run(KisViewManager *view) override;
};

struct KRITAUI_EXPORT KisStrokeSelectionActionFactory : public  KisOperation  {
    KisStrokeSelectionActionFactory()  :  KisOperation("selection-to-shape-action") {}
    void run(KisViewManager *view, const StrokeSelectionOptions& params);
};

struct KRITAUI_EXPORT KisStrokeBrushSelectionActionFactory : public  KisOperation  {
    KisStrokeBrushSelectionActionFactory()  :  KisOperation("selection-to-shape-action") {}
    void run(KisViewManager *view, const StrokeSelectionOptions& params);
};



#endif /* __KIS_SELECTION_ACTION_FACTORIES_H */
