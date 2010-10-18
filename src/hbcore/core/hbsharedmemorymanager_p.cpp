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
#include "hbsharedmemoryallocators_p.h"
#include "hbmemoryutils_p.h"
#include <QSharedMemory>
#include <QString>
#include <QList>
#include <QDebug>
#ifdef HB_THEME_SERVER_MEMORY_REPORT
#include <QDate>
#include <QDir>
#include <QFile>
#endif
#include "hbsharedcache_p.h"
#include "hbsharedmemorymanagerut_p.h"
#ifdef HB_BIN_CSS
#include "hbcssconverterutils_p.h"
#endif // HB_BIN_CSS

#define USE_SUBALLOCATOR

#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
// identifier and size are packed in one quint32
static const int allocIdentifier =   0x80000000;
static const int freeIdentifier =    0x40000000;
static const int reallocIdentifier = 0xC0000000;
#endif

#if defined(HB_HAVE_PROTECTED_CHUNK) && defined(Q_OS_SYMBIAN)
HbSharedMemoryWrapper::HbSharedMemoryWrapper(const QString &key, QObject *parent) :
    wrapperError(QSharedMemory::NoError),        
    key(key),
    memorySize(0),
    memory(0)
{
    Q_UNUSED(parent);
}

HbSharedMemoryWrapper::~HbSharedMemoryWrapper()
{
    chunk.Close();

    memory = 0;
    memorySize = 0;

}

void HbSharedMemoryWrapper::setErrorString(const QString &function, TInt errorCode)
{
    if (errorCode == KErrNone)
        return;
    switch (errorCode) {
    case KErrAlreadyExists:
        wrapperError = QSharedMemory::AlreadyExists;
        errorString = QSharedMemory::tr("%1: already exists").arg(function);
    break;
    case KErrNotFound:
        wrapperError = QSharedMemory::NotFound;
        errorString = QSharedMemory::tr("%1: doesn't exists").arg(function);
        break;
    case KErrArgument:
        wrapperError = QSharedMemory::InvalidSize;
        errorString = QSharedMemory::tr("%1: invalid size").arg(function);
        break;
    case KErrNoMemory:
        wrapperError = QSharedMemory::OutOfResources;
        errorString = QSharedMemory::tr("%1: out of resources").arg(function);
        break;
    case KErrPermissionDenied:
        wrapperError = QSharedMemory::PermissionDenied;
        errorString = QSharedMemory::tr("%1: permission denied").arg(function);
        break;
    default:
        errorString = QSharedMemory::tr("%1: unknown error %2").arg(function).arg(errorCode);
        wrapperError = QSharedMemory::UnknownError;
    }
}

bool HbSharedMemoryWrapper::create(int size, QSharedMemory::AccessMode mode)
{
    Q_UNUSED(mode);
    TPtrC ptr(TPtrC16(static_cast<const TUint16*>(key.utf16()), key.length()));

    TChunkCreateInfo info;
    info.SetReadOnly();
    info.SetGlobal(ptr);
    info.SetNormal(size, size);
    
    //TInt err = chunk.CreateGlobal(ptr, size, size); // Original Qt version
    TInt err = chunk.Create(info);

    QString function = QLatin1String("HbSharedMemoryWrapper::create");    
    setErrorString(function, err);

    if (err != KErrNone)
        return false;

    // Zero out the created chunk
    Mem::FillZ(chunk.Base(), chunk.Size());

    memorySize = chunk.Size();
    memory = chunk.Base();
    
    return true;
}

QSharedMemory::SharedMemoryError HbSharedMemoryWrapper::error() const
{
    return wrapperError;
}

bool HbSharedMemoryWrapper::attach(QSharedMemory::AccessMode mode)
{
    Q_UNUSED(mode);    
    // Grab a pointer to the memory block
    if (!chunk.Handle()) {
        TPtrC ptr(TPtrC16(static_cast<const TUint16*>(key.utf16()), key.length()));        

        TInt err = KErrNoMemory;

        err = chunk.OpenGlobal(ptr, false);

        if (err != KErrNone) {
            QString function = QLatin1String("HbSharedMemoryWrapper::attach");        
            setErrorString(function, err);
            return false;
        }
    }

    memorySize = chunk.Size();
    memory = chunk.Base();

    return true;
}

void *HbSharedMemoryWrapper::data()
{
    return memory;
}

int HbSharedMemoryWrapper::size() const
{
    return memorySize;
}
#else // use QSharedMemory
HbSharedMemoryWrapper::HbSharedMemoryWrapper(const QString &key, QObject *parent)
{
    chunk = new QSharedMemory(key, parent);
}

HbSharedMemoryWrapper::~HbSharedMemoryWrapper()
{
    delete chunk;
    chunk = 0;
}

bool HbSharedMemoryWrapper::create(int size, QSharedMemory::AccessMode mode)
{
    if (chunk) {
        return chunk->create(size, mode);
    }
    return false;
}

QSharedMemory::SharedMemoryError HbSharedMemoryWrapper::error() const
{
    return chunk->error();
}

bool HbSharedMemoryWrapper::attach(QSharedMemory::AccessMode mode)
{
    if (chunk) {
        return chunk->attach(mode);
    }
    return false;
}

void *HbSharedMemoryWrapper::data()
{
    if (chunk) {
        return chunk->data();
    }
    return 0;
}

int HbSharedMemoryWrapper::size() const
{
    if (chunk) {
        return chunk->size();
    }
    return 0;
}
#endif

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
    chunk = new HbSharedMemoryWrapper(HB_THEME_SHARED_PIXMAP_CHUNK);
    // check if app filename is same as server filename ..
    // ToDo: improve server identification logic.. UID on symbian?
    const QString &appName = HbMemoryUtils::getCleanAppName();
    bool binCSSConverterApp = (appName == BIN_CSS_APP || appName == BIN_CSS_APP_SYMBIAN);
    // Testability support: allowing unit test to write to shared chunk also
    if (appName == THEME_SERVER_NAME || appName == SHARED_MEMORY_MANAGER_UNIT_TEST || binCSSConverterApp) {
        // This is server, create shared memory chunk
        success = chunk->create(CACHE_SIZE, QSharedMemory::ReadWrite);
        // If sharedMemory already exists.
        // (This can happpen if ThemeServer crashed without releasing QSharedMemory)
        if (!success && QSharedMemory::AlreadyExists == chunk->error()) {
            success = chunk->attach(QSharedMemory::ReadWrite);
        }
        writable = true;
    } else {
        // this is not server so just attach to shared memory chunk in ReadOnly mode
        success = chunk->attach(QSharedMemory::ReadOnly);
        writable = false;
    }
    if ( !success ) {
        THEME_GENERIC_DEBUG() << "HbSharedMemoryManager:: Could not initialize shared memory chunk";
        delete chunk; 
        chunk = 0;
    }
    if (success && isWritable()) {
        // if we are recovering from theme server crash, shared chunk may
        // already be ready
        bool enableRecovery = false;
        if (binCSSConverterApp) {
            enableRecovery = false;
        }
        HbSharedChunkHeader *chunkHeader = static_cast<HbSharedChunkHeader*>(chunk->data());
        HbSharedCache *cachePtr = 0;
        if (enableRecovery && chunkHeader->identifier == INITIALIZED_CHUNK_IDENTIFIER) {
            // just reconnect allocators to the shared chunk
            mainAllocator->initialize(chunk, chunkHeader->mainAllocatorOffset);
            subAllocator->initialize(chunk, chunkHeader->subAllocatorOffset, mainAllocator);
        } else {
            memset(chunkHeader, 0, sizeof(HbSharedChunkHeader));
            // Load memory file in the beginning of the chunk first.
            int memoryFileSize = 0;

#ifdef Q_OS_SYMBIAN
            if (!binCSSConverterApp) {
                QString memoryFile("z:/resource/hb/themes/hbdefault.cssbin");
                memoryFileSize = loadMemoryFile(memoryFile);
            }
#endif
            // Put main allocator after the memory file or if memory file was not loaded, after chunk header.
            chunkHeader->mainAllocatorOffset = memoryFileSize ? ALIGN(memoryFileSize)
                                                              : sizeof(HbSharedChunkHeader);
            // Clear also allocator identifier so that they will not try to re-connect
            quint32 *mainAllocatorIdentifier = address<quint32>(chunkHeader->mainAllocatorOffset);
            *mainAllocatorIdentifier = 0;
            mainAllocator->initialize(chunk, chunkHeader->mainAllocatorOffset);
            chunkHeader->subAllocatorOffset = alloc(SPACE_NEEDED_FOR_MULTISEGMENT_ALLOCATOR);
            quint32 *subAllocatorIdentifier = address<quint32>(chunkHeader->subAllocatorOffset);
            *subAllocatorIdentifier = 0;
            subAllocator->initialize(chunk, chunkHeader->subAllocatorOffset, mainAllocator);
            chunkHeader->identifier = INITIALIZED_CHUNK_IDENTIFIER;
            
            if (!binCSSConverterApp) {
                if (memoryFileSize == 0) {
                    cachePtr = createSharedCache(0, 0, 0, -1);
                }
            }
        }
        if (!cachePtr) {
            cachePtr = cache();
        }
        if (cachePtr && !binCSSConverterApp) {
            cachePtr->initServer();
        }

        success = true;
    } else {
        HbSharedCache *cachePtr = cache();
        if (cachePtr) {
            cachePtr->initClient();
        }
    }
    return success;
}

/**
 * HbSharedMemoryManager::alloc
 * 
 * This function throws std::bad_alloc in case of OOM condition, else
 * proper offset in case of a successful memory allocation
 */
qptrdiff HbSharedMemoryManager::alloc(int size)
{
#ifdef HB_THEME_SERVER_MEMORY_REPORT
// some code copied, but much more readable this way
    if (isWritable() && size > 0) {
        int offset = -1;
#ifdef USE_SUBALLOCATOR
        if (size <= MAXIMUM_ALLOC_SIZE_FOR_SUBALLOCATOR) {
            offset = subAllocator->alloc(size);
        } else {
            try{
                // main allocator may throw bad_alloc
                offset = mainAllocator->alloc(size);
            }
            catch(std::bad_alloc &badAlloc){
                Q_UNUSED(badAlloc);
                OOMCount++;
                if (!OOMReportCreated) {
                    OOMReportCreated = true;
                    createReport();
                }
                throw;
            }
        }
#else
        try{
            // main allocator may throw bad_alloc
            offset = mainAllocator->alloc(size);
        }
        catch(std::bad_alloc &badAlloc){
            OOMCount++;
            throw;
        }
#endif
        if (offset == -1) {
            // we ran out of memory
            OOMCount++;
            if (!OOMReportCreated) {
                OOMReportCreated = true;
                createReport();
            }
            throw std::bad_alloc();
        }
    if (allocations.contains(size)) {
        allocations[size].first++;
    } else {
        allocations.insert(size, QPair<quint32, quint32>(1,0));
    }

#else // normal alloc without reporting
    if (isWritable() && size > 0) {
        qptrdiff offset = -1;
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
#endif // HB_THEME_SERVER_MEMORY_REPORT

#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
        fullAllocationHistory.append(QPair<quint32,quint32>(size | allocIdentifier, offset));
#endif

#ifdef HB_BIN_CSS
        HbCssConverterUtils::cellAllocated(offset, size);
#endif
        return offset;
    } else {
        // memory manager attached in read only mode, cannot allocate memory
        throw std::bad_alloc();
    }
}

/**
 * free
 */
void HbSharedMemoryManager::free(qptrdiff offset)
{
    // don't do anything when freeing NULL (pointer)offset
    if (isWritable() && (offset > 0)) {
        qptrdiff metaData = *address<qptrdiff>(offset - sizeof(qptrdiff));
#ifdef HB_THEME_SERVER_MEMORY_REPORT
        int size = 0;
        if (metaData & MAIN_ALLOCATOR_IDENTIFIER) {
            size = mainAllocator->allocatedSize(offset);
        } else {
            size = subAllocator->allocatedSize(offset);
        }

        if (allocations.contains(size)) {
            allocations[size].second++;
        }
#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
        fullAllocationHistory.append(QPair<quint32,quint32>(size | freeIdentifier, offset));
#endif

#endif //HB_THEME_SERVER_MEMORY_REPORT
        if (metaData & MAIN_ALLOCATOR_IDENTIFIER) {
            mainAllocator->free(offset);
        } else {
            subAllocator->free(offset);
        }
#ifdef HB_BIN_CSS
        HbCssConverterUtils::cellFreed(offset);
#endif
    }
}

/**
 * realloc
 * 
 * This function can throw if alloc fails
 */
qptrdiff HbSharedMemoryManager::realloc(qptrdiff offset, int newSize)
{
    qptrdiff newOffset = -1;
    if (isWritable()) {
#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
        if (offset > 0) { // if offset == -1, just do normal alloc and not report realloc
            fullAllocationHistory.append(QPair<quint32, quint32>(newSize | reallocIdentifier, offset));
        }
#endif
        newOffset = alloc(newSize);
        int allocatedSize = ALIGN(newSize);
        if (offset > 0) {
#ifdef HB_BIN_CSS
            HbCssConverterUtils::cellMoved(offset, newOffset);
#endif
            unsigned char *scrPtr = address<unsigned char>(offset);
            qptrdiff metaData = *address<qptrdiff>(offset - sizeof(qptrdiff));
            if (metaData & MAIN_ALLOCATOR_IDENTIFIER) {
                int oldSize = mainAllocator->allocatedSize(offset);
                memcpy(address<unsigned char>(newOffset), scrPtr, qMin(oldSize, allocatedSize));
#ifdef HB_THEME_SERVER_MEMORY_REPORT
                free(offset);
#else
                mainAllocator->free(offset);
#endif
            } else {
                int oldSize = subAllocator->allocatedSize(offset);
                memcpy(address<unsigned char>(newOffset), scrPtr, qMin(oldSize, allocatedSize));
#ifdef HB_THEME_SERVER_MEMORY_REPORT
                free(offset);
#else
                subAllocator->free(offset);
#endif
            }
#if HB_BIN_CSS
            // Does not matter if already called when calling free() above.
            HbCssConverterUtils::cellFreed(offset);
#endif
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
    : HbMemoryManager(true),
     mainAllocator(new HbSplayTreeAllocator),
     subAllocator(new HbMultiSegmentAllocator),
     chunk(0)
#ifdef HB_THEME_SERVER_MEMORY_REPORT
     ,totalAllocated(0),
     OOMCount(0),
     OOMReportCreated(false),
     ordinal(1)
#endif
{
}

HbSharedCache *HbSharedMemoryManager::createSharedCache(
    const char *dataArray, int size, int offsetItemCount, int globalParametersOffset, qptrdiff sharedCacheOffset)
{
    HbSharedCache *cache = 0;
    HbSharedChunkHeader *chunkHeader = static_cast<HbSharedChunkHeader*>(chunk->data());
    if (chunkHeader->sharedCacheOffset != 0) {
        free(chunkHeader->sharedCacheOffset);
        chunkHeader->sharedCacheOffset = 0;
    }
    if (!dataArray) {
        size = 0;
    }

    int sharedCacheSize = sizeof(HbSharedCache) + size;
    // If shared cache offset is not given as parameter, alloc it
    if (sharedCacheOffset < 0) {
        sharedCacheOffset = alloc(sharedCacheSize);
    }

    if (sharedCacheOffset >= 0) {
        cache = new (address<char>(sharedCacheOffset)) HbSharedCache();
        cache->setContent(dataArray, size, offsetItemCount, globalParametersOffset);
        chunkHeader->sharedCacheOffset = sharedCacheOffset;
    }
    return cache;
}

int HbSharedMemoryManager::size()
{
    if(mainAllocator) {
        return (static_cast<HbSplayTreeAllocator*>(mainAllocator))->size();
    }
    return -1;
}

HbSharedCache *HbSharedMemoryManager::cache()
{
    HbSharedCache *cachePtr = 0;
    if (chunk) {
        const HbSharedChunkHeader *chunkHeader =
                static_cast<const HbSharedChunkHeader*>(chunk->data());
        cachePtr = address<HbSharedCache>(chunkHeader->sharedCacheOffset);
    }
    return cachePtr;
}

/**
 * destructor
 */
HbSharedMemoryManager::~HbSharedMemoryManager()
{
#ifdef HB_THEME_SERVER_MEMORY_REPORT
    allocations.clear();
#endif
    if (chunk) {
        const HbSharedChunkHeader *chunkHeader =
                static_cast<const HbSharedChunkHeader*>(chunk->data());
        if (chunkHeader->sharedCacheOffset > 0) {
            HbSharedCache *cachePtr = address<HbSharedCache>(chunkHeader->sharedCacheOffset);
            cachePtr->freeResources();
        }
    }
    delete subAllocator;
    delete mainAllocator;
    delete chunk;
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

int HbSharedMemoryManager::loadMemoryFile(const QString &filePath)
{
    int loadedSize = 0;
    THEME_CSSBIN_DEBUG() << "loading: " << filePath;
    
    QFile file(filePath);
    if(file.open(QFile::ReadOnly)) {
        qint64 fileSize = file.size();
        file.read(static_cast<char*>(chunk->data()), fileSize);
        file.close();
        loadedSize = (int)fileSize;
    }
    THEME_CSSBIN_DEBUG() << "Loading memory file status: " << (loadedSize > 0 ? "no error" : file.errorString());
    return loadedSize;
}

#ifdef HB_THEME_SERVER_MEMORY_REPORT
bool pairGreaterThan(const QPair<quint32, QPair<quint32, quint32> > &p1,
                     const QPair<quint32, QPair<quint32, quint32> > &p2)
{
     return p1.first > p2.first;
}

void HbSharedMemoryManager::createReport()
{
#if defined(Q_OS_SYMBIAN) || defined(Q_OS_WIN)
    QString filePath("C:\\data\\log\\themeserver\\");
#else
    QString filePath(QDir::tempPath());
    filePath.append("\\data\\log\\themeserver\\");
#endif

    filePath = QDir::toNativeSeparators(filePath);

    QDir dir(filePath);
    if (!dir.exists()) {
        dir.mkpath(filePath);
    }

    filePath.append(QString::number(QDate::currentDate().year()));
    filePath.append("wk");
    filePath.append(QString::number(QDate::currentDate().weekNumber()));
#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
    filePath.append("_");
    filePath.append(QString::number(ordinal));
    ordinal++;
#endif
    filePath.append(".txt");

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Append | QFile::Text)) {
        hbWarning() << "Create shared memory report error - can't write to file " << filePath;
        return;
    }

    QTextStream reportWriter(&file);

    reportWriter << "********************************************************************************\n";
    reportWriter << "HbSharedMemoryManager: memory report\n";
    reportWriter << "********************************************************************************\n\n";

    // list for sorting allocations and frees
    QList<QPair<quint32, QPair<quint32, quint32> > > valueList;
    QMap<quint32, QPair<quint32, quint32> >::const_iterator i; // size, <allocated, freed>
    for (i = allocations.constBegin(); i != allocations.constEnd(); ++i) {
        valueList.append(QPair<quint32, QPair<quint32, quint32> >
                         (i.value().first, QPair<quint32, quint32>(i.key(), i.value().second)));
    }
    qSort(valueList.begin(), valueList.end(), pairGreaterThan);

    reportWriter << "Top 30 allocation counts:\n";
    reportWriter << "(allocation sizes for small allocations are chunk sizes from multisegment algorithm, not actual original allocation sizes)\n";
    reportWriter << "times allocated - times released - size\n";
    int count = 0;
    for (int i = 0; i < valueList.size(); i++) {
        if (count > 30) {
            break; // only report top 30 sizes
        }
        reportWriter << valueList.at(i).first << " - "
                     << valueList.at(i).second.second << " - "
                     << valueList.at(i).second.first << "\n";
        count++;
    }
    reportWriter << "\n";

    valueList.clear();
    for (i = allocations.constBegin(); i != allocations.constEnd(); ++i) {
        valueList.append(QPair<quint32, QPair<quint32, quint32> >
                         (i.key(), QPair<quint32, quint32>(i.value().first, i.value().second)));
    }
    qSort(valueList.begin(), valueList.end(), pairGreaterThan);
    reportWriter << "Top 30 allocated sizes:\n";
    reportWriter << "size - times allocated - times released\n";
    count = 0;
    for (int i = 0; i < valueList.size(); i++) {
        if (count > 30) {
            break; // only report top 30 sizes
        }
        reportWriter << valueList.at(i).first << " - "  << valueList.at(i).second.first << " - "
                     << valueList.at(i).second.second << "\n";
        count++;
    }
    reportWriter << "\n";

    mainAllocator->writeReport(reportWriter);
#ifdef USE_SUBALLOCATOR
    subAllocator->writeReport(reportWriter);
#endif

#ifdef HB_THEME_SERVER_FULL_MEMORY_REPORT
    reportWriter << "\n********************* FULL ALLOCATION HISTORY ***********************\n\n";
    for (int i = 0; i < fullAllocationHistory.size(); i++) {
        quint32 size = fullAllocationHistory.at(i).first & 0x3FFFFFFF;
        quint32 identifier = fullAllocationHistory.at(i).first & 0xC0000000;
        quint32 offset = fullAllocationHistory.at(i).second;
        switch (identifier) {
            case allocIdentifier:
                reportWriter << "allocated " << size << " bytes from offset " << offset << "\n";
                break;
            case freeIdentifier:
                reportWriter << "freed " << size << " bytes from offset " << offset << "\n";
                break;
            case reallocIdentifier:
                reportWriter << "reallocation from offset " << offset << " with "
                             << size << " bytes" << "\n";
                i++;
                if (i <= fullAllocationHistory.size()) {
                    size = fullAllocationHistory.at(i).first & 0x3FFFFFFF;
                    identifier = fullAllocationHistory.at(i).first & 0xC0000000;
                    offset = fullAllocationHistory.at(i).second;
                    if (identifier == allocIdentifier) { // should come right after realloc
                        reportWriter << "    from realloc: allocated " << size
                                     << " bytes from offset " << offset << "\n";
                    } else {
                        reportWriter << "ERROR: no alloc after realloc! How is this possible?\n";
                    }
                }
                i++;
                if (i <= fullAllocationHistory.size()) {
                    size = fullAllocationHistory.at(i).first & 0x3FFFFFFF;
                    identifier = fullAllocationHistory.at(i).first & 0xC0000000;
                    offset = fullAllocationHistory.at(i).second;
                    if (identifier == freeIdentifier) { // should come right after realloc and alloc
                        reportWriter << "    from realloc: freed " << size
                                     << " bytes from offset " << offset << "\n";
                    } else {
                        reportWriter << "ERROR: no free after realloc and alloc! How is this possible?\n";
                    }
                }
                break;
            default:
                break;
        }
    }
#endif
    file.close();
    if (file.error()) {
        hbWarning() << "Create shared memory report error - error with file " << filePath;
    }
}
#endif

/**
* helper function to know whether process is a shared container unittest
*/
#ifndef HB_BIN_CSS

#define SHARED_CONTAINER_UNITTEST_PREFIX "unittest_hbsharedcontainer_"
#define SHARED_MEMORYMANAGER_UNITTEST_PREFIX "unittest_hbsharedmemory"

static inline bool isSharedContainerUnitTest()
{
    static bool isSharedContainerUnit = false;
    // to avoid string operations multiple times in each call
    static bool appNameNotTested=true;

    if(!appNameNotTested)
        return isSharedContainerUnit;

    appNameNotTested = false;
    const QString& appName = HbMemoryUtils::getCleanAppName();

    isSharedContainerUnit = false;
    if ( appName.startsWith(SHARED_CONTAINER_UNITTEST_PREFIX) ||
         appName.startsWith(SHARED_MEMORYMANAGER_UNITTEST_PREFIX) ) {
        isSharedContainerUnit = true;
    }

    return isSharedContainerUnit;
}
#endif // HB_BIN_CSS

/**
 * Allocates and initializes a new HbSharedMemoryManager
 */
HbSharedMemoryManager* HbSharedMemoryManager::create()
{
#ifndef HB_BIN_CSS
    // Check for shared container unit test
    if ( isSharedContainerUnitTest() ) {
        return HbSharedMemoryManagerUt::create();
    }
#endif
        
    // Normal case
    return new HbSharedMemoryManager();
}
