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

#ifndef HB_BOOTSTRAPPED
#include "hbglobal.h"
#else
#define HB_CORE_PRIVATE_EXPORT
#endif // HB_BOOTSTRAPPED
#include <QString>
#include <QSize>
#include <QDataStream>

class HbThemeIndexItem;

static const int ThemeIndexTableCountMax = 4;

class HB_CORE_PRIVATE_EXPORT HbThemeIndex
{
public:
    HbThemeIndex(const char *baseAddress);
    ~HbThemeIndex();
public:
    int itemCount();
    const HbThemeIndexItem *itemArray();
    const char *stringAreaStart();
private:
    const char *mBaseAddress;
    int mCount;
    const HbThemeIndexItem *mItemArray;
    const char *mStringAreaStart;
};

class HbThemeIndexItem
{
public:
    HbThemeIndexItem() :
        iconnameOffset(-1),
        folderOffset(-1),
        extOffset(-1),
        mirroredExtOffset(-1),
        defaultSize(),
        mirroredDefaultSize()
    {
    }
    
    ~HbThemeIndexItem() {}

public:
    // Offsets to char* from the theme index base address
    int iconnameOffset;
    int folderOffset;
    int extOffset;
    int mirroredExtOffset;
    QSize defaultSize;
    QSize mirroredDefaultSize;
};

// Version number is always the first integer in the header so the code can use correct header
// struct after reading the version.
struct HbThemeIndexHeaderV1
{
    // Theme index version, current latest one is 1
    int version;
    // Number of items in the theme index
    int count;
};

/**
* Contains shared chunk addresses for theme indexes for the active theme
* and its parent themes starting from table1.
* E.g. currentTheme is SfBlackTheme
* => table1 points to SfBlackTheme theme index
* => table2 points to its parent theme (base theme) index
* => table3 and table4 are -1 because there are no more parents
*/
class HB_CORE_PRIVATE_EXPORT ThemeIndexTables
{
public:
    inline ThemeIndexTables()
    {
        clear();
    }

    inline void clear()
    {
        tables[0] = -1;
        tables[1] = -1;
        tables[2] = -1;
        tables[3] = -1;
        tablesRetrieved = false;
    }

    bool isValid();
    const HbThemeIndexItem *getItem(const QString &itemName, int &tableIndex);
    
public:
    int tables[4];
    QChar drives[4];
    bool tablesRetrieved;
};

#endif // HBTHEMEINDEX_P_H 
