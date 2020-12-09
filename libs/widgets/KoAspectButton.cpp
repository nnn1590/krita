/* This file is part of the KDE project
 * Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoAspectButton.h"

#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

namespace {
    /* XPM -- copyright The Gimp */
    const char * const _chain_broken_24[] = {
        /* columns rows colors chars-per-pixel */
        "9 24 10 1",
        "  c black",
        ". c #020204",
        "X c #5A5A5C",
        "o c gray43",
        "O c #8F8F91",
        "+ c #9A9A98",
        "@ c #B5B5B6",
        "# c #D0D0D1",
        "$ c #E8E8E9",
        "% c None",
        /* pixels */
        "%%.....%%",
        "%.o##@X.%",
        "%.+...$.%",
        "%.#.%.#.%",
        "%.#.%.#.%",
        "%.@.%.#.%",
        "%.+...#.%",
        "%.O.o.O.%",
        "%%..@..%%",
        "%%%.#.%%%",
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%%.#.%%%",
        "%%..#..%%",
        "%.o.@.O.%",
        "%.@...@.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.#...$.%",
        "%.o$#$@.%",
        "%%.....%%"
    };

    /* XPM  -- copyright The Gimp */
    const char * const _chain_24[] = {
        /* columns rows colors chars-per-pixel */
        "9 24 10 1",
        "  c black",
        ". c #020204",
        "X c #5A5A5C",
        "o c gray43",
        "O c #8F8F91",
        "+ c #9A9A98",
        "@ c #B5B5B6",
        "# c #D0D0D1",
        "$ c #E8E8E9",
        "% c None",
        /* pixels */
        "%%%%%%%%%",
        "%%%%%%%%%",
        "%%.....%%",
        "%.o##@X.%",
        "%.+...$.%",
        "%.#.%.#.%",
        "%.#.%.#.%",
        "%.@.%.#.%",
        "%.+...#.%",
        "%.O.o.O.%",
        "%%..@..%%",
        "%%%.#.%%%",
        "%%%.#.%%%",
        "%%..#..%%",
        "%.o.@.O.%",
        "%.@...@.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.@.%.$.%",
        "%.#...$.%",
        "%.o$#$@.%",
        "%%.....%%",
        "%%%%%%%%%",
        "%%%%%%%%%"
    };
}

class Q_DECL_HIDDEN KoAspectButton::Private
{
public:
    Private()
        : chain(_chain_24),
        brokenChain(_chain_broken_24),
        keepAspect(true)
    {
    }
    const QPixmap chain, brokenChain;
    bool keepAspect;
};

KoAspectButton::KoAspectButton(QWidget *parent)
    : QAbstractButton(parent),
    d( new Private() )
{
    //setPixmap(d->chain);
    //setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

KoAspectButton::~KoAspectButton()
{
    delete d;
}

void KoAspectButton::mouseReleaseEvent (QMouseEvent *ev) {
    if(! isEnabled() || ev->button() != Qt::LeftButton)
        return;
    setKeepAspectRatio(!d->keepAspect);
}

void KoAspectButton::setKeepAspectRatio(bool on) {
    if(d->keepAspect == on)
        return;
    d->keepAspect = on;
    update();
    emit keepAspectRatioChanged(d->keepAspect);
}

void KoAspectButton::paintEvent (QPaintEvent *) {
    QPainter painter(this);
    painter.drawPixmap(0, (height() - 24) / 2, 9, 24, d->keepAspect ? d->chain : d->brokenChain, 0, 0, 9, 24);
    painter.end();
}

QSize KoAspectButton::sizeHint () const {
    return QSize(9, 24);
}

void KoAspectButton::keyReleaseEvent (QKeyEvent *e) {
    if(e->text() == " ") {
        setKeepAspectRatio(! d->keepAspect);
        e->accept();
    }
}

bool KoAspectButton::keepAspectRatio() const
{
    return d->keepAspect;
}
