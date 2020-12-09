/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_object_catcher.h"

#include <QString>
#include <QPointF>
#include <QColor>

#include <resources/KoAbstractGradient.h>

#include <kis_debug.h>

KisAslObjectCatcher::KisAslObjectCatcher()
    : m_arrayMode(false)
{
}

KisAslObjectCatcher::~KisAslObjectCatcher()
{
}

void KisAslObjectCatcher::addDouble(const QString &path, double value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "double" << value;
}

void KisAslObjectCatcher::addInteger(const QString &path, int value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "int" << value;
}

void KisAslObjectCatcher::addEnum(const QString &path, const QString &typeId, const QString &value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "enum" << ppVar(typeId) << ppVar(value);
}

void KisAslObjectCatcher::addUnitFloat(const QString &path, const QString &unit, double value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "unitfloat" << ppVar(unit) << ppVar(value);
}

void KisAslObjectCatcher::addText(const QString &path, const QString &value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "text" << value;
}

void KisAslObjectCatcher::addBoolean(const QString &path, bool value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "bool" << value;
}

void KisAslObjectCatcher::addColor(const QString &path, const QColor &value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "color" << value;
}

void KisAslObjectCatcher::addPoint(const QString &path, const QPointF &value) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "point" << value;
}

void KisAslObjectCatcher::addCurve(const QString &path, const QString &name, const QVector<QPointF> &points) {
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "curve" << name << ppVar(points.size());
}

void KisAslObjectCatcher::addPattern(const QString &path, const KoPatternSP value, const QString& patternUuid)
{
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "pattern" << value << " uuid " << patternUuid;
}

void KisAslObjectCatcher::addPatternRef(const QString &path, const QString &patternUuid, const QString &patternName)
{
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "pattern-ref" << ppVar(patternUuid) << ppVar(patternName);
}

void KisAslObjectCatcher::addGradient(const QString &path, KoAbstractGradientSP value)
{
    dbgKrita << "Unhandled:" << (m_arrayMode ? "[A]" : "[ ]") << path << "gradient" << value;
}

void KisAslObjectCatcher::newStyleStarted()
{
    dbgKrita << "Unhandled:" << "new style started";
}

void KisAslObjectCatcher::setArrayMode(bool value) {
    m_arrayMode = value;
}

