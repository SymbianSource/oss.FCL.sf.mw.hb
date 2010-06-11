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

#ifndef HBSHAREDCACHE_P_H
#define HBSHAREDCACHE_P_H

#include <hbstring_p.h>
#include <hbvector_p.h>

struct HbOffsetItem
{
    explicit HbOffsetItem(quint32 widgetHash = 0) :
            widgetHash(widgetHash),
            offsetCSS(-1),
            offsetColorCSS(-1),
            offsetLayoutIndexTable(-1) {}

    quint32 widgetHash;
    qint32 offsetCSS;
    qint32 offsetColorCSS;
    qint32 offsetLayoutIndexTable;
};

inline
bool operator < (const HbOffsetItem &offsetItem1,
                 const HbOffsetItem &offsetItem2)
{
    return offsetItem1.widgetHash < offsetItem2.widgetHash;
}

struct HbLayoutIndexItem
{
    explicit HbLayoutIndexItem(quint32 layoutNameHash = 0, quint32 sectionNameHash = 0) :
                          layoutNameHash(layoutNameHash),
                          sectionNameHash(sectionNameHash),
                          offset(-1) {}
    quint32 layoutNameHash;
    quint32 sectionNameHash;
    qint32 offset;
};

inline
bool operator < (const HbLayoutIndexItem &layoutIndexItem1,
                 const HbLayoutIndexItem &layoutIndexItem2)
{
    if (layoutIndexItem1.layoutNameHash < layoutIndexItem2.layoutNameHash) {
        return true;
    }
    if (layoutIndexItem1.layoutNameHash > layoutIndexItem2.layoutNameHash) {
        return false;
    }
    return (layoutIndexItem1.sectionNameHash < layoutIndexItem2.sectionNameHash);
}

class HB_CORE_PRIVATE_EXPORT HbSharedCache
{
private:
    struct CacheItem {
        HbString key;
        int offset;
        CacheItem()
           : key(HbMemoryManager::SharedMemory), offset(-1)
        {
        }
        CacheItem(const QString &cacheKey,
                  int cacheOffset )
                    : key(cacheKey, HbMemoryManager::SharedMemory), offset(cacheOffset)
        {
        }
    };
public:
    enum ItemType {
        LayoutDefinition,
        Stylesheet,
        Effect
    };
    static HbSharedCache *instance();
    static quint32 hash(const QStringRef &string);
    bool add(ItemType itemType, const QString &key, int offset);
    bool addLayoutDefinition(const QString &filePath,
                             const QString &layout,
                             const QString &section,
                             int offset);
    int layoutDefinitionOffset(const QString &filePath,
                               const QString &layout,
                               const QString &section) const;
    int offset(ItemType itemType, const QString &key) const;
    bool remove(ItemType itemType, const QString &key);
    bool removeLayoutDefinition(const QString &filePath,
                                const QString &layout,
                                const QString &section);

private:
    HbSharedCache();
    void initServer();
    void initClient();
    void freeResources();
    HbVector<CacheItem> &itemCache(ItemType type);
    const HbVector<CacheItem> &itemCache(ItemType type) const;
    void addOffsetMap(const char *offsetMapData, int size, int offsetItemCount);
    int findOffsetFromDynamicMap(ItemType itemType, const QStringRef &key) const;
    const HbLayoutIndexItem *layoutIndexItemBegin(int offset, int *size) const
    {
        return const_cast<HbSharedCache*>(this)->layoutIndexItemBegin(offset, size);
    }
    HbLayoutIndexItem *layoutIndexItemBegin(int offset, int *size);
    static QString layoutDefinitionKey(const QString &filePath,
                                       const QString &layout,
                                       const QString &section);

    friend class HbSharedMemoryManager;
    friend class HbSharedMemoryManagerUt;
private:
    friend bool testCss();

    HbVector<CacheItem> mLayoutDefCache;
    HbVector<CacheItem> mStylesheetCache;
    HbVector<CacheItem> mEffectCache;
    int mOffsetItemCount;
    HbOffsetItem mOffsetItems[1]; //actual size of array is mOffsetItemCount
};

#endif // HBSHAREDCACHE_P_H 
