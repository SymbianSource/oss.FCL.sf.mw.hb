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

#include "hbthemecommon_p.h"
#include <QSharedMemory>

static const int ALIGN_SIZE = 4;
#define ALIGN(x) ((x + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))

// space for multisegment allocator bookkeeping - to be allocated from shared memory
static const int SPACE_NEEDED_FOR_MULTISEGMENT_ALLOCATOR = 512;

// threshold for allocs going to sub allocator and main allocator
static const int MAXIMUM_ALLOC_SIZE_FOR_SUBALLOCATOR = 224;

// used to identify memory allocated by main or sub allocator
// if metadata (qptrdiff just before allocated data) & MAIN_ALLOCATOR_IDENTIFIER
// == true, data was allocated using main allocator, otherwise suballocator
// was used. This works because suballocator saves aligned offsets to metadata and
// they can't be odd.
static const qptrdiff MAIN_ALLOCATOR_IDENTIFIER = 1;

// max. amount of different chunk sizes in multisegment allocator
static const int AMOUNT_OF_DIFFERENT_CHUNK_SIZES = 8;


// wrapper for hiding Symbian specific protected chunk
class HbSharedMemoryWrapper
{
public:
    HbSharedMemoryWrapper(const QString &key, QObject *parent = 0);
    ~HbSharedMemoryWrapper();
    
    bool create(int size, QSharedMemory::AccessMode mode = QSharedMemory::ReadWrite);
    QSharedMemory::SharedMemoryError error() const;
    bool attach(QSharedMemory::AccessMode mode = QSharedMemory::ReadWrite);
    void *data();
    int size() const;
    
#if defined(HB_HAVE_PROTECTED_CHUNK) && defined(Q_OS_SYMBIAN)
    void setErrorString(const QString &function, TInt errorCode);
#endif    
private:
#if defined(HB_HAVE_PROTECTED_CHUNK) && defined(Q_OS_SYMBIAN)
    QSharedMemory::SharedMemoryError wrapperError;
    QString errorString;
    const QString key;
    RChunk chunk;
    int memorySize;
    void *memory;
#else
    QSharedMemory *chunk;
#endif
};


class HbSharedMemoryAllocator
{
public:
    virtual void initialize(HbSharedMemoryWrapper *sharedChunk,
        const quintptr offset = 0,
        HbSharedMemoryAllocator *mainAllocator = 0) = 0;
    virtual qptrdiff alloc(int size) = 0;
    virtual int allocatedSize(qptrdiff offset) = 0;
    virtual void free(qptrdiff offset) = 0;
    virtual ~HbSharedMemoryAllocator() { }
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    virtual void writeReport(QTextStream &reportWriter) = 0;
#endif
};


class HbSplayTreeAllocator : public HbSharedMemoryAllocator
{
public:
    HbSplayTreeAllocator();
    ~HbSplayTreeAllocator();

    qptrdiff alloc(int size);
    int allocatedSize(qptrdiff offset);
    void free(qptrdiff offset);
    void initialize(HbSharedMemoryWrapper *sharedChunk,
        const quintptr offset = 0,
        HbSharedMemoryAllocator *mainAllocator = 0);
    int size();
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void writeReport(QTextStream &reportWriter);
#endif

    int freeBytes();
    int allocatedBytes();

private:
    struct TreeNode
    {
        quintptr key;
        quintptr rightNode;
        quintptr leftNode;
    };

    struct MemoryBlock
    {
        TreeNode lengthNode;
        TreeNode pointerNode;
        quintptr next;
        quintptr prev;
        qptrdiff allocatorIdentifier;
    };

    struct HeapHeader
    {
        quint32 identifier;
        quintptr lengthNode;
        quintptr pointerNode;
        int freeBytes;
        int allocatedBytes;
    };

private:
    quintptr splay(quintptr *root, quintptr key);

    void insertNode(quintptr *root, TreeNode *node);
    void insertNode(quintptr *root, TreeNode *node, TreeNode *temp);
    void deleteNode(quintptr *root, TreeNode *node, bool splayed);

    void insertLengthNode(quintptr *root, TreeNode *node);
    void deleteLengthNode(quintptr *root, TreeNode *node, bool splayed);

    void *toPointer(quintptr offset) const;
    template<typename T>
    inline T *address(qptrdiff offset)
    {
        return reinterpret_cast<T *>(static_cast<char *>(chunk->data()) + offset);
    }

private:
    HbSharedMemoryWrapper *chunk;
    quintptr offset;
    HeapHeader *header;
};



class HbMultiSegmentAllocator : public HbSharedMemoryAllocator
{
public:
    HbMultiSegmentAllocator();
    ~HbMultiSegmentAllocator();

    qptrdiff alloc(int size);
    int allocatedSize(qptrdiff offset);
    void free(qptrdiff offset);
    void initialize(HbSharedMemoryWrapper *sharedChunk,
        const quintptr offset = 0,
        HbSharedMemoryAllocator *mainAllocator = 0);
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    void writeReport(QTextStream &reportWriter);
#endif

private:
    struct MultiAllocatorHeader
    {
        quint32 identifier;
        // always points to the first list
        qptrdiff offsetsToChunkLists[AMOUNT_OF_DIFFERENT_CHUNK_SIZES];
        // always points to the list with free chunks
        qptrdiff offsetsToFreeChunkLists[AMOUNT_OF_DIFFERENT_CHUNK_SIZES];
    };

    struct ChunkListHeader
    {
        int chunkListIndex;
        qptrdiff freedChunkCursor;
        qptrdiff allocCursor;
        qptrdiff previousListOffset;
        qptrdiff nextListOffset;
        int allocatedChunks;
    };

private:
    // helper methods
    void addList(int index, qptrdiff offset);
    bool setFreeList(int index);
    template<typename T>
    inline T *address(qptrdiff offset)
    {
        return reinterpret_cast<T *>(static_cast<char *>(chunk->data()) + offset);
    }

private:
    HbSharedMemoryWrapper *chunk;
    quintptr offset;
    HbSharedMemoryAllocator *mainAllocator;
    MultiAllocatorHeader *header;
    int indexTable[MAXIMUM_ALLOC_SIZE_FOR_SUBALLOCATOR+1];
};

#endif //HBSHAREDMEMORYALLOCATORS_P_H
