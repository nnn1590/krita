/* This file is part of the KDE project
 * Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_TOOL_OPTIONS_POPUP_H
#define KIS_TOOL_OPTIONS_POPUP_H

#include <QWidget>
#include <QPointer>

class KisToolOptionsPopup : public QWidget
{
    Q_OBJECT
public:
    explicit KisToolOptionsPopup(QWidget *parent = 0);
    ~KisToolOptionsPopup() override;

    bool detached() const;

    void newOptionWidgets(const QList<QPointer<QWidget> > &optionWidgetList);

Q_SIGNALS:

public Q_SLOTS:

    void switchDetached(bool show = true);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

private:

    struct Private;
    Private *const d;
};

#endif // KIS_TOOL_OPTIONS_POPUP_H
