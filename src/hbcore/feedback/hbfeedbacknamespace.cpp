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
    \brief The HbFeedback namespace contains the feedback effects supported by the current haptic frawework.
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

    Instant feedback is fire&forget type of feedback which initiates a short haptic and/or sound 
    feedback effect. Each enumeration value corresponds to a certain instant feedback effect.

    \sa HbInstantFeedback
*/

/*!
    \enum HbFeedback::ContinuousEffect

    Continuous feedback is a feedback type, which has to be explicitly started, updated and
    stopped by the framework and is used to provide ongoing feedback in situations in which
    the user is performing some longer duration touch interaction, e.g. dragging slider handle
    to change the slider value. Continuous feedback intensity can be updated during the playback 
    between values 0 and 100.

    \sa HbContinuousFeedback
*/

/*!
    \enum HbFeedback::IntensityLevel

    A set of predefined values for continuous feedback intensity.

    \sa HbContinuousFeedback
*/

/*!
    \enum HbFeedback::Modality
    
    The available modalities for feedback effects. Effects can be played using one or several 
    of the available modalities, e.g. haptic, audio.

    \sa HbInstantFeedback, HbContinuousFeedback
*/

/*!
    HbFeedback::StandardFeedbackTimeout

    A timeout value has to be defined for each continuous feedback to avoid situations where 
    the continuous feedback is never stopped and unintentionally continues to play indefinetly.

    The recommended standard value is 300 milliseconds.

    \sa HbContinuousFeedback
*/
