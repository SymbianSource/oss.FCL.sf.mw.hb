/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbicondatacache_p.h"
#include "hbdoublelinkedlist_p.h"
#include "hbmemoryutils_p.h"
#ifdef HB_SGIMAGE_ICON
#include <sgresource/sgimage.h>
#include <sgresource/sgresource.h>
#include "hbsgimageiconprocessor_p.h"
#endif

/*!
    @hbserver
    \class HbIconDataCache
    \brief HbIconDataCache provides an implementation for the theme server's icon cache.
    It acts as a central repository for storing the various details of all the icons cached in the server.
    It provides various methods to insert new items, remove items as well as find existing cached items in the cache.
    It also has methods to limit the cache size both on Gpu as well as Cpu shared memory.

    Reference count based caching - On performing an Icon look up in the server, if the icon is not already cached,
    the cache item is created and inserted into cache and its reference count is incremented.
    If the same icon is requested by another application, the cached instance is returned and only the reference
    count is incremented. Similarly, the refrence count is decremented whenever an icon is destroyed.

    LRU policy is used for removal of icons from cache.
    The cache maintains two separate doubly link lists to maintain the order of the least recently used icons created
    in GPU memory as well as those created in the shared memory. Whenever, the reference count for a cached item becomes 0,
    it is not deleted from the cache rather; the cached icon instance is appended to the LRU list.
    Consider now that a scenario arrives when the cache has reached its max limit and there are some unused icons
    (i.e. icons with reference count = 0) in the LRU list. Suppose at this point there is a new icon caching request.
    In such a scenario, the unused icons, starting with those at the beginning of the LRU lists are removed from the cache,
    one after the other, till the new icon can be accommodated.

    Description of data members
    // A list that maintains an ordered collection of least recently used icons in GPU
    // which are not being referred to anymore( i.e icons with reference count = 0)
    HbDLinkList gpuLruList;
    // A list that maintains an ordered collection of least recently used icons in CPU
    // which are not being referred to anymore( i.e icons with reference count = 0)
    HbDLinkList cpuLruList;
    // Consolidated size of the GPU Cache being occupied by Icons
    int currentGpuCacheSize;
    // Consolidated size of the CPU Cache being occupied by Icons
    int currentCpuCacheSize;
    // Consolidated size of all the icons with reference count = 0 in GPU Cache
    int gpuLruListSize;
    // Consolidated size of all the icons with reference count = 0 in CPU Cache
    int cpuLruListSize;
    // Maximum GPU Cache Limit size
    int maxGpuCacheLimit;
    // Maximum CPU Cache Limit size
    int maxCpuCacheLimit;

*/


/*!
    \fn HbIconDataCache::HbIconDataCache()
    Constructor
 */
HbIconDataCache::HbIconDataCache()
        : gpuLruList(&HbIconCacheItem::gpuLink),
        cpuLruList(&HbIconCacheItem::cpuLink),
        currentGpuCacheSize(0),
        currentCpuCacheSize(0),
        gpuLruListSize(0),
        cpuLruListSize(0),
        maxGpuCacheLimit(0),
        maxCpuCacheLimit(0),
        goodMemory(true)
{
    cache = new QHash<HbIconKey, HbIconCacheItem*>();

    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    addedItemRefCount = 0;
    addedItemMem = 0;
    removedItemMem = 0;
    enableCaching = true;
    cacheHit = 0;
    cacheMiss = 0;
    vectorLruListCount = 0;
    rasterLruListCount = 0;
    remRfCount = 0;
#endif
}

/*!
    \fn HbIconDataCache::~HbIconDataCache()
    Destructor
 */
HbIconDataCache::~HbIconDataCache()
{
    clear();

    delete cache;
}


/*!
    \fn HbIconDataCache::clear()
    Clears the complete cache. Also clears the LRU lists.

 */
void HbIconDataCache::clear()
{
    currentGpuCacheSize = 0;
    currentCpuCacheSize = 0;
    gpuLruListSize = 0;
    cpuLruListSize = 0;

    gpuLruList.removeAll();
    cpuLruList.removeAll();

    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    QHash<HbIconKey, HbIconCacheItem*>::const_iterator itEnd(cache->constEnd());
    for (QHash < HbIconKey,
            HbIconCacheItem* >::const_iterator iter = cache->constBegin();
            iter != itEnd;
            ++iter) {
        HbIconCacheItem* temp = iter.value();
        if (temp->rasterIconData.type != INVALID_FORMAT) {
            switch (temp->rasterIconData.type) {
            case PIC :
                manager->free(temp->rasterIconData.picData.offset);
                break;
            case NVG :
                manager->free(temp->rasterIconData.nvgData.offset);
                break;
            case OTHER_SUPPORTED_FORMATS :
                manager->free(temp->rasterIconData.pixmapData.offset);
                break;
            case SGIMAGE:
#ifdef HB_SGIMAGE_ICON
                HbSgImageRenderer::removeSgImageFromHash(temp->rasterIconData.sgImageData.id);
#endif
                break;
            default:
                break;
            }
        }
        if (temp->vectorIconData.type != INVALID_FORMAT) {
            switch (temp->vectorIconData.type) {
            case PIC :
                manager->free(temp->vectorIconData.picData.offset);
                break;
            case NVG :
                manager->free(temp->vectorIconData.nvgData.offset);
                break;
            case OTHER_SUPPORTED_FORMATS :
                manager->free(temp->vectorIconData.pixmapData.offset);
                break;
            default:
                break;
            }
        }
        if (temp->blobIconData.type == BLOB) {
            manager->free(temp->blobIconData.blobData.offset);
        }
        delete   iter.value();
    }
    cache->clear();

    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    addedItemRefCount = 0;
    addedItemMem = 0;
    removedItemMem = 0;
    remRfCount = 0;
    enableCaching = true;
    cacheHit = 0;
    cacheMiss = 0;
    vectorLruListCount = 0;
    rasterLruListCount = 0;
#endif

}

/*!
    \fn HbIconDataCache::getCacheItem()
    Provides a mechanism for finidng whether a cache item is present in the cache.
    If found, returns the cache item and increments the reference count else returns NULL.
    \a key denotes the unique identifier for the cache item that is to be searched in the cache.

 */
HbIconCacheItem* HbIconDataCache::getCacheItem(const HbIconKey &key ,
        HbRenderingMode currentRenderingMode,
        bool isMultiIconPiece)
{
    HbIconCacheItem* item = 0;

    if (!cache->contains(key)) {
        return 0;
    }
    // Get the cache item associated with the key
    item = (*cache)[(key)];

//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    addedItemMem = item->rasterIconDataCost;
    cacheHit++;
#endif

    // If the Icon is present in GPU LRU list, then remove it from the list
    if (((item->gpuLink.next() != 0) || (item->gpuLink.prev() != 0)) ||
            ((item == gpuLruList.front()) && (item == gpuLruList.back()))) {
        gpuLruList.removeNode(item);
        updateGpuLruSize(-item->rasterIconDataCost);
        if (gpuLruListSize < 0) {
            gpuLruListSize = 0;
        }

        //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
        addedItemMem = item->rasterIconDataCost;
        rasterLruListCount--;
        if (rasterLruListCount < 0) {
            rasterLruListCount = 0;
        }
#endif
    }

    // If the Icon does not have GPU shared data and there is enough space to cache
    // the icon in GPU cache now, we go ahead and create GPU shared data
    if ((item->rasterIconData.type == INVALID_FORMAT) &&
            (goodMemory && !isMultiIconPiece)) {
        if (item->vectorIconData.type == NVG) {

            HbIconCacheItemCreator::createCacheItem(*item, key, currentRenderingMode);
            if (item->rasterIconData.type != INVALID_FORMAT) {
                currentGpuCacheSize += item->rasterIconDataCost;
            }
        }

//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
        addedItemMem = item->rasterIconDataCost;
#endif
    }

    // If the Icon is present in CPU LRU list, then remove it from the list
    if (((item->cpuLink.next() != 0) || (item->cpuLink.prev() != 0)) ||
            ((item == cpuLruList.front()) && (item == cpuLruList.back()))) {
        cpuLruList.removeNode(item);
        if (item->rasterIconData.type == OTHER_SUPPORTED_FORMATS) {
            updateCpuLruSize(-item->rasterIconDataCost);
        } else {
            updateCpuLruSize(-item->vectorIconDataCost);
        }
                 
        if (cpuLruListSize < 0){
            cpuLruListSize = 0;
        }

        //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
        addedItemMem = item->vectorIconDataCost;
        vectorLruListCount--;
        if (vectorLruListCount < 0) {
            vectorLruListCount = 0;
        }
#endif
    }

    // If the Icon does not have CPU data and there is enough space to create
    // the icon in CPU cache now, we go ahead and create CPU shared data
    if ((item->vectorIconData.type == INVALID_FORMAT) &&
            (item->rasterIconData.type == SGIMAGE)) {

        if ((item->vectorIconDataCost < (maxCpuCacheLimit - currentCpuCacheSize))) {
            HbIconCacheItemCreator::createCacheItem(*item, key, currentRenderingMode);
            if (item->vectorIconData.type != INVALID_FORMAT) {
                currentCpuCacheSize += item->vectorIconDataCost;
            }
        }
    }
    item->refCount ++;

    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    addedItemRefCount = item->refCount;
    qDebug() << "HbIconDataCache::getCacheItem: " << "Cache hit in Server-Cache for" << key.filename;
    qDebug() << "HbIconDataCache::getCacheItem: Server RefCount now = " << item->refCount;
#endif

    return item;
}

/*!
    \fn HbIconDataCache::insert()
    Provides a mechanism for inserting items into the cache.
    Checks are first done to see whether item can be accomodated in the GPU memory.
    If so the Gpu limits are updated. Next it tries to cache the item in the Cpu.
    If possible, the Cpu limits are updated. If niether is possible, a failure to insert is returned.
    In case of success, the item is inserted into the cache and the reference count for the item is incremented by 1.
    \a key denotes the unique identifier for the cache item that is to be inserted into the cache.
    \a item the cache item that is to be inserted into the cache.

 */
bool HbIconDataCache::insert(const HbIconKey &key, HbIconCacheItem* item)
{
    if (!item)
        return false;

    // Check if Item can be accomdated in GPU cache
    bool gpuCaching = isItemCachableInGpu(item);
    // Check if Item can be accomdated in CPU cache
    bool cpuCaching = isItemCachableInCpu(item);

    // Item cannot be inserted either into GPU cache memory or CPU cache memory
    if ((!gpuCaching) && (!cpuCaching)) {
        return false;
    }

    // Item can be accomdated in GPU cache
    if (gpuCaching) {
        // Increment the GPU cache size
        if( item->rasterIconDataCost <=  maxGpuCacheLimit ) {
            currentGpuCacheSize += item->rasterIconDataCost;
        } 
    }

    // Item can be accomdated in CPU cache
    if (cpuCaching) {
        if (item->rasterIconData.type == OTHER_SUPPORTED_FORMATS) {
            if (item->rasterIconDataCost <= (maxCpuCacheLimit - currentCpuCacheSize)) {
                currentCpuCacheSize += item->rasterIconDataCost;
            } else {
                createCpuCacheSpace(item->rasterIconDataCost);
                currentCpuCacheSize += item->rasterIconDataCost;
            }
        }
        if (item->vectorIconData.type != INVALID_FORMAT) {
            // Increment the CPU cache size
            if (item->vectorIconDataCost <= (maxCpuCacheLimit - currentCpuCacheSize)) {
                currentCpuCacheSize += item->vectorIconDataCost;
            } else {
                // New item's icon data cost is more than available free CPU cahe size
                // Check if some items, whose ref count is 0, can be removed to make way for new item
                createCpuCacheSpace(item->vectorIconDataCost);
                currentCpuCacheSize += item->vectorIconDataCost;
            }
        }

        if (currentCpuCacheSize > maxCpuCacheLimit) {
            currentCpuCacheSize = maxCpuCacheLimit;
        }
    }
    QHash<HbIconKey, HbIconCacheItem*>::iterator iter = cache->insert(key, const_cast<HbIconCacheItem*>(item));
    if (iter == cache->end()) {
        return false;
    }

    item->refCount ++;

    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    cacheMiss++;
    addedItemRefCount = item->refCount;
    if (gpuCaching) {
        addedItemMem = item->rasterIconDataCost;
    } else if (cpuCaching) {
        addedItemMem = item->vectorIconDataCost;
    }
    qDebug() << "HbIconDataCache::insert: " << "Item " << key.filename<<" inserted in Server-Cache";
    qDebug() << "HbIconDataCache::insert: Server RefCount now = " << item->refCount;
#endif

    return true;
}

/*!
    \fn HbIconDataCache::remove()
    Remove provides a mechanism for decrementing the reference count of a cached item.
    In case the reference count becomes 0, the cache item instance is appended to the corresponding LRU list.
    Actual removal of the cache item from the cache only occurs when the cache has reached a max limit and a new request for
    insert comes.
    \a key denotes the unique identifier for the cache item whose ref count is to be decremented in the cache.

 */
bool HbIconDataCache::remove(const HbIconKey& key, bool keepInCache)
{
    if (key.filename.isEmpty() || !cache->contains(key)) {
        return false;
    }
    HbIconCacheItem* item = (*cache)[(key)];
    item->refCount--;

    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    remRfCount = item->refCount;
#endif

    if (item->refCount == 0) {
        if (item->rasterIconData.type == SGIMAGE) {
            if (keepInCache) {
                gpuLruList.insertBack(item);
                updateGpuLruSize(item->rasterIconDataCost);
            } else {
                releaseRasterItem(item);
                removeFromCache(key, item);
                return true;
            }
        }

        if (item->rasterIconData.type == OTHER_SUPPORTED_FORMATS) {
            if (keepInCache) {
                cpuLruList.insertBack(item);
                updateCpuLruSize(item->rasterIconDataCost);
            } else {
                releaseRasterItem(item);
                removeFromCache(key, item);
                return true;
            }
        }


        //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
        if (! enableCaching) {
            currentGpuCacheSize -= item->rasterIconDataCost;
            removedItemMem = item->rasterIconDataCost;
            removeFromCache(key, item);
            rasterLruListCount--;
            if (rasterLruListCount < 0) {
                rasterLruListCount = 0;
            }

            if (currentGpuCacheSize < 0) {
                currentGpuCacheSize = 0;
            }
        } else {
#endif


            //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
            rasterLruListCount++;
        }
#endif


        if ((item->vectorIconData.type != INVALID_FORMAT) &&  item->refCount == 0) {

            //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
            if (! enableCaching) {
                currentCpuCacheSize -= item->vectorIconDataCost;
                removedItemMem = item->vectorIconDataCost;
                removeFromCache(key, item);
                vectorLruListCount--;
                if (vectorLruListCount < 0) {
                    vectorLruListCount = 0;
                }
                if (currentCpuCacheSize < 0) {
                    currentCpuCacheSize = 0;
                }
            } else {
#endif
                if (keepInCache) {
                    cpuLruList.insertBack(item);
                    updateCpuLruSize(item->vectorIconDataCost);
                } else {
                    releaseVectorItem(item);
                    removeFromCache(key, item);
                    return true;
                }
                //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
                vectorLruListCount++;
            }
#endif
        }
    }
    return true;
}

/*!
    \fn HbIconDataCache::setMaxGpuCacheSize()
    Provides an internal mechanism for setting the Gpu cache limit
    \a size denotes the cache limit in bytes e.g. size = 0x500000

 */
void HbIconDataCache::setMaxGpuCacheSize(int size)
{
    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    if (maxGpuCacheLimit != size) {
        if (maxGpuCacheLimit > size) {
            // remove the item with refcount = 0 in the cache
            // i.e all the item in both LRU lists.
            cleanRasterLRUList();
            if (size <= currentGpuCacheSize) {
                maxGpuCacheLimit = currentGpuCacheSize;
            } else {
                maxGpuCacheLimit = size;
            }
        } else {
            maxGpuCacheLimit = size;
        }
    }
#else
    maxGpuCacheLimit = size;
#endif

}

/*!
    \fn HbIconDataCache::setMaxCpuCacheSize()
    Provides an internal mechanism for setting the Cpu cache limit
    \a size denotes the cache limit in bytes e.g. size = 0x500000
 */
void HbIconDataCache::setMaxCpuCacheSize(int size)
{
    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    if (maxCpuCacheLimit != size) {
        if (maxCpuCacheLimit > size) {
            // remove the item with refcount = 0 in the cache
            // i.e all the item in both LRU lists.
            cleanVectorLRUList();
            if (size <= currentCpuCacheSize) {
                maxGpuCacheLimit = currentCpuCacheSize;
            } else {
                maxCpuCacheLimit = size;
            }
        } else {
            maxCpuCacheLimit = size;
        }
    }
#else
    maxCpuCacheLimit = size;
#endif
}

/*!
    \fn HbIconDataCache::contains()
    Provides a mecahnism for finding whether an item exists in cache.
    Is different from the find function in that, this function simply checks whether an item is presentin cache, whereas find
    also performs additional operations such as incrementing the reference count.
    \a key denotes the unique identifier for the cache item that is to be found into the cache.

 */
bool HbIconDataCache::contains(const HbIconKey &key)const
{
    return (cache->contains(key));
}

/*!
    \fn HbIconDataCache::value()
    Value provides a mechanism for returning the value of the cache item associated with the key
    \a key denotes the unique identifier for the cache item whose value is to be returned

 */
HbIconCacheItem* HbIconDataCache::value(const HbIconKey &key)const
{
    if (cache->contains(key)) {
        return cache->value(key);
    } else {
        return 0;
    }
}


/*!
    \fn HbIconDataCache::isItemCachableInGpu()
    Checks if the new item can be accomdated in the Gpu memory.
    \a item is the new item to be cached
  BLOB is always cached in cpu so this function always returns false for such items.
 */
bool HbIconDataCache::isItemCachableInGpu(const HbIconCacheItem* item)const
{
    if (maxGpuCacheLimit <= 0 || item->rasterIconDataCost <= 0 || item->blobIconData.type != INVALID_FORMAT ||
            item->rasterIconData.type != SGIMAGE) {
        return false;
    }
    // Item's GPU Icon's cost is greater than the max GPU Limit
    if (item->rasterIconDataCost  > maxGpuCacheLimit)
        return false;

    return true;
}

/*!
    \fn HbIconDataCache::isItemCachableInCpu()
    Checks if the new item can be accomdated in the cpu shared memory.
    \a item is the new item to be cached
  BLOB is always cached in cpu, never in gpu.
 */
bool HbIconDataCache::isItemCachableInCpu(const HbIconCacheItem* item)const
{
    if (maxCpuCacheLimit <= 0) {
        return false;
    }
    if (item->rasterIconData.type == OTHER_SUPPORTED_FORMATS) {
        if (item->rasterIconDataCost <= 0 || item->rasterIconDataCost > maxCpuCacheLimit) {
            return false;
        }
        if (item->rasterIconDataCost <= (maxCpuCacheLimit - currentCpuCacheSize)) {
            return true;
        } else {
            return (item->rasterIconDataCost <= (maxCpuCacheLimit - currentCpuCacheSize) + cpuLruListSize);
        }
    }

    if (item->vectorIconData.type != INVALID_FORMAT) {
        if (item->vectorIconDataCost <= 0 || item->vectorIconDataCost > maxCpuCacheLimit) {
            return false;
        }
        if (item->vectorIconDataCost <= (maxCpuCacheLimit - currentCpuCacheSize)) {
            return true;
        } else {
            return (item->vectorIconDataCost <= (maxCpuCacheLimit - currentCpuCacheSize) + cpuLruListSize);
        }
    }
    return false;
}

/*!
    \fn HbIconDataCache::createGpuCacheSpace()
    This method provides a way to remove the unused icons( icons with ref count =0.
    It starts removing the icons from the cache, starting with those that are at the front of the Gpu LRU list.
    It continues removal of items till there is enough space created to cache the new item in Gpu
    \a itemCost - cost of the new item to be cached in the Gpu memory

 */
void HbIconDataCache::createGpuCacheSpace(int itemCost)
{
    if (!gpuLruList.isEmpty()) {
        // Keep removing  items from the cache till there is
        // enough space to accomdate the new item
        int freedMemory = 0;
        while (itemCost > (freedMemory)) {
            HbIconCacheItem* itemToRemove = gpuLruList.removeFront();
            // Decrement the Size by the cost of the removed icon's data cost
            //GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
            //qDebug() << "HbIconDataCache : Calling SgImage Close. Cost = %d  "<< itemToRemove->rasterIconDataCost;
#ifdef HB_SGIMAGE_ICON
#ifdef HB_ICON_CACHE_DEBUG
            qDebug() << "HbIconDataCache : Calling SgImage Close. Cost = %d  "<< itemToRemove->rasterIconDataCost;
#endif
            HbSgImageRenderer::removeSgImageFromHash(itemToRemove->rasterIconData.sgImageData.id);  
#endif
            itemToRemove->rasterIconData.type = INVALID_FORMAT;
            itemToRemove->gpuLink.setNext(0);
            itemToRemove->gpuLink.setPrev(0);
            currentGpuCacheSize -= itemToRemove->rasterIconDataCost;
            updateGpuLruSize(-itemToRemove->rasterIconDataCost);
            freedMemory += itemToRemove->rasterIconDataCost;

            if (currentGpuCacheSize < 0) {
                currentGpuCacheSize = 0;
            }

            if (gpuLruListSize < 0) {
                gpuLruListSize = 0;
            }
            //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
            removedItemMem = itemToRemove->rasterIconDataCost;
            rasterLruListCount--;
            if (rasterLruListCount < 0) {
                rasterLruListCount = 0;
            }
#endif

            // This is the case where Icon has no CPU data and
            // the GPU cached data has also  been deleted to make way for new Icon
            // In such a case the Item can be removed from the Hash

            if ((itemToRemove->rasterIconData.type == INVALID_FORMAT) &&
                    (itemToRemove->vectorIconData.type == INVALID_FORMAT)) {
                cache->remove(cache->key(itemToRemove));
                delete itemToRemove;
            }
        }
    }
}

/*!
    \fn HbIconDataCache::createCpuCacheSpace()
    This method provides a way to remove the unused icons( icons with ref count =0).
    It starts removing the icons from the cache, starting with those that are at the front of the CPU LRU list.
    It continues removal of items till there is enough space created to cache the new item in Cpu
    \a itemCost - cost of the new item to be cached in the Cpu memory

 */
void HbIconDataCache::createCpuCacheSpace(int itemCost)
{
    if (!cpuLruList.isEmpty()) {
        // Keep removing  items from the cache till there is
        // enough space to accomdate the new item
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
        while (itemCost > (maxCpuCacheLimit - currentCpuCacheSize)) {
            HbIconCacheItem* itemToRemove = cpuLruList.removeFront();
            if (itemToRemove->rasterIconData.type == OTHER_SUPPORTED_FORMATS) {
                manager->free(itemToRemove->rasterIconData.pixmapData.offset);
                itemToRemove->rasterIconData.type = INVALID_FORMAT;
                currentCpuCacheSize -= itemToRemove->rasterIconDataCost;
                updateCpuLruSize(-itemToRemove->rasterIconDataCost);
            } else {
                // Decrement the Size by the cost of the removed icon's data  cost
                if((itemToRemove->vectorIconData.type == PIC)){
                    manager->free(itemToRemove->vectorIconData.picData.offset);
                } else if (itemToRemove->vectorIconData.type == NVG) {
                    manager->free(itemToRemove->vectorIconData.nvgData.offset);
                } else if (itemToRemove->blobIconData.type == BLOB) {
                    manager->free(itemToRemove->blobIconData.blobData.offset);
                }

                itemToRemove->vectorIconData.type = INVALID_FORMAT;
                currentCpuCacheSize -= itemToRemove->vectorIconDataCost;
                updateCpuLruSize(-itemToRemove->vectorIconDataCost);
            }


            itemToRemove->cpuLink.setNext(0);
            itemToRemove->cpuLink.setPrev(0);

            if (currentCpuCacheSize < 0) {
                currentCpuCacheSize = 0;
            }

            if (cpuLruListSize < 0) {
                cpuLruListSize = 0;
            }

            //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
            removedItemMem = itemToRemove->vectorIconDataCost;
            vectorLruListCount--;
            if (vectorLruListCount < 0) {
                vectorLruListCount = 0;
            }
#endif

            // This is the case where Icon has no CPU data and
            // the GPU cached data has also  been deleted to make way for new Icon
            // In such a case the Item can be removed from the Hash

            if ((itemToRemove->vectorIconData.type == INVALID_FORMAT) &&
                    (itemToRemove->rasterIconData.type == INVALID_FORMAT)) {
                cache->remove(cache->key(itemToRemove));
                delete itemToRemove;
            }
        }
    }
}

void HbIconDataCache::updateGpuLruSize(int iconDataCost)
{
    gpuLruListSize += iconDataCost;
}

void HbIconDataCache::updateCpuLruSize(int iconDataCost)
{
    cpuLruListSize += iconDataCost;
}

void HbIconDataCache::memoryGood()
{
    goodMemory = true;
}

void HbIconDataCache::freeGpuRam(int bytes)
{
    goodMemory = false;
    if (bytes  <= gpuLruListSize) {
        createGpuCacheSpace(bytes);
    } else {
        createGpuCacheSpace(gpuLruListSize);
    }
    
    // Iterate through the cache and remove any active SgImages, before the context
    // is destroyed.
    QHash<HbIconKey, HbIconCacheItem*>::const_iterator itEnd(cache->constEnd());
        for (QHash < HbIconKey,
                HbIconCacheItem* >::const_iterator iter = cache->constBegin();
                iter != itEnd;
                ++iter) {
            HbIconCacheItem* temp = iter.value();
            if( temp->rasterIconData.type == SGIMAGE ){
#ifdef HB_SGIMAGE_ICON
                HbSgImageRenderer::removeSgImageFromHash(temp->rasterIconData.sgImageData.id);
#endif
                temp->rasterIconData.type = INVALID_FORMAT;
                temp->gpuLink.setNext(0);
                temp->gpuLink.setPrev(0);
                currentGpuCacheSize -= temp->rasterIconDataCost;
            }
        }
    
}

/*!
    \fn HbIconDataCache::freeUnusedGpuResources()
    This function internally calls createGpuCacheSpace() which will free up
    all the unused sgImage icons.
 */
void HbIconDataCache::freeUnusedGpuResources()
{
    createGpuCacheSpace(gpuLruListSize);
}

QVector<const HbIconKey *> HbIconDataCache::getKeys(const QString &filename) const
{
    QVector<const HbIconKey *> keys;
    QHash<HbIconKey, HbIconCacheItem*>::const_iterator itEnd(cache->constEnd());
    for (QHash < HbIconKey,
            HbIconCacheItem* >::const_iterator iter = cache->constBegin();
            iter != itEnd;
            ++iter) {
        const HbIconKey *key = &iter.key();
        if (key->filename == filename) {
            keys.append(key);
        }
    }
    return keys;
}

//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
void HbIconDataCache::cleanVectorLRUList()
{

    // remove all the items in cpu LRU list.
    while (cpuLruList.front()) {
        HbIconCacheItem* itemToRemove = cpuLruList.removeFront();

        // update the member
        currentCpuCacheSize -= itemToRemove->vectorIconDataCost;
        cpuLruListSize -= itemToRemove->vectorIconDataCost;
        if (currentCpuCacheSize < 0) {
            currentCpuCacheSize = 0;
        }

        if (cpuLruListSize < 0) {
            cpuLruListSize = 0;
        }

#ifdef HB_ICON_CACHE_DEBUG
        removedItemMem = itemToRemove->vectorIconDataCost;
        vectorLruListCount--;
        if (vectorLruListCount < 0) {
            vectorLruListCount = 0;
        }
#endif

        // remove the shared memory allocatedfor this item.
        releaseVectorItem(itemToRemove);

        // release item from cache
        removeFromCache(cache->key(itemToRemove), itemToRemove);
    }
}
#endif // HB_ICON_CACHE_DEBUG

void HbIconDataCache::releaseVectorItem(HbIconCacheItem* releaseItem)
{
    if (!releaseItem) {
        return;
    }
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    // release the specific items  data
    if (releaseItem->vectorIconData.type == OTHER_SUPPORTED_FORMATS) {
        manager->free(releaseItem->vectorIconData.pixmapData.offset);
    } else if ((releaseItem->vectorIconData.type == PIC) ||
               (releaseItem->vectorIconData.type == SVG)) {
        manager->free(releaseItem->vectorIconData.picData.offset);
    } else if (releaseItem->vectorIconData.type == NVG) {
        manager->free(releaseItem->vectorIconData.nvgData.offset);
    } else if (releaseItem->blobIconData.type == BLOB) {
        manager->free(releaseItem->blobIconData.blobData.offset);
    }
    releaseItem->vectorIconData.type = INVALID_FORMAT;
    releaseItem->cpuLink.setNext(0);
    releaseItem->cpuLink.setPrev(0);
}

#ifdef HB_ICON_CACHE_DEBUG
void HbIconDataCache::cleanRasterLRUList()
{

    // remove all the items from the gpu LRU list
    while (gpuLruList.front()) {
        HbIconCacheItem* itemToRemove = gpuLruList.removeFront();

        // update the member
        currentGpuCacheSize -= itemToRemove->rasterIconDataCost;
        gpuLruListSize -= itemToRemove->rasterIconDataCost;

        if (currentGpuCacheSize < 0) {
            currentGpuCacheSize = 0;
        }

        if (gpuLruListSize < 0) {
            gpuLruListSize = 0;
        }
#ifdef HB_ICON_CACHE_DEBUG
        removedItemMem = itemToRemove->rasterIconDataCost;
        rasterLruListCount--;
        if (rasterLruListCount < 0) {
            rasterLruListCount = 0;
        }
#endif

        // release the shared memory (later raster memory)of this item.
        releaseRasterItem(itemToRemove);

        // relese from the cache.
        removeFromCache(cache->key(itemToRemove), itemToRemove);
    }
}
#endif // HB_ICON_CACHE_DEBUG

void HbIconDataCache::releaseRasterItem(HbIconCacheItem* releaseItem)
{
    if (!releaseItem) {
        return;
    }
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    // release the removed item data
    manager->free(releaseItem->rasterIconData.pixmapData.offset);
    releaseItem->rasterIconData.type = INVALID_FORMAT;
}

void HbIconDataCache::removeFromCache(const HbIconKey &key, const HbIconCacheItem* releaseItem)
{
    if (!releaseItem) {
        return;
    }

    if ((releaseItem->vectorIconData.type == INVALID_FORMAT) &&
            (releaseItem->rasterIconData.type == INVALID_FORMAT)) {
        cache->remove(key);
        delete releaseItem;
    }
}

#ifdef HB_ICON_CACHE_DEBUG
int HbIconDataCache::count() const
{
    return cache->count();
}

int HbIconDataCache::gpuLRUSize() const
{
    return gpuLruListSize;
}

int HbIconDataCache::freeVectorMemory()
{
    return (maxCpuCacheLimit - currentCpuCacheSize);
}

int HbIconDataCache::freeRasterMemory()
{
    return (maxGpuCacheLimit - currentGpuCacheSize);
}
int HbIconDataCache::lastAddedRefCount()
{
    return addedItemRefCount;
}
int HbIconDataCache::lastAddedItemMem()
{
    return addedItemMem;
}
int HbIconDataCache::lastRemovedItemMem()
{
    return removedItemMem;
}

int HbIconDataCache::lastRemovedItemRfCount()
{
    return remRfCount;
}

bool HbIconDataCache::enableCache(bool cacheIt)
{
    bool success = false;
    if (enableCaching != cacheIt) {
        enableCaching = cacheIt;
        if (!enableCaching) {
            cleanVectorLRUList();
            cleanRasterLRUList();
        }
        success = true;
    }
    return success;
}

int HbIconDataCache::cacheHitCount()
{
    return cacheHit;
}

int HbIconDataCache::cacheMissCount()
{
    return cacheMiss;
}

int HbIconDataCache::rasterLruCount()
{
    return gpuLruListSize;
}

int HbIconDataCache::vectorLruCount()
{
    return cpuLruListSize;
}
#endif
