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

#include "hbiconcacheitemcreator_p.h"
#include "hbpixmapiconprocessor_p.h"
#include "hbpiciconprocessor_p.h"
#include "hbthemeserverutils_p.h"
#include "hbmemorymanager_p.h"
#include "hbmemoryutils_p.h"
#ifdef Q_OS_SYMBIAN
#include "hbthemeserver_symbian_p_p.h"
#endif
#include <QFile>
#include <QDebug>

#ifdef HB_NVG_CS_ICON
#include "hbnvgiconprocessor_p.h"
#ifdef HB_SGIMAGE_ICON
#include "hbsgimageiconprocessor_p.h"
#endif
#endif

/*!
    @hbserver
    \class HbIconCacheItemCreator
    \brief HbIconCacheItemCreator is a factory class responsible for creating the cache items.
    The cache item structure internally maintains details of the icons created both in the Gpu and the Cpu memory.

*/

/*! Description of data members for HbIconCacheItem
    //Icon data for icon created on GPU
    HbSharedIconInfo       rasterIconData;
    //Icon data for icon created on CPU
    HbSharedIconInfo       vectorIconData;
    // reference count
    int refCount;
    // linkage of HbIconCacheItems used by LRU list for GPU memory
    HBDLink<HbIconCacheItem> gpuLink;
    // linkage of HbIconCacheItems used by LRU list for CPU memory
    HBDLink<HbIconCacheItem> cpuLink;
    //size of shared raster Icon data on GPU
    int     rasterIconDataCost;
    //size of shared CPU Icon data
    int     vectorIconDataCost;
    // IconLoader options
    HbIconLoader::IconLoaderOptions iconOptions;
*/

QString HbIconCacheItemCreator::KSvg = "SVG";
QString HbIconCacheItemCreator::KNvg = "NVG";
QString HbIconCacheItemCreator::KPic = "PIC";
QString HbIconCacheItemCreator::KBlob = "BOLB";
QString HbIconCacheItemCreator::KSgimage = "SGIMAGE";


/*!
    \fn HbIconCacheItemCreator::createCacheItem()
    The createCacheItem is responsible for creating an icon in cpu or gpu memory based on the format provided
    \a key denotes the unique identifier for the cache item
    \a options indicate different ways of loading icons
    \a format indicates the icon format e.g. svg/nvg etc.\
    \a gpuMemoryStatus indicates the global GPU memory status (GoodMemory state or FreeRAM state)
 */
HbIconCacheItem* HbIconCacheItemCreator::createCacheItem(const HbIconKey &key,
        HbIconLoader::IconLoaderOptions options,
        const QString &format,
        bool isMultiPiece)
{
#ifndef Q_OS_SYMBIAN
    Q_UNUSED(isMultiPiece)
#endif
    QScopedPointer <HbIconCacheItem> tempIconCacheItem(new HbIconCacheItem);
    HbIconCacheItem* item = tempIconCacheItem.data();
    QScopedPointer <HbIconProcessor> rasterIcon;
    QScopedPointer <HbIconProcessor> vectorIcon;
    bool isIconCreated = false;

    if ((format == KSvg) || (format == KPic)) {

#ifdef HB_NVG_TLV_ICON
        rasterIcon.reset(new HbNvgIconProcessor(key, options, format));
        vectorIcon.reset(new HbNvgIconProcessor(key, options, format));
#else
        rasterIcon.reset(new HbPixmapIconProcessor(key, options, format));
#endif

#ifdef SVG_INTERMEDIATE_PIC
        vectorIcon.reset(new HbPicIconProcessor(key, options, format));
#endif
    } else if (format == KNvg) {
#ifdef HB_NVG_CS_ICON
        if (!isMultiPiece) {
#ifdef HB_SGIMAGE_ICON
            if (HbThemeServerPrivate::gpuMemoryState()) {
                rasterIcon.reset(new HbSgimageIconProcessor(key, options, format));
            }
#endif
            vectorIcon.reset(new HbNvgIconProcessor(key, options, format));
        } else {
            // multipieceIcon So make nvgiconimpl for .nvg files
            // No raster icon data is created
            vectorIcon.reset(new HbNvgIconProcessor(key, options, format));
        }
#endif
    } else if (format == KBlob) {
        item->blobIconData.type = BLOB;
        QFile f(key.filename);
        bool fail = true;
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray content = f.readAll();
            f.close();
            GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
            item->blobIconData.blobData.offset = manager->alloc(content.length());
            if (item->blobIconData.blobData.offset != -1) {
                fail = false;
                // Store vector cost because blobs go into the "cpu" cache.
                item->blobIconData.blobData.dataSize
                = item->vectorIconDataCost
                  = content.length();
                memcpy(HbMemoryUtils::getAddress<char>(
                           HbMemoryManager::SharedMemory,
                           item->blobIconData.blobData.offset),
                       content.data(),
                       content.length());
            }
        }
        if (fail) {
            return 0;
        }
    } else {
        rasterIcon.reset(new HbPixmapIconProcessor(key, options, format));
    }

    if (rasterIcon.data()) {
        isIconCreated = rasterIcon.data()->createIconData(key.filename);
        if (isIconCreated) {
            item->rasterIconData = rasterIcon.data()->sharedIconData();
            item->rasterIconDataCost = rasterIcon.data()->sharedIconDataCost();
        }
    }

    if (vectorIcon.data()) {
        isIconCreated = vectorIcon.data()->createIconData(key.filename);
        if (isIconCreated) {
            item->vectorIconData = vectorIcon.data()->sharedIconData();
            item->vectorIconDataCost = vectorIcon.data()->sharedIconDataCost();
        }
    }

    if (item) {
        item->iconOptions = options;
    }
    if (tempIconCacheItem.data()) {
        tempIconCacheItem.take();
    }
    return item;
}


/*!
    \fn HbIconCacheItemCreator::createCacheItem()
    This overloaded createCacheItem is a helper function to populate a cache item if this item is already
    created with some parameters either on the Gpu or the Cpu
    \a iconCacheItem denotes the cacheItem to be populated
    \a key unique identifier to identify the cache item

 */
void HbIconCacheItemCreator::createCacheItem(HbIconCacheItem& iconCacheItem,
        const HbIconKey &key)
{
    QScopedPointer <HbIconProcessor> rasterIcon;
    QScopedPointer <HbIconProcessor> vectorIcon;
    bool isIconCreated = false;
    QString format = HbThemeServerUtils::formatFromPath(key.filename);

    // If the item is already created on the CPU side but not on the GPU side
    if (iconCacheItem.rasterIconData.type == INVALID_FORMAT &&
            iconCacheItem.vectorIconData.type != INVALID_FORMAT) {
        if ((format == KSvg) || (format == KPic)) {
            rasterIcon.reset(new HbPixmapIconProcessor(key, iconCacheItem.iconOptions, format));
        }
        if (format == KNvg) {
#ifdef HB_SGIMAGE_ICON
            if (HbThemeServerPrivate::gpuMemoryState()) {
                rasterIcon.reset(new HbSgimageIconProcessor(key, iconCacheItem.iconOptions, format));
            }
#endif
#ifdef NVG_ICON
            rasterIcon.reset(new HbPixmapIconProcessor(key, options, format));
#endif
        }
        if (rasterIcon.data()) {
            isIconCreated = rasterIcon.data()->createIconData(key.filename);
            if (isIconCreated) {
                iconCacheItem.rasterIconData = rasterIcon.data()->sharedIconData();
                iconCacheItem.rasterIconDataCost = rasterIcon.data()->sharedIconDataCost();
            }
        }
    }

    // If the item is already created on the GPU side but not on the CPU side
    if (iconCacheItem.rasterIconData.type != INVALID_FORMAT &&
            iconCacheItem.vectorIconData.type == INVALID_FORMAT) {
        if ((format == KSvg) || (format == KPic)) {
#ifdef SVG_INTERMEDIATE_PIC
            vectorIcon.reset(new HbPicIconProcessor(key, options, format));
#endif
        }
        if (format == KNvg) {
#ifdef NVG_ICON
            vectorIcon.reset(new HbNvgIconProcessor(key, options, format));
#endif
        }
        if (vectorIcon.data()) {
            isIconCreated = vectorIcon.data()->createIconData(key.filename);
            if (isIconCreated) {
                iconCacheItem.vectorIconData = vectorIcon.data()->sharedIconData();
                iconCacheItem.vectorIconDataCost = vectorIcon.data()->sharedIconDataCost();
            }
        }
    }
}

HbIconCacheItem * HbIconCacheItemCreator::createMultiPieceCacheItem(
    const HbIconKey &finalIconKey,
    HbIconLoader::IconLoaderOptions options,
    const QString &format,
    const QVector<HbSharedIconInfo> &multiPieceIconInfo,
    HbMultiIconParams &multiPieceIconParams,
    bool allNvg)
{

    HbIconCacheItem* item = 0;
    QScopedPointer <HbIconCacheItem> tempIconCacheItem;
    bool isIconCreated = false;
    QScopedPointer<HbIconProcessor> rasterIcon;

    if (allNvg) {
#ifdef HB_SGIMAGE_ICON
        if (HbThemeServerPrivate::gpuMemoryState()) {
            rasterIcon.reset(new HbSgimageIconProcessor(
                                 finalIconKey,
                                 (HbIconLoader::IconLoaderOptions)multiPieceIconParams.options,
                                 KSgimage));
        }
#else
        return item;
#endif
    } else {
        rasterIcon.reset(new HbPixmapIconProcessor(finalIconKey, (HbIconLoader::IconLoaderOptions)multiPieceIconParams.options, format));
    }

    if (rasterIcon.data()) {
        isIconCreated = rasterIcon->createMultiPieceIconData(multiPieceIconInfo, multiPieceIconParams);
        if (isIconCreated) {
            tempIconCacheItem.reset(new HbIconCacheItem());
            item = tempIconCacheItem.data();
            item->rasterIconData = rasterIcon->sharedIconData();
            item->rasterIconDataCost = rasterIcon->sharedIconDataCost();
            item->iconOptions = options;
        }
    }
    tempIconCacheItem.take();
    return item;
}
