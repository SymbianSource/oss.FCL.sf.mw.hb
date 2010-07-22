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

#include <QtDebug>

#include <QTextDocument>
#include <QTextBlock>
#include <QRegExp>

#include "hbdatetimeedit_p.h"
#include "hbdatetimeedit.h"
#include "hbdatetimevalidator_p.h"
#include "hbinputeditorinterface.h"

HbDateTimeEditPrivate::HbDateTimeEditPrivate () :
    HbLineEditPrivate(),
    validator(new HbDateTimeValidator()),
    isSetDateTimeActive( false )
{
}

HbDateTimeEditPrivate::~HbDateTimeEditPrivate ()
{
    delete validator;
}

void HbDateTimeEditPrivate::init()
{
    Q_Q(HbDateTimeEdit);

    QString defaultFormat = QLocale().dateFormat(QLocale::ShortFormat);
    validator->setDisplayFormat(defaultFormat);
    q->setValidator(validator);
    q->setDateTime(QDateTime::currentDateTime());

#if QT_VERSION >= 0x040600
    q->setInputMethodHints(Qt::ImhDigitsOnly);
#endif
}

void HbDateTimeEditPrivate::init(const QDate &date)
{
    Q_Q(HbDateTimeEdit);

    QString defaultFormat = QLocale().dateFormat(QLocale::ShortFormat);
    validator->setDisplayFormat(defaultFormat);
    q->setValidator(validator);
    q->setDateTime(QDateTime(date));

#if QT_VERSION >= 0x040600
    q->setInputMethodHints(Qt::ImhDigitsOnly);
#endif
}

void HbDateTimeEditPrivate::init(const QTime &time)
{
    Q_Q(HbDateTimeEdit);

    QString defaultFormat = QLocale().timeFormat(QLocale::ShortFormat);
    validator->setDisplayFormat(defaultFormat);
    q->setValidator(validator);
    q->setDateTime(QDateTime(QDate::currentDate(), time));

#if QT_VERSION >= 0x040600
    q->setInputMethodHints(Qt::ImhDigitsOnly);
#endif
}

void HbDateTimeEditPrivate::init(const QDateTime &dateTime)
{
    Q_Q(HbDateTimeEdit);

    QString defaultFormat = QLocale().dateTimeFormat(QLocale::ShortFormat);
    validator->setDisplayFormat(defaultFormat);
    q->setValidator(validator);
    q->setDateTime(dateTime);

#if QT_VERSION >= 0x040600
    q->setInputMethodHints(Qt::ImhDigitsOnly);
#endif
}

void HbDateTimeEditPrivate::_q_textChanged()
{
    if(!isSetDateTimeActive) {
        QDateTime tmpDateTime = QDateTime::fromString(doc->toPlainText(), validator->displayFormat());

        if( tmpDateTime.isValid() ) {
            dateTime = tmpDateTime;
        }
    }
    HbLineEditPrivate::_q_textChanged();
}

#include <moc_hbdatetimeedit.cpp>
