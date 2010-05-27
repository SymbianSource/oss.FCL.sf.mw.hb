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

#if !defined(HB_BOOTSTRAPPED) || defined(HB_BIN_CSS)
#include "hbmemorymanager_p.h"
#include "hbthemecommon_p.h"
#ifdef HB_THEME_SERVER_MEMORY_REPORT
#include <QMap>
#include <QPair>
#endif
#endif // HB_BOOTSTRAPPED

// 13 MB cache size
#define CACHE_SIZE 1024*1024*13

#if !defined(HB_BOOTSTRAPPED) || defined(HB_BIN_CSS)

class QSharedMemory;
class HbSharedMemoryAllocator;
class HbSharedCache;

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
    int size();
    HbSharedCache *createSharedCache(
        const char *offsetMapData, int size, int offsetItemCount, int sharedCacheOffset = -1);
    HbSharedCache *cache();

    int freeSharedMemory();
    int allocatedSharedMemory();

#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void createReport();
#endif

protected:
    HbSharedMemoryManager();
    ~HbSharedMemoryManager();

private:
    bool initialize();
    int loadMemoryFile(const QString &filePath);
    template<typename T>
    inline T *address(int offset)
    {
        return reinterpret_cast<T *>(static_cast<char *>(base()) + offset);
    }

protected:
    bool writable;
    HbSharedMemoryAllocator *mainAllocator;
    HbSharedMemoryAllocator *subAllocator;
	QSharedMemory *chunk;

private:
    static HbSharedMemoryManager *memManager;

#ifdef HB_THEME_SERVER_MEMORY_REPORT
    int totalAllocated;
    int OOMCount; // how many out of memory exceptions have occurred
    bool OOMReportCreated; // write report in first OOM
    int ordinal; // used in report filename
    QMap<quint32, QPair<quint32, quint32> > allocations; // size, <allocated, freed>
#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
    QVector<QPair<quint32, quint32> > fullAllocationHistory;
#endif
#endif
};

#endif // HB_BOOTSTRAPPED

#endif // HBSHAREDMEMORYMANAGER_P_H
