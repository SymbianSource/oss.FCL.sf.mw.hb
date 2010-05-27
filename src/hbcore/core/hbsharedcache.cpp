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

#include <QSystemSemaphore>

static QSystemSemaphore *Semaphore = 0;

static QLatin1String SemaphoreName("hbsharedcache_semaphore");

#include "hbsharedcache_p.h"
#include "hbsharedmemorymanager_p.h"

#ifdef HB_BIN_CSS
#include "hbcssconverterutils_p.h"
#endif

class HbCacheLocker
{
public:
    HbCacheLocker(QSystemSemaphore &semaphore) : semaphore(semaphore)
    {
        semaphore.acquire();
    }
    ~HbCacheLocker()
    {
        semaphore.release();
    }
    QSystemSemaphore &semaphore;
};

const HbOffsetItem *binaryFind(const QStringRef &key, const HbOffsetItem *itemArray, int count)
{
    int begin = 0;
    int end = count - 1;

    // binary search
    while (begin <= end) {
        int mid = begin + (end - begin) / 2;
        // Fast string comparison, no unnecessary mem copy
        QLatin1String offsetName(reinterpret_cast<const char*>(itemArray)
                                 + itemArray[mid].nameOffset);
        int comparison = key.compare(offsetName);
        // If the item was found, we're done.
        if (!comparison) {
            return &itemArray[mid];
        } else if (comparison < 0) {
            end = mid - 1;
        } else {
            begin = mid + 1;
        }
    }

    // Did not find the target, return 0.
    return 0;
}

HbSharedCache *HbSharedCache::instance()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    HbSharedCache *ptr = 0;
    if (manager) {
        ptr = static_cast<HbSharedMemoryManager*>(manager)->cache();
    }
    return ptr;
}

//doesn't check, if the item is already in the cache.
bool HbSharedCache::add(ItemType itemType, const QString &key, int offset)
{
    bool added = false;
    if (offset >= 0) {
        try {
            CacheItem cacheItem(key, offset);
            HbVector<CacheItem> &vector = itemCache(itemType);
            HbCacheLocker locker(*Semaphore);
            vector.append(cacheItem);
            added = true;
        } catch (std::exception &) {

        }
    }
    return added;
}

int HbSharedCache::offset(ItemType itemType, const QString &key) const
{
    int offset = -1;
    if (itemType == Stylesheet && mOffsetItemCount > 0) {
        //try first in offset map.
        QStringRef cacheKey(&key);
        if (key.at(0) == ':') {
            //resource css use only filename as a key.
            int index = key.lastIndexOf('/');
            if (index >= 0) {
                cacheKey = key.rightRef((key.size() - 1) - index);
            }
        }
        const HbOffsetItem *offsetItem = binaryFind(cacheKey, mOffsetItems, mOffsetItemCount);
        if (offsetItem) {
#ifdef CSSBIN_TRACES
            qDebug() << "Css offset found from static map for key: " << cacheKey;
#endif
            offset = offsetItem->offset;
            return offset;
        }
    }
    const HbVector<CacheItem> &cacheVector = itemCache(itemType);
    HbCacheLocker locker(*Semaphore);
    Q_FOREACH(const CacheItem &item, cacheVector) {
        if (item.key == key) {
            offset = item.offset;
            break;
        }
    }
    return offset;
}

bool HbSharedCache::remove(ItemType itemType, const QString &key)
{
    bool removed = false;

    HbVector<CacheItem> &cacheVector = itemCache(itemType);
    HbCacheLocker locker(*Semaphore);
    int count = cacheVector.count();
    for (int i = 0; i < count; ++i) {
        if (cacheVector.at(i).key == key) {
            cacheVector.remove(i);
            removed = true;
            break;
        }
    }
    return removed;
}

void HbSharedCache::addOffsetMap(const char *offsetMapData, int size, int offsetItemCount)
{
    if (offsetMapData) {
        memcpy(mOffsetItems, offsetMapData, size);
        mOffsetItemCount = offsetItemCount;
#ifdef HB_BIN_CSS
        for (int i=0; i<offsetItemCount; ++i) {
            HbCssConverterUtils::registerOffsetHolder(&(mOffsetItems[i].offset));
        }
#endif
    } else {
        mOffsetItemCount = 0;
    }
}

HbSharedCache::HbSharedCache()
    : mLayoutDefCache(HbMemoryManager::SharedMemory),
      mStylesheetCache(HbMemoryManager::SharedMemory),
      mEffectCache(HbMemoryManager::SharedMemory)
{
}

void HbSharedCache::freeResources()
{
    delete Semaphore;
    Semaphore = 0;
}

void HbSharedCache::initServer()
{
    mLayoutDefCache.reserve(20);
    mEffectCache.reserve(20);

    //server creates the semaphore.
    Semaphore = new QSystemSemaphore(SemaphoreName, 1, QSystemSemaphore::Create);
#ifdef CSSBIN_TRACES
    qDebug() << "css offsets total: " << mOffsetItemCount;
#endif
}

void HbSharedCache::initClient()
{
    //client opens the semaphore created by the server.
    Semaphore = new QSystemSemaphore(SemaphoreName, 1, QSystemSemaphore::Open);
}

HbVector<HbSharedCache::CacheItem> &HbSharedCache::itemCache(ItemType type)
{
    const HbVector<CacheItem> &items = const_cast<const HbSharedCache*>(this)->itemCache(type);
    return const_cast<HbVector<CacheItem>&>(items);
}

const HbVector<HbSharedCache::CacheItem> &HbSharedCache::itemCache(ItemType type) const
{
    const HbVector<CacheItem> *items = 0;
    switch(type) {
    case LayoutDefinition:
        items = &mLayoutDefCache;
        break;
    case Effect:
        items = &mEffectCache;
        break;
    case Stylesheet:
        items = &mStylesheetCache;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return *items;
}

