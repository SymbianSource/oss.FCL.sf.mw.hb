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

#ifndef HBFEEDBACKNAMESPACE_H
#define HBFEEDBACKNAMESPACE_H

#include <QtGlobal>
#include <QObject>
#include <hbglobal.h>

/*!
    \namespace HbFeedback
    \brief The HbFeedback namespace lists the feedback effects supported 
    by the current haptic software and hardware.
*/

#ifndef HB_DOXYGEN
class HB_CORE_EXPORT HbFeedback
{
    Q_GADGET
    Q_ENUMS(Type InstantEffect ContinuousEffect TacticonEffect HitAreaType IntensityLevel Modality)
    Q_FLAGS(Types Modalities)

public:
#else
namespace HbFeedback
{
#endif // HB_DOXYGEN

    /*!
        Three different types of feedback mechanisms are supported: instant,
        continuous and hit area feedbacks.
     */

    enum Type {
        TypeInstant = 0x001,
        TypeContinuous = 0x002,
        TypeHitArea = 0x004,
        TypeTacticon = 0x008
    };

    Q_DECLARE_FLAGS(Types, Type)

    /*!
        Instant feedback is fire&forget approach that initiates short
        haptic and sound effect which lasts on average between 100-500
        milliseconds. Each effect value corresponds to a certain haptic
        and/or sound effect defined in the themes.

        \sa HbInstantFeedback
     */

    enum InstantEffect {
        None,
        Basic, // For generic instant feedback for custom widgets
        Sensitive, // For generic instant feedback for custom widgets
        BasicButton,
        SensitiveButton,
        BasicKeypad,
        SensitiveKeypad,
        BasicSlider,
        SensitiveSlider,
        BasicItem,
        SensitiveItem,
        ItemScroll,
        ItemPick,
        ItemDrop,
        ItemMoveOver,
        BounceEffect,
        Checkbox,
        MultipleCheckbox,
        Editor,
        TextSelection,
        BlankSelection,
        LineSelection,
        EmptyLineSelection,
        PopUp,
        PopupOpen,
        PopupClose,
        Flick,
        StopFlick,
        MultitouchActivate,
        RotateStep,
        LongPress,
        PositiveTacticon,
        NeutralTacticon,
        NegativeTacticon,
        /* new standard instant feedbacks here */
        NumberOfInstantFeedbacks,
        NoOverride, // utility effect
        InstantUser = 65535,
        /* user defined custom instant feedbacks */
        InstantMaxUser = 262140
    };

    /*!
        Continuous feedback has to be explicitly started, updated and
        cancelled by the system and is used to provide ongoing feedback
        whenever user is dragging slider handles or swiping flickable widgets
        with her finger. Continuous feedback intensity can be varied
        during playback between values 0 and 100.

        \sa HbContinuousFeedback
     */

    enum ContinuousEffect {
        ContinuousNone,
        ContinuousSmooth, // For generic continuous feedback for custom widgets
        ContinuousSlider,
        ContinuousPopup,
        ContinuousInput,
        ContinuousPinch,
        /* new standard continuous feedbacks here */
        NumberOfContinuousFeedbacks,
        NoContinuousOverride, // utility effect
        ContinuousUser = 1000,
        /* user defined continuous instant feedbacks */
        ContinuousMaxUser = 65535
    };

    /*!
        \deprecated HbFeedback::TacticonEffect
            is deprecated. Use HbFeedback::InstantEffect instead.

        Tacticon feedback is a special kind of instant
        feedback reserved for tacticon use cases.

        \sa HbTacticonsFeedback
     */

    enum TacticonEffect {
        TacticonNone,
        TacticonPositive,
        TacticonNeutral,
        TacticonNegative,
        /* new tacticon feedbacks here */
        NumberOfTacticonFeedbacks
    };

    /*!
        Minimum, smooth and maximum intensity values of continuous feedback.

        \sa HbContinuousFeedback
     */
    enum IntensityLevel {
        IntensityZero = 0,
        IntensitySmooth = 50,
        IntensityFull = 100
    };

    /*!
        Hit area feedback is a special type of low-latency instant
        feedback that is initiated by preregistered hit area rectangles
        in the windowing system that are hit area matched before the
        touch events are even forwarded to the application windows.
        Hit area feedback can be set to initiate either when finger or
        stylus a.) presses down the screen or b.) is released from the
        screen.

        \sa HbHitAreaFeedback
     */
    enum HitAreaType {
        MouseButtonPress,
        MouseButtonRelease
    };

    /*!
      \enum Modality
      The available modalities for feedback effects.
      Effects can be synthesized using one or several of the available modalities.
    */
    enum Modality {
        All     = 0xFFFF,
        Audio   = 0x0001,
        Tactile = 0x0002
    };

    Q_DECLARE_FLAGS(Modalities, Modality)

    /*!
        Timeout value has to be defined for each continuous feedback
        to avoid situations where continuous feedback is never cancelled
        and accidentally continues to play infinitely.

        Recommended standard value is 300 milliseconds.

        \sa HbContinuousFeedback
     */
    static const int StandardFeedbackTimeout = 300;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbFeedback::Types)
Q_DECLARE_OPERATORS_FOR_FLAGS(HbFeedback::Modalities)

#endif // HBFEEDBACKNAMESPACE_H

