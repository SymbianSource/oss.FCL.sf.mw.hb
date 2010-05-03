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

#ifndef HBSHAREDMEMORYALLOCATORS_P_H
#define HBSHAREDMEMORYALLOCATORS_P_H

//#include <hbglobal.h>
//#include <hbthemeperf_p.h>
#include "hbthemecommon_p.h"

#define ALIGN_SIZE 4
#define ALIGN(x) ((x + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))

// space for multisegment allocator bookkeeping - to be allocated from shared memory
static const int SPACE_NEEDED_FOR_MULTISEGMENT_ALLOCATOR = 512;

// threshold for allocs going to sub allocator and main allocator
static const int MAXIMUM_ALLOC_SIZE_FOR_SUBALLOCATOR = 224;

// used to identify memory allocated by main or sub allocator
// if metadata (integer just before allocated data) & MAIN_ALLOCATOR_IDENTIFIER
// == true, data was allocated using main allocator, otherwise suballocator
// was used
static const int MAIN_ALLOCATOR_IDENTIFIER = 0x80000000;

// max. amount of different chunk sizes in multisegment allocator
static const int AMOUNT_OF_DIFFERENT_CHUNK_SIZES = 8;

class QSharedMemory;

class HB_CORE_PRIVATE_EXPORT HbSharedMemoryAllocator
{
public:
    virtual void initialize(QSharedMemory *sharedChunk,
        const unsigned int offset = 0,
        HbSharedMemoryAllocator *mainAllocator = 0) = 0;
    virtual int alloc(int size) = 0;
    virtual int allocatedSize(int offset) = 0;
    virtual void free(int offset) = 0;
    virtual ~HbSharedMemoryAllocator() { }
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    virtual void writeReport(QTextStream &reportWriter) = 0;
#endif
};



class HB_CORE_PRIVATE_EXPORT HbSplayTreeAllocator : public HbSharedMemoryAllocator
{
public:
    HbSplayTreeAllocator();
    ~HbSplayTreeAllocator();

    int alloc(int size);
    int allocatedSize(int offset);
    void free(int offset);
	void initialize(QSharedMemory *sharedChunk,
        const unsigned int offset = 0,
        HbSharedMemoryAllocator *mainAllocator = 0);
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void writeReport(QTextStream &reportWriter);
#endif

    int freeBytes();
    int allocatedBytes();

private:
    struct TreeNode
    {
        unsigned int key;
        unsigned int rightNode;
        unsigned int leftNode;
    };

    struct MemoryBlock
    {
        TreeNode lengthNode;
        TreeNode pointerNode;
        unsigned int next;
        unsigned int prev;
        int allocatorIdentifier;
    };

    struct HeapHeader
    {
        unsigned int identifier;
        unsigned int lengthNode;
        unsigned int pointerNode;
        int freeBytes;
        int allocatedBytes;
    };

private:
    unsigned int splay(unsigned int *root, unsigned int key);

    void insertNode(unsigned int *root, TreeNode *node);
    void insertNode(unsigned int *root, TreeNode *node, TreeNode *temp);
    void deleteNode(unsigned int *root, TreeNode *node, bool splayed);

    void insertLengthNode(unsigned int *root, TreeNode *node);
    void deleteLengthNode(unsigned int *root, TreeNode *node, bool splayed);

    void *toPointer(unsigned int offset) const;

private:
    QSharedMemory *chunk;
    unsigned int offset;
    HeapHeader *header;
};



class HB_CORE_PRIVATE_EXPORT HbMultiSegmentAllocator : public HbSharedMemoryAllocator
{
public:
    HbMultiSegmentAllocator();
    ~HbMultiSegmentAllocator();

    int alloc(int size);
    int allocatedSize(int offset);
    void free(int offset);
	void initialize(QSharedMemory *sharedChunk,
        const unsigned int offset = 0,
        HbSharedMemoryAllocator *mainAllocator = 0);
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void writeReport(QTextStream &reportWriter);
#endif

private:
    struct MultiAllocatorHeader
    {
        unsigned int identifier;
        // always points to the first list
        int offsetsToChunkLists[AMOUNT_OF_DIFFERENT_CHUNK_SIZES];
        // always points to the list with free chunks
        int offsetsToFreeChunkLists[AMOUNT_OF_DIFFERENT_CHUNK_SIZES];
    };

    struct ChunkListHeader
    {
        int chunkListIndex;
        int freedChunkCursor;
        int allocCursor;
        int previousListOffset;
        int nextListOffset;
        int allocatedChunks;
    };

private:
    // helper methods
    void addList(int index, int offset);
    bool setFreeList(int index);

private:
    QSharedMemory *chunk;
    unsigned int offset;
    HbSharedMemoryAllocator *mainAllocator;
    MultiAllocatorHeader *header;
    int indexTable[MAXIMUM_ALLOC_SIZE_FOR_SUBALLOCATOR+1];
};

#endif //HBSHAREDMEMORYALLOCATORS_P_H
