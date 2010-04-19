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

#include "hbxmlloaderabstractactions_p.h"

#include <QCoreApplication>


/*
    \class HbXmlLoaderAbstractActions
    \internal
    \proto
*/

HbXmlLoaderAbstractActions::HbXmlLoaderAbstractActions() : 
    mContext(), 
    mStack(),
    mCurrentContainer(0)
{
}

HbXmlLoaderAbstractActions::~HbXmlLoaderAbstractActions()
{
    reset();    
}

QList <QObject*> HbXmlLoaderAbstractActions::takeAll()
{
    QList<QPointer<QObject> > objects = mTopObjectMap.values();
    
    QList<QObject *> result;
    while (objects.size()) {
        QPointer<QObject> ptr = objects.takeLast();
        if (ptr.data()) {
            result.append(ptr.data());
        }
    }
    
    return result;
    
}


void HbXmlLoaderAbstractActions::removeChildren( QPointer<QObject> parent )
{
    mObjectMap.remove( mObjectMap.key( parent ) );
    QList<QPointer<QObject> > objects = mObjectMap.values();
    for( int i = 0; i < objects.size(); i++ ) {
        if( objects[i].data() ) {
            if( objects[i].data()->parent() == parent ) {
                removeChildren( objects[i] ); 
            }
        }
    }
}

QGraphicsWidget* HbXmlLoaderAbstractActions::findWidget( const QString &name )
{
    QGraphicsWidget *result = 0;
    
    ObjectMap::iterator it = mObjectMap.find(name);
    if (it != mObjectMap.end()) {
        QObject *current = it.value().data();
        result = qobject_cast<QGraphicsWidget *>(current);
    }    
        
    return result;        
}


QObject* HbXmlLoaderAbstractActions::findObject( const QString &name )
{
    if( mObjectMap.contains(name) ) {               
        return mObjectMap.value(name).data();
    }
    return 0;    
}



bool HbXmlLoaderAbstractActions::pushDocument(const QString& context)
{
    Element e;
    e.type = DOCUMENT;
    e.data = 0;
    mStack.append( e );

    mContext = context;

    HB_DOCUMENTLOADER_PRINT( QString( "ADD ELEMENT " )  );
    return true;
}



bool HbXmlLoaderAbstractActions::pop( const ElementType type )
{
    
    // No check for now...
    
    switch( type ) {
         case OBJECT:
         case WIDGET:
         case SPACERITEM:
         case DOCUMENT:
         {
            if( mStack.isEmpty() ) {
                return false;
            }
            
            HB_DOCUMENTLOADER_PRINT( QString( "REMOVE ELEMENT " ) );            
            mStack.removeLast();
            break;
         }
         
         case LAYOUT:
         case CONTAINER:
         case CONNECT:
         case PROPERTY:
         default:
         {
         }
    }         
    return true;
}


void HbXmlLoaderAbstractActions::cleanUp()
{
    mStack.clear();
    
    // Create mTopObjectMap
    for (ObjectMap::const_iterator it = mObjectMap.constBegin(); 
         it != mObjectMap.constEnd(); 
         ++it ) {
        QObject *object = it.value().data();
        
        QGraphicsWidget *asWidget = qobject_cast<QGraphicsWidget *>(object);
        if (asWidget) {
            if (!asWidget->parentItem() && !asWidget->parent()) {
                mTopObjectMap.insert(it.key(), object);
            }
        } else if (object && !object->parent()) {
            mTopObjectMap.insert(it.key(), object);
        } else {
            // not added - owned by another object.
        }
    }
}

QObject* HbXmlLoaderAbstractActions::createObject( const QString& type, const QString &name, const QString &plugin )
{
    Q_UNUSED( type );
    Q_UNUSED( name );
    Q_UNUSED( plugin );
    return 0;
}


QObject* HbXmlLoaderAbstractActions::lookUp(const QString& type, const QString &name, const QString &plugin)
{   
    const bool nameNotEmpty = name.size() != 0;
    bool doLookUp = true;
    QObject *current = 0;
    
    if (nameNotEmpty) {
        ObjectMap::iterator it = mObjectMap.find(name);
        if (it != mObjectMap.end()) {
            current = it.value();
            
            if (!current) {
                mObjectMap.remove(name);
            }
            if (current && !type.isEmpty()) {
                const QByteArray array = type.toUtf8();
                
                if (!current->inherits(array.data())) {                    
                    HB_DOCUMENTLOADER_PRINT( QString( "Existing object requested with invalid type" ) );
                    // We have object already in mObjectMap, but it does not fulfill
                    // all needs. So object look up has failed.
                    doLookUp = false;
                    current = 0;
                }
            }
        }
    }
    
    if (doLookUp && !current) {
        current = createObject(type, name, plugin);
        
        if (nameNotEmpty) {
            mObjectMap.insert(name, current);
        }
    }

    return current;
}

QGraphicsLayoutItem *HbXmlLoaderAbstractActions::findSpacerItemFromStackTop() const
{
    QGraphicsLayoutItem *current = 0;
    if ( mStack[mStack.size()-1].type == SPACERITEM ) {
        current = (QGraphicsLayoutItem*)mStack[mStack.size()-1].data;
    }
    return current;
}

QObject *HbXmlLoaderAbstractActions::findFromStack(bool *isWidgetElement) const
{
    QObject *current = 0;
    bool widget = false;
    
    for( int i = mStack.size() - 1; i >=0; i-- )
    {
        if( ( mStack[i].type == OBJECT ) || ( mStack[i].type == WIDGET ) ) {
            current = (QObject*)mStack[i].data;
            widget = ( mStack[i].type == WIDGET );
            break;
        }
    }
    
    if (isWidgetElement) {
        *isWidgetElement = widget;
    }
    return current;
}

void HbXmlLoaderAbstractActions::deleteAll()
{
    QList<QObject *> list = takeAll();
    
    qDeleteAll( list );
    
    reset();
}


void HbXmlLoaderAbstractActions::reset()
{
    mStack.clear();
    mTopObjectMap.clear();
    mObjectMap.clear();
}



bool HbXmlLoaderAbstractActions::setObjectTree( QList<QObject *> roots )
{
    reset();
    
    for( int i = 0; i < roots.size(); i++ ) {
        mTopObjectMap.insert( roots[i]->objectName(), roots[i] );        
    }
    
    addToObjectMap( roots );
    
    return true;
}

void HbXmlLoaderAbstractActions::addToObjectMap( QList<QObject *> objects )
{
    for( int i = 0; i < objects.size(); i++ ) {
        mObjectMap.insert( objects[i]->objectName(), objects[i] );
        QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>( objects[i] );
        if( widget ) {
            addToObjectMap( widget->childItems() );
        } else {
            addToObjectMap( objects[i]->children() );
        }
    }
}

void HbXmlLoaderAbstractActions::addToObjectMap( QList<QGraphicsItem *> objects )
{
    for( int i = 0; i < objects.size(); i++ ) {
        if( objects[i]->isWidget() ) {
            QGraphicsWidget *widget = static_cast<QGraphicsWidget *>( objects[i] );
            mObjectMap.insert( widget->objectName(), widget );
            addToObjectMap( widget->childItems() );
        }
    }    
}

QString HbXmlLoaderAbstractActions::translate( const QString &value, const QString &comment )
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

int HbXmlLoaderAbstractActions::getAnchorEdge( const QString &edge ) const
{
    if( edge=="TOP" ) {
        return Hb::TopEdge;
    } else if( edge=="BOTTOM" ) {
        return Hb::BottomEdge;
    } else if( edge=="LEFT" ) {
        return Hb::LeftEdge;
    } else if( edge=="RIGHT" ) {
        return Hb::RightEdge;
    } else if( edge=="CENTERH" ) {
        return Hb::CenterHEdge;
    } else if( edge=="CENTERV" ) {
        return Hb::CenterVEdge;
    }
    return -1;
}

QString HbXmlLoaderAbstractActions::getAnchorOppositeEdge( const QString &edge ) const
{
    if( edge=="TOP" ) {
        return "BOTTOM";
    } else if( edge=="BOTTOM" ) {
        return "TOP";
    } else if( edge=="LEFT" ) {
        return "RIGHT";
    } else if( edge=="RIGHT" ) {
        return "LEFT";
    }

    return edge;
}


