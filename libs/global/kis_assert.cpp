/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_assert.h"

#include <QString>
#include <QMessageBox>
#include <QThread>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QApplication>

#include <klocalizedstring.h>
#include <kis_assert_exception.h>
#include <KisUsageLogger.h>
#include <string>
#include "config-safe-asserts.h"

/**
 * TODO: Add automatic saving of the documents
 *
 * Requirements:
 * 1) Should save all open KisDocument objects
 * 2) Should *not* overwrite original document since the saving
 *    process may fail.
 * 3) Should *not* overwrite any autosaved documents since the saving
 *    process may fail.
 * 4) Double-fault tolerance! Assert during emergency saving should not
 *    lead to an infinite loop.
 */

void kis_assert_common(const char *assertion, const char *file, int line, bool throwException, bool isIgnorable)
{
    QString shortMessage =
        QString("%4ASSERT (krita): \"%1\" in file %2, line %3")
        .arg(assertion)
        .arg(file)
        .arg(line)
        .arg(isIgnorable ? "SAFE " : "");

    QString longMessage =
        QString(
            "Krita has encountered an internal error:\n\n"
            "%1\n\n"
            "Please report a bug to developers!\n\n"
            "Press Ignore to try to continue.\n"
            "Press Abort to see developers information (all unsaved data will be lost)")
        .arg(shortMessage);

    KisUsageLogger::log(shortMessage);

    bool disableAssertMsg =
        QProcessEnvironment::systemEnvironment().value("KRITA_NO_ASSERT_MSG", "0").toInt();

    // disable message box if the assert happened in non-gui thread :(
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        disableAssertMsg = true;
    }

    bool shouldIgnoreAsserts = false;
    bool forceCrashOnSafeAsserts = false;

#ifdef HIDE_SAFE_ASSERTS
    shouldIgnoreAsserts |= HIDE_SAFE_ASSERTS;
#endif

#ifdef CRASH_ON_SAFE_ASSERTS
    forceCrashOnSafeAsserts |= CRASH_ON_SAFE_ASSERTS;
#endif

    disableAssertMsg |= shouldIgnoreAsserts || forceCrashOnSafeAsserts;

    QMessageBox::StandardButton button =
        isIgnorable && !forceCrashOnSafeAsserts ?
            QMessageBox::Ignore : QMessageBox::Abort;

    if (!disableAssertMsg) {
        button =
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita: Internal Error"),
                                  longMessage,
                                  QMessageBox::Ignore | QMessageBox::Abort,
                                  QMessageBox::Ignore);
    }

    if (button == QMessageBox::Abort) {
        qFatal("%s", shortMessage.toLatin1().data());
    } else if (isIgnorable) {
        // Assert is a bug! Please don't change this line to warnKrita,
        // the user must see it!
        qWarning("%s", shortMessage.toLatin1().data());
    } else if (throwException) {
        throw KisAssertException(shortMessage.toLatin1().data());
    }
}


void kis_assert_recoverable(const char *assertion, const char *file, int line)
{
    kis_assert_common(assertion, file, line, false, false);
}

void kis_safe_assert_recoverable(const char *assertion, const char *file, int line)
{
    kis_assert_common(assertion, file, line, false, true);
}

void kis_assert_exception(const char *assertion, const char *file, int line)
{
    kis_assert_common(assertion, file, line, true, false);
}

void kis_assert_x_exception(const char *assertion,
                            const char *where,
                            const char *what,
                            const char *file, int line)
{
    QString res =
        QString("ASSERT failure in %1: \"%2\" (%3)")
        .arg(where, what, assertion);

    kis_assert_common(res.toLatin1().data(), file, line, true, false);
}
