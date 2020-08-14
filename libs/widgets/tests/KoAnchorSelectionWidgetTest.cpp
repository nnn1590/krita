/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoAnchorSelectionWidgetTest.h"

#include <QTest>
#include <QDialog>

#include <QVBoxLayout>

#include "kis_debug.h"


void KoAnchorSelectionWidgetTest::test()
{
    QDialog dlg;

    KoAnchorSelectionWidget *widget = new KoAnchorSelectionWidget(&dlg);
    connect(widget,
            SIGNAL(valueChanged(KoFlake::AnchorPosition)),
            SLOT(slotValueChanged(KoFlake::AnchorPosition)));

    QVBoxLayout *layout = new QVBoxLayout(&dlg);
    layout->addWidget(widget);
    dlg.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    //dlg.exec();
    qWarning() << "WARNING: showing of the dialogs in the unittest is disabled!";
}

void KoAnchorSelectionWidgetTest::slotValueChanged(KoFlake::AnchorPosition id)
{
    ENTER_FUNCTION() << ppVar(id);
}

QTEST_MAIN(KoAnchorSelectionWidgetTest)
