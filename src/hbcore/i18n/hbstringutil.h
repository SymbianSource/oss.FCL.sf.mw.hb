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

#ifndef HBSTRINGUTIL_H
#define HBSTRINGUTIL_H

#include <QString>
#include <QStringList>
#include <hbi18ndef.h>

class HB_CORE_EXPORT HbStringUtil
{
public:
    enum Option {        
        Default = 0,         
        IgnoreNone = 1,                      
        SwapCase = 2,        
        AccentsBackwards = 4,
        SwapKana = 8,        
        FoldCase = 16,       
        MatchingTable = 32,  
        IgnoreCombining = 64 
    };
    Q_DECLARE_FLAGS(Options, Option)
    
    static int collationMethods(); // Will be deprecated 
    static int matchC( const QString &strFrom, const QString &strToMatch, 
                       int maxLevel = 0, Options flags = Default, 
                       int wildChar = '?', int wildSequenceChar = '*', int escapeChar = '\\' );
    static int compareC( const QString &string1, const QString &string2, 
                         int maxLevel = 3, Options flags = Default );
    static int findC( const QString &strFrom, 
                      const QString &strToFind, 
                      int maxLevel = 0 );
    static int matchF( const QString &strFrom, const QString &strToMatch );
    static int compareF( const QString &string1, const QString &string2 );    
    static int findF( const QString &strFrom, const QString &strToFind );
    
    static uint collate( const uint chr ); // Will be deprecated
    
    static QString convertDigits( const QString str );
    static QString convertDigitsTo( const QString str, const DigitType digitType ); 
    static void sort( QStringList &strList );   
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbStringUtil::Options)

#endif
