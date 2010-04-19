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


#ifndef HBXMLLOADERABSTRACTACTIONS_P_H
#define HBXMLLOADERABSTRACTACTIONS_P_H

#include <hbglobal.h>
#include <hbwidget.h>

#include <QGraphicsWidget>
#include <QGraphicsLayout>
#include <QPointer>


// Uncomment the following in order to get additional debug prints
//#define HB_DOCUMENTLOADER_DEBUG
 
#ifndef HB_DOCUMENTLOADER_DEBUG
#define HB_DOCUMENTLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_DOCUMENTLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_DOCUMENTLOADER_DEBUG


enum ElementType {
    DOCUMENT,
    OBJECT,
    WIDGET,
    LAYOUT,
    SPACERITEM,
    CONNECT,
    CONTAINER,
    PROPERTY,
    SECTION,
    REF,
    VARIABLE,
    METADATA,
    UNKNOWN,
    DEPRECATED
};



class HbXmlLoaderAbstractPrivate;

class HB_CORE_EXPORT HbXmlLoaderAbstractActions
{
    public:
        
        struct Element 
        {
            ElementType type;
            void *data;
        };
        
        typedef QMap<QString, QPointer<QObject> > ObjectMap;
        
    public:    
    
        HbXmlLoaderAbstractActions();
        virtual ~HbXmlLoaderAbstractActions();
        
        QList<QObject *> takeAll();
        
        
        QGraphicsWidget* findWidget(const QString &name);
        QObject* findObject(const QString &name);

        virtual QObject *createObject( const QString& type, const QString &name, const QString &plugin );
        
        
        bool pushDocument( const QString& context);
        bool pop( const ElementType type );
        
        int getAnchorEdge( const QString &edge ) const;
        QString getAnchorOppositeEdge( const QString &edge ) const;

        void cleanUp();
        void reset();
        void deleteAll();
        
        bool setObjectTree( QList<QObject *> roots );

        QString translate( const QString &value, const QString &comment );
        
    
        Q_DISABLE_COPY(HbXmlLoaderAbstractActions)

        QObject *lookUp(const QString& type, const QString &name, const QString &plugin = QString());
        QObject *findFromStack(bool *isWidgetElement = 0) const;
        QGraphicsLayoutItem *findSpacerItemFromStackTop() const;
        void removeChildren( QPointer<QObject> parent );
        
        void addToObjectMap( QList<QObject *> objects );
        void addToObjectMap( QList<QGraphicsItem *> objects );

    public:
        QString mContext;

        QList<Element> mStack;
        ObjectMap mObjectMap;    
        ObjectMap mTopObjectMap;
        QGraphicsLayout *mCurrentLayout;
        QList<QVariant> *mCurrentContainer;
};

#endif // HBXMLLOADERABSTRACTACTIONS_P_H
