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
#include "hbinputlanguage.h"
#include "hbinpututils.h"

/*!
\alpha
\class HbInputLanguage
\brief Holds Input language value.

When enumerating input languages, sometimes a single QLocale::Language value is not enough. For
example in case of Chinese. QLocale defines only one language constant for Chinese, QLocale::Chinese,
but input framework supports three Chinese dialect. We need a secondary value for the dialect and
we're using QLocale::Country values for that. In most cases, however,  the variant field is not needed.
QLocale itself has those two fields but using it would have been an overkill for this purpuse.
This class also implements a set of language related convenience methods.
*/

//
// Localized language names.
//
const ushort HbLangNameEnglish[] = {'E', 'n', 'g', 'l', 'i', 's', 'h', 0};
const ushort HbLangNameLithuanian[] = {'L', 'i', 'e', 't', 'u', 'v', 0x0173, 0};
const ushort HbLangNameMalay[] = {'M', 'e', 'l', 'a', 'y', 'u', 0};
const ushort HbLangNamePolish[] = {'P', 'o', 'l', 's', 'k', 'i', 0};
const ushort HbLangNamePortuguese[] = {'P', 'o', 'r', 't', 'u', 'g', 0x00ea, 's', 0};
const ushort HbLangNameRomanian[] = {'R', 'o', 0x00e2, 'n', 0x0103, 0};
const ushort HbLangNameSerbian[] = {'S', 'r', 'p', 's', 'k', 'i', 0};
const ushort HbLangNameSlovak[] = {'S', 'l', 'o', 'v', 'e', 'n', 0x010d, 'i', 'n', 'a', 0};
const ushort HbLangNameSlovene[] = {'S', 'l', 'o', 'v', 'e', 'n', 0x0161, 0x010d, 'i', 'n', 'a', 0};
const ushort HbLangNameSpanish[] = {'E', 's', 'p', 'a', 0x00f1, 'o', 'l', 0};
const ushort HbLangNameSwedish[] = {'S', 'w', 'e', 'd', 'i', 's', 'h', 0};
const ushort HbLangNameTagalog[] = {'P', 'i', 'l', 'i', 'p', 'i', 'n', 'o', 0};
const ushort HbLangNameCzech[] = {0x010c, 'e', 0x0161, 't', 'i', 'n', 'a', 0};
const ushort HbLangNameDutch[] = {'N', 'e', 'd', 'e', 'r', 'l', 'a', 'n', 'd', 's', 0};
const ushort HbLangNameTurkish[] = {'T', 0x00fc, 'r', 'k', 0x00e7, 'e', 0};
const ushort HbLangNameEstonian[] = {'E', 'e', 's', 't', 'i', 0};
const ushort HbLangNameFrench[] = {'F', 'r', 'a', 'n', 0x00e7, 'a', 'i', 's', 0};
const ushort HbLangNameGreek[] = {0x0395, 0x03bb, 0x03bb, 0x03b7, 0x03bd, 0x03b9, 0x03ba, 0x03ac, 0};
const ushort HbLangNameIcelandic[] = {0x00cd, 's', 'l', 'e', 'n', 's', 'k', 'a', 0};
const ushort HbLangNameIndonesian[] = {'I', 'n', 'd', 'o', 'n', 'e', 's', 'i', 'a', 0};
const ushort HbLangNameItalian[] = {'I', 't', 'a', 'l', 'i', 'a', 'n', 'o', 0};
const ushort HbLangNameLatvian[] = {'L', 'a', 't', 'v', 'i', 'e', 0x0161, 'u', 0};
const ushort HbLangNameCroatian[] = {'C', 'r', 'o', 'a', 't', 'i', 'a', 'n', 0};
const ushort HbLangNameGerman[] = {'D', 'e', 'u', 't', 'c', 'h', 0};
const ushort HbLangNameHungarian[] = {'M', 'a', 'g', 'y', 'a', 'r', 0};
const ushort HbLangNameBulgarian[] = {0x0411, 0x044a, 0x043b, 0x0433, 0x0430, 0x0440, 0x0441, 0x043a, 0x0438, 0};
const ushort HbLangNameFinnish[] = {'S', 'u', 'o', 'm', 'i', 0};
const ushort HbLangNameRussian[] = {0x0420, 0x0443, 0x0441, 0x0441, 0x043a, 0x0438, 0x0439, 0};
const ushort HbLangNameDanish[] = {'D', 'a', 'n', 's', 'k', 0};
const ushort HbLangNameNorwegian[] = {'N', 'o', 'r', 's', 'k', 0};
const ushort HbLangNameUkrainian[] = {0x0423, 0x043a, 0x0440, 0x0430, 0x0457, 0x043d, 0x0441, 0x044c, 0x043a, 0x0430, 0};
const ushort HbLangNameArabic[] = {0x0627, 0x0644, 0x0639, 0x0631, 0x0628, 0x064a, 0x0629, 0};
const ushort HbLangNameHebrew[] = {0x05e2, 0x05d1, 0x05e8, 0x05d9, 0x05ea, 0};
const ushort HbLangNameThai[] = {0x0e20, 0x0e32, 0x0e29, 0x0e32, 0x0e44, 0x0e17, 0x0e22, 0};
const ushort HbLangNameJapanese[] = {0x65e5, 0x672c, 0x8a9e, 0};
const ushort HbLangNameVietnamese[] = {'T', 'i', 0x00ea, 0x0301, 'n', 'g', ' ', 'V', 'i', 0x00ea, 0x0323, 't', 0};
const ushort HbLangNameFarsi[] = {0x0641, 0x0627, 0x0631, 0x0633, 0x0649, 0};
const ushort HbLangNameHindi[] = {0x0939, 0x093f, 0x0928, 0x0928, 0x094d, 0x0926, 0x0940, 0};
const ushort HbLangNameUrdu[] = {0x0627, 0x0631, 0x062f, 0x0648, 0};
const ushort HbLangNameCatalan[] = {'C', 'a', 't', 'a', 'l', 0x00e0, 0};
const ushort HbLangNameGalician[] = {'G', 'a', 'l', 'e', 'g', 'o', 0};
const ushort HbLangNameBasque[] = {'E', 'u', 's', 'k', 'a', 'r', 'a', 0};
const ushort HbLangNameMarathi[] = {0x092e, 0x0930, 0x093e, 0x0920, 0x0940, 0};
const ushort HbLangNameChinesePrc[] = {0x7B80, 0x4F53, 0x4E2D, 0x6587, 0};
const ushort HbLangNameChineseHongKong[] = {0x7E41, 0x9AD4, 0x4E2D, 0x6587, 0x0028, 0x9999, 0x6E2F, 0x0029, 0};
const ushort HbLangNameChineseTaiwan[] = {0x7E41, 0x9AD4, 0x4E2D, 0x6587, 0x0028, 0x53F0, 0x7063, 0x0029, 0};
const ushort HbLangNameKorean[] = {'K', 'o', 'r', 'e', 'a', 'n', 0};

//
// Helper class for binding language names to QLocale constants.
//
class HbLocNameBinding
{
public:
    int mLanguage;
    const ushort *mName;
};

//
// This table binds QLocale language value to language name string.
//
const HbLocNameBinding nameBindings[] = {
    {QLocale::English, HbLangNameEnglish},
    {QLocale::Lithuanian, HbLangNameLithuanian},
    {QLocale::Malay, HbLangNameMalay},
    {QLocale::Polish, HbLangNamePolish},
    {QLocale::Portuguese, HbLangNamePortuguese},
    {QLocale::Romanian, HbLangNameRomanian},
    {QLocale::Serbian, HbLangNameSerbian},
    {QLocale::Slovak, HbLangNameSlovak},
    {QLocale::Slovenian, HbLangNameSlovene},
    {QLocale::Spanish, HbLangNameSpanish},
    {QLocale::Swedish, HbLangNameSwedish},
    {QLocale::Tagalog, HbLangNameTagalog},
    {QLocale::Czech, HbLangNameCzech},
    {QLocale::Dutch, HbLangNameDutch},
    {QLocale::Turkish, HbLangNameTurkish},
    {QLocale::Estonian, HbLangNameEstonian},
    {QLocale::French, HbLangNameFrench},
    {QLocale::Greek, HbLangNameGreek},
    {QLocale::Icelandic, HbLangNameIcelandic},
    {QLocale::Indonesian, HbLangNameIndonesian},
    {QLocale::Italian, HbLangNameItalian},
    {QLocale::Latvian, HbLangNameLatvian},
    {QLocale::Croatian, HbLangNameCroatian},
    {QLocale::German, HbLangNameGerman},
    {QLocale::Hungarian, HbLangNameHungarian},
    {QLocale::Bulgarian, HbLangNameBulgarian},
    {QLocale::Finnish, HbLangNameFinnish},
    {QLocale::Russian, HbLangNameRussian},
    {QLocale::Danish, HbLangNameDanish},
    {QLocale::Norwegian, HbLangNameNorwegian},
    {QLocale::Ukrainian, HbLangNameUkrainian},
    {QLocale::Arabic, HbLangNameArabic},
    {QLocale::Hebrew, HbLangNameHebrew},
    {QLocale::Thai, HbLangNameThai},
    {QLocale::Japanese, HbLangNameJapanese},
    {QLocale::Vietnamese, HbLangNameVietnamese},
    {QLocale::Persian, HbLangNameFarsi},
    {QLocale::Hindi, HbLangNameHindi},
    {QLocale::Urdu, HbLangNameUrdu},
    {QLocale::Catalan, HbLangNameCatalan},
    {QLocale::Galician, HbLangNameGalician},
    {QLocale::Basque, HbLangNameBasque},
    {QLocale::Marathi, HbLangNameMarathi},
    {QLocale::Korean, HbLangNameKorean},
    {0, 0}
};

/*!
\fn QLocale::Language language() const

Returns languge code as QLocale::Language.
*/

/*!
\fn QLocale::Country variant() const

Returns variant code as QLocale::Country.
*/

/*!
\fn bool undefined() const

Returns true if the language is undefined (language() returns 0).
*/

/*!
\fn bool defined()

Returns true if this structure is initialised to contain a valid language/variant pair.
*/

/*!
Returns localized language name for given language.
Chinese requires valid value in language's variant field,
otherwise null string will be returned. Valid variants are
QLocale::China, QLocale::Taiwan and QLocale::HongKong.

\sa HbInputSettingProxy
*/
QString HbInputLanguage::localisedName()
{
    QString result;

    if (mLanguage == QLocale::Chinese) {
        switch (mVariant) {
        case QLocale::China:
            result = QString::fromUtf16(HbLangNameChinesePrc);
            break;
        case QLocale::Taiwan:
            result = QString::fromUtf16(HbLangNameChineseTaiwan);
            break;
        case QLocale::HongKong:
            result = QString::fromUtf16(HbLangNameChineseHongKong);
            break;
        default:
            break;
        }
    }

    for (int i = 0; nameBindings[i].mLanguage != 0; i++) {
        if (nameBindings[i].mLanguage == mLanguage) {
            result = QString::fromUtf16(nameBindings[i].mName);
            break;
        }
    }

    return result;
}

/*!
Returns true if the language is case sensitive.
*/
bool HbInputLanguage::isCaseSensitiveLanguage() const
{
    if (mLanguage == QLocale::Arabic || mLanguage == QLocale::Hebrew
        || mLanguage == QLocale::Urdu || mLanguage == QLocale::Chinese
        || mLanguage == QLocale::Thai || mLanguage == QLocale::Japanese
        || mLanguage == QLocale::Persian) {
        // These languageas don't have a concept of upper and lower case.
        return false;
    }

    return true;
}

const int knownLatinAlphabetLanguages[] = {
    QLocale::English,
    QLocale::Lithuanian,
    QLocale::Malay,
    QLocale::Polish,
    QLocale::Portuguese,
    QLocale::Romanian,
    QLocale::Serbian,
    QLocale::Slovak,
    QLocale::Slovenian,
    QLocale::Spanish,
    QLocale::Swedish,
    QLocale::Tagalog,
    QLocale::Czech,
    QLocale::Dutch,
    QLocale::Turkish,
    QLocale::Estonian,
    QLocale::French,
    QLocale::Icelandic,
    QLocale::Indonesian,
    QLocale::Italian,
    QLocale::Latvian,
    QLocale::Croatian,
    QLocale::German,
    QLocale::Hungarian,
    QLocale::Finnish,
    QLocale::Danish,
    QLocale::Norwegian,
    QLocale::Vietnamese,
    QLocale::Catalan,
    QLocale::Galician,
    QLocale::Basque,
    0
};

/*!
Returns true if given language is a latin alphabet language.
*/
bool HbInputLanguage::isLatinAlphabetLanguage() const
{
    // Right now we'll just use hard coded list...
    for (int i = 0; knownLatinAlphabetLanguages[i] != 0; i++) {
        if (knownLatinAlphabetLanguages[i] == mLanguage) {
            return true;
        }
    }

    return false;
}

/*!
Returns true if the given language is a right-to-left language. Otherwise returns false.
*/
bool HbInputLanguage::isRightToLeftLanguage() const
{
    if (mLanguage == QLocale::Arabic || mLanguage == QLocale::Hebrew
        || mLanguage == QLocale::Urdu || mLanguage == QLocale::Persian) {
        return true;
    }

    return false;
}

/*!
Returns language in string format. Returned string contains language and country values in an integer
format, separated by a space.

\sa fromString
*/
QString HbInputLanguage::asString() const
{
    return QString(QString::number(language()) + QString(" ") + QString::number(variant()));
}

/*!
Parses language from given string. The string must have same format as returned by asString() method.
Invalid string will set both languag and country codes to zero (in which case undefined() will
return true).

\sa asString
*/
void HbInputLanguage::fromString(const QString &string)
{
    mLanguage = (QLocale::Language)0;
    mVariant = (QLocale::Country)0;

    QStringList parts = string.split(QChar(' '));
    if (parts.count() == 2) {
        mLanguage = (QLocale::Language)parts[0].toLong();
        mVariant = (QLocale::Country)parts[1].toLong();
    }
}

// End of file
