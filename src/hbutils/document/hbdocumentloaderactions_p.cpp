/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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

#include "hbdocumentloaderactions_p.h"

#include <QCoreApplication>
#include <QGraphicsLinearLayout> 
#include <QGraphicsGridLayout>

#include <QMetaObject>
#include <QMetaEnum>

#include <hbanchorlayout.h>
#include <hbstackedlayout.h>
#include <hbdocumentloader.h>

#include "hbdocumentloader_p.h"
#include <hbwidget_p.h>
#include <hbwidgetbase_p.h>


class AccessToMetadata : public QObject
    {
    public:
        int getEnumValue( const char *enumeration, const char *str )
            {
                QMetaObject metaobject = staticQtMetaObject; 
                QMetaEnum e = metaobject.enumerator( metaobject.indexOfEnumerator( enumeration ) );
                return e.keysToValue( str );
            }
    };        

/*
    \class HbDocumentLoaderActions
    \internal
    \proto
*/

HbDocumentLoaderActions::HbDocumentLoaderActions( HbDocumentLoaderPrivate *ref ) : 
    HbXmlLoaderAbstractActions(), 
    d( ref )
{
}

HbDocumentLoaderActions::~HbDocumentLoaderActions()
{
    reset();    
}




QObject* HbDocumentLoaderActions::createObject( const QString& type, const QString &name, const QString &plugin )
{
    return d->lookUp( type, name, plugin );
}

QObject* HbDocumentLoaderActions::createObjectWithFactory( const QString& type, const QString &name )
{
    return factory.create(type, name);
}



bool HbDocumentLoaderActions::pushObject( const QString& type, const QString &name )
{
    QObject *parent = findFromStack();
 
    if ( !parent && name.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Top level object must have name" ) );
        return false;
    }

    QObject *current = lookUp(type, name);
    
    if( current == 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Not supported object: " ) + type );
        return false;
    }
    
    Element e;
    e.type = OBJECT;
    e.data = current;
    mStack.append( e );
    
    if (parent) {
        current->setParent(parent);
    }
    
    HB_DOCUMENTLOADER_PRINT( QString( "ADD ELEMENT " ) + name );

    return true;
}

bool HbDocumentLoaderActions::pushWidget( const QString& type, const QString &name, const QString &role, const QString &plugin )
{
    bool parentWidget = false;
    QObject *parent = findFromStack(&parentWidget);

    if ( !parent && name.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Top level widget must have name" ) );
        return false;
    }
    
    if ( parent && !parentWidget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Object element cannot be parent of widget" ) );
        return false;
    }    

    QObject *current = lookUp(type, name, plugin);
    
    if( current == 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Not supported object: " ) + type );
        return false;
    }
    
    QGraphicsWidget *parentAsWidget = qobject_cast<QGraphicsWidget *>(parent);
    QGraphicsWidget *asWidget = qobject_cast<QGraphicsWidget *>(current);

    if (!asWidget || (parent && !parentAsWidget)) {
        HB_DOCUMENTLOADER_PRINT( QString( "Not a widget" ) );
        return false;
    } 

    if (parentAsWidget && !setWidgetRole(parentAsWidget, asWidget, role)) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to set role" ) );
        return false;
    }

    Element e;
    e.type = WIDGET;
    e.data = current;
    mStack.append( e );
    HB_DOCUMENTLOADER_PRINT( QString( "ADD ELEMENT " ) + name );
    
    return true;
}

bool HbDocumentLoaderActions::pushSpacerItem( const QString &name, const QString &widget )
{
    if ( name.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "SpacerItem needs to have a name" ) );
        return false;
    }

    // find the widget which owns the spacer i.e. the parent
    HbWidget *parent = 0;

    if( widget.isEmpty() ) {
        bool isWidget = false;
        parent = qobject_cast<HbWidget *>( findFromStack( &isWidget ) );
        if( !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "SPACERITEM: CANNOT SET SPACERITEM TO NON-HBWIDGET " ) );
            return false;
        }
    } else if( !( mObjectMap.contains( widget ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "SPACERITEM: NO SUCH ITEM " ) + widget );
        return false;
    } else {   
        parent = qobject_cast<HbWidget *>( mObjectMap[ widget ].data() );
        if( !parent ) {
            HB_DOCUMENTLOADER_PRINT( QString( "SPACERITEM: CANNOT SET SPACERITEM TO NON-HBWIDGET " ) );
            return false;
        }
    }

    // look-up spacer item from widget
    QGraphicsLayoutItem *current = parent->layoutPrimitive( name );
    if ( !current ) {
        current =  static_cast<HbWidgetPrivate*>(HbWidgetBasePrivate::d_ptr(parent))->createSpacerItem(name);
    }

    // add it onto stack for further processing
    Element e;
    e.type = SPACERITEM;
    e.data = current;
    mStack.append( e );
    HB_DOCUMENTLOADER_PRINT( QString( "ADD ELEMENT " ) + name );
    
    return true;

}

bool HbDocumentLoaderActions::pushConnect( const QString &srcName, const QString &signalName, 
                                            const QString &dstName, const QString &slotName )
{
    if( srcName.isEmpty() || signalName.isEmpty() || dstName.isEmpty() || slotName.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Wrong parameters for signal/slot connection" ) );
        return false;
    }
    
    if( ! mObjectMap.contains( srcName ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, no instance with name " ) + srcName );
        return false;        
    }
    if( ! mObjectMap.contains( dstName ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, no instance with name " ) + dstName );
        return false;   
    }
    
    QObject *src = mObjectMap[ srcName ];

    if( !src ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, already destroyed " ) + srcName );
        return false;        
    }

    QObject *dst = mObjectMap[ dstName ];

    if( !dst ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, already destroyed " ) + dstName );
        return false;
    }
    
    const QMetaObject *msrc = src->metaObject();
    const QMetaObject *mdst = dst->metaObject();
    
    int signalIndex = msrc->indexOfSignal( QMetaObject::normalizedSignature( signalName.toLatin1() ) );
    int slotIndex = mdst->indexOfSlot( QMetaObject::normalizedSignature( slotName.toLatin1() ) );
    
    if( signalIndex == -1 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, no such signal " ) + signalName );
        return false;
    }
    
    if( slotIndex == -1 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, no such slot " ) + slotName );
        return false;
    }
    
    QMetaObject::connect(src, signalIndex, dst, slotIndex );    
    
    return true;
}

bool HbDocumentLoaderActions::pushProperty( const QString &propertyName, const QVariant &value )
{
    QObject *current = findFromStack();
    
    if( current == 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to set property " ) + propertyName );
        return false;
    }
    
        
    if( propertyName.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "No property name for " ) + propertyName );
        return false;
    } 
    
    QByteArray asLatin1 = propertyName.toLatin1();    
    current->setProperty( asLatin1, value );
    return true;    
}

bool HbDocumentLoaderActions::pushRef( const QString &name, const QString &role )
{
    QObject *current = findFromStack();
    QObject *ref = mObjectMap[ name ].data();
    
    if( ( current == 0 ) || ( ref == 0 ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Wrong role name or role context" ) );
        return false;        
    }

    if ( !setObjectRole(current, ref, role)) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to set role" ) );
        return false;
    }
    return true;
}

bool HbDocumentLoaderActions::setContentsMargins( qreal left, qreal top, qreal right, qreal bottom )
{
    bool isWidget = false;
    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(findFromStack(&isWidget));
    if( !isWidget || !widget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Cannot set contentsmargins for non-QGraphicsWidget" ) );
        return false;
    }
    widget->setContentsMargins( left, top, right, bottom );
    return true;
}


bool HbDocumentLoaderActions::setSizeHint(Qt::SizeHint hint, qreal *hintWidth, qreal *hintHeight, bool fixed)
{
    QGraphicsLayoutItem *current = findSpacerItemFromStackTop();
    if (!current) {
        bool isWidget = false;
        QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(findFromStack(&isWidget));
        if( !isWidget || !widget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "Cannot set sizehint for non-QGraphicsWidget" ) );
            return false;
        }
        current = widget;
    }

    switch (hint) {
    case Qt::MinimumSize: 
        if ( hintWidth ) {
            current->setMinimumWidth(*hintWidth);
        }
        if ( hintHeight ) {
            current->setMinimumHeight(*hintHeight);
        }
        break;

    case Qt::PreferredSize: 
        if ( hintWidth ) {
            current->setPreferredWidth(*hintWidth);
        }
        if ( hintHeight ) {
            current->setPreferredHeight(*hintHeight);
        }
        break;

    case Qt::MaximumSize: 
        if ( hintWidth ) {
            current->setMaximumWidth(*hintWidth);
        }
        if ( hintHeight ) {
            current->setMaximumHeight(*hintHeight);
        }
        break;

    default:
        break;
    }
    
    if (fixed) {
        QSizePolicy policy = current->sizePolicy();
        if ( hintWidth && *hintWidth >= 0) {
            policy.setHorizontalPolicy(QSizePolicy::Fixed);
        }
        if ( hintHeight && *hintHeight >= 0) {
            policy.setVerticalPolicy(QSizePolicy::Fixed);
        }
        current->setSizePolicy(policy);
    }

    return true;
}

bool HbDocumentLoaderActions::setZValue( qreal zValue )
{
    bool isWidget = false;
    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(findFromStack(&isWidget));
    if( !isWidget || !widget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Cannot set z value for non-QGraphicsWidget" ) );
        return false;
    }

    widget->setZValue( zValue );
    return true;
}

bool HbDocumentLoaderActions::setToolTip( const QString &tooltip )
{
    bool isWidget = false;
    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(findFromStack(&isWidget));
    if( !isWidget || !widget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Cannot set tooltip for non-QGraphicsWidget" ) );
        return false;
    }

    widget->setToolTip( tooltip );
    return true;
}

bool HbDocumentLoaderActions::setSizePolicy( 
    const QSizePolicy::Policy *horizontalPolicy, 
    const QSizePolicy::Policy *verticalPolicy, 
    const int *horizontalStretch,
    const int *verticalStretch )
{
    QGraphicsLayoutItem *current = findSpacerItemFromStackTop();
    if (!current) {
        bool isWidget = false;
        QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(findFromStack(&isWidget));
        if( !isWidget || !widget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "Cannot set size policy for non-QGraphicsWidget" ) );
            return false;
        }
        current = widget;
    }

    bool changed = false;
    QSizePolicy sizePolicy = current->sizePolicy();

    if ( horizontalPolicy && (*horizontalPolicy != sizePolicy.horizontalPolicy() ) ) {
        sizePolicy.setHorizontalPolicy( *horizontalPolicy );
        changed = true;
    }
    if ( verticalPolicy && (*verticalPolicy != sizePolicy.verticalPolicy() ) ) {
        sizePolicy.setVerticalPolicy( *verticalPolicy );
        changed = true;
    }

    if ( horizontalStretch && ( *horizontalStretch != sizePolicy.horizontalStretch() ) ) {
        sizePolicy.setHorizontalStretch( uchar( *horizontalStretch ) );
        changed = true;
    }

    if ( verticalStretch && ( *verticalStretch != sizePolicy.verticalStretch() ) ) {
        sizePolicy.setVerticalStretch( uchar( *verticalStretch ) );
        changed = true;
    }

    if ( changed ) {
        current->setSizePolicy( sizePolicy );
    }

    return true;
}


bool HbDocumentLoaderActions::createAnchorLayout( const QString &widget )
{
    QGraphicsWidget *parent = 0;
    
    if( widget.isEmpty() ) {
        bool isWidget = false;
        parent = qobject_cast<QGraphicsWidget *>( findFromStack( &isWidget ) );
        if( !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: CANNOT SET LAYOUT TO NON-QGRAPHICSWIDGET " ) );
            return false;
        }
    } else if( !( mObjectMap.contains( widget ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO SUCH ITEM " ) + widget );
        return false;
    } else {   
        parent = qobject_cast<QGraphicsWidget *>( mObjectMap[ widget ].data() );
    }
    
    mCurrentLayout = new HbAnchorLayout();  
    
    parent->setLayout( mCurrentLayout );
    
    return true;
}

QGraphicsLayoutItem *findLayoutItem( const QGraphicsLayout &layout, const QString &layoutItemName ) 
{
    QGraphicsLayoutItem *result = 0;
    if ( layout.parentLayoutItem() ) {
        QGraphicsItem *asGraphicsItem = layout.parentLayoutItem()->graphicsItem();
        if ( asGraphicsItem && asGraphicsItem->isWidget() ){
            HbWidget *asWidget = qobject_cast<HbWidget*>( static_cast<QGraphicsWidget*>(asGraphicsItem) );
            if( asWidget ) {
                result = asWidget->layoutPrimitive( layoutItemName );
            }
        }
    }
    return result;
}

bool HbDocumentLoaderActions::addAnchorLayoutEdge( const QString &src, const QString &srcEdge, 
                                                    const QString &dst, const QString &dstEdge, qreal spacing, const QString &spacer )
{
    if ( !spacer.isEmpty() ) {
        // spacer is added
        // divide original anchor definition into two. src->dst becomes src->spacer->dst
        bool ok = true;
        if ( src.isEmpty() ) {
            // if the starting item is layout
            // "layout --(spacing)--> item" 
            // becomes 
            // "layout --(spacing)--> spacer --(0)--> item"
            ok &= addAnchorLayoutEdge( src, srcEdge, spacer, srcEdge, spacing );
            ok &= addAnchorLayoutEdge( spacer, getAnchorOppositeEdge(srcEdge), dst, dstEdge, 0 );
        } else {
            // if the starting item is not layout
            // "item1 --(spacing)--> item2" 
            // becomes 
            // "item1 --(spacing)--> spacer --(0)--> item2"
            ok &= addAnchorLayoutEdge( src, srcEdge, spacer, getAnchorOppositeEdge(srcEdge), spacing );
            ok &= addAnchorLayoutEdge( spacer, srcEdge, dst, dstEdge, 0 );
        }
        return ok;
    }

    QGraphicsLayoutItem *item1 = 0;
    QGraphicsLayoutItem *item2 = 0;
    
    HbAnchorLayout *layout = static_cast<HbAnchorLayout *>( mCurrentLayout );
    
    if( src.isEmpty() ) {
        item1 = layout;
    } else if( !( mObjectMap.contains( src ) ) ) {
        item1 = findLayoutItem( *layout, src );
        if ( !item1 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO SUCH ITEM " ) + src );
            return false;
        }
    } else {
        item1 = qobject_cast<QGraphicsWidget *>( mObjectMap[ src ].data() );
    }
    
    if( dst.isEmpty() ) {
        item2 = layout;
    } else if( !( mObjectMap.contains( dst ) ) ) {
        item2 = findLayoutItem( *layout, dst );
        if ( !item2 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO SUCH ITEM " ) + dst );
            return false;
        }
    } else {
        item2 = qobject_cast<QGraphicsWidget *>( mObjectMap[ dst ].data() );
    }  
    
    int edge1 = getAnchorEdge( srcEdge );
    int edge2 = getAnchorEdge( dstEdge );
    
    if( edge1 < 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: UNKNOWN EDGE " ) + srcEdge );
        return false;
    }

    if( edge2 < 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: UNKNOWN EDGE " ) + dstEdge );
        return false;
    }

    layout->setAnchor( item1, ( HbAnchorLayout::Edge )edge1, item2, ( HbAnchorLayout::Edge )edge2, spacing );    
    return true;
}


bool HbDocumentLoaderActions::createGridLayout( const QString &widget, qreal *spacing )
{       
    QGraphicsWidget *parent = 0;
    
    if( widget.isEmpty() ) {
        bool isWidget = false;
        parent = qobject_cast<QGraphicsWidget *>( findFromStack( &isWidget ) );
        if( !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: CANNOT SET LAYOUT TO NON-QGRAPHICSWIDGET " ) );
            return false;
        }
    } else if( !( mObjectMap.contains( widget ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ITEM " ) + widget );
        return false;
    } else {   
        parent = qobject_cast<QGraphicsWidget *>( mObjectMap[ widget ].data() );
    }
    
    QGraphicsGridLayout* layout = new QGraphicsGridLayout();  
    if (spacing) {
        layout->setSpacing(*spacing);
    }

    mCurrentLayout = layout;    
    parent->setLayout( mCurrentLayout );
    
    return true;
}

bool HbDocumentLoaderActions::addGridLayoutCell( const QString &src, const QString &row, 
                                                 const QString &column, const QString &rowspan, const QString &columnspan,
                                                 const QString &alignment )
{
    QGraphicsLayoutItem *item = 0;
    
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) + src );
        return false;        
    }
    
    if( src.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: TRY TO ADD EMPTY ITEM " ) + src );
        return false;
    } else if( !( mObjectMap.contains( src ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ITEM " ) + src );
        return false;
    } else {
        item = qobject_cast<QGraphicsWidget *>( mObjectMap[ src ].data() );
    }
    
    bool ok = false;
    int rownum = row.toInt( &ok );
    if( !ok ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO ROW SPECIFIED" ) );
        return false;                        
    } 

    int columnnum = column.toInt( &ok );
    if( !ok ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO COLUMN SPECIFIED" ) );
        return false;                    
    } 
    
    int rowspannum = rowspan.toInt( &ok );
    if( !ok ) {
        rowspannum = 1;
    }                  

    int columnspannum = columnspan.toInt( &ok );
    if( !ok ) {
        columnspannum = 1;
    }                   
    
    Qt::Alignment align = 0;
    if( !alignment.isEmpty() ) {
        AccessToMetadata myAccess;
                
        int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
        if( value == -1 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ALIGNMENT " ) + alignment );
            return false;            
        } 
        align = ( Qt::Alignment )value;
    }

    layout->addItem( item, rownum, columnnum, rowspannum, columnspannum, align );
    
    return true;
}

bool HbDocumentLoaderActions::setGridLayoutRowProperties( const QString &row, const QString &rowStretchFactor, 
                                                          const QString &alignment )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }

    bool ok = false;
    const int rownum = row.toInt( &ok );
    if( !ok ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO ROW NUMBER SPECIFIED FOR STRETCH FACTOR" ) );
        return false;                        
    } 

    if( !rowStretchFactor.isEmpty() ) {
        bool ok = false;
        int rowStretch = rowStretchFactor.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: UNABLE TO PARSE ROW STRETCH FACTOR VALUE" ) );
            return false;                        
        } 
        layout->setRowStretchFactor( rownum, rowStretch );
    }

    if( !alignment.isEmpty() ) {
        AccessToMetadata myAccess;
                
        int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
        if( value == -1 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ROW ALIGNMENT " ) + alignment );
            return false;            
        } 
        layout->setRowAlignment(rownum, ( Qt::Alignment )value );
    }

    return true;
}

bool HbDocumentLoaderActions::setGridLayoutColumnProperties( const QString &column, const QString &columnStretchFactor,
                                                          const QString &alignment )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }

    bool ok = false;
    const int columnnum = column.toInt( &ok );
    if( !ok ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO COLUMN NUMBER SPECIFIED FOR STRETCH FACTOR" ) );
        return false;                        
    } 

    if( !columnStretchFactor.isEmpty() ) {
        bool ok = false;
        int columnStretch = columnStretchFactor.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: UNABLE TO PARSE COLUMN STRETCH FACTOR VALUE" ) );
            return false;                        
        } 
        layout->setColumnStretchFactor( columnnum, columnStretch );
    }

    if( !alignment.isEmpty() ) {
        AccessToMetadata myAccess;
                
        int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
        if( value == -1 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH COLUMN ALIGNMENT " ) + alignment );
            return false;            
        } 
        layout->setColumnAlignment( columnnum, ( Qt::Alignment )value );
    }

    return true;
}

bool HbDocumentLoaderActions::setGridLayoutRowHeights( const QString &row, const qreal minHeight, 
                                                       const qreal maxHeight, const qreal prefHeight,
                                                       const qreal fixedHeight, const qreal rowSpacing, 
                                                       const int flagsPropertyAvailable )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }

    bool ok = false;
    const int rownum = row.toInt( &ok );
    if( !ok ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO ROW NUMBER SPECIFIED FOR ROW HEIGHTS" ) );
        return false;                        
    } 

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyMin ) {
        layout->setRowMinimumHeight( rownum, minHeight );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyMax ) {
        layout->setRowMaximumHeight( rownum, maxHeight );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyPref ) {
        layout->setRowPreferredHeight( rownum, prefHeight );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyFixed ) {
        layout->setRowFixedHeight( rownum, fixedHeight );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertySpacing ) {
        layout->setRowSpacing( rownum, rowSpacing );
    }

    return true;

}

bool HbDocumentLoaderActions::setGridLayoutColumnWidths( const QString &column, const qreal minWidth, 
                                                         const qreal maxWidth, const qreal prefWidth, 
                                                         const qreal fixedWidth, const qreal columnSpacing,
                                                         const int flagsPropertyAvailable )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }

    bool ok = false;
    const int columnnum = column.toInt( &ok );
    if( !ok ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO COLUMN NUMBER SPECIFIED FOR COLUMN WIDTHS" ) );
        return false;                        
    } 

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyMin ) {
        layout->setColumnMinimumWidth( columnnum, minWidth );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyMax ) {
        layout->setColumnMaximumWidth( columnnum, maxWidth );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyPref ) {
        layout->setColumnPreferredWidth( columnnum, prefWidth );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertyFixed ) {
        layout->setColumnFixedWidth( columnnum, fixedWidth );
    }

    if ( flagsPropertyAvailable & HbDocumentLoaderActions::propertySpacing ) {
        layout->setColumnSpacing( columnnum, columnSpacing );
    }

    return true;
}

bool HbDocumentLoaderActions::createLinearLayout( const QString &widget, const QString &orientation, qreal *spacing )
{
    QGraphicsWidget *parent = 0;
    QGraphicsLinearLayout *layout = 0;
    
    if( widget.isEmpty() ) {
        bool isWidget = false;
        parent = qobject_cast<QGraphicsWidget *>( findFromStack( &isWidget ) );
        if( !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: CANNOT SET LAYOUT TO NON-QGRAPHICSWIDGET " ) );
            return false;
        }
    } else if( !( mObjectMap.contains( widget ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ITEM " ) + widget );
        return false;
    } else {   
        parent = qobject_cast<QGraphicsWidget *>( mObjectMap[ widget ].data() );
    }
    
    Qt::Orientation orient = Qt::Horizontal;
    
    if( ! orientation.isEmpty() ) {
        AccessToMetadata myAccess;
                
        int value = myAccess.getEnumValue( "Orientation", orientation.toLatin1().data() );
        if( value == -1 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ORIENTATION " ) + orientation );
            return false;            
        } 
        orient = ( Qt::Orientation )value;
        layout = new QGraphicsLinearLayout( orient );
    } else {
        layout = new QGraphicsLinearLayout();
    }  
    
    if ( spacing ) {
        layout->setSpacing(*spacing);
    }

    mCurrentLayout = layout;
    parent->setLayout( mCurrentLayout );
    
    return true;
}

bool HbDocumentLoaderActions::addLinearLayoutItem( const QString &itemname, const QString &index, 
                                                   const QString &stretchfactor, const QString &alignment,
                                                   qreal *spacing )
{
    QGraphicsLayoutItem *item = 0;
    
    QGraphicsLinearLayout *layout = static_cast<QGraphicsLinearLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }
    
    if( itemname.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: TRY TO ADD EMPTY ITEM " ) + itemname );
        return false;
    } else if( !( mObjectMap.contains( itemname ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ITEM " ) + itemname );
        return false;
    } else {
        item = qobject_cast<QGraphicsWidget *>( mObjectMap[ itemname ].data() );
    }
    
    int indexValue = -1;
    
    if( ! index.isEmpty() ) {
        bool ok = false;
        indexValue = index.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE ITEM INDEX" ) );
            return false;                        
        } 
    }
    
    layout->insertItem( indexValue, item );
    if ( spacing ) {
        // Need to resolve the item index for spacing
        int i = layout->count();
        while (i--) {
            if ( layout->itemAt(i) == item ) {
                layout->setItemSpacing(i, *spacing);
                break;
            }
        }
    }
    
    if( !stretchfactor.isEmpty() ) {
        bool ok = false;
        int stretch = stretchfactor.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE STRETCH VALUE" ) );
            return false;                        
        } 
        layout->setStretchFactor( item, stretch );
    }
    
    if( !alignment.isEmpty() ) {
        AccessToMetadata myAccess;
                
        int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
        if( value == -1 ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ITEM ALIGNMENT " ) + alignment );
            return false;            
        } 
        layout->setAlignment( item, ( Qt::Alignment )value );
    }
    
    return true;
}

bool HbDocumentLoaderActions::addLinearLayoutStretch( const QString &index, const QString &stretchfactor )
{
    QGraphicsLinearLayout *layout = static_cast<QGraphicsLinearLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }
    
    int indexValue = -1;
    int stretch = 1;
    
    if( ! index.isEmpty() ) {
        bool ok = false;
        indexValue = index.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE STRETCH INDEX" ) );
            return false;                        
        } 
    }

    if( ! stretchfactor.isEmpty() ) {
        bool ok = false;
        stretch = stretchfactor.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE STRETCH VALUE" ) );
            return false;                        
        } 
    }
    
    layout->insertStretch( indexValue, stretch );
    
    return true;
}

bool HbDocumentLoaderActions::setLayoutContentsMargins( qreal left, qreal top, qreal right, qreal bottom )
{
    if( !mCurrentLayout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LAYOUT: INTERNAL ERROR " ) );
        return false;        
    }
    mCurrentLayout->setContentsMargins( left, top, right, bottom );
    return true;
}

bool HbDocumentLoaderActions::createStackedLayout( const QString &widget )
{
    QGraphicsWidget *parent = 0;
    
    if( widget.isEmpty() ) {
        bool isWidget = false;
        parent = qobject_cast<QGraphicsWidget *>( findFromStack( &isWidget ) );
        if( !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: CANNOT SET LAYOUT TO NON-QGRAPHICSWIDGET " ) );
            return false;
        }
    } else if( !( mObjectMap.contains( widget ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: NO SUCH ITEM " ) + widget );
        return false;
    } else {   
        parent = qobject_cast<QGraphicsWidget *>( mObjectMap[ widget ].data() );
    }
    
    mCurrentLayout = new HbStackedLayout();
    
    parent->setLayout( mCurrentLayout );
    
    return true;
}

bool HbDocumentLoaderActions::addStackedLayoutItem( const QString &itemname, const QString &index )
{
    QGraphicsLayoutItem *item = 0;
    
    HbStackedLayout *layout = static_cast<HbStackedLayout *>( mCurrentLayout );
    
    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: INTERNAL ERROR " ) );
        return false;        
    }
    
    if( itemname.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: TRY TO ADD EMPTY ITEM " ) + itemname );
        return false;
    } else if( !( mObjectMap.contains( itemname ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: NO SUCH ITEM " ) + itemname );
        return false;
    } else {
        item = qobject_cast<QGraphicsWidget *>( mObjectMap[ itemname ].data() );
    }
    
    int indexValue = -1;
    
    if( ! index.isEmpty() ) {
        bool ok = false;
        indexValue = index.toInt( &ok );
        if( !ok ) {
            HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: UNABLE TO PARSE ITEM INDEX" ) );
            return false;                        
        } 
    }
    
    layout->insertItem( indexValue, item );
    
    return true;
}


bool HbDocumentLoaderActions::createNullLayout( const QString &widget )
{
    QGraphicsWidget *parent = 0;
    
    if( widget.isEmpty() ) {
        bool isWidget = false;
        parent = qobject_cast<QGraphicsWidget *>( findFromStack( &isWidget ) );
        if( !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "NULL LAYOUT: CANNOT UNSET LAYOUT FROM NON-QGRAPHICSWIDGET " ) );
            return false;
        }
    } else if( !( mObjectMap.contains( widget ) ) ) {
        HB_DOCUMENTLOADER_PRINT( QString( "NULL LAYOUT: NO SUCH ITEM " ) + widget );
        return false;
    } else {   
        parent = qobject_cast<QGraphicsWidget *>( mObjectMap[ widget ].data() );
    }
    
    mCurrentLayout = 0;  
    
    parent->setLayout( mCurrentLayout );
    
    return true;
    
}

bool HbDocumentLoaderActions::createContainer()
{
    if (mCurrentContainer) {
        delete mCurrentContainer;
    }
    mCurrentContainer = new QList<QVariant>();      
    return true;
}

bool HbDocumentLoaderActions::appendPropertyToContainer( const QVariant &value )
{
    bool result(false);
    if (!mCurrentContainer) {
        result = false;
    } else {
        // note that for a successful conversion later on, all of the appended items need
        // to be of the same (appropriate type) e.g. String
        mCurrentContainer->append(value);
        result = true;
    }
    return result;
}

bool HbDocumentLoaderActions::setWidgetRole(
    QGraphicsWidget *parent, QGraphicsWidget *child, const QString &role)
{
    return factory.setWidgetRole(parent, child, role);
}

bool HbDocumentLoaderActions::setObjectRole(
    QObject *parent, QObject *child, const QString &role)
{
    return factory.setObjectRole(parent, child, role);
}


