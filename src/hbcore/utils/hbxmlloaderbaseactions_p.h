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


#ifndef HBXMLLOADERBASEACTIONS_P_H
#define HBXMLLOADERBASEACTIONS_P_H

#include "hbxmlloaderabstractactions_p.h"

#include <hbglobal.h>
#include <hbdeviceprofile.h>

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

class HbXmlLoaderAbstractPrivate;
struct HbXmlLengthValue;

class HB_CORE_PRIVATE_EXPORT HbXmlLoaderBaseActions : public HbXmlLoaderAbstractActions
{
    public:

        struct ObjectMapItem
        {
            ObjectMapItem() : mObject(0), mType(HbXml::UNKNOWN), mOwned(true) {};

            QPointer<QObject> mObject;
            HbXml::ElementType mType;
            bool mOwned;
        };
        typedef QMap<QString, ObjectMapItem> ObjectMap;
        
    public:    
    
        HbXmlLoaderBaseActions();
        virtual ~HbXmlLoaderBaseActions();

        virtual QObject *createObject(const QString& type, const QString &name, const QString &plugin);

        bool setObjectTree(QList<QObject *> roots);
        QList<QObject *> takeAll();

        QGraphicsWidget* findWidget(const QString &name);
        QObject* findObject(const QString &name);

	public:

        virtual bool pushDocument(const QString& context);
        virtual bool pop(const HbXml::ElementType type);
        virtual void cleanUp();
        virtual void reset();
        virtual void deleteAll();

    protected:

        ObjectMapItem lookUp(const QString& type, const QString &name, const QString &plugin = QString());
        QObject *findFromStack(bool *isWidgetElement = 0) const;
        QGraphicsLayoutItem *findSpacerItemFromStackTop() const;
        
        void addToObjectMap(QList<QObject *> objects);
        void addToObjectMap(QList<QGraphicsItem *> objects);

        bool toPixels(const HbXmlLengthValue &lengthVal, qreal& result) const;
        QString translate(const QString &value, const QString &comment);
        Hb::Edge getAnchorOppositeEdge( Hb::Edge edge ) const;
		
    protected:
        QString mContext;

        QList<HbXml::Element> mStack;
        ObjectMap mObjectMap;    
        ObjectMap mTopObjectMap;
        QGraphicsLayout *mCurrentLayout;
    private:
        Q_DISABLE_COPY(HbXmlLoaderBaseActions)
    public:
#ifndef HB_BIN_CSS
        HbDeviceProfile mCurrentProfile;
#endif
};

#endif // HBXMLLOADERBASEACTIONS_P_H
