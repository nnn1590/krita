/*
 *  kis_tool_select_rectangular.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 1999 Michael Koch <koch@kde.org>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_SELECT_RECTANGULAR_H_
#define KIS_TOOL_SELECT_RECTANGULAR_H_

#include "KisSelectionToolFactoryBase.h"
#include "kis_tool_rectangle_base.h"
#include <kis_tool_select_base.h>
#include "kis_selection_tool_config_widget_helper.h"
#include <kis_icon.h>
#include <QKeySequence>


class __KisToolSelectRectangularLocal : public KisToolRectangleBase
{
    Q_OBJECT

public:
    __KisToolSelectRectangularLocal(KoCanvasBase * canvas);

    bool hasUserInteractionRunning() const;

protected:
    virtual SelectionMode selectionMode() const = 0;
    virtual SelectionAction selectionAction() const = 0;
    virtual bool antiAliasSelection() const = 0;

private:
    void finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY) override;
};


class KisToolSelectRectangular : public KisToolSelectBase<__KisToolSelectRectangularLocal>
{
    Q_OBJECT
public:
    KisToolSelectRectangular(KoCanvasBase* canvas);

    void resetCursorStyle() override;
};

class KisToolSelectRectangularFactory : public KisSelectionToolFactoryBase
{

public:
    KisToolSelectRectangularFactory()
        : KisSelectionToolFactoryBase("KisToolSelectRectangular")
    {
        setToolTip(i18n("Rectangular Selection Tool"));
        setSection(TOOL_TYPE_SELECTION);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("tool_rect_selection"));
        setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
        setPriority(0);
    }

    ~KisToolSelectRectangularFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolSelectRectangular(canvas);
    }
};



#endif // KIS_TOOL_SELECT_RECTANGULAR_H_

