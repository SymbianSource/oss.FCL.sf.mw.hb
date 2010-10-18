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


#include <QPoint>
#include <hbglobal.h>
#include "hbparameterlengthlimiter.h"
#include "hbparameterlengthlimiter_p.h"

/*!
    @stable
    @hbcore
    \class HbParameterLengthLimiter
    \brief The HbParameterLengthLimiter class provides support for localizing text strings, and for limiting the length of runtime values inserted into the strings.

    The HbParameterLengthLimiter class provides the same support for string 
    localization based on unique text IDs as the global hbTrId() function, and 
    the same support for inserting values based on the given arguments as 
    QString::arg(). In addition, %HbParameterLengthLimiter supports limiting the 
    length of values that are inserted into the translations using arguments. 
    You can use up to nine arguments to insert values, as supported by QString.
    
    In most cases, it is simpler to use hbTrId() for localization (see hbTrId() 
    documentation and examples). However, the number of characters in a text 
    string can vary widely from language to language. Sometimes when space is 
    limited, you may want to limit the number of characters that are inserted. 
    In this case, use the HbParameterLengthLimiter class instead of %hbTrId().
    
    Even if the inserted value does not require limiting at the time of coding, 
    there is one advantage in using HbParameterLengthLimiter right away: if you 
    later need to limit the value length for some languages, no code changes are 
    required. You only need to modify the <tt>.ts</tt> translation files for 
    those languages. However, if you already know that all translations will fit 
    without any limitations (for example, if you are using scrolling text with 
    HbMarqueeItem), use hbTrId().
    
    The text strings in .ts localization files can have placeholders, where you 
    insert values at runtime. HbParameterLengthLimiter supports the same 
    placeholder formats as hbTrId() (\c \%x, \c \%Lx and \c \%Ln), and the 
    following additional placeholder:
    
    <ul>
    
    <li><tt>%[y]x</tt> for inserting strings, where \c x is a number (1-9) that 
    indicates the order of placeholders in the source text string. This 
    corresponds to your argument inserting order when you use 
    HbParameterLengthLimiter. The order of placeholders may vary in different 
    translations, and the numbering ensures that values are inserted into the 
    correct placeholders. <tt>[y]</tt> is a number that indicates the maximum 
    length (in characters) for the inserted value.</li>
    
    </ul>
    
    For example, the text string \c "Downloading file %[10]1 from URL %2" has 
    two placeholders to insert two strings, a file name and a URL. The [] 
    notation in the first placeholder \c %[10]1 limits the length of the file 
    name to 10 characters.
    
    HbParameterLengthLimiter provides all the same %arg() variants as QString, 
    which means you can use it in place of QString::arg() when needed. The 
    following examples describe the most common use cases that you would use 
    %HbParameterLengthLimiter for. The other functions available can be used for 
    more complex cases, such as processing the localized text strings.
    
    \section _usecases_hbparameterlengthlimiter Using the HbParameterLengthLimiter class
    
    \subsection _uc_001_hbparameterlengthlimiter Limiting the length of values inserted using an argument
      
      This example shows how you get the translation based on a given text ID, 
      replace the two placeholders with values using runtime arguments, and 
      limit the length of both of the inserted values.
      
      \code
      // "Download file %1 from URL %2?"
      // fileName = "test_application_data_file.txt"
      // urlName = "www.something.org"
      HbParameterLengthLimiter myTranslation("txt_browser_download_data");
      label->setPlainText(myTranslation.arg(fileName, urlName));
      \endcode
      
      The translation is defined in the <tt>.ts</tt> file in the following 
      format:
      
      \code
      <message id="txt_browser_download_data">
        <source>"Download file %1 from URL %2?</source>
        <translation variants="no">Download file %[10]1 from URL %[12]2?</translation>
      </message>
      \endcode
      
      This limits the length of the inserted values to 10 and 12 characters, 
      producing a text string such as \c "Download file test_appli... from URL 
      www.somethin...?".
    
      \subsection _uc_002_hbparameterlengthlimiter Inserting values using runtime arguments
      
      This example shows how you simply get a translation and replace the 
      placeholder with a runtime value. You can achieve the same result by using 
      hbTrId().
      
      \code
      // "Enter %L1-digit passcode"
      // int number = 4
      label->setPlainText(HbParameterLengthLimiter("txt_give_numeric_passcode").arg(number));
      \endcode
      
      This example also uses the 'L' notation in the placeholder in the text 
      string, which formats the number in the active locale's display format.
      
      \sa hbTrId(), QString::arg(), HbDirectoryNameLocalizer, HbTranslator


*/

/*!
    Copy constructor.
        
    \param a The HbParameterLengthLimiter object to copy.
*/
HbParameterLengthLimiter::HbParameterLengthLimiter( const HbParameterLengthLimiter& a )
{
    p = new HbParameterLengthLimiterPrivate();
    p->str = a.p->str;
}

/*!
    Constructor for processing a string that has already been localized.
    
    \param a A localized string containing the placeholders for inserting values.
*/
HbParameterLengthLimiter::HbParameterLengthLimiter( QString a )
{
    p = new HbParameterLengthLimiterPrivate();
    p->str = a;
}

/*!

    Constructs an HbParameterLengthLimiter using a text ID. The functionality is 
    similar to what hbTrId() provides, except for the additional support for 
    limiting the length of the inserted values.
    
    \param a The text ID that identifies the localized string in the 
    <tt>.ts</tt> file.
    
*/
HbParameterLengthLimiter::HbParameterLengthLimiter( const char* a )
{
    p = new HbParameterLengthLimiterPrivate();
    p->str = hbTrId(a);
}

/*!

    Constructs an HbParameterLengthLimiter using a text ID, and a numeric 
    parameter for handling singular and plural forms correctly. The 
    functionality is the same as provided by the hbTrId() function, except for 
    the additional support for limiting the length of the inserted values.
    
    \param a The text ID that identifies the localized string in the 
    <tt>.ts</tt> file.    
    \param n A numeric parameter that indicates quantity, used to determine 
    which singular or plural translation to use from the <tt>.ts</tt> file.
    
*/
HbParameterLengthLimiter::HbParameterLengthLimiter( const char* a, int n )
{
    p = new HbParameterLengthLimiterPrivate();
    p->str = hbTrId(a, n);
}

/*!
    Constructs an HbParameterLengthLimiter with a blank string.
*/
HbParameterLengthLimiter::HbParameterLengthLimiter()
{
    p = new HbParameterLengthLimiterPrivate();
    p->str = "";
}

/*!
    Destructor.
*/
HbParameterLengthLimiter::~HbParameterLengthLimiter()
{
    delete p;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(qlonglong, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.
    \param fieldwidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg(qlonglong a,
                                      int fieldwidth,
                                      int base,
                                      const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldwidth,base,tmpChar);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(qulonglong, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldwidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( qulonglong a,
                                       int fieldwidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldwidth,base,tmpChar);

    return *this;
 }

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(long, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldwidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( long a,
                                       int fieldwidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldwidth,base,tmpChar);

    return *this;
 }

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(ulong, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldwidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( ulong a,
                                       int fieldwidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldwidth,base,tmpChar);
    return *this;
 }

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(int, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( int a,
                                       int fieldWidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldWidth,base,tmpChar);

    return *this;
 }

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(uint, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( uint a,
                                       int fieldWidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldWidth,base,tmpChar);
    return *this;
 }

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(short, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( short a,
                                       int fieldWidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldWidth,base,tmpChar);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(ushort, int, int, const QChar &) const.
    
    \param a The number that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param base The number base.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( ushort a,
                                       int fieldWidth,
                                       int base,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a,fieldWidth,base,tmpChar);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(double, int, char, int, const QChar &) const.
    
    \param a Value formatted according to the specified format and precision.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param fmt The format to be used.
    \param prec The precision to be used.
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( double a,
                                       int fieldWidth,
                                       char fmt,
                                       int prec,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a, fieldWidth, fmt, prec, tmpChar);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(char, int, const QChar &) const.
    
    \param a The character that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( char a,
                                       int fieldWidth,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a, fieldWidth, tmpChar);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments.
    
    This function corresponds to QString::arg(QChar, int, const QChar &) const.
    
    \param a The character that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( QChar a,
                                       int fieldWidth,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;
    p->str = p->str.arg(a, fieldWidth, tmpChar);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with runtime values using the 
    given parameters and arguments. Limits the length of the inserted value, if 
    this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, int, const QChar &) const.
    
    \param a The string that is to be inserted to the output string.    
    \param fieldWidth The minimum amount of space that \a a is padded to and 
    filled with the character \a fillChar.    
    \param fillChar The fill character.
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a,
                                       int fieldWidth,
                                       const QChar &fillChar)
{
    QChar tmpChar = fillChar;

    QString result = "";
    int i = 0;

    int signPosition;

    QPoint position(10,0);
    while ( i < p->str.length() ) {
        if ( p->str.at(i) == '%' ) {
            signPosition = i;
            i++;
            if ( i >= p->str.length() ) {
                break;
            }
            if ( p->str.at(i).isDigit() ) {
                //normal QString
                QString number = p->str.at( i );
                if( number.toInt() < position.x() ) {
                    position.setX( number.toInt() );
                    position.setY( signPosition );
                }
            } else if ( p->str.at(i) == '[' ) {
                //limited QString
                i++;
                if( i >= p->str.length() ) {
                    break;
                }

                while( p->str.at(i) != ']' ) {
                    i++;
                    if( i >= p->str.length() ) {
                        break;
                    }
                }

                i++;
                if( i >= p->str.length() ) {
                    break;
                }

                if( p->str.at(i).isDigit() ) {
                    QString number = p->str.at(i);
                    if( number.toInt() < position.x() ) {
                        position.setX( number.toInt() );
                        position.setY( signPosition );
                    }
                }
                else{
                    --i;
                }
            }
        }
        i++;
    }

    i = position.y();
    if ( p->str.at(i) == '%' ) {
        result.append( p->str.at(i) );
        i++;
        if ( p->str.at(i).isDigit() ) {
            //normal QString

            p->str = p->str.arg( a, fieldWidth, tmpChar );
        } else if ( p->str.at(i) == '[' ) {
            //limited QString

            i++;
            if( i >= p->str.length() ) {
                return *this;
            }
            QString limiter = "";
            bool limitedFound = false;
            while (p->str.at(i) != ']'){
                if ( p->str.at(i).isNumber() ) {
                    limitedFound = true;
                    limiter.append( p->str.at(i) );
                    i++; 
                    if ( i >= p->str.length() ) {
                        return *this;
                    }
                }
            }
            i++;
            if ( i >= p->str.length() ) {
                return *this;
            }
            if( limitedFound && p->str.at(i).isNumber() ) {
                i++;
                result = p->str.left( position.y() );
                int length = limiter.toInt();
                QString limitedA = a;

                int o = 0;
                int u = 0;
                if( length < a.length() ) {
                    while ( o <= a.length() ) {
                        // If character is Thai vowel or tone mark, skip it as it does not have length
                        // U+E34 = 3636, U+E3B = 3643, U+E47 = 3655, U+E4D = 3661
                        // or zero wide space 200B = 8203
                        if ( (a.at(o).unicode() < 3636 || a.at(o).unicode() > 3643)
                             && (a.at(o).unicode() < 3655 || a.at(o).unicode() > 3661)
                             && a.at(o).unicode() != 8203 ) {
                            u++;
                        }
                        if ( u == length+1 ) {
                            limitedA = a.left( o) ;
                            // Add ellipsis as a mark that QString is truncated
                            limitedA.append( QChar(0x2026) );
                            break;
                        }
                        o++;
                    }
                }
                result.append( "%1" );
                int cutter = p->str.length() - i ;
                QString tmp = p->str.right(cutter);
                result.append( tmp );
                
                p->str = result.arg( limitedA, fieldWidth, tmpChar );
            }

        }
    }
    return *this;
}

/*!

    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1 and \a a2). Limits the length of the inserted value, if 
    this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2 )
{
    this->arg(a1).arg(a2);
    return *this;
}

/*!

    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2,
                                       const QString &a3)
{
    this->arg(a1, a2).arg(a3);
    return *this;
}

/*!

    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2,
                                       const QString &a3,
                                       const QString &a4 )
{
    this->arg(a1, a2, a3).arg(a4);
    return *this;
}

/*!

    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2,
                                       const QString &a3,
                                       const QString &a4,
                                       const QString &a5 )
{
    this->arg(a1, a2, a3, a4 ).arg(a5);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2,
                                       const QString &a3,
                                       const QString &a4,
                                       const QString &a5,
                                       const QString &a6 )
{
    this->arg(a1, a2, a3, a4, a5).arg(a6);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2,
                                       const QString &a3,
                                       const QString &a4,
                                       const QString &a5,
                                       const QString &a6,
                                       const QString &a7 )
{
    this->arg(a1, a2, a3, a4, a5, a6).arg(a7);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2,
                                       const QString &a3,
                                       const QString &a4,
                                       const QString &a5,
                                       const QString &a6,
                                       const QString &a7,
                                       const QString &a8 )
{
    this->arg(a1, a2, a3, a4, a5, a6, a7).arg(a8);
    return *this;
}

/*!
    
    Replaces the placeholders in the translation with the given strings using 
    the arguments (\a a1, \a a2, and so on). Limits the length of the inserted 
    value, if this is specified in the placeholder using the [] notation.
    
    This function corresponds to QString::arg(const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString &) const.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::arg( const QString &a1,
                                       const QString &a2, 
                                       const QString &a3,
                                       const QString &a4, 
                                       const QString &a5, 
                                       const QString &a6,
                                       const QString &a7, 
                                       const QString &a8, 
                                       const QString &a9 )
{
    this->arg(a1, a2, a3, a4, a5, a6, a7, a8).arg(a9);
    return *this;
}

/*!

    Sets the localized string of the HbParameterLengthLimiter object to the 
    value of the \a a string.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::operator=( const QString &a )
{
    p->str = a;
    return *this;
}

/*!

    Sets the localized string of the target HbParameterLengthLimiter object to 
    the string contained in the \a a object.
    
*/
HbParameterLengthLimiter& HbParameterLengthLimiter::operator=( const HbParameterLengthLimiter &a )
{
    p->str = a.p->str;
    return *this;
}

/*!

    Implicit conversion function that allows you to pass an 
    HbParameterLengthLimiter object in a context where a QString object is 
    expected. For example, you can use an HbParameterLengthLimiter object when 
    setting the localized text in a widget, rather than first getting the string 
    out of HbParameterLengthLimiter and then passing it to the widget.

*/
HbParameterLengthLimiter::operator QString() const
{
    return p->str;
}



