/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at developer.feedback@nokia.com.
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QIntValidator>
#include <QRegExpValidator>
#include <QTextCursor>
#include <QRegExp>
#include <QtDebug>
#include <QTextDocument>
#include <QDate>

#include "hbdatetimevalidator_p_p.h"
#include "hbvalidator_p.h"

#define DEFAULT_YEAR_VALUE "2009"

const int KMinimumYear = -4713;
const int KMaximumYear = 11000000; // see more in QDate documentation

HbDateTimeValidatorPrivate::HbDateTimeValidatorPrivate() :
    HbValidatorPrivate()
{

}

HbDateTimeValidatorPrivate::~HbDateTimeValidatorPrivate()
{

}

class HbFixableIntQValidator : public QIntValidator
{
public:
    HbFixableIntQValidator( QObject * parent ) :
            QIntValidator ( parent ),
            mMaxLeght(10000)
    {
    }

    HbFixableIntQValidator ( int minimum, int maximum, QObject * parent ) :
            QIntValidator ( minimum, maximum, parent )
    {
        int topLenght = QString::number(top()).length();
        int bottomLenght = QString::number(bottom()).length();
        mMaxLeght = qMax(topLenght, bottomLenght);
    }

    void fixup ( QString & input ) const
    {
        QString newTextValue=input;
        newTextValue.remove(QRegExp((bottom()>=0)?"[^0-9]":"[^\\-0-9]"));
        bool ok;
        int value = newTextValue.toInt(&ok);
        if(ok) {
            value = qBound(bottom(), value, top());
            newTextValue.setNum(value);
            if(newTextValue!=input) {
                input = newTextValue;
            }
        }
    }

    State validate ( QString & input, int & pos ) const
    {
        if(input.length()>mMaxLeght) {
            return Invalid;
        }

        return QIntValidator::validate(input, pos);
    }

private:
    int mMaxLeght;
};

void HbDateTimeValidatorPrivate::addDateTimeField(const DataPositions& data)
{
    Q_Q(HbDateTimeValidator);
    QValidator *fieldValidator = NULL;
    QString defaultValue;

    switch(data.type) {
    case Year: {
            if(data.lenght() == 4) {
                fieldValidator = new HbFixableIntQValidator(KMinimumYear,
                                                            KMaximumYear,
                                                            q);
                defaultValue = "2009";
            } else {
                fieldValidator = new HbFixableIntQValidator(0,99,q);
                defaultValue = "09";
            }
        }
        break;
    case Month: {
            fieldValidator = new HbFixableIntQValidator(1,12,q);
            defaultValue = (data.lenght() == 1)?"1":"01";
        }
        break;
    case Day: {
            fieldValidator = new HbFixableIntQValidator(1,31,q);
            defaultValue = (data.lenght() == 1)?"1":"01";
        }
        break;
    case AmPm: {
            QRegExp regUxp("AM|PM", Qt::CaseInsensitive);
            fieldValidator = new QRegExpValidator(regUxp,q);
            defaultValue = "AM";
        }
        break;
    case Hour24: {
            fieldValidator = new HbFixableIntQValidator(0,23,q);
            defaultValue = (data.lenght() == 1)?"0":"00";
        }
        break;
    case Hour12: {
            fieldValidator = new HbFixableIntQValidator(1,12,q);
            defaultValue = "12";
        }
        break;
    case Minutes: {
            fieldValidator = new HbFixableIntQValidator(0,59,q);
            defaultValue = (data.lenght() == 1)?"0":"00";
        }
        break;
    case Seconds: {
            fieldValidator = new HbFixableIntQValidator(0,59,q);
            defaultValue = (data.lenght() == 1)?"0":"00";
        }
        break;
    case MilliSeconds: {
            fieldValidator = new HbFixableIntQValidator(0,999,q);
            defaultValue = (data.lenght() == 1)?"0":"000";
        }
        break;
    default:
        Q_ASSERT(false);
    }
    fieldTypes.append(data.type);
    addField(separatorToAdd,fieldValidator,defaultValue);
}

bool HbDateTimeValidatorPrivate::fixDate(QTextCursor *cursor, bool updateCursor)
{
    Q_ASSERT(!fieldTypes.isEmpty());

    QRegExp regExp = QRegExp(parseString,Qt::CaseInsensitive);

    Q_ASSERT(regExp.isValid());

    bool result = false;

    if(regExp.exactMatch(cursor->document()->toPlainText())) {
        QTextCursor fixingCursor(cursor->document());
        fixingCursor.beginEditBlock();

        // step one: fixing each field separately
        for(int i = fieldTypes.count()-1; i>=0; --i ) {
            int pos = regExp.pos(i+1);
            QString fieldTxt = regExp.cap(i+1);
            fixingCursor.setPosition(pos);
            fixingCursor.setPosition(pos + fieldTxt.length());

            int x = fieldTxt.length();
            if( QValidator::Acceptable != fields.at(i)->mValidator->validate(fieldTxt, x) ) {
                fields.at(i)->mValidator->fixup(fieldTxt);
                if(QValidator::Acceptable == fields.at(i)->mValidator->validate(fieldTxt, x)) {
                    // successful fix
                    fixingCursor.insertText(fieldTxt);
                    result = true;
                }
            }
        }

        // step two: fixing correlations between fields
        // Note: this functionality is specific for HbDateTimeValidator
        int yearCapture  = fieldTypes.indexOf(Year)+1;
        int monthCapture = fieldTypes.indexOf(Month)+1;
        int dayCapture   = fieldTypes.indexOf(Day)+1;

        if(dayCapture<=0 || monthCapture<=0) {
            fixingCursor.endEditBlock();
            return result; // nothing to fix no date
        }

        // match content again
        if(regExp.exactMatch(cursor->document()->toPlainText())) {
            int year=1900, month, day;

            if(yearCapture>0) {
                year = regExp.cap(yearCapture).toInt();
            }
            month = regExp.cap(monthCapture).toInt();

            int daySize =0;
            QString s = regExp.cap(dayCapture);
            daySize = s.length();
            day   = s.toInt();

            if( !QDate::isValid(year, month, day) ) {
                QDate dateFoFix(year,month,1);
                if(dateFoFix.daysInMonth()<day) {
                    dateFoFix.setDate(year,month,dateFoFix.daysInMonth());
                    int dayCapPos = regExp.pos(dayCapture);
                    fixingCursor.setPosition(dayCapPos);
                    fixingCursor.setPosition(dayCapPos+daySize, QTextCursor::KeepAnchor);

                    // updating date by changing document
                    // this will prevent some unexpected isues
                    fixingCursor.insertText(QString().setNum(dateFoFix.daysInMonth()));

                    if(updateCursor) {
                        fixingCursor.setPosition(dayCapPos);
                        fixingCursor.setPosition(dayCapPos+daySize, QTextCursor::KeepAnchor);
                        *cursor = fixingCursor;
                    }
                    result = true;
                }
            }
        }
        fixingCursor.endEditBlock();
    }

    return result;
}

/*!
    \class HbDateTimeValidator
    \brief HbDateTimeValidator provides validation services for QAbstractEdit derived editors.
*/

/*!
    Constructs a HbDateTimeValidator object with a /a parent
*/
HbDateTimeValidator::HbDateTimeValidator(QObject *parent) :
    HbValidator(*new HbDateTimeValidatorPrivate, parent)
{
    Q_D(HbDateTimeValidator);
    d->q_ptr = this;
}

HbDateTimeValidator::HbDateTimeValidator(HbDateTimeValidatorPrivate &dd, QObject *parent) :
    HbValidator(dd, parent)
{
    Q_D(HbDateTimeValidator);
    d->q_ptr = this;
}

/*!
    Destructor
*/
HbDateTimeValidator::~HbDateTimeValidator()
{
}

inline void appendIfFound(DataType type, QList<DataPositions>* where, QRegExp& what, const QString& from)
{
    int pos = from.indexOf(what);
    if(pos>=0) {
        int matchLenght = what.cap(0).length();
        Q_ASSERT( matchLenght>0 );
        Q_ASSERT( from.indexOf(what, pos+matchLenght)==-1 );
        
        where->append(
                DataPositions(type, pos, matchLenght)
                );
    }
}

void HbDateTimeValidator::setDisplayFormat(const QString &format)
{
    QList<DataPositions> positions;

    QRegExp regExp;
    QString newParseString = QRegExp::escape(format);
    const QString captureNumber = "([0-9]+)";

    regExp.setPattern("yy(yy)?");
    appendIfFound(Year, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    regExp.setPattern("M{1,3}");
    appendIfFound(Month, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    regExp.setPattern("d{1,3}");
    appendIfFound(Day, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    const QString captureAmPm("(AM|PM|am|pm)");
    regExp.setPattern("ap");
    appendIfFound(AmPm, &positions, regExp, format);
    newParseString.replace(regExp, captureAmPm);

    regExp.setPattern("AP");
    appendIfFound(AmPm, &positions, regExp, format);
    newParseString.replace(regExp, captureAmPm);

    DataType hourType = Hour24;
    if(!positions.isEmpty()) {
        if(positions.last().type==AmPm) {
            hourType = Hour12;
        }
    }

    regExp.setPattern("hh?");
    appendIfFound(hourType, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    regExp.setPattern("HH?");
    appendIfFound(hourType, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    regExp.setPattern("mm?");
    appendIfFound(Minutes, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    regExp.setPattern("ss?");
    appendIfFound(Seconds, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    regExp.setPattern("z(zz)?");
    appendIfFound(MilliSeconds, &positions, regExp, format);
    newParseString.replace(regExp, captureNumber);

    if(positions.size()==0) {
        qWarning() << "HbDateTimeValidator::setDisplayFormat: format: \""
                << format << "\" is invalid so it was ignored" ;
        return;
    }

    qSort(positions);

    Q_D(HbDateTimeValidator);
    d->format = format;
    // ignore trailing separators:
    if(positions.back().stop < format.length()) {
        newParseString.chop(QRegExp::escape(format.mid(positions.back().stop)).length());
        d->format.resize(positions.back().stop);
    }

    // ignore front separators:
    if(positions.front().start != 0) {
        newParseString.remove(0,positions.front().start);
    }

    d->parseString = newParseString;
    d->fieldTypes.clear();
    d->deleteAllFields();

    const int n=positions.size()-1;
    for(int i=0; i<n; ++i) {
        d->addDateTimeField(positions.at(i));
        int separatorBegin = positions.at(i).stop;
        int separatorLenght = positions.at(i+1).start - separatorBegin;
        if(separatorLenght>0) {
            setDefaultSeparator(format.mid(separatorBegin,separatorLenght));
        } else {
            setDefaultSeparator(" ");
        }
    }
    d->addDateTimeField(positions.at(n));
}

const QString HbDateTimeValidator::displayFormat() const
{
    Q_D(const HbDateTimeValidator);
    return d->format;
}

bool HbDateTimeValidator::fixDate(QTextCursor *cursor, bool updateCursor)
{
    return d_func()->fixDate(cursor, updateCursor);
}
