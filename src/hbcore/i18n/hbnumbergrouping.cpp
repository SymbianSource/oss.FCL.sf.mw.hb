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

#include "hbnumbergrouping.h"
#include "hbngnormalnumber_p.h"

/*!
    @stable
    @hbcore
    \class HbNumberGrouping
    \brief The HbNumberGrouping class formats numbers according to the conventions of a certain locale. 
    
    The grouping of numbers may vary in different locales. For example, for 
    Finland, the number \c "123456789.00" would be formatted as \c "123 456 
    789,00". The grouping may also vary depending on the context of where 
    numbers are used. HbNumberGrouping provides functions for grouping currency 
    values and generic numbers.

    For example, to format \c "1234567890" according to the current system 
    locale:
    
    \code
    QString number("1234567890");
    QString result = HbNumberGrouping::formatGeneric(number);
    \endcode
    
    Use HbStringUtil to display the number using the locale digit type, and use 
    HbExtendedLocale and QLocale for other currency formatting.
    
*/

/*!
       
    Returns a number grouped based on the given country format, or the current 
    locale.
    
    \param number The source number string to be grouped. The characters must be 
    Latin digits, and always use '.' as the decimal separator. '+' and '-' 
    characters are also supported. No other characters are supported.
    
    \param country (optional) The QLocale::Country enumeration (in QLocale) that 
    determines the format to be used for converting the number. If \a country is 
    not given, the number is grouped based on the current locale.

    \return The modified number as a QString if successful, otherwise an empty 
    QString.
    
*/
QString HbNumberGrouping::formatGeneric( const QString &number,
                                         QLocale::Country country )                          
{
    if ( number.size() == 0 ) {
        return QString();
    }
    
    if ( country == QLocale::AnyCountry) {
        QLocale locale;
        country = locale.country();
    }
    
    return HbNgNormalNumber::normalNumberGrouping(number, country);
}

/*!
   
    Returns a currency value grouped based on the given country format, or the 
    current locale.

    \param number The source number string to be grouped. The characters must be 
    Latin digits, and always use '.' as the decimal separator. '+' and '-' 
    characters are also supported. No other characters are supported.
    
    \param country (optional) The QLocale::Country enumeration (in QLocale) that 
    determines the format to be used for converting the number. If \a country is 
    not given, the number is grouped based on the current locale.

    \return The modified number as a QString if successful, otherwise an empty 
    QString.
    
    \note The currency symbol is not added to the return value. Use 
    HbExtendedLocale to handle the currency symbol.
    
*/
QString HbNumberGrouping::formatCurrency( const QString &number,
                                          QLocale::Country country )                             
{
    if ( number.size() == 0 ) {
        return QString();
    }
    
    if ( country == QLocale::AnyCountry) {
        QLocale locale;
        country = locale.country();
    }
    
    return HbNgNormalNumber::normalNumberGrouping(number, country);
}

/*!
    \internal
    
    Returns a phone number grouped based on the given country format, or the 
    current locale.
    
    \note The grouping of phone numbers is not supported yet.

    \param number The source number for grouping. The characters must be 
    Latin digits, and always use '.' as the decimal separator. '+' and '-' 
    characters are also supported. No other characters are supported.
    
    \param country (optional) The QLocale::country numeration (in QLocale) that 
    determines the format to be used for converting the number. If \a country is 
    not given, the number is grouped based on the current locale.

    \return The modified number as a QString if successful, otherwise an 
    empty QString.
    
*/
QString HbNumberGrouping::formatPhoneNumber( const QString &number,
                                             QLocale::Country country )                          
{
    if ( number.size() == 0 ) {
        return QString();
    }
    
    if ( country == QLocale::AnyCountry) {
        QLocale locale;
        country = locale.country();
    }

    return QString();
}
