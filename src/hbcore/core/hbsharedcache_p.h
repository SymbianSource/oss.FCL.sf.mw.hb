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
    HbOffsetItem() : nameOffset(-1), offset(-1) {}
    HbOffsetItem(int nameOffset, int offset)
        : nameOffset(nameOffset), offset(offset) {}
    int nameOffset;
    int offset;
};

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
    bool add(ItemType itemType, const QString &key, int offset);
    int offset(ItemType itemType, const QString &key) const;
    bool remove(ItemType itemType, const QString &key);

private:
    HbSharedCache();
    void initServer();
    void initClient();
    void freeResources();
    HbVector<CacheItem> &itemCache(ItemType type);
    const HbVector<CacheItem> &itemCache(ItemType type) const;
    void addOffsetMap(const char *offsetMapData, int size, int offsetItemCount);
    friend class HbSharedMemoryManager;
    friend class HbSharedMemoryManagerUt;
private:
    friend bool writeCssBinary(const QStringList &, const QString &);

    HbVector<CacheItem> mLayoutDefCache;
    HbVector<CacheItem> mStylesheetCache;
    HbVector<CacheItem> mEffectCache;
    int mOffsetItemCount;
    HbOffsetItem mOffsetItems[1]; //actual size or array is mOffsetItemCount
};

#endif // HBSHAREDCACHE_P_H 
