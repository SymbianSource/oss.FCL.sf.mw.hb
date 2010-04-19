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

#include "hbeffectabstract_p.h"

/*
    \class HbEffectAbstract

    \brief HbEffectAbstract is the base class for all the effects.

*/

/*
    Constructor taking a graphics item.
*/
HbEffectAbstract::HbEffectAbstract(int startTime, QGraphicsItem *item, HbEffectGroup *group)
    : HbTimerEntry(startTime), mItem(item), mGroup(group)
{
}

/*
    Destructor.
*/
HbEffectAbstract::~HbEffectAbstract()
{
}

/*
    Returns the animated graphics item.
*/
QGraphicsItem *HbEffectAbstract::item() const
{
    return mItem;
}

/*
    Returns the effect group, which can be a combination of basic effects.
    The effect group represents one logical effect, e.g. 'appear' effect.
*/
HbEffectGroup *HbEffectAbstract::group() const
{
    return mGroup;
}

/*!
    \reimp
 */
void HbEffectAbstract::timerFired()
{
    start();
}
