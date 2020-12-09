/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCANVASOBSERVERBASE_H
#define KOCANVASOBSERVERBASE_H

class KoCanvasBase;
class KoCanvasObserverBasePrivate;

#include "kritaflake_export.h"
#include <QString>
#include <QMainWindow>

/**
 * An abstract canvas observer interface class.
 * Dockers that want to be notified of active canvas changes
 * should implement that interface so that the tool controller
 * can give them the active canvas.
 */
class KRITAFLAKE_EXPORT KoCanvasObserverBase
{
public:
    KoCanvasObserverBase();
    virtual ~KoCanvasObserverBase();

    virtual QString observerName() { return ""; }

    /**
     * set observed canvas
     * @param canvas canvas to observe. Can be 0.
     */
    void setObservedCanvas(KoCanvasBase *canvas);

    /**
     * notify the observer that canvas is gone
     */
    void unsetObservedCanvas();

    /**
     * the currently observed canvas
     * @return observed canvas, can be 0
     */
    KoCanvasBase* observedCanvas() const;

protected:
    /**
     * re-implement this method in your canvas observer. It will be called
     * whenever a canvas becomes active. Note that you are responsible for
     * not connecting more than one time to the signals of a canvas or any
     * of the QObjects you can access through the canvas.
     */
    virtual void setCanvas(KoCanvasBase *canvas) = 0;

    /**
     * Re-implement to notify the observer that its canvas is no longer
     * among the living. The daisies, it is pushing up. This means you
     * don't have to unconnect, it's dead.
     *
     * The old canvas should be deleted already, so if you stored a
     * pointer to it, don't touch!
     *
     * Note that currently there is a bug where in certain specific
     * circumstances unsetCanvas can be called when it shouldn't, see for
     * example KWStatisticsDocker for a workaround for this problem.
     */
    virtual void unsetCanvas() = 0;

private:
    KoCanvasObserverBasePrivate * const d;
};

#endif // KOCANVASOBSERVERBASE_H
