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

#include "hbheapmemorymanager_p.h"
#include <stdlib.h>

/*
* c'tor
*/
HbHeapMemoryManager::HbHeapMemoryManager() :
    HbMemoryManager(true)
{
}

/*
* To allocate memory
* 
* This function can throw an exception in case of out of memory condition
*/
qptrdiff HbHeapMemoryManager::alloc(int size)
{
    qptrdiff offset = -1;
    if( size > 0) {
        void* ptr = ::malloc(size);
        HB_CHECK_PTR(ptr)
        offset = (qptrdiff)ptr;        
    } 
   return offset;
}

/*
* To reallocate memory
* 
* This function can throw an exception in case of out of memory condition
*
*/
qptrdiff HbHeapMemoryManager::realloc(qptrdiff oldOffset, int newSize)
{
    qptrdiff offset = -1;
    if ( newSize > 0 ) {
        if (oldOffset == -1) {
            offset = (qptrdiff)::malloc(newSize);
        } else {
            offset = (qptrdiff)::realloc((void*)oldOffset, newSize);
        }
        HB_CHECK_PTR(offset) 
    }
    return offset;
}

/*
* To free memory
*/
void HbHeapMemoryManager::free(qptrdiff offset)
{
    if (offset != -1) {
        ::free((void*)offset);
    }
}
