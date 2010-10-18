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

#include <hbextendedlocale.h>
#include <QtAlgorithms>

#include "hbstringutil_p.h"
#include "hbstringutil.h"

#if defined(__SYMBIAN32__)
#include <e32std.h>
#include <e32def.h>
#include <e32des16.h>
#include <collate.h>
#define Q_OS_SYMBIAN
#endif

#ifndef QT_NO_REGEXP
#include <QRegExp>
#endif

/*!
    @stable
    @hbcore
    \class HbStringUtil
    \brief The HbStringUtil class supports locale-based comparing and ordering of strings, and digit conversion.
    
    \section _hbstringutil_stringcomparison Comparing and ordering strings
    
    Using HbStringUtil, you can execute locale-aware operations on collated 
    strings, such as comparisons and finding data sequences in target strings. 
    HbStringUtil also provides similar functions for operations on folded data.
    
    Choose the right HbStringUtil functions for your purpose, depending on 
    whether you want to operate on collated or folded data:
    
    <ul>
    
    <li>Because different languages have different alphabets, they also require 
    different ways of ordering strings. The technical term for ordering and 
    comparing strings in a locale-specific manner is \b collating. When 
    comparing strings in natural languages, it is recommended that you use the 
    HbStringUtil collation functions: matchC(), compareC() and findC(), and 
    sort() for operations on multiple strings.
    
    For example, for languages using the Latin script, collation defines when 
    punctuation should be ignored, how accents are handled, and so on. A 
    locale's settings usually include rules for collation.
    
    As an example of handling accents, in German, the letter "o with umlaut" or 
    "ö" is just a variation of "o", whereas in Swedish it is the letter "ö", 
    which comes last in the alphabet. There can also be country-specific 
    variations within a language, for example, between Iberian Spanish and Latin 
    American Spanish.</li>
    
    <li>\b Folding normalises text for comparison, for example, by removing case 
    distinctions and accents from characters. This can be useful, for example, 
    if you need to determine if two file names are identical.
    
    Folding is not locale-aware. For example, folding is not able to handle 
    character relationships that are not one-to-one, for example, mapping from 
    the German uppercase SS to the lowercase ß. When you need to compare 
    strings in natural languages, use collation instead.</li>
    
    </ul>
    
    \section _hbstringutil_digitconversion Digit conversion
    
    Internal processing of numeric expressions uses Latin digits. To display 
    digits correctly to the user, you need to convert them to the appropriate 
    digit type.
   
    If you are inserting numbers into a localizable string at runtime using 
    arguments, you can localize the digit type used for the numbers using the \c 
    L notation in the placeholder in the string (see hbTrId()). However, this 
    only works for integers. When displaying more complex numeric expressions 
    such as dates and times, use HbStringUtil to convert the digits to the digit 
    type appropriate to the UI language.

    \note In internal processing, converting numbers from one digit type to 
    another is not always allowed. For example, native digits are filtered out 
    of IP addresses, and native digits in phone numbers are converted and sent 
    to the network as Latin digits. This is because different networks may not 
    be able to handle native digits.
    
    HbExtendedLocale, QLocale and HbNumberGrouping also provide functions for 
    locale-dependent number formatting.

    \sa HbLocaleUtil
*/

/*!
    \enum HbStringUtil::Option

    Defines the collation levels ('flags') for matchC() and compareC(). This is 
    based on the Symbian TCollationMethod enumeration, and it is not used on 
    other platforms.
    
*/

/*!
    \var HbStringUtil::Default
    Use the default system flags.
   
*/

/*!     
    \var HbStringUtil::IgnoreNone
    
    Do not ignore any keys (by default, for example, punctuation marks and 
    spaces are ignored).

*/
    
/*!    
    \var HbStringUtil::SwapCase
    
    Reverse the normal order for characters that only differ in case.

    
*/
    
/*!    
    \var HbStringUtil::AccentsBackwards
    
    Compare secondary keys representing accents in reverse order (from right to 
    left). This is needed for French when comparing words that only differ in 
    accents.

    
*/
    
/*!    
    \var HbStringUtil::SwapKana
    
    Reverse the normal order for characters that only differ in whether they are 
    Katakana or Hiragana.

    
*/
   
/*!    
    \var HbStringUtil::FoldCase
    
    Fold all characters to lowercase before extracting keys. This is needed when 
    comparing file names; case is ignored but other Unicode collation level 2 
    distinctions are not.

*/
   
/*!    
    \var HbStringUtil::MatchingTable
    
    Specify a specific collation method to be used for matching purposes.
    
*/
   
/*!    
    \var HbStringUtil::IgnoreCombining
    
    Ignore a check for adjacent combining characters. A combining character 
    effectively changes the character it combines with into another character, 
    which means a match does not occur. Setting this flag allows character 
    matching regardless of any combining characters.
        
*/

/*!
      
    Searches \a strFrom for a match of \a strToMatch based on the locale's 
    collation settings. You can optionally specify the level of collation with 
    \a maxLevel and \a flags, and the wild card and escape characters for the 
    search with \a wildChar, \a wildSequenceChar and \a escapeChar. If the 
    parameters are not specified, the default values are used.
    
    \param strFrom The source string.    
    \param strToMatch The string whose data is to be searched within the source 
    string. The value can contain the wildcard characters "*" and "?", where "*" 
    matches zero or more consecutive occurrences of any character, and "?" 
    matches a single occurrence of any character (default).
    \param maxLevel (optional) The level of collation. Possible values:
    - \c 0: Only character identities are distinguished.
    - \c 1: Character identities and accents are distinguished.
    - \c 2: Character identities, accents, and case are distinguished.    
    - \c 3: All valid Unicode characters are considered different.
    - For details, see Symbian documentation on collation, for example, as used 
    by TDesC16::MatchC().
    \param flags A list of (comma-separated) HbStringUtil::Option flags that 
    will be used. The default value is \c Default.
    \param wildChar (optional) Wild card character ('?' by default).
    \param wildSequenceChar (optional) Wild card sequence character ('*' by default).
    \param escapeChar (optional) The escape character, for example, '?', '*' or '\\\\' (default).
    
    \return The offset from the beginning of \a strFrom where the match first 
    occurs. If the data sequence in \a strToMatch is not found, returns -1.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,3}
    
    \attention On the Symbian platform, this class uses a Symbian-specific 
    collation match. On other platforms, the search is not locale-based, and 
    only the \a strFrom and \a strToMatch parameters are used.
    
    \sa findC()
 */
int HbStringUtil::matchC( const QString &strFrom, const QString &strToMatch,
                                    int maxLevel, Options flags,
                                    int wildChar, int wildSequenceChar, int escapeChar )
{
#if defined( Q_OS_SYMBIAN )
    TPtrC s1Ptr( strFrom.utf16() );
    TPtrC s2Ptr( strToMatch.utf16() );

    if ( (maxLevel < 0) || (maxLevel > 3) ) {
        maxLevel = 0;
    }
    if ( (flags < 0) || (flags > 127) ) {
        flags = Default;
    }

    TCollationMethod m = *Mem::GetDefaultMatchingTable();
    m.iFlags |= flags;

    return s1Ptr.MatchC(s2Ptr, wildChar, wildSequenceChar, escapeChar, maxLevel, &m);
#else
    Q_UNUSED(maxLevel);
    Q_UNUSED(flags);
    Q_UNUSED(wildChar);
    Q_UNUSED(wildSequenceChar);
    Q_UNUSED(escapeChar);
#ifdef QT_NO_REGEXP
    // if no regular expressions defined do standard MatchF
    return strFrom.indexOf( strToMatch, 0, Qt::CaseSensitive );
#else    
    // works with standard wildcards
    QRegExp locStrToMatch( strToMatch, Qt::CaseSensitive, QRegExp::Wildcard );    
    return strFrom.indexOf( locStrToMatch, 0 );
#endif    
    
#endif
}

/*!
 
    Compares \a string1 with \a string2 based on the locale's collation 
    settings. You can optionally specify the level of collation with \a maxLevel 
    and \a flags. If the parameters are not specified, the default values are 
    used.
    
    \param string1 The source string.
    \param string2 The string whose data is to be compared with the source string.
    \param maxLevel (optional) The level of collation. Possible values:
    - \c 0: Only character identities are distinguished.
    - \c 1: Character identities and accents are distinguished.
    - \c 2: Character identities, accents, and case are distinguished.    
    - \c 3: All valid Unicode characters are considered different (default).
    - For details, see Symbian documentation on collation, for example, as used 
    by TDesC16::CompareC().
    \param flags (optional) A list of (comma-separated) HbStringUtil::Option flags that 
    will be used. The default value is \c Default.
        
    \return Positive if the \a string1 is greater (that is, comes after \a 
    string2 when the strings are ordered), negative if \a string2 is greater, 
    and zero if the content of the strings matches.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,1}
    
    \attention Locale-specific collation settings are used, and the return value 
    may vary on different platforms. The \a maxLevel and \a flags parameters are 
    not used.
    
    \sa compareF()
 */
int HbStringUtil::compareC( const QString &string1, const QString &string2,
                                        int maxLevel, Options flags )
{
#if defined( Q_OS_SYMBIAN )
    TPtrC s1Ptr(string1.utf16());
    TPtrC s2Ptr(string2.utf16());
   
    if ( (maxLevel < 0) || (maxLevel > 3) ) {
        maxLevel = 3;
    }
    if ( (flags < 0) || (flags > 127) ) {
        flags = Default;
    }
    
    TCollationMethod m = *Mem::CollationMethodByIndex( 0 );
    m.iFlags |= flags;

    return s1Ptr.CompareC( s2Ptr, maxLevel, &m);
#else
    Q_UNUSED(maxLevel);
    Q_UNUSED(flags);
    return string1.localeAwareCompare( string2 );
#endif    
}

/*!
   
    Searches \a strFrom for the first occurrence of \a strToFind based on the 
    locale's collation settings. You can optionally specify the collation level 
    with \a maxLevel. If the parameter is not specified, the default value is 
    used.
    
    \param strFrom The source string.
    \param strToFind The string whose data is to be searched within the source string.
    \param maxLevel (optional) The level of collation. Possible values:
    - \c 0: Only character identities are distinguished (default).
    - \c 1: Character identities and accents are distinguished.
    - \c 2: Character identities, accents, and case are distinguished.    
    - \c 3: All valid Unicode characters are considered different.
    - For details, see Symbian documentation on collation, for example, as used 
    by TDesC16::FindC().
    
    \return The offset from the beginning of \a strFrom where the match first 
    occurs. If the data sequence in \a strToFind cannot be found, returns -1. If 
    the length of \a strToFind is zero, returns zero.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,5}
    
    \attention On the Symbian platform, this class uses a Symbian-specific 
    collation search. On other platforms, the search is not locale-based, and 
    the \a maxLevel parameter is not used.
    
    \sa matchC()
 */
int HbStringUtil::findC( const QString &strFrom,
                         const QString &strToFind,
                         int           maxLevel )
{
#if defined( Q_OS_SYMBIAN )
    TPtrC s1Ptr( strFrom.utf16() );
    TPtrC s2Ptr( strToFind.utf16() );
    
    if ( (maxLevel < 0) || (maxLevel > 3) ) {
        maxLevel = 0;
    }
    return s1Ptr.FindC( s2Ptr.Ptr(),
                        s2Ptr.Length(),
                        maxLevel );
#else
    Q_UNUSED(maxLevel);
    return strFrom.indexOf( strToFind, 0, Qt::CaseSensitive );
#endif 
}

/*!
     
    Searches the folded data in \a strFrom for a match with the folded data in 
    \a strToMatch.
    
    \param strFrom The source string.    
    \param strToMatch The string whose data is to be searched within the source 
    string. The value can contain the wildcard characters "*" and "?", where "*" 
    matches zero or more consecutive occurrences of any character, and "?" 
    matches a single occurrence of any character.
    
    \return The offset from the beginning of \a strFrom where the match first 
    occurs. If the data sequence in \a strToMatch is not found, returns -1.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,4}
    
    \attention On the Symbian platform, this class uses a Symbian-specific 
    folding match. On other platforms, the search is not locale-based.
    
    \sa findF()
 */
int HbStringUtil::matchF( const QString &strFrom,
                          const QString &strToMatch )
{
#if defined( Q_OS_SYMBIAN )
    TPtrC s1Ptr( strFrom.utf16() );
    TPtrC s2Ptr( strToMatch.utf16() );
    return s1Ptr.MatchF( s2Ptr );
#else
    // folding is just case insensitive      
#ifdef QT_NO_REGEXP
    // if no regular expressions defined do standard FindF
    return strFrom.indexOf( strToMatch, 0, Qt::CaseInsensitive );
#else    
    QRegExp locStrToMatch( strToMatch, Qt::CaseInsensitive, QRegExp::Wildcard );
    return strFrom.indexOf( locStrToMatch, 0 );
#endif
    
#endif      
}

/*!

    Searches the folded data in \a strFrom for the first occurrence of the 
    folded data in \a strToFind.
    
    \param strFrom The source string.
    \param strToFind The string whose data is to be searched within the source string.
    
    \return The offset from the beginning of \a strFrom where the match first 
    occurs. If the data sequence in \a strToFind cannot be found, returns -1. If 
    the length of \a strToFind is zero, returns zero.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,6}
    
    \attention On the Symbian platform, this class uses a Symbian-specific 
    folding search. On other platforms, the search is not locale-based, and the 
    \a maxLevel parameter is not used.
    
    \sa matchF()
 */
int HbStringUtil::findF( const QString &strFrom,
                         const QString &strToFind )
{
#if defined( Q_OS_SYMBIAN )
    TPtrC s1Ptr( strFrom.utf16() );
    TPtrC s2Ptr( strToFind.utf16() );
    return s1Ptr.FindF( s2Ptr );
#else
    // folding is just case insensitive    
    return strFrom.indexOf( strToFind, 0, Qt::CaseInsensitive );   
#endif 
}

/*!

    Compares the folded data in \a string1 with the folded data in \a string2.
    
    \param string1 The source string.
    \param string2 The string whose data is to be compared with the source string.
    
    \return Positive if \a string1 is greater (that is, comes after \a string2 
    when the strings are ordered), negative if \a string2 is greater, and zero 
    if the content of the strings matches.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,2}
    
    \attention On the Symbian platform, this class uses Symbian-specific folding 
    comparison. On other platforms, the comparison is not locale-based.
    
    \sa compareC()
 */
int HbStringUtil::compareF( const QString &string1,
                            const QString &string2 )
{
#if defined( Q_OS_SYMBIAN )
    TPtrC s1Ptr( string1.utf16() );
    TPtrC s2Ptr( string2.utf16() );
    return s1Ptr.CompareF( s2Ptr );
#else
    // folding is just case insensitive     
    return string1.compare( string2, Qt::CaseInsensitive );   
#endif 
}

/*!
    
    Returns the starting digit range of the native digit.
    
    \param ch The native digit.
    
 */
static QChar nativeDigitBase(QChar ch)
{
    DigitType d[] = { WesternDigit, ArabicIndicDigit, EasternArabicIndicDigit, DevanagariDigit, ThaiDigit };
    int i = 0;
    int num = sizeof(d)/sizeof(d[0]);
    while(i<num) {
        if (ch>QChar(d[i]) && ch<QChar(d[i]+10)) { return d[i]; }
        i++;
    }
    return ch;
}

/*!
    
    Converts digits to the native digits based on the current UI language.
    
    \param str The digits to be converted.
    
    For example:
    
    \code
    QString date = "07.09.2010";
    QString result = HbStringUtil::convertDigits(date);
    \endcode
    
    \sa convertDigitsTo(), QLocale::toString()
 */
QString HbStringUtil::convertDigits( const QString str ) 
{
    DigitType digitType = WesternDigit;
#if defined( Q_OS_SYMBIAN )
    TExtendedLocale extLocale;
    extLocale.LoadSystemSettings();
    TDigitType type = extLocale.GetLocale()->DigitType();
    switch (type)
        {
        case EDigitTypeArabicIndic:
            digitType = ArabicIndicDigit;
            break;
        case EDigitTypeEasternArabicIndic:
            digitType = EasternArabicIndicDigit;
            break;
        default:
            break;
        };
#else
    HbExtendedLocale locale = HbExtendedLocale::system();
    QChar zero = locale.zeroDigit();
    if (zero == 0x660) {
        digitType = ArabicIndicDigit;
    }
    if (zero == 0x6F0) {
        digitType = EasternArabicIndicDigit;
    }
#endif
    QString converted = HbStringUtil::convertDigitsTo(str, digitType);
    return converted;
}

/*!

    Returns digits converted into the specified digit type. If you need to 
    process an integer, use QLocale::toString() to first convert it to a string.
    
    \param str The digits to be converted.
    \param digitType The digit type that the given digits are to be converted to. Possible values:
    - \c WesternDigit - Latin digits
    - \c ArabicIndicDigit - Arabic-Indic digits
    - \c EasternArabicIndicDigit - Eastern Arabic-Indic digits
    - \c DevanagariDigit - Devanagari digits
    
    \sa convertDigits(), QLocale::toString()
 */
QString HbStringUtil::convertDigitsTo( const QString str, const DigitType digitType ) 
{
    QString convDigit;
    int length = str.length();
    for(int i=0; i<length; i++) 
       {
       ushort digit = str[i].unicode();
       ushort digitBase = nativeDigitBase(str[i]).unicode();
       ushort convertedDigit = 0;
       switch (digitBase) 
           {
           case WesternDigit:
           case ArabicIndicDigit:
           case EasternArabicIndicDigit:
           case DevanagariDigit:
           case ThaiDigit:
               convertedDigit += digitType + digit - digitBase; 
               convDigit[i] = QChar(convertedDigit);
               break;
           default:
               convDigit[i] = QChar(digit);
               break;
           };
       }
    return convDigit;
}

/*!
    
    Sorts QString objects into alphabetical order based on the locale's 
    collation settings. This overwrites the original content of \a strList.
    
    \param strList The list of QStrings that need to be sorted.
    
    Example:
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,7}
    
    \attention Locale-specific collation settings are used, and the return value 
    may vary on different platforms.
    
 */
void HbStringUtil::sort( QStringList &strList )
{
    if ( strList.size() > 1 ) {
        qSort(strList.begin(), strList.end(), hbStringUtil_SortHelper);
    }
}

bool hbStringUtil_SortHelper( const QString &s1, const QString &s2 )
{
    if ( HbStringUtil::compareC(s1, s2) < 0 ) {
        // s1 is before s2
        return true;
    } else {
        // s1 is after s2 (or they are equal) 
        return false;
    }           
}
