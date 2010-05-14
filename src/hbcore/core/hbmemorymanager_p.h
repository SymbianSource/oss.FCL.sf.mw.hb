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

#ifndef HBMEMORYMANAGER_P_H
#define HBMEMORYMANAGER_P_H

// To enable-disable css parser related traces
#undef CSS_PARSER_TRACES

#include "hbglobal.h"
#include <hbthemeperf_p.h>
#include <new>

//Temporary for std::bad_alloc, till the macro HB_CHECK_PTR is replaced by q_check_ptr
//in 4.6.
#define HB_CHECK_PTR(ptr) if(!ptr) throw std::bad_alloc();


class HB_CORE_PRIVATE_EXPORT HbMemoryManager
{
public:
    enum MemoryType{
        InvalidMemory = -1,
        SharedMemory = 0,
        HeapMemory
    };
    virtual int alloc( int size ) = 0;
    virtual int realloc( int oldOffset, int newSize ) = 0;
    virtual void free( int offset ) = 0;
    virtual void * base() = 0;
    virtual bool isWritable() = 0;
    virtual ~HbMemoryManager() { }
#ifdef HB_PERF_MEM
    virtual unsigned int memoryConsumed() 
    {
        return 0;
    }
#endif
    static HbMemoryManager * instance( MemoryType type );
    static void releaseInstance( MemoryType type );
};

#endif // HBMEMORYMANAGER_P_H

