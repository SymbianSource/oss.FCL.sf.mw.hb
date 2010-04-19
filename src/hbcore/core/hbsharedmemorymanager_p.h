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

#ifndef HBSHAREDMEMORYMANAGER_P_H
#define HBSHAREDMEMORYMANAGER_P_H

#include "hbmemorymanager_p.h"

#include <new>

// 13 MB cache size
#define CACHE_SIZE 1024*1024*13

class QSharedMemory;
class HbSharedMemoryAllocator;

class HB_CORE_PRIVATE_EXPORT HbSharedMemoryManager
    : public HbMemoryManager
{
public:
    int alloc( int size );
    int realloc( int oldOffset,int newSize );
    void free( int offset );
    void *base();
    bool isWritable()
    {
        return writable;
    }
    static HbMemoryManager *instance();
    static void releaseInstance();
#ifdef HB_PERF_MEM
    unsigned int memoryConsumed();
#endif

protected:
    HbSharedMemoryManager();
    ~HbSharedMemoryManager();

private:
    bool initialize();

protected:
    bool writable;
    HbSharedMemoryAllocator *mainAllocator;
    HbSharedMemoryAllocator *subAllocator;
	QSharedMemory *chunk;

private:
    static HbSharedMemoryManager *memManager;
};

#endif // HBSHAREDMEMORYMANAGER_P_H
