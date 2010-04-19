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

#include <QDebug>
#include "hbthemeindex_p.h"
#ifndef HB_BOOTSTRAPPED
#include "hbthemeclient_p.h"
#endif // HB_BOOTSTRAPPED

// Class HbThemeIndex has the logic of handling different versions of
// the theme index file formats.

HbThemeIndex::HbThemeIndex(const char *baseAddress) :
    mBaseAddress(baseAddress),
    mCount(0),
    mItemArray(0),
    mStringAreaStart(0)
{
}

HbThemeIndex::~HbThemeIndex()
{
}

int HbThemeIndex::itemCount()
{
    if (!mCount) {
        //int version = *(reinterpret_cast<const int *>(mBaseAddress));
        // Assumes version 1 for now
        const HbThemeIndexHeaderV1 *header = reinterpret_cast<const HbThemeIndexHeaderV1 *>(mBaseAddress);
        mCount = header->count;
    }

    return mCount;
}

const HbThemeIndexItem *HbThemeIndex::itemArray()
{
    if (!mItemArray) {
        // Assumes version 1 for now
        mItemArray = reinterpret_cast<const HbThemeIndexItem *>(mBaseAddress + sizeof(HbThemeIndexHeaderV1));
    }
    return mItemArray;
}

const char *HbThemeIndex::stringAreaStart()
{
    if (!mStringAreaStart) {
        // Assumes version 1 for now
        mStringAreaStart = mBaseAddress + sizeof(HbThemeIndexHeaderV1) + itemCount() * sizeof(HbThemeIndexItem);
    }

    return mStringAreaStart;
}

const HbThemeIndexItem *binaryFind(const QString &itemName, const char* themeIndexBase, const HbThemeIndexItem *itemArray, int count)
{
    int begin = 0;
    int end = count - 1;

    // binary search
    while (begin <= end) {
        int mid = begin + (end-begin)/2;
        // Fast string comparison, no unnecessary mem copy
        int comparison = itemName.compare(QLatin1String(themeIndexBase + itemArray[mid].iconnameOffset));
        // If the item was found, we're done.
        if (!comparison) {
            return &itemArray[mid];
        }
        
        // Is the target in lower or upper half?
        else if (comparison < 0) {
            end = mid - 1;
        } else {
            begin = mid + 1;
        }
    }   

    // Did not find the target, return 0.
    return 0;
}

/**
* Returns true if the current theme has a valid theme index included in it.
*/
bool ThemeIndexTables::isValid()
{
#ifndef HB_BOOTSTRAPPED
    // Retrieve tables from server if not done yet
    if (!tablesRetrieved) {
        HbThemeClient::global()->getThemeIndexTables(*this);
        tablesRetrieved = true;

#ifdef THEME_INDEX_TRACES
        qDebug() << "ThemeIndex: retrieved index tables, table[0] =" << tables[0] << "table[1] =" << tables[1];

#endif // THEME_INDEX_TRACES

    }
#endif // HB_BOOTSTRAPPED
    return tables[0] >= 0;
}

const HbThemeIndexItem *ThemeIndexTables::getItem(const QString &itemName, int &tableIndex)
{
    if (isValid()) {
        // Check the current theme index and its parents 
        for (int i = 0; i < ThemeIndexTableCountMax; ++i) {
            int usedTable = tables[i];
            if (usedTable < 0) {
                break;
            }
        
            const char *baseAddress = reinterpret_cast<const char *>(usedTable);
            HbThemeIndex index(baseAddress);
            // Check the table
            int count = index.itemCount();
            const HbThemeIndexItem *itemArray = index.itemArray();
    
            // Get the correct item from the table using binary find
            const HbThemeIndexItem *item = binaryFind(itemName, baseAddress, itemArray, count);
    
            if (item) {
                tableIndex = i;
#ifdef THEME_INDEX_TRACES
                qDebug() << "ThemeIndex: Returned item for" << itemName.toUtf8() << "from table" << tableIndex;
#endif
                return item;
            }
        }
    }
    
    return 0;
}
