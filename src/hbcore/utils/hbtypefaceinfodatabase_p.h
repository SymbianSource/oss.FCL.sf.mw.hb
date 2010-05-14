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

#ifndef HBTYPEFACEINFODATABASE_H
#define HBTYPEFACEINFODATABASE_H

#include "hbtypefaceinfo_p.h"
#include "hbtypefacexmlparser_p.h"
#include "hbvector_p.h"
#include "hbstring_p.h"


struct HbTypefaceInfoItem {
    HbFontSpec::Role mRoleEnum;
    HbString mFamily;
    bool mIsBold;

    struct HbTypefaceMeasureInfoStruct
    {
        int textHeight;
        int pixelSize;
        int measuredAscent;
    };

    HbVector<HbTypefaceMeasureInfoStruct> mDownSizeTable;
    int mLowestExtent;
    int mHighestExtent;

    HbTypefaceInfoItem(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : mFamily(type), mDownSizeTable(type)
    {
    }

    bool tableContainsHeight( int textHeight, int &index )
    {
        for( int i = 0; i < mDownSizeTable.size(); i++ ) {
            if( mDownSizeTable.at(i).textHeight == textHeight ) {
                index = i;
                return true;
            }
        }
        return false;
    }
};


typedef HbVector<HbTypefaceInfoItem> HbTypefaceInfoVector;



class HB_CORE_PRIVATE_EXPORT HbTypefaceInfoDatabase
{
public:
    static HbTypefaceInfoDatabase *instance(HbMemoryManager::MemoryType type =
                                             HbMemoryManager::HeapMemory);

    int typefaceInfoVectorOffset();

private:
    HbTypefaceInfoDatabase(HbMemoryManager::MemoryType type =
                            HbMemoryManager::HeapMemory);
    void init();
    bool readTypefaceMetricsFile( HbTypefaceXmlParser *parser, HbTypefaceInfoItem *typeFaceInfoItem );
    void autoGenerateMetrics( HbTypefaceInfoItem *typeFaceInfoItem );
    void outputMetrics( HbTypefaceInfoItem *typeFaceInfoItem ) const;
    Q_DISABLE_COPY(HbTypefaceInfoDatabase)

private:
    HbTypefaceInfoVector *mTypefaceInfoVector;
    int mTypefaceInfoVectorOffset;
    HbMemoryManager::MemoryType mType;
};


#endif // HBTYPEFACEINFODATABASE_H









