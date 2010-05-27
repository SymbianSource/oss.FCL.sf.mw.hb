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

#include "hbcache_p.h"
#include "hbmemoryutils_p.h"

/*!
    \fn HbCache::HbCache()
    Ctor
 */
HbCache::HbCache()
{

}

/*!
    \fn HbCache::HbCache()
    Dtor
 */
HbCache::~HbCache()
{
    clear();
}

/*!
    \fn HbCache::value()
    Value provides a mechanism for returning the value of the cache item associated with the key
    \a key denotes the unique identifier for the cache item whose value is to be returned

 */
HbCacheItem* HbCache::value(const QString &key) const
{
    return cache.value(key, 0);
}

/*!
    \fn HbCache::cacheItem()
    Provides a mechanism for finidng whether a cache item is present in the cache.
    If found, returns the cache item and increments the reference count else returns NULL.
    \a key denotes the unique identifier for the cache item that is to be searched in the cache.

 */
HbCacheItem* HbCache::cacheItem(const QString &key)
{
    HbCacheItem *item = 0;
    if (!cache.contains(key)) {
        return 0;
    }
    // Get the cache item associated with the key
    item = cache[key];
    item->refCount++;
    //if item is also present in LRU list, remove it from there to avoid
    // deletion following LRU policy
    if (unusedResources.contains(item)) {
        unusedResources.removeAll(item);
    }
    return item;
}

/*!
    \fn HbCache::insert()
    Provides a mechanism for inserting a cache item into cache.
    If successful, this function increases item's reference count by one and returns true.
    \a key denotes the unique identifier for the cache item that is to be searched in the cache.
    \a item represents the cache-item to be inserted.
 */
bool HbCache::insert(const QString &key, HbCacheItem *item)
{
    if (!item) {
        return false;
    }
    cache.insert(key, const_cast<HbCacheItem *>(item));
    item->refCount++;
    //if item is also present in LRU list, remove it from there to avoid
    // deletion following LRU policy
    if (unusedResources.contains(item)) {
        unusedResources.removeAll(item);
    }
    return true;
}

/*!
    \fn HbCache::remove()
    Provides a mechanism for removing a cache item from cache.
    If successful, this function decreases item's reference count by one and if
    the reference count reaches zero for an item, it's pushed at the back of
    unused-resources list for removal later in case of OOM scenario.
    \a key denotes the unique identifier for the cache item that is to be searched in the cache.
 */
bool HbCache::remove(const QString &key)
{
    if (key.isEmpty() || !cache.contains(key)) {
        return false;
    }
    HbCacheItem *item = cache[key];
    //reference count can obviously be never less than zero, meaning that for all the
    //css files stored with server-css-cache, there would be minimum zero client (app)
    //associated
    if (item->refCount > 0) {
        item->refCount--;
    }
    if (item->refCount == 0) {
        //Inserting item with ref-count 0 into unused-resources list for removal policy later
        // in case of OOM condition
        unusedResources.append(item);
    }
    return true;
}

/*!
    \fn HbCache::contains()
    Returns true if an item is found in the cache corresponding to given key.
    \a key denotes the unique identifier for the cache item that is to be searched in the cache.
 */
bool HbCache::contains(const QString &key) const
{
    return (cache.contains(key));
}

/*!
    \fn HbCache::lruList()
    Returns a handle to the list of cache which holds cache-items with zero reference count.
 */
QList<HbCacheItem*> &HbCache::lruList()
{
    return unusedResources;
}

/*!
    \fn HbCache::cacheHandle()
    Returns a handle to the cache which holds (css-file-name, cacheItem) key-value pair.
 */
QHash<QString, HbCacheItem *> &HbCache::cacheHandle()
{
    return cache;
}
/*!
    \fn HbCache::clear()
    Clears the shared resources allocated by cache-items and deletes all cache-items.
 */
void HbCache::clear()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    QHash<QString, HbCacheItem*>::const_iterator itEnd(cache.constEnd());
    for (QHash<QString, HbCacheItem*>::const_iterator iter = cache.constBegin();
            iter != itEnd;
            ++iter) {
        HbCacheItem *temp = iter.value();
        manager->free(temp->offset);
        delete temp;
    }
    cache.clear();
}
