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

#include "hbwidgetloaderactions_p.h"
#include "hbwidgetloadersyntax_p.h"
#include "hbwidget_p.h"
#include "hbwidgetbase_p.h"
#include <QDebug>

/*
    \class HbWidgetLoaderActions
    \internal
    \proto
*/

/*!
    \internal
*/
HbWidgetLoaderActions::HbWidgetLoaderActions() 
    : HbXmlLoaderBaseActions(), mWidget(0), mLayout(0)
{
}

/*!
    \internal
*/
HbWidgetLoaderActions::~HbWidgetLoaderActions()
{
}

#ifndef HB_BIN_CSS
/*!
    \internal
*/
bool HbWidgetLoaderActions::createMeshLayout( const QString &widget )
{
    Q_UNUSED( widget );
    HbMeshLayout *layout = static_cast<HbMeshLayout*>(mWidget->layout());
    if (!layout) {
        layout = new HbMeshLayout();
        mWidget->setLayout(layout);
    } else {
        // Reset layout's state
        while (layout->count()) {
            layout->removeAt(0);
        }
        layout->clearAnchors();
        layout->clearSpacingOverrides();
        layout->clearItemIds();
    }
    mLayout = layout;
    return true;
}

/*!
    \internal
*/
bool HbWidgetLoaderActions::addMeshLayoutEdge( const QString &src, Hb::Edge srcEdge, 
                                               const QString &dst, Hb::Edge dstEdge,
                                               const HbXmlLengthValue &spacing, const QString &spacer )
{
    bool ok = true;
    if ( !spacer.isEmpty() ) {
        // spacer is added
        // divide original mesh definition into two. src->dst becomes src->spacer->dst
        if ( src.isEmpty() ) {
            // if the starting item is layout
            // "layout --(spacing)--> item" 
            // becomes 
            // "layout --(spacing)--> spacer --(0)--> item"
            ok &= addMeshLayoutEdge( src, srcEdge, spacer, srcEdge, spacing, QString() );
            HbXmlLengthValue val(0, HbXmlLengthValue::Pixel);
            ok &= addMeshLayoutEdge( spacer, getAnchorOppositeEdge(srcEdge), dst, dstEdge, val, QString() );
        } else {
            // between two items, or if end item is layout
            // "item1 --(spacing)--> item2" 
            // becomes 
            // "item1 --(spacing)--> spacer --(0)--> item2"
            ok &= addMeshLayoutEdge( src, srcEdge, spacer, getAnchorOppositeEdge(srcEdge), spacing, QString() );
            HbXmlLengthValue val(0, HbXmlLengthValue::Pixel);
            ok &= addMeshLayoutEdge( spacer, srcEdge, dst, dstEdge, val, QString() );
        }
        if ( ok & !mWidget->layoutPrimitive( spacer ) ) {
            static_cast<HbWidgetPrivate*>(HbWidgetBasePrivate::d_ptr(mWidget))
                ->createSpacerItem(spacer);
        }
    } else {
        qreal spacingPx=0.0;
        if (spacing.mType != HbXmlLengthValue::None ) {
            ok = toPixels(spacing, spacingPx);
        } // else default to zero.
        if ( ok ) {
            mLayout->setAnchor(src, srcEdge, dst, dstEdge, spacingPx);
        }
    }
    return ok;
}
#endif
/*
    \class HbWidgetLoaderMemoryActions
    \internal
    \proto
*/

/*!
    \internal
*/
HbWidgetLoaderMemoryActions::HbWidgetLoaderMemoryActions() : HbXmlLoaderAbstractActions(), mLayoutDef(0)
{
}

/*!
    \internal
*/
HbWidgetLoaderMemoryActions::~HbWidgetLoaderMemoryActions()
{
}

/*!
    \internal
*/
bool HbWidgetLoaderMemoryActions::createMeshLayout( const QString &widget )
{
    Q_UNUSED(widget);
    mLayoutDef->meshItems.clear();
    return true;
}

/*!
    \internal
*/
bool HbWidgetLoaderMemoryActions::addMeshLayoutEdge( const QString &src, Hb::Edge srcEdge, 
                                               const QString &dst, Hb::Edge dstEdge,
                                               const HbXmlLengthValue &spacing, const QString &spacer )
{    
    HbWidgetLoader::MeshItem item(mLayoutDef->type);
    item.src = src;
    item.dst = dst;    
    item.srcEdge = srcEdge;    
    item.dstEdge = dstEdge;
    item.spacingType = spacing.mType;
    item.spacingVal = spacing.mValue;
    item.spacingText = spacing.mString;
    item.spacer = spacer;
    
    mLayoutDef->meshItems.append(item);

    return true;
}

