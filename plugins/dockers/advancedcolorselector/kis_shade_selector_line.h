/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_SHADE_SELECTOR_LINE_H
#define KIS_SHADE_SELECTOR_LINE_H

#include <QWidget>
#include <KoColor.h>
#include "kis_types.h"

class KisCanvas2;
class KisShadeSelectorLineComboBox;
class KisColorSelectorBaseProxy;
class KoColorSpace;

class KisShadeSelectorLineBase : public QWidget {
public:
    KisShadeSelectorLineBase(QWidget* parent) : QWidget(parent)
    {}

    void setLineNumber(int n) {m_lineNumber=n;}
    virtual QString toString() const = 0;
    virtual void fromString(const QString& string) = 0;

protected:
    int m_lineNumber {0};
};

class KisShadeSelectorLine : public KisShadeSelectorLineBase
{
    Q_OBJECT
public:

    explicit KisShadeSelectorLine(KisColorSelectorBaseProxy *parentProxy,
                                  QWidget *parent = 0);
    explicit KisShadeSelectorLine(qreal hueDelta, qreal satDelta, qreal valDelta,
                                  KisColorSelectorBaseProxy *parentProxy, QWidget *parent = 0, qreal hueShift = 0, qreal satShift = 0, qreal valShift = 0);

    ~KisShadeSelectorLine() override;

    void setParam(qreal hue, qreal sat, qreal val, qreal hueShift, qreal satShift, qreal shiftVal);
    void setColor(const KoColor& color);
    void updateSettings();
    void setCanvas(KisCanvas2* canvas);
    void showHelpText() {m_displayHelpText=true;}
    QString toString() const override;
    void fromString(const QString& string) override;

    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    qreal m_hueDelta {0.0};
    qreal m_saturationDelta {0.0};
    qreal m_valueDelta {0.0};

    qreal m_hueShift {0.0};
    qreal m_saturationShift {0.0};
    qreal m_valueShift {0.0};

    KoColor m_realColor;
    KisPaintDeviceSP m_realPixelCache;
    const KoColorSpace *m_cachedColorSpace {0};

    bool m_gradient {false};
    int m_patchCount {0};
    int m_lineHeight {0};
    bool m_displayHelpText {false};
    qreal m_mouseX {0.0};
    QPoint m_ev;
    qreal m_width {0.0};
    bool m_isDown {false};

    friend class KisShadeSelectorLineComboBox;

    KisColorSelectorBaseProxy* m_parentProxy {0};
};

#endif // KIS_SHADE_SELECTOR_LINE_H
