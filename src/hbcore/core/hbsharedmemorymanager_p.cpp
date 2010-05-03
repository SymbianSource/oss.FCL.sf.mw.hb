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

#include "hbsharedmemorymanager_p.h"
#include <QSharedMemory>
#include <QString>
#include <QList>
#include <QDebug>

#include "hbsharedmemoryallocators_p.h"
#include "hbmemoryutils_p.h"

#define USE_SUBALLOCATOR

HbSharedMemoryManager *HbSharedMemoryManager::memManager = 0;

/* Functions implementation of HbSharedMemoryManager class */

/**
 * HbSharedMemoryManager::initialize
 * 
 * If due to OOM scenario, the initialization of HbSharedMemoryManager fails,
 * it handles the exception and returns false. 
 */
bool HbSharedMemoryManager::initialize()
{
    if (chunk) {
        return true;
    }
    bool success = false;
    chunk = new QSharedMemory(HB_THEME_SHARED_PIXMAP_CHUNK);
    // check if app filename is same as server filename ..
    // ToDo: improve server identification logic.. UID on symbian?
    if ( HbMemoryUtils::getCleanAppName() == THEME_SERVER_NAME ) {
        // This is server, create shared memory chunk
        success = chunk->create( CACHE_SIZE, QSharedMemory::ReadWrite );
        // If sharedMemory already exists.
        // (This can happpen if ThemeServer crashed without releasing QSharedMemory)
        if (!success && QSharedMemory::AlreadyExists == chunk->error()) {
            success = chunk->attach(QSharedMemory::ReadWrite);
        }
        writable = true;
    } else {
        // this is not server so just attach to shared memory chunk in ReadOnly mode
        success = chunk->attach( QSharedMemory::ReadOnly );
        writable = false;
    }
    if ( !success ) {
#ifdef THEME_SERVER_TRACES
        qDebug() << "HbSharedMemoryManager:: Could not initialize shared memory chunk";
#endif //THEME_SERVER_TRACES
        delete chunk; 
        chunk = 0;
    }
    
    if (success && isWritable()) {
        // if we are recovering from theme server crash, shared chunk may
        // already be ready
        HbSharedChunkHeader *chunkHeader = (HbSharedChunkHeader*)(chunk->data());
        if (chunkHeader->identifier == INITIALIZED_CHUNK_IDENTIFIER) {
            // just reconnect allocators to the shared chunk
            mainAllocator->initialize(chunk, chunkHeader->mainAllocatorOffset);
            subAllocator->initialize(chunk, chunkHeader->subAllocatorOffset, mainAllocator);
        } else {
            chunkHeader->mainAllocatorOffset = sizeof(HbSharedChunkHeader);
            mainAllocator->initialize(chunk, chunkHeader->mainAllocatorOffset);
            chunkHeader->subAllocatorOffset = alloc(SPACE_NEEDED_FOR_MULTISEGMENT_ALLOCATOR);
            subAllocator->initialize(chunk, chunkHeader->subAllocatorOffset, mainAllocator);
            chunkHeader->identifier = INITIALIZED_CHUNK_IDENTIFIER;
        }
		success = true;
	}
	return success;
}

/**
 * HbSharedMemoryManager::alloc
 * 
 * This function throws std::bad_alloc in case of OOM condition, else
 * proper offset in case of a successful memory allocation
 */
int HbSharedMemoryManager::alloc(int size)
{
    if (isWritable() && size > 0) {
        int offset = -1;
#ifdef USE_SUBALLOCATOR
        if (size <= MAXIMUM_ALLOC_SIZE_FOR_SUBALLOCATOR) {
            offset = subAllocator->alloc(size);
        } else {
            offset = mainAllocator->alloc(size);
        }
#else
        offset = mainAllocator->alloc(size);
#endif
        if (offset == -1) {
            // we ran out of memory
            throw std::bad_alloc();
        }
        return offset;
    } else {
        // memory manager attached in read only mode, cannot allocate memory
        throw std::bad_alloc();
    }
}

/**
 * free
 */
void HbSharedMemoryManager::free(int offset)
{
    // don't do anything when freeing NULL (pointer)offset
    if (isWritable() && (offset > 0)) {
        int metaData = *(int*)((unsigned char*)(base())+offset-sizeof(int));
        if (metaData & MAIN_ALLOCATOR_IDENTIFIER) {
            mainAllocator->free(offset);
        } else {
            subAllocator->free(offset);
        }
    }
}

/**
 * realloc
 * 
 * This function can throw if alloc fails
 */
int HbSharedMemoryManager::realloc(int offset, int newSize)
{
    int newOffset = -1;
    if (isWritable()) {
	    newOffset = alloc(newSize);
        int allocatedSize = ALIGN(newSize);
        if (offset > 0) {
            unsigned char *scrPtr = (unsigned char*)(base())+offset;
            int metaData = *(int*)((unsigned char*)(base())+offset-sizeof(int));
            if (metaData & MAIN_ALLOCATOR_IDENTIFIER) {
                int oldSize = mainAllocator->allocatedSize(offset);
                memcpy((unsigned char*)(base())+newOffset, scrPtr, qMin(oldSize, allocatedSize));
                mainAllocator->free(offset);
            } else {
                int oldSize = subAllocator->allocatedSize(offset);
                memcpy((unsigned char*)(base())+newOffset, scrPtr, qMin(oldSize, allocatedSize));
                subAllocator->free(offset);
            }
        }
    } else {
        // tried to free non-writable memory???
        throw std::bad_alloc();
    }

    return newOffset;
}

/**
 * base
 */
void *HbSharedMemoryManager::base()
{
	return chunk->data();
}

/**
 * constructor
 */
HbSharedMemoryManager::HbSharedMemoryManager()
    :writable(true), 
     mainAllocator(new HbSplayTreeAllocator),
     subAllocator(new HbMultiSegmentAllocator),
     chunk(0)
{
}

/**
 * destructor
 */
HbSharedMemoryManager::~HbSharedMemoryManager()
{
    delete subAllocator;
    delete mainAllocator;
    delete chunk;
}

/**
 * to get instance of HbSharedMemoryManager
 */
HbMemoryManager *HbSharedMemoryManager::instance()
{
    if (!memManager) {
        memManager = new HbSharedMemoryManager();
        if (!memManager->initialize()) {
            qWarning( "HbSharedMemoryManager:Could not initialize shared memory" );
            delete memManager;
            memManager = 0;
        }
    }
    return memManager;
}

/**
 * release the HbSharedMemoryManager-instance.
 */
void HbSharedMemoryManager::releaseInstance()
{
    delete memManager;
    memManager = 0;
}

/**
 * gets the free memory reported by main allocator
 */
int HbSharedMemoryManager::freeSharedMemory()
{
    HbSplayTreeAllocator *splayAllocator = static_cast<HbSplayTreeAllocator*>(mainAllocator);
    return splayAllocator->freeBytes();
}

/**
 * gets the allocated memory reported by main allocator
 */
int HbSharedMemoryManager::allocatedSharedMemory()
{
    HbSplayTreeAllocator *splayAllocator = static_cast<HbSplayTreeAllocator*>(mainAllocator);
    return splayAllocator->allocatedBytes();
}
