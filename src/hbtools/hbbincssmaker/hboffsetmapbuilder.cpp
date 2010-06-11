/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbTools module of the UI Extensions for Mobile.
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

#include "hboffsetmapbuilder_p.h"

#include <QFileInfo>

extern QTextStream err;

/*!
    Adds \a offsets for a \a className. \a fileInfo points to the layout css file.
    \a offsets are indexed using CSSFileType - enum value.
*/
bool HbOffsetMapBuilder::addWidgetOffsets(const QString &className,
                                       const QFileInfo *fileInfo,
                                       int offsets[])
{
    bool retValue = true;
    quint32 nameHash = HbSharedCache::hash(QStringRef(&className));
    HbBinMakerOffsetItem mapItem = _mapItems.value(nameHash, HbBinMakerOffsetItem());
    if (mapItem.isNull()) {
        if (fileInfo) {
            mapItem.name = fileInfo->absoluteFilePath();
        }
        mapItem.widgetHash = nameHash;
        mapItem.offsetCSS = offsets[CSSFile];
        mapItem.offsetColorCSS = offsets[ColorCSSFile];
        _mapItems.insert(nameHash, mapItem);
    } else {
        err << "duplicate hash value found!" << endl;
        retValue = false;
    }
    return retValue;
}

/*!
    Adds \a widgetML layout offsets for a class, which hash is \a classNameHash.
    Widget offsets for a class must already be added, before calling this method,
    see \sa addWidgetOffsets.
    Offsets for each layout is in \a layoutInfoList. \a filePath contains the path to the
    widgetml file for the class.
*/
bool HbOffsetMapBuilder::addWidgetMLOffsets(const QString &filePath,
                                            quint32 classNameHash,
                                            const QList<LayoutItem> &layoutInfoList)
{
    bool retValue = true;
    QMap<quint32, HbBinMakerOffsetItem>::iterator offsetItem = _mapItems.find(classNameHash);
    if (offsetItem != _mapItems.end()) {
        QSet<quint64> hashCheck;
        QList<HbLayoutIndexItem> &layoutIndexTable = offsetItem.value().layoutIndexItemList;
        Q_FOREACH(const LayoutItem &layoutInfo, layoutInfoList) {
            HbLayoutIndexItem item;
            item.layoutNameHash = HbSharedCache::hash(QStringRef(&layoutInfo.layout->layoutname));
            item.sectionNameHash = HbSharedCache::hash(QStringRef(&layoutInfo.layout->section));
            quint64 hash = (quint64(item.layoutNameHash) << 32) | item.sectionNameHash;
            if (!hashCheck.contains(hash)) {
                hashCheck.insert(hash);
            } else {
                err << "duplicate layout name hash found for: " << filePath << endl;
                retValue = false;
                break;
            }
            item.offset = layoutInfo.offset;
            layoutIndexTable.append(item);
        }
    }
    return retValue;
}

/*!
    dumps the contents of the offset map to bytearray.

*/
QByteArray HbOffsetMapBuilder::result()
{
    QByteArray dataArray;

    //first layoutindextable is locates after the offsetitem-array.
    int currentLayoutIndexTableOffset = _mapItems.size() * sizeof(HbOffsetItem);

    //store offsetitems, update layout index table offset
    foreach(const HbBinMakerOffsetItem &mapItem, _mapItems) {
        HbOffsetItem tmp(mapItem);
        if (!mapItem.layoutIndexItemList.isEmpty()) {
            tmp.offsetLayoutIndexTable = currentLayoutIndexTableOffset;
            currentLayoutIndexTableOffset += sizeof(quint32) // space for the size of the layoutindex table
                                             + mapItem.layoutIndexItemList.size()
                                             * sizeof(HbLayoutIndexItem);
        }
        dataArray.append(reinterpret_cast<const char*>(&tmp), sizeof(HbOffsetItem));
    }

    //store layout index tables
    QMap<quint32, HbBinMakerOffsetItem>::iterator end = _mapItems.end();
    for(QMap<quint32, HbBinMakerOffsetItem>::iterator i = _mapItems.begin(); i != end; ++i) {
        HbBinMakerOffsetItem &mapItem = i.value();
        if (!mapItem.layoutIndexItemList.isEmpty()) {
            qSort(mapItem.layoutIndexItemList); //sort for binary search.
            //store the table size first.
            quint32 size = mapItem.layoutIndexItemList.size();
            dataArray.append(reinterpret_cast<const char*>(&size), sizeof(quint32));
            //store the layout-index items.
            foreach(const HbLayoutIndexItem &layoutIndexItem, mapItem.layoutIndexItemList) {
                dataArray.append(reinterpret_cast<const char*>(&layoutIndexItem),
                                 sizeof(HbLayoutIndexItem));
            }
        }
    }
    return dataArray;
}
