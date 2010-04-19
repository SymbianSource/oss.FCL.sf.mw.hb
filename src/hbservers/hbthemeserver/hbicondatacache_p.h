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

#ifndef HBICONDATACACHE_P_H
#define HBICONDATACACHE_P_H

#include <QHash>
#include "hbthemeserverutils_p.h"
#include "hbiconcacheitemcreator_p.h"

class HbIconDataCache
{

public:
    HbIconDataCache();
    ~HbIconDataCache();
    void clear();
    HbIconCacheItem* getCacheItem(const HbIconKey &key ,
                                  bool isMultiIconPiece = false);
    bool insert(const HbIconKey &key,  HbIconCacheItem* item);
    bool remove(const HbIconKey&  key);
    void setMaxGpuCacheSize(int size);
    void setMaxCpuCacheSize(int size);
    bool contains(const HbIconKey &key) const;
    HbIconCacheItem* value(const HbIconKey &key) const;
    bool isItemCachableInGpu(const HbIconCacheItem* item)const;
    bool isItemCachableInCpu(const HbIconCacheItem* item)const;
    void memoryGood();
    void freeGpuRam(int bytes);

//Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    void cleanVectorLRUList();
    void cleanRasterLRUList();
    int count() const;
    int freeVectorMemory();
    int freeRasterMemory();
    int lastAddedRefCount();
    int lastAddedItemMem();
    int lastRemovedItemMem();
    int lastRemovedItemRfCount();
    bool enableCache(bool cacheIt);
    int cacheHitCount();
    int cacheMissCount();
    int rasterLruCount();
    int vectorLruCount();
    int gpuLRUSize() const;
#endif

private:

    void createGpuCacheSpace(int itemCost);
    void createCpuCacheSpace(int itemCost);
    void updateGpuLruSize(int iconDataCost);
    void updateCpuLruSize(int iconDataCost);
    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    void removeFromCache(const HbIconKey &key, const HbIconCacheItem* releaseItem);
    void releaseVectorItem(HbIconCacheItem* releaseItem);
    void releaseRasterItem(HbIconCacheItem* releaseItem);
#endif

private:
    QHash<HbIconKey, HbIconCacheItem*> *cache;
    HbDLinkList<HbIconCacheItem> gpuLruList;
    HbDLinkList<HbIconCacheItem> cpuLruList;
    int currentGpuCacheSize;
    int currentCpuCacheSize;
    int gpuLruListSize;
    int cpuLruListSize;
    int maxGpuCacheLimit;
    int maxCpuCacheLimit;
    bool goodMemory;

    //Debug Code for Test Purpose
#ifdef HB_ICON_CACHE_DEBUG
    int addedItemRefCount;
    int addedItemMem;
    int removedItemMem;
    int remRfCount;
    bool enableCaching;
    int cacheHit;
    int cacheMiss;
    int vectorLruListCount;
    int rasterLruListCount;
#endif
};
#endif // HBICONCACHE_P_H

