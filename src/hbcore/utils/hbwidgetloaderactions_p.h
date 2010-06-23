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
#include <hbanchorlayout.h>
#include <hbmemorymanager_p.h>
#include "hbwidgetloader_p.h"
#include "hbwidgetloaderactions_p.h"
#include "hbxmlloaderbaseactions_p.h"
#include "hbxmlloaderabstractsyntax_p.h"
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

class HbWidgetLoaderActions : public HbXmlLoaderBaseActions
{
    public:
        HbWidgetLoaderActions();
        virtual ~HbWidgetLoaderActions();

        void setWidget( HbWidget *widget );
#ifndef HB_BIN_CSS
        bool createMeshLayout( const QString &widget );
        bool addMeshLayoutEdge( const QString &src, Hb::Edge srcEdge,
                                const QString &dst, Hb::Edge dstEdge,
                                const HbXmlLengthValue &spacing, const QString &spacer = QString() );
#endif

    public:
        Q_DISABLE_COPY(HbWidgetLoaderActions)

        HbWidget *mWidget;
    private:
        HbAnchorLayout *mLayout;
};

class HbWidgetLoaderMemoryActions : public HbXmlLoaderAbstractActions
{
    public:
        HbWidgetLoaderMemoryActions();
        virtual ~HbWidgetLoaderMemoryActions();
        bool createMeshLayout( const QString &widget );
        bool addMeshLayoutEdge( const QString &src, Hb::Edge srcEdge,
                                const QString &dst, Hb::Edge dstEdge,
                                const HbXmlLengthValue &spacing, const QString &spacer = QString() );

    public:
        Q_DISABLE_COPY(HbWidgetLoaderMemoryActions)

    public:
        HbWidgetLoader::LayoutDefinition *mLayoutDef;
};

#endif // HBWIDGETLOADERACTIONS_P_H
