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
#include "hbinputstandardfilters.h"

#include "hbinputsettingproxy.h"
/*!
@alpha
@hbcore
\class HbPhoneNumberFilter
\brief Phone number editor filter.

This class implements standard phone number filter. It accepts
characters 0..9, +, #, *, p and w.
Can be used with editors that have Qt::ImhDialableCharactersOnly set.

\sa HbEditorInterface
*/

/* Returns true if the character is a valid number in the current input language
*/
 
static bool isValidNumber(QChar aChar)
{
    bool ret = false;

    QLocale::Language language = HbInputSettingProxy::instance()->globalInputLanguage().language();
	QLocale::Language systemLanguage = QLocale::system().language();

	if (language != systemLanguage) {
        if (aChar >= '0' && aChar <= '9') {
            ret = true;
        }	
	} else if (language == QLocale::Arabic) {
        if (aChar >= 0x0660 && aChar <= 0x0669) {
            ret = true;
        }
    } else if (aChar >= '0' && aChar <= '9') {
        ret = true;
    }         
    return ret;	
}
HbPhoneNumberFilter* HbPhoneNumberFilter::instance()
{
    static HbPhoneNumberFilter myInstance;
    return &myInstance;
}

HbPhoneNumberFilter::HbPhoneNumberFilter()
{
}

HbPhoneNumberFilter::~HbPhoneNumberFilter()
{
}

/*!
Returns true if given character is valid.
*/
bool HbPhoneNumberFilter::filter(QChar character)
{
    if (character == '+' || character == '*'
        || character == '#' || character == 'p'
        || character == 'w') {
        return true;
    }  else if (isValidNumber(character)) {
        return true;
    }	
    return false;
}

/*!
@alpha
@hbcore
\class HbFormattedNumbersFilter
\brief Converter number editor filter.

This class implements formatted numbers filter. It accepts
characters 0..9, decimal point and minus sign.
Can be used with editors that have Qt::ImhFormattedNumbersOnly hint set.

\sa HbEditorInterface
*/

HbFormattedNumbersFilter* HbFormattedNumbersFilter::instance()
{
    static HbFormattedNumbersFilter myInstance;
    return &myInstance;
}

HbFormattedNumbersFilter::HbFormattedNumbersFilter()
{
}

HbFormattedNumbersFilter::~HbFormattedNumbersFilter()
{
}

/*!
Returns true if given character is valid.
*/
bool HbFormattedNumbersFilter::filter(QChar character)
{
    if (character == '.' || character == ',' || character == '-') {
        return true;
    } else if (isValidNumber(character)) {
        return true;
    }
    return false;
}

/*!
@alpha
@hbcore
\class HbDigitsOnlyFilter
\brief digits only editor filter.

This class implements digits only filter. It accepts
characters 0..9
Can be used with editors that have Qt::ImhDigitsOnly hint set.

\sa HbEditorInterface
*/
HbDigitsOnlyFilter* HbDigitsOnlyFilter::instance()
{
    static HbDigitsOnlyFilter myInstance;
    return &myInstance;
}

HbDigitsOnlyFilter::HbDigitsOnlyFilter()
{
}

HbDigitsOnlyFilter::~HbDigitsOnlyFilter()
{
}

/*!
Returns true if given character is valid.
*/
bool HbDigitsOnlyFilter::filter(QChar character)
{
    if (isValidNumber(character)) {
        return true;
    }
    return false;
}

/*!
@alpha
@hbcore
\class HbUrlFilter
\brief Url editor filter.

This class implements url character filter. It accepts
characters as defined in rfc1738.

\sa HbEditorInterface
*/

HbUrlFilter* HbUrlFilter::instance()
{
    static HbUrlFilter myInstance;
    return &myInstance;
}

HbUrlFilter::HbUrlFilter()
{
}

HbUrlFilter::~HbUrlFilter()
{
}

/*!
Returns true if given character is valid.
*/
bool HbUrlFilter::filter(QChar character)
{
    if (character > 0x20 && character < 0x7f) {
        return true;
    }

    return false;
}

/*!
@proto
@hbcore
\class HbEmailAddressFilter
\brief EMail address filter filter.

This class implements email address character filter. It accepts
characters as defined in rfc5322.
Can be used with editors that have Qt::ImhEmailCharactersOnly set.

\sa HbEditorInterface
*/

HbEmailAddressFilter* HbEmailAddressFilter::instance()
{
    static HbEmailAddressFilter myInstance;
    return &myInstance;
}

HbEmailAddressFilter::HbEmailAddressFilter()
{
}

HbEmailAddressFilter::~HbEmailAddressFilter()
{
}

/*!
Returns true if given character is valid.
*/
bool HbEmailAddressFilter::filter(QChar character)
{
    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z') ||
        (character >= '0' && character <= '9')) {
    return true;
    }

    const QString others(".@,;?'-_&/~*+="); 
    for (int i = 0; i < others.size(); i++) {
        if (others[i] == character) {
            return true;
        }
    }

    return false;
}


/*!
@proto
@hbcore
\class HbInputLowerCaseFilter
\brief Lower case character filter.

This filter passes only lower case characters and those characters that cannot be classified.

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbInputLowerCaseFilter* HbInputLowerCaseFilter::instance()
{
    static HbInputLowerCaseFilter theInstance;
    return &theInstance;
}

/*!
Returns true if the given character is in lower case or the case cannot be determined.
*/
bool HbInputLowerCaseFilter::filter(QChar character)
{
    QChar lowerCase = character.toLower();
    if (lowerCase != character) {
        return false;
    }

    return true;
}


/*!
@proto
@hbcore
\class HbInputUpperCaseFilter
\brief Upper case character filter.

This filter passes only upper case characters and those characters that cannot be classified.

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbInputUpperCaseFilter* HbInputUpperCaseFilter::instance()
{
    static HbInputUpperCaseFilter theInstance;
    return &theInstance;
}

/*!
Returns true if the given character is in upper case or the case cannot be determined.
*/
bool HbInputUpperCaseFilter::filter(QChar character)
{
    QChar upperCase = character.toUpper();
    if (upperCase != character) {
        return false;
    }

    return true;
}

// End of file
