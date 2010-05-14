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

#ifndef HBICONCACHEITEMCREATOR_P_H
#define HBICONCACHEITEMCREATOR_P_H

#include "hbthemecommon_p.h"
#include "hbiconloader_p.h"
#include "hbiconprocessor_p.h"
#include "hbdoublelinkedlist_p.h"

struct HbIconCacheItem
{
    HbIconCacheItem()
    {
        rasterIconData.type = INVALID_FORMAT;
        vectorIconData.type = INVALID_FORMAT;
        blobIconData.type = INVALID_FORMAT;
        refCount = 0;
        rasterIconDataCost = 0;
        vectorIconDataCost = 0;
        iconOptions = (HbIconLoader::ReturnUnknownIcon | HbIconLoader::BitmapIcons |
                       HbIconLoader::VectorIcons);
    }

    ~HbIconCacheItem()
    {
    }

    HbSharedIconInfo  rasterIconData;
    HbSharedIconInfo  vectorIconData;
    HbSharedIconInfo  blobIconData;
    int refCount;
    HbDLink<HbIconCacheItem> gpuLink;
    HbDLink<HbIconCacheItem> cpuLink;
    int     rasterIconDataCost;
    int     vectorIconDataCost;
    HbIconLoader::IconLoaderOptions iconOptions;
};

class HbIconCacheItemCreator
{
public:

    static HbIconCacheItem* createCacheItem(const HbIconKey &key,
                                            HbIconLoader::IconLoaderOptions options,
                                            const QString &format,
                                            HbRenderingMode currentRenderingMode,
                                            bool isMultiPiece = false);
    static void createCacheItem(HbIconCacheItem& iconCacheItem,
                                const HbIconKey &key,
                                HbRenderingMode currentRenderingMode);

    static HbIconCacheItem * createMultiPieceCacheItem(const HbIconKey &key,
            HbIconLoader::IconLoaderOptions options,
            const QString &format,
            const QVector<HbSharedIconInfo> &multiPieceIconInfo,
            HbMultiIconParams &multiPieceIconParams,
            bool allNvg,
            HbRenderingMode currentRenderingMode);



    static QString KSvg ;
    static QString KNvg ;
    static QString KPic ;
    static QString KBlob ;
    static QString KSgimage ;
};

#endif // HBICONCACHEITEMCREATOR_P_H
