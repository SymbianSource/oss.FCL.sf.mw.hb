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


HbWidgetLoaderActions::HbWidgetLoaderActions(HbMemoryManager::MemoryType type) : HbXmlLoaderAbstractActions(),mType(type),mLayoutDefinitionOffset(-1)
{
    // if the memory type is shared memory then allocate memory in the sharedmemory
    // for the vector of meshitems
    // else allocate the memory in the heap.
    if(HbMemoryManager::SharedMemory == mType){
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        mLayoutDefinitionOffset = manager->alloc(sizeof(LayoutDefinition));

        if (mLayoutDefinitionOffset > -1) {
        	mCacheLayout = new((char*)manager->base() + mLayoutDefinitionOffset) LayoutDefinition(HbMemoryManager::SharedMemory);
        }
    }
    else {
        if(mCacheLayout == NULL){
        	mCacheLayout = new LayoutDefinition(HbMemoryManager::HeapMemory);
        }
    }
}

HbWidgetLoaderActions::~HbWidgetLoaderActions()
{
}

bool HbWidgetLoaderActions::createMeshLayout()
{    
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
    return true;
}

bool HbWidgetLoaderActions::addMeshLayoutEdge( const QString &src, const QString &srcEdge, 
                                               const QString &dst, const QString &dstEdge,
                                               const QString &spacing, const QString &spacer )
{    
    if ( !spacer.isEmpty() ) {
        // spacer is added
        // divide original mesh definition into two. src->dst becomes src->spacer->dst
        bool ok = true;
        if ( src.isEmpty() ) {
            // if the starting item is layout
            // "layout --(spacing)--> item" 
            // becomes 
            // "layout --(spacing)--> spacer --(0)--> item"
            ok &= addMeshLayoutEdge( src, srcEdge, spacer, srcEdge, spacing, QString() );
            ok &= addMeshLayoutEdge( spacer, getAnchorOppositeEdge(srcEdge), dst, dstEdge, QString(), QString() );
        } else {
            // if the starting item is not layout
            // "item1 --(spacing)--> item2" 
            // becomes 
            // "item1 --(spacing)--> spacer --(0)--> item2"
            ok &= addMeshLayoutEdge( src, srcEdge, spacer, getAnchorOppositeEdge(srcEdge), spacing, QString() );
            ok &= addMeshLayoutEdge( spacer, srcEdge, dst, dstEdge, QString(), QString() );
        }

        if ( ok ) {
            mCacheLayout->spacers.append(HbString(spacer,mType));
        }

        return ok;
    }

    MeshItem item(mType);
    item.src = src;
    item.dst = dst;
    
    int edgeIndex = getAnchorEdge( srcEdge );
    if( edgeIndex < 0 ) {
        qWarning() << "Invalid mesh start edge"; 
        return false;
    }
    
    item.srcEdge = (Hb::Edge)edgeIndex;
    
    edgeIndex = getAnchorEdge( dstEdge );
    if( edgeIndex < 0 ){
        qWarning() << "Invalid mesh end edge"; 
        return false;
    }
    
    item.dstEdge = (Hb::Edge)edgeIndex;
    
    item.spacing = spacing;

    mCacheLayout->meshItems.append(item);

    return true;
}

/*
  Updates the widget's layout with the data given in LayoutDefinition.
  If no LayoutDefinition it's assumed that the correct layout definition
  is found in internal cache.
*/
void HbWidgetLoaderActions::updateWidget(LayoutDefinition* layoutDef)
{
    if (!layoutDef) {
        // Use client side cache.
        layoutDef = mCacheLayout;
    }
    if (layoutDef){
        // Construct layout from layout definition
	    createMeshLayout();
	    HbWidgetLoaderSyntax loaderSyntax(NULL);
	    HbMeshLayout *layout = static_cast<HbMeshLayout*>(mWidget->layout());

	    for (int i = 0; i < layoutDef->meshItems.count(); i++){
			const MeshItem &item = layoutDef->meshItems.at(i);
	        // Mesh item stored in the sharedmemory doesn't contain
	        // the actual data. so, converting that to pixels.
	        qreal spacing=0.0;
            if (!(item.spacing.isEmpty())){
                loaderSyntax.toPixels(HbDeviceProfile::profile(mWidget), item.spacing,spacing);
            }
            layout->setAnchor(item.src, item.srcEdge, item.dst, item.dstEdge, spacing);
	    }
        for (int i=0; i<layoutDef->spacers.count(); i++){
            QString spacer = layoutDef->spacers.at(i);
            if ( !mWidget->layoutPrimitive( spacer ) ) {
                static_cast<HbWidgetPrivate*>(HbWidgetBasePrivate::d_ptr(mWidget))
                    ->createSpacerItem(spacer);
            }
        }
    }
}

/*
  Returns the mesh items offset.
*/

int HbWidgetLoaderActions::getLayoutDefintionOffset()
{
    if(HbMemoryManager::SharedMemory == mType) {
    	return mLayoutDefinitionOffset;
    }else {
    	return -1;
    }
}

/*
  Sets the MeshItemsOffset
*/
void HbWidgetLoaderActions::setLayoutDefintionOffset(int offset)
{
    mLayoutDefinitionOffset = offset;
}

