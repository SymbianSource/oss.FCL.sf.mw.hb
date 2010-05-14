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

#include "hbcontinuousinteractionevent_p.h"

/*!
    \fn const HbWidget* HbContinuousInteractionEvent::widget() const

    Returns the widget being interacted with. Should never be null.

    \internal
*/

/*!
    \fn Hb::ContinuousInteraction HbContinuousInteractionEvent::interaction() const

    Returns the type of continuous interaction.

    \internal
*/

/*!
    \fn QPointF HbContinuousInteractionEvent::delta() const

    Returns the movement vector of the continuous interaction.

    \internal
*/

const int HbContinuousInteractionEvent::ContinuousInteraction = registerEventType();
const int HbContinuousInteractionEvent::ContinuousInteractionStop = registerEventType();

/*!
    Constructs a HbContinuousInteractionEvent with continuous interaction information.

    Continuous interaction event contains widget interaction information. Feedback engines use that
    information to implement various feedback effects.

    \param eventType either HbContinuousInteractionEvent::ContinuousInteraction or HbContinuousInteractionEvent::ContinuousInteractionStop.
    \param widget - widget being interacted with
    \param interaction - type of continuous interaction
    \param delta - movement vector of the continuous interaction

    \internal
*/
HbContinuousInteractionEvent::HbContinuousInteractionEvent(int eventType, const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
    : QEvent((QEvent::Type)eventType), m_widget(widget), m_interaction(interaction), m_delta(delta)
{
}

HbContinuousInteractionEvent::~HbContinuousInteractionEvent()
{
}
