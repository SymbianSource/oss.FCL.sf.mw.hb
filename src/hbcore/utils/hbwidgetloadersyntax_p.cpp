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
    \class HbWidgetLoaderActions
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
#define VERSION_MINOR 2

#define MIN_SUPPORTED_VERSION_MAJOR 0
#define MIN_SUPPORTED_VERSION_MINOR 1


HbWidgetLoaderSyntax::HbWidgetLoaderSyntax( HbWidgetLoaderActions *actions ) :
    HbXmlLoaderAbstractSyntax( actions ), mRealActions( actions )
{
}

HbWidgetLoaderSyntax::~HbWidgetLoaderSyntax()
{
}

bool HbWidgetLoaderSyntax::load( QIODevice *device, const QString &name, const QString &section )
{
    //  HbXmlLoaderAbstractSyntax::load is not called here as it is coupled with the
    //  HbDeviceProfile. Due to this HbInstance will get created.
    //  That is why loadDevice() is called.

    mLayoutName = name;
    mLayoutFound = false;

    bool result = HbXmlLoaderAbstractSyntax::loadDevice(device,section);

    if ( result && !mLayoutFound ) {
        // File parsed ok, but requested layout not found.
        result = false;
        qWarning() << "Requested layout not found"; 
    }
    
    return result;
}

ElementType HbWidgetLoaderSyntax::elementType( QStringRef name ) const
{
    const QString stringName = name.toString();

    if( stringName == lexemValue(TYPE_HBWIDGET) ){
        return DOCUMENT;
    } 
    return HbXmlLoaderAbstractSyntax::elementType( name );
}

bool HbWidgetLoaderSyntax::processDocument()
{
    bool ok, ok1, ok2, res = true;
    
    QString ver_str =  attribute( ATTR_VERSION );
    
    ver_str.toDouble( &ok );
    QStringList ver = ver_str.split( "." ); 

        
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

    return mRealActions->pushDocument( attribute( ATTR_CONTEXT ) );
}


bool HbWidgetLoaderSyntax::processLayout()
{
    const QString layout_type = attribute( ATTR_TYPE );
    const QString layout_name = attribute( ATTR_NAME );
    bool result = false;
    
    if( layout_type == lexemValue(LAYOUT_MESH) ) {
        if( layout_name == mLayoutName ) {
            mCurrentLayoutType = LAYOUT_MESH_TARGET;
            mLayoutFound = true;

            mRealActions->mCacheLayout->meshItems.clear();
            mRealActions->mCacheLayout->spacers.clear();
            result = true;
        } else {
            mCurrentLayoutType = LAYOUT_MESH_ALIEN;
            result = true;
        }
    } else {        
        result =  HbXmlLoaderAbstractSyntax::processLayout();
    }
    
    return result;
}

bool HbWidgetLoaderSyntax::readLayoutStartItem()
{
    bool result = false;
    switch( mCurrentLayoutType ) {
        case LAYOUT_MESH_ALIEN:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: ALIEN MESH ITEM" );
            result = true;
            break;
        }
        case LAYOUT_MESH_TARGET: 
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: TARGET MESH ITEM" );
            if( mReader.name() == lexemValue(ML_MESHITEM) ) {
            
                const QString src = attribute( ML_SRC_NAME );  
                const QString dst = attribute( ML_DST_NAME );
                const QString srcEdge = attribute( ML_SRC_EDGE );
                const QString dstEdge = attribute( ML_DST_EDGE );
                const QString spacing = attribute( ML_SPACING );
                const QString spacer = attribute( ML_SPACER );
                
                                
                result = mRealActions->addMeshLayoutEdge( src, srcEdge, dst, dstEdge, spacing, spacer );
              
            }
            break;
        }
        default:
        {
            result = HbXmlLoaderAbstractSyntax::readLayoutStartItem();
            break;
        }
    }
    return result;
}

QString HbWidgetLoaderSyntax::version()
{
    return ( QString::number( VERSION_MAJOR ) + QString( "." )
            + QString::number( VERSION_MINOR ) + QString( " (" )
            + QString::number( MIN_SUPPORTED_VERSION_MAJOR ) + QString( "." )
            + QString::number( MIN_SUPPORTED_VERSION_MINOR ) + QString( ")" ) );
}
