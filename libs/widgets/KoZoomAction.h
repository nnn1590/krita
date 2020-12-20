/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Ariya Hidayat <ariya@kde.org>
    SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KOZOOMACTION_H
#define KOZOOMACTION_H

#include <kselectaction.h>
#include <KoZoomMode.h>

#include "kritawidgets_export.h"

/**
 * Class KoZoomAction implements an action to provide zoom values.
 * In a toolbar, KoZoomAction will show a dropdown list (combobox), also with
 * the possibility for the user to enter arbitrary zoom value
 * (must be an integer). The values shown on the list are always
 * sorted.
 * In a statusbar it provides a scale (slider) plus an editable value plus
 * some buttons for special zoommodes
 */
class KRITAWIDGETS_EXPORT KoZoomAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY(qreal effectiveZoom READ effectiveZoom NOTIFY zoomChanged)
public:

  /**
   * Creates a new zoom action.
   * @param zoomModes which zoom modes that should be shown
   * @param text The text that will be displayed.
   * @param parent The action's parent object.
   */
    KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, QObject *parent);
    ~KoZoomAction() override;

    /**
     * Reimplemented from QWidgetAction.
     */
    QWidget* createWidget(QWidget* parent) override;


    qreal effectiveZoom() const;

public Q_SLOTS:

    /**
     * Sets the zoom. If zoom not yet on the list of zoom values, it will be inserted
     * into the list at proper place so that the values remain sorted.
     * emits zoomChanged
     */
    void setZoom( qreal zoom );

  /**
   * Change the zoom to a closer look than current
   * Zoom mode will be CONSTANT afterwards
   * emits zoomChanged
   */
    void zoomIn( );

  /**
   * Change the zoom to a wider look than current
   * Zoom mode will be CONSTANT afterwards
   * emits zoomChanged
   */
    void zoomOut( );

  /**
   * Set the actual zoom value used in the app. This is needed when using @ref zoomIn() , @ref zoomOut() and/or when
   * plugged into the viewbar.
   */
    void setEffectiveZoom(qreal zoom);

  /**
   * Change the selected zoom mode.
   */
    void setSelectedZoomMode( KoZoomMode::Mode mode );

  /**
   * Change status of "Use same aspect as pixels" button
   * (emits aspectModeChanged(bool) after the change, ALWAYS)
   */
    void setAspectMode(bool status);

    /**
     * Returns next preferred zoom level that should be used for
     * zoom in operations.
     *
     * This can be used by the caller, when it needs some special
     * mode of zooming (e.g. relative to point) and needs
     * KoCanvasControllerWidget to accomplish this.
     */
    qreal nextZoomLevel() const;

    /**
     * Returns previous preferred zoom level that should be used for
     * zoom out operations.
     *
     * This can be used by the caller, when it needs some special
     * mode of zooming (e.g. relative to point) and needs
     * KoCanvasControllerWidget to accomplish this.
     */
    qreal prevZoomLevel() const;

protected Q_SLOTS:

    void triggered( const QString& text );
    void sliderValueChanged(int value);
    void slotUpdateGuiAfterZoom();

Q_SIGNALS:

  /**
   * Signal zoomChanged is triggered when user changes the zoom value, either by
   * choosing it from the list or by entering new value.
   * @param mode The selected zoom mode
   * @param zoom the zoom, only defined if @p mode is KoZoomMode::ZOOM_CONSTANT
   */
    void zoomChanged( KoZoomMode::Mode mode, qreal zoom );

  /**
   * Signal aspectModeChanged is triggered when the user toggles the widget.
   * Nothing else happens except that this signal is emitted.
   * @param status Whether the special aspect mode is on
   */
    void aspectModeChanged( bool status );

    /**
     * Signal is triggered when the user clicks the zoom to selection button.
     * Nothing else happens except that this signal is emitted.
     */
    void zoomedToSelection();

    /**
     * Signal is triggered when the user clicks the zoom to all button.
     * Nothing else happens except that this signal is emitted.
     */
    void zoomedToAll();

    void zoomLevelsChanged(const QStringList &values);
    void currentZoomLevelChanged(const QString &valueString);
    void sliderChanged(int value);

public:
    /**
     * Return the minimum zoom possible for documents.
     *
     * \return The minimum zoom possible.
     */
    qreal minimumZoom();
    /**
     * Return the maximum zoom possible for documents.
     *
     * \return The maximum zoom possible.
     */
    qreal maximumZoom();
    /**
     * Clamp the zoom value so that mimimumZoom <= zoom <= maximumZoom.
     *
     * \param zoom The value to clamp.
     *
     * \return minimumZoom if zoom < minimumZoom, maximumZoom if zoom >
     * maximumZoom, zoom if otherwise.
     */
    qreal clampZoom(qreal zoom);

    /**
     * Set the minimum zoom possible for documents.
     *
     * Note that after calling this, any existing KoZoomAction instances
     * should be recreated.
     *
     * \param zoom The minimum zoom to use.
     */
    void setMinimumZoom(qreal zoom);
    /**
     * Set the maximum zoom possible for documents.
     *
     * Note that after calling this, any existing KoZoomAction instances
     * should be recreated.
     *
     * \param zoom The maximum zoom to use.
     */
    void setMaximumZoom(qreal zoom);

protected:
    /// Regenerates the action's items
    void regenerateItems( const qreal zoom);

private:
    void syncSliderWithZoom();

    Q_DISABLE_COPY( KoZoomAction )

    class Private;
    Private * const d;
};

#endif
