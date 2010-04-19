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
#include "hbfeedbackplayer.h"

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

    \brief Tool class for instant feedback effects.

    Instant feedbacks are used to initiate fire&forget type of sound and haptic effects defined in the device themes.
    Effects are used as a feedback indication to the user when she/he is navigating and interacting
    with the device.
*/

/*!
    \fn void HbInstantFeedback::setInstantEffect(HbFeedback::InstantEffect effect)

    Sets the instant effect that determines what kind of haptic and sound effects will
    be played when calling HbFeedbackPlayer::playInstantFeedback() or when user touches the screen
    over the hit area with the given instant effect. The actual effects are
    defined in the device themes.
*/

void HbInstantFeedback::setInstantEffect(HbFeedback::InstantEffect effect)
{
    d->cEffect = effect;
}

/*!
    \fn void HbFeedback::InstantEffect HbInstantFeedback::instantEffect() const

    Returns the instant effect of the instant feedback object. Instant effect is used to determine what kind of
    haptic and sound effects will be played when calling HbFeedbackPlayer::playInstantFeedback() or
    when user touches the screen over the hit area with the given instant effect. The actual effects
    are defined in the device themes.
*/

HbFeedback::InstantEffect HbInstantFeedback::instantEffect() const
{
    return d->cEffect;
}

/*!
    \fn bool HbInstantFeedback::isValid() const

    Instant feedback is valid if a proper instant effect (not HbFeedback::None) has beed
    defined for the feedback.
*/

bool HbInstantFeedback::isValid() const
{
    switch(d->cEffect) {
    case HbFeedback::None:
    case HbFeedback::NoOverride:
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

    \param effect instant feedback to be played
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
    Initiates the instant feedback effect.
*/
void HbInstantFeedback::play()
{
    HbFeedbackPlayer* player = HbFeedbackPlayer::instance();
    if (player) {
        player->playInstantFeedback(d->cEffect);
    }
}

/*!
    Initiates the given instant feedback effect.
*/
void HbInstantFeedback::play(HbFeedback::InstantEffect effect)
{
    HbFeedbackPlayer* player = HbFeedbackPlayer::instance();
    if (player) {
        player->playInstantFeedback(effect);
    }
}

/*!
    Assigns a copy of the feedback \a feedback to this feedback, and returns a
    reference to it.
*/
HbInstantFeedback &HbInstantFeedback::operator=(const HbInstantFeedback & feedback)
{
    HbAbstractFeedback::operator =(feedback);
    setInstantEffect(feedback.instantEffect());
    return *this;
}

/*!
    Returns true if this feedback has the same configuration as the feedback \a
    feedback; otherwise returns false.
*/
bool HbInstantFeedback::operator==(const HbInstantFeedback &feedback) const
{
    return (rect() == feedback.rect()
            && window() == feedback.window()
            && d->cEffect == feedback.instantEffect());
}

/*!
    Returns true if this feedback has different configuration than the feedback \a
    feedback; otherwise returns false.
*/
bool HbInstantFeedback::operator!=(const HbInstantFeedback &feedback) const
{
    return !(*this == feedback);
}
