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


#include "hbsharedmemoryallocators_p.h"

#include <QDebug>
#include <QSharedMemory>

// this identifier is used to check, if the multisegment allocator is already
// initialized in given shared chunk
static const quint32 INITIALIZED_MULTISEGMENTALLOCATOR_IDENTIFIER = 0x4D554C54; //'MULT'

// chunk sizes
// every size is aligned to 8
static const int ChunkSizes[AMOUNT_OF_DIFFERENT_CHUNK_SIZES] = {8, 16, 24, 32, 48, 64, 120, 224};

static const int CHUNKS_IN_ONE_LIST = 512;

/* 
* HbMultiSegmentAlgo implementation
*
* Since this function is crucial to the working of HbSharedMemory manager, it's required
* that it's initialize function is successfully completed. In the OOM condition,
* the exception is thrown.
*
* All chunk sizes should be multiples of 8 (so that the memory is automatically aligned).
* Based on actual data, following ranges are used:
*
* Range of sizes      Number of allocations for several applications
* between 0-8         926
* between 9-16        7715
* between 17-24       5814
* between 25-32       364
* between 33-48       409
* between 49-64       587
* between 65-120      430
* between 121-224     2322
*
*/

/**
 * HbMultiSegmentAllocator::initialize
 *
 * Initializes multisegment allocator.
 * This allocates memory from main allocator and won't block main allocator's
 * OOM exception.
  */
void HbMultiSegmentAllocator::initialize(HbSharedMemoryWrapper *sharedChunk,
                                         const quintptr offset,
                                         HbSharedMemoryAllocator *mainAllocator)
{
    chunk = sharedChunk;
    this->offset = offset;
    this->mainAllocator = mainAllocator;

    // initialize fast index-getter table
    int index = 0;
    for (int i = 0; i < AMOUNT_OF_DIFFERENT_CHUNK_SIZES; i++) {
        int limit = ChunkSizes[i];
        do {
            indexTable[index++] = i;
        } while (index <= limit);
    }

    header = address<MultiAllocatorHeader>(offset);
    if (header->identifier == INITIALIZED_MULTISEGMENTALLOCATOR_IDENTIFIER) {
        return; // already initialized
    }

    // every chunk list have space for 512 chunks
    ChunkListHeader *listHeader;
    for (int i = 0; i < AMOUNT_OF_DIFFERENT_CHUNK_SIZES; i++) {
        header->offsetsToChunkLists[i] = mainAllocator->alloc(sizeof(ChunkListHeader)
                                                              + (sizeof(qptrdiff) + ChunkSizes[i])
                                                              * CHUNKS_IN_ONE_LIST);
        header->offsetsToFreeChunkLists[i] = header->offsetsToChunkLists[i];
        listHeader = address<ChunkListHeader>(header->offsetsToChunkLists[i]);
        listHeader->chunkListIndex = i;
        listHeader->freedChunkCursor = -1;
        listHeader->allocCursor = header->offsetsToChunkLists[i] + sizeof(ChunkListHeader);
        listHeader->previousListOffset = -1;
        listHeader->nextListOffset = -1;
        listHeader->allocatedChunks = 0;
    }

    header->identifier = INITIALIZED_MULTISEGMENTALLOCATOR_IDENTIFIER;
}

/*
* Constructor
*/
HbMultiSegmentAllocator::HbMultiSegmentAllocator():
    chunk(0),
    offset(0),
    mainAllocator(0),
    header(0)
{
}

/*
* Destructor
*/
HbMultiSegmentAllocator::~HbMultiSegmentAllocator()
{
}

/*
* alloc function
* Will throw OOM (from main allocator) in case we run out of memory.
*/
qptrdiff HbMultiSegmentAllocator::alloc(int size)
{
    // size should already be between 1...max. chunk size - no need to check

    // first find out correct list of chunks
    int i = indexTable[size];
    qptrdiff dataOffset = -1;
    qptrdiff *metaData = 0;
    // this should always point to list with free chunks
    ChunkListHeader *listHeader = address<ChunkListHeader>(header->offsetsToFreeChunkLists[i]);

    if (listHeader->freedChunkCursor >= 0) { // freedChunkCursor points to freed chunk
        dataOffset = listHeader->freedChunkCursor + sizeof(qptrdiff);
        metaData = address<qptrdiff>(listHeader->freedChunkCursor);
        listHeader->freedChunkCursor = *metaData; // points to next freed chunk
    } else { // no chunks freed -> allocate in order
        dataOffset = listHeader->allocCursor + sizeof(qptrdiff);
        metaData = address<qptrdiff>(listHeader->allocCursor);
        // we will never allocate from full list, so allocCursor is always valid
        listHeader->allocCursor += ChunkSizes[listHeader->chunkListIndex] + sizeof(qptrdiff);
    }

    // for allocated chunks metadata is the offset to this list's header
    // for freed chunks, metadata points to next free chunk (or -1 if no more freed chunks)
    *metaData = header->offsetsToFreeChunkLists[i];
    listHeader->allocatedChunks++;
    if (listHeader->allocatedChunks == CHUNKS_IN_ONE_LIST) { // list full
        if (!setFreeList(listHeader->chunkListIndex)) {
            // there is no list(s) with free chunks, so add new list
            addList(listHeader->chunkListIndex,
                mainAllocator->alloc(sizeof(ChunkListHeader) + (sizeof(qptrdiff)
                                      + ChunkSizes[listHeader->chunkListIndex])
                                        * CHUNKS_IN_ONE_LIST));
        }
    }

    return dataOffset;
}

/*
* free the offset
*/
void HbMultiSegmentAllocator::free(qptrdiff offset)
{
    // metadata has offset to list's header
    qptrdiff *metaData = address<qptrdiff>(offset - sizeof(qptrdiff));
    qptrdiff listHeaderOffset = *metaData;
    ChunkListHeader *listHeader = address<ChunkListHeader>(listHeaderOffset);
    listHeader->allocatedChunks--;
    if (listHeader->allocatedChunks == 0) {
        // if there are multiple lists, this list could be released
        ChunkListHeader *previous = 0;
        if (listHeader->previousListOffset > -1) {
            previous = address<ChunkListHeader>(listHeader->previousListOffset);
        }
        ChunkListHeader *next = 0;
        if (listHeader->nextListOffset > -1) {
            next = address<ChunkListHeader>(listHeader->nextListOffset);
        }

        if (previous || next) {
            if (previous) {
                previous->nextListOffset = listHeader->nextListOffset;
            } else {
                // with no previous list, we are removing first list
                // and because if we don't have previous, next will be valid list
                header->offsetsToChunkLists[listHeader->chunkListIndex] = listHeader->nextListOffset;
            }
            if (next) {
                next->previousListOffset = listHeader->previousListOffset;
            }
            if (setFreeList(listHeader->chunkListIndex)) {
                // there is space in other list(s), so this list can be removed
                mainAllocator->free(listHeaderOffset);
            } else {
                // all the other lists are full, so just add this list back
                addList(listHeader->chunkListIndex, listHeaderOffset);
            }
        } else {
            // only list can't be freed
            *metaData = listHeader->freedChunkCursor;
            listHeader->freedChunkCursor = offset - sizeof(qptrdiff);
        }
    } else {
        // this list is not yet empty
        *metaData = listHeader->freedChunkCursor;
        listHeader->freedChunkCursor = offset - sizeof(qptrdiff);
    }
}

/**
 * HbMultiSegmentAllocator::allocatedSize
 *
 * Used for reallocation.
 * Returns actual allocated size for given offset.
 */
int HbMultiSegmentAllocator::allocatedSize(qptrdiff offset)
{
    qptrdiff *metaData = address<qptrdiff>(offset - sizeof(qptrdiff));
    ChunkListHeader *listHeader = address<ChunkListHeader>(*metaData);
    // not actual size in alloc(), but the size of chunk, where this data is stored
    return ChunkSizes[listHeader->chunkListIndex];
}

/*
* Helper method for adding new list of chunks.
* Won't fail, because memory for the list is already allocated (offset).
*/
void HbMultiSegmentAllocator::addList(int index, qptrdiff offset)
{
    ChunkListHeader *newListHeader = address<ChunkListHeader>(offset);
    ChunkListHeader *oldListHeader = address<ChunkListHeader>(header->offsetsToChunkLists[index]);
    // when this method is called, there will be at least one list,
    // so oldListHeader is valid and also all the lists are full
    newListHeader->allocatedChunks = 0;
    newListHeader->allocCursor = offset + sizeof(ChunkListHeader);
    newListHeader->chunkListIndex = index;
    newListHeader->freedChunkCursor = -1;
    newListHeader->previousListOffset = -1;
    newListHeader->nextListOffset = header->offsetsToChunkLists[index];
    oldListHeader->previousListOffset = offset;
    header->offsetsToChunkLists[index] = offset;
    setFreeList(index); // won't fail, because we just added free list
}

/*
* Try to set (new) free list to given index. If no such list exists (all are full),
* return false. Otherwise sets free list and returns true.
*/
bool HbMultiSegmentAllocator::setFreeList(int index)
{
    ChunkListHeader *listHeader;
    qptrdiff offset = header->offsetsToChunkLists[index];
    listHeader = address<ChunkListHeader>(offset);
    bool retVal = false;
    for (;;) {
        if (listHeader->allocatedChunks < CHUNKS_IN_ONE_LIST) {
            header->offsetsToFreeChunkLists[index] = offset;
            retVal = true;
            break;
        }
        if (listHeader->nextListOffset == -1) {
            break;
        }
        offset = listHeader->nextListOffset;
        listHeader = address<ChunkListHeader>(offset);
    }
    return retVal;
}

#ifdef HB_THEME_SERVER_MEMORY_REPORT
void HbMultiSegmentAllocator::writeReport(QTextStream &reportWriter)
{
    reportWriter << "***** (Sub)HbMultiSegmentAllocator report *****\n\n";
    reportWriter << SPACE_NEEDED_FOR_MULTISEGMENT_ALLOCATOR
                 << " bytes allocated for internal bookkeeping\n";
    reportWriter << AMOUNT_OF_DIFFERENT_CHUNK_SIZES << " different chunk sizes (";
    for (int i = 0; i < AMOUNT_OF_DIFFERENT_CHUNK_SIZES-1; i++) {
        reportWriter << ChunkSizes[i] << ", ";
    }
    reportWriter << ChunkSizes[AMOUNT_OF_DIFFERENT_CHUNK_SIZES-1] << ")\n";
    reportWriter << CHUNKS_IN_ONE_LIST << " chunks in one list\n\n";

    // calculate all the data from chunklists
    int totalMemoryReserved = 0; // all the memory needed from shared chunk
    int totalBookkeepingMemory = 0; // all the space needed for internal bookkeeping
    int totalAllocatedMemory = 0; // all the allocated memory visible for clients
    for (int i = 0; i < AMOUNT_OF_DIFFERENT_CHUNK_SIZES; i++) {
        int allocations = 0;
        int listCount = 0;
        ChunkListHeader *listHeader;
        listHeader = address<ChunkListHeader>(header->offsetsToChunkLists[i]);
        for (;;) {
            allocations += listHeader->allocatedChunks;
            listCount++;
            if (listHeader->nextListOffset != -1) {
                listHeader = address<ChunkListHeader>(listHeader->nextListOffset);
            } else {
                break;
            }
        }
        reportWriter << "for chunk size " << ChunkSizes[i] << ", "
                     << listCount << " list(s) used\n";
        reportWriter << "and in these lists " << allocations << " chunks are allocated\n";
        int totalSize = listCount * (sizeof(ChunkListHeader) + (sizeof(int) + ChunkSizes[i])
                        * CHUNKS_IN_ONE_LIST);
        totalMemoryReserved += totalSize;
        reportWriter << "Total size reserved from shared chunk for these list(s): "
                     << totalSize << " bytes\n";
        int bookKeeping = listCount * (sizeof(ChunkListHeader) + sizeof(int) * CHUNKS_IN_ONE_LIST);
        totalBookkeepingMemory += bookKeeping;
        reportWriter << "  - bytes used for bookkeeping: " << bookKeeping << "\n";
        reportWriter << "  - actual allocated bytes (in chunks, not in actual data, which might be less than chunk size): "
                     << allocations * ChunkSizes[i] << "\n\n";
        totalAllocatedMemory += allocations*ChunkSizes[i];
    }

    reportWriter << "*** HbMultiSegmentAllocator summary ***\n";
    reportWriter << "Total memory reserved from shared chunk: "
                 << totalMemoryReserved << " bytes\n";
    reportWriter << "  - internal bookkeeping uses " << totalBookkeepingMemory << " bytes\n";
    reportWriter << "  - actual memory allocated by clients: "
                 << totalAllocatedMemory << " bytes\n";
    int totalFragmentationPercent = int(float(totalAllocatedMemory)
                                        / float(totalMemoryReserved) * 100);
    int usableFragmentationPercent = int(float(totalAllocatedMemory)
                                         / float(totalMemoryReserved - totalBookkeepingMemory)
                                         * 100);
    reportWriter << "allocated memory / all memory reserved from shared chunk = "
                 << totalFragmentationPercent << "%\n";
    reportWriter << "allocated memory / all usable memory for client data = "
                 << usableFragmentationPercent << "%\n";

}
#endif
