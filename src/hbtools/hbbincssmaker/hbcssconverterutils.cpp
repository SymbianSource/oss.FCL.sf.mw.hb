/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbTools module of the UI Extensions for Mobile.
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

#include <QMap>
#include "hbcssconverterutils_p.h"
#include "hbsharedmemorymanager_p.h"
#include "hbsharedmemoryallocators_p.h"

// Global list that stores pointers to the member variables where shared container instances
// store offsets returned my memory allocator.
// CSS converter utilizes it to automatically adjust the offsets if allocated cells are moved.

// Map is used only to get faster lookups, item's value is obsolete
static QMap<int *, int> registered;


// Shared chunk allocation information for css data
static int totalAllocated = 0;
// Using map instead of hash to guarantee that the items are in order
// so the cell with offset 0 which is the HbCss::StyleSheet structure
// is always first in the cell list, so its offset does not get changed
// when the chunk is defragmented.
static QMap<int, int> cells;


void HbCssConverterUtils::registerOffsetHolder(int *offset)
{
    registered.insert(offset, 1);
}

void HbCssConverterUtils::unregisterOffsetHolder(int *offset)
{
    registered.remove(offset);
}


QList<int *> HbCssConverterUtils::registeredOffsetHolders()
{
    return registered.keys();
}

void HbCssConverterUtils::unregisterAll()
{
    registered.clear();
}


void HbCssConverterUtils::cellAllocated(int offset, int size)
{
    cells.insert(offset, ALIGN(size));
    totalAllocated += ALIGN(size);
}

void HbCssConverterUtils::cellFreed(int offset)
{
    int size = cells.value(offset, 0);
    totalAllocated -= size;
    cells.remove(offset);

    if (size > 0) {
        // Make sure there are no registered offset holders in the freed cell any more
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);
        const char *chunkBase = static_cast<const char *>(shared->base());

        QList<int *> offsetHolders = HbCssConverterUtils::registeredOffsetHolders();
        for (int i = 0; i<offsetHolders.count(); ++i) {
            int *holder = offsetHolders.at(i);
            if ((char*)holder >= chunkBase + offset && (char*)holder < chunkBase + offset + size) {
                HbCssConverterUtils::unregisterOffsetHolder(holder);
            }
        }
    }
}

void HbCssConverterUtils::cellMoved(int offset, int newOffset)
{
    int size = cells.value(offset, 0);

    if (size > 0) {
        // Check if there were registered offset holders in the old cell
        // and register corresponding ones in the reallocated cell.
        QList<int *> holders = HbCssConverterUtils::registeredOffsetHolders();

        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);
        const char *chunkBase = static_cast<const char *>(shared->base());    
        
        for (int i=0; i<holders.count(); i++) {
            int *holder = holders.at(i);
            char *holderC = (char*)holder;
            if (holderC >= chunkBase + offset && holderC < chunkBase + offset + size) {
                HbCssConverterUtils::unregisterOffsetHolder(holder);
                HbCssConverterUtils::registerOffsetHolder((int*)(holderC + newOffset - offset));
            }
        }
    }
}

/**
* Defragments the shared chunk contents and places defragmented buffer in the beginning of the chunk.
* Registered chunk offset holders are updated during the process.
* Returns the next free offset in the chunk.
*/
int HbCssConverterUtils::defragmentChunk()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);

    // Register shared cache pointer in chunk header as shared cache may also be moved in defragmentation
    HbSharedChunkHeader *chunkHeader = static_cast<HbSharedChunkHeader*>(shared->base());    
    HbCssConverterUtils::registerOffsetHolder(reinterpret_cast<int *>(&chunkHeader->sharedCacheOffset));

    QList<int *> offsetHolders = HbCssConverterUtils::registeredOffsetHolders();

    // Create new buffer where the current chunk contents are defragmented
    void *buffer = ::malloc(shared->size());
    int newCurrentOffset = 0;

    // Create new cell order and update offset holders
    QMap<int,int>::const_iterator i = cells.constBegin();

    while (i != cells.constEnd()) {
        // Get the old cell
        int offset = i.key();
        int size = i.value();
        
        // Update registered offset holders

        // TODO: optimize this, now there's linear search for each cell!
		for (int j=0; j<offsetHolders.count(); ++j) {
			int *holder = offsetHolders.at(j);
			if (*holder == offset) {
				// Change stored offset value
				*holder = newCurrentOffset + sizeof(HbSharedChunkHeader);
			}
		}

        newCurrentOffset += size;
        i++;
    }

    i = cells.constBegin();
    newCurrentOffset = 0;

    // Move allocated cells to a linear buffer
    while (i != cells.constEnd()) {
        // Get the old cell
        int offset = i.key();
        int size = i.value();
        // Copy to new chunk
        memcpy((char*)buffer + newCurrentOffset, (char*)shared->base() + offset, size);

        newCurrentOffset += size;
        i++;
    }

    // Free all cells from the shared chunk and move the defragmented buffer in the beginning of the chunk.
    // Note that chunk memory management is screwed up after this point, so no more allocations should be
    // done in it after this.

    HbCssConverterUtils::unregisterAll();
    QList<int> keys = cells.keys();

    for (int j=0; j<keys.count(); ++j) {
        shared->free(keys.at(j));
    }

    // CSS binary data is placed after the chunk header.
    int cssBinaryOffset = sizeof(HbSharedChunkHeader);
    char *address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, cssBinaryOffset);
    memcpy(address, buffer, newCurrentOffset);

    cells.clear();
    totalAllocated = 0;

    // Free the temp buffer
    ::free(buffer);

    // Return the next free address in the chunk
    return cssBinaryOffset + newCurrentOffset;
}
