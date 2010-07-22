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

#include "hbxmlloaderbaseactions_p.h"
#include "hbxmlloaderabstractsyntax_p.h"
#include <hbinstance.h>
#include <hbdeviceprofile.h>
#include <hbstyle.h>

#include <QCoreApplication>

const char *GRAPHICSWIDGETCLASSNAME = "QGraphicsWidget";

/*
    \class HbXmlLoaderBaseActions
    \internal
    \proto
*/

HbXmlLoaderBaseActions::HbXmlLoaderBaseActions() : 
    mContext(), 
    mStack()
{
}

HbXmlLoaderBaseActions::~HbXmlLoaderBaseActions()
{
    reset();    
}

QList <QObject*> HbXmlLoaderBaseActions::takeAll()
{
    QList<ObjectMapItem> objects = mTopObjectMap.values();
    
    QList<QObject *> result;
    while (objects.size()) {
        ObjectMapItem item = objects.takeLast();
        if (item.mObject.data()) {
            result.append(item.mObject.data());
        }
    }
    mTopObjectMap.clear();
    return result;
    
}

QGraphicsWidget* HbXmlLoaderBaseActions::findWidget( const QString &name )
{
    QGraphicsWidget *result = 0;
    
    ObjectMap::const_iterator it = mObjectMap.find(name);
    if (it != mObjectMap.end() && it.value().mType == HbXml::WIDGET ) {
        result = static_cast<QGraphicsWidget *>(it.value().mObject.data());
    }    
        
    return result;        
}


QObject* HbXmlLoaderBaseActions::findObject( const QString &name )
{
    if( mObjectMap.contains(name) ) {               
        return mObjectMap.value(name).mObject.data();
    }
    return 0;    
}



bool HbXmlLoaderBaseActions::pushDocument(const QString& context)
{
    HbXml::Element e;
    e.type = HbXml::DOCUMENT;
    e.data = 0;
    mStack.append( e );

    mContext = context;

    HB_DOCUMENTLOADER_PRINT( QString( "ADD ELEMENT " )  );
    return true;
}



bool HbXmlLoaderBaseActions::pop( const HbXml::ElementType type )
{
    
    // No check for now...
    
    switch( type ) {
         case HbXml::OBJECT:
         case HbXml::WIDGET:
         case HbXml::SPACERITEM:
         case HbXml::DOCUMENT:
         {
            if( mStack.isEmpty() ) {
                return false;
            }
            
            HB_DOCUMENTLOADER_PRINT( QString( "REMOVE ELEMENT " ) );            
            mStack.removeLast();
            break;
         }
         
         case HbXml::LAYOUT:
         case HbXml::CONTAINER:
         case HbXml::CONNECT:
         case HbXml::PROPERTY:
         default:
         {
         }
    }         
    return true;
}


void HbXmlLoaderBaseActions::cleanUp()
{
    mStack.clear();
    
    // Create mTopObjectMap
    for (ObjectMap::iterator it = mObjectMap.begin(); 
         it != mObjectMap.end(); 
         ++it ) {               
        QObject *object = it.value().mObject.data();
        if ( it.value().mOwned ) {
            if (object && it.value().mType == HbXml::WIDGET) {
                QGraphicsWidget *asWidget = static_cast<QGraphicsWidget *>(object);
                if (!asWidget->parentItem() && !asWidget->parent()) {
                    mTopObjectMap.insert(it.key(), it.value());
                }
            } else if (object && !object->parent()) {
                mTopObjectMap.insert(it.key(), it.value());
            } else {
                // not added - owned by another object.
            }
            it.value().mOwned = false;
        }
    }
}

QObject* HbXmlLoaderBaseActions::createObject( const QString& type, const QString &name, const QString &plugin )
{
    Q_UNUSED( type );
    Q_UNUSED( name );
    Q_UNUSED( plugin );
    return 0;
}


HbXmlLoaderBaseActions::ObjectMapItem HbXmlLoaderBaseActions::lookUp(const QString& type, const QString &name, const QString &plugin)
{   
    const bool nameNotEmpty = name.size() != 0;
    bool doLookUp = true;
    ObjectMapItem current;
    current.mObject = 0;
    current.mType = HbXml::OBJECT;
    
    if (nameNotEmpty) {
        ObjectMap::const_iterator it = mObjectMap.find(name);
        if (it != mObjectMap.end()) {
            current = it.value();
            
            if (!current.mObject) {
                mObjectMap.remove(name);
            }
            // CHECK DISABLED FOR NOW.
            /*
            if (current.mObject && !type.isEmpty()) {
                const QByteArray array = type.toUtf8();
                
                if (!current.mObject->inherits(array.data())) {                    
                    HB_DOCUMENTLOADER_PRINT( QString( "Existing object requested with invalid type" ) );
                    // We have object already in mObjectMap, but it does not fulfill
                    // all needs. So object look up has failed.
                    doLookUp = false;
                    current.mObject = 0;
                }
            }
            */
        }
    }
    
    if (doLookUp && !current.mObject) {
        QObject *obj = createObject(type, name, plugin);        
        if (obj) {
            current.mObject = obj;
            current.mType = qobject_cast<QGraphicsWidget*>(obj) ? HbXml::WIDGET : HbXml::OBJECT;
        }
        if (nameNotEmpty) {
            mObjectMap.insert(name, current);
        }
    }

    return current;
}

QObject *HbXmlLoaderBaseActions::findFromStack(bool *isWidgetElement) const
{
    QObject *current = 0;
    bool widget = false;
    
    for( int i = mStack.size() - 1; i >=0; i-- )
    {
        if( ( mStack.at(i).type == HbXml::OBJECT ) || ( mStack.at(i).type == HbXml::WIDGET ) ) {
            current = static_cast<QObject*>(mStack.at(i).data);
            widget = ( mStack.at(i).type == HbXml::WIDGET );
            break;
        }
    }
    
    if (isWidgetElement) {
        *isWidgetElement = widget;
    }
    return current;
}

void HbXmlLoaderBaseActions::deleteAll()
{
    QList<QObject *> list = takeAll();
    
    qDeleteAll( list );
    
    reset();
}


void HbXmlLoaderBaseActions::reset()
{
    mStack.clear();
    mTopObjectMap.clear();
    mObjectMap.clear();
}



bool HbXmlLoaderBaseActions::setObjectTree( QList<QObject *> roots )
{
    reset();
    addToObjectMap( roots );
    return true;
}

void HbXmlLoaderBaseActions::addToObjectMap( QList<QObject *> objects )
{
    for ( int i = 0; i < objects.size(); i++ ) {
        QObject *obj = objects.at(i);
        QGraphicsWidget *widget = qobject_cast<QGraphicsWidget*>(obj);

        ObjectMapItem item;
        item.mObject = obj;
        item.mType = widget ? HbXml::WIDGET : HbXml::OBJECT;
        item.mOwned = false;
        mObjectMap.insert( obj->objectName(), item );

        if ( widget ) {
            addToObjectMap( widget->childItems() );
        } else {
            addToObjectMap( obj->children() );
        }
    }
}

void HbXmlLoaderBaseActions::addToObjectMap( QList<QGraphicsItem *> objects )
{
    for ( int i = 0; i < objects.size(); i++ ) {
        if ( objects.at(i)->isWidget() ) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>( objects.at(i) );
            ObjectMapItem item;
            item.mObject = widget;
            item.mType = HbXml::WIDGET;
            item.mOwned = false;
            mObjectMap.insert( widget->objectName(), item );
            addToObjectMap( widget->childItems() );
        }
    }    
}

QString HbXmlLoaderBaseActions::translate( const QString &value, const QString &comment )
{
    if( ! mContext.isEmpty() ) {
        QByteArray contextUtf8(mContext.toUtf8());
        QByteArray valueUtf8(value.toUtf8());
		
		if (comment.isEmpty()) {
			return QCoreApplication::translate( 
						contextUtf8.data(), valueUtf8.data(), 
						0, QCoreApplication::UnicodeUTF8 );
		} else {
			QByteArray commentUtf8(comment.toUtf8());
			return QCoreApplication::translate( 
						contextUtf8.data(), valueUtf8.data(), 
						commentUtf8.data(), QCoreApplication::UnicodeUTF8 );
		}        
    } else {
        return value;
    }
}

Hb::Edge HbXmlLoaderBaseActions::getAnchorOppositeEdge( Hb::Edge edge ) const
{
    if ( edge == Hb::TopEdge ) {
        return Hb::BottomEdge;
    } else if ( edge == Hb::BottomEdge ) {
        return Hb::TopEdge;
    } else if ( edge == Hb::LeftEdge ) {
        return Hb::RightEdge;
    } else if ( edge == Hb::RightEdge ) {
        return Hb::LeftEdge;
    }

    return edge;
}
#ifndef HB_BIN_CSS
bool HbXmlLoaderBaseActions::toPixels(const HbXmlLengthValue &lengthVal, qreal& result) const
{
    bool retVal(true);
    switch (lengthVal.mType) {
        case HbXmlLengthValue::None:
            retVal = false;
            break;
        case HbXmlLengthValue::PlainNumber:
        case HbXmlLengthValue::Pixel:
            result = lengthVal.mValue;
            break;
        case HbXmlLengthValue::Unit:
            result = mCurrentProfile.unitValue() * lengthVal.mValue;
            break;
        case HbXmlLengthValue::Millimeter:
            result = mCurrentProfile.ppmValue() * lengthVal.mValue;
            break;
        case HbXmlLengthValue::Variable:
        case HbXmlLengthValue::Expression:
            retVal = hbInstance->style()->parameter( lengthVal.mString, result, mCurrentProfile );
            break;
    }
    return retVal;
}
#endif


