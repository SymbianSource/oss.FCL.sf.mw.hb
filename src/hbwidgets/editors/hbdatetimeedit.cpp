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

#include <QDateTime>
#include <QMetaMethod>

#include "hblineedit.h"
#include "hbdatetimevalidator_p.h"
#include "hbdatetimeedit.h"
#include "hbdatetimeedit_p.h"

/*!
    @alpha
    @hbwidgets
    \brief The HbDateTimeEdit class provides a widget for editing dates and times.



    \sa QDateTime

 */

/*!
    Constructs datetime edit widget.
 */
HbDateTimeEdit::HbDateTimeEdit (QGraphicsItem *parent) :
    HbLineEdit(*new HbDateTimeEditPrivate, parent)
{
    Q_D(HbDateTimeEdit);
    d->q_ptr = this;

    d->init();
}

/*!
    Constructs date time editor to display \a date using default locale.

 */
HbDateTimeEdit::HbDateTimeEdit (const QDate &date, QGraphicsItem *parent) :
    HbLineEdit(*new HbDateTimeEditPrivate, parent)
{
    Q_D(HbDateTimeEdit);
    d->q_ptr = this;

    d->init(date);
}

/*!
    Constructs date time editor to display \a time using default locale.
 */
HbDateTimeEdit::HbDateTimeEdit (const QTime &time, QGraphicsItem *parent) :
    HbLineEdit(*new HbDateTimeEditPrivate, parent)
{
    Q_D(HbDateTimeEdit);
    d->q_ptr = this;

    d->init(time);
}

/*!
    Constructs date time editor to display \a dateTime using default locale.
 */
HbDateTimeEdit::HbDateTimeEdit (const QDateTime &dateTime, QGraphicsItem *parent) :
    HbLineEdit(*new HbDateTimeEditPrivate, parent)
{
    Q_D(HbDateTimeEdit);
    d->q_ptr = this;

    d->init(dateTime);
}

/*
    for internal use only
 */
HbDateTimeEdit::HbDateTimeEdit (HbDateTimeEditPrivate &dd, QGraphicsItem *parent) :
    HbLineEdit(dd, parent)
{
    Q_D(HbDateTimeEdit);
    d->q_ptr = this;

    d->init();
}

/*!
    Destructor.
 */
HbDateTimeEdit::~HbDateTimeEdit ()
{
}

/*!
    \reimp
 */
int HbDateTimeEdit::type () const
{
    return Type;
}

/*!
    Sets the current value.

    \sa dateTime()
 */
void HbDateTimeEdit::setDateTime (const QDateTime& dateTime)
{
    Q_D(HbDateTimeEdit);
    d->dateTime = dateTime;
    d->isSetDateTimeActive = true;
    setPlainText(dateTime.toString(d->validator->displayFormat()));
    d->isSetDateTimeActive = false;
}

/*!
    Returns the current value as a QDateTime.

    /sa setDateTime()
 */
QDateTime HbDateTimeEdit::dateTime () const
{
    Q_D(const HbDateTimeEdit);
    return d->dateTime;
}

/*!
    Returns the current display format.

    /sa setDisplayFormat() and QDateTime::toString
 */
QString HbDateTimeEdit::displayFormat () const
{
    Q_D(const HbDateTimeEdit);
    return d->validator->displayFormat();
}

/*!
    Sets editor to display date and/or time in custom \a format.

    \sa displayFormat()
    \sa QDateTime::toString
 */
void HbDateTimeEdit::setDisplayFormat (const QString& format)
{
    Q_D(HbDateTimeEdit);

    QString suportedFormat = format;

    static const char KNotSupportedDayLongNameFormat[] = "dddd";
    static const char KNotSupportedDayShortNameFormat[] = "ddd";
    static const char KNotSupportedMonthLongNameFormat[] = "MMMM";
    static const char KNotSupportedMonthShortNameFormat[] = "MMM";
    static const char KSupportedMonthFormat[] = "MM";

    suportedFormat.remove(KNotSupportedDayLongNameFormat);
    suportedFormat.remove(KNotSupportedDayShortNameFormat);

    suportedFormat.replace(KNotSupportedMonthLongNameFormat, KSupportedMonthFormat);
    suportedFormat.replace(KNotSupportedMonthShortNameFormat, KSupportedMonthFormat);

    if(suportedFormat!=format) {
        qWarning() << "Names of days and months are not suported. Replacing format string: "
                << format << " by a format string: "
                << suportedFormat;
    } else {
        // to save memory
        suportedFormat = format;
    }
    d->validator->setDisplayFormat(suportedFormat);
    setPlainText(d->dateTime.toString(d->validator->displayFormat()));
}

/*!
    Sets editor to display date in format described by \a locale

    \sa QLocale::dateFormat
 */
void HbDateTimeEdit::setDateFormat(const QLocale& locale)
{
    setDisplayFormat(locale.dateFormat(QLocale::ShortFormat));
}

/*!
    Sets editor to display time in format described by \a locale

    \sa QLocale::timeFormat
 */
void HbDateTimeEdit::setTimeFormat(const QLocale& locale)
{
    setDisplayFormat(locale.timeFormat(QLocale::ShortFormat));
}

/*!
    Sets editor to display date and time in format described by \a locale

    \sa QLocale::dateTimeFormat
 */
void HbDateTimeEdit::setDateTimeFormat(const QLocale& locale)
{
    setDisplayFormat(locale.dateTimeFormat(QLocale::ShortFormat));
}


void HbDateTimeEdit::setMaxLength (int length)
{
    HbLineEdit::setMaxLength(length);
}

void HbDateTimeEdit::setText (const QString &text)
{
    HbLineEdit::setText(text);
}

/*!
    \reimp
*/
void HbDateTimeEdit::focusOutEvent(QFocusEvent *event)
{
    Q_D(HbDateTimeEdit);

    if(d->validator->fixDate(&d->cursor, true)) {
        // fixing so restore focus to editor
        // do it asynchronously by private slot:
        int metodIndex = metaObject()->indexOfMethod("_q_regainFocus()");
        if (metodIndex<0) {
            qWarning("HbDateTimeEdit::focusOutEvent private method "
                     "_q_regainFocus() NOT FOUND");
        } else {
            if (!metaObject()->method(metodIndex).invoke(this, Qt::QueuedConnection)) {
                qWarning("HbDateTimeEdit::focusOutEvent failed to call "
                         "private method _q_regainFocus()");
            }
        }
        // prevent emmiting signal editingFinished so omnit HbLineEdit
        HbAbstractEdit::focusOutEvent(event);
    } else {
        HbLineEdit::focusOutEvent(event);
    }
}
