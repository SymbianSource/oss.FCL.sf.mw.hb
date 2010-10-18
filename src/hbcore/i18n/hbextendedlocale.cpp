/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include <qglobal.h>
#include <QDate>
#include <QtDebug>
#include <qvariant.h>
#if defined(Q_OS_SYMBIAN)
#include <e32std.h>
#include <e32def.h>
#include <e32des16.h>
#include <e32const.h>
#include <e32base.h> 
#endif

#include "hbextendedlocale.h"

static const int CURRENCY_FORMAT_SIZE_STEP = 8; 

#if defined(Q_OS_SYMBIAN)

static TExtendedLocale _symbianLocale;

QT_BEGIN_NAMESPACE

TPtrC QString2TPtrC( const QString& string )
{
    return TPtrC16(static_cast<const TUint16*>(string.utf16()), string.length());
}
HBufC* QString2HBufC(const QString& aString)
{
    HBufC *buffer;
#ifdef QT_NO_UNICODE
    TPtrC8 ptr(reinterpret_cast<const TUint8*>(aString.toLocal8Bit().constData()));
#else
    TPtrC16 ptr(QString2TPtrC(aString));
#endif
    buffer = HBufC::New(ptr.Length());
    Q_CHECK_PTR(buffer);
    buffer->Des().Copy(ptr);
    return buffer;
}

QString TDesC2QString(const TDesC& aDescriptor)
{
#ifdef QT_NO_UNICODE
    return QString::fromLocal8Bit(aDescriptor.Ptr(), aDescriptor.Length());
#else
    return QString::fromUtf16(aDescriptor.Ptr(), aDescriptor.Length());
#endif
}

QT_END_NAMESPACE

#endif

/*!
    @stable
    @hbcore
    \class HbExtendedLocale
    \brief The HbExtendedLocale class is an extension class to Qlocale.
    
    Your application must be able to format language- and region-dependent 
    elements correctly (for example, dates, times, and currencies) based on the 
    active locale settings. HbExtendedLocale is inherited from the QLocale 
    class, and extends the %Qt locale model by providing access to Symbian 
    platform locale data. The HbStringUtil, HbNumberGrouping, and HbLocaleUtil 
    classes also provide some locale-related functionality.
    
    In Symbian devices, users can modify some locale settings, such as the time 
    and date format and the separator characters used, or the digits used. If 
    the user changes the device language, the changes made for the previous 
    language are saved, and will be restored when the user switches back to that 
    language. For example, if the user often switches between two languages that 
    use a different date format, the user can define a different date format to 
    be used for each device language. The device may also modify some locale-
    related settings automatically, for example, update the date from the 
    network due to a daylight savings change, or if the user moves to a 
    different time zone.
    
    With HbExtendedLocale, you access the active locale data in the device, 
    whether it is the predefined locale data provided by the platform, or data 
    modified by the user or the device.
    
    HbExtendedLocale also provides functions for mapping Symbian language codes 
    to corresponding ISO and RFC 3066 codes.
    
    \section _hbextendedlocale_localedependentelements Language- and country-dependent elements to be formatted
    
    \subsection _hbextendedlocale_calendarformatting Calendar
    
    The Western (Gregorian) calendar is perhaps the most common calendar system 
    in use, but there are also other calendars in use worldwide, such as the 
    Japanese, Buddhist era, Hijri, Hebrew lunar, and Taiwan calendars.

    The following calendars are supported:
    \li Gregorian calendar
    \li Chinese lunar calendar
    \li  Vietnamese lunar calendar
    \li  Thai Buddhist calendar
    
    The Symbian platform also provides an API for integrating an Arabic 
    calendar.
    
    When implementing calendar formatting, take into account the use of 
    different calendars and the differences they may have. For example, there 
    may be differences in the year values, the length of the year and months, 
    whether or not week numbers are used, and the way leap years are handled. 
    Even the same calendar may have locale-specific differences, for example, 
    the first day of the week in the Gregorian calendar (usually Sunday or 
    Monday). Handling week numbers locale-dependently is not supported.
    
    For calendar formatting, use HbExtendedLocale workDays(), setWorkDays(), 
    startOfWeek() and setStartOfWeek().
    
    \subsection _hbextendedlocale_datetimeformatting Date and time
    
    Date and time formatting often varies per locale. Examples of differences 
    are the order of the day, month, and year components in the date, the first 
    day of the week, whether a 12-hour or 24-hour clock is used, and the 
    separators used in the date and time formats.

    For example, January 30th in 2009 can be displayed in the following ways:
    - Danish: 30-01-09
    - Hungarian: 2009.01.30.
    - Swedish: 09/01/30
    - English (US): 1/30/09
    
    And examples of differences in time formatting:
    - USA: 9:35 pm
    - France: 21:35
    - Canada (French): 21 h 35
    - Finland: 21.35

    It is possible to also include seconds in the time format, but Symbian-based 
    applications usually only use hours and minutes.
    
    To format dates, use:
    
    \li HbExtendedLocale format(const QDate &, const QString &), dateStyle(), 
    setDateStyle(), dateSeparator(), setDateSeparator(), symbianDateTimeToQt()
    
    \li QLocale::dayName(), QLocale::monthName()
    
    To format times, use:
    
    \li HbExtendedLocale format(const QTime &, const QString &), timeStyle(), 
    setTimeStyle(), timeSeparator(), setTimeSeparator(), amPmSymbolPosition(), 
    setAmPmSymbolPosition(), amPmSpace(), setAmPmSpace(), 
    homeDaylightSavingZone(), homeHasDaylightSavingOn(), universalTimeOffset(), 
    symbianDateTimeToQt()
    
    \li QLocale::amText(), QLocale::pmText()
    
    \subsection _hbextendedlocale_numberformatting Numbers
    
    Possible differences in number formatting include the digit type used, and 
    the decimal separator.

    For example:
    - Finnish: 13,5
    - US English: 38.6
    - Japanese: 1,300.500
    
    To format numbers, use:
     
    \li HbExtendedLocale setDecimalPoint(), setGroupSeparator(), setZeroDigit()
    
    \li QLocale::decimalPoint(), QLocale::groupSeparator(), QLocale::zeroDigit()
    
    \li HbStringUtil::convertDigits(), HbStringUtil::convertDigitsTo()
    
    \li HbNumberGrouping::formatGeneric()
    
    \subsection _hbextendedlocale_currencyformatting Currencies
    
    Possible differences in the currency formats include the currency symbol and 
    its position, the number of characters used in the currency symbol, and the 
    formatting of negative currencies.

    For example:
    - France: 20,15 €
    - Denmark: kr-20,15 - negative currency is indicated by a minus sign
    - United States: ($20.15) - negative currency is indicated by brackets
    
    To format currencies, use:
    
    \li HbExtendedLocale formatCurrency(), currencySymbol(), 
    setCurrencySymbol(), currencySpace(), setCurrencySpace(), 
    currencySymbolPosition(), setCurrencySymbolPosition(), 
    currencyDecimalPlaces(), setCurrencyDecimalPlaces(), 
    currencyTriadsAllowed(), setCurrencyTriadsAllowed(), 
    negativeCurrencyFormat(), setNegativeCurrencyFormat(), 
    negativeCurrencySpace(), setNegativeCurrencySpace(), 
    negativeCurrencySymbolOpposite(), setNegativeCurrencySymbolOpposite()
    
    \li QLocale::decimalPoint(), QLocale::groupSeparator()
    
    \li HbNumberGrouping::formatCurrency()
    
    \subsection _hbextendedlocale_unitofmeasurementformatting Units of measurement
    
    Units of measurement must be formatted correctly using either metric or 
    imperial units.
    
    To format units of measurement, use:
    
    \li HbExtendedLocale unitsDistanceLong(), setUnitsDistanceLong(), 
    unitsDistanceShort(), setUnitsDistanceShort(), setUnitsGeneral()
    
    \li QLocale::measurementSystem(), QLocale::groupSeparator(), 
    QLocale::decimalPoint()
    
    \note To display locale-specific names of measurement units (for example, 
    kB), you need to localize this data in the <tt>.ts</tt> translations files, 
    and use the global hbTrId() function to display the correct translation. 
    Alternatively, you can enable users to set the unit. For user-changeable 
    settings, there must be a localizable default.

    \section _usecases_hbextendedlocale Using the HbExtendedLocale class

    Examples:

    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,1}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,2}

    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,3}

    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,4}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,5}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,6}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,7}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,8}

    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,9}

    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,10}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,11}
    
    \snippet{unittest_hbextendedlocale/unittest_hbextendedlocale.cpp,12}
    
    \sa QLocale, HbStringUtil, HbNumberGrouping, HbLocaleUtil
*/

/*!
    
    Returns the date separator from the system locale. The separator is one of 
    the four characters (space, '.', '-' or '/') used to separate the day, month 
    and year components in a date expression.

    If the current locale uses no separator in the specified position, an empty 
    character is returned.

    \param index The position of the separator in the date expression (0-3).
    - \c 0 - at the beginning of the expression
    - \c 1 - between the first and second component
    - \c 2 - between the second and third component
    - \c 3 - at the end of the expression
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, returns '/' for index values 0-3, otherwise an empty QChar.
    
    \sa setDateSeparator()

 */
QChar HbExtendedLocale::dateSeparator( int index ) const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();    
    TChar val = _symbianLocale.GetLocale()->DateSeparator(index);
    return QChar(val);
#else
    if ( index > 0 && index < 3 ) {
        return QLatin1Char('/');
    } else {
        return QChar();
    }
#endif
}

/*!

    Sets the date separator.

    \param ch The separator character to set.
    \param index The position of the separator in the date expression (0-3).
 
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa dateSeparator()
 */
bool HbExtendedLocale::setDateSeparator( const QChar ch, const int index )
{
#if defined(Q_OS_SYMBIAN)
    if ( index < 0 || index > 3 ) {
        return false;
    }
    _symbianLocale.LoadSystemSettings();    
    TChar symbianch( ch.unicode() );
    _symbianLocale.GetLocale()->SetDateSeparator(symbianch, index);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(ch);
    Q_UNUSED(index);
    return false;
#endif
}

/*!
    
    Returns the time separator from the system locale. The separator is one of 
    the four characters (':', '.', '-' and '/') used to separate the hour, 
    minute and second components in a time expression.

    \param index The position of the separator in the time expression (0-3).
    - \c 0 - at the beginning of the expression
    - \c 1 - between the first and second component
    - \c 2 - between the second and third component
    - \c 3 - at the end of the expression
        
    \attention Only fully implemented on the Symbian platform. On other 
    platforms,  returns ':' for index values 0-3, otherwise an empty QChar.
    
    \sa setTimeSeparator()

 */
QChar HbExtendedLocale::timeSeparator( int index ) const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TChar val = _symbianLocale.GetLocale()->TimeSeparator(index);
    return QChar(val);
#else
    if ( index > 0 && index < 3 ) {
        return QLatin1Char(':');
    } else {
        return QChar();
    }
#endif
}

/*!

    Sets the time separator.

    \param ch The character to set.
    \param index The position of the separator in the time expression (0-3).
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa timeSeparator()
 */
bool HbExtendedLocale::setTimeSeparator( const QChar ch, const int index )
{
#if defined(Q_OS_SYMBIAN)
    if ( index < 0 || index > 3 ) {
        return false;
    }
    
    _symbianLocale.LoadSystemSettings();
    TChar symbianch( ch.unicode() );
    _symbianLocale.GetLocale()->SetTimeSeparator(symbianch, index);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(ch);
    Q_UNUSED(index);
    return false;
#endif
}

/*! 
    \enum HbExtendedLocale::DateStyle
    
    Defines the order of the date components in a date.
    
    \sa dateStyle(), setDateStyle()
*/

/*! 
    \var HbExtendedLocale::American
    
    The format 'month-day-year' (mm/dd/yyyy), which is mostly used in the U.S. 
    and Canada.

*/

/*! 
    \var HbExtendedLocale::European
    The most commonly used format 'day-month-year' (dd/mm/yyyy).
*/

/*! 
    \var HbExtendedLocale::Japanese
    
    The format 'year-month-day' (yyyy/mm/dd), which is used, for example, in the 
    Japanese, Chinese and Swedish locales.
    
 */

/*!
    
    Returns the date style from the system locale according to 
    HbExtendedLocale::DateStyle.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::American.
    
    \sa setDateStyle()
    
 */
HbExtendedLocale::DateStyle HbExtendedLocale::dateStyle() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TDateFormat val = _symbianLocale.GetLocale()->DateFormat();

    switch ( val ) {
    case EDateAmerican:
        // mm/dd/yyyy
        return HbExtendedLocale::American;
    case EDateEuropean:
        // dd/mm/yyyy
        return HbExtendedLocale::European;
    case EDateJapanese:
        // yyyy/mm/dd
        return HbExtendedLocale::Japanese;
    }
    return HbExtendedLocale::American;
#else
    return HbExtendedLocale::American;
#endif
}

/*!
    
    Sets the date style specified in \a style to the system locale.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa dateStyle(), HbExtendedLocale::DateStyle
 */
bool HbExtendedLocale::setDateStyle( const DateStyle style )
{
#if defined(Q_OS_SYMBIAN)
    TDateFormat set;

    switch ( style ) {
    default:
    case HbExtendedLocale::American:
        set = EDateAmerican;
        break;
    case HbExtendedLocale::European:
        set = EDateEuropean;
        break;
    case HbExtendedLocale::Japanese:
        set = EDateJapanese;
        break;
    }

    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetDateFormat(set);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(style);
    return false;
#endif
}


/*! 
    \enum HbExtendedLocale::TimeStyle

    Defines whether the 12-hour or 24-hour clock is used in the time expression.
    
    \sa timeStyle(), setTimeStyle()
*/

/*!
    \var HbExtendedLocale::Time12
    12-hour clock.
*/

/*!
    \var HbExtendedLocale::Time24
    24-hour clock.
*/

/*!
    
    Returns the time style from the system locale according to 
    HbExtendedLocale::TimeStyle.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::Time12.
    
    \sa setTimeStyle()
    
 */
HbExtendedLocale::TimeStyle HbExtendedLocale::timeStyle() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TTimeFormat val = _symbianLocale.GetLocale()->TimeFormat();

    switch ( val ) {
    case ETime12:
        return HbExtendedLocale::Time12;
    case ETime24:
        return HbExtendedLocale::Time24;
    }
    return HbExtendedLocale::Time12;
#else
    return HbExtendedLocale::Time12;
#endif
}

/*!

    Sets the time style specified in \a style to the system locale.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa timeStyle(), HbExtendedLocale::TimeStyle

 */
bool HbExtendedLocale::setTimeStyle( const TimeStyle style )
{
#if defined(Q_OS_SYMBIAN)
    TTimeFormat set;
    switch ( style ) {
    default:
    case HbExtendedLocale::Time12:
        set = ETime12;
        break;
    case HbExtendedLocale::Time24:
        set = ETime24;
        break;
    }
    
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetTimeFormat(set);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(style);
    return false;
#endif
}


/*!
    
    Returns \c true if a space is inserted between the time expression and the 
    am/pm symbol (preceding or trailing). If no space is inserted, returns \c 
    false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c true.
    
    \sa setAmPmSpace()
 */
bool HbExtendedLocale::amPmSpace() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return _symbianLocale.GetLocale()->AmPmSpaceBetween();
#else
    return true;
#endif
}

/*!
    
    Sets the space inserting in time expressions to the system locale as 
    specified in \a space.
    
    \param space Specify \c true if a space is inserted between the time and the 
    am/pm text (preceding or trailing), and \c false if a space is not inserted.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa amPmSpace()
 */
bool HbExtendedLocale::setAmPmSpace( const bool space )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetAmPmSpaceBetween(space);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(space);
    return false;
#endif
}

/*! 
    \enum HbExtendedLocale::SymbolPos
    
    Defines the position of the am/pm symbol.
    
    \sa amPmSymbolPosition(), setAmPmSymbolPosition(), currencySymbolPosition(), setCurrencySymbolPosition()
*/

/*!
    \var HbExtendedLocale::Before
    Before the time expression.    
*/

/*!
    \var HbExtendedLocale::After
    After the time expression.
 */

/*!
    
    Returns the position of the am/pm symbol in a time expression from the 
    locale according to HbExtendedLocale::SymbolPos.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::After.
    
    \sa setAmPmSymbolPosition()

 */
HbExtendedLocale::SymbolPos HbExtendedLocale::amPmSymbolPosition() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TLocalePos position = _symbianLocale.GetLocale()->AmPmSymbolPosition();
    if ( position == ELocaleBefore ) {
        return HbExtendedLocale::Before;
    } else {            
        return HbExtendedLocale::After;
    }
#else
    return HbExtendedLocale::After;
#endif
}

/*!
    
    Sets the am/pm symbol's position to the system locale as specified in \a 
    position.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa amPmSymbolPosition(), HbExtendedLocale::SymbolPos
    
 */
bool HbExtendedLocale::setAmPmSymbolPosition( const SymbolPos position )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    if ( position == ( HbExtendedLocale::Before ) ) {
        _symbianLocale.GetLocale()->SetAmPmSymbolPosition(ELocaleBefore);
    } else if ( position == ( HbExtendedLocale::After ) ) {
        _symbianLocale.GetLocale()->SetAmPmSymbolPosition(ELocaleAfter);
    } else {
        return false;
    }
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(position);
    return false;
#endif
}

/*!
    
    Returns the short unit distance format from the locale according to 
    QLocale::MeasurementSystem in QLocale (that is, whether metric or imperial 
    units are used).
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns QLocale::MetricSystem.
    
    \sa setUnitsDistanceShort()
    
*/
QLocale::MeasurementSystem HbExtendedLocale::unitsDistanceShort() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TUnitsFormat val = _symbianLocale.GetLocale()->UnitsDistanceShort();
    switch ( val ) {
    case EUnitsMetric:
        return QLocale::MetricSystem;
    case EUnitsImperial:
        return QLocale::ImperialSystem;
    }
    return QLocale::MetricSystem;
#else
    return QLocale::MetricSystem;
#endif
}

/*!
    
    Sets the short unit distance format to the system locale as specified in \a 
    format.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa unitsDistanceShort(), QLocale::MeasurementSystem in QLocale
 */
bool HbExtendedLocale::setUnitsDistanceShort( const QLocale::MeasurementSystem format )
{
#if defined(Q_OS_SYMBIAN)    
    TUnitsFormat set;

    switch ( format ) {
    default:
    case QLocale::MetricSystem:
        set = EUnitsMetric;
        break;
    case QLocale::ImperialSystem:
        set = EUnitsImperial;
        break;
    }
    
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetUnitsDistanceShort(set);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(format);
    return false;
#endif
}

/*!
      
    Returns the long unit distance format from the system locale according to 
    QLocale::MeasurementSystem in QLocale (that is, whether metric or imperial 
    units are used).

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns QLocale::MetricSystem.
    
    \sa setUnitsDistanceLong()
 */
QLocale::MeasurementSystem HbExtendedLocale::unitsDistanceLong() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TUnitsFormat val = _symbianLocale.GetLocale()->UnitsDistanceLong();
    switch ( val ) {
    case EUnitsMetric:
        return QLocale::MetricSystem;
    case EUnitsImperial:
        return QLocale::ImperialSystem;
    }
    return QLocale::MetricSystem;
#else
    return QLocale::MetricSystem;
#endif
}

/*!

    Sets the long unit distance format to the system locale as specified in \a 
    format.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa unitsDistanceLong(), QLocale::MeasurementSystem in QLocale
 */
bool HbExtendedLocale::setUnitsDistanceLong( const QLocale::MeasurementSystem format )
{
#if defined(Q_OS_SYMBIAN)
    TUnitsFormat set;

    switch ( format ) {
    default:
    case QLocale::MetricSystem:
        set = EUnitsMetric;
        break;
    case QLocale::ImperialSystem:
        set = EUnitsImperial;
        break;
    }
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetUnitsDistanceLong(set);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(format);
    return false;
#endif
}

/*!
    
    Sets the general unit distance to the system locale as specified in \a 
    format.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa QLocale::MeasurementSystem in QLocale
 */
bool HbExtendedLocale::setUnitsGeneral( const QLocale::MeasurementSystem format )
{
#if defined(Q_OS_SYMBIAN)
    TUnitsFormat set;

    switch ( format ) {
    default:
    case QLocale::MetricSystem:
        set = EUnitsMetric;
        break;
    case QLocale::ImperialSystem:
        set = EUnitsImperial;
        break;
    }
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetUnitsGeneral(set);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(format);
    return false;
#endif
}

/*! 
    \enum HbExtendedLocale::NegativeCurrencyFormat

    Defines the format used for a negative currency, for example, if the 
    indicator is a minus sign (with different options as to where it is placed) 
    or brackets around the currency expression.
    
    \sa negativeCurrencyFormat(), setNegativeCurrencyFormat()
*/

/*!
    \var HbExtendedLocale::LeadingMinusSign

    A minus sign is inserted before the currency symbol and value.

*/

/*!
    \var HbExtendedLocale::InBrackets
    
    The currency value and symbol are enclosed in brackets (no minus sign is 
    used).

*/

/*!
    \var HbExtendedLocale::TrailingMinusSign

    A minus sign is inserted after the currency symbol and value.

*/

/*!
    \var HbExtendedLocale::InterveningMinusSign

    A minus sign is inserted between the currency symbol and the value.

*/

/*!

    Returns the negative currency format from the system locale according to 
    HbExtendedLocale::NegativeCurrencyFormat.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::LeadingMinusSign.
    
    \sa setNegativeCurrencyFormat()
*/
HbExtendedLocale::NegativeCurrencyFormat HbExtendedLocale::negativeCurrencyFormat() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TLocale::TNegativeCurrencyFormat val = _symbianLocale.GetLocale()->NegativeCurrencyFormat();
    switch ( val ) {
    default:
    case TLocale::ELeadingMinusSign:
        return HbExtendedLocale::LeadingMinusSign;
    case TLocale::EInBrackets:
        return HbExtendedLocale::InBrackets;
    case TLocale::ETrailingMinusSign:
        return HbExtendedLocale::TrailingMinusSign;
    case TLocale::EInterveningMinusSign:
        return HbExtendedLocale::InterveningMinusSign;
    }
#else
    return HbExtendedLocale::LeadingMinusSign;
#endif
}

/*!
    
    Sets the negative currency format to the system locale as specified in \a 
    format.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa negativeCurrencyFormat(), HbExtendedLocale::NegativeCurrencyFormat
 */
bool HbExtendedLocale::setNegativeCurrencyFormat( const NegativeCurrencyFormat format )
{
#if defined(Q_OS_SYMBIAN)
    TLocale::TNegativeCurrencyFormat set;
    switch ( format ) {
    default:
    case HbExtendedLocale::LeadingMinusSign:
        set = TLocale::ELeadingMinusSign;
        break;
    case HbExtendedLocale::InBrackets:
        set = TLocale::EInBrackets;
        break;
    case HbExtendedLocale::TrailingMinusSign:
        set = TLocale::ETrailingMinusSign;
        break;
    case HbExtendedLocale::InterveningMinusSign:
        set = TLocale::EInterveningMinusSign;
        break;
    }
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetNegativeCurrencyFormat(set);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(format);
    return false;
#endif
}

/*!

    Returns from the system locale \c true if a space is inserted between a 
    negative currency value and the currency symbol. If no space is inserted, 
    returns \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa setNegativeCurrencySpace()
 */
bool HbExtendedLocale::negativeCurrencySpace() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return !_symbianLocale.GetLocale()->NegativeLoseSpace(); // the not here is intended
#else
    return false;
#endif
}

/*!
    
    Sets the space inserting in negative currency expressions to the system 
    locale as specified in \a space.
    
    \param space Specify \c true if a space is inserted between a negative 
    currency value and the currency symbol, and \c false if a space is not 
    inserted.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa negativeCurrencySpace(), setCurrencySpace()
 */
bool HbExtendedLocale::setNegativeCurrencySpace( const bool space )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetNegativeLoseSpace(!space); // the not here is intended
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(space);
    return false;
#endif
}

/*!

    Returns \c true from the system locale if the currency symbol's position in 
    negative currency values is the opposite of the position set by 
    setCurrencySymbolPosition(). Otherwise returns \c false.

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa setNegativeCurrencySymbolOpposite()
*/
bool HbExtendedLocale::negativeCurrencySymbolOpposite() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return _symbianLocale.GetLocale()->NegativeCurrencySymbolOpposite();
#else
    return false;
#endif
}

/*!
    
    Sets the currency symbol position in negative currency expressions to the 
    system locale.
    
    \param opposite Specify \c true if the position of the currency symbol in 
    negative currency expressions is the opposite of the position set using 
    setCurrencySymbolPosition(), and \c false if the position is the same.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa negativeCurrencySymbolOpposite()
*/
bool HbExtendedLocale::setNegativeCurrencySymbolOpposite( const bool opposite )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetNegativeCurrencySymbolOpposite(opposite);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(opposite);
    return false;
#endif
}

/*!
    
    Returns from the locale \c true if currency triads are allowed, otherwise \c 
    false. The use of currency triads means the grouping of digits in large 
    numbers, such as '123 456 789.' The Symbian platform only supports the 
    grouping of currency amounts.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa setCurrencyTriadsAllowed()
 */
bool HbExtendedLocale::currencyTriadsAllowed() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return _symbianLocale.GetLocale()->CurrencyTriadsAllowed();
#else
    return false;
#endif
}

/*!

    Sets to the system locale whether or not currency triads are allowed in 
    currency values.
    
    \param allowed Specify \c true if currency triads are allowed, and \c false 
    if not allowed.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa currencyTriadsAllowed()
 */
bool HbExtendedLocale::setCurrencyTriadsAllowed( const bool allowed )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetCurrencyTriadsAllowed(allowed);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(allowed);
    return false;
#endif
}


/*!
    
    Returns from the system locale \c true if a space is inserted between a 
    positive currency value and the currency symbol. If no space is inserted, 
    returns \c false.

    \note In negative currency expressions, the space can be inserted using 
    setNegativeCurrencySpace().
           
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa setCurrencySpace()
 */
bool HbExtendedLocale::currencySpace() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return _symbianLocale.GetLocale()->CurrencySpaceBetween();
#else
    return false;
#endif
}

/*!
    
    Sets to the system locale whether or not a space is inserted between the 
    currency symbol and the currency amount.
    
    \param space Specify \c true if a space is inserted between a currency 
    amount and the currency symbol, and \c false if a space is not inserted.
    
    \return \c true if successful, otherwise \c false.

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \note You can set the space inserting separately for negative currency 
    values using setNegativeCurrencySpace(). 
    
    \sa currencySpace()
 */
bool HbExtendedLocale::setCurrencySpace( const bool space )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetCurrencySpaceBetween(space);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(space);
    return false;
#endif
}


/*!

    Returns the currency symbol from the system locale, for example, œ, $, Ft, 
    kn, Euro. The number of characters used in the currency symbol may vary in 
    different countries/regions.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns an empty QString.
    
    \sa setCurrencySymbol()
 */
QString HbExtendedLocale::currencySymbol() const
{
    // copy from other similar method
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return QString::fromUtf16(_symbianLocale.GetCurrencySymbol().Ptr(), _symbianLocale.GetCurrencySymbol().Length());
#else
    return QString();
#endif
}

/*!
    
    Sets the currency symbol as specified in \a symbol.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa currencySymbol()
 */
bool HbExtendedLocale::setCurrencySymbol( const QString &symbol )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    // prepare for symbol being larger than KMaxCurrencySymbol
    if ( symbol.length() < KMaxCurrencySymbol ) {
        TPtrC newCurrencySymbol(symbol.utf16());
        _symbianLocale.SetCurrencySymbol(newCurrencySymbol);
        return true;
    }
    return false;
#else
    Q_UNUSED(symbol);
    return false;
#endif
}

/*!
    
    Returns the position of the currency symbol (before or after the amount) 
    according to HbExtendedLocale::SymbolPos.

    \note For negative currency expressions, you can reverse the position using 
    SetNegativeCurrencySymbolOpposite().
               
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::Before.
    
    \sa setCurrencySymbolPosition()
 */
HbExtendedLocale::SymbolPos HbExtendedLocale::currencySymbolPosition() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TLocalePos pos = _symbianLocale.GetLocale()->CurrencySymbolPosition();
    switch ( pos ) {
    case ELocaleBefore:
        return HbExtendedLocale::Before;
    case ELocaleAfter:
        return HbExtendedLocale::After;
    }
    return HbExtendedLocale::After;
#else
    return HbExtendedLocale::Before;
#endif
}

/*!
    Sets the position of the currency symbol as specified in \a position.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa currencySymbolPosition(), HbExtendedLocale::SymbolPos
 */
bool HbExtendedLocale::setCurrencySymbolPosition( const SymbolPos position )
{
#if defined(Q_OS_SYMBIAN)
    TLocalePos pos;
    switch ( position ) {
    default:
    case HbExtendedLocale::Before:
        pos = ELocaleBefore;
        break;
    case HbExtendedLocale::After:
        pos = ELocaleAfter;
        break;
    }
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetCurrencySymbolPosition(pos);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(position);
    return false;
#endif
}

/*!
    
    Returns from the locale the number of digits that follow the decimal 
    separator in the currency.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns '0'.
    
    \sa setCurrencyDecimalPlaces()
 */
int HbExtendedLocale::currencyDecimalPlaces() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return _symbianLocale.GetLocale()->CurrencyDecimalPlaces();
#else
    return 0;
#endif
}

/*!
    
    Sets the number of decimals for currency values.
    
    \param places The number of digits to be used after the decimal separator.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa currencyDecimalPlaces()
 */
bool HbExtendedLocale::setCurrencyDecimalPlaces( const int places )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetCurrencyDecimalPlaces(places);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(places);
    return false;
#endif
}

/*
    Helper class to handle overflows that may occur when
    calling TLocale::FormatCurrency(TDes &aText, TDesOverflow &aOverflowHandler, TInt aAmount);
    Increases currencyFormatSize by CURRENCY_FORMAT_SIZE_STEP with each overflow.
 */
#if defined(Q_OS_SYMBIAN)
class TestOverflow : public TDesOverflow
{
public:
    void Overflow( TDes& ) { currencyFormatSize += CURRENCY_FORMAT_SIZE_STEP; }
    void setCurrencyFormatSize( int newCurrencyFormatSize ) { currencyFormatSize = newCurrencyFormatSize; }
    int getCurrencyFormatSize() { return currencyFormatSize; }

private:
    int currencyFormatSize;
};
#endif

/*!

    Returns the currency amount as a string based on the system locale's 
    currency and numeric format settings. These settings include the currency 
    symbol, the symbol's position in the currency expression, and how negative 
    currencies are formatted.
    
    \param amount The currency amount.

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, this function uses the given amount as a QString.

    \sa negativeCurrencyFormat()
        
*/
QString HbExtendedLocale::formatCurrency( const qint64 amount )
{
#if defined(Q_OS_SYMBIAN)

    TestOverflow overflow;

    // set buffer minimum size
    const int base = 10;
    int bufferMinSize = QString::number(amount, base).length() + CURRENCY_FORMAT_SIZE_STEP;
    overflow.setCurrencyFormatSize(bufferMinSize);

    HBufC *bufPtr = HBufC::New(bufferMinSize);

    if ( !bufPtr ) {
        return QString();
    }

    // stores the previous formatted value size
    int fSize = 0;

    // Compare current currency formatted value size to previous.
    // Increase indicates that overflow has occurred.
    // Initial value of fSize is zero so the iteration is done
    // at least once.

    while ( fSize < overflow.getCurrencyFormatSize() ) {
        // fSize value before calling formatCurrency
        fSize = overflow.getCurrencyFormatSize();

        // handle Panic: USER 18 if aMaxLength is negative.
        if ( fSize < 0 ) {
            return QString();
        }

        HBufC *newBufPtr = bufPtr->ReAlloc(fSize);

        if ( !newBufPtr ) {
            delete bufPtr;
            return QString();
        }

        bufPtr = newBufPtr;

        TPtr modifiableBufPtr(bufPtr->Des());

        _symbianLocale.LoadSystemSettings();
        _symbianLocale.GetLocale()->FormatCurrency(modifiableBufPtr, overflow, TInt64(amount));
     }
    QString value = QString::fromUtf16(bufPtr->Ptr(), bufPtr->Length());
    delete bufPtr;
    bufPtr = 0;
    return value;

#else
    return QString::number(amount);
#endif
}

/*!
    
    Sets the decimal separator character to the system locale as specified in \a 
    ch.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
*/
bool HbExtendedLocale::setDecimalPoint( const QChar ch )
{
#if defined(Q_OS_SYMBIAN)
    TChar symbianch( ch.unicode() );
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetDecimalSeparator(symbianch);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(ch);
    return false;
#endif
}

/*!

    Sets the group separator character (that is, the thousands separator) to the 
    system locale as specified in \a ch.

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
*/
bool HbExtendedLocale::setGroupSeparator( const QChar ch )
{
#if defined(Q_OS_SYMBIAN)
    TChar symbianch( ch.unicode() );
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetThousandsSeparator(symbianch);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(ch);
    return false;
#endif
}

/*!

    Sets the specified zero digit type to the system locale.
    
    \param type The zero digit type. Possible values:
    - \c WesternDigit - Latin digits
    - \c ArabicIndicDigit - Arabic-Indic digits
    - \c EasternArabicIndicDigit - Eastern Arabic-Indic digits
    - \c DevanagariDigit - Devanagari digits

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
*/
bool HbExtendedLocale::setZeroDigit( const DigitType type )
{
#if defined(Q_OS_SYMBIAN)
    TDigitType digit;
    switch ( type ) {
        default:
        case WesternDigit :
            digit = EDigitTypeWestern;
            break;
        case ArabicIndicDigit :
            digit = EDigitTypeArabicIndic;
            break;
        case EasternArabicIndicDigit:
            digit = EDigitTypeEasternArabicIndic;
            break;
        case DevanagariDigit:
            digit = EDigitTypeDevanagari;
            break;
        case ThaiDigit:
            digit = EDigitTypeThai;
            break;
        }
    _symbianLocale.LoadSystemSettings();
    _symbianLocale.GetLocale()->SetDigitType(digit);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(type);
    return false;
#endif
}

/*!
    Mapping from Symbian to ISO locale
*/
struct symbianToISO {
    // enumeration of symbian language
    int symbian_language;
    // string of ISO value 
    char iso_name[8];
};

#if defined(Q_OS_SYMBIAN)
/*!
    Mapping from Symbian to ISO locale
*/
static const symbianToISO symbian_to_iso_list[] = {
        { ELangEnglish,             "en_GB" },
        { ELangFrench,              "fr_FR" },
        { ELangGerman,              "de_DE" },
        { ELangSpanish,             "es_ES" },
        { ELangItalian,             "it_IT" },
        { ELangSwedish,             "sv_SE" },
        { ELangDanish,              "da_DK" },
        { ELangNorwegian,           "nb_NO" },
        { ELangFinnish,             "fi_FI" },
        { ELangAmerican,            "en_US" },
        { ELangSwissFrench,         "fr_CH" },
        { ELangSwissGerman,         "de_CH" },
        { ELangPortuguese,          "pt_PT" },
        { ELangTurkish,             "tr_TR" },
        { ELangIcelandic,           "is_IS" },
        { ELangRussian,             "ru_RU" },
        { ELangHungarian,           "hu_HU" },
        { ELangDutch,               "nl_NL" },
        { ELangBelgianFlemish,      "nl_BE" },
        { ELangAustralian,          "en_AU" },
        { ELangBelgianFrench,       "fr_AU" },
        { ELangAustrian,            "de_AT" },
        { ELangNewZealand,          "en_NZ" },
        { ELangInternationalFrench, "fr_ZZ" },
        { ELangCzech,               "cs_CZ" },
        { ELangSlovak,              "sk_SK" },
        { ELangPolish,              "pl_PL" },
        { ELangSlovenian,           "sl_SI" },
        { ELangPrcChinese,          "zh_CN" },
        { ELangTaiwanChinese,       "zh_TW" },
        { ELangHongKongChinese,     "zh_HK" },
        { ELangJapanese,            "ja_JP" },
        { ELangThai,                "th_TH" },
        { ELangAfrikaans,           "af_ZA" },
        { ELangAlbanian,            "sq_AL" },
        { ELangAmharic,             "am_ET" },
        { ELangArabic,              "ar_AE" },
        { ELangArmenian,            "hy_AM" },
        { ELangTagalog,             "tl_PH" },
        { ELangBelarussian,         "be_BY" },
        { ELangBengali,             "bn_IN" },
        { ELangBulgarian,           "bg_BG" },
        { ELangBurmese,             "my_MM" },
        { ELangCatalan,             "ca_ES" },
        { ELangCroatian,            "hr_HR" },
        { ELangCanadianEnglish,     "en_CA" },
        { ELangInternationalEnglish,"en_ZZ" },
        { ELangSouthAfricanEnglish, "en_ZA" },
        { ELangEstonian,            "et_EE" },
        { ELangFarsi,               "fa_IR" },
        { ELangCanadianFrench,      "fr_CA" },
        { ELangScotsGaelic,         "gd_GB" },
        { ELangGeorgian,            "ka_GE" },
        { ELangGreek,               "el_GR" },
        { ELangCyprusGreek,         "el_CY" },
        { ELangGujarati,            "gu_IN" },
        { ELangHebrew,              "he_IL" },
        { ELangHindi,               "hi_IN" },
        { ELangIndonesian,          "id_ID" },
        { ELangIrish,               "ga_IE" },
        { ELangSwissItalian,        "it_CH" },
        { ELangKannada,             "kn_IN" },
        { ELangKazakh,              "kk_KZ" },
        { ELangKhmer,               "km_KH" },
        { ELangKorean,              "ko_KR" },
        { ELangLao,                 "lo_LA" },
        { ELangLatvian,             "lv_LV" },
        { ELangLithuanian,          "lt_LT" },
        { ELangMacedonian,          "mk_MK" },
        { ELangMalay,               "ms_MY" },
        { ELangMalayalam,           "ml_IN" },
        { ELangMarathi,             "mr_IN" },
        { ELangMoldavian,           "ro_MD" },
        { ELangMongolian,           "mn_MN" },
        { ELangNorwegianNynorsk,    "nn_NO" },
        { ELangBrazilianPortuguese, "pt_BR" },
        { ELangPunjabi,             "pa_IN" },
        { ELangRomanian,            "ro_RO" },
        { ELangSerbian,             "sr_YU" },
        { ELangSinhalese,           "si_LK" },
        { ELangSomali,              "so_SO" },
        { ELangInternationalSpanish,"es_ZZ" },
        { ELangLatinAmericanSpanish,"es_419" },
        { ELangSwahili,             "sw_KE" },
        { ELangFinlandSwedish,      "sv_FI" },
        { ELangTamil,               "ta_IN" },
        { ELangTelugu,              "te_IN" },
        { ELangTibetan,             "bo_CN" },
        { ELangTigrinya,            "ti_ER" },
        { ELangCyprusTurkish,       "tr_CY" },
        { ELangTurkmen,             "tk_TM" },
        { ELangUkrainian,           "uk_UA" },
        { ELangUrdu,                "ur_PK" },
        { ELangVietnamese,          "vi_VN" },
        { ELangWelsh,               "cy_GB" },
        { ELangZulu,                "zu_ZA" },
        { ELangManufacturerEnglish, "en_XZ" },
        { ELangSouthSotho,          "st_LS" },
#ifdef __E32LANG_H__
// 5.0
        { ELangBasque,              "eu_ES" },
        { ELangGalician,            "gl_ES" },
#endif
        { ELangJavanese,            "jv_ID" },  
        { ELangMaithili,            "bh_IN" },
        { ELangAzerbaijani_Latin,   "az_AZ" }, 
        { ELangOriya,               "or_IN" },
        { ELangBhojpuri,            "bh_IN" },
        { ELangSundanese,           "su_ID" },
        { ELangKurdish_Latin,       "ku_TR" },
        { ELangKurdish_Arabic,      "ku_IQ" },
        { ELangPashto,              "ps_AF" },
        { ELangHausa,               "ha_NG" }, 
        { ELangOromo,               "om_ET" },
        { ELangUzbek_Latin,         "uz_UZ" },
        { ELangSindhi_Arabic,       "sd_PK" },
        { ELangSindhi_Devanagari,   "sd_IN" },
        { ELangYoruba,              "yo_NG" },
        { ELangIgbo,                "ig_NG" },
        { ELangMalagasy,            "mg_MG" },
        { ELangNepali,              "ne_NP" },
        { ELangAssamese,            "as_IN" },
        { ELangShona,               "sn_ZW" },
        { ELangZhuang,              "za_CN" },
        { ELangEnglish_Taiwan,      "en_TW" },
        { ELangEnglish_HongKong,    "en_HK" },
        { ELangEnglish_Prc,         "en_CN" },
        { ELangEnglish_Japan,       "en_JP" },
        { ELangEnglish_Thailand,    "en_TH" },
        { ELangFulfulde,            "ff_NE" },
        { ELangBolivianQuechua,     "qu_BO" },
        { ELangPeruQuechua,         "qu_PE" },
        { ELangEcuadorQuechua,      "qu_EC" },
        { ELangTajik_Cyrillic,      "tg_TJ" },
        { ELangNyanja,              "ny_MW" },
        { ELangHaitianCreole,       "ht_HT" },
        { ELangKoongo,              "kg_CG" },
        { ELangAkan,                "ak_GH" },
        { ELangYi,                  "ii_CN" },
        { ELangUyghur,              "ug_CN" },
        { ELangRwanda,              "rw_RW" },
        { ELangXhosa,               "xh_ZA" },
        { ELangGikuyu,              "ki_KE" },
        { ELangRundi,               "rn_BI" },
        { ELangTswana,              "tn_BW" },
        { ELangKanuri,              "kr_NE" },
        { ELangKashmiri_Devanagari, "ks_ZZ" },
        { ELangKashmiri_PersoArabic,"ks_XZ" },
        { ELangWolof,               "wo_SN" },
        { ELangTsonga,              "ts_ZA" },
        { ELangYiddish,             "yi_IL" },
        { ELangKirghiz,             "ky_KG" },
        { ELangGanda,               "lg_UG" },
        { ELangBambara,             "bm_ML" },
        { ELangCentralAymara,       "ay_BO" },
        { ELangLingala,             "ln_CG" },
        { ELangBashkir,             "ba_RU" },
        { ELangChuvash,             "cv_RU" },
        { ELangSwati,               "ss_SZ" },
        { ELangTatar,               "tt_RU" },
        { ELangSouthernNdebele,     "nr_ZA" },
        { ELangSardinian,           "sc_IT" },
        { ELangWalloon,             "wa_BE" },
        { ELangEnglish_India,       "en_IN" }
};
#endif

/*!

    Returns the ISO 639 language code that corresponds to the Symbian language 
    code specified in \a code. If the code does not correspond to any Symbian 
    language code, returns a empty string.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns an empty QString.
    
    \sa ISOToSymbianLang(), HbLocaleUtil

*/
QString HbExtendedLocale::symbianLangToISO( const int code )
{
#if defined(Q_OS_SYMBIAN)
    int begin = 0;
    int end = sizeof(symbian_to_iso_list)/sizeof(symbianToISO);
    int mid;
    int cmp = code - symbian_to_iso_list[0].symbian_language;
    if (cmp < 0) {
        return QString();
    } else {
        if (cmp == 0) {
            return symbian_to_iso_list[0].iso_name;
        }
    }
    symbianToISO *elt;
    while (end > begin) {
        mid = (begin + end) >> 1;
        elt = const_cast<symbianToISO *>(symbian_to_iso_list+mid);
        cmp = elt->symbian_language - code;
        if (cmp < 0) {
            begin = mid+1;
        } 
        else {
            if (!cmp) {
                return elt->iso_name;  
            }
            else {
                end = mid;
            }    
        } 
    }
    if (!cmp) {
        return elt->iso_name;  
    }
    return QString();
#else
    Q_UNUSED(code);
    return QString();
#endif
}

/*!

    Returns the RFC 3066 language code that corresponds to the Symbian language 
    code specified in \a code. If the code does not correspond to any Symbian 
    language code, returns a empty string.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns an empty QString.  
    
*/
QString HbExtendedLocale::symbianLangToRfc3066( const int code )
{
    return symbianLangToISO(code).replace('_', '-');
}

/*!

    Returns the Symbian language code for the specified ISO code. If the 
    conversion fails, returns -1.

    \param langAndCountry The ISO code, for example, "fi_FI". This is a 
    combination of an ISO 639 language code and an ISO 3166 country code.

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns -1.
    
    \sa symbianLangToISO(), HbLocaleUtil
    
 */
int HbExtendedLocale::ISOToSymbianLang( const QString &langAndCountry )
{
#if defined(Q_OS_SYMBIAN)
    if ( langAndCountry.length() == 0 ) {
        return -1;
    }
    int count = sizeof(symbian_to_iso_list)/sizeof(symbianToISO);
    for ( int i = 0; i < count; i++) {
        QString tag = QString(symbian_to_iso_list[i].iso_name);
        if (langAndCountry.length() == 2) {
            tag = tag.left(2);
        }
        if (langAndCountry == tag) {
            return symbian_to_iso_list[i].symbian_language;
        }
    }
    return -1;
#else
    Q_UNUSED(langAndCountry);
    return -1;
#endif
}

#if defined(Q_OS_SYMBIAN)
// order is: normal, abbr, nmode, nmode+abbr
static const char *us_locale_dep[] = {
    "MM", "dd", "yyyy", "MM", "dd",
    "M", "d", "yy", "M", "d",
    "MMMM", "dd", "yyyy", "MMMM", "dd",
    "MMM", "d", "yy", "MMM", "d" };

static const char *eu_locale_dep[] = {
    "dd", "MM", "yyyy", "dd", "MM",
    "d", "M", "yy", "d", "M",
    "dd", "MMMM", "yyyy", "dd", "MMMM",
    "d", "MMM", "yy", "d", "MMM" };

static const char *jp_locale_dep[] = {
    "yyyy", "MM", "dd", "MM", "dd",
    "yy", "M", "d", "M", "d",
    "yyyy", "MMMM", "dd", "MMMM", "dd",
    "yy", "MMM", "d", "MMM", "d" };
#endif

/*!
    
    Returns a Qt datetime format string for the Symbian datetime format string 
    given in \a sys_fmt. The possible date formats are defined in the 
    hbi18ndef.h header file.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns the string "not supported".
  
*/
QString HbExtendedLocale::symbianDateTimeToQt( const QString &sys_fmt )
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TLocale *locale = _symbianLocale.GetLocale();

    QString result;
    QString other;
    QString qtformatchars = QString::fromLatin1("adhmsyzAHM");

    QChar c;
    int i = 0;
    bool open_escape = false;
    bool abbrev_next = false;
    bool locale_indep_ordering = false;
    bool minus_mode = false;
    bool plus_mode = false;
    bool n_mode = false;
    TTimeFormat tf = locale->TimeFormat();

    while ( i < sys_fmt.size() ) {
        c = sys_fmt.at(i);

        // let formatting thru
        if ( c.unicode() == '%' ) {
            // if we have gathered string, concat it
            if ( !other.isEmpty() ) {
                result += other;
                other.clear();
            }
            // if we have open escape, end it
            if ( open_escape ) {
                result += QLatin1Char('\'');
                open_escape = false;
            }

            ++i;
            if ( i >= sys_fmt.size() ) {
                break;
            }

            c = sys_fmt.at(i);

            // process specials
            abbrev_next = c.unicode() == '*';
            plus_mode = c.unicode() == '+';
            minus_mode = c.unicode() == '-';

            if ( abbrev_next || plus_mode || minus_mode ) {
                ++i;
                if ( i >= sys_fmt.size() ) {
                    break;
                }

                c = sys_fmt.at(i);
                
                if ( plus_mode || minus_mode ) {
                    // break on undefined plus/minus mode
                    if ( c.unicode() != 'A' && c.unicode() != 'B' ) {
                        break;
                    }
                }
            }

            switch ( c.unicode() ) {
                case 'F':
                    // locale indep mode on
                    locale_indep_ordering = true;
                    break;
                    
                case '/':
                    // date sep 0-3
                    ++i;
                    if ( i >= sys_fmt.size() ) {
                        break;
                    }

                    c = sys_fmt.at(i);
                    if ( c.isDigit() && c.digitValue() <= 3 ) {
                        TChar s = locale->DateSeparator(c.digitValue());
                        TUint val = s;
                        // some indexes return zero for empty
                        if ( val > 0 ) {
                            result += QChar(val);
                        }
                    }
                    break;
                    
                case 'D':
                    if ( !locale_indep_ordering ) {
                        break;
                    }

                    if ( !abbrev_next ) {
                        result += QLatin1String("dd");
                    } else {
                        result += QLatin1Char('d');
                    }
                    break;

                case 'M':
                    if ( !locale_indep_ordering ) {
                        break;
                    }

                    if ( !n_mode ) {
                        if ( !abbrev_next ) {
                            result += QLatin1String("MM");
                        } else {
                            result += QLatin1String("M");
                        }
                    } else {
                        if ( !abbrev_next ) {
                            result += QLatin1String("MMMM");
                        } else {
                            result += QLatin1String("MMM");
                        }
                    }
                    break;
                    
                case 'N':
                    n_mode = true;

                    if ( !locale_indep_ordering ) {
                        break;
                    }

                    if ( !abbrev_next ) {
                        result += QLatin1String("MMMM");
                    } else {
                        result += QLatin1String("MMM");
                    }
                    break;

                case 'Y':
                    if ( !locale_indep_ordering ) {
                        break;
                    }

                    if ( !abbrev_next ) {
                        result += QLatin1String("yyyy");
                    } else {
                        result += QLatin1String("yy");
                    }
                    break;

                case 'E':
                    if ( !abbrev_next ) {
                        result += QLatin1String("dddd");
                    } else {
                        result += QLatin1String("ddd");
                    }    
                    break;

                case ':':
                    // timesep 0-3
                    ++i;
                    if ( i >= sys_fmt.size() ) {
                        break;
                    }

                    c = sys_fmt.at(i);
                    if ( c.isDigit() && c.digitValue() <= 3 ) {
                        TChar s = locale->TimeSeparator(c.digitValue());
                        TUint val = s;
                        // some indexes return zero for empty
                        if ( val > 0 ) {
                            result += QChar(val);
                        }
                    }
                    break;
                    
                case 'J':
                    if ( tf == ETime24 && !abbrev_next ) {
                        result += QLatin1String("hh");
                    } else {
                        result += QLatin1Char('h');
                    }
                    break;

                case 'H':
                    if ( !abbrev_next ) {
                        result += QLatin1String("hh");
                    } else {
                        result += QLatin1Char('h');
                    }
                    break;

                case 'I':
                    result += QLatin1Char('h');
                    break;

                case 'T':
                    if ( !abbrev_next ) {
                        result += QLatin1String("mm");
                    } else {
                        result += QLatin1Char('m');
                    }
                    break;

                case 'S':
                    if ( !abbrev_next ) {
                        result += QLatin1String("ss");
                    } else {
                        result += QLatin1Char('s');
                    }
                    break;

                case 'B':
                    // only done for 12h clock
                    if ( tf == ETime24 ) {
                        break;
                    }

                    // fallthru to A
                case 'A': {
                    // quickie to get capitalization, can't use s60 string as is because Qt 'hh' format's am/pm logic
                    TAmPmName ampm = TAmPmName();
                    if ( ampm.Size() == 0 ) {
                        return QString();
                    }
                    TChar first(ampm[0]);
                    QString qtampm = QString::fromLatin1(first.IsUpper() ? "AP" : "ap");

                    int pos = locale->AmPmSymbolPosition();

                    if ( ( minus_mode && pos != ELocaleBefore ) ||
                       ( plus_mode && pos != ELocaleAfter ) ) {
                       break;
                    }

                    if ( !abbrev_next && locale->AmPmSpaceBetween() ) {
                        if ( pos == ELocaleBefore ) {
                            qtampm.append(QLatin1Char(' '));
                        } else {
                           qtampm.prepend(QLatin1Char(' '));
                        }
                    }

                    result += qtampm;
                    }
                    break;

                case '.': {
                    // decimal sep
                    TChar s = locale->DecimalSeparator();
                    TUint val = s;
                    if ( val > 0 ) {
                        result += QChar(val);
                    }
                    }
                    break;

                case 'C':
                    // six digits in s60, three digits in qt
                    if ( !abbrev_next ) {
                        result += QLatin1String("zzz");
                    } else {
                        // next char is number from 0-6, how many digits to display
                        ++i;
                        if ( i >= sys_fmt.size() ) {
                            break;
                        }

                        c = sys_fmt.at(i);

                        if ( c.isDigit() ) {
                            // try to match wanted digits
                            QChar val(c.digitValue());

                            if ( val >= 3 ) {
                                result += QLatin1String("zzz");
                            } else if ( val > 0 ) {
                                result += QLatin1Char('z');
                            }
                        }
                    }
                    break;

                // these cases fallthru
                case '1':
                case '2':
                case '3':
                case '4':
                case '5': {

                    // shouldn't parse these with %F
                    if ( locale_indep_ordering ) {
                        break;
                    }

                    TDateFormat df = locale->DateFormat();

                    const char **locale_dep;
                    switch ( df ) {
                        default: // fallthru to american
                        case EDateAmerican:
                            locale_dep = us_locale_dep;
                            break;
                        case EDateEuropean:
                            locale_dep = eu_locale_dep;
                            break;
                        case EDateJapanese:
                            locale_dep = jp_locale_dep;
                            break;
                    }
                    int offset = 0;
                    if ( abbrev_next ) {
                        offset += 5;
                    }
                    if ( n_mode ) {
                        offset += 10;
                    }
                    
                    // 'offset + (c.digitValue()-1' cannot be bigger than us_locale_dep, eu_locale_dep or jp_locale_dep table
                    if ( (offset + (c.digitValue()-1)) > 19 ) {
                        return QString();
                    }
                    
                    result += QLatin1String(locale_dep[offset + (c.digitValue()-1)]);
                    }
                    break;

                case '%': // fallthru percent
                // any junk gets copied as is
                default:
                    result += c;
                    break;

                case 'Z': // Qt doesn't support these :(
                case 'X':
                case 'W':
                    break;
            }
        } else {
            // double any single quotes, don't begin escape
            if ( c.unicode() == '\'' ) {
                // end open escape
                if ( open_escape ) {
                    result += other;
                    other.clear();
                    result += QLatin1Char('\'');
                    open_escape = false;
                }

                other += c;
            }

            // gather chars and escape them in one go if any format chars are found
            if ( !open_escape && qtformatchars.indexOf(c) != -1 ) {
                result += QLatin1Char('\'');
                open_escape = true;
            }
            other += c;
        }

        ++i;
    }

    if ( !other.isEmpty() ) {
        result += other;
    }
    if ( open_escape ) {
        result += QLatin1Char('\'');
    }

    return result;
#else
    Q_UNUSED(sys_fmt);
    return QString("not supported");
#endif    
}

/*! 
    \enum HbExtendedLocale::WeekDay
    
    Defines the days of the week.

    \sa startOfWeek()
 */
 
 /*! 
    \var HbExtendedLocale::Monday
    Monday
 */

 /*! 
    \var HbExtendedLocale::Tuesday
    Tuesday
 */

 /*! 
    \var HbExtendedLocale::Wednesday
    Wednesday
 */

 /*! 
    \var HbExtendedLocale::Thursday
    Thursday
 */

 /*! 
    \var HbExtendedLocale::Friday
    Friday
 */

 /*! 
    \var HbExtendedLocale::Saturday
    Saturday
 */

 /*! 
    \var HbExtendedLocale::Sunday
    Sunday
 */

/*!

    Returns the first day of the week as specified in HbExtendedLocale::WeekDay. 
    The first day is usually Saturday, Sunday or Monday, but the Symbian 
    platform allows setting any weekday as the first.

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::Monday.
    
    \sa setStartOfWeek
 */
HbExtendedLocale::WeekDay HbExtendedLocale::startOfWeek() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TDay day = _symbianLocale.GetLocale()->StartOfWeek();
    switch ( day ) {
    case EMonday:
        return HbExtendedLocale::Monday;
    case ETuesday:
        return HbExtendedLocale::Tuesday;
    case EWednesday:
        return HbExtendedLocale::Wednesday;
    case EThursday:
        return HbExtendedLocale::Thursday;
    case EFriday:
        return HbExtendedLocale::Friday;
    case ESaturday:
        return HbExtendedLocale::Saturday;
    case ESunday:
        return HbExtendedLocale::Sunday;
    }
    return HbExtendedLocale::Monday;
#else
    return HbExtendedLocale::Monday;
#endif
}

/*!
    
    Sets the first day of the week as specified in \a day.
    
    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa startOfWeek(), HbExtendedLocale::WeekDay
 */

bool HbExtendedLocale::setStartOfWeek(WeekDay day)
    {
#if defined(Q_OS_SYMBIAN)
    if((day < HbExtendedLocale::Monday) || (day > HbExtendedLocale::Sunday ))
        return false;
    
    _symbianLocale.LoadSystemSettings();
    TDay startofweek = EMonday;
    switch ( day ) {
        case HbExtendedLocale::Monday:
            startofweek = EMonday;
            break;
        case HbExtendedLocale::Tuesday:
            startofweek = ETuesday;
            break;
        case HbExtendedLocale::Wednesday:
            startofweek = EWednesday;
            break;
        case HbExtendedLocale::Thursday:
            startofweek = EThursday;
            break;
        case HbExtendedLocale::Friday:
            startofweek = EFriday;
            break;
        case HbExtendedLocale::Saturday:
            startofweek = ESaturday;
            break;
        case HbExtendedLocale::Sunday:
            startofweek = ESunday;
            break;                    
        }
    _symbianLocale.GetLocale()->SetStartOfWeek(startofweek);
    _symbianLocale.GetLocale()->Set();
    return true;
#else
    Q_UNUSED(day);
    return false;
#endif    
    }


/*!

    Returns a QString that specifies as a binary array which days of the week 
    are working days. In the array, 1 indicates a working day and 0 indicates a 
    non working day. The week days are given from end to start in the array, 
    that is, the last digit indicates Monday (regardless of the active locale). 
    For example, "0011111" indicates that Monday to Friday are working days.

    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns "0011111".

    \sa setWorkDays()
 */
QString HbExtendedLocale::workDays() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    int days = _symbianLocale.GetLocale()->WorkDays();
    QString s = QString::number(days, 2);
    while ( s.size() < 7 ) {
        s.prepend("0");
    }
    return s;
#else
    return QString("0011111");
#endif
}

/*!
    Sets the working days of the week as specified in \a days.
    
    \param days A binary array that specifies which days of the week are working 
    days, for example, "0011111" (the last digit indicates Monday).

    \return \c true if successful, otherwise \c false.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.

    \sa workDays()
*/
bool HbExtendedLocale::setWorkDays( const QString &days )
{
#if defined(Q_OS_SYMBIAN)
    if ( days.size() != 7 ) {
        return false;
    }
    bool ok;
    int mask = days.toInt(&ok, 2);
    _symbianLocale.LoadSystemSettings();
    if ( ok ) {
        _symbianLocale.GetLocale()->SetWorkDays(mask);
        _symbianLocale.GetLocale()->Set();
        return true;
    }
    return false;
#else
    Q_UNUSED(days);
    return false;
#endif
}

/*!
    
    Returns \c true if daylight saving is set for the home city, \c false if it 
    is not set.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns \c false.
    
    \sa homeDaylightSavingZone()

 */

bool HbExtendedLocale::homeHasDaylightSavingOn() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    return (_symbianLocale.GetLocale()->QueryHomeHasDaylightSavingOn());
#else
    return false;
#endif    
}

/*! 
    \enum HbExtendedLocale::DaylightSavingZone
     
    Defines the settings for daylight saving time.

    \sa homeDaylightSavingZone(), homeHasDaylightSavingOn()
*/

/*! 
    \var HbExtendedLocale::HomeZone
    Home daylight saving zone.
*/ 

/*! 
    \var HbExtendedLocale::EuropeanZone
    European daylight saving zone.
*/

/*! 
    \var HbExtendedLocale::NorthernZone
    Northern hemisphere (non-European) daylight saving zone.
*/

/*! 
    \var HbExtendedLocale::SouthernZone
    Southern hemisphere daylight saving zone.
*/

/*! 
    \var HbExtendedLocale::None
    No daylight saving zone.
*/

/*! 
    
    Returns the daylight saving zone in which the home city is located according 
    to HbExtendedLocale::DaylightSavingZone.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns HbExtendedLocale::None.
    
    \sa homeHasDaylightSavingOn()
 */
HbExtendedLocale::DaylightSavingZone HbExtendedLocale::homeDaylightSavingZone() const
{
#if defined(Q_OS_SYMBIAN)    
    _symbianLocale.LoadSystemSettings();
    TDaylightSavingZone zone = _symbianLocale.GetLocale()->HomeDaylightSavingZone();
    switch ( zone ) {
    case EDstHome:
        return HbExtendedLocale::HomeZone;
    case EDstEuropean:
        return HbExtendedLocale::EuropeanZone;
    case EDstNorthern:
        return HbExtendedLocale::NorthernZone;
    case EDstSouthern:
        return HbExtendedLocale::SouthernZone;
    case EDstNone:
        return HbExtendedLocale::None;
    }
    return HbExtendedLocale::None;
#else
    return HbExtendedLocale::None;
#endif    
}

/*!

    Gets the locale's offset in seconds from universal time.
    
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, always returns 0.
    
 */
int HbExtendedLocale::universalTimeOffset() const
{
#if defined(Q_OS_SYMBIAN)    
    _symbianLocale.LoadSystemSettings();
    TTimeIntervalSeconds seconds = _symbianLocale.GetLocale()->UniversalTimeOffset();
    return seconds.Int();
#else
    return 0;
#endif
}

/*!
    Constructor.
    
    \attention Cross-Platform API
 */
HbExtendedLocale::HbExtendedLocale()
{
#if defined(Q_OS_SYMBIAN)
    QLocale::system();
    _symbianLocale.LoadSystemSettings();    
#endif
}

/*!
    Returns a new/dummy copy of HbExtendedLocale.
    
    \attention Cross-Platform API
 */
HbExtendedLocale HbExtendedLocale::system()
{
    // make sure QLocale's lp is updated if in future QApplication does not do it
    // currently this does not do anything
    // just return default set locale
    return HbExtendedLocale();
}

/*!
    
    Formats the specified date into a string according to the specified date 
    format (for example, which date components are included, and if leading 
    zeroes are used).

    \param date The date to be formatted.
    \param dateFormat The date format to be used in the formatting. The possible 
    date formats are defined in the hbi18ndef.h header file.
     
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, returns a localized string representation of the given date in 
    short format.

 */
QString HbExtendedLocale::format( const QDate &date, const QString &dateFormat )
{
#if defined(Q_OS_SYMBIAN)    
    QScopedPointer<CTrapCleanup> sp;
    if (!User::TrapHandler()) {       
       CTrapCleanup* cleanup = CTrapCleanup::New();
       if (cleanup) {
           sp.reset(cleanup);
       }
    }    
    QString resultString;
    _symbianLocale.LoadSystemSettings();
    TDateTime s60DateTime;
         
    //Convert the QDate to match the Symbian Date
    
    int month = date.month();
    int day = date.day();
    if (month>=1 && month<=12 && day>=1 && day<=31 ) {
       if (s60DateTime.Set(date.year(), TMonth(month-1), day-1, 0, 0, 0, 0)) {
           return QString("");
       }
    } 
    else {     
        return QString("");
    }
    
    TTime s60Date(s60DateTime);
    
    HBufC *s60Format = QString2HBufC(dateFormat);
    if (!s60Format) {
        return QString();
    }
    QScopedPointer<HBufC> sp1(s60Format);
        
    HBufC *s60DateStr = HBufC::New(50);
    if (!s60DateStr) {
        return QString();
    }         
    QScopedPointer<HBufC> sp2(s60DateStr);
    
    TPtr s60FormatPtr = s60Format->Des();    
    TPtr s60DatePtr = s60DateStr->Des();
    
    TRAPD(err, s60Date.FormatL( s60DatePtr, s60FormatPtr ));
    if( err ){
        return QString("");
    }
    return TDesC2QString(s60DateStr->Des());
    
#else
    Q_UNUSED(dateFormat);
    return toString(date, ShortFormat );
#endif
}

/*!
    
    Formats the specified time into a string according to the specified time 
    format (for example, which time components are included, and if leading 
    zeroes and am/pm symbols are used).

    \param time The time to be formatted.  
    \param timeFormat The time format to be used in the formatting. The possible 
    time formats are defined in the hbi18ndef.h header file.
     
    \attention Only fully implemented on the Symbian platform. On other 
    platforms, returns a localized string representation of the given time in 
    short format.
    
 */
QString HbExtendedLocale::format( const QTime &time, const QString &timeFormat )
{
#if defined(Q_OS_SYMBIAN)
    QScopedPointer<CTrapCleanup> sp;
    if (!User::TrapHandler()) {       
        CTrapCleanup* cleanup = CTrapCleanup::New();
        if (cleanup) {
            sp.reset(cleanup);
        }
    }    

    QString resultString;
    
    TDateTime s60DateTime;
    if (s60DateTime.Set( 0, TMonth(0), 0, time.hour(), time.minute(), time.second(), time.msec())) {
        return QString();
    }
    
    TTime s60Time(s60DateTime);
    
    HBufC *s60Format = QString2HBufC(timeFormat);
    if (!s60Format) {
        return QString();
    }

    QScopedPointer<HBufC> sp1(s60Format);
    HBufC *s60TimeStr = HBufC::New(50);
    if (!s60TimeStr) {
        return QString();
    }         
    QScopedPointer<HBufC> sp2(s60TimeStr);
    
    TPtr s60FormatPtr = s60Format->Des();
    TPtr s60TimePtr = s60TimeStr->Des();
    
    TRAPD(err, s60Time.FormatL( s60TimePtr, s60FormatPtr ));
    if( err ){
        return QString("");
    }

    QString result = TDesC2QString(s60TimeStr->Des());
    if (timeFormat.compare(r_qtn_time_usual_with_zero) == 0 ||
        timeFormat.compare(r_qtn_time_long_with_zero) == 0) {
        if (!result.at(1).isNumber()) {
            result.prepend("0");
        }
    }
    return result;
#else 
    Q_UNUSED(timeFormat);
    return toString(time, ShortFormat);
#endif  
}
