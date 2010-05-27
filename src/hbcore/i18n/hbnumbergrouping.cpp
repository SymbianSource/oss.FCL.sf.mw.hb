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
    \brief HbNumberGrouping formats numbers according to conventions of a certain country. 
    Eg. for Finland: "123456789.00" => "123 456 789,00".
    
    NOTE!
    - Characters in 'number' parameter needs to be latin numbers even though the type of parameter is QString.
    - Also '+' and '-' characters are supported.
    - Decimal separator needs to be a dot '.' always.
    - Any other characters than above descripted are not allowed.
    - If grouping fails for some reason then return value is empty QString.
    - At the moment currency mark is not added to return value (formatCurrency).
    - Grouping for phone numbers is not activated yet.    

    Example:

    \snippet{unittest_hbnumbergrouping/unittest_hbnumbergrouping.cpp,1} 
	
*/

/*!
    Static function for creating a (generic) number group.
    
    \attention Cross-Platform API

    \param number Source number for grouping.
    \param country Format for number converting. 
    If the country is not given (country = QLocale::AnyCountry) then it will be requested from QLocale::country.

    \return modified string.
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
    Static function for creating a currency group.
    
    \attention Cross-Platform API

    \param number Source number for grouping.
    \param country Format for number converting.
    If the country is not given (country = QLocale::AnyCountry) then it will be requested from QLocale::country.

    \return modified string.
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
    Static function for creating a phone number group.

    \attention Cross-Platform API

    \param number Source number for grouping.
    \param country Format for number converting.
    If the country is not given (country = QLocale::AnyCountry) then it will be requested from QLocale::country.

    \return modified string.
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
