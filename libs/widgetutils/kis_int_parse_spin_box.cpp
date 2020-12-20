/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_int_parse_spin_box.h"

#include "kis_num_parser.h"

#include <QLabel>
#include <QPixmap>
#include <QIcon>
#include <QFile>
#include <QLineEdit>

KisIntParseSpinBox::KisIntParseSpinBox(QWidget *parent) :
    QSpinBox(parent),
    boolLastValid(true)
{
    setAlignment(Qt::AlignRight);

    lastExprParsed = new QString("0");

    connect(this, SIGNAL(noMoreParsingError()),
                    this, SLOT(clearErrorStyle()));

    //hack to let the clearError be called, even if the value changed method is the one from QSpinBox.
    connect(this, SIGNAL(valueChanged(int)),
                    this, SLOT(clearError()));

    connect(this, SIGNAL(errorWhileParsing(QString)),
                    this, SLOT(setErrorStyle()));

    oldVal = value();

    warningIcon = new QLabel(this);

    if (QFile(":/./16_light_warning.svg").exists()) {
        warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(16, 16));
    } else {
        warningIcon->setText("!");
    }

    warningIcon->setStyleSheet("background:transparent;");
    warningIcon->move(1, 1);
    warningIcon->setVisible(false);

    isOldPaletteSaved = false;
    areOldMarginsSaved = false;

}

KisIntParseSpinBox::~KisIntParseSpinBox()
{

    //needed to avoid a segfault during destruction.
    delete lastExprParsed;

}

int KisIntParseSpinBox::valueFromText(const QString & text) const
{

    *lastExprParsed = text;

    bool ok;

    int val;

    if ( (suffix().isEmpty() || !text.endsWith(suffix())) &&
         (prefix().isEmpty() || !text.startsWith(prefix())) ) {

        val = KisNumericParser::parseIntegerMathExpr(text, &ok);

    } else {

        QString expr = text;

        if (text.endsWith(suffix())) {
            expr.remove(text.size()-suffix().size(), suffix().size());
        }

        if(text.startsWith(prefix())){
            expr.remove(0, prefix().size());
        }

        *lastExprParsed = expr;

        val = KisNumericParser::parseIntegerMathExpr(expr, &ok);

    }

    if (text.trimmed().isEmpty()) { //an empty text is considered valid in this case.
            ok = true;
    }

    if (!ok) {

            if (boolLastValid == true) {
                oldVal = value();
            }

            boolLastValid = false;
            //emit errorWhileParsing(text); //if uncommented become red every time the string is wrong.
            val = oldVal;
    } else {

            if (boolLastValid == false) {
                oldVal = val;
            }

            boolLastValid = true;
            //emit noMoreParsingError();
    }

    return val;

}

QString KisIntParseSpinBox::textFromValue(int val) const
{

    if (!boolLastValid) {
        emit errorWhileParsing(*lastExprParsed);
        return *lastExprParsed;
    }

    emit noMoreParsingError();

    return QSpinBox::textFromValue(val);

}

QValidator::State KisIntParseSpinBox::validate ( QString & input, int & pos ) const
{

    Q_UNUSED(input);
    Q_UNUSED(pos);

    //this simple definition is sufficient for the moment
    //TODO: see if needed to get something more complex.
    return QValidator::Acceptable;

}

void KisIntParseSpinBox::stepBy(int steps)
{

    boolLastValid = true;
    emit noMoreParsingError();

    QSpinBox::stepBy(steps);

}

void KisIntParseSpinBox::setValue(int val)
{

    if(val == oldVal && hasFocus()){ //avoid to reset the button when it set the value of something that will recall this slot.
        return;
    }

    if (!hasFocus()) {
        clearError();
    }

    QSpinBox::setValue(val);
}

void KisIntParseSpinBox::setErrorStyle()
{

    if (!boolLastValid) {
        //setStyleSheet(_oldStyleSheet + "Background: red; color: white; padding-left: 18px;");

        if (!isOldPaletteSaved) {
            oldPalette = palette();
        }
        isOldPaletteSaved = true;

        QPalette nP = oldPalette;
        nP.setColor(QPalette::Background, Qt::red);
        nP.setColor(QPalette::Base, Qt::red);
        nP.setColor(QPalette::Text, Qt::white);
        setPalette(nP);

        if (!areOldMarginsSaved) {
            oldMargins = lineEdit()->textMargins();
        }
        areOldMarginsSaved = true;

        if (width() - height() >= 3*height()) { //if we have twice as much place as needed by the warning icon then display it.
            QMargins newMargins = oldMargins;
            newMargins.setLeft( newMargins.left() + height() - 4 );
            lineEdit()->setTextMargins(newMargins);

            int h = warningIcon->height();
            int hp = height()-2;

            if (h != hp) {
                warningIcon->resize(hp, hp);

                if (QFile(":/./16_light_warning.svg").exists()) {
                    warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(hp-7, hp-7));
                }
            }

            warningIcon->move(oldMargins.left()+4, 1);
            warningIcon->setVisible(true);
        }
    }
}

void KisIntParseSpinBox::clearErrorStyle()
{
    if (boolLastValid) {
        warningIcon->setVisible(false);

        //setStyleSheet(QString());

        setPalette(oldPalette);
        isOldPaletteSaved = false;

        lineEdit()->setTextMargins(oldMargins);
        areOldMarginsSaved = false;
    }
}


void KisIntParseSpinBox::clearError()
{
    boolLastValid = true;
    emit noMoreParsingError();
    oldVal = value();
    clearErrorStyle();
}
