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
#include <QHash>
#include "hbstring_p.h"
#include "hbcssconverterutils_p.h"
#include "hbsharedmemorymanager_p.h"
#include "hbsharedmemoryallocators_p.h"

static QHash<QString, HbString> strMap;

// Global list that stores pointers to the member variables where shared container instances
// store offsets returned my memory allocator.
// CSS converter utilizes it to automatically adjust the offsets if allocated cells are moved.

// Map is used only to get faster lookups, item's value is obsolete
static QMap<qptrdiff *, int> registered;


// Shared chunk allocation information for css data
static int totalAllocated = 0;
// Using map instead of hash to guarantee that the items are in order
// so the cell with offset 0 which is the HbCss::StyleSheet structure
// is always first in the cell list, so its offset does not get changed
// when the chunk is defragmented.
static QMap<qptrdiff, int> cells;


void HbCssConverterUtils::registerOffsetHolder(qptrdiff *offset)
{
    registered.insert(offset, 1);
}

void HbCssConverterUtils::unregisterOffsetHolder(qptrdiff *offset)
{
    registered.remove(offset);
}

QMultiHash<int, qptrdiff *> HbCssConverterUtils::registeredOffsetHolders()
{
    QMultiHash<int, qptrdiff *> holders;
    holders.reserve(registered.size());
    QMap<qptrdiff *, int>::const_iterator end = registered.constEnd();
    for (QMap<qptrdiff *, int>::const_iterator i = registered.constBegin(); i != end; ++i) {
        qptrdiff *holder = i.key();
        holders.insertMulti(*holder, holder);
    }
    return holders;
}

void HbCssConverterUtils::unregisterAll()
{
    registered.clear();
}


void HbCssConverterUtils::cellAllocated(qptrdiff offset, int size)
{
    cells.insert(offset, ALIGN(size));
    totalAllocated += ALIGN(size);
}

void HbCssConverterUtils::cellFreed(qptrdiff offset)
{
    int size = cells.value(offset, 0);
    totalAllocated -= size;
    cells.remove(offset);

    if (size > 0) {
        // Make sure there are no registered offset holders in the freed cell any more
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);
        const char *chunkBase = static_cast<const char *>(shared->base());
        const char *freedEnd = chunkBase + offset + size;
        const qptrdiff *freedStart = reinterpret_cast<const qptrdiff *>(chunkBase + offset);

        QMap<qptrdiff *, int>::iterator freedHolders =
                registered.lowerBound(const_cast<qptrdiff *>(freedStart));
        while(freedHolders != registered.end()
              && reinterpret_cast<char*>(freedHolders.key()) <  freedEnd) {
            freedHolders = registered.erase(freedHolders);
        }
    }
}

void HbCssConverterUtils::cellMoved(qptrdiff offset, qptrdiff newOffset)
{
    int size = cells.value(offset, 0);

    if (size > 0) {
        // Check if there were registered offset holders in the old cell
        // and register corresponding ones in the reallocated cell.
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);
        const char *chunkBase = static_cast<const char *>(shared->base());
        const char *freedEnd = chunkBase + offset + size;
        const qptrdiff *freedStart = reinterpret_cast<const qptrdiff *>(chunkBase + offset);

        QMap<qptrdiff *, int>::iterator freedHolders =
                registered.lowerBound(const_cast<qptrdiff *>(freedStart));
        QList<qptrdiff *> newHolders;
        while(freedHolders != registered.end()
              && reinterpret_cast<char*>(freedHolders.key()) <  freedEnd) {
            char *holderC = reinterpret_cast<char*>(freedHolders.key());
            newHolders.append(reinterpret_cast<qptrdiff*>(holderC + newOffset - offset));
            freedHolders = registered.erase(freedHolders);
        }
        for (int i = 0; i < newHolders.size(); ++i) {
            registerOffsetHolder(newHolders.at(i));
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
    strMap.clear();

    // Register shared cache pointer in chunk header
    //as shared cache may also be moved in defragmentation
    HbSharedChunkHeader *chunkHeader = static_cast<HbSharedChunkHeader*>(shared->base());
    registerOffsetHolder(reinterpret_cast<qptrdiff *>(&chunkHeader->sharedCacheOffset));

    QMultiHash<int, qptrdiff *> offsetHolders = registeredOffsetHolders();

    // Create new buffer where the current chunk contents are defragmented
    char *buffer = static_cast<char*>(::malloc(shared->size()));
    qptrdiff newCurrentOffset = 0;

    // Create new cell order and update offset holders
    QMap<qptrdiff,int>::const_iterator end = cells.constEnd();

    for (QMap<qptrdiff,int>::const_iterator i = cells.constBegin(); i != end; ++i) {
        // Get the old cell
        qptrdiff offset = i.key();
        int size = i.value();
        
        // Update registered offset holders
        QList<qptrdiff *> values = offsetHolders.values(offset);
        offsetHolders.remove(offset);
        qptrdiff newOffset = newCurrentOffset + sizeof(HbSharedChunkHeader);
        for (int j = 0; j < values.size(); ++j) {
            qptrdiff *holder = values[j];
            *holder = newOffset;
            offsetHolders.insertMulti(*holder, holder);
        }
        newCurrentOffset += size;
    }

    newCurrentOffset = 0;
    // Move allocated cells to a linear buffer
    for (QMap<qptrdiff, int>::const_iterator i = cells.constBegin(); i != end; ++i) {
        // Get the old cell
        qptrdiff offset = i.key();
        int size = i.value();
        // Copy to new chunk
        memcpy(buffer + newCurrentOffset, static_cast<char*>(shared->base()) + offset, size);
        newCurrentOffset += size;
    }

    // Free all cells from the shared chunk and move the defragmented buffer in the beginning of the chunk.
    // Note that chunk memory management is screwed up after this point, so no more allocations should be
    // done in it after this.

    unregisterAll();
    QList<qptrdiff> keys = cells.keys();

    for (int j = 0; j < keys.count(); ++j) {
        shared->free(keys.at(j));
    }

    // CSS binary data is placed after the chunk header.
    qptrdiff cssBinaryOffset = sizeof(HbSharedChunkHeader);
    char *address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, cssBinaryOffset);
    memcpy(address, buffer, newCurrentOffset);

    cells.clear();
    totalAllocated = 0;

    // Free the temp buffer
    ::free(buffer);

    // Return the next free address in the chunk
    return cssBinaryOffset + newCurrentOffset;
}

void HbCssConverterUtils::addSharedStringData(const QString &str, const HbString &hbstr)
{
    if (!strMap.contains(str)) {
        strMap.insert(str, hbstr);
    }
}

HbString* HbCssConverterUtils::sharedStringData(const QString &str)
{
    HbString *ret = 0;
    QHash<QString, HbString>::iterator i = strMap.find(str);
    if (i != strMap.end()) {
        ret = &i.value();
    }
    return ret;
}

