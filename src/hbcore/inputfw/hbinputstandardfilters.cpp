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

#define HB_DIGIT_ARABIC_INDIC_START_VALUE   0x0660
#define HB_DIGIT_ARABIC_INDIC_END_VALUE     0x0669
#define HB_DIGIT_EASTERN_ARABIC_START_VALUE 0x06F0
#define HB_DIGIT_EASTERN_ARABIC_END_VALUE   0x06F9

/*!
@stable
@hbcore
\class HbPhoneNumberFilter
\brief The HbPhoneNumberFilter class is a phone number editor filter.

This class implements a standard phone number filter. It accepts
characters <tt>0..9</tt>, \c +, \c #, \c *, \c p, and \c w.
Can be used with editors where the Qt::ImhDialableCharactersOnly hint is set
(see Qt::InputMethodHint in Qt).

\sa HbEditorInterface
*/

/*
Returns \c true if \a aChar is a valid number in the current input language.
*/
static bool isValidNumber(QChar aChar)
{
    bool ret = false;

    QLocale::Language language = HbInputSettingProxy::instance()->globalInputLanguage().language();
    QLocale::Language systemLanguage = QLocale::system().language();

    // If both phone language and writing language are same, then language specific
    // digits should be allowed in phone number and digits only editors. If the current
    // language is Arabic, then Arabic-indic digits are supported. Eastern-Arabic digits
    // are supported in Persian and Urdu languages.
    if (language == systemLanguage) {
        if (language == QLocale::Arabic &&
            (aChar >= HB_DIGIT_ARABIC_INDIC_START_VALUE &&
             aChar <= HB_DIGIT_ARABIC_INDIC_END_VALUE)) {
            ret = true;
        } else if ((language == QLocale::Persian ||
                    language == QLocale::Urdu) &&
                   (aChar >= HB_DIGIT_EASTERN_ARABIC_START_VALUE &&
                    aChar <= HB_DIGIT_EASTERN_ARABIC_END_VALUE)) {
            ret = true;
        } else if (aChar >= '0' && aChar <= '9') {
            ret = true;
        }
    } else if (aChar >= '0' && aChar <= '9') {
        ret = true;
    }
    return ret;
}

/*!
Returns the singleton instance. 
*/
HbPhoneNumberFilter *HbPhoneNumberFilter::instance()
{
    static HbPhoneNumberFilter myInstance;
    return &myInstance;
}

HbPhoneNumberFilter::HbPhoneNumberFilter()
{
}

/*!
Destructor. 
*/
HbPhoneNumberFilter::~HbPhoneNumberFilter()
{
}

/*!
Returns \c true if \a character is valid.
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
@stable
@hbcore
\class HbFormattedNumbersFilter
\brief The HbFormattedNumbersFilter class is an editor filter for formatted numbers (such as 1,309).

This class implements a formatted numbers filter. It accepts
characters <tt>0..9</tt>, a decimal point (\c . or \c ,), and a minus sign.
An example of a valid formatted number is <tt>-1,309</tt>.
Can be used with editors where the Qt::ImhFormattedNumbersOnly hint is set
(see Qt::InputMethodHint in Qt).

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbFormattedNumbersFilter *HbFormattedNumbersFilter::instance()
{
    static HbFormattedNumbersFilter myInstance;
    return &myInstance;
}

HbFormattedNumbersFilter::HbFormattedNumbersFilter()
{
}

/*!
Destructor.
*/
HbFormattedNumbersFilter::~HbFormattedNumbersFilter()
{
}

/*!
Returns \c true if \a character is valid.
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
@stable
@hbcore
\class HbDigitsOnlyFilter
\brief The HbDigitsOnlyFilter class is a digits-only editor filter.

This class implements a digits-only filter. It accepts characters
<tt>0..9</tt>. Can be used with editors where the Qt::ImhDigitsOnly hint is set
(see Qt::InputMethodHint in Qt).

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbDigitsOnlyFilter *HbDigitsOnlyFilter::instance()
{
    static HbDigitsOnlyFilter myInstance;
    return &myInstance;
}

HbDigitsOnlyFilter::HbDigitsOnlyFilter()
{
}

/*!
Destructor.
*/
HbDigitsOnlyFilter::~HbDigitsOnlyFilter()
{
}

/*!
Returns \c true if \a character is valid.
*/
bool HbDigitsOnlyFilter::filter(QChar character)
{
    if (isValidNumber(character)) {
        return true;
    }
    return false;
}

/*!
@stable
@hbcore
\class HbUrlFilter
\brief The HbUrlFilter class is a URL editor filter.

This class implements a URL character filter. It accepts
characters allowed in URLs as defined in RFC 1738.

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbUrlFilter *HbUrlFilter::instance()
{
    static HbUrlFilter myInstance;
    return &myInstance;
}

HbUrlFilter::HbUrlFilter()
{
}

/*!
Destructor.
*/
HbUrlFilter::~HbUrlFilter()
{
}

/*!
Returns \c true if \a character is valid.
*/
bool HbUrlFilter::filter(QChar character)
{
    if (character > 0x20 && character < 0x7f) {
        return true;
    }

    QString others(".@,;?'-_&/~*+=/.:,┐?б!'\"-()\\_;+&%*=<>г$ед[]{}~^з#|"); 
    const QChar euro(0x20AC);
    const QChar space(0xE118);
    const QChar enter(0xE125);
    others.append(euro);
    others.append(space);
    others.append(enter);
    for (int i = 0; i < others.size(); i++) {
        if (others[i] == character) {
            return true;
        }
    }
    return false;
}

/*!
@stable
@hbcore
\class HbEmailAddressFilter
\brief The HbEmailAddressFilter class is an e-mail address filter.

This class implements an e-mail address character filter. It accepts
characters allowed in e-mail addresses as defined in RFC 5322.
Can be used with editors where the Qt::ImhEmailCharactersOnly hint is set
(see Qt::InputMethodHint in Qt).

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbEmailAddressFilter *HbEmailAddressFilter::instance()
{
    static HbEmailAddressFilter myInstance;
    return &myInstance;
}

HbEmailAddressFilter::HbEmailAddressFilter()
{
}

/*!
Destructor.
*/
HbEmailAddressFilter::~HbEmailAddressFilter()
{
}

/*!
Returns \c true if \a character is valid.
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
@stable
@hbcore
\class HbInputLowerCaseFilter
\brief The HbInputLowerCaseFilter class is a lower case character filter.

This filter passes only lower case characters and those characters that cannot be classified.

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbInputLowerCaseFilter *HbInputLowerCaseFilter::instance()
{
    static HbInputLowerCaseFilter theInstance;
    return &theInstance;
}

/*!
Returns \c true if \a character is in lower case or the case cannot be determined.
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
@stable
@hbcore
\class HbInputUpperCaseFilter
\brief The HbInputUpperCaseFilter class is an upper case character filter.

This filter passes only upper case characters and those characters that cannot be classified.

\sa HbEditorInterface
*/

/*!
Returns the singleton instance.
*/
HbInputUpperCaseFilter *HbInputUpperCaseFilter::instance()
{
    static HbInputUpperCaseFilter theInstance;
    return &theInstance;
}

/*!
Returns \c true if \a character is in upper case or the case cannot be determined.
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
