/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.3
import org.krita.sketch 1.0

OpenImagePageStack {

    id: openImagePageStack
    onWidthChanged: Constants.setGridWidth( width / Constants.GridColumns );
    onHeightChanged: Constants.setGridHeight( height / Constants.GridRows );
    window: mainWindow;
}
