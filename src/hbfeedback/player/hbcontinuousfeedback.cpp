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

    \brief The HbContinuousFeedback class is used for setting, playing and
    stopping continuous feedback effects from custom widgets.

    Continuous feedback is used for feedback effects that last as long as the user
    is touching the screen, for example when dragging a slider handle.
    You can use the class HbContinuousFeedback to ask the feedback framework
    to play a selected continuous effect, by giving HbContinuousFeedback
    a parameter of type HbFeedback::ContinuousEffect. The type
    HbFeedback::ContinuousEffect has a number of predefined continuous effects,
    for example \link HbFeedback::ContinuousSlider ContinuousSlider \endlink and
    \link HbFeedback::ContinuousPinch ContinuousPinch\endlink.
    The effect \link HbFeedback::ContinuousSmooth ContinuousSmooth \endlink
    can be used for a generic continuous feedback. 
    
    Continuous feedback effects need to be started and stopped using the
    HbContinuousFeedback methods play() and stop(), respectively. The effect
    intensity can be varied during the feedback.
           
    When you call play() from your widget, HbContinuousFeedback calls
    the internal feedback framework. If effects are enabled in
    the current application, the effect will be played. Applications can
    disable effects with the class HbFeedbackSettings, when effects are
    not wanted.
    
    If your custom widget derives from one of the standard %Hb widgets, it
    probably has some default feedback behaviour defined, and you should
    rely on the base class widget for triggering the feedback effects through
    the HbWidgetFeedback class. Note that if you choose to use this class,
    HbContinuousFeedback, to play effects directly, you may end up getting
    feedback effects twice, both through your call to HbContinuousFeedback
    and through the base class widget feedback mechanism.
    
    If your custom widget derives directly from HbWidget, and not through
    any other %Hb widget, HbContinuousFeedback is the recommended way
    to play continuous feedback effects.
    
    \sa HbFeedback::ContinuousEffect, HbInstantFeedback, HbWidgetFeedback
    
*/

/*!
    Sets the continuous feedback effect to be played when calling play().
    
    \sa continuousEffect()
*/

void HbContinuousFeedback::setContinuousEffect(HbFeedback::ContinuousEffect effect)
{
    d->cEffect = effect;
}

/*!
    Returns the continuous effect of this object. The continuous effect is
    the continuous feedback effect that will be played when calling play().
    
    \sa HbFeedback::ContinuousEffect
*/

HbFeedback::ContinuousEffect HbContinuousFeedback::continuousEffect() const
{
    return d->cEffect;
}

/*!
    Returns the timeout value of the feedback in milliseconds. The continuous
    feedback is automatically stopped if it is not updated within the timeout
    (by subsequent calls to play()).
    
    \sa setTimeout(), play()
*/

int HbContinuousFeedback::timeout() const {
    return d->cTimeout;
}

/*!
    Returns the intensity of the continuous feedback effect. Intensity can vary
    between HbFeedback::IntensityZero (0) and HbFeedback::IntensityFull (100).
    
    \sa setIntensity(), HbFeedback::IntensityLevel
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
    
    If you use this constructor, you need to set the continuous feedback effect
    and an owning window before calling play().
*/
HbContinuousFeedback::HbContinuousFeedback() :d(new HbContinuousFeedbackPrivate)
{
}

/*!
    Constructor.

    \param effect Continuous feedback effect to be played
    \param widget Used to determine the window where the continuous feedback is active.
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
    Sets the timeout value in milliseconds. The continuous feedback is
    automatically stopped if the continuous feedback is not updated
    within the timeout (by subsequent calls to play()). This is a safety mechanism
    to prevent situations in which the  client fails to explicitly stop()
    a continuous feedback effect. The default timeout value is
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
    Sets the intensity of the continuous feedback effect. The intensity can vary
    between HbFeedback::IntensityZero (0) and HbFeedback::IntensityFull (100).
    Smaller or bigger values are ignored.
    
    \sa intensity(), HbFeedback::IntensityLevel
*/
void HbContinuousFeedback::setIntensity(int intensity)
{
    if (intensity >= 0 && intensity <= HbFeedback::IntensityFull) {
        d->cIntensity = intensity;
    }
}

/*!
    Starts/updates the currently set continuous feedback effect.
    The feedback effect will be played until the timeout is reached
    or a call to stop() is made.
    
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
    Returns \c true if the continuous feedback is currently being played.
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
    This object is valid if the feedback effect is not set to HbFeedback::ContinuousNone
    and if the owning window has been defined. There can only be one ongoing
    continuous feedback effect per one application window.
*/
bool HbContinuousFeedback::isValid() const
{
    return d->cEffect != HbFeedback::ContinuousNone && window();
};

/*!
    Assigns a copy of \a feedback to this object, and returns a
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
    Returns \c true if this object and \a feedback have the same parent window,
    area rect, continuous effect, timeout, and intensity.
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
    Returns \c true if this object and \a feedback have a different parent window,
    area rect, continuous effect, timeout, or intensity.
*/
bool HbContinuousFeedback::operator!=(const HbContinuousFeedback &feedback) const
{
    return !(*this == feedback);
}
