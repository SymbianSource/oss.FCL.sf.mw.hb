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

    \brief The HbWidgetFeedback class provides an interface for widgets
    to inform the feedback framework about user interactions in order
    to trigger feedback effects.
    
    The HbWidgetFeedback class is for use in widget implementations.
    It is not relevant when you are simply using existing widgets in
    an application.
    
    As a widget developer, you may want to give feedback when the user
    interacts with your widget. Audio signals or vibration are examples of
    possible feedback effects. To trigger the feedback effects supported by
    the feedback framework, you must inform the feedback framework about
    the detected user interaction and its specific type.
    
    Interactions are divided into two main types:
    
    <ul>
    <li><b>Instant interactions.</b> Short user actions, such as clicking
    a button or changing a selection on a list. A typical feedback effect
    for these interactions is a fire-and-forget type of event, where
    the physical response is played from beginning to end.</li>
    <li><b>Continuous interactions.</b> Longer user actions, such as scrolling
    or dragging. Playing the feedback for these interactions continues until
    explicitly stopped.</li>
    </ul>
    
    HbWidgetFeedback provides static methods for informing the feedback
    framework about user interactions:
    \link HbWidgetFeedback::triggered() triggered() \endlink for instant interactions,
    and \link HbWidgetFeedback::continuousTriggered() continuousTriggered() \endlink
    and \link HbWidgetFeedback::continuousStopped() continuousStopped() \endlink
    for continuous interactions. Information on the target widget of the interaction
    and the interaction type is passed in the method parameters. The feedback framework
    makes decisions about the actual effects, based on the widget type, situation,
    and specific interaction type.
    
    A widget should only call interaction methods when the user actually
    interacts with the widget, and not when the widget state changes for some
    other reason. For example, feedback effects are not desirable when
    an application resets widget states by calling the widget API, or when
    a progress bar is moved during a file transfer.
    
    How you use this class or whether you need to use it at all depends on 
    what kind of widget you are developing:
    
    <b>Standard widgets.</b> If you are developing a standard %Hb widget, 
    such as HbPushButton, HbComboBox, and so on, you should use this class,
    %HbWidgetFeedback, for triggering feedback effects upon user interaction.
    See the use case below for details.
    
    <b>Custom widgets that derive from a standard widget.</b> If you are 
    developing a custom widget that derives from one of the standard
    %Hb widgets (such as HbPushButton, HbComboBox, and so on),
    the base class widget most probably has predefined feedback effects,
    and you do not need to do anything. The base class takes care of triggering
    the default effects upon user interaction.
    
    <b>Custom widgets that derive directly from HbWidget.</b> If your custom
    widget derives directly from HbWidget, and not through any of the standard
    %Hb widgets, you should not use %HbWidgetFeedback. The recommended way
    to define feedback for your widget is to use classes HbInstantFeedback and
    HbContinuousFeedback.
    
    \section _usecases_hbwidgetfeedback Using HbWidgetFeedback
    
    \subsection _uc_standardhb_hbwidgetfeedback Supporting feedback effects in a standard widget
    
    Call the HbWidgetFeedback methods in a standard widget when the user
    interacts with the widget. Choose the correct method according to
    the interaction style: instant or continuous.
            
    To support instant feedback features in a standard %Hb widget, call 
    HbWidgetFeedback::triggered() with a suitable Hb::InstantInteraction
    parameter to specify the interaction type, whenever you detect any
    user interaction that is instant by nature. For example:
    
    \code
    // The widget has been pressed down and released
    HbWidgetFeedback::triggered(this, Hb::InstantClicked);
    \endcode
    
    See Hb::InstantInteraction for the list of all predefined instant
    interactions.
    
    Whenever you detect any of the specified continuous user interactions in
    your standard %Hb widget, or when the continuous interaction ends, call
    either HbWidgetFeedback::continuousTriggered() or HbWidgetFeedback::continuousStopped()
    with the suitable Hb::ContinuousInteraction parameter. For example:
    
    \code
    // User has started moving the slider handle.
    HbWidgetFeedback::continuousTriggered(this, Hb::ContinuousDragged);
    \endcode
    
    \code
    // User has stopped moving the slider handle.
    HbWidgetFeedback::continuousStopped(this, Hb::ContinuousDragged);
    \endcode
    
    See Hb::ContinuousInteraction for the list of all predefined
    continuous interactions.
        

    \sa HbInstantFeedback, HbContinuousFeedback
*/

/*!
    Passes information about an instant interaction from the widget to the
    feedback manager, which forwards it to all active feedback plugins.

    \param widget Target widget of the interaction
    \param interaction The instant interaction type
    \param modifiers Optional Hb::InteractionModifier flags with more detailed
    information about the interaction
*/
void HbWidgetFeedback::triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers)
{
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->triggered(widget, interaction, modifiers);
    }
}

/*!
    Passes information about a started continuous interaction from the
    widget to the feedback manager, which forwards it to all active feedback
    plugins.

    \param widget Target widget of the interaction
    \param interaction The continuous interaction type
    \param delta The direction and distance of the interaction
*/
void HbWidgetFeedback::continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->continuousTriggered(widget, interaction, delta);
    }
}

/*!
    Passes information about the stopped continuous interaction from the
    widget to the feedback manager, which forwards it to all active feedback
    plugins. This method is needed for stopping the continuous feedback
    effects started by the continuous interaction.

    \param widget Target widget of the interaction
    \param interaction The continuous interaction type
*/
void HbWidgetFeedback::continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->continuousStopped(widget, interaction);
    }
}
