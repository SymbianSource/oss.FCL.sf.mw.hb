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

#include "hbdatetimepicker_p.h"
#include "hbdatetimepicker.h"
#include "hbstyleoption_p.h"

/*!
    @beta
    \class HbDateTimePicker
    \brief HbDateTimePicker class provides a widget for picking the date, time, date and time. 
    By default date picker will be created, with date functionality only.
    For exclusive time or datetime picker creation, use QTime or QDateTime variable as parameter for the constructor.
    \li Date and Time
    \li Date
    \li Time
    
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,51}

    By default, display format is from HbExtendedLocale. User can set the format using \sa setDisplayFormat.
    Based on the display format the tumblers or sections will be rearranged.
    
    HbDateTimePicker provides various date and time range functionalities.
    currently the date range is independent of the time range.
    \sa setDateRange \sa setTimeRange \sa setDateTimeRange
    \sa setMinimumTime \sa setMaximumTime \sa setMinimumDate \sa setMaximumDate
    \sa setMinimumDateTime \sa setMaximumDateTime
*/

/*!
    \fn void dateChanged(const QDate &date)

    This signal is emitted when item selection changes in any of the date pickers in the datetimepicker widget.
    
    \param date  selected by the user.

*/

/*!
    \fn void timeChanged(const QTime &time)

    This signal is emitted when item selection changes in any of the time pickers in the datetimepicker widget.
    \param time  selected by the user.

*/

/*!
    \fn void dateTimeChanged(const QDateTime &datetime)

    This signal is emitted when item selection changes in any of the pickers in the datetimepicker widget.
    \param datetime  selected by the user.

*/

/*!
    Constructs date picker widget by default.

    \param parent parent item.
*/
HbDateTimePicker::HbDateTimePicker( QGraphicsItem *parent ):
HbWidget(*new HbDateTimePickerPrivate, parent)
{
	Q_D(HbDateTimePicker);

	//no mode passed so it should take date as mode by default
	d->init(QVariant::Date);

	setDateTime(QDateTime::currentDateTime());
}

/*!
    Constructs datetime picker widget.

    \param datetime QDateTime value.
*/
HbDateTimePicker::HbDateTimePicker(const QDateTime &datetime, QGraphicsItem *parent ):
HbWidget(*new HbDateTimePickerPrivate, parent)
{
    Q_D(HbDateTimePicker);

	d->init(QVariant::DateTime);
	setDateTime(datetime);
}

/*!
    Constructs date picker widget with default locale's date format.
    
    \param date QDate value.
*/
HbDateTimePicker::HbDateTimePicker(const QDate &date, QGraphicsItem *parent ):
HbWidget(*new HbDateTimePickerPrivate, parent)
{
    Q_D(HbDateTimePicker);

	d->init(QVariant::Date);
    setDate(date);
}

/*!
    Constructs time picker widget with default locale's time format.
    
    \param time QTime value.
*/
HbDateTimePicker::HbDateTimePicker(const QTime &time, QGraphicsItem *pParent ):
HbWidget(*new HbDateTimePickerPrivate, pParent)
{
    Q_D(HbDateTimePicker);

	d->init(QVariant::Time);
    setTime(time);
}

/*!
    Internal. Protected constructor for derivations.
    the default mode is DateTimeMode, if other mode is required, set the mDateTimeMode variable.
    this does not set any default datetime, needs to be explicitly done in the derived constructor.

    \sa setDateTime, setTime, setDate
*/
HbDateTimePicker::HbDateTimePicker(HbDateTimePickerPrivate &dd, QGraphicsItem *parent):
HbWidget(dd, parent)
{
    Q_D(HbDateTimePicker);

	d->init(QVariant::DateTime);
}

/*!
    \reimp
*/
bool HbDateTimePicker::event(QEvent *e)
{    
    bool result = HbWidget::event(e);
    if (e->type()==QEvent::LayoutRequest) {
        updatePrimitives();
    }
    return result;
}

/*!
    Destructor.
*/
HbDateTimePicker::~HbDateTimePicker()
{
}

/*!
    Returns current display format as QString value.

    \return display format.

    \sa setDisplayFormat()
 */
QString HbDateTimePicker::displayFormat() const
{
    Q_D(const HbDateTimePicker);
    return d->mFormat;
}

/*!
    Sets the display \a format . eg: DDMMYY, MMDDYY
    
    These expressions may be used for the format:

    <TABLE>
    <TR><TD><b><i> Expression </TD><TD> Output </TD></i></b></TR>
    <TR><TD> d </TD><TD><i> The day as a number without a leading zero (1 to 31)</i></TD></TR>
    <TR><TD> dd </TD><TD><i> The day as a number with a leading zero (01 to 31) </i></TD></TR>
    <TR><TD> ddd </TD><TD><i>
          The abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().</i></TD></TR>
    <TR><TD> dddd </TD><TD><i>
          The long localized day name (e.g. 'Monday' to 'Sunday').
            Uses QDate::longDayName().</i></TD></TR>
    <TR><TD> M </TD><TD><i> The month as a number without a leading zero (1 to 12)</i></TD></TR>
    <TR><TD> MM </TD><TD><i> The month as a number with a leading zero (01 to 12)</i></TD></TR>
    <TR><TD> MMM </TD><TD><i>
         The abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().</i></TD></TR>
    <TR><TD> MMMM </TD><TD><i>
          The long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().</i></TD></TR>
    <TR><TD> yy </TD><TD><i> The year as two digit number (00 to 99)</i></TD></TR>
    <TR><TD> yyyy </TD><TD><i> The year as four digit number. If the year is negative,
            a minus sign is prepended in addition.</i></TD></TR>
    </TABLE>

    NOTE:setDisplayFormat works only when the seperators are mentioned in the format like 'dd.mm.yy' or 'dd mm yy', this
         will be fixed in the future versions.

    \param format is the display format in QString format.

    \sa displayFormat()
*/
void HbDateTimePicker::setDisplayFormat(const QString &format)
{
	Q_D(HbDateTimePicker);

	if(d->isFormatValid(format)){ 
		d->mFormat = format;
		d->parseDisplayFormat(format);        
		d->rearrangeTumbleViews();
        d->emitDateTimeChange();
	}//End If format is valid
}

/*!
    Returns the current date in QDate format.

    \return Date Picker's current date.

    \sa setDate
*/
QDate HbDateTimePicker::date() const
{
    Q_D(const HbDateTimePicker);

    return d->mDateTime.date();
}

/*!
    Sets the current \a date in the form of QDate.

    \param date date in QDate format

    \sa date
*/
void HbDateTimePicker::setDate(const QDate& date)
{
    Q_D(HbDateTimePicker);
    setDateTime(QDateTime(date,d->mDateTime.time()));
}

/*!
    Returns minimum date in QDate format.

    \return Minimum date in QDate format.

    \sa setMinimumDate
*/
QDate HbDateTimePicker::minimumDate()const
{
    Q_D(const HbDateTimePicker);
    return d->mMinimumDate.date();
}

/*!
    Sets minimum \a date in QDate format.

    \param Minimum date in QDate format.
    
    \sa minimumDate
*/
void HbDateTimePicker::setMinimumDate(const QDate& date)
{
    Q_D(HbDateTimePicker);
    setMinimumDateTime(QDateTime(date, d->mMinimumDate.time()));
}

/*!
    Returns maximum date in QDate format.

    \return Maximum Date in QDate format.

    \sa setMaximumDate
*/
QDate HbDateTimePicker::maximumDate()const
{
    Q_D(const HbDateTimePicker);
    return d->mMaximumDate.date();
}

/*!
    Sets maximum \a date in QDate format.

    \param date Maximum date in QDate format.

    \sa maximumDate
*/
void HbDateTimePicker::setMaximumDate(const QDate& date)
{
    Q_D(HbDateTimePicker);
    setMaximumDateTime(QDateTime(date, d->mMaximumDate.time()));
}

/*!
    Sets minimum \a minDate and maximum \a maxDate dates in QDate format.

    \param minDate Minimum date in QDate format.
    \param maxDate Maximum date in QDate format.

    \sa setMinimumDate \sa setMaximumDate
*/
void HbDateTimePicker::setDateRange(const QDate &minDate, const QDate &maxDate)
{
    Q_D(HbDateTimePicker);
    setDateTimeRange(QDateTime(minDate, d->mMinimumDate.time()),
                         QDateTime(maxDate, d->mMaximumDate.time()));
}

/*!
    Returns the current datetime in QDateTime format.

    \return date and time value in QDateTime format.

    \sa setDateTime
*/
QDateTime HbDateTimePicker::dateTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mDateTime;
}

/*!
    Sets the current \a datetime in the form of QDateTime.

    \param datetime in QDateTime format.

    \sa dateTime
*/
void HbDateTimePicker::setDateTime(const QDateTime &datetime)
{
    Q_D(HbDateTimePicker);
    d->setDateTime(datetime);
}

/*!
    Returns minimum date time in QDateTime format.

    \return Minimum date and minimum time in QDateTime format.

    \sa setMinimumDateTime
*/
QDateTime HbDateTimePicker::minimumDateTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMinimumDate;
}

/*!
    Sets minimum \a datetime in QDateTime format.
	<b><i>Note:</i></b> There's no link between Date and time in this API, using this API as of now
	would be similar to using combination of \sa setMinimumDate and \sa setMinimumTime

    \param datetime minimum date and minimum time in QDateTime format.

    \sa minimumDateTime \sa setMinimumDate \sa setMinimumTime
*/
void HbDateTimePicker::setMinimumDateTime(const QDateTime& datetime)
{
    Q_D(HbDateTimePicker);
    d->setMinimumDateTime(datetime);
}

/*!
    Returns maximum date time in QDateTime format.

    \return Maximum date and maximum time in QDateTime format.

    \sa setMaximumDate
*/
QDateTime HbDateTimePicker::maximumDateTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMaximumDate;
}

/*!
    Sets maximum \a datetime in QDateTime format.
	
	<b><i>Note:</i></b> There's no link between Date and time in this API, using this API as of now
	would be similar to using combination of \sa setMaximumDate and \sa setMaximumTime

    \param date Maximum date and maximum time in QDateTime format.

    \sa maximumDateTime \sa setMaximumDate \sa setMaximumTime
*/
void HbDateTimePicker::setMaximumDateTime(const QDateTime& date)
{
    Q_D(HbDateTimePicker);
    d->setMaximumDateTime(date);
}

/*!
    Sets minimum \a minDatetime and maximum \a maxDatetime datetimes in QDateTime format.

	<b><i>Note:</i></b> There's no link between Date and time in this API, using this API as of now
	would be similar to using combination of \sa setMinimumDate \sa setMaximumTime and 
	\sa setMinimumTime, \sa setMaximumTime.

    \param minDateTime minimum date and time in QDateTime format.
    \param maxDateTime maximum date and time in QDateTime format.

    \sa setMinimumDateTime \sa setMaximumDateTime \sa setMinimumDate \sa setMaximumDate
	\sa setMinimumTime \sa setMaximumTime
*/
void HbDateTimePicker::setDateTimeRange(const QDateTime &minDateTime, const QDateTime &maxDateTime)
{
    Q_D(HbDateTimePicker);
    d->setDateTimeRange(minDateTime, maxDateTime);
}

/*!
    Returns the current time in QTime format.

    \return time in QTime format.

    \sa setTime
*/
QTime HbDateTimePicker::time() const
{
    Q_D(const HbDateTimePicker);
    return d->mDateTime.time();
}

/*!
    Sets the current \a time in the form of QTime.

    \param time in QTime format.

    \sa time
*/
void HbDateTimePicker::setTime(const QTime &time)
{
    Q_D(HbDateTimePicker);
    setDateTime(QDateTime(d->mDateTime.date(), time));
}

/*!
    Returns minimum time in QTime format.

    \return Minimum time in QTime format.

    \sa setMinimumTime
*/
QTime HbDateTimePicker::minimumTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMinimumDate.time();
}

/*!
    Sets minimum \a time in QTime format.

    \param time minimum time in QTime format.

    \sa minimumTime
*/
void HbDateTimePicker::setMinimumTime(const QTime& time)
{
    Q_D(HbDateTimePicker);
    setMinimumDateTime(QDateTime(d->mDateTime.date(),time));
}

/*!
    Returns maximum time in QTime format.

    \return maximum time in QTime format.

    \sa setMaximumTime
*/
QTime HbDateTimePicker::maximumTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMaximumDate.time();
}

/*!
    Sets maximum \a time in QTime format.

    \param time maximum time in QTime format

    \sa maximumTime
*/
void HbDateTimePicker::setMaximumTime(const QTime& time)
{
    Q_D(HbDateTimePicker);
    setMaximumDateTime(QDateTime(d->mMaximumDate.date(), time));
}

/*!
    Sets minimum \a minTime and maximum \a maxTime in QTime format.

    \param minTime minimum time in QTime format.
    \param maxTime maximum time in QTime format.

    \sa setMinimumTime \sa setMaximumTime
*/
void HbDateTimePicker::setTimeRange(const QTime &minTime, const QTime &maxTime)
{
    Q_D(HbDateTimePicker);
    setDateTimeRange(QDateTime(d->mMinimumDate.date(), minTime),
                     QDateTime(d->mMinimumDate.date(), maxTime));
}

/*!
  sets the \a interval for the corresponding \a section.

  Note: Only MinuteSection is supported at this time.

  \param section can be a MinuteSection.
  \param interval to be set on each picker.
*/
void HbDateTimePicker::setInterval(QDateTimeEdit::Section section,int interval)
{
    Q_D(HbDateTimePicker);

    //Currently supporting interval for minute section. If other sections should be enabled in future,remove the 
    //below validation for minute section.
    if(section != QDateTimeEdit::MinuteSection){
        d->mIntervals[section] = 1;
        return;
    }

    d->mIntervals[section] = interval;

    if((section == QDateTimeEdit::MinuteSection) && (d->mMinuteModel)){

        d->mMinuteModel->removeRows(0, d->mMinuteModel->rowCount());

        d->resizeModel(d->mMinuteModel, d->mMinimumDate.time().minute(), d->mMaximumDate.time().minute(),
            d->mMinimumDate.time().minute(), d->mMaximumDate.time().minute(),&HbDateTimePickerPrivate::localeMinute, interval);
    }
}

/*!
  returns the \a interval for the corresponding \a section.

  Note: Only MinuteSection is supported at this time.

  \return interval or duration set on a particular \a section.
*/
int HbDateTimePicker::interval(QDateTimeEdit::Section section) const
{
    Q_D(const HbDateTimePicker);

    return d->mIntervals[section];
}

/*!

    \deprecated HbDateTimePicker::primitive(HbStyle::Primitive)
        is deprecated.

    \reimp
*/
QGraphicsItem *HbDateTimePicker::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbDateTimePicker);
    switch(primitive) {
        case HbStyle::P_DateTimePicker_background:
            return d->mBackground;

        case HbStyle::P_DateTimePicker_frame:
            return d->mFrame;

        case HbStyle::P_TumbleView_highlight:
            return d->mHighlight;

        default:
            return HbWidget::primitive(primitive);
    }
}

/*!
    \reimp
*/
void HbDateTimePicker::setGeometry(const QRectF &rect) 
{
    HbWidget::setGeometry(rect);
    updatePrimitives();
}

/*!
    \reimp
*/
void HbDateTimePicker::updatePrimitives()
{
    Q_D(HbDateTimePicker);
    HbStyleOption option;
    initStyleOption(&option);
    if(d->mBackground) {
        style()->updatePrimitive(d->mBackground,HbStyle::P_DateTimePicker_background,&option);
    }
    if(d->mFrame) {
        style()->updatePrimitive(d->mFrame,HbStyle::P_DateTimePicker_frame,&option);
    }

    if(d->mHighlight) {
        style()->updatePrimitive(d->mHighlight,HbStyle::P_TumbleView_highlight,&option);
    }
}

#include "moc_hbdatetimepicker.cpp"
