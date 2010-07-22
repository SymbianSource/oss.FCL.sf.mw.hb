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
#include "hbfeedbackplayer_p.h"

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

    \brief Class for continuous feedback effects.

    Continuous feedbacks are feedback effects that last as long as the user is touching the screen,
    for example when dragging a slider handle. Continuous feedback effects need to be started, updated 
    and stopped using methods play() and stop().
*/

/*!
    \fn void HbContinuousFeedback::setContinuousEffect(HbFeedback::ContinuousEffect effect)

    Sets the continuous feedback effect that will be played by the feedback object.
    \sa continuousEffect()
*/

void HbContinuousFeedback::setContinuousEffect(HbFeedback::ContinuousEffect effect)
{
    d->cEffect = effect;
}

/*!
    \fn void HbFeedback::ContinuousEffect HbContinuousFeedback::continuousEffect() const

    Returns the continuous effect of the continuous feedback object.

    \sa setContinuousEffect()
*/

HbFeedback::ContinuousEffect HbContinuousFeedback::continuousEffect() const
{
    return d->cEffect;
}

/*!
    \fn int HbContinuousFeedback::timeout() const

    Returns the timeout value (in milliseconds) of the feedback object. Continuous feedback is
    automatically stopped if it is not updated within the timeout (by subsequent calls to
    play()).

    \sa setTimeout(), play()
*/

int HbContinuousFeedback::timeout() const {
    return d->cTimeout;
}

/*!
    \fn int HbContinuousFeedback::intensity() const

    Returns the intensity of the continuous feedback effect. Intensity can be varied between
    values HbFeedback::IntensityZero and HbFeedback::IntensityFull.

    \sa setIntensity()
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

    \param effect continuous feedback effect to be played
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
    Sets the timeout value (in milliseconds) of the feedback object. Continuous feedback is
    automatically stopped if it is not updated within the timeout (by subsequent calls to
    play()). This is a safety mechanism to prevent situations in which the  client fails to
    explicitly stop() a continuous feedback effect. The default timeout value is
    HbFeedback::StandardFeedbackTimeout.

    \sa timeout()
*/
void HbContinuousFeedback::setTimeout(int msecTimeout)
{
    if (msecTimeout > 0) {
        d->cTimeout = msecTimeout;
    }
}

/*!
    Sets the intensity of the continuous feedback effect. The intensity can be varied between 
    HbFeedback::IntensityZero and HbFeedback::IntensityFull.

    \sa intensity()
*/
void HbContinuousFeedback::setIntensity(int intensity)
{
    if (intensity >= 0 && intensity <= HbFeedback::IntensityFull) {
        d->cIntensity = intensity;
    }
}

/*!
    Starts/updates the continuous feedback.
    The feedback effect will be played untill the timeout is reached or a call to stop() is made.
    \sa stop()
*/
void HbContinuousFeedback::play()
{
    HbFeedbackPlayer *feedbackPlayer = HbFeedbackPlayer::instance();
    if(feedbackPlayer) {
        if( feedbackPlayer->continuousFeedbackOngoing(d->cFeedbackId)) {
            feedbackPlayer->updateContinuousFeedback(d->cFeedbackId,*this);
        } else {
            d->cFeedbackId = feedbackPlayer->startContinuousFeedback(*this);
        }
    }
}

/*!
    Stops the continuous feedback.
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
    Continuous feedback is valid if the feedback effect is not set to HbFeedback::ContinuousNone
    and if the owning window has been defined. There can only be one ongoing continuous feedback effect
    per one application window.
*/
bool HbContinuousFeedback::isValid() const
{
    return d->cEffect != HbFeedback::ContinuousNone && window();
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
