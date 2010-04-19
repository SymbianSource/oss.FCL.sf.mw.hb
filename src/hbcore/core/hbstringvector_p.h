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

#ifndef HBSTRINGVECTOR_P_H
#define HBSTRINGVECTOR_P_H

#include <QStringList>

#include "hbstring_p.h"
#include "hbvector_p.h"

class HB_CORE_PRIVATE_EXPORT HbStringVector : public HbVector<HbString>
{
public:
    HbStringVector( HbMemoryManager::MemoryType memoryType ): HbVector<HbString>( memoryType )
    {
    }

    bool contains( const HbString & str, Qt::CaseSensitivity cs ) const
    {
        HbVector<HbString>::const_iterator iter = constBegin();

        while ( iter != constEnd() ) {
            if ( QString(*iter).compare(str, cs) == 0 ) {
                return true;
            }
        }
        return false;
    }
    
/*
 * operator != taking HbStringVector and QStringList
*/
// To Do: Move this to cpp file once it is created
    bool operator != ( const QStringList &b ) const
    {
        int index = 0;
        if ( this->size() == b.size() ) {
            while ( index<this->size() ) {
                if ( this->operator[](index) != b[index] ) {
                    break;
                }
                ++index;
            }
        }
        else{
            return true;
        }
        return ( index != this->size() );
    }

};

#endif // HBSTRINGVECTOR_P_H

