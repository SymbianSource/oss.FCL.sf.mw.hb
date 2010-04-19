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
static const unsigned int INITIALIZED_MULTISEGMENTALLOCATOR_IDENTIFIER = 0x4D554C54; //'MULT'

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
void HbMultiSegmentAllocator::initialize(QSharedMemory *sharedChunk,
                                         const unsigned int offset,
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

    header = (MultiAllocatorHeader*)((unsigned char*)(chunk->data()) + offset);
    if (header->identifier == INITIALIZED_MULTISEGMENTALLOCATOR_IDENTIFIER) {
        return; // already initialized
    }

    // every chunk list have space for 512 chunks
    ChunkListHeader *listHeader;
    for (int i = 0; i < AMOUNT_OF_DIFFERENT_CHUNK_SIZES; i++) {
        header->offsetsToChunkLists[i] = mainAllocator->alloc(sizeof(ChunkListHeader) + (sizeof(int)+ChunkSizes[i])*CHUNKS_IN_ONE_LIST);
        header->offsetsToFreeChunkLists[i] = header->offsetsToChunkLists[i];
        listHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + header->offsetsToChunkLists[i]);
        listHeader->chunkListIndex = i;
        listHeader->freedChunkCursor = -1;
        listHeader->allocCursor = header->offsetsToChunkLists[i]+sizeof(ChunkListHeader);
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
int HbMultiSegmentAllocator::alloc(int size)
{
    // size should already be between 1...max. chunk size - no need to check

    // first find out correct list of chunks
    int i = indexTable[size];

    // qDebug() << "HbMultiSegmentAllocator::alloc, with size " << size << " chunkList " << i << " used\n";

    int dataOffset = -1;
    int *metaData = 0;
    // this should always point to list with free chunks
    ChunkListHeader *listHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + header->offsetsToFreeChunkLists[i]);

    if (listHeader->freedChunkCursor >= 0) { // freedChunkCursor points to freed chunk
        dataOffset = listHeader->freedChunkCursor + sizeof(int);
        metaData = (int*)((unsigned char*)(chunk->data())+listHeader->freedChunkCursor);
        listHeader->freedChunkCursor = *metaData; // point to next freed chunk
    } else { // no chunks freed -> allocate in order
        dataOffset = listHeader->allocCursor + sizeof(int);
        metaData = (int*)((unsigned char*)(chunk->data())+listHeader->allocCursor);
        // we will never allocate from full list, so allocCursor is always valid
        listHeader->allocCursor += ChunkSizes[listHeader->chunkListIndex]+sizeof(int);
    }

    // for allocated chunks metadata is:
    // 00xxxxxx xxxxxxxx xxxxxxxx xxxxxxxx,
    // where xx... = offset to this list's header (max. 1 GB)
    *metaData = header->offsetsToFreeChunkLists[i] /* & 0x3FFFFFFF */;
    listHeader->allocatedChunks++;
    if (listHeader->allocatedChunks == CHUNKS_IN_ONE_LIST) { // list full
        if (!setFreeList(listHeader->chunkListIndex)) {
            // there is no list(s) with free chunks, so add new list
            addList(listHeader->chunkListIndex,
                mainAllocator->alloc((sizeof(ChunkListHeader)+sizeof(int)+ChunkSizes[listHeader->chunkListIndex])*CHUNKS_IN_ONE_LIST));
        }
    }

    return dataOffset;
}

/*
* free the offset
*/
void HbMultiSegmentAllocator::free(int offset)
{
    // metadata has offset to list's header
    int *metaData = (int*)((unsigned char*)(chunk->data())+offset-sizeof(int));
    int listHeaderOffset = *metaData;
    ChunkListHeader *listHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + listHeaderOffset);
    listHeader->allocatedChunks--;
    if (listHeader->allocatedChunks == 0) {
        // if there are multiple lists, this list could be released
        ChunkListHeader *previous = 0;
        if (listHeader->previousListOffset > -1) {
            previous = (ChunkListHeader*)((unsigned char*)(chunk->data()) + listHeader->previousListOffset);
        }
        ChunkListHeader *next = 0;
        if (listHeader->nextListOffset > -1) {
            next = (ChunkListHeader*)((unsigned char*)(chunk->data()) + listHeader->nextListOffset);
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
            listHeader->freedChunkCursor = offset-sizeof(int);
        }
    } else {
        // this list is not yet empty
        *metaData = listHeader->freedChunkCursor;
        listHeader->freedChunkCursor = offset-sizeof(int);
    }
}

/**
 * HbMultiSegmentAllocator::allocatedSize
 *
 * Used for reallocation.
 * Returns actual allocated size for given offset.
 */
int HbMultiSegmentAllocator::allocatedSize(int offset)
{
    int *metaData = (int*)((unsigned char*)(chunk->data())+offset-sizeof(int));
    ChunkListHeader *listHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + *metaData);
    // not actual size in alloc(), but the size of chunk, where this data is stored
    return ChunkSizes[listHeader->chunkListIndex];
}

/*
* Helper method for adding new list of chunks.
* Won't fail, because memory for the list is already allocated (offset).
*/
void HbMultiSegmentAllocator::addList(int index, int offset)
{
    ChunkListHeader *newListHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + offset);
    ChunkListHeader *oldListHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + header->offsetsToChunkLists[index]);
    // when this method is called, there will be at least one list,
    // so oldListHeader is valid and also all the lists are full
    newListHeader->allocatedChunks = 0;
    newListHeader->allocCursor = offset+sizeof(ChunkListHeader);
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
    listHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + header->offsetsToChunkLists[index]);
    bool retVal = false;
    for (;;) {
        if (listHeader->allocatedChunks < CHUNKS_IN_ONE_LIST) {
            int offset = (int)((char*)(listHeader)-(char*)(chunk->data()));
            header->offsetsToFreeChunkLists[index] = offset;
            retVal = true;
            break;
        }
        if (listHeader->nextListOffset == -1) {
            break;
        }
        listHeader = (ChunkListHeader*)((unsigned char*)(chunk->data()) + listHeader->nextListOffset);
    }

    return retVal;
}
