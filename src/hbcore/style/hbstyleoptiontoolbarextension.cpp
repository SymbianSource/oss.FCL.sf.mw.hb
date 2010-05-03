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

#include "hbstyleoptiontoolbarextension.h"

/*!
    \class HbStyleOptionToolBarExtension
    \brief HbStyleOptionToolBarExtension has the style component for toolbar extension primitives

    This class will cease to exist in near future.
*/

/*!
    Style component for toolbar extension primitives

    \deprecated HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension()
        is deprecated.

*/


/*!

    \deprecated HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension()
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension() :
    HbStyleOptionPopup(),
    opacity(1.0)
{
    qWarning("HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension() - is deprecated.");
    type = Type;
    version = Version;
}

/*!

    \deprecated HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension(const HbStyleOptionToolBarExtension&)
    is deprecated. Styleoptions will not be public.

*/
HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension(
    const HbStyleOptionToolBarExtension &other) :
    HbStyleOptionPopup(other),
    opacity(other.opacity)
{
    qWarning("HbStyleOptionToolBarExtension::HbStyleOptionToolBarExtension() - is deprecated.");
    type = Type;
    version = Version;
}

HbStyleOptionToolBarExtension::~HbStyleOptionToolBarExtension()
{
}
