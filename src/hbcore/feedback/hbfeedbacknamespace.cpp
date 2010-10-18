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

#include <hbfeedbacknamespace.h>

/*!
    @hbcore
    \namespace HbFeedback
    \brief The HbFeedback namespace defines the feedback effects supported
    by the current feedback framework.
    
    As a widget developer, you may want to give feedback to the user
    corresponding to how the user interacts with the device. Feedback can
    be given using different modalities supported on the device, such as
    tactile, audio, or visual. Tactile feedback is feedback that is 'felt'
    by the user, for example vibration of the device. Extra visual feedback,
    in addition to the user interface changes on the screen, can be flashing
    the screen or switching extra lights on the device on or off. The feedback
    mechanism of the device can be further extended by implementing other
    modality plug-ins (see HbFeedbackEffectPlugin).
    
*/

/*!
    \enum HbFeedback::Type
    The supported feedback types.
*/

/*! 
    \var HbFeedback::Type HbFeedback::TypeInstant
    Instant feedback.
*/

/*! 
    \var HbFeedback::Type HbFeedback::TypeContinuous
    Continuous feedback.
*/

/*!
    \enum HbFeedback::InstantEffect

    Instant feedback is a fire-and-forget type of feedback which initiates a short
    tactile and/or sound feedback effect. Each enumeration value corresponds
    to a certain instant feedback effect.
    
    For many standard instant effects, there is both a 'basic' and 'sensitive'
    version. The sensitive feedback effect is for situations where the triggering
    action is not important, or where there can be a large number of feedback
    instances within a short time. An example of a less important touch event is
    when the focus is changed in a list, while an example of a situation where
    lots of events could be triggered is when the device user selects text and
    feedback is given for every character selected.

    \sa HbInstantFeedback
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::None
    No valid instant effect defined.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::Basic
    Basic generic instant feedback for custom widget interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::Sensitive
    Sensitive generic instant feedback for custom widget interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::BasicButton
    Basic instant feedback for button interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::SensitiveButton
    Sensitive instant feedback for button interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::BasicKeypad
    Basic instant feedback for keypad interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::SensitiveKeypad
    Sensitive instant feedback for keypad interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::BasicSlider
    Basic instant feedback for moving the slider.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::SensitiveSlider
    Sensitive instant feedback for moving the slider.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::BasicItem
    Basic instant feedback for interacting with an item view item
    (for example, a list or grid view item).
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::SensitiveItem
    Sensitive instant feedback for interacting with an item view item
    (for example, a list or grid view item).
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::ItemScroll
    Instant feedback for scrolling an item view.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::ItemPick
    Instant feedback for picking an item in an arrange mode.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::ItemDrop
    Instant feedback for dropping an item in an arrange mode.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::ItemMoveOver
    Instant feedback for moving an item in an arrange mode.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::BounceEffect
    Instant feedback for a bounce effect.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::Checkbox
    Instant feedback for selecting a checkbox item.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::MultipleCheckbox
    Instant feedback for selecting multiple checkbox items.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::Editor
    Instant feedback for generic editor interaction, for example
    when the editor gets focus.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::TextSelection
    Instant feedback for selecting text.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::BlankSelection
    Instant feedback for a blank selection.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::LineSelection
    Instant feedback for selecting a line.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::EmptyLineSelection
    Instant feedback for selecting an empty line.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::PopUp
    Instant feedback for a generic popup interaction.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::PopupOpen
    Instant feedback for opening a popup.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::PopupClose
    Instant feedback for closing a popup.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::Flick
    Instant feedback at the start of a flick (swipe) gesture.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::StopFlick
    Instant feedback when the user stops a flick (swipe) gesture
    by tapping the scroll item view.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::AdvancedGestureActivate
    Instant feedback when a touch gesture with more than one finger is activated:
    a second touch point is detected and a pinch gesture (for example zooming)
    is likely to follow.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::RotateStep
    Instant feedback for a rotation step.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::LongPress
    Instant feedback for a long press (tap-and-hold) gesture.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::PositiveTacticon
    Instant feedback for a notification of a successful action.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::NeutralTacticon
    Instant feedback for a notification.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::NegativeTacticon
    Instant feedback for a notification of a failed action.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::NumberOfInstantFeedbacks
    Used by the internal framework to keep track of the number of
    standard instant effects.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::InstantUser
    Start value for the range of custom instant effects.
*/

/*! 
    \var HbFeedback::InstantEffect HbFeedback::InstantMaxUser
    End value for the range of custom instant effects.
*/

/*!
    \enum HbFeedback::ContinuousEffect

    Continuous feedback is a feedback type that you can use to provide
    ongoing feedback in situations where the user is performing some
    longer duration touch interaction, such as dragging a slider handle
    to change the slider value. You need to explicitly start and stop
    a continuous feedback effect. You can update continuous feedback intensity
    during the playback between values 0 and 100 with HbContinuousFeedback::setIntensity().
    The feedback framework uses HbFeedback::ContinuousEffect for its
    internal purposes as well, and it will also stop any continuous feedback
    playback when a timeout occurs.

    \sa HbContinuousFeedback
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousNone
    No valid continuous effect defined.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousSmooth
    Generic continuous feedback for custom widget interaction.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousSlider
    Continuous feedback for dragging the slider.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousPopup
    Continuous feedback for popup interaction.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousInput
    Continuous feedback for giving input.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousPinch
    Continuous feedback for a pinch gesture.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::NumberOfContinuousFeedbacks
    Used by the internal framework to keep track of the number of
    standard continuous effects.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousUser
    Start value for the range of custom continuous effects.
*/

/*! 
    \var HbFeedback::ContinuousEffect HbFeedback::ContinuousMaxUser
    End value for the range of custom continuous effects.
*/


/*!
    \enum HbFeedback::IntensityLevel

    A set of predefined values for continuous feedback intensity.

    \sa HbContinuousFeedback
*/

/*! 
    \var HbFeedback::IntensityLevel HbFeedback::IntensityZero
    Minimum intensity, 0.
*/

/*! 
    \var HbFeedback::IntensityLevel HbFeedback::IntensitySmooth
    Medium intensity, 50.
*/

/*! 
    \var HbFeedback::IntensityLevel HbFeedback::IntensityFull
    Maximum intensity, 100.
*/

/*!
    \enum HbFeedback::Modality
    
    The available modalities for feedback effects. The currently supported
    modalities are audio and tactile. Effects can be played using both
    modalities or only one of them.

    \sa HbAbstractFeedback
*/

/*! 
    \var HbFeedback::Modality HbFeedback::All
    All available modalities.
*/

/*! 
    \var HbFeedback::Modality HbFeedback::Audio
    Audio modality.
*/

/*! 
    \var HbFeedback::Modality HbFeedback::Tactile
    Tactile modality.
*/

/*!
    HbFeedback::StandardFeedbackTimeout

    A timeout value has to be defined for each continuous feedback to avoid
    situations where the continuous feedback is never stopped and unintentionally
    continues to play indefinitely.

    The recommended standard value is 300 milliseconds.

    \sa HbContinuousFeedback
*/
