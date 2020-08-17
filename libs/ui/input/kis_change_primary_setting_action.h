/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#ifndef KISCHANGEPRIMARYSETTINGACTION_H
#define KISCHANGEPRIMARYSETTINGACTION_H

#include "kis_abstract_input_action.h"
#include "kis_tool.h"

/**
 * \brief Change Primary Setting implementation of KisAbstractInputAction.
 *
 * The Change Primary Setting action changes a tool's "Primary Setting",
 * for example the brush size for the brush tool.
 */
class KisChangePrimarySettingAction : public KisAbstractInputAction
{
public:
    explicit KisChangePrimarySettingAction();
    ~KisChangePrimarySettingAction() override;

    enum Shortcut {
        PrimaryAlternateChangeSizeShortcut, ///< Default Mapping: Shift+Left Mouse
        SecondaryAlternateChangeSizeShortcut, ///< Secondary Mode (snap to closest pixel value): Shift+Z+Left Mouse
    };

    void activate(int shortcut) override;
    void deactivate(int shortcut) override;
    int priority() const override;

    void begin(int shortcut, QEvent *event) override;
    void end(QEvent *event) override;
    void inputEvent(QEvent* event) override;
private:
    KisTool::ToolAction decodeAction(int shorcut);
    KisTool::ToolAction savedAction;
};

#endif // KISCHANGEPRIMARYSETTINGACTION_H
