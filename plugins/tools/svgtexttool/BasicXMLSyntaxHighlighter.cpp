/*
 * SPDX-FileCopyrightText: 2015 Dmitry Ivanov
 *
 * SPDX-License-Identifier: MIT
 */
#include "BasicXMLSyntaxHighlighter.h"

#include <QApplication>
#include <QPalette>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QObject * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QTextDocument * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QTextEdit * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

void BasicXMLSyntaxHighlighter::highlightBlock(const QString & text)
{
    // Special treatment for xml element regex as we use captured text to emulate lookbehind
    int xmlElementIndex = m_xmlElementRegex.indexIn(text);
    while(xmlElementIndex >= 0)
    {
        int matchedPos = m_xmlElementRegex.pos(1);
        int matchedLength = m_xmlElementRegex.cap(1).length();
        setFormat(matchedPos, matchedLength, m_xmlElementFormat);

        xmlElementIndex = m_xmlElementRegex.indexIn(text, matchedPos + matchedLength);
    }

    // Highlight xml keywords *after* xml elements to fix any occasional / captured into the enclosing element
    typedef QList<QRegExp>::const_iterator Iter;
    Iter xmlKeywordRegexesEnd = m_xmlKeywordRegexes.constEnd();
    for(Iter it = m_xmlKeywordRegexes.constBegin(); it != xmlKeywordRegexesEnd; ++it) {
        const QRegExp & regex = *it;
        highlightByRegex(m_xmlKeywordFormat, regex, text);
    }

    highlightByRegex(m_xmlAttributeFormat, m_xmlAttributeRegex, text);
    highlightByRegex(m_xmlCommentFormat, m_xmlCommentRegex, text);
    highlightByRegex(m_xmlValueFormat, m_xmlValueRegex, text);
}

void BasicXMLSyntaxHighlighter::highlightByRegex(const QTextCharFormat & format,
                                                 const QRegExp & regex, const QString & text)
{
    int index = regex.indexIn(text);

    while(index >= 0)
    {
        int matchedLength = regex.matchedLength();
        setFormat(index, matchedLength, format);

        index = regex.indexIn(text, index + matchedLength);
    }
}

void BasicXMLSyntaxHighlighter::setRegexes()
{
    m_xmlElementRegex.setPattern("<[\\s]*[/]?[\\s]*([^\\n]\\w*)(?=[\\s/>])");
    m_xmlAttributeRegex.setPattern("[\\w\\-]+(?=\\=)");
    m_xmlValueRegex.setPattern("\"[^\\n\"]+\"(?=[\\s/>])");
    m_xmlCommentRegex.setPattern("<!--[^\\n]*-->");

    m_xmlKeywordRegexes = QList<QRegExp>() << QRegExp("<\\?") << QRegExp("/>")
                                           << QRegExp(">") << QRegExp("<") << QRegExp("</")
                                           << QRegExp("\\?>");
}

void BasicXMLSyntaxHighlighter::setFormats()
{
    KConfigGroup cfg(KSharedConfig::openConfig(), "SvgTextTool");
    QColor background = cfg.readEntry("colorEditorBackground", qApp->palette().window().color());

    m_xmlKeywordFormat.setForeground(cfg.readEntry("colorKeyword", QColor(background.value() < 100 ? Qt::cyan : Qt::blue)));
    m_xmlKeywordFormat.setFontWeight(cfg.readEntry("BoldKeyword", true) ? QFont::Bold : QFont::Normal);
    m_xmlKeywordFormat.setFontItalic((cfg.readEntry("ItalicKeyword", false)));

    m_xmlElementFormat.setForeground(cfg.readEntry("colorElement", QColor(background.value() < 100 ? Qt::magenta : Qt::darkMagenta)));
    m_xmlElementFormat.setFontWeight(cfg.readEntry("BoldElement", true) ? QFont::Bold : QFont::Normal);
    m_xmlElementFormat.setFontItalic((cfg.readEntry("ItalicElement", false)));

    m_xmlAttributeFormat.setForeground(cfg.readEntry("colorAttribute", QColor(background.value() < 100 ? Qt::green : Qt::darkGreen)));
    m_xmlAttributeFormat.setFontWeight(cfg.readEntry("BoldAttribute", true) ? QFont::Bold : QFont::Normal);
    m_xmlAttributeFormat.setFontItalic((cfg.readEntry("ItalicAttribute", true)));

    m_xmlValueFormat.setForeground(cfg.readEntry("colorValue", QColor(background.value() < 100 ? Qt::red: Qt::darkRed)));
    m_xmlValueFormat.setFontWeight(cfg.readEntry("BoldValue", true) ? QFont::Bold : QFont::Normal);
    m_xmlValueFormat.setFontItalic(cfg.readEntry("ItalicValue", false));

    m_xmlCommentFormat.setForeground(cfg.readEntry("colorComment", QColor(background.value() < 100 ? Qt::lightGray : Qt::gray)));
    m_xmlCommentFormat.setFontWeight(cfg.readEntry("BoldComment", false) ? QFont::Bold : QFont::Normal);
    m_xmlCommentFormat.setFontItalic(cfg.readEntry("ItalicComment", false));

}

