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


#ifndef HBWIDGETLOADERACTIONS_P_H
#define HBWIDGETLOADERACTIONS_P_H

#include <hbglobal.h>
#include <hbwidget.h>
#include <hbstring_p.h>
#include <hbmeshlayout_p.h>
#include <hbmemorymanager_p.h>
#include "hbwidgetloaderactions_p.h"
#include "hbxmlloaderabstractactions_p.h"
#include "hbvector_p.h"
#include <QDateTime>

// Uncomment the following in order to get additional debug prints
//#define HB_DOCUMENTLOADER_DEBUG
 
#ifndef HB_DOCUMENTLOADER_DEBUG
#define HB_DOCUMENTLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_DOCUMENTLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_DOCUMENTLOADER_DEBUG


struct MeshItem
{
    HbString src;
    HbString dst;
    Hb::Edge srcEdge;
    Hb::Edge dstEdge;
    HbString spacing;
    MeshItem(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory):src(type),dst(type),spacing(type)
    {
    }
};

struct LayoutDefinition {
    HbVector<MeshItem> meshItems;
    HbVector<HbString> spacers;
    LayoutDefinition(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory):meshItems(type),spacers(type)
    {
    }
};

class HB_CORE_PRIVATE_EXPORT HbWidgetLoaderActions : public HbXmlLoaderAbstractActions
{
    public:
        HbWidgetLoaderActions(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory);
        virtual ~HbWidgetLoaderActions();

        bool createMeshLayout();
        bool addMeshLayoutEdge( const QString &src, const QString &srcEdge, 
                                const QString &dst, const QString &dstEdge,
                                const QString &spacing, const QString &spacer = QString() );

        void updateWidget(LayoutDefinition *layoutDef);

        int getLayoutDefintionOffset();
        void setLayoutDefintionOffset(int offset);

    public:
        Q_DISABLE_COPY(HbWidgetLoaderActions)

        HbWidget *mWidget;
        bool mLayoutFound;

        // Layout caching
        static LayoutDefinition *mCacheLayout;
        static QString mCacheFileName;
        static QString mCacheName;
        static QString mCacheSection;
        static QDateTime mCacheModified;
    private:
        HbMemoryManager::MemoryType mType;
        int mLayoutDefinitionOffset;
};

#endif // HBWIDGETLOADERACTIONS_P_H
