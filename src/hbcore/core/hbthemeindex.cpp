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

#include "hbthemeindex_p.h"
#include <QDebug>
#ifndef HB_BOOTSTRAPPED
#include "hbthemeclient_p.h"
#include "hbinstance.h"
#endif // HB_BOOTSTRAPPED


HbThemeIndexResource::HbThemeIndexResource(const QString &resourceName) :
    resourceName(resourceName),
    data(0)
{
#ifndef HB_BOOTSTRAPPED
    getResourceData();
#endif // HB_BOOTSTRAPPED
}

void HbThemeIndexResource::getResourceData()
{
#ifndef HB_BOOTSTRAPPED
#ifdef THEME_INDEX_TRACES
    qDebug() << "HbThemeIndexResource::getResourceData(), resourceName: " << resourceName;
#endif
    // Theme index tables are always valid in shared memory

    // Try to find themable item with following logic:
    // If item is locked in base theme
    //     Select item from base theme
    // Else if item is found from C-drive operator theme
    //     Select item from C-drive operator theme
    // Else if item is found from ROM operator theme
    //    Select item from ROM operator theme
    // Else if item is found from active theme
    //    Select item from active theme
    // Else
    //    Select item from base theme

    type = BaseTheme; // This is the default, even we couldn't find themable resource

    // First check base theme, which should be always valid
    HbThemeIndexInfo info = HbThemeUtils::getThemeIndexInfo(BaseTheme);
    if (!info.address) { // This shouldn't happen, as there must be valid base theme
        return; // Data will be 0
    }
    
    HbThemeIndex baseIndex(info.address);
    const HbThemeIndexItemData *baseItemData = baseIndex.getItemData(resourceName);

    if (!baseItemData) { // If the item is not found from base theme, it can't be found elsewhere
        return; // Data will be 0
    }

    if (baseItemData->flags & HbThemeIndexItemData::Locked) {
        basePath = info.path;
        themeName = info.name;
        data = baseItemData;
        return;
    }

    // Base wasn't locked, next check operator theme in C-drive
    info = HbThemeUtils::getThemeIndexInfo(OperatorC);
    if (info.address) {
        HbThemeIndex operatorCIndex(info.address);
        const HbThemeIndexItemData *operatorCItemData = operatorCIndex.getItemData(resourceName);

        if (operatorCItemData) { // Found, use it
            type = OperatorC;
            basePath = info.path;
            themeName = info.name;
            data = operatorCItemData;
            return;
        }
    }

    // Not found from operator theme in C-drive, next check operator theme in ROM
    info = HbThemeUtils::getThemeIndexInfo(OperatorROM);
    if (info.address) {
        HbThemeIndex operatorZIndex(info.address);
        const HbThemeIndexItemData *operatorZItemData = operatorZIndex.getItemData(resourceName);

        if (operatorZItemData) { // Found, use it
            type = OperatorROM;
            basePath = info.path;
            themeName = info.name;
            data = operatorZItemData;
            return;
        }
    }

    // Not found from operator themes, try active theme
    info = HbThemeUtils::getThemeIndexInfo(ActiveTheme);
    if (info.address) {
        HbThemeIndex activeThemeIndex(info.address);
        const HbThemeIndexItemData *activeThemeItemData =
                activeThemeIndex.getItemData(resourceName);

        if (activeThemeItemData) { // Found, use it
            type = ActiveTheme;
            basePath = info.path;
            themeName = info.name;
            data = activeThemeItemData;
            return;
        }
    }

    // Not found from active theme, use base
    info = HbThemeUtils::getThemeIndexInfo(BaseTheme);
    basePath = info.path;
    themeName = info.name;
    data = baseItemData;
    return;
#endif // HB_BOOTSTRAPPED
}


HbThemeIndexResource::~HbThemeIndexResource()
{
}

bool HbThemeIndexResource::isValid()
{
    if (data && data->itemType == HbThemeIndexItemData::ColorItem) {
        Q_ASSERT_X(!(data->flags & HbThemeIndexItemData::Reference),
            "HbThemeIndexResource::isValid()",
            "Reference to another color variable not supported");
    }

    if (data) {
        return true;
    }
    return false;
}

const QSize HbThemeIndexResource::defaultItemSize()
{
    if (data) {
        return QSize(data->defaultWidth, data->defaultHeight);
    }
    return QSize();
}

const QSize HbThemeIndexResource::mirroredItemSize()
{
    if (data) {
        return QSize(data->mirroredWidth, data->mirroredHeight);
    }
    return QSize();
}

bool HbThemeIndexResource::isAutomaticallyMirrored()
{
    if (data) {
        return (data->flags & HbThemeIndexItemData::Mirrorable);
    }
    return false;
}

bool HbThemeIndexResource::isLocked()
{
    if (data) {
        return (data->flags & HbThemeIndexItemData::Locked);
    }
    return false;
}

QString HbThemeIndexResource::fullFileName()
{
    if (!data) {
        return QString();
    }

    QString fullName = basePath;
    switch (data->itemType) {
        case HbThemeIndexItemData::SvgItem:
            {
            fullName = fullName + "/icons/" + themeName + "/scalable/" + resourceName + ".svg";
            break;
            }
        case HbThemeIndexItemData::PngItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/" + resourceName + ".png";
            break;
            }
        case HbThemeIndexItemData::MngItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/" + resourceName + ".mng";
            break;
            }
        case HbThemeIndexItemData::GifItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/" + resourceName + ".gif";
            break;
            }
        case HbThemeIndexItemData::XpmItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/" + resourceName + ".xpm";
            break;
            }
        case HbThemeIndexItemData::JpgItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/" + resourceName + ".jpg";
            break;
            }
        case HbThemeIndexItemData::NvgItem:
            {
            fullName = fullName + "/icons/" + themeName + "/scalable/" + resourceName + ".nvg";
            break;
            }
        case HbThemeIndexItemData::SvgzItem:
            {
            fullName = fullName + "/icons/" + themeName + "/scalable/" + resourceName + ".svgz";
            break;
            }
        case HbThemeIndexItemData::QpicItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/" + resourceName + ".qpic";
            break;
            }
        case HbThemeIndexItemData::AxmlItem:
            {
            fullName = fullName + "/animations/" + themeName + '/' + resourceName;
            break;
            }
        case HbThemeIndexItemData::FxmlItem:
            {
            fullName = fullName + "/effects/" + themeName + '/' + resourceName;
            break;
            }
        default:
            {
            break;
            }
        }

    return fullName;
}

QString HbThemeIndexResource::fullMirroredFileName()
{
    if (!data) {
        return QString();
    }

    QString fullName = basePath;
    switch (data->mirroredItemType) {
        case HbThemeIndexItemData::SvgItem:
            {
            fullName = fullName + "/icons/" + themeName + "/scalable/mirrored/"
                       + resourceName + ".svg";
            break;
            }
        case HbThemeIndexItemData::PngItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/mirrored/"
                       + resourceName + ".png";
            break;
            }
        case HbThemeIndexItemData::MngItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/mirrored/"
                       + resourceName + ".mng";
            break;
            }
        case HbThemeIndexItemData::GifItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/mirrored/"
                       + resourceName + ".gif";
            break;
            }
        case HbThemeIndexItemData::XpmItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/mirrored/"
                       + resourceName + ".xpm";
            break;
            }
        case HbThemeIndexItemData::JpgItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/mirrored/"
                       + resourceName + ".jpg";
            break;
            }
        case HbThemeIndexItemData::NvgItem:
            {
            fullName = fullName + "/icons/" + themeName + "/scalable/mirrored/"
                       + resourceName + ".nvg";
            break;
            }
        case HbThemeIndexItemData::SvgzItem:
            {
            fullName = fullName + "/icons/" + themeName + "/scalable/mirrored/"
                       + resourceName + ".svgz";
            break;
            }
        case HbThemeIndexItemData::QpicItem:
            {
            fullName = fullName + "/icons/" + themeName + "/pixmap/mirrored/"
                       + resourceName + ".qpic";
            break;
            }
        default:
            {
            return fullFileName(); // There was no mirrored icon, return normal icon
            }
        }

    return fullName;
}

QColor HbThemeIndexResource::colorValue()
{
    if (!data || data->itemType != HbThemeIndexItemData::ColorItem) {
        return QColor();
    }

    return QColor(data->colorValue);
}


// Class HbThemeIndex has the logic of handling different versions of
// the theme index file formats.

HbThemeIndex::HbThemeIndex(const char *baseAddress) :
    mBaseAddress(baseAddress),
    mItemCount(0),
    mThemeItemDataArray(0),
    initialized(false)
{
}

HbThemeIndex::~HbThemeIndex()
{
}

void HbThemeIndex::init()
{
    //int version = *(reinterpret_cast<const int *>(mBaseAddress));
    // Assumes version 1 for now
    const HbThemeIndexHeaderV1 *header =
            reinterpret_cast<const HbThemeIndexHeaderV1 *>(mBaseAddress);
    mItemCount = header->itemCount;
    mThemeItemDataArray = reinterpret_cast<const HbThemeIndexItemData *>
                            (mBaseAddress + sizeof(HbThemeIndexHeaderV1));

    initialized = true;
}

quint32 HbThemeIndex::hash(const QString &string)
{
    quint32 hashValue = 0;
    quint32 c;
    QByteArray array = string.toLatin1();
    char *data = array.data();
    c = *data++;

    while (c) {
        hashValue = c + (hashValue << 6) + (hashValue << 16) - hashValue;
        c = *data++;
    }

    return hashValue;
}

const HbThemeIndexItemData *HbThemeIndex::getItemData(const QString &itemName)
{
    if (!initialized) {
        init();
    }

    quint32 hashValue = hash(itemName);
    int begin = 0;
    int end = mItemCount - 1;

    const HbThemeIndexItemData *retItem = 0;

    // binary search
    while (begin <= end) {
        int mid = begin + (end - begin) / 2;

        if (mThemeItemDataArray[mid].itemNameHash == hashValue) {
            retItem = &mThemeItemDataArray[mid];
            return retItem;
        } else if (hashValue < mThemeItemDataArray[mid].itemNameHash) {
            end = mid - 1;
        } else {
            begin = mid + 1;
        }
    }   

    // Did not find the target, return 0.
    return retItem;
}

int HbThemeIndex::itemCount()
{
    if (!initialized) {
        init();
    }

    return mItemCount;
}

bool HbThemeIndex::validateItems(qint64 byteSize)
{
    if (!initialized) {
        init();
    }

    bool indexOK = false;

    qint64 indexCalculatedSize = (qint64)(sizeof(HbThemeIndexHeaderV1) +
        (mItemCount * sizeof(HbThemeIndexItemData)));

    if (indexCalculatedSize == byteSize) {
        indexOK = true;
    }

    #ifdef THEME_INDEX_TRACES
    if (!indexOK) {
        qDebug() <<  "ThemeIndex: Index file corrupted (index size is wrong)!";
    }
    #endif    

    return indexOK;
}
