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
#include <hbxmlloaderabstractsyntax_p.h>

#include <QCoreApplication>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#include <QMetaObject>
#include <QMetaEnum>
#include <QDebug>

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

HbDocumentLoaderActions::HbDocumentLoaderActions( HbDocumentLoaderPrivate *ref, const HbMainWindow *window ) :
    HbXmlLoaderBaseActions(),
    d( ref )
{
    if ( window ) {
        mCurrentProfile = HbDeviceProfile::profile(window);
    } else {
        mCurrentProfile = HbDeviceProfile::current();
    }
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
    return mFactory.create(type, name);
}



bool HbDocumentLoaderActions::pushObject( const QString& type, const QString &name )
{
    QObject *parent = findFromStack();

    if ( !parent && name.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Top level object must have name" ) );
        return false;
    }

    QObject *current = lookUp(type, name).mObject.data();

    if( current == 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Not supported object: " ) + type );
        return false;
    }

    HbXml::Element e;
    e.type = HbXml::OBJECT;
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
    bool parentIsWidget = false;
    QObject *parent = findFromStack(&parentIsWidget);

    if ( !parent && name.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Top level widget must have name" ) );
        return false;
    }

    if ( parent && !parentIsWidget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Object element cannot be parent of widget" ) );
        return false;
    }

    ObjectMapItem item = lookUp(type, name, plugin);
    QObject *current = item.mObject.data();

    if( current == 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Not supported object: " ) + type );
        return false;
    }

    QGraphicsWidget *parentAsWidget(0);
    if (parentIsWidget) {
        parentAsWidget = static_cast<QGraphicsWidget *>(parent);
    }
    QGraphicsWidget *asWidget(0);
    if (item.mType == HbXml::WIDGET) {
        asWidget = static_cast<QGraphicsWidget *>(current);
    }

    if (!asWidget || (parent && !parentAsWidget)) {
        HB_DOCUMENTLOADER_PRINT( QString( "Not a widget" ) );
        return false;
    }

    if (parentAsWidget && !setWidgetRole(parentAsWidget, asWidget, role)) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to set role" ) );
        return false;
    }

    HbXml::Element e;
    e.type = HbXml::WIDGET;
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
        ObjectMapItem &item = mObjectMap[ widget ];
        if (item.mType == HbXml::WIDGET) {
            parent = qobject_cast<HbWidget *>( item.mObject.data() );
        }
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
    HbXml::Element e;
    e.type = HbXml::SPACERITEM;
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

    QObject *src = mObjectMap[ srcName ].mObject;

    if( !src ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to establish signal/slot connection, already destroyed " ) + srcName );
        return false;
    }

    QObject *dst = mObjectMap[ dstName ].mObject;

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

bool HbDocumentLoaderActions::pushProperty( const char *propertyName, const HbXmlVariable &variable )
{
    QObject *current = findFromStack();

    if( current == 0 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Unable to set property " ) + propertyName );
        return false;
    }

    QVariant value;
    bool ok = variableToQVariant(variable, value);
    if (ok) {
        bool isQProperty = current->setProperty( propertyName, value );
        if ( !isQProperty ) {
            qWarning() << "DOCML warning: Dynamic property" << propertyName << "set. Property may have been removed from" << current->metaObject()->className();
        }
    }
    return ok;
}

bool HbDocumentLoaderActions::pushRef( const QString &name, const QString &role )
{
    QObject *current = findFromStack();
    QObject *ref = mObjectMap[ name ].mObject.data();

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

bool HbDocumentLoaderActions::pushContainer( const char *propertyName,
                                             HbXmlLoaderAbstractSyntax::DocumentLexems type,
                                             const QList<HbXmlVariable*> &container )
{
    bool result = true;
    if ( type == HbXmlLoaderAbstractSyntax::CONTAINER_STRINGLIST ) {
        QStringList list;
        for ( int i=0; i<container.count(); i++ ) {
            QVariant variant;
            result = variableToQVariant(*(container.value(i)), variant);
            if ( result ) {
                list.append( variant.toString() );
            }
        }
        if ( result ) {
            QObject *current = findFromStack();

            if (current == 0) {
                HB_DOCUMENTLOADER_PRINT( QString( "Unable to set property " ) + propertyName );
                result = false;
            }

            if (result) {
                current->setProperty( propertyName, list );
            }
        }
    } else {
        result = false;
    }

    return result;
}

bool HbDocumentLoaderActions::setContentsMargins( const HbXmlLengthValue &left,
                                                 const HbXmlLengthValue &top,
                                                 const HbXmlLengthValue &right,
                                                 const HbXmlLengthValue &bottom )
{
    bool isWidget = false;
    QObject* obj = findFromStack(&isWidget);
    if( !obj || !isWidget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Cannot set contentsmargins for non-QGraphicsWidget" ) );
        return false;
    }
    QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(obj);

    qreal leftVal=0, topVal=0, rightVal=0, bottomVal=0;
    bool ok = true;
    if (left.mType != HbXmlLengthValue::None) {
        ok &= toPixels(left, leftVal);
    }
    if (top.mType != HbXmlLengthValue::None) {
        ok &= toPixels(top, topVal);
    }
    if (right.mType != HbXmlLengthValue::None) {
        ok &= toPixels(right, rightVal);
    }
    if (bottom.mType != HbXmlLengthValue::None) {
        ok &= toPixels(bottom, bottomVal);
    }
    if ( ok ) {
        widget->setContentsMargins( leftVal, topVal, rightVal, bottomVal );
    }
    return ok;
}


bool HbDocumentLoaderActions::setSizeHint(Qt::SizeHint hint, const HbXmlLengthValue &hintWidth, const HbXmlLengthValue &hintHeight, bool fixed)
{
    QGraphicsLayoutItem *current = findSpacerItemFromStackTop();
    if (!current) {
        bool isWidget = false;
        QObject* obj = findFromStack(&isWidget);
        if( !obj || !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "Cannot set sizehint for non-QGraphicsWidget" ) );
            return false;
        }
        QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(obj);
        current = widget;
    }
    qreal hintWidthVal, hintHeightVal;

    bool ok = true;
    if ( hintWidth.mType != HbXmlLengthValue::None ) {
        ok &= toPixels(hintWidth, hintWidthVal);
    }
    if ( hintHeight.mType != HbXmlLengthValue::None ) {
        ok &= toPixels(hintHeight, hintHeightVal);
    }
    if (!ok) {
        return false;
    }

    // TODO: Use set <Min/Pref/Max> Size if both declared. It's more efficient.
    switch (hint) {
    case Qt::MinimumSize:
        if ( hintWidth.mType != HbXmlLengthValue::None ) {

            current->setMinimumWidth(hintWidthVal);
        }
        if ( hintHeight.mType != HbXmlLengthValue::None ) {
            current->setMinimumHeight(hintHeightVal);
        }
        break;

    case Qt::PreferredSize:
        if ( hintWidth.mType != HbXmlLengthValue::None ) {
            current->setPreferredWidth(hintWidthVal);
        }
        if ( hintHeight.mType != HbXmlLengthValue::None ) {
            current->setPreferredHeight(hintHeightVal);
        }
        break;

    case Qt::MaximumSize:
        if ( hintWidth.mType != HbXmlLengthValue::None ) {
            current->setMaximumWidth(hintWidthVal);
        }
        if ( hintHeight.mType != HbXmlLengthValue::None ) {
            current->setMaximumHeight(hintHeightVal);
        }
        break;

    default:
        break;
    }

    if (fixed) {
        QSizePolicy policy = current->sizePolicy();
        if ( hintWidth.mType != HbXmlLengthValue::None && hintWidthVal >= 0) {
            policy.setHorizontalPolicy(QSizePolicy::Fixed);
        }
        if ( hintHeight.mType != HbXmlLengthValue::None && hintHeightVal >= 0) {
            policy.setVerticalPolicy(QSizePolicy::Fixed);
        }
        current->setSizePolicy(policy);
    }

    return true;
}

bool HbDocumentLoaderActions::setToolTip( const HbXmlVariable &tooltip )
{
    bool isWidget;
    QObject* obj = findFromStack(&isWidget);
    if( !obj || !isWidget ) {
        HB_DOCUMENTLOADER_PRINT( QString( "Cannot set tooltip for non-QGraphicsWidget" ) );
        return false;
    }
    QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(obj);

    QVariant variant;
    bool result = variableToQVariant( tooltip, variant );
    if ( result ) {
        widget->setToolTip( variant.toString() );
    }
    return result;
}

bool HbDocumentLoaderActions::setSizePolicy(
    QSizePolicy::Policy *horizontalPolicy,
    QSizePolicy::Policy *verticalPolicy,
    int *horizontalStretch,
    int *verticalStretch )
{
    QGraphicsLayoutItem *current = findSpacerItemFromStackTop();
    if (!current) {
        bool isWidget = false;
        QObject* obj = findFromStack(&isWidget);
        if( !obj || !isWidget ) {
            HB_DOCUMENTLOADER_PRINT( QString( "Cannot set size policy for non-QGraphicsWidget" ) );
            return false;
        }
        QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(obj);
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
        QObject *parentObj = findFromStack( &isWidget );
        if( isWidget ) {
            parent = static_cast<QGraphicsWidget *>( parentObj );
        }
    } else if ( mObjectMap.contains( widget ) && mObjectMap[ widget ].mType == HbXml::WIDGET ) {
        parent = static_cast<QGraphicsWidget *>( mObjectMap[ widget ].mObject.data() );
    }
    if ( !parent ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: PARENT NOT FOUND" ) );
        return false;
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

bool HbDocumentLoaderActions::addAnchorLayoutEdge( const QString &src, Hb::Edge srcEdge,
                                                   const QString &dst, Hb::Edge dstEdge,
                                                   const HbXmlLengthValue &spacing, const QString &spacer )
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
            HbXmlLengthValue val(0, HbXmlLengthValue::Pixel);
            ok &= addAnchorLayoutEdge( spacer, getAnchorOppositeEdge(srcEdge), dst, dstEdge, val );
        } else {
            // if the starting item is not layout
            // "item1 --(spacing)--> item2"
            // becomes
            // "item1 --(spacing)--> spacer --(0)--> item2"
            ok &= addAnchorLayoutEdge( src, srcEdge, spacer, getAnchorOppositeEdge(srcEdge), spacing );
            HbXmlLengthValue val(0, HbXmlLengthValue::Pixel);
            ok &= addAnchorLayoutEdge( spacer, srcEdge, dst, dstEdge, val );
        }
        return ok;
    }

    QGraphicsLayoutItem *item1 = 0;
    QGraphicsLayoutItem *item2 = 0;

    HbAnchorLayout *layout = static_cast<HbAnchorLayout *>( mCurrentLayout );

    if ( src.isEmpty() ) {
        item1 = layout;
    } else if ( !( mObjectMap.contains( src ) ) ) {
        item1 = findLayoutItem( *layout, src );
    } else {
        if (mObjectMap[ src ].mType == HbXml::WIDGET) {
            item1 = static_cast<QGraphicsWidget *>( mObjectMap[ src ].mObject.data() );
        }
    }
    if ( !item1 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO SUCH ITEM " ) + src );
        return false;
    }

    if ( dst.isEmpty() ) {
        item2 = layout;
    } else if( !( mObjectMap.contains( dst ) ) ) {
        item2 = findLayoutItem( *layout, dst );
    } else {
        if (mObjectMap[ dst ].mType == HbXml::WIDGET) {
            item2 = static_cast<QGraphicsWidget *>( mObjectMap[ dst ].mObject.data() );
        }
    }
    if ( !item2 ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO SUCH ITEM " ) + dst );
        return false;
    }

    qreal spacingVal(0);
    if ( spacing.mType != HbXmlLengthValue::None && !toPixels(spacing, spacingVal) ) {
        return false;
    }
    layout->setAnchor( item1, srcEdge, item2, dstEdge, spacingVal );
    return true;
}


bool HbDocumentLoaderActions::createGridLayout( const QString &widget, const HbXmlLengthValue &spacing )
{
    QGraphicsWidget *parent = 0;

    if( widget.isEmpty() ) {
        bool isWidget = false;
        QObject *parentObj = findFromStack( &isWidget );
        if( isWidget ) {
            parent = static_cast<QGraphicsWidget *>( parentObj );
        }
    } else if ( mObjectMap.contains( widget ) && mObjectMap[ widget ].mType == HbXml::WIDGET ) {
        parent = static_cast<QGraphicsWidget *>( mObjectMap[ widget ].mObject.data() );
    }
    if ( !parent ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: PARENT NOT FOUND" ) );
        return false;
    }

    QGraphicsGridLayout* layout = new QGraphicsGridLayout();
    if (spacing.mType != HbXmlLengthValue::None) {
        qreal spacingVal;
        if ( toPixels(spacing, spacingVal) ) {
            layout->setSpacing(spacingVal);
        } else {
            delete layout;
            return false;
        }
    }

    mCurrentLayout = layout;
    parent->setLayout( mCurrentLayout );

    return true;
}

bool HbDocumentLoaderActions::addGridLayoutCell(
    const QString &src,
    int row,
    int column,
    int *rowspan,
    int *columnspan,
    Qt::Alignment *alignment )
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
    } else if ( mObjectMap.contains( src ) && mObjectMap[ src ].mType == HbXml::WIDGET ) {
        item = static_cast<QGraphicsWidget *>( mObjectMap[ src ].mObject.data() );
    } else {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ITEM " ) + src );
        return false;
    }

    int rowspannum = rowspan ? *rowspan : 1;
    int columnspannum = columnspan ? *columnspan : 1;
    Qt::Alignment align = alignment ? *alignment : (Qt::Alignment)0;

    layout->addItem( item, row, column, rowspannum, columnspannum, align );

    return true;
}

bool HbDocumentLoaderActions::setGridLayoutRowProperties(
    int row,
    int *rowStretchFactor,
    Qt::Alignment *alignment )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );

    if (!layout) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    if (rowStretchFactor) {
        layout->setRowStretchFactor( row, *rowStretchFactor );
    }

    if (alignment) {
        layout->setRowAlignment( row, *alignment );
    }

    return true;
}

bool HbDocumentLoaderActions::setGridLayoutColumnProperties(
    int column,
    int *columnStretchFactor,
    Qt::Alignment *alignment )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );

    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    if (columnStretchFactor) {
        layout->setColumnStretchFactor( column, *columnStretchFactor );
    }

    if (alignment) {
        layout->setColumnAlignment( column, *alignment );
    }

    return true;
}

bool HbDocumentLoaderActions::setGridLayoutRowHeights( int row,
                                                       const HbXmlLengthValue &minHeight,
                                                       const HbXmlLengthValue &maxHeight,
                                                       const HbXmlLengthValue &prefHeight,
                                                       const HbXmlLengthValue &fixedHeight,
                                                       const HbXmlLengthValue &rowSpacing )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );

    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    if ( minHeight.mType != HbXmlLengthValue::None ) {
        qreal minHeightVal;
        if ( !toPixels(minHeight, minHeightVal) ) {
            return false;
        }
        layout->setRowMinimumHeight( row, minHeightVal );
    }

    if ( maxHeight.mType != HbXmlLengthValue::None ) {
        qreal maxHeightVal;
        if ( !toPixels(maxHeight, maxHeightVal) ) {
            return false;
        }
        layout->setRowMaximumHeight( row, maxHeightVal );
    }

    if ( prefHeight.mType != HbXmlLengthValue::None ) {
        qreal prefHeightVal;
        if ( !toPixels(prefHeight, prefHeightVal) ) {
            return false;
        }
        layout->setRowPreferredHeight( row, prefHeightVal );
    }

    if ( fixedHeight.mType != HbXmlLengthValue::None ) {
        qreal fixedHeightVal;
        if ( !toPixels(fixedHeight, fixedHeightVal) ) {
            return false;
        }
        layout->setRowFixedHeight( row, fixedHeightVal );
    }

    if ( rowSpacing.mType != HbXmlLengthValue::None ) {
        qreal rowSpacingVal;
        if ( !toPixels(rowSpacing, rowSpacingVal) ) {
            return false;
        }
        layout->setRowSpacing( row, rowSpacingVal );
    }

    return true;

}

bool HbDocumentLoaderActions::setGridLayoutColumnWidths( int column,
                                                         const HbXmlLengthValue &minWidth,
                                                         const HbXmlLengthValue &maxWidth,
                                                         const HbXmlLengthValue &prefWidth,
                                                         const HbXmlLengthValue &fixedWidth,
                                                         const HbXmlLengthValue &columnSpacing )
{
    QGraphicsGridLayout *layout = static_cast<QGraphicsGridLayout *>( mCurrentLayout );

    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    if ( minWidth.mType != HbXmlLengthValue::None ) {
        qreal minWidthVal;
        if ( !toPixels(minWidth, minWidthVal) ) {
            return false;
        }
        layout->setColumnMinimumWidth( column, minWidthVal );
    }

    if ( maxWidth.mType != HbXmlLengthValue::None ) {
        qreal maxWidthVal;
        if ( !toPixels(maxWidth, maxWidthVal) ) {
            return false;
        }
        layout->setColumnMaximumWidth( column, maxWidthVal );
    }

    if ( prefWidth.mType != HbXmlLengthValue::None ) {
        qreal prefWidthVal;
        if ( !toPixels(prefWidth, prefWidthVal) ) {
            return false;
        }
        layout->setColumnPreferredWidth( column, prefWidthVal );
    }

    if ( fixedWidth.mType != HbXmlLengthValue::None ) {
        qreal fixedWidthVal;
        if ( !toPixels(fixedWidth, fixedWidthVal) ) {
            return false;
        }
        layout->setColumnFixedWidth( column, fixedWidthVal );
    }

    if ( columnSpacing.mType != HbXmlLengthValue::None ) {
        qreal columnSpacingVal;
        if ( !toPixels(columnSpacing, columnSpacingVal) ) {
            return false;
        }
        layout->setColumnSpacing( column, columnSpacingVal );
    }

    return true;
}

bool HbDocumentLoaderActions::createLinearLayout(
    const QString &widget,
    Qt::Orientation *orientation,
    const HbXmlLengthValue &spacing )
{
    QGraphicsWidget *parent = 0;
    QGraphicsLinearLayout *layout = 0;

    if( widget.isEmpty() ) {
        bool isWidget = false;
        QObject *parentObj = findFromStack( &isWidget );
        if ( isWidget ) {
            parent = static_cast<QGraphicsWidget *>( parentObj );
        }
    } else if ( mObjectMap.contains( widget ) && mObjectMap[ widget ].mType == HbXml::WIDGET ) {
        parent = static_cast<QGraphicsWidget *>( mObjectMap[ widget ].mObject.data() );
    }
    if ( !parent ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: PARENT NOT FOUND" ) );
        return false;
    }

    if( orientation ) {
        layout = new QGraphicsLinearLayout( *orientation );
    } else {
        layout = new QGraphicsLinearLayout();
    }

    if ( spacing.mType != HbXmlLengthValue::None ) {
        qreal spacingVal;
        if ( !toPixels(spacing, spacingVal) ) {
            return false;
        }
        layout->setSpacing(spacingVal);
    }

    mCurrentLayout = layout;
    parent->setLayout( mCurrentLayout );

    return true;
}

bool HbDocumentLoaderActions::addLinearLayoutItem(
    const QString &itemname,
    int *index,
    int *stretchfactor,
    Qt::Alignment *alignment,
    const HbXmlLengthValue &spacing )
{
    QGraphicsLayoutItem *item = 0;

    QGraphicsLinearLayout *layout = static_cast<QGraphicsLinearLayout *>( mCurrentLayout );

    if ( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    if ( itemname.isEmpty() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: TRY TO ADD EMPTY ITEM " ) + itemname );
        return false;
    } else if ( mObjectMap.contains( itemname ) && mObjectMap[ itemname ].mType == HbXml::WIDGET ) {
        item = static_cast<QGraphicsWidget *>( mObjectMap[ itemname ].mObject.data() );
    } else {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ITEM " ) + itemname );
        return false;
    }

    int indexValue = index ? *index : -1;
    layout->insertItem( indexValue, item );

    if ( spacing.mType != HbXmlLengthValue::None ) {
        qreal spacingVal;
        if ( !toPixels(spacing, spacingVal) ) {
            return false;
        }

        // Need to resolve the item index for spacing
        int i = layout->count();
        while (i--) {
            if ( layout->itemAt(i) == item ) {
                layout->setItemSpacing(i, spacingVal);
                break;
            }
        }
    }

    if ( stretchfactor ) {
        layout->setStretchFactor( item, *stretchfactor );
    }

    if( alignment ) {
        layout->setAlignment( item, *alignment );
    }

    return true;
}

bool HbDocumentLoaderActions::addLinearLayoutStretch(
    int *index,
    int *stretchfactor )
{
    QGraphicsLinearLayout *layout = static_cast<QGraphicsLinearLayout *>( mCurrentLayout );

    if( !layout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    int indexValue = index ? *index : -1;
    int stretch = stretchfactor ? * stretchfactor : 1;

    layout->insertStretch( indexValue, stretch );

    return true;
}

bool HbDocumentLoaderActions::setLayoutContentsMargins( const HbXmlLengthValue &left,
                                                        const HbXmlLengthValue &top,
                                                        const HbXmlLengthValue &right,
                                                        const HbXmlLengthValue &bottom )
{
    if( !mCurrentLayout ) {
        HB_DOCUMENTLOADER_PRINT( QString( "LAYOUT: INTERNAL ERROR " ) );
        return false;
    }

    qreal leftVal=0, topVal=0, rightVal=0, bottomVal=0;
    bool ok = true;
    if (left.mType != HbXmlLengthValue::None) {
        ok &= toPixels(left, leftVal);
    }
    if (top.mType != HbXmlLengthValue::None) {
        ok &= toPixels(top, topVal);
    }
    if (right.mType != HbXmlLengthValue::None) {
        ok &= toPixels(right, rightVal);
    }
    if (bottom.mType != HbXmlLengthValue::None) {
        ok &= toPixels(bottom, bottomVal);
    }
    if ( ok ) {
        mCurrentLayout->setContentsMargins( leftVal, topVal, rightVal, bottomVal );
    }
    return ok;
}

bool HbDocumentLoaderActions::createStackedLayout( const QString &widget )
{
    QGraphicsWidget *parent = 0;

    if( widget.isEmpty() ) {
        bool isWidget = false;
        QObject *parentObj = findFromStack( &isWidget );
        if( isWidget ) {
            parent = static_cast<QGraphicsWidget *>( parentObj );
        }
    } else if ( mObjectMap.contains( widget ) && mObjectMap[ widget ].mType == HbXml::WIDGET ) {
        parent = static_cast<QGraphicsWidget *>( mObjectMap[ widget ].mObject.data() );
    }
    if ( !parent ) {
        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: PARENT NOT FOUND" ) );
        return false;
    }

    mCurrentLayout = new HbStackedLayout();

    parent->setLayout( mCurrentLayout );

    return true;
}

bool HbDocumentLoaderActions::addStackedLayoutItem( const QString &itemname, int *index )
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
    } else if ( mObjectMap.contains( itemname ) && mObjectMap[ itemname ].mType == HbXml::WIDGET ) {
        item = static_cast<QGraphicsWidget *>( mObjectMap[ itemname ].mObject.data() );
    } else {
        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: NO SUCH ITEM " ) + itemname );
        return false;
    }

    int indexValue = index ? *index : -1;
    layout->insertItem( indexValue, item );

    return true;
}


bool HbDocumentLoaderActions::createNullLayout( const QString &widget )
{
    QGraphicsWidget *parent = 0;

    if( widget.isEmpty() ) {
        bool isWidget = false;
        QObject *parentObj = findFromStack( &isWidget );
        if( isWidget ) {
            parent = static_cast<QGraphicsWidget *>( parentObj );
        }
    } else if ( mObjectMap.contains( widget ) && mObjectMap[ widget ].mType == HbXml::WIDGET ) {
        parent = static_cast<QGraphicsWidget *>( mObjectMap[ widget ].mObject.data() );
    }
    if ( !parent ) {
        HB_DOCUMENTLOADER_PRINT( QString( "NULL LAYOUT: PARENT NOT FOUND" ) );
        return false;
    }

    mCurrentLayout = 0;

    parent->setLayout( mCurrentLayout );

    return true;

}

bool HbDocumentLoaderActions::setWidgetRole(
    QGraphicsWidget *parent, QGraphicsWidget *child, const QString &role)
{
    bool roleTransfersOwnership = false; 

    // updates roleTransfersOwnership only on succesfull execution
    const bool result = mFactory.setWidgetRole(parent, child, role, roleTransfersOwnership);

    if ( roleTransfersOwnership ) {
        // remove ownership from own data structure    
        ObjectMapItem &item = mObjectMap[child->objectName()];
        item.mOwned = false;
    }

    return result;
}

bool HbDocumentLoaderActions::setObjectRole(
    QObject *parent, QObject *child, const QString &role)
{
    return mFactory.setObjectRole(parent, child, role);
}

bool HbDocumentLoaderActions::variableToQVariant( const HbXmlVariable& variable, QVariant &variant )
{
    Q_UNUSED(variable);
    Q_UNUSED(variant);
    bool result(true);

    switch (variable.mType) {
        case HbXmlVariable::INT:
        {
        qint16* int_b =(qint16*)variable.mParameters.at(0);
        variant.setValue((int)(*int_b));
        break;
        }

        case HbXmlVariable::REAL:
        {
        HbXmlLengthValue* realVal = (HbXmlLengthValue*)variable.mParameters.at(0);
        qreal realNum;
        result = toPixels(*realVal, realNum );
        if (result) {
            variant.setValue(realNum);
        }
        break;
        }

        case HbXmlVariable::LOCALIZED_STRING:
        {
        QString *value = (QString*)variable.mParameters.at(0);
        QString *comment = (QString*)variable.mParameters.at(1);
        const QString text = translate( *value, *comment );
        variant.setValue( text );
        break;
        }

        case HbXmlVariable::STRING:
        {
        QString *value = (QString*)variable.mParameters.at(0);
        QString *locId = (QString*)variable.mParameters.at(1);
        variant.setValue( locId->isEmpty() ? *value : hbTrId(locId->toUtf8()) );
        break;
        }

        case HbXmlVariable::BOOL:
        {
        bool *bool_b = (bool*)variable.mParameters.at(0);
        variant.setValue( *bool_b );
        break;
        }

        case HbXmlVariable::ICON:
        {
        QString *iconName = (QString*)variable.mParameters.at(0);
        HbXmlLengthValue* widthVal = (HbXmlLengthValue*)variable.mParameters.at(1);
        HbXmlLengthValue* heightVal = (HbXmlLengthValue*)variable.mParameters.at(2);

        HbIcon icon(*iconName);
        qreal width, height;
        if ( widthVal->mType != HbXmlLengthValue::None ) {
            result = toPixels(*widthVal, width);
        }
        if ( result && heightVal->mType != HbXmlLengthValue::None ) {
            result = toPixels(*heightVal, height);
        }
        if ( result ) {
            if ( widthVal->mType != HbXmlLengthValue::None &&
                 heightVal->mType != HbXmlLengthValue::None ) {
                icon.setSize(QSizeF(width, height));
            } else if ( widthVal->mType != HbXmlLengthValue::None ) {
                icon.setWidth(width);
            } else if ( heightVal->mType != HbXmlLengthValue::None ) {
                icon.setHeight(height);
            }
            variant.setValue( icon );
        }
        break;
        }

        case HbXmlVariable::SIZE:
        {
        HbXmlLengthValue* widthVal = (HbXmlLengthValue*)variable.mParameters.at(0);
        HbXmlLengthValue* heightVal = (HbXmlLengthValue*)variable.mParameters.at(1);
        qreal width, height;
        result &= toPixels(*widthVal, width);
        result &= toPixels(*heightVal, height);
        if ( result ) {
            variant.setValue( QSizeF( width, height ) );
        }
        break;
        }

        case HbXmlVariable::RECT:
        {
        HbXmlLengthValue* widthVal = (HbXmlLengthValue*)variable.mParameters.at(0);
        HbXmlLengthValue* heightVal = (HbXmlLengthValue*)variable.mParameters.at(1);
        HbXmlLengthValue* posxVal = (HbXmlLengthValue*)variable.mParameters.at(2);
        HbXmlLengthValue* posyVal = (HbXmlLengthValue*)variable.mParameters.at(3);
        qreal width, height, posx, posy;
        result &= toPixels(*widthVal, width);
        result &= toPixels(*heightVal, height);
        result &= toPixels(*posxVal, posx);
        result &= toPixels(*posyVal, posy);
        if ( result ) {
            variant.setValue(QRectF(QPointF(posx, posy), QSizeF(width, height)));
        }
        break;
        }

        case HbXmlVariable::POINT:
        {
        HbXmlLengthValue* posxVal = (HbXmlLengthValue*)variable.mParameters.at(0);
        HbXmlLengthValue* posyVal = (HbXmlLengthValue*)variable.mParameters.at(1);
        qreal posx, posy;
        result &= toPixels(*posxVal, posx);
        result &= toPixels(*posyVal, posy);
        if ( result ) {
            variant.setValue(QPointF(posx, posy));
        }
        break;
        }

        case HbXmlVariable::ENUMS:
        {
        // Relies on implicit conversion.
        QString *string = (QString*)variable.mParameters.at(0);
        variant.setValue(*string);
        break;
        }

        case HbXmlVariable::COLOR:
        {
        QColor *color = (QColor*)variable.mParameters.at(0);
        variant.setValue(*color);
        break;
        }

        case HbXmlVariable::FONTSPEC:
        {
        quint8* role_b = (quint8*)variable.mParameters.at(0);
        HbXmlLengthValue* textHeightVal = (HbXmlLengthValue*)variable.mParameters.at(1);
        qreal textHeight;
        if ( textHeightVal->mType != HbXmlLengthValue::None ) {
            result = toPixels(*textHeightVal, textHeight);
        }
        if (result) {
            HbFontSpec fontSpec((HbFontSpec::Role)(*role_b));
            if ( textHeightVal->mType != HbXmlLengthValue::None ) {
                fontSpec.setTextHeight(textHeight);
            }
            variant.setValue(fontSpec);
        }
        break;
        }

        default:
        {
        result = false;
        break;
        }
        }

    return result;
}


