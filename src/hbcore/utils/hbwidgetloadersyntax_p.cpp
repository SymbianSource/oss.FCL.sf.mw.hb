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

#include "hbwidgetloadersyntax_p.h"

#include <QtDebug>
#include <QXmlStreamReader>
#include <hbwidget.h>


/*
    \class HbWidgetLoaderSyntax
    \internal
    \proto
*/

// Uncomment the following in order to get additional debug prints
//#define HB_WIDGETLOADER_DEBUG
 
#ifndef HB_WIDGETLOADER_DEBUG
#define HB_WIDGETLOADER_PRINT(a) 
#else
#include <QDebug>
#define HB_WIDGETLOADER_PRINT(a) qDebug() << QString(a);
#endif // HB_WIDGETLOADER_DEBUG


// Widget loader version number
#define VERSION_MAJOR 0
#define VERSION_MINOR 3

#define MIN_SUPPORTED_VERSION_MAJOR 0
#define MIN_SUPPORTED_VERSION_MINOR 1

/*!
    \internal
*/
HbWidgetLoaderSyntax::HbWidgetLoaderSyntax( HbXmlLoaderAbstractActions *actions ) :
    HbXmlLoaderBaseSyntax( actions )
{
}

/*!
    \internal
*/
HbWidgetLoaderSyntax::~HbWidgetLoaderSyntax()
{
}

/*!
    \internal
*/
bool HbWidgetLoaderSyntax::load( QIODevice *device, const QString &name, const QString &section )
{
    //  HbXmlLoaderAbstractSyntax::load is not called here as it is coupled with the
    //  HbDeviceProfile. Due to this HbInstance will get created.
    //  That is why loadDevice() is called.

    mLayoutName = name;
    mLayoutFound = false;

    bool result = HbXmlLoaderBaseSyntax::loadDevice(device,section);

    if ( result && !mLayoutFound ) {
        // File parsed ok, but requested layout not found.
        result = false;
        qWarning() << "Requested layout not found"; 
    }
    
    return result;
}

/*!
    \internal
*/
HbXml::ElementType HbWidgetLoaderSyntax::elementType( QStringRef name ) const
{
    const QString stringName = name.toString();

    if( stringName == lexemValue(TYPE_HBWIDGET) ){
        return HbXml::DOCUMENT;
    } 
    return HbXmlLoaderBaseSyntax::elementType( name );
}

/*!
    \internal
*/
bool HbWidgetLoaderSyntax::processDocument()
{
    bool ok, ok1, ok2, res = true;
    
    QString ver_str =  attribute( ATTR_VERSION );
    
    ver_str.toDouble( &ok );
    QStringList ver = ver_str.split( '.' ); 

        
    if( ( !ok ) || ( ver.size() != 2 ) ) {
        qWarning() << "Wrong hbwidget version format " << mReader.lineNumber();
        return false;
    }

    int major = ver.at(0).toInt( &ok1 );
    int minor = ver.at(1).toInt( &ok2 );

    if( ( !ok1 ) || ( !ok2 ) ) {
        qWarning() << "Wrong hbwidget version format " << mReader.lineNumber();
        return false;
    }
    
    
    if( ( major > VERSION_MAJOR ) || ( major < MIN_SUPPORTED_VERSION_MAJOR ) ) {
        res = false;
    } else if( ( ( major == VERSION_MAJOR ) && ( minor > VERSION_MINOR )  ) || 
               ( ( major == MIN_SUPPORTED_VERSION_MAJOR ) && ( minor < MIN_SUPPORTED_VERSION_MINOR ) ) ) {
        res = false;
    }
    
    if( ! res ) {
        qWarning() << "Not supported document version " + ver_str + ". Current parser version is: " + version();
        return false;
                
    }

    return mActions->pushDocument( attribute( ATTR_CONTEXT ) );
}

/*!
    \internal
*/
bool HbWidgetLoaderSyntax::processLayout()
{
    const QString layout_type = attribute( ATTR_TYPE );
    const QString layout_name = attribute( ATTR_NAME );
    bool result = false;
    
    if( layout_type == lexemValue(LAYOUT_ANCHOR) ||
        layout_type == lexemValue(LAYOUT_MESH) ) {
        if( layout_name == mLayoutName ) {
            mCurrentLayoutType = LAYOUT_ANCHOR;
            mLayoutFound = true;
            result = mActions->createAnchorLayout( QString(), false );
        } else {
            mCurrentLayoutType = LAYOUT_ALIEN;
            result = true;
        }
    } else {        
        result =  HbXmlLoaderBaseSyntax::processLayout();
    }
    
    return result;
}

/*!
    \internal
*/
bool HbWidgetLoaderSyntax::readLayoutStartItem()
{
    bool result = false;
    switch( mCurrentLayoutType ) {
        case LAYOUT_ALIEN:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: ALIEN" );
            result = true;
            break;
        }
        case LAYOUT_ANCHOR: 
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: TARGET ANCHOR ITEM" );
            if( mReader.name() == lexemValue(AL_ANCHOR) ||
                mReader.name() == lexemValue(ML_MESHITEM) ) {
                result = readAnchorLayoutStartItem(true);
            }
            break;
        }
        default:
        {
            result = HbXmlLoaderBaseSyntax::readLayoutStartItem();
            break;
        }
    }
    return result;
}

/*!
    \internal
*/
QString HbWidgetLoaderSyntax::version()
{
    return ( QString::number( VERSION_MAJOR ) + QString( "." )
            + QString::number( VERSION_MINOR ) + QString( " (" )
            + QString::number( MIN_SUPPORTED_VERSION_MAJOR ) + QString( "." )
            + QString::number( MIN_SUPPORTED_VERSION_MINOR ) + QString( ")" ) );
}


/*
    \class HbWidgetLoaderMemorySyntax
    \internal
    \proto
*/

/*!
    \internal
*/
HbWidgetLoaderMemorySyntax::HbWidgetLoaderMemorySyntax( HbXmlLoaderAbstractActions *actions )
    : HbXmlLoaderAbstractSyntax(actions)
{
}

/*!
    \internal
*/
HbWidgetLoaderMemorySyntax::~HbWidgetLoaderMemorySyntax()
{
}
    
/*!
    \internal
*/
bool HbWidgetLoaderMemorySyntax::load( HbWidgetLoader::LayoutDefinition* layoutDef )
{
    bool retVal(true);

    // Construct layout from layout definition
    retVal = mActions->createAnchorLayout(QString(), false);
    for (int i = 0; retVal && i < layoutDef->anchorItems.count(); i++){
        const HbWidgetLoader::AnchorItem &item = layoutDef->anchorItems.at(i);
        HbXmlLengthValue minLength, prefLength, maxLength;


        for (int i=0; i<item.minVal.count(); i++) {
            minLength.mValues.append(item.minVal.at(i));
        }
        for (int j=0; j<item.prefVal.count(); j++) {
            prefLength.mValues.append(item.prefVal.at(j));
        }
        for (int k=0; k<item.maxVal.count(); k++) {
            maxLength.mValues.append(item.maxVal.at(k));
        }

        QSizePolicy::Policy sizepolicy;
        QSizePolicy::Policy *sizepolicy_p = 0;
        if ( item.sizepolicy != -1 ) {
            sizepolicy = (QSizePolicy::Policy)item.sizepolicy;
            sizepolicy_p = &sizepolicy;
        }
        HbAnchor::Direction direction;
        HbAnchor::Direction *direction_p = 0;
        if ( item.direction != -1 ) {
            direction = (HbAnchor::Direction)item.direction;
            direction_p = &direction;
        }

        retVal = mActions->addAnchorLayoutItem(
            QString(), item.srcId, item.srcEdge, QString(), item.dstId, item.dstEdge,
            minLength, prefLength, maxLength, sizepolicy_p, direction_p, item.anchorId );
    }

    return retVal;
}
