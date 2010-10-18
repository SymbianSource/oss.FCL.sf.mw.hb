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

#include "hbdatetimepicker.h"
#include "hbdatetimepicker_p.h"
#include "hbstyleoption_p.h"
#include "hbframeitem.h"
#include "hbstyleprimitivedata.h"
#include <hbstyleframeprimitivedata.h>

/*!
    @beta
    \class HbDateTimePicker
    \brief The HbDateTimePicker class provides a widget that has multiple vertical tumblers 
    for selecting a date, time, or date and time by flick, drag and tap gestures.
    
    The number of tumblers depends on the display format. The default constructor creates
    a widget for selecting a date by internally setting the display format to the default 
    locale's short date format. Other constructors enable you to create a widget
    for selecting a time or date and time by passing a QTime or QDateTime variable as an 
    argument. Internally these constructors set the display format to the default locale's
    short time or date and time format, respectively.
    
    Use setDisplayFormat() to set the display format. Be aware that changing the display format 
    can change the widget from one for selecting the date to one for selecting the time, for 
    example.
    
    Looping is enabled in the tumblers. The general appearance of the widget depends on the 
    theme that is in use.
    
    <TABLE border="0">
    <TR><TD>\image html hbdatetimepicker_date.png "Display format: d.MMMM"</TD>
    <TD></TD>
    <TD>\image html hbdatetimepicker_time.png "Display format: h.m.AP"</TD></TR>
    </TABLE>
    
    You can set the range of values available in the tumblers by defining minimum and maximum 
    values or using convenience range functions. If you change the range, if necessary 
    the widget changes the displayed values so that they are within the new range. Users 
    cannot select a value outside the range. One effect of this is that if you set, 
    for example, an afternoon time range, users cannot scroll to the AM value when the 
    display format includes an AM/PM tumbler. The date range is currently independent of the 
    time range.
    
    You can use setInterval() to define the increments for the minute tumbler. This is 
    useful when you want users to be able to set the time in 15 minute increments, for example.
    
    The number of rows displayed depends on how much space is available. To change the number 
    of rows shown when the widget is is in a layout, for example, use 
    QGraphicsLayoutItem::setPreferredHeight() to increase the space available. The widget then
    increases the number of rows to occupy the available space. 
    
    %HbDateTimePicker emits a dateChanged(), timeChanged() or dateTimeChanged() signal when 
    the user selects a new value. The selected date, time, or date and time value is passed
    in a QDate, QTime, or QDateTime argument. 
    
    \section _usecases_hbdatetimepicker Using HbDateTimePicker
    
    The following example demonstrates creating a widget for selecting a date:
    
    \code
    // Use the default constructor to create a widget for selecting a date.
    HbDateTimePicker *datePicker = new HbDateTimePicker();

    // Set the minumum and maximum dates to display.
    datePicker->setMinimumDate(QDate::currentDate());
    datePicker->setMaximumDate(QDate::currentDate().addDays(365));

    // Specify the display format.
    datePicker->setDisplayFormat("dd.MMMM.yyyy");
    \endcode
    
    The following example demonstrates connecting the dateChanged() signal to a slot
    on an object, \a d, of a class (not shown). This has a \c setMeetingDate() function 
    that takes a QDate argument.
    
    \code
    QObject::connect(datePicker, SIGNAL(dateChanged(const QDate&)),
                     d, SLOT(setMeetingDate(const QDate&)));
    \endcode

    The following example demonstrates creating a widget for selecting a time:
    
    \code
    // Create a widget for selecting a time.
    HbDateTimePicker *timePicker = new HbDateTimePicker(QTime());

    // Set the minimum and maximum times using the convenience range method.
    timePicker->setTimeRange(QTime::currentTime(),
                             QTime::currentTime().addSecs(10*60*60));
    
    // Set the time interval to be 15 minutes.
    timePicker->setInterval(QDateTimeEdit::MinuteSection, 15);
    
    // Specify the display format.
    timePicker->setDisplayFormat("hh:mm.ap");
    \endcode
    
    The following example demonstrates creating a widget for selecting a date and time:
    
    \code
    // Create a widget for selecting a date and time.
    HbDateTimePicker *dateTimePicker =
            new HbDateTimePicker(QDateTime::currentDateTime());

    // Specify the display format.
    dateTimePicker->setDisplayFormat("dd.MM.yyyy.hh.mm.ap");
    \endcode
    
    The properties that hold the minimum and maximum values take a QDate, QTime 
    or QDateTime object as an argument. DocML does not directly support properties that
    use QDate, QTime or QDateTime as a parameter. This means that when you add an \b %HbDateTimePicker 
    widget in Application Designer, you must pass the date, time and date and time to these
    properties as a string in a valid ISO 8601 extended format. For example, YYYY-MM-DD for dates, 
    hh:mm:ss for times, and YYYY-MM-DDTHH:MM:SS for combined dates and times. 
    
    Here is an example of the \c maximumTime property defined in DocML:
    
    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'maximumTime' is a QTime property of HbDateTimePicker. -->
        <string name ="displayFormat" value="hh:mm:ss" />
        <string name="maximumTime" value="08:30:00" />
    </widget>
    \endcode
    
    \section _methodgroups_HbDateTimePicker Function groups
    
    \li \b Range: setDateRange(), setTimeRange(), setDateTimeRange()
    \li \b Minimum \b value: setMinimumDate(), setMinimumTime(), setMinimumDateTime()
    \li \b Maximum \b value: setMaximumDate(), setMaximumTime(), setMaximumDateTime()
    \li \b Time \b interval: setInterval()
    
    \sa HbTumbleView, HbExtendedLocale
*/

/*!
    \fn void HbDateTimePicker::dateChanged(const QDate &date);

    This signal is emitted when there is a change in the day, month or year selected.
    The new date is passed in \a date.

*/

/*!
    \fn void HbDateTimePicker::timeChanged(const QTime &time);

    This signal is emitted when there is a change in the hour, minute, second, or AM/PM
    selected. The new time is passed in \a time.

*/

/*!
    \fn void HbDateTimePicker::dateTimeChanged(const QDateTime &datetime);

    This signal is emitted when there is a change in the value selected in any of the
    tumblers in a date and time picker. The new date and time are passed in \a dateTime.

*/

/*!
    Constructs an %HbDateTimePicker object with the locale's default short date display format 
    and with parent \a parent.

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
    Constructs an %HbDateTimePicker object with the locale's default short date and time 
    display format and with parent \a parent.
     
*/
HbDateTimePicker::HbDateTimePicker(const QDateTime &datetime, QGraphicsItem *parent ):
HbWidget(*new HbDateTimePickerPrivate, parent)
{
    Q_D(HbDateTimePicker);

    d->init(QVariant::DateTime);
    setDateTime(datetime);
}

/*!
    Constructs an %HbDateTimePicker object with the locale's default short date 
    display format and with parent \a parent.

*/
HbDateTimePicker::HbDateTimePicker(const QDate &date, QGraphicsItem *parent ):
HbWidget(*new HbDateTimePickerPrivate, parent)
{
    Q_D(HbDateTimePicker);

    d->init(QVariant::Date);
    setDate(date);
}

/*!
    Constructs an %HbDateTimePicker object with the locale's default time 
    display format and with parent \a parent.
    
*/
HbDateTimePicker::HbDateTimePicker(const QTime &time, QGraphicsItem *pParent ):
HbWidget(*new HbDateTimePickerPrivate, pParent)
{
    Q_D(HbDateTimePicker);

    d->init(QVariant::Time);
    setTime(time);
}

/*!
    Protected constructor for derivations.
    
    The default mode is date and time. If another mode is required, set the mDateTimeMode 
    variable explicitly in the constructor of the class derived from HbDateTimePicker. This 
    does not set the default date and time value. 

    \sa setDateTime(), setTime(), setDate()
*/
HbDateTimePicker::HbDateTimePicker(HbDateTimePickerPrivate &dd, QGraphicsItem *parent):
HbWidget(dd, parent)
{
    Q_D(HbDateTimePicker);

    d->init(QVariant::DateTime);
}


bool HbDateTimePicker::event(QEvent *e)
{   
    Q_D(HbDateTimePicker);
    
    bool result = HbWidget::event(e);
    if(e->type() == d->mFormatEventType){
        d->processFormatEvent();
    }
    else if (e->type()==QEvent::LayoutRequest) {
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
    Returns the current display format as a QString value.

    \sa setDisplayFormat()
 */
QString HbDateTimePicker::displayFormat() const
{
    Q_D(const HbDateTimePicker);
    return d->mFormat;
}

/*!
    Sets the display format using a suitable combination of the following expressions 
    in QString format.
    
    These expressions may be used for the date:

    <TABLE>
    <TR><TD><b><b>Expression </b></TD><TD><b>Output</b> </TD></b></TR>
    <TR><TD> d </TD><TD> The day as a number without a leading zero (1 to 31).</TD></TR>
    <TR><TD> dd </TD><TD> The day as a number with a leading zero (01 to 31). </TD></TR>
    <TR><TD> ddd </TD><TD>The abbreviated localized day name (such as <i>Mon</i> to <i>Sun</i>).
            Uses QDate::shortDayName().</TD></TR>
    <TR><TD> dddd </TD><TD>The long localized day name (such as <i>Monday</i> to <i>Sunday</i>).
            Uses QDate::longDayName().</TD></TR>
    <TR><TD> M </TD><TD> The month as a number without a leading zero (1 to 12).</TD></TR>
    <TR><TD> MM </TD><TD> The month as a number with a leading zero (01 to 12).</TD></TR>
    <TR><TD> MMM </TD><TD>The abbreviated localized month name (such as <i>Jan</i> to <i>Dec</i>).
            Uses QDate::shortMonthName().</TD></TR>
    <TR><TD> MMMM </TD><TD>The long localized month name (such as <i>January</i> to <i>December</i>).
            Uses QDate::longMonthName().</TD></TR>
    <TR><TD> yy </TD><TD>The year as a two digit number (00 to 99).</TD></TR>
    <TR><TD> yyyy </TD><TD>The year as a four digit number. If the year is negative,
            a minus sign is prepended.</TD></TR>
    </TABLE>
    
    These expressions may be used for the time:

    <TABLE>
    <TR><TD><b><b>Expression </b></TD><TD><b>Output</b> </TD></b></TR>
    <TR><TD> h </TD><TD>The hour without a leading zero (0 to 23 or 1 to 12).</TD></TR>
    <TR><TD> hh </TD><TD>The hour with a leading zero (00 to 23 or 01 to 12). </TD></TR>
    <TR><TD>H   </TD><TD>The hour without a leading zero (0 to 23, even with AM/PM display).</TD></TR>
    <TR><TD>HH  </TD><TD>The hour with a leading zero (00 to 23, even with AM/PM display).</TD></TR>
    <TR><TD> m </TD><TD>The minute without a leading zero (0 to 59).</TD></TR>
    <TR><TD> mm </TD><TD>The minute with a leading zero (00 to 59).</TD></TR>
    <TR><TD> s </TD><TD>The second without a leading zero (0 to 59).</TD></TR>
    <TR><TD> ss </TD><TD>The second with a leading zero (00 to 59).</TD></TR>
    <TR><TD>AP or A </TD><TD>Displays AM and PM.</i></TD></TR>
    <TR><TD>ap or a </TD><TD>Displays am and pm.</TD></TR>
    </TABLE>

    \note Currently you must include separators in the format; for example, 'dd.mm.yy'. 
    \note Use max upto 4 fields like hh:mm:ss:ap in the format. More than 4 fields will not be usable on typical mobile display.

    \sa displayFormat()
*/
void HbDateTimePicker::setDisplayFormat(const QString &format)
{
    Q_D(HbDateTimePicker);

    if(d->isFormatValid(format)){
        d->mFormat = format;
            d->postFormatEvent();
    }
}

/*!
    Returns the currently selected date in QDate format.
    
    \note DocML does not directly support properties that use QDate as a parameter. For these 
    properties, pass the date as a string in a valid ISO 8601 extended format,
    such as YYYY-MM-DD.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'date' is a QDate property of HbDateTimePicker. -->
        <string name="date" value="02-02-15" />
    </widget>
    \endcode

    \sa setDate()
*/
QDate HbDateTimePicker::date() const
{
    Q_D(const HbDateTimePicker);

    return d->mDateTime.date();
}

/*!
    Sets the current date to \a date.

    \param date The date in QDate format.

    \sa date()
*/
void HbDateTimePicker::setDate(const QDate& date)
{
    Q_D(HbDateTimePicker);
    setDateTime(QDateTime(date,d->mDateTime.time()));
}

/*!
    Returns the minimum date in QDate format.
    
    \note DocML does not directly support properties that use QDate as a parameter. For these 
    properties, pass the date as a string in a valid ISO 8601 extended format, such as YYYY-MM-DD.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'minimumDate' is a QDate property of HbDateTimePicker. -->
        <string name="minimumDate" value="02-02-15" />
    </widget>
    \endcode
    
    \sa setMinimumDate()
*/
QDate HbDateTimePicker::minimumDate()const
{
    Q_D(const HbDateTimePicker);
    return d->mMinimumDate.date();
}

/*!
    Sets the minimum date to \a date. 

    \param date The minimum date in QDate format.
    
    \sa minimumDate(), setDateRange()
*/
void HbDateTimePicker::setMinimumDate(const QDate& date)
{
    Q_D(HbDateTimePicker);
    setMinimumDateTime(QDateTime(date, d->mMinimumDate.time()));
}

/*!
    Returns the maximum date in QDate format.
    
    \note DocML does not directly support properties that use QDate as a parameter. For these 
    properties, pass the date as a string in a valid ISO 8601 extended format, such as YYYY-MM-DD.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'maximumDate' is a QDate property of HbDateTimePicker. -->
        <string name="maximumDate" value="02-02-15" />
    </widget>
    \endcode

    \sa setMaximumDate()
*/
QDate HbDateTimePicker::maximumDate()const
{
    Q_D(const HbDateTimePicker);
    return d->mMaximumDate.date();
}

/*!
    Sets the maximum date to \a date. 

    \param date The maximum date in QDate format.

    \sa maximumDate(), setDateRange()
*/
void HbDateTimePicker::setMaximumDate(const QDate& date)
{
    Q_D(HbDateTimePicker);
    setMaximumDateTime(QDateTime(date, d->mMaximumDate.time()));
}

/*!
    Sets the minimum and maximum dates.

    \param minDate The minimum date in QDate format.
    \param maxDate The maximum date in QDate format.

    \sa setMinimumDate(), setMaximumDate()
*/
void HbDateTimePicker::setDateRange(const QDate &minDate, const QDate &maxDate)
{
    Q_D(HbDateTimePicker);
    setDateTimeRange(QDateTime(minDate, d->mMinimumDate.time()),
                         QDateTime(maxDate, d->mMaximumDate.time()));
}

/*!
    Returns the currently selected date and time in QDateTime format.
    
    \note DocML does not directly support properties that use QDate, QTime or QDateTime as a 
    parameter. For these properties, pass the date, time or date and time as a string in a 
    valid ISO 8601 extended format. For example, YYYY-MM-DD for dates and YYYY-MM-DDTHH:MM:SS
    for combined dates and times.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'dateTime' is a QDateTime property of HbDateTimePicker. -->
        <string name="dateTime" value="02-02-15T02-15-30" />
    </widget>
    \endcode

    \sa setDateTime()
*/
QDateTime HbDateTimePicker::dateTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mDateTime;
}

/*!
    Sets the current date and time.

    \param datetime The date and time in QDateTime format.

    \sa dateTime()
*/
void HbDateTimePicker::setDateTime(const QDateTime &datetime)
{
    Q_D(HbDateTimePicker);
    d->setDateTime(datetime);
}

/*!
    Returns the minimum date and time in QDateTime format.
    
    \note DocML does not directly support properties that use QDate, QTime or QDateTime as a 
    parameter. For these properties, pass the date, time or date and time as a string in a 
    valid ISO 8601 extended format. For example, YYYY-MM-DD for dates and YYYY-MM-DDTHH:MM:SS
    for combined dates and times.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'minimumDateTime' is a QDateTime property of HbDateTimePicker. -->
        <string name="minimumDateTime" value="02-02-15T02-15-30" />
    </widget>
    \endcode

    \sa setMinimumDateTime()
*/
QDateTime HbDateTimePicker::minimumDateTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMinimumDate;
}

/*!
    Sets the minimum date and time.

    \note The date and time are not linked in this API. This means that 
    calling this function is similar to calling setMinimumDate() and setMinimumTime().

    \param datetime The minimum date and time in QDateTime format.

    \sa minimumDateTime(), setDateTimeRange()
*/
void HbDateTimePicker::setMinimumDateTime(const QDateTime& datetime)
{
    Q_D(HbDateTimePicker);
    d->setMinimumDateTime(datetime);
}

/*!
    Returns the maximum date and time in QDateTime format.
    
    \note DocML does not directly support properties that use QDate, QTime or
    QDateTime as parameters. For these properties, pass the date, time, or date and time
    as a string in a valid ISO 8601 extended format. For example, YYYY-MM-DD for dates
    or YYYY-MM-DDTHH:MM:SS for combined dates and times.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'maximumDateTime' is a QDateTime property of HbDateTimePicker. -->
        <string name="maximumDateTime" value="02-02-15T02-15-30" />
    </widget>
    \endcode

    \sa setMaximumDateTime()
*/
QDateTime HbDateTimePicker::maximumDateTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMaximumDate;
}

/*!
    Sets the maximum date and time.
    
    \note The date and time are not linked in this API. This means that 
    calling this function is similar to calling setMaximumDate() and setMaximumTime().

    \param date The maximum date and time in QDateTime format.

    \sa maximumDateTime(), setDateTimeRange()
*/
void HbDateTimePicker::setMaximumDateTime(const QDateTime& date)
{
    Q_D(HbDateTimePicker);
    d->setMaximumDateTime(date);
}

/*!
    Sets the minimum and maximum date and time.
    
    \note The date and time are not linked in this API. This means that calling 
    this function is equivalent to calling setMinimumDate(), setMaximumDate(), 
    setMinimumTime(), and setMaximumTime().

    \param minDateTime The minimum date and time in QDateTime format.
    \param maxDateTime The maximum date and time in QDateTime format.

    \sa setMinimumDateTime(), setMaximumDateTime()
*/
void HbDateTimePicker::setDateTimeRange(const QDateTime &minDateTime, const QDateTime &maxDateTime)
{
    Q_D(HbDateTimePicker);
    d->setDateTimeRange(minDateTime, maxDateTime);
}

/*!
    Returns the currently selected time in QTime format.
    
    \note DocML does not directly support properties that use QTime as a parameter.
    For these properties, pass the time as a string in a valid ISO 8601 extended format; 
    for example, HH:MM:SS.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'time' is a QTime property of HbDateTimePicker. -->
        <string name="time" value="02-15-30" />
    </widget>
    \endcode

    \sa setTime()
*/
QTime HbDateTimePicker::time() const
{
    Q_D(const HbDateTimePicker);
    return d->mDateTime.time();
}

/*!
    Sets the current time to \a time.

    \param time The time in QTime format.

    \sa time()
*/
void HbDateTimePicker::setTime(const QTime &time)
{
    Q_D(HbDateTimePicker);
    setDateTime(QDateTime(d->mDateTime.date(), time));
}

/*!
    Returns the minimum time in QTime format.
    
    \note DocML does not directly support properties that use QTime as a parameter.
    For these properties, pass the time as a string in a valid ISO 8601 extended format; 
    for example, HH:MM:SS.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'minimumTime' is a QTime property of HbDateTimePicker. -->
        <string name="minimumTime" value="08-00-00" />
    </widget>
    \endcode

    \sa setMinimumTime()
*/
QTime HbDateTimePicker::minimumTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMinimumDate.time();
}

/*!
    Sets the minimum time to \a time.

    \param time The minimum time in QTime format.

    \sa minimumTime(), setTimeRange()
*/
void HbDateTimePicker::setMinimumTime(const QTime& time)
{
    Q_D(HbDateTimePicker);
    setMinimumDateTime(QDateTime(d->mDateTime.date(),time));
}

/*!
    Returns the maximum time in QTime format.

    \note DocML does not directly support properties that use QTime as a parameter.
    For these properties, pass the time as a string in a valid ISO 8601 extended format; 
    for example, HH:MM:SS.

    \code
    <widget name="t:dtp" type="HbDateTimePicker">
        <!-- 'maximumTime' is a QTime property of HbDateTimePicker. -->
        <string name="maximumTime" value="18-30-00" />
    </widget>
    \endcode

    \sa setMaximumTime()
*/
QTime HbDateTimePicker::maximumTime()const
{
    Q_D(const HbDateTimePicker);
    return d->mMaximumDate.time();
}

/*!
    Sets the maximum time to \a time.

    \param time The maximum time in QTime format.

    \sa maximumTime(), setTimeRange()
*/
void HbDateTimePicker::setMaximumTime(const QTime& time)
{
    Q_D(HbDateTimePicker);
    setMaximumDateTime(QDateTime(d->mMaximumDate.date(), time));
}

/*!
    Sets the minimum and maximum time. 

    \param minTime The minimum time in QTime format.
    \param maxTime The maximum time in QTime format.

    \sa setMinimumTime(), setMaximumTime()
*/
void HbDateTimePicker::setTimeRange(const QTime &minTime, const QTime &maxTime)
{
    Q_D(HbDateTimePicker);
    setDateTimeRange(QDateTime(d->mMinimumDate.date(), minTime),
                     QDateTime(d->mMinimumDate.date(), maxTime));
}

/*!
    Sets the interval or periodic gap for a tumbler. Currently this is
    supported for minute tumblers only. 

    \param section This identifies the tumbler. This must be QDateTimeEdit::MinuteSection.
    \param interval The interval to set on the tumbler. This must be a divisor of 60; that
                    is, 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, or 30.
    
    \sa interval()
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

    if(60 % interval)
    {
        return;
    }

    d->mIntervals[section] = interval;

    //trigger minute range change
    int start=0,end=0;
    if(d->mMinutePicker) {
        start=d->mMinuteOffset;
        end=start+d->mMinuteModel->rowCount()-1;
        if(d->isMinimumHour() )  {
            start = d->mMinimumDate.time().minute();
        } else {               
            if((d->mIntervals[QDateTimeEdit::MinuteSection]!=1) && (d->mIntervals[section]>0)) {
                start = d->mMinimumDate.time().minute()%d->mIntervals[section];
            } else {
                start = 0;
            }
        }
        if(d->isMaximumHour()) {
            end = d->mMaximumDate.time().minute();
        } else {
            end = 59;
        }

        d->setMinuteRange(start,end);
    }
}

/*!
    Returns the interval set for a tumbler. Currently this is supported for 
    minute tumblers only. 

    \param section Identifies the tumbler. This must be QDateTimeEdit::MinuteSection.

    \sa setInterval()
*/
int HbDateTimePicker::interval(QDateTimeEdit::Section section) const
{
    Q_D(const HbDateTimePicker);

    return d->mIntervals[section];
}

/*!
    Reimplemented from QGraphicsWidget::setGeometry().
*/
void HbDateTimePicker::setGeometry(const QRectF &rect) 
{
    HbWidget::setGeometry(rect);
    updatePrimitives();
      Q_D(HbDateTimePicker);

    for(int i=0;i<d->mParser.mSectionNodes.count();i++) {
        switch(d->mParser.mSectionNodes[i].type) {
            case HbDateTimeParser::AmPmSection:
                if(!d->mAmPmPicker){
                    break;
                }
                break;

            case HbDateTimeParser::DaySection:
            case HbDateTimeParser::DayOfWeekSection:
                if(!d->mDayPicker){
                    break;
                }
                d->mLabelDay->setPreferredWidth(d->mDayPicker->preferredSize().width());
               break;

            case HbDateTimeParser::MonthSection:
                if(!d->mMonthPicker){
                    break;
                }
                d->mLabelMonth->setPreferredWidth(d->mMonthPicker->preferredSize().width());
               break;

            case HbDateTimeParser::YearSection:
            case HbDateTimeParser::YearSection2Digits:
                if(!d->mYearPicker){
                    break;
                }

                d->mLabelYear->setPreferredWidth(d->mYearPicker->preferredSize().width());
               break;

            case HbDateTimeParser::SecondSection:
                if(!d->mSecondPicker){
                    break;
                }

                d->mLabelSecond->setPreferredWidth(d->mSecondPicker->preferredSize().width());
               break;

            case HbDateTimeParser::MinuteSection:
                if(!d->mMinutePicker){
                    break;
                }
               d->mLabelMinute->setPreferredWidth(d->mMinutePicker->preferredSize().width());
               break;

            case HbDateTimeParser::Hour12Section:
            case HbDateTimeParser::Hour24Section:
                if(!d->mHourPicker){
                    break;
                }
                d->mLabelHour->setPreferredWidth(d->mHourPicker->preferredSize().width());
               break;

            default:
                break;
        }
    }
 
}

void HbDateTimePicker::initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive)
{
    HbWidget::initPrimitiveData(primitiveData, primitive);
    QString itemName = HbStyle::itemName(primitive);
    if(itemName == QLatin1String("background")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_fr_tumbler_bg");
        frameItem->frameType = HbFrameDrawer::NinePieces;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(-5);
    }

    if(itemName == QLatin1String("frame")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_fr_tumbler_overlay");
        frameItem->frameType = HbFrameDrawer::NinePieces;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(1);
        
    }
    if(itemName == QLatin1String("highlight")) {
        HbStyleFramePrimitiveData *frameItem  = hbstyleprimitivedata_cast<HbStyleFramePrimitiveData*>(primitiveData);
        frameItem->frameGraphicsName= QLatin1String("qtg_fr_tumbler_highlight_pri");
        frameItem->frameType = HbFrameDrawer::ThreePiecesHorizontal;
        (const_cast<QGraphicsObject *> (primitive))->setZValue(-1);
        
    }

}

void HbDateTimePicker::updatePrimitives()
{
    Q_D(HbDateTimePicker);
    HbWidget::updatePrimitives();
    
    if(d->mBackground) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data,d->mBackground);
        style()->updatePrimitive(d->mBackground,&data,this);
        if(!d->mLastAdded.isNull()){
            QRectF geom = ((HbFrameItem*)d->mBackground)->geometry();
            ((HbFrameItem*)d->mBackground)->setGeometry(geom.x(), d->mLastAdded->pos().y(), geom.width(), d->mLastAdded->size().height());
        }
    }

    if(d->mFrame) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData(&data,d->mFrame);
        style()->updatePrimitive(d->mFrame,&data,this);
        if(!d->mLastAdded.isNull()){
            QRectF geom = ((HbFrameItem*)d->mFrame)->geometry();
            ((HbFrameItem*)d->mFrame)->setGeometry(geom.x(), d->mLastAdded->pos().y(), geom.width(), d->mLastAdded->size().height());
        }
    }

    if(d->mHighlight) {
        HbStyleFramePrimitiveData data;
        initPrimitiveData (&data,d->mHighlight);
        style()->updatePrimitive(d->mHighlight,&data,this);
        if(!d->mLastAdded.isNull()){
            qreal top = d->mLastAdded->pos().y() + d->mLastAdded->size().height()/2 - ((HbFrameItem*)d->mHighlight)->geometry().height()/2;
            d->mHighlight->setPos(d->mHighlight->pos().x(), top);
        }

    }
}

QGraphicsItem *HbDateTimePicker::primitive(const QString &itemName) const
{
    Q_D(const HbDateTimePicker);

    if(!itemName.compare(QString("background"))){
        return d->mBackground;
    }
    if(!itemName.compare(QString("frame"))){
        return d->mFrame;
    }
    if(!itemName.compare(QString("highlight"))){
        return d->mHighlight;
    }

    return HbWidget::primitive(itemName);
}

#include "moc_hbdatetimepicker.cpp"
