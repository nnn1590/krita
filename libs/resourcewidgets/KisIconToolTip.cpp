/* This file is part of the KDE project
 * Copyright (c) 1999 Carsten Pfeiffer (pfeiffer@kde.org)
 * Copyright (c) 2002 Igor Jansen (rm@kde.org)
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KisIconToolTip.h"

#include <QTextDocument>
#include <QUrl>
#include <QModelIndex>

#include <KisResourceModel.h>

#include <klocalizedstring.h>

KisIconToolTip::KisIconToolTip()
{
}

KisIconToolTip::~KisIconToolTip()
{
}

QTextDocument *KisIconToolTip::createDocument(const QModelIndex &index)
{
    QTextDocument *doc = new QTextDocument(this);

    QImage thumb = index.data(Qt::DecorationRole).value<QImage>();
    if (thumb.isNull()) {
        thumb = index.data(Qt::UserRole + KisResourceModel::Thumbnail).value<QImage>();
    }
    doc->addResource(QTextDocument::ImageResource, QUrl("data:thumbnail"), thumb);

    QString name = index.data(Qt::DisplayRole).toString();
    QString presetDisplayName = index.data(Qt::UserRole + KisResourceModel::Name).toString().replace("_", " ");
    //This is to ensure that the other uses of this class don't get an empty string, while resource management should get a nice string.
    if (!presetDisplayName.isEmpty()) {
        name = presetDisplayName;
    }

    QString tags;
    QString tagsData = index.data(Qt::UserRole + KisResourceModel::Tags).toStringList().join(", ");
    if (tagsData.length() > 0) {
        const QString list = QString("<ul style=\"list-style-type: none; margin: 0px;\">%1</ul> ").arg(tagsData);
        tags = QString("<p><table><tr><td>%1:</td><td>%2</td></tr></table></p>").arg(i18n("Tags"), list);
    }

    const QString image = QString("<center><img src=\"data:thumbnail\"></center>");
    const QString body = QString("<h3 align=\"center\">%1</h3>%2%3").arg(name, image, tags);
    const QString html = QString("<html><body>%1</body></html>").arg(body);

    doc->setHtml(html);

    const int margin = 16;
    doc->setTextWidth(qMin(doc->size().width() + 2 * margin, qreal(500.0)));
    doc->setDocumentMargin(margin);
    doc->setUseDesignMetrics(true);

    return doc;
}
