/* ============================================================
 * Author: Tom Albers <tomalbers@kde.nl>
 * Date  : 2005-01-01
 * Description :
 *
 * SPDX-FileCopyrightText: 2005 Tom Albers <tomalbers@kde.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "KisSqueezedComboBox.h"
/** @file KisSqueezedComboBox.cpp */

// Qt includes.

#include <QComboBox>
#include <QPair>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QResizeEvent>

KisSqueezedComboBox::KisSqueezedComboBox(QWidget *parent, const char *name)
        : QComboBox(parent)
{
    setObjectName(name);
    setMinimumWidth(100);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
}

KisSqueezedComboBox::~KisSqueezedComboBox()
{
    delete m_timer;
}

bool KisSqueezedComboBox::contains(const QString& _text) const
{
    if (_text.isEmpty())
        return false;

    for (QMap<int, QString>::const_iterator it = m_originalItems.begin() ; it != m_originalItems.end();
            ++it) {
        if (it.value() == _text) {
            return true;
        }
    }
    return false;
}

qint32 KisSqueezedComboBox::findOriginalText(const QString& text) const
{
    for (int i = 0; i < m_originalItems.size(); i++) {
        if(m_originalItems.value(i) == text) {
            return i;
        }
    }
    return -1;
}

QStringList KisSqueezedComboBox::originalTexts() const
{
    return m_originalItems.values();
}

void KisSqueezedComboBox::resetOriginalTexts(const QStringList &texts)
{
    if (texts == m_originalItems.values()) return;

    clear();
    m_originalItems.clear();

    Q_FOREACH (const QString &item, texts) {
        addSqueezedItem(item);
    }
}

QSize KisSqueezedComboBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    int maxW = count() ? 18 : 7 * fm.boundingRect(QChar('x')).width() + 18;
    int maxH = qMax(fm.lineSpacing(), 14) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options, QSize(maxW, maxH), this);
}

void KisSqueezedComboBox::insertSqueezedItem(const QString& newItem, int index, QVariant userData)
{
    m_originalItems[index] = newItem;
    QComboBox::insertItem(index, squeezeText(newItem, this), userData);
    setItemData(index, newItem, Qt::ToolTipRole);
}

void KisSqueezedComboBox::insertSqueezedItem(const QIcon &icon, const QString &newItem, int index, QVariant userData)
{
    m_originalItems[index] = newItem;
    QComboBox::insertItem(index, icon, squeezeText(newItem, this), userData);
    setItemData(index, newItem, Qt::ToolTipRole);
}

void KisSqueezedComboBox::addSqueezedItem(const QString& newItem, QVariant userData)
{
    insertSqueezedItem(newItem, count(), userData);
}

void KisSqueezedComboBox::addSqueezedItem(const QIcon &icon, const QString &newItem, QVariant userData)
{
    insertSqueezedItem(icon, newItem, count(), userData);
}

void KisSqueezedComboBox::setCurrent(const QString& itemText)
{
    qint32 itemIndex = findOriginalText(itemText);
    if (itemIndex >= 0) {
        setCurrentIndex(itemIndex);
    }
}

void KisSqueezedComboBox::resizeEvent(QResizeEvent *)
{
    m_timer->start(200);
}

void KisSqueezedComboBox::slotTimeOut()
{
    for (QMap<int, QString>::iterator it = m_originalItems.begin() ; it != m_originalItems.end();
            ++it) {
        setItemText(it.key(), squeezeText(it.value(), this));
    }
}

QString KisSqueezedComboBox::squeezeText(const QString& original, const QWidget *widget)
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = widget->width() - 30;
    QFontMetrics fm(widget->fontMetrics());

    // If we can fit the full text, return that.
    if (fm.boundingRect(original).width() < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize = widgetSize - fm.boundingRect("...").width();
    for (int i = 0 ; i != original.length(); ++i) {
        if ((int)fm.boundingRect(original.right(i)).width() > widgetSize) {
            sqItem = QString("..." + original.right(--i));
            break;
        }
    }
    return sqItem;
}

QString KisSqueezedComboBox::currentUnsqueezedText()
{
    int curItem = currentIndex();
    return m_originalItems[curItem];
}

void KisSqueezedComboBox::removeSqueezedItem(int index)
{
    removeItem(index);
    m_originalItems.remove(index);
}

