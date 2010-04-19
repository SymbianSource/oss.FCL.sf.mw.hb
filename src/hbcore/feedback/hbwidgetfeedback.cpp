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

#include "hbwidgetfeedback.h"
#include "hbfeedbackmanager.h"

/*!
    @beta
    @hbcore

    \class HbWidgetFeedback

    \brief Widget Feedback API is used by widgets for providing feedback information for the feedback framework.

    For widgets to support feedback features, a widget needs to call method triggered() with Hb::InstantInteraction parameter
    \li Hb::InstantPressed when a widget is pressed down
    \li Hb::InstantReleased when a press is released
    \li Hb::InstantClicked when an widget is pressed and released immediately
    \li Hb::InstantKeyRepeated when an widget sends key repeats and is pressed down
    \li Hb::InstantLongPressed when an widget is kept pressed for a while
    \li Hb::InstantDraggedOver when the finger is dragged on top of an widget
    \li Hb::InstantFlicked when a current widget is flicked using a flick gesture
    \li Hb::InstantBoundaryReached when the boundary of a scroll area is reached
    \li Hb::InstantRotated90Degrees every time the multitouch area passes 90 degrees of rotation
    \li Hb::InstantPopupOpened when a popup, for example a note, tooltip or menu, is opened
    \li Hb::InstantPopupClosed when a popup, for example a note, tooltip or menu, is closed
    \li between Hb::InstantUser and Hb::InstantMax when a custom widget is being interacted with a custom interaction
    
    Widget needs to call method continuousTriggered() and continuousStopped() with Hb::ContinuousInteraction parameter
    \li Hb::ContinuousScrolled when an widget area is scrolled by quickly flicking or continuously panning the finger
    \li Hb::ContinuousDragged when user is dragging a widget like moving slider handle
    \li Hb::ContinuousPinched when an widget is being zoomed or rotated with two fingers
    \li between Hb::ContinuousUser and Hb::ContinuousMax when a custom widget is being interacted with a continuous custom interaction

    The information provided by HbFeedbackManager is forwarded to the feedback engines.
    
    Widget should only call interaction methods when <i>user actually interacts with the widget</i> and not for example 
    when an application resets widget states by calling an widget API or when a progress bar is moved during a file transfer. 
    We don't want to initiate haptic and sound feedback effects when the device is not getting any real user interaction.
*/

/*!
    Mediates instant interaction information from widgets to feedback manager 
    that forwards it to all active feedback plugins.

    \param widget the widget being interacted with
    \param interaction the interaction
    \param modifiers optional specifiers to the interaction
*/
void HbWidgetFeedback::triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers)
{
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->triggered(widget, interaction, modifiers);
    }
}

/*!
    Mediates continuous interaction information from widgets to feedback manager 
    that forwards it to all active feedback plugins.

    \param widget the widget being interacted with
    \param interaction the continuous interaction in progress
    \param delta supplies the direction and distance of the interaction
*/
void HbWidgetFeedback::continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->continuousTriggered(widget, interaction, delta);
    }
}

/*!
    Mediates information about stopped continuous interaction from widget 
    to feedback manager that forwards it to all active feedback plugins.
    This methods is needed for knowing when to stop continuous feedback
    effects started by the continuous interaction.

    \param widget the widget being interacted with
    \param interaction the continuous interaction in progress
*/
void HbWidgetFeedback::continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->continuousStopped(widget, interaction);
    }
}
