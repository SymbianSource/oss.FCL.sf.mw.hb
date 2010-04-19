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
    @beta
    @hbcore
    \class HbStringUtil
    \brief The HbStringUtil class can be used to execute operations on strings,
    such as comparisons and finding data sequences.

    \ingroup i18n

    \warning This class is only useful in Symbian platforms since it uses Symbian
    methods in order to implement different functionalities.

    \sa HbStringUtil
*/

/*!
    \enum HbStringUtil::Option

    This enum describes the way collation is done for matchC, compareC
    Pass one of these values to setReadChannel() to set the
    current read channel of QProcess.

    \value Default          Use the default System flags.
     
    \value IgnoreNone       Don't ignore anything.
    
    \value SwapCase         Reverse case ordering.
    
    \value AccentsBackwards Compare secondary keys which represent accents in reverse order.
    
    \value SwapKana         Reverse order for katakana/hiragana.
    
    \value FoldCase         Fold to lower case, file comparisons.
    
    \value MatchingTable    Table used for matching.
    
    \value IgnoreCombining  Ignore check for adjacent combining characters. 
    
    \sa compareC, matchC
*/

/*!
    \deprecated HbStringUtil::collationMethods()
        is deprecated.
    
    Returns the number of collation methods supported.
    On Symbian platform uses Mem::CollationMethods,
    elsewhere return 0.
     
    \return The number of collation methods available.
 */
int HbStringUtil::collationMethods()
{
    qWarning("HbStringUtil::collationMethods is DEPRECATED. Do not use this function.");
#if defined( Q_OS_SYMBIAN )
    return Mem::CollationMethods();
#else
    return 0;
#endif 
}

/*!
    Searches source string's collated data for a
    match with collated data supplied in pattern string
    
    \param strFrom Source string.
    \param strToMatch Pattern string.
    \param maxLevel Determines the tightness of the collation.
    Level 0 - Character identity; 
    Level 1 - Character identity and accents; 
    Level 2 - Character identity, accents and case; 
    Level 3 - Character identity, accents, case and Unicode value; 
    \param flags The flags that will be used. Default value is Default.
    \param wildChar Wild card character.
    \param wildSequenceChar Wild card sequence character.
    \param escapeChar The escape character, for example,  '?', '*'.
    \return If a match is found the offset within source string's
    data where the match first occurs. -1 if match is not found.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,3}
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
    // works with standard wildcards is not correct
    QRegExp locStrToMatch( strToMatch, Qt::CaseSensitive, QRegExp::Wildcard );    
    return strFrom.indexOf( locStrToMatch, 0 );
#endif    
    
#endif
}

/*!
    Compares source string's data with the other string's
    data using the specified collation method.
    
    \param string1 Source string.
    \param string2 String whose data is to be compared with the source string.
    \param maxLevel Maximum level to use for comparing.
    Level 0 - Character identity; 
    Level 1 - Character identity and accents; 
    Level 2 - Character identity, accents and case; 
    Level 3 - Character identity, accents, case and Unicode value; 
    \param flags The flags that will be used. Default value is Default.
    \return Positive if source string is greater, negative if it is less and 
    zero	if the content of both strings match.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,1}
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
    Searches for the first occurence of the specified collated 
    data sequence in the aStrFrom to the specified maximum
    collation level.
    
    \param strFrom Source string.
    \param strToFind String whose data is to be compared with the source string.
    \param maxLevel The maximum collation level.
    Level 0 - Character identity; 
    Level 1 - Character identity and accents; 
    Level 2 - Character identity, accents and case; 
    Level 3 - Character identity, accents, case and Unicode value; 
    \return Offset of the data sequence from the beginning of the
    aStrFrom. -1 if the data sequence cannot be found.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,5}
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
    Searches source string's folded data for a
    match with folded data supplied in pattern string
    
    \param strFrom Source string.
    \param strToMatch Pattern string.
    \return If a match is found the offset within source string's
    data where the match first occurs. -1 if match is not found.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,4}
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
    Searches for the first occurence of the specified folded 
    data sequence in the strFrom.
    
    \param strFrom Source string.
    \param strToFind String whose data is to be compared with the source string.
    \return Offset of the data sequence from the beginning of the
    strFrom. -1 if the data sequence cannot be found. Zero,
    if the length of search data sequence is zero.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,6}
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
    Compares source string's folded data with the other string's
    folded data.
    
    \param string1 Source string.
    \param string2 String whose data is to be compared with the source string.
    \return Positive if source string is greater, negative if it is less and 
    zero if the content of both strings match.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,2}
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
    \deprecated HbStringUtil::collate( const uint chr )
        is deprecated.
    
    Converts the character to its collated form.
    Collating is the process of removing differences between characters 
    that are considered unimportant for the purposes of ordering characters.
    The result of the conversion depends on the locale
    
    \param chr The charcter to be collated.
    \return The converted character.
 */
uint HbStringUtil::collate( const uint chr )
{
    qWarning("HbStringUtil::collate is DEPRECATED. Do not use this function.");
#if defined( Q_OS_SYMBIAN )
    return User::Collate( chr );
#else
    return chr;
#endif 
}

/*!
    Returns the starting digit range of the native digit
    \param ch native digit
    \return starting digit range
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
    Converts digits to native digits based on current UI language.
    \param str digits to be converted.
 */
QString HbStringUtil::convertDigits( const QString str ) 
{
	HbExtendedLocale locale = HbExtendedLocale::system();
	DigitType digitType = WesternDigit;
	if (locale.language() == HbExtendedLocale::Arabic) {
		digitType = ArabicIndicDigit;
	}
	if (locale.language() == HbExtendedLocale::Persian) {
		digitType = EasternArabicIndicDigit;
	}
	if (locale.language() == HbExtendedLocale::Urdu) {
		digitType = EasternArabicIndicDigit;
	}
	QString converted = HbStringUtil::convertDigitsTo(str, digitType);
	return converted;
}

/*!
    Converts the digit from Latin to native or native to latin or native to native
    \param str digits to be converted.
    \param digitType type of the digit to be converted to
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
    Sorts QStrings into alphabetically order (overwrites the strList's original content)
    
    \param strList List of QStrings which need to be sorted.
    
    Example
    \snippet{unittest_hbstringutil/unittest_hbstringutil.cpp,7}
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
