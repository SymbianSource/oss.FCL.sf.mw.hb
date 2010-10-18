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
bool HbWidgetLoaderActions::createAnchorLayout( const QString &widget, bool modify )
{
    Q_UNUSED( widget );
    Q_UNUSED( modify );
    HbAnchorLayout *layout = static_cast<HbAnchorLayout*>(mWidget->layout());
    if (!layout) {
        layout = new HbAnchorLayout();
        mWidget->setLayout(layout);
    } else {
        // Reset layout's state
        while (layout->count()) {
            layout->removeAt(0);
        }
        layout->removeAnchors();
        layout->removeMappings();
    }
    mLayout = layout;
    return true;
}

/*!
    \internal
*/
bool HbWidgetLoaderActions::addAnchorLayoutItem(
    const QString &src,
    const QString &srcId,
    Hb::Edge srcEdge, 
    const QString &dst,
    const QString &dstId,
    Hb::Edge dstEdge,
    const HbXmlLengthValue &minLength,
    const HbXmlLengthValue &prefLength,
    const HbXmlLengthValue &maxLength,
    QSizePolicy::Policy *policy, 
    HbAnchor::Direction *dir,
    const QString &anchorId )
{
    // widgetml is purely id based.
    Q_UNUSED(src); 
    Q_UNUSED(dst);

    HbAnchor* anchor = new HbAnchor( srcId, srcEdge, dstId, dstEdge );

    if ( minLength.mValues.count() ) {
        qreal minVal(0);
        if ( !toPixels(minLength, minVal) ) {
            delete anchor;
            return false;
        } else {
            anchor->setMinimumLength( minVal );
        }
    }

    if ( prefLength.mValues.count() ) {
        qreal prefVal(0);
        if ( !toPixels(prefLength, prefVal) ) {
            delete anchor;
            return false;
        } else {
            // if the expression resulted a negative result, we must reverse the direction
            if ( prefVal < 0 && dir ) {
                *dir = (*dir==HbAnchor::Positive) ? HbAnchor::Negative : HbAnchor::Positive;
            }
            anchor->setPreferredLength( qAbs(prefVal) );         
        }
    }

    if ( maxLength.mValues.count() ) {
        qreal maxVal(0);
        if ( !toPixels(maxLength, maxVal) ) {
            delete anchor;
            return false;
        } else {
            anchor->setMaximumLength( maxVal );
        }
    }

    if ( policy ) {
        anchor->setSizePolicy( *policy );
    }

    if ( dir ) {
        anchor->setDirection( *dir );
    }

    if ( !anchorId.isEmpty() ) {
        anchor->setAnchorId( anchorId );
    }

    return mLayout->setAnchor( anchor );
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
bool HbWidgetLoaderMemoryActions::createAnchorLayout( const QString &widget, bool modify )
{
    Q_UNUSED(widget);
    Q_UNUSED(modify);
    mLayoutDef->anchorItems.clear();
    return true;
}

/*!
    \internal
*/
bool HbWidgetLoaderMemoryActions::addAnchorLayoutItem(
    const QString &src,
    const QString &srcId,
    Hb::Edge srcEdge, 
    const QString &dst,
    const QString &dstId,
    Hb::Edge dstEdge,
    const HbXmlLengthValue &minLength,
    const HbXmlLengthValue &prefLength,
    const HbXmlLengthValue &maxLength,
    QSizePolicy::Policy *policy, 
    HbAnchor::Direction *dir,
    const QString &anchorId )
{
    // widgetml is purely id based.
    Q_UNUSED(src); 
    Q_UNUSED(dst);

    HbWidgetLoader::AnchorItem item(mLayoutDef->type);
    item.srcId = srcId;
    item.dstId = dstId;
    item.srcEdge = srcEdge;
    item.dstEdge = dstEdge;

    for (int i=0; i<minLength.mValues.count(); i++) {
        item.minVal.append(minLength.mValues.at(i));
    }
    for (int j=0; j<prefLength.mValues.count(); j++) {
        item.prefVal.append(prefLength.mValues.at(j));
    }
    for (int k=0; k<maxLength.mValues.count(); k++) {
        item.maxVal.append(maxLength.mValues.at(k));
    }

    item.sizepolicy = policy ? *policy : -1;
    item.direction = dir ? *dir : -1;
    item.anchorId = anchorId;

    mLayoutDef->anchorItems.append(item);
    return true;
}

