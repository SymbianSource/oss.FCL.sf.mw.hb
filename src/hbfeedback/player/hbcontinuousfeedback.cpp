/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbFeedback module of the UI Extensions for Mobile.
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

#include "hbcontinuousfeedback.h"
#include "hbfeedbackplayer.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QDebug>

class HbContinuousFeedbackPrivate
{
public:
    HbContinuousFeedbackPrivate() : cEffect(HbFeedback::ContinuousSmooth),
        cFeedbackId(-1),
        cTimeout(HbFeedback::StandardFeedbackTimeout),
        cIntensity(HbFeedback::IntensityFull)
    {
    };

    ~HbContinuousFeedbackPrivate() {};

public:
    HbFeedback::ContinuousEffect cEffect;
    int cFeedbackId;
    int cTimeout;
    int cIntensity;
};

/*!
    @beta
    @hbfeedback

    \class HbContinuousFeedback

    \brief Tool class for continuous feedback effects.

    Continuous feedbacks are used to play sound and haptic effects that last as long as the user is touching the screen,
    for example when dragging an interface object, scrolling a list or a slider. Continuous feedback
    effects need to be started, updated and cancelled during the life time of the feedback using methods
    HbFeedbackPlayer::startContinuousFeedback(), HbFeedbackPlayer::updateContinuousFeedback() and
    HbFeedbackPlayer::cancelContinuousFeedback(). The effect intensity can be varied during the feedback.
*/


/*!
    \fn void HbContinuousFeedback::setContinuousEffect(HbFeedback::ContinuousEffect effect)

    Sets the continuous effect that determines what kind of continuous haptic and sound effects will
    be played when calling HbFeedbackPlayer::startContinuousFeedback(). The actual effects are
    defined in the device themes.
*/

void HbContinuousFeedback::setContinuousEffect(HbFeedback::ContinuousEffect effect)
{
    d->cEffect = effect;
}

/*!
    \fn void HbFeedback::ContinuousEffect HbContinuousFeedback::continuousEffect() const

    Returns the continuous effect of the continuous feedback object. Continuous effect is used to determine what kind of continuous
    haptic and sound effects will be played when calling HbFeedbackPlayer::startContinuousFeedback(). The actual effects are
    defined in the device themes.
*/

HbFeedback::ContinuousEffect HbContinuousFeedback::continuousEffect() const
{
    return d->cEffect;
}

/*!
    \fn int HbContinuousFeedback::timeout() const

    The timeout value of the feedback in milliseconds. Continuous feedback is
    automatically cancelled if previously started continuous feedback hasn't been
    updated within the timeout.
*/

int HbContinuousFeedback::timeout() const {
    return d->cTimeout;
}

/*!
    \fn int HbContinuousFeedback::intensity() const

    The intensity of the continuous feedback effect. Intensity
    can be varied between values zero and HbFeedback::IntensityFull = 100.
*/

int HbContinuousFeedback::intensity() const {
    return d->cIntensity;
}

/*!
    \fn HbFeedback::Type HbContinuousFeedback::type() const

    Returns HbFeedback::TypeContinuous.
*/

/*!
    Constructor.
*/
HbContinuousFeedback::HbContinuousFeedback() :d(new HbContinuousFeedbackPrivate)
{
}

/*!
    Constructor.

    \param effect continuous feedback to be played
    \param widget used to determine the window where continuous feedback is active.
           There can only be one ongoing continuous feedback per application window.
*/
HbContinuousFeedback::HbContinuousFeedback(HbFeedback::ContinuousEffect effect, const QWidget *widget) : d(new HbContinuousFeedbackPrivate)
{
    d->cEffect = effect;
    setOwningWindow(widget);
}

/*!
    Destructor.
*/
HbContinuousFeedback::~HbContinuousFeedback()
{
    delete d;
}

/*!
    Sets the timeout value in milliseconds. Continuous feedback is automatically cancelled
    if the continuous feedback hasn't been updated within the timeout.
*/
void HbContinuousFeedback::setTimeout(int msecTimeout)
{
    if (msecTimeout > 0) {
        d->cTimeout = msecTimeout;
    }
}

/*!
    Sets the intensity of the continuous feedback effect. The intensity
    has to always be between HbFeedback::IntensityZero and HbFeedback::IntensityFull.
*/
void HbContinuousFeedback::setIntensity(int intensity)
{
    if (intensity >= 0 && intensity <= HbFeedback::IntensityFull) {
        d->cIntensity = intensity;
    }
}
/*!
    Plays the continuous feedback.
*/
void HbContinuousFeedback::play()
{
    HbFeedbackPlayer* feedbackPlayer = HbFeedbackPlayer::instance();

    if (feedbackPlayer) {
        d->cFeedbackId = feedbackPlayer->startContinuousFeedback(*this);
    }
}
/*!
    Stops the continous feedback.
  */
void HbContinuousFeedback::stop()
{
    HbFeedbackPlayer* feedbackPlayer = HbFeedbackPlayer::instance();

    if (feedbackPlayer) {
        feedbackPlayer->cancelContinuousFeedback(d->cFeedbackId);
    }
}


/*!
    Returns true if the continuous feedback is being played.
*/
bool HbContinuousFeedback::isPlaying()
{
    bool feedbackOngoing = false;
    HbFeedbackPlayer* feedbackPlayer = HbFeedbackPlayer::instance();

    if (feedbackPlayer) {
        feedbackOngoing = feedbackPlayer->continuousFeedbackOngoing(d->cFeedbackId);
    }

    return feedbackOngoing;
}

/*!
  \deprecated HbContinuousFeedback::isValid() const
        is deprecated.

    Continuous feedback is valid if the feedback effect is not set to HbFeedback::ContinuousNone
    and if the owning window has been defined. There can only be one ongoing continuous feedback effect
    per one application window.
*/
bool HbContinuousFeedback::isValid() const
{
    switch(d->cEffect) {
    case HbFeedback::NoContinuousOverride :
        return false;
    default:
        return d->cEffect != HbFeedback::ContinuousNone && window();
    }
};


/*!
    Assigns a copy of the feedback \a feedback to this feedback, and returns a
    reference to it.
*/
HbContinuousFeedback &HbContinuousFeedback::operator=(const HbContinuousFeedback &feedback)
{
    HbAbstractFeedback::operator =(feedback);
    setContinuousEffect(feedback.continuousEffect());
    setTimeout(feedback.timeout());
    setIntensity(feedback.intensity());
    return *this;
}

/*!
    Returns true if the continuous feedbacks have the same parent window,
    area rect, continuous effect, timeout and intensity.
*/
bool HbContinuousFeedback::operator==(const HbContinuousFeedback &feedback) const
{
    return (rect() == feedback.rect()
            && window() == feedback.window()
            && d->cEffect == feedback.continuousEffect()
            && d->cTimeout == feedback.timeout()
            && d->cIntensity == feedback.intensity());
}

/*!
    Returns true if the continuous feedbacks have different parent window,
    area rect, continuous effect, timeout or intensity.
*/
bool HbContinuousFeedback::operator!=(const HbContinuousFeedback &feedback) const
{
    return !(*this == feedback);
}
