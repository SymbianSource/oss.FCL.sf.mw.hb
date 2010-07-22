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

static const QString ColorCSSEnding = "_color.css";
static const QString CSSFileExtension = ".css";
static const QString WidgetMLFileExtension = ".widgetml";
static const QChar KeySeparator('\0');

#include "hbsharedcache_p.h"
#include "hbsharedmemorymanager_p.h"

#ifdef HB_BIN_CSS
#include "hbcssconverterutils_p.h"
#endif

/*!
    Helper class for locking the cache.
*/
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

/*!
    return the singleton instance.
*/
HbSharedCache *HbSharedCache::instance()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    HbSharedCache *ptr = 0;
    if (manager) {
        ptr = static_cast<HbSharedMemoryManager*>(manager)->cache();
    }
    return ptr;
}

/*!
    return hash value for \a string.
*/
quint32 HbSharedCache::hash(const QStringRef &string)
{
    quint32 hashValue = 0;
    const QChar *data = string.unicode();
    int size = string.size();
    while (size--) {
        hashValue = data->unicode() + (hashValue << 6) + (hashValue << 16) - hashValue;
        ++data;
    }
    return hashValue;
}

/*!
    adds \a offset using key \a key. \a itemType is the type of the structure,
    where offset points to.
    Only acceptable separator for key is '/'.

    doesn't check, if the item is already in the cache.
*/
bool HbSharedCache::add(ItemType itemType, const QString &key, int offset)
{
    bool added = false;
    if (offset >= 0) {
        QString cacheKey(key);
        if (cacheKey.at(0) == ':') {
            //use only filename as a key.
            int index = cacheKey.lastIndexOf('/');
            if (index >= 0) {
                cacheKey = cacheKey.right((cacheKey.size() - 1) - index);
            }
        }
        try {
            CacheItem cacheItem(cacheKey, offset);
            HbVector<CacheItem> &vector = itemCache(itemType);
            HbCacheLocker locker(*Semaphore);
            vector.append(cacheItem);
            added = true;
        } catch (std::exception &) {

        }
    }
    return added;
}

/*!
    adds layout definition \a offset for a path \a filePath, \a layout, \a section.

    doesn't check, if the item is already in the cache.
    Only acceptable separator for filePath is '/'.
*/
bool HbSharedCache::addLayoutDefinition(const QString &filePath,
                                        const QString &layout,
                                        const QString &section,
                                        int offset)
{
    return add(LayoutDefinition, layoutDefinitionKey(filePath, layout, section), offset);
}

/*!
    return layout offset for a \a filePath, \a layout, \a section.

    Only acceptable separator for filePath is '/'.
*/
int HbSharedCache::layoutDefinitionOffset(const QString &filePath,
                                          const QString &layout,
                                          const QString &section) const
{
    int offset = -1;
    int position = 0;
    int length = filePath.length();
    if (filePath.at(0) == ':') {
        //resource css use only file name as a key.
        int index = filePath.lastIndexOf('/');
        if (index >= 0) {
            position = index + 1;
            length = filePath.length() - position;
        }
    }
    if (mOffsetItemCount > 0) {
        //try first in prebuilt offset map.
        length -= WidgetMLFileExtension.length();
        QStringRef widgetname(&filePath, position, length);
        HbOffsetItem find(hash(widgetname));
        const HbOffsetItem *end = mOffsetItems + mOffsetItemCount;
        const HbOffsetItem *offsetItem = qBinaryFind(mOffsetItems, end, find);
        if (offsetItem != end) {
#ifdef CSSBIN_TRACES
            qDebug() << "Offset item found from static cache map for widget: " << widgetname;
#endif
            int tableSize = 0;
            const HbLayoutIndexItem *begin = layoutIndexItemBegin(
                    offsetItem->offsetLayoutIndexTable, &tableSize);
            if (begin) {
                const HbLayoutIndexItem *end = begin + tableSize;
                HbLayoutIndexItem find(hash(QStringRef(&layout)), hash(QStringRef(&section)));
                const HbLayoutIndexItem *item = qBinaryFind(begin, end, find);
                if (item != end) {
#ifdef CSSBIN_TRACES
                    qDebug() << "Layout definition offset found for layout: " << layout;
#endif
                    offset = item->offset;
                }
            }
        }
    }
    if (offset == -1) {
        QStringRef filePathRef(&filePath, position, filePath.length() - position);
        QString key;
        key.reserve(filePathRef.length() + 2 //key separators
                    + layout.length() + section.length());
        key.append(filePathRef)
           .append(KeySeparator)
           .append(layout)
           .append(KeySeparator)
           .append(section);
        offset = findOffsetFromDynamicMap(LayoutDefinition, QStringRef(&key));
    }
    return offset;
}

/*!
    return offset for a key \a key. \a itemType is the type of the structure,
    where offset points to.

    Only acceptable separator for key is '/'.
*/
int HbSharedCache::offset(ItemType itemType, const QString &key) const
{
    int offset = -1;
    int position = 0;
    int length = key.length();
    if (key.at(0) == ':') {
        //resource css use only filename as a key.
        int index = key.lastIndexOf('/');
        if (index >= 0) {
            position = index + 1;
            length = key.length() - position;
        }
    }
    if (itemType == Stylesheet && mOffsetItemCount > 0) {
        //try first in prebuilt offset map.
        int isColorCSS = false;
        if (key.endsWith(ColorCSSEnding)) {
            length -= ColorCSSEnding.length();
            isColorCSS = true;
        } else {
            length -= CSSFileExtension.length();
        }
        QStringRef widgetname(&key, position, length);
        HbOffsetItem find(hash(widgetname));
        const HbOffsetItem *end = mOffsetItems + mOffsetItemCount;
        const HbOffsetItem *offsetItem = qBinaryFind(mOffsetItems, end, find);
        if (offsetItem != end) {
#ifdef CSSBIN_TRACES
            qDebug() << "Offset item found from static cache map for widget: " << widgetname;
#endif
            offset = (isColorCSS) ? offsetItem->offsetColorCSS : offsetItem->offsetCSS;
        }
    }
    if (offset == -1) {
        QStringRef keyRef(&key, position, key.length() - position);
        offset = findOffsetFromDynamicMap(itemType, keyRef);
    }
    return offset;
}

/*!
    remove offset for a key \a key.  \a itemType is the type of the structure,
    where offset points to.
*/
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

/*!
    remove layout definition offset.
*/
bool HbSharedCache::removeLayoutDefinition(const QString &filePath,
                                           const QString &layout,
                                           const QString &section)
{
    return remove(LayoutDefinition, layoutDefinitionKey(filePath, layout, section));
}

/*!
    add static offset map.
*/
void HbSharedCache::addOffsetMap(const char *offsetMapData, int size, int offsetItemCount)
{
    if (offsetMapData) {
        memcpy(mOffsetItems, offsetMapData, size);
        mOffsetItemCount = offsetItemCount;
#ifdef HB_BIN_CSS
        for (int i = 0; i < offsetItemCount; ++i) {
            HbCssConverterUtils::registerOffsetHolder(&(mOffsetItems[i].offsetCSS));
            HbCssConverterUtils::registerOffsetHolder(&(mOffsetItems[i].offsetColorCSS));
            int size = 0;
            HbLayoutIndexItem *layoutItem = layoutIndexItemBegin(
                    mOffsetItems[i].offsetLayoutIndexTable, &size);
            for(;size > 0; --size, ++layoutItem) {
                HbCssConverterUtils::registerOffsetHolder(&(layoutItem->offset));
            }
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

int HbSharedCache::findOffsetFromDynamicMap(ItemType itemType, const QStringRef &key) const
{
    int offset = -1;
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

/*!
    return the first layoutindextitem and size in offset \a offset.
    \a offset is a value in HbOffsetItem::offsetLayoutIndexTable.
*/
HbLayoutIndexItem *HbSharedCache::layoutIndexItemBegin(int offset, int *size)
{
    HbLayoutIndexItem *begin = 0;
    *size = 0;
    if (offset >= 0) {
        void *layoutIndexBase = mOffsetItems;
        int *sizePtr = reinterpret_cast<int *>(
                       static_cast<char *>(layoutIndexBase) + offset);
        *size = *sizePtr;
        begin = reinterpret_cast<HbLayoutIndexItem *>(sizePtr + 1);
    }
    return begin;
}

/*!
    build a layoutdefinition key.
*/
QString HbSharedCache::layoutDefinitionKey(const QString &filePath,
                                           const QString &layout,
                                           const QString &section)
{
    QStringRef nameKey(&filePath);
    if (nameKey.at(0) == ':') {
        //use only filename as a key.
        int index = filePath.lastIndexOf('/');
        if (index >= 0) {
            nameKey = filePath.rightRef((filePath.size() - 1) - index);
        }
    }
    QString key;
    key.append(nameKey) += KeySeparator + layout + KeySeparator + section;
    return key;
}

void HbSharedCache::freeResources()
{
    delete Semaphore;
    Semaphore = 0;
}

/*!
    themeserver initialization function.
*/
void HbSharedCache::initServer()
{
    mEffectCache.reserve(20);

    //server creates the semaphore.
    Semaphore = new QSystemSemaphore(SemaphoreName, 1, QSystemSemaphore::Create);
#ifdef CSSBIN_TRACES
    qDebug() << "css offset items total: " << mOffsetItemCount;
#endif
}

/*!
    client side initialization function.
*/
void HbSharedCache::initClient()
{
    //client opens the semaphore created by the server.
    Semaphore = new QSystemSemaphore(SemaphoreName, 1, QSystemSemaphore::Open);
}

/*!
    return the cache for a cache item type.
*/
HbVector<HbSharedCache::CacheItem> &HbSharedCache::itemCache(ItemType type)
{
    const HbVector<CacheItem> &items = const_cast<const HbSharedCache*>(this)->itemCache(type);
    return const_cast<HbVector<CacheItem>&>(items);
}

/*!
    return the cache for a cache item type.
*/
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
