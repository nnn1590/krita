/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_file_name_requester.h"
#include "ui_wdg_file_name_requester.h"

#include <QStandardPaths>
#include <QDebug>

#include "KoIcon.h"
#include <KisFileUtils.h>

KisFileNameRequester::KisFileNameRequester(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::WdgFileNameRequester)
    , m_mode(KoFileDialog::OpenFile)
    , m_name("OpenDocument")
{
    m_ui->setupUi(this);

    m_ui->btnSelectFile->setIcon(kisIcon("folder"));

    connect(m_ui->btnSelectFile, SIGNAL(clicked()), SLOT(slotSelectFile()));
    connect(m_ui->txtFileName, SIGNAL(textChanged(QString)), SIGNAL(textChanged(QString)));
}

KisFileNameRequester::~KisFileNameRequester()
{
}

void KisFileNameRequester::setStartDir(const QString &path)
{
    m_basePath = path;
}

void KisFileNameRequester::setConfigurationName(const QString &name)
{
    m_name = name;
}

void KisFileNameRequester::setFileName(const QString &path)
{
    m_ui->txtFileName->setText(path);
    emit fileSelected(path);
}

QString KisFileNameRequester::fileName() const
{
    return m_ui->txtFileName->text();
}

void KisFileNameRequester::setMode(KoFileDialog::DialogType mode)
{
    m_mode = mode;
}

KoFileDialog::DialogType KisFileNameRequester::mode() const
{
    return m_mode;
}

void KisFileNameRequester::setMimeTypeFilters(const QStringList &filterList, QString defaultFilter)
{
    m_mime_filter_list = filterList;
    m_mime_default_filter = defaultFilter;
}

void KisFileNameRequester::slotSelectFile()
{
    KoFileDialog dialog(this, m_mode, m_name);
    if (m_mode == KoFileDialog::OpenFile)
    {
        dialog.setCaption(i18n("Select a file to load..."));
    }
    else if (m_mode == KoFileDialog::OpenDirectory)
    {
        dialog.setCaption(i18n("Select a directory to load..."));
    }

    const QString basePath =
        KritaUtils::resolveAbsoluteFilePath(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                                            m_basePath);

    const QString filePath =
        KritaUtils::resolveAbsoluteFilePath(basePath, m_ui->txtFileName->text());

    dialog.setDefaultDir(filePath, true);
    dialog.setMimeTypeFilters(m_mime_filter_list, m_mime_default_filter);

    QString newFileName = dialog.filename();

    if (!newFileName.isEmpty()) {
        setFileName(newFileName);
    }
}
