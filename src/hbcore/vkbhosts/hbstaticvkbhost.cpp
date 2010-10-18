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
#include "hbstaticvkbhost.h"
#include "hbabstractvkbhost_p.h"

#include <hbinputvirtualkeyboard.h>
#include <hbinputmethod.h>

#include "hbwidget.h"
#include "hbmainwindow.h"

/*!
\proto
@hbcore
\class HbStaticVkbHost
\brief Static virtual keyboard host

This virtual keyboard host disables container animations and only takes care of keyboard movement.
It only brings up the virtual keyboard but does not move the underlying view or reposition
the editor. In other words, it does not gurantee that the editor cursor remains visible.

This host is meant to be used in those special cases where none of the other vkb hosts works.

See \ref vkbHandling "virtual keyboard handling guide" for more information

\sa HbVkbHost
\sa HbAbstractVkbHost
\sa HbShrinkingVkbHost
*/

/// @cond

class HbStaticVkbHostPrivate : public HbAbstractVkbHostPrivate
{
public:
    HbStaticVkbHostPrivate(HbAbstractVkbHost *myHost, HbWidget *widget);
    bool prepareContainerAnimation(HbVkbHost::HbVkbStatus status);
};

HbStaticVkbHostPrivate::HbStaticVkbHostPrivate(HbAbstractVkbHost *myHost, HbWidget *widget)
    : HbAbstractVkbHostPrivate(myHost, widget)
{
}

bool HbStaticVkbHostPrivate::prepareContainerAnimation(HbVkbHost::HbVkbStatus status)
{
    Q_UNUSED(status);

    // This host doesn't move the container, only the keypad.
    return false;
}

/// @endcond

/*!
Constructs the object.
*/
HbStaticVkbHost::HbStaticVkbHost(HbWidget *widget) : HbAbstractVkbHost(new HbStaticVkbHostPrivate(this, widget))
{
    setParent(widget);
}

/*!
Destructs the object.
*/
HbStaticVkbHost::~HbStaticVkbHost()
{
}

// End of file

