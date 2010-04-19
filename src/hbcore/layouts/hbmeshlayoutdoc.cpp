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

/*!
    \class HbMeshLayout
    \brief HbMeshLayout manages geometries of its child items with anchors that
    that connect the layout items with each other. This is different from 
    \c HbAnchorLayout in such way that this allows layout items to be missing
    and can fix anchor attachments.

    Currently, the HbMeshLayout is an internal class which can be only utilized via the
    WidgetML within a widget. 

    The mesh layout is a bit more fragile than the anchor layout. The anchor definitions
    need to support the missing items. Here are some simple rules how the mesh can be 
    created (the example is only for horizontal direction - the same needs to be done 
    for portrait as well).

    First, we need to find the child item (=node), which is always present i.e. cannot be missing. 

    \image html hbmeshlayout1.png

    From the image above, we have decided that the green node is always present. This
    means that all the other nodes in the horizontal graph can be optional.

    \image html hbmeshlayout2.png

    Then, we create the anchors starting from the non-optional node and point towards
    the edges of the layout. The mesh layout definition in the WidgetML would look like:

    \code

    <meshitem src="green_item" srcEdge="LEFT" dst="blue_item" dstEdge="RIGHT" />
    <meshitem src="blue_item" srcEdge="LEFT" dst="" dstEdge="LEFT" />
    <meshitem src="green_item" srcEdge="RIGHT" dst="red_item" dstEdge="LEFT" />
    <meshitem src="red_item" srcEdge="RIGHT" dst="yellow_item" dstEdge="LEFT" />
    <meshitem src="yellow_item" srcEdge="RIGHT" dst="" dstEdge="RIGHT" />

    \endcode

    As mentioned, the green node needs be present always. In practice, this means that the 
    parent widget, which owns this mesh layout, needs to have a child widget with item
    name "green_item". \c HbStyle::setItemName for more details.
    
    If an optional node is missing, the anchors pointing to the node are
    changed to point to the node after (=towards the parent layout) the missing one - this 
    is called "fixing the mesh". The fixing only works if the end result can be determined
    i.e. two anchor starting from a missing node is prohibited.

    \image html hbmeshlayout3.png

    In the picture above, the blue and yellow items are missing. The mesh is fixed by removing
    the anchor definitions starting from the missing nodes.

    \proto
    \internal 
*/
