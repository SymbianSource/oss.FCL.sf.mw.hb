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
#ifndef HBVKBCONSTANTS_P_H
#define HBVKBCONSTANTS_P_H

#include "hbinputdef.h"

// This value specifies the margin that the vkb host leaves
// between editor border and the container border (or the cursor
// rect and the container border) when it performs
// container adjustements. The value is in layout units.
const qreal HbVkbHostMargin = 2.238806605;

// This value specifies the keyboard animation time in milliseconds.
const int HbVkbAnimationTime = 200;

// The maximum vertical size for the keypad widget (as a portion of the screen).
const qreal HbHeightVerticalFactor = 0.5;

// The maximum horizontal size for the keypad widget
const qreal HbHeightHorizFactor = 0.7;

#endif // HBVKBCONSTANTS_P_H

// End of file
