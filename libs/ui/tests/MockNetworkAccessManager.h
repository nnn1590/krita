/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
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
 * #
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MOCKNETWORKACCESSMANAGER_H
#define MOCKNETWORKACCESSMANAGER_H

#include <QObject>
#include <QNetworkReply>
#include <QBuffer>
#include <QNetworkRequest>

#include <KisNetworkAccessManager.h>

#include "kritaui_export.h"


struct FakeReplyData {
    QUrl url;
    QNetworkAccessManager::Operation requestMethod;
    int statusCode;
    QString contentType;
    QByteArray responseBody;
};
Q_DECLARE_METATYPE(FakeReplyData);

class FakeNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    explicit FakeNetworkReply(const QNetworkRequest &originalRequest, FakeReplyData& replyData);

    void abort() override;
    bool atEnd() const override;
    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    void close() override;
    qint64 size() const override;
    qint64 pos() const override;

protected:
    qint64 readData(char *data, qint64 maxLen) override;
    qint64 writeData(const char *data, qint64 maxLen) override;

private:
    QBuffer m_data;
};

class MockNetworkAccessManager : public KisNetworkAccessManager
{
    Q_OBJECT

public:
    explicit MockNetworkAccessManager(QObject *parent = 0);

    void setReplyData(FakeReplyData& replyData);

    QNetworkReply* createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

private:
    FakeReplyData m_replyData;
};

#endif // MOCKNETWORKACCESSMANAGER_H
