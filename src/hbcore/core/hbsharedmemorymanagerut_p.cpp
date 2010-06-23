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

#include "hbsharedmemorymanagerut_p.h"
#include <QSharedMemory>
#include <QString>
#include <QDebug>

#include "hbthemecommon_p.h"
#include "hbsharedcache_p.h"

#define HB_THEME_SHARED_AUTOTEST_CHUNK "hbthemesharedautotest"

HbSharedMemoryManagerUt * HbSharedMemoryManagerUt::memManager = 0;

/**
 * initialize
 */
bool HbSharedMemoryManagerUt::initialize()
{
    bool success = false;
    
    if ( !chunk ) {
        chunk = new HbSharedMemoryWrapper(HB_THEME_SHARED_AUTOTEST_CHUNK);
        success = chunk->create( CACHE_SIZE, QSharedMemory::ReadWrite );
        // If SharedMemory Already Exists.
        // (This can happpen if ThemeServer crashed without releasing QSharedMemory)
        if ( !success && QSharedMemory::AlreadyExists == chunk->error() ) {
            success = chunk->attach( QSharedMemory::ReadWrite );
        }
        writable = true;
        if ( !success ) {
#ifdef THEME_SERVER_TRACES
            qDebug() << "HbSharedMemoryManager:: Could not initialize shared memory chunk";
#endif //THEME_SERVER_TRACES
            delete chunk; 
            chunk = 0;
        }
    }

    if (success) {
        // if we are recovering from theme server crash, shared chunk may
        // already be ready
        bool enableRecovery = false;
        HbSharedChunkHeader *chunkHeader = (HbSharedChunkHeader*)(chunk->data());
        if (enableRecovery && chunkHeader->identifier == INITIALIZED_CHUNK_IDENTIFIER) {
            // just reconnect allocators to the shared chunk
            mainAllocator->initialize(chunk, chunkHeader->mainAllocatorOffset);
            subAllocator->initialize(chunk, chunkHeader->subAllocatorOffset, mainAllocator);
        } else {
            chunkHeader->mainAllocatorOffset = sizeof(HbSharedChunkHeader);
            // Clear also allocator identifier so that they will not try to re-connect
            int *mainAllocatorIdentifier = reinterpret_cast<int *>(static_cast<char *>(base()) + chunkHeader->mainAllocatorOffset);
            *mainAllocatorIdentifier = 0;
            mainAllocator->initialize(chunk, chunkHeader->mainAllocatorOffset);
            chunkHeader->subAllocatorOffset = alloc(SPACE_NEEDED_FOR_MULTISEGMENT_ALLOCATOR);
            int *subAllocatorIdentifier = reinterpret_cast<int *>(static_cast<char *>(base()) + chunkHeader->subAllocatorOffset);
            *subAllocatorIdentifier = 0;
            subAllocator->initialize(chunk, chunkHeader->subAllocatorOffset, mainAllocator);
            chunkHeader->identifier = INITIALIZED_CHUNK_IDENTIFIER;

            // Create empty shared cache for unit test purposes
            HbSharedCache *cachePtr = createSharedCache(0, 0, 0);
            if (cachePtr) {
                const QString &appName = HbMemoryUtils::getCleanAppName();
                if (appName == THEME_SERVER_NAME) {
                    cachePtr->initServer();
                } else {
                    cachePtr->initClient();
                }
            }
        }
		success = true;
	}

    return success;
}

/**
 * c'tor 
 */
HbSharedMemoryManagerUt::HbSharedMemoryManagerUt()
{
}

/**
 * d'tor 
 */
HbSharedMemoryManagerUt::~HbSharedMemoryManagerUt()
{
}

/**
 * to change access right..
 */
void HbSharedMemoryManagerUt::setWritable( bool readWrite )
{
    writable = readWrite;
}

/**
 * to get instance of HbSharedMemoryManagerUt
 */
HbMemoryManager * HbSharedMemoryManagerUt::instance()
{
    if( !memManager ){
        memManager = new HbSharedMemoryManagerUt();
        if( !memManager->initialize() ) {
            qWarning( "HbSharedMemoryManager:Could not initialize shared memory" );
            delete memManager;
            memManager = 0;
        }
    }
    return memManager;
}

/**
 * to release instance of HbSharedMemoryManagerUt
 */
void HbSharedMemoryManagerUt::releaseInstance()
{
    delete memManager;
    memManager = 0;
}

