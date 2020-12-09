/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_splash_screen.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QPainter>
#include <QCheckBox>
#include <kis_debug.h>
#include <QFile>
#include <QScreen>
#include <QWindow>

#include <KisPart.h>
#include <KisApplication.h>

#include <kis_icon.h>

#include <klocalizedstring.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QIcon>

KisSplashScreen::KisSplashScreen(const QString &version, const QPixmap &pixmap, const QPixmap &pixmap_x2, bool themed, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint
#ifdef Q_OS_LINUX
              | Qt::WindowStaysOnTopHint
#endif
              | f),
      m_themed(themed)
{

    setupUi(this);
    setWindowIcon(KisIconUtils::loadIcon("krita"));

    QImage img;

    if (devicePixelRatioF() > 1.01) {
        img = pixmap_x2.toImage();
        img.setDevicePixelRatio(devicePixelRatioF());

        // actual size : image size (x1)
        m_scaleFactor = 2 / devicePixelRatioF();
    } else {
        img = pixmap.toImage();
        m_scaleFactor = 1;
    }

    setFixedWidth(pixmap.width() * m_scaleFactor);
    setFixedHeight(pixmap.height() * m_scaleFactor);
    lblSplash->setFixedWidth(pixmap.width() * m_scaleFactor);
    lblSplash->setFixedHeight(pixmap.height() * m_scaleFactor);

    QFont font = this->font();
    font.setPointSize(11);
    font.setBold(true);
    QFontMetrics metrics(font);

    QPainter p(&img);
    p.setFont(font);
    p.setRenderHint(QPainter::Antialiasing);

    // positioning of the text over the image (version)
    // also see setLoadingText() for positiong (loading progress text)
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    qreal leftEdge = 475 * m_scaleFactor - metrics.horizontalAdvance(version);
#else
    qreal leftEdge = 475 * m_scaleFactor - metrics.width(version);
#endif
    qreal topEdge = 58 * m_scaleFactor + metrics.ascent();

    //draw shadow
    QPen pen(QColor(0, 0, 0, 80));
    p.setPen(pen);
    p.drawText(QPointF(leftEdge+1, topEdge+1), version);
    //draw main text
    p.setPen(QPen(QColor(255, 255, 255, 255)));
    p.drawText(QPointF(leftEdge, topEdge), version);
    p.end();


    //get this to have the loading text painted on later.
    m_splashImage = img;
    m_textTop = topEdge+metrics.height();

    // Maintain the aspect ratio on high DPI screens when scaling
    lblSplash->setPixmap(QPixmap::fromImage(img));

    bnClose->hide();
    connect(bnClose, SIGNAL(clicked()), this, SLOT(close()));
    chkShowAtStartup->hide();
    connect(chkShowAtStartup, SIGNAL(toggled(bool)), this, SLOT(toggleShowAtStartup(bool)));

    KConfigGroup cfg( KSharedConfig::openConfig(), "SplashScreen");
    bool hideSplash = cfg.readEntry("HideSplashAfterStartup", false);
    chkShowAtStartup->setChecked(hideSplash);

    connect(lblRecent, SIGNAL(linkActivated(QString)), SLOT(linkClicked(QString)));
    connect(&m_timer, SIGNAL(timeout()), SLOT(raise()));

    // hide these labels by default
    displayLinks(false);
    displayRecentFiles(false);

    m_timer.setSingleShot(true);
    m_timer.start(10);
}

void KisSplashScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateText();
}

void KisSplashScreen::updateText()
{
    QString color = colorString();

    KConfigGroup cfg2( KSharedConfig::openConfig(), "RecentFiles");
    int i = 1;

    QString recent = i18n("<html>"
                          "<head/>"
                          "<body>"
                          "<p><b><span style=\" color:%1;\">Recent Files</span></b></p>", color);

    QString path;
    QStringList recentfiles;

    QFontMetrics metrics(lblRecent->font());

    do {
        path = cfg2.readPathEntry(QString("File%1").arg(i), QString());
        if (!path.isEmpty()) {
            QString name = cfg2.readPathEntry(QString("Name%1").arg(i), QString());
            QUrl url(path);
            if (name.isEmpty()) {
                name = url.fileName();
            }

            name = metrics.elidedText(name, Qt::ElideMiddle, lblRecent->width());

            if (!url.isLocalFile() || QFile::exists(url.toLocalFile())) {
                recentfiles.insert(0, QString("<p><a href=\"%1\"><span style=\"color:%3;\">%2</span></a></p>").arg(path).arg(name).arg(color));
            }
        }

        i++;
    } while (!path.isEmpty() || i <= 8);

    recent += recentfiles.join("\n");
    recent += "</body>"
        "</html>";
    lblRecent->setText(recent);
}

void KisSplashScreen::displayLinks(bool show) {

    if (show) {
        QString color = colorString();
        lblLinks->setTextFormat(Qt::RichText);
        lblLinks->setText(i18n("<html>"
                               "<head/>"
                               "<body>"
                               "<p><span style=\" color:%1;\"><b>Links</b></span></p>"

                               "<p><a href=\"https://krita.org/support-us/\"><span style=\" text-decoration: underline; color:%1;\">Support Krita</span></a></p>"

                               "<p><a href=\"https://docs.krita.org/en/user_manual/getting_started.html\"><span style=\" text-decoration: underline; color:%1;\">Getting Started</span></a></p>"
                               "<p><a href=\"https://docs.krita.org/\"><span style=\" text-decoration: underline; color:%1;\">Manual</span></a></p>"
                               "<p><a href=\"https://krita.org/\"><span style=\" text-decoration: underline; color:%1;\">Krita Website</span></a></p>"
                               "<p><a href=\"https://forum.kde.org/viewforum.php?f=136\"><span style=\" text-decoration: underline; color:%1;\">User Community</span></a></p>"

                               "<p><a href=\"https://phabricator.kde.org/source/krita/\"><span style=\" text-decoration: underline; color:%1;\">Source Code</span></a></p>"

                               "</body>"
                               "</html>", color));

        filesLayout->setContentsMargins(10,10,10,10);
        actionControlsLayout->setContentsMargins(5,5,5,5);

    } else {
        // eliminating margins here allows for the splash screen image to take the entire area with nothing underneath
        filesLayout->setContentsMargins(0,0,0,0);
        actionControlsLayout->setContentsMargins(0,0,0,0);
    }

    lblLinks->setVisible(show);

    updateText();
}


void KisSplashScreen::displayRecentFiles(bool show) {
    lblRecent->setVisible(show);
    line->setVisible(show);
}

void KisSplashScreen::setLoadingText(QString text)
{
    QFont font = this->font();
    font.setPointSize(10);
    font.setItalic(true);

    QImage img = m_splashImage;
    QPainter p(&img);
    QFontMetrics metrics(font);
    p.setFont(font);
    p.setRenderHint(QPainter::Antialiasing);

    // position text for loading text
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    qreal leftEdge = 475 *  m_scaleFactor - metrics.horizontalAdvance(text);
#else
    qreal leftEdge = 475 *  m_scaleFactor - metrics.width(text);
#endif
    qreal topEdge = m_textTop;

    //draw shadow
    QPen pen(QColor(0, 0, 0, 80));
    p.setPen(pen);
    p.drawText(QPointF(leftEdge+1, topEdge+1), text);
    //draw main text
    p.setPen(QPen(QColor(255, 255, 255, 255)));
    p.drawText(QPointF(leftEdge, topEdge), text);
    p.end();
    lblSplash->setPixmap(QPixmap::fromImage(img));
}



QString KisSplashScreen::colorString() const
{
    QString color = "#FFFFFF";
    if (m_themed && qApp->palette().window().color().value() > 100) {
        color = "#000000";
    }

    return color;
}


void KisSplashScreen::repaint()
{
    QWidget::repaint();
    qApp->sendPostedEvents();
}

void KisSplashScreen::show()
{
    QRect r(QPoint(), sizeHint());
    resize(r.size());
    if (!this->parentWidget()) {
        this->winId(); // Force creation of native window
        if (this->windowHandle()) {
            // At least on Windows, the window may be created on a non-primary
            // screen with a different scale factor. If we don't explicitly
            // move it to the primary screen, the position will be scaled with
            // the wrong factor and the splash will be offset.
            this->windowHandle()->setScreen(QApplication::primaryScreen());
        }
    }
    move(QApplication::primaryScreen()->availableGeometry().center() - r.center());
    if (isVisible()) {
        repaint();
    }
    m_timer.setSingleShot(true);
    m_timer.start(1);
    QWidget::show();
}

void KisSplashScreen::toggleShowAtStartup(bool toggle)
{
    KConfigGroup cfg( KSharedConfig::openConfig(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", toggle);
}

void KisSplashScreen::linkClicked(const QString &link)
{
    KisPart::instance()->openExistingFile(QUrl::fromLocalFile(link));
    if (isTopLevel()) {
        close();
    }
}
