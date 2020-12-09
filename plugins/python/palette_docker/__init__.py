#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from krita import DockWidgetFactory, DockWidgetFactoryBase
from .palette_docker import PaletteDocker


Application.addDockWidgetFactory(
    DockWidgetFactory("palette_docker", DockWidgetFactoryBase.DockRight,
                      PaletteDocker))
