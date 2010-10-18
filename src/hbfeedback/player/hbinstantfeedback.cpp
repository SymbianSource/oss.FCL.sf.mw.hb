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

#include "hbinstantfeedback.h"
#include "hbfeedbackplayer_p.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QDebug>

class HbInstantFeedbackPrivate
{
public:
    HbInstantFeedbackPrivate() : cEffect(HbFeedback::SensitiveButton) {};
    ~HbInstantFeedbackPrivate() {};

public:
    HbFeedback::InstantEffect cEffect;
};

/*!
    @beta
    @hbfeedback

    \class HbInstantFeedback

    \brief The HbInstantFeedback class is used for setting and playing
    instant feedback effects from custom widgets.

    Instant feedback requests are used for initiating short, fire-and-forget
    type of feedback effects as a response to short user interactions,
    such as clicking a button or selecting an item on a list. You can
    use the class HbInstantFeedback to ask the feedback framework to play
    a selected instant effect, by giving HbInstantFeedback a parameter
    of type HbFeedback::InstantEffect. The type HbFeedback::InstantEffect
    has predefined instant effects for many widget types, such as buttons,
    checkboxes, sliders, and so on.
    
    When you call play() from your widget, HbInstantFeedback calls the internal
    feedback framework. If effects are enabled in the current application,
    the effect will be played. Applications can disable effects with the class
    HbFeedbackSettings, when effects are not wanted. 
    
    If your custom widget derives from one of the standard %Hb widgets, it
    probably has some default feedback behaviour defined, and you should
    rely on the base class widget for triggering the feedback effects through
    the HbWidgetFeedback class. Note that if you choose to use this class,
    HbInstantFeedback, to play effects directly, you may end up getting
    feedback effects twice, both through your call to HbInstantFeedback and
    through the base class widget feedback mechanism.
    
    If your custom widget derives directly from HbWidget, and not through
    any other %Hb widget, HbInstantFeedback provides you with an easy way
    to play instant feedback effects. Just call the static method 
    HbInstantFeedback::play(HbFeedback::InstantEffect effect) to play the
    effect. Alternatively, you can instantiate an HbInstantFeedback object
    first and set the effect to be played, before calling play().
            
    
    
    \sa HbFeedback::InstantEffect, HbContinuousFeedback, HbWidgetFeedback
*/

/*!
    Sets the instant effect to be played when calling play().
    
    \sa instantEffect()
*/
void HbInstantFeedback::setInstantEffect(HbFeedback::InstantEffect effect)
{
    d->cEffect = effect;
}

/*!
    Returns the currently set instant effect. The instant effect is
    the feedback effect to be played when calling play().
    
    \sa setInstantEffect()
*/
HbFeedback::InstantEffect HbInstantFeedback::instantEffect() const
{
    return d->cEffect;
}

/*!
    Returns \c true if an instant effect (other than HbFeedback::None) has been
    defined for this object.
*/
bool HbInstantFeedback::isValid() const
{
    switch(d->cEffect) {
    case HbFeedback::None:
        return false;
    default:
        return true;
    }
}

/*!
    \fn HbFeedback::Type HbInstantFeedback::type() const

    Returns HbFeedback::TypeInstant.
*/

/*!
    Constructor.
*/
HbInstantFeedback::HbInstantFeedback() : d(new HbInstantFeedbackPrivate)
{
}

/*!
    Constructor.

    \param effect Instant feedback to be played.
*/
HbInstantFeedback::HbInstantFeedback(HbFeedback::InstantEffect effect) : d(new HbInstantFeedbackPrivate)
{
    d->cEffect = effect;
}

/*!
    Destructor.
*/
HbInstantFeedback::~HbInstantFeedback()
{
    delete d;
}

/*!
    Plays the currently set instant feedback effect.
*/
void HbInstantFeedback::play()
{
    HbFeedbackPlayer* feedbackPlayer = HbFeedbackPlayer::instance();
    if (feedbackPlayer) {
        feedbackPlayer->playInstantFeedback(*this);
    }
}

/*!
    Plays the given instant feedback effect.
    
    \sa HbFeedback::InstantEffect
*/
void HbInstantFeedback::play(HbFeedback::InstantEffect effect)
{
    HbFeedbackPlayer* player = HbFeedbackPlayer::instance();
    if (player) {
        player->playInstantFeedback(effect);
    }
}

/*!
    Assigns a copy of \a feedback to this object, and returns a
    reference to it.
*/
HbInstantFeedback &HbInstantFeedback::operator=(const HbInstantFeedback & feedback)
{
    HbAbstractFeedback::operator =(feedback);
    setInstantEffect(feedback.instantEffect());
    return *this;
}

/*!
    Returns \c true if this object has the same parameters as \a feedback,
    otherwise returns \c false.
*/
bool HbInstantFeedback::operator==(const HbInstantFeedback &feedback) const
{
    return (rect() == feedback.rect()
            && window() == feedback.window()
            && d->cEffect == feedback.instantEffect());
}

/*!
    Returns \c true if this object has different parameters from \a feedback,
    otherwise returns \c false.
*/
bool HbInstantFeedback::operator!=(const HbInstantFeedback &feedback) const
{
    return !(*this == feedback);
}
