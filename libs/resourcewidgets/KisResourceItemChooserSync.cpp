/* This file is part of the KDE project

   Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KisResourceItemChooserSync.h"

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisResourceItemChooserSync, s_instance)

struct Q_DECL_HIDDEN KisResourceItemChooserSync::Private
{
    int baseLength;
};


KisResourceItemChooserSync::KisResourceItemChooserSync()
    : d(new Private)
{
    d->baseLength = 50;
}

KisResourceItemChooserSync::~KisResourceItemChooserSync()
{
}

KisResourceItemChooserSync* KisResourceItemChooserSync::instance()
{
    return s_instance;
}

int KisResourceItemChooserSync::baseLength()
{
    return d->baseLength;
}

void KisResourceItemChooserSync::setBaseLength(int length)
{
    d->baseLength = qBound(25, length, 100);
    emit baseLengthChanged(d->baseLength);
}







