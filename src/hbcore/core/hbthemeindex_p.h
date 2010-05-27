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

#ifndef HBTHEMEINDEX_P_H
#define HBTHEMEINDEX_P_H

#include <hbnamespace.h>
#include <hbthemecommon_p.h>

#ifndef HB_BOOTSTRAPPED
#include "hbglobal.h"
#include "hbthemeutils_p.h"
#else
#define HB_CORE_PRIVATE_EXPORT
#endif // HB_BOOTSTRAPPED
#include <QString>
#include <QSize>
#include <QDataStream>

class HbThemeIndexItem;
struct HbThemeIndexItemData;

class HB_CORE_PRIVATE_EXPORT HbThemeIndex
{
public:
    HbThemeIndex(const char *baseAddress);
    ~HbThemeIndex();

    int itemCount();
    bool validateItems(qint64 byteSize);

    static quint32 hash(const QString &string);
    const HbThemeIndexItemData *getItemData(const QString &itemName);

private:
    void init();
    
    const char *mBaseAddress;
    int mItemCount;
    const HbThemeIndexItemData *mThemeItemDataArray;
    bool initialized;
};

struct HbThemeIndexItemData
{
    enum Flag {
        Default         = 0x00,
        Mirrorable      = 0x01,
        Locked          = 0x02
    };

    enum Type {
        NotDefined      = 0,
        SvgItem         = 1, // .svg
        PngItem         = 2, // .png
        MngItem         = 3, // .mng
        GifItem         = 4, // .gif
        XpmItem         = 5, // .xpm
        JpgItem         = 6, // .jpg
        NvgItem         = 7, // .nvg
        SvgzItem        = 8, // .svgz
        QpicItem        = 9, // .qpic
        FxmlItem        = 10, // .fxml
        AxmlItem        = 11 // .axml
    };

    HbThemeIndexItemData() :
            itemType(NotDefined),
            itemNameHash(0),
            flags(Default),
            mirroredItemType(NotDefined),
            defaultWidth(-1),
            defaultHeight(-1),
            mirroredWidth(-1),
            mirroredHeight(-1) {}

    quint32 itemType; // from enum Type
    quint32 itemNameHash;
    quint32 flags; // from enum Flag

    // These will go to every themable item, but overhead is still small
    // because most of the items are icons
    quint32 mirroredItemType; // from enum Type
    qint32 defaultWidth;
    qint32 defaultHeight;
    qint32 mirroredWidth;
    qint32 mirroredHeight;
};

// Helper class for getting data out of HbThemeIndexItemData
class HB_AUTOTEST_EXPORT HbThemeIndexResource
{
public:
    HbThemeIndexResource(const QString &resourceName);
    ~HbThemeIndexResource();

    bool isValid();
    const QSize defaultItemSize();
    const QSize mirroredItemSize();
    bool isAutomaticallyMirrored();
    bool isLocked();
    QString fullFileName();
    QString fullMirroredFileName();

private:
    void getResourceData();
    const QString &resourceName;
    const HbThemeIndexItemData *data; // not owned
    QString basePath;
    QString themeName;
    HbThemeType type;
};

// Version number is always the first integer in the header so the code can use correct header
// struct after reading the version.
struct HbThemeIndexHeaderV1
{
    // Theme index version, current latest one is 1
    quint32 version;
    // Number of themable items (currently they are icons, effecs and animations)
    quint32 itemCount;
};

#endif //HBTHEMEINDEX_P_H
