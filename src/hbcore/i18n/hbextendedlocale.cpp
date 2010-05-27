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
    \brief The HbExtendedLocale class is extension class to QLocale.
    It provides methods to access Symbian locale data absent from QLocale.
    Methods mainly focus to date and time related issues.

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
    
    \sa QLocale
*/

/*!
    Returns the date separator, which can occur in four different positions:
    Beginning of the expression;
    Between the first and second part;
    Between the second and third part;
    At the end of the expression;
    Some of the positions may contain an empty string if a separator is not used in that position in the locale in question.
    
    \attention Symbian specific API

    \return Symbian - One of the four characters used to separate the day,
    month and year components of the date according to the
    system locale    
    \return other platforms - Empty QChar or '\' (depending on index)

    \param index Index of the separator (0-3)
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
    Sets one of the four characters used to separate the day, 
    month and year components of the date according to the
    system locale.
 
    \attention Symbian specific API

    \param ch Character to set,
    \param index Index of the separator (0-3)
 
    \return true for Symbian if succesfull and false for other OS
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
    Retrieves the time separator (for example, colon or full stop).
    
    \attention Symbian specific API

    \return Symbian - One of the four characters used to separate the hour,
    minute and second components of the date according to the
    system locale    
    \return other platforms - Empty QChar or ':' (depending on index) 

    \param index Index of the separator (0-3)
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
    Sets one of the four characters used to separate the hour,
    minute and second components of the date.

    \attention Symbian specific API

    \param ch Character to set,
    \param index Index of the separator (0-3)
    
    \return true for Symbian if succesfull and false for other OS
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
    returns the order of the date components: day-month-year (the most commonly used format)
    month-day-year (mostly used in the U.S. and Canada)
    or year-month-day (used, for example, in Japanese, Chinese and Swedish)
    This enum defines the order of date components.

    \value American American order (mm/dd/yyyy)
    \value European European order (dd/mm/yyyy)
    \value Japanese Japanese order (yyyy/mm/dd)

    \sa dateStyle(), setDateStyle()
 */

/*!
    Returns date style from system locale. The date style is returned according to DateStyle. 
    
    \attention Symbian specific API
    
    \return date style for Symbian and HbExtendedLocale::American for other platforms    
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
    Sets date style to system locale.
    The date style is chosen according to the \a style parameter.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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

    Finds out if the 12-hour or the 24-hour clock is used
    \value Time12 12 hour clock style
    \value Time24 24 hour clock style

    \sa timeStyle(), setTimeStyle()
 */

/*!
    Returns time style from system locale. The time style is returned according to TimeStyle.  
    
    \attention Symbian specific API
    
    \return time style for Symbian and HbExtendedLocale::Time12 for other platforms    
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
    Sets time style to system locale.
    The time style is chosen according to the \a style parameter.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Finds out if the AM/PM symbol is separated by a space from the time expression.
    
    \attention Symbian specific API

    \return Symbian - True if space is inserted between the time and the preceding or trailing am/pm text; otherwise returns false.   
    \return other platforms - true
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
    Sets whether a \a space is inserted between the time and the preceding or trailing am/pm text.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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

    \value Before
    \value After

    Retrieves the position of the AM/PM symbol (before or after the time expression)
    \sa amPmSymbolPosition(), setAmPmSymbolPosition()
    \sa currencySymbolPosition(), setCurrencySymbolPosition()
 */

/*!
    Returns HbExtendedLocale::Before if am/pm text is positioned before time; otherwise returns HbExtendedLocale::After.
    
    \attention Symbian specific API

    \return After/Before for Symbian and HbExtendedLocale::After for other platforms
 */
HbExtendedLocale::SymbolPos HbExtendedLocale::amPmSymbolPosition() const
{
#if defined(Q_OS_SYMBIAN)
    _symbianLocale.LoadSystemSettings();
    TLocalePos position = _symbianLocale.GetLocale()->AmPmSymbolPosition();
    if ( position == ELocaleBefore ) {
    	return HbExtendedLocale::Before;
    } else if ( position == ELocaleAfter ) {			
    	return HbExtendedLocale::After;
    }
#else
    return HbExtendedLocale::After;
#endif
}

/*!
    Sets the am/pm text position.
    The position is chosen according to the \a position parameter.
    
    \attention Symbian specific API

    \return true for Symbian if succesfull and false for other OS
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
    \enum HbExtendedLocale::UnitsFormat
    Retrives the  measurement system (if metric or imperial units are in use) .
    This enum defines which units are used for measurement.

    \value Imperial This value indicates imperial units, such as inches and
             miles. There are several distinct imperial systems in the world; this
             value stands for the official United States imperial units.
    \value Metric This value indicates metric units, such as meters,
             centimeters and millimeters.

    \sa unitsDistanceShort(), unitsDistanceLong()
 */

/*!
    Retrives the  measurement system (if metric or imperial units are in use). 
    
    \attention Symbian specific API

    \return Symbian - Short unit distance format from system locale. Format is specified by UnitsFormat.
    \return other platforms - QLocale::MetricSystem
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
    Sets short unit distance \a format to system locale.

    \attention Symbian specific API
    
    \return true for Symbian and false for other OS
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
    Returns long unit distance format from system locale. Format is specified by UnitsFormat.

    \attention Symbian specific API

    \return long unit distance format for Symbian and QLocale::MetricSystem for other platforms
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
    Sets long unit distance \a format to system locale.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Sets general unit distance \a format to system locale.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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

    Retrives the indicator for negative currency.
    For example, by using a minus sign (with different options as to where it is placed),
    or by placing the negative currency amount in parenthesis.

    \value LeadingMinusSign      A minus sign is inserted before the currency symbol and value.
    \value InBrackets            The currency value and symbol are enclosed in brackets (no minus sign is used).
    \value TrailingMinusSign     A minus sign is inserted after the currency symbol and value.
    \value InterveningMinusSign  A minus sign is inserted between the currency symbol and the value.

    \sa negativeCurrencyFormat(), setNegativeCurrencyFormat()
 */

/*!
    Returns the negative currency format from system locale.
    
    \attention Symbian specific API
    
    \return the negative currency format for Symbian and HbExtendedLocale::LeadingMinusSign for other platforms
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
    Sets negative currency \a format to system locale.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Finds out if the currency symbol is separated by a space from the amount.
    
    \attention Symbian specific API
    
    \return Symbian - True if negative currency values have a space between the currency symbol and the value; otherwise returns false.
    \return other platforms - false
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
    Sets whether negative currency values have a space between the currency symbol and the value.
    
    \attention Symbian specific API

    \param space True to set a flag which indicates that negative currency values should have
           the space between the value and the symbol. False to unset it. 
    
    \return true for Symbian and false for other OS
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
    Finds out if the currency symbol is placed on the opposite side with negative 
    currencies compared to where it is placed with non-negative currencies.

    \attention Symbian specific API

    \return Symbian - True if in negative currency values, the position of the currency
    symbol is set to be the opposite of the position used for non-negative
    values; otherwise returns false.
    \return other platforms - false
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
    Sets whether the position of the currency symbol for negative currency
    values should be the \a opposite of the position used for
    non-negative values.

    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Finds out if currency triads are used (the grouping of digits in large numbers, for example, 123 456 789).
    The Symbian OS only supports the grouping of currency amounts.

    \attention Symbian specific API

    \return Symbian - True if currency triads are allowed in currency values; otherwise returns false.
    \return other platforms - false    
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
    Sets whether triads are \a allowed in currency values.

    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Returns boolean which tells if currency format contains a space.
      
    \attention Symbian specific API

    \return Symbian - True if a space is inserted between the currency symbol and
    a positive currency value; otherwise returns false.
    \return other platforms - false    

    \note For negative currency values, the space can be inserted using
           setNegativeCurrencySpace().
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
     Sets whether a \a space is inserted between the currency symbol and the
     currency amount.

    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    The currency symbol may contain a different number of characters in different countries/regions.
    Example: œ, $, Ft, kn, Euro symbol
    
    \attention Symbian specific API

    \return the currency symbol for Symbian and empty QString for other platforms
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
    Sets the currency \a symbol.

    \attention Symbian specific API
    
    \return true for Symbian if succesfull and false for other OS
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
    Retrieves position of the currency symbol (before or after the amount).
    
    \attention Symbian specific API
    
    \return Symbian - the currency symbol position
    \return other platforms - HbExtendedLocale::Before

    \note For negative currency values, this position may be reversed using 
           SetNegativeCurrencySymbolOpposite().
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
    Sets the currency symbol \a position.
     
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Returns the number of decimal places to which currency values are set.
    
    \attention Symbian specific API
    
    \return Symbian - the number of decimal place
    \return other platforms - '0'   
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
    Sets the number of decimal \a places to which currency values should be set.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Returns a currency \a amount as a string, based on the locale's
    currency and numeric format settings.
    These settings include the currency symbol, the symbol's
    position and how negative values are formatted.

    \attention Cross-Platform API
    \attention Uses QString::number() function with other platforms than Symbian.

    \sa setCurrencyDecimalPlaces(), setCurrencySpace(), setCurrencySymbol(),
        setCurrencySymbolPosition(), setNegativeCurrencySpace(), setNegativeCurrencyFormat(),
        setCurrencyTriadsAllowed(), setNegativeCurrencySymbolOpposite(),
        setGroupSeparator(), setDecimalPoint()
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

    if ( bufPtr == NULL ) {
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

        if ( newBufPtr == NULL ) {
            delete bufPtr;
            return QString();
        }

        bufPtr = newBufPtr;

        TPtr modifiableBufPtr(bufPtr->Des());

        _symbianLocale.LoadSystemSettings();
        _symbianLocale.GetLocale()->FormatCurrency(modifiableBufPtr, overflow, TInt64(amount));
     }

    return QString::fromUtf16(bufPtr->Ptr(), bufPtr->Length());

#else
    return QString::number(amount);
#endif
}

/*!
    Sets decimal point character \a ch to system locale.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Sets group separator character \a ch to system locale.
    
    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    Sets zero digit \a type to system locale.

    \attention Symbian specific API

    \return true for Symbian and false for other OS
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
    int symbian_language;
    char iso_name[8];
};

#if defined(Q_OS_SYMBIAN)
static const symbianToISO symbian_to_iso_list[] = {
    { ELangEnglish,             "en_GB" },
    { ELangFrench,              "fr_FR" },
    { ELangGerman,              "de_DE" },
    { ELangSpanish,             "es_ES" },
    { ELangItalian,             "it_IT" },
    { ELangSwedish,             "sv_SE" },
    { ELangDanish,              "da_DK" },
    { ELangNorwegian,           "no_NO" },
    { ELangFinnish,             "fi_FI" },
    { ELangAmerican,            "en_US" },
    { ELangPortuguese,          "pt_PT" },
    { ELangTurkish,             "tr_TR" },
    { ELangIcelandic,           "is_IS" },
    { ELangRussian,             "ru_RU" },
    { ELangHungarian,           "hu_HU" },
    { ELangDutch,               "nl_NL" },
    { ELangCzech,               "cs_CZ" },
    { ELangSlovak,              "sk_SK" },
    { ELangPolish,              "pl_PL" },
    { ELangSlovenian,           "sl_SI" },
    { ELangTaiwanChinese,       "zh_TW" },
    { ELangHongKongChinese,     "zh_HK" },
    { ELangPrcChinese,          "zh_CN" },
    { ELangJapanese,            "ja_JP" },
    { ELangThai,                "th_TH" },
    { ELangArabic,              "ar_AE" },
    { ELangTagalog,             "tl_PH" },
    { ELangBengali,             "bn_IN" }, // Bangladesh/India
    { ELangBulgarian,           "bg_BG" },
    { ELangCatalan,             "ca_ES" },
    { ELangCroatian,            "hr_HR" },
    { ELangEstonian,            "et_EE" },
    { ELangFarsi,               "fa_IR" },
    { ELangCanadianFrench,      "fr_CA" },
    { ELangGreek,               "el_GR" },
    { ELangGujarati,            "gu_IN" },
    { ELangHebrew,              "he_IL" },
    { ELangHindi,               "hi_IN" },
    { ELangIndonesian,          "id_ID" },
    { ELangKannada,             "kn_IN" },
    { ELangKorean,              "ko_KR" },
    { ELangLatvian,             "lv_LV" },
    { ELangLithuanian,          "lt_LT" },
    { ELangMalay,               "ms_MY" },
    { ELangMalayalam,           "ml_IN" },
    { ELangMarathi,             "mr_IN" },
    { ELangBrazilianPortuguese, "pt_BR" },
    { ELangRomanian,            "ro_RO" },
    { ELangSerbian,             "sr_YU" },
    { ELangLatinAmericanSpanish, "es_MX" },
    { ELangTamil,               "ta_IN" },
    { ELangTelugu,              "te_IN" },
    { ELangUkrainian,           "uk_UA" },
    { ELangUrdu,                "ur_PK" }, // India/Pakistan
    { ELangVietnamese,          "vi_VN" },
#ifdef __E32LANG_H__
// 5.0
    { ELangBasque,              "eu_ES" },
    { ELangGalician,            "gl_ES" },
#endif
    { ELangEnglish_Apac,        "en_APAC" },
    { ELangEnglish_Taiwan,      "en_TW" },
    { ELangEnglish_HongKong,    "en_HK" },
    { ELangEnglish_Prc,         "en_CN" },
    { ELangEnglish_Japan,       "en_JP"},
    { ELangEnglish_Thailand,    "en_TH" },
    { ELangEnglish_India,       "en_IN" },
    { ELangMalay_Apac,          "ms_APAC" },
    { ELangIndonesian_Apac,     "id_APAC" }
};
#endif

/*!
    Returns ISO name corresponding to the Symbian language \a code.
    If the code does not does not correspond to any Symbian language,
    returns a empty string.
    
    \attention Symbian specific API

    \return Symbian - ISO style language code
    \return other platforms - empty QString    
    
    \sa User::Language()
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
    }
    else {
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
    Returns RFC3066 name corresponding to the Symbian language \a code.
    If the code does not does not correspond to any Symbian language,
    returns a empty string.    
    
    \attention Symbian specific API

    \return Symbian - RFC3066 style language code
    \return other platforms - empty QString    
    
    \sa User::Language()
*/
QString HbExtendedLocale::symbianLangToRfc3066( const int code )
{
    return symbianLangToISO(code).replace('_', '-');
}

/*!
    Converts ISO tag to Symbian language code.

    \attention Symbian specific API

    \param langAndCountry ISO tag, example "fi_FI"
    
    \return Symbian language code if successful. With other platforms or if case fails then '-1'.
 */
int HbExtendedLocale::ISOToSymbianLang( const QString &langAndCountry )
{
#if defined(Q_OS_SYMBIAN)
    
    if ( langAndCountry.length() == 0 ){
        return -1;
    }        
    
    int count = sizeof(symbian_to_iso_list)/sizeof(symbianToISO);
    for ( int i = 0; i < count; i++) {
        if ( langAndCountry.compare(symbian_to_iso_list[i].iso_name) == 0 ) {
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
    Returns a Qt version of the given \a sys_fmt Symbian datetime format string.
    Some convertable fields use data from current system locale,
    unwanted locale may cause unexpected results.    
    
    \attention Symbian specific API

    \return Symbian - datetime format string
    \return other platforms - "not supported"    
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
     This enum defines weekdays.

    \sa startOfWeek()
 */

/*!
     Gets first day of the week.
     It is usually Saturday, Sunday or Monday, but the Symbian OS allows setting any weekday as the first.

    \attention Symbian specific API

    \return Symbian - enum of WeekDay
    \return other platforms - HbExtendedLocale::Monday
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
    Sets the day which is considered to be the first day of the week.
    
    \attention Symbian specific API
    
    \param day The first day of the week.
    
    \return true for Symbian and false for other OS
    
    \sa startOfWeek()
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
    Gets days which are working days of week.
    1 meaning workday and 0 as non working day.

    \attention Symbian specific API

    \return Symbian - QString which describes workdays as binary array
    \return other platforms - "0011111"

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
    Sets working days of week.
     
    \attention Symbian specific API
    
     \param days which describes workdays as QString binary array.
            1 meaning workday and 0 non workday.

     \return true for Symbian if succesfull and false for other OS

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
    Checks whether or not daylight saving is set for the home city.
    
    \attention Symbian specific API
    
    \return Symbian - True if home daylight saving is set. False if home daylight saving is not set.
    \return other platforms - false    
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
    Returns the daylight saving zone in which the home city is located.
    
    \attention Symbian specific API

    \value Home Home daylight saving zone
    \value European European daylight saving zone
    \value Northern Northern hemisphere (non-European) daylight saving zone
    \value Southern Southern hemisphere daylight saving zone
    \value None No daylight saving zone
    
    \return Symbian - the daylight saving zone
    \return other platforms - HbExtendedLocale::None
    
    \sa setHomeDaylightSavingZone()
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
    Gets the locale’s universal time offset.
    
    \attention Symbian specific API
    
    \return Offset in seconds from universal time. For other platforms it returns 0.
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
    Constructor of HbExtendedLocale.
    
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
    Returns new/dummy copy of HbExtendedLocale.
    
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
    Formats the given date to a string according to the given date format.
    For example, what date components are included, and if leading zeroes are used.
    This is a function uses the date formats defined in the hbi18ndef.h header file.

    \attention Cross-Platform API

     \param date The date to be formatted.  
     \param dateFormat The wanted format to be used.  
       
     \return the date as a string   
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
    Formats the given time to a string according to the given time format.
    For example, what components are included (hours/minutes/seconds), and if leading zeroes and AM/PM or am/pm symbols are used.
    This is a function uses the time formats defined in the hbi18ndef.h header file.

    \attention Cross-Platform API

    \param time The time to be formatted.  
    \param timeFormat The wanted format to be used.  
       
    \return the time as a string   
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
	return TDesC2QString(s60TimeStr->Des());
#else 
    Q_UNUSED(timeFormat);
    return toString(time, ShortFormat);
#endif	
}
