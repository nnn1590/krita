/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <QDebug>
#include <QMap>
#include <QIODevice>

#include "kritastore_export.h"

/**
 * A class for writing out XML (to any QIODevice), with a special attention on performance.
 * The XML is being written out along the way, which avoids requiring the entire
 * document in memory (like QDom does).
 */
class KRITASTORE_EXPORT KoXmlWriter
{
public:
    /**
     * Create a KoXmlWriter instance to write out an XML document into
     * the given QIODevice.
     */
    explicit KoXmlWriter(QIODevice* dev, int indentLevel = 0);

    /// Destructor
    ~KoXmlWriter();

    /**
     * Start the XML document.
     * This writes out the \<?xml?\> tag with utf8 encoding, and the DOCTYPE.
     *
     * @param rootElemName the name of the root element, used in the DOCTYPE tag.
     * @param publicId the public identifier, e.g. "-//OpenOffice.org//DTD OfficeDocument 1.0//EN"
     * @param systemId the system identifier, e.g. "office.dtd" or a full URL to it.
     */
    void startDocument(const char* rootElemName, const char* publicId = 0, const char* systemId = 0);

    /// Call this to terminate an XML document.
    void endDocument();

    /**
     * Start a new element, as a child of the current element.
     *
     * @param tagName the name of the tag.
     * @param indentInside if set to false, there will be no indentation inside
     * this tag. This is useful for elements where whitespace matters.
     */
    void startElement(const char* tagName, bool indentInside = true);

    /**
     * Overloaded version of addAttribute( const char*, const char* ),
     * which is a bit slower because it needs to convert @p value to utf8 first.
     */
    inline void addAttribute(const char* attrName, const QString& value) {
        addAttribute(attrName, value.toUtf8());
    }
    /**
     * Add an attribute whose value is an integer
     */
    inline void addAttribute(const char* attrName, int value) {
        addAttribute(attrName, QByteArray::number(value));
    }
    /**
     * Add an attribute whose value is an unsigned integer
     */
    inline void addAttribute(const char* attrName, uint value) {
        addAttribute(attrName, QByteArray::number(value));
    }
    /**
     * Add an attribute whose value is an bool
     * It is written as "true" or "false" based on value
     */
    inline void addAttribute(const char* attrName, bool value) {
        addAttribute(attrName, value ? "true" : "false");
    }
    /**
     * Add an attribute whose value is a floating point number
     * The number is written out with the highest possible precision
     * (unlike QString::number and setNum, which default to 6 digits)
     */
    void addAttribute(const char* attrName, double value);

    /**
     * Add an attribute whose value is a floating point number
     * The number is written out with the highest possible precision
     * (unlike QString::number and setNum, which default to 6 digits)
     */
    void addAttribute(const char* attrName, float value);

    /// Overloaded version of the one taking a const char* argument, for convenience
    void addAttribute(const char* attrName, const QByteArray& value);

    /**
     * Add an attribute to the current element.
     */
    void addAttribute(const char* attrName, const char* value);

    /**
     * Terminate the current element. After this you should start a new one (sibling),
     * add a sibling text node, or close another one (end of siblings).
     */
    void endElement();

    /**
     * Overloaded version of addTextNode( const char* ),
     * which is a bit slower because it needs to convert @p str to utf8 first.
     */
    inline void addTextNode(const QString& str) {
        addTextNode(str.toUtf8());
    }

    /// Overloaded version of the one taking a const char* argument
    void addTextNode(const QByteArray& cstr);

    /**
     * @brief Adds a text node as a child of the current element.
     *
     * This is appends the literal content of @p str to the contents of the element.
     * E.g. addTextNode( "foo" ) inside a \<p\> element gives \<p\>foo\</p\>,
     * and startElement( "b" ); endElement( "b" ); addTextNode( "foo" ) gives \<p\>\<b/\>foo\</p\>
     */
    void addTextNode(const char* cstr);

    /**
     * This is quite a special-purpose method, not for everyday use.
     * It adds a complete element (with its attributes and child elements)
     * as a child of the current element. The iodevice is supposed to be escaped
     * for XML already, so it will usually come from another KoXmlWriter.
     * This is usually used with KTempFile.
     */
    void addCompleteElement(QIODevice* dev);

    // #### Maybe we want to subclass KoXmlWriter for manifest files.
    /**
     * Special helper for writing "manifest" files
     * This is equivalent to startElement/2*addAttribute/endElement
     * This API will probably have to change (or not be used anymore)
     * when we add support for encrypting/signing.
     * @note OASIS-specific
     */
    void addManifestEntry(const QString& fullPath, const QString& mediaType);

private:
    struct Tag {
        Tag(const char *t = 0, bool ind = true)
            : hasChildren(false)
            , lastChildIsText(false)
            , openingTagClosed(false)
            , indentInside(ind)
        {
            tagName = new char[qstrlen(t) + 1];
            qstrcpy(tagName, t);
        }

        ~Tag() {
            delete[] tagName;
        }

        Tag(const Tag &original)
        {
            tagName = new char[qstrlen(original.tagName) + 1];
            qstrcpy(tagName, original.tagName);

            hasChildren = original.hasChildren;
            lastChildIsText = original.lastChildIsText;
            openingTagClosed = original.openingTagClosed;
            indentInside = original.indentInside;
        }

        char *tagName {0};
        bool hasChildren : 1; ///< element or text children
        bool lastChildIsText : 1; ///< last child is a text node
        bool openingTagClosed : 1; ///< true once the '\>' in \<tag a="b"\> is written out
        bool indentInside : 1; ///< whether to indent the contents of this tag
    };

    /// Write out \n followed by the number of spaces required.
    void writeIndent();

    void writeCString(const char* cstr);

    void writeChar(char c);


    inline void closeStartElement(Tag& tag) {
        if (!tag.openingTagClosed) {
            tag.openingTagClosed = true;
            writeChar('>');
        }
    }
    char *escapeForXML(const char* source, int length) const;
    bool prepareForChild(bool indentInside = true);
    void prepareForTextNode();

    class Private;
    Private * const d;

    KoXmlWriter(const KoXmlWriter &);   // forbidden
    KoXmlWriter& operator=(const KoXmlWriter &);   // forbidden
};

#endif /* XMLWRITER_H */

