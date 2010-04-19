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

#include "hbtacticonfeedback.h"
#include "hbfeedbackplayer.h"

#include <QGraphicsItem>
#include <QGraphicsView>

class HbTacticonFeedbackPrivate
{
public:
    HbTacticonFeedbackPrivate() : cEffect(HbFeedback::TacticonNeutral) {};
    ~HbTacticonFeedbackPrivate() {};

public:
    HbFeedback::TacticonEffect cEffect;
};

/*!
    \deprecated HbTacticonFeedback
        is deprecated. Please use HbInstantFeedback instead.

    @hbfeedback

    \class HbTacticonFeedback

    \brief Tool class for tacticon effects. [DEPRECATED]

    Tacticon effect can be either positive, negative and neutral. Tacticons are normally used
    in conjuction with various warning, error and information dialogs. The actual effects are
    defined in the device themes.

    \sa HbInstantFeedback
*/

/*!
    Initiates the instant feedback effect.

    \deprecated HbTacticonFeedback::play()
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
void HbTacticonFeedback::play()
{
    HbFeedbackPlayer* player = HbFeedbackPlayer::instance();
    if (player) {
        player->playTacticonFeedback(d->cEffect);
    }
}

/*!
    Initiates the given instant feedback effect.

    \param effect the tacticon effect to be played

    \deprecated HbTacticonFeedback::play(HbFeedback::TacticonEffect)
        is deprecated. Please use HbInstantFeedback instead.
    
    \sa HbInstantFeedback
*/
void HbTacticonFeedback::play(HbFeedback::TacticonEffect effect)
{
    HbFeedbackPlayer* player = HbFeedbackPlayer::instance();
    if (player) {
        player->playTacticonFeedback(effect);
    }
}

/*!
    \fn void HbTacticonFeedback::setTacticonEffect(HbFeedback::TacticonEffect effect)

    Sets the tacticon effect that determines what kind of haptic and sound effects will
    be played when calling HbFeedbackPlayer::playTacticonFeedback(). The actual effects are
    defined in the device themes.

    \deprecated HbTacticonFeedback::setTacticonEffect(HbFeedback::TacticonEffect)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

void HbTacticonFeedback::setTacticonEffect(HbFeedback::TacticonEffect effect)
{
    d->cEffect = effect;
}

/*!
    \fn HbFeedback::TacticonEffect HbTacticonFeedback::tacticonEffect() const

    Returns the tacticon effect of the tacticon feedback object. Tacticon effect is used to determine what
    kind of haptic and sound effects will be played when calling HbFeedbackPlayer::playTacticonFeedback().
    The actual effects are defined in the device themes.

    \deprecated HbTacticonFeedback::tacticonEffect() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

HbFeedback::TacticonEffect HbTacticonFeedback::tacticonEffect() const
{
    return d->cEffect;
}

/*!
    \fn bool HbTacticonFeedback::isValid() const

    Tacticon feedback is valid if a proper tacticon effect (not HbFeedback::TacticonNone) has beed
    defined for the feedback.

    \deprecated HbTacticonFeedback::isValid() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

bool HbTacticonFeedback::isValid() const
{
    return d->cEffect != HbFeedback::TacticonNone;
}

/*!
    \fn HbFeedback::Type HbTacticonFeedback::type() const

    Returns HbFeedback::TypeTacticon.

    \deprecated HbTacticonFeedback::type() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

/*!
    Constructor.

    \deprecated HbTacticonFeedback::HbTacticonFeedback()
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbTacticonFeedback::HbTacticonFeedback() : d(new HbTacticonFeedbackPrivate)
{
}

/*!
    Constructor.

    \param effect tacticon feedback to be played

    \deprecated HbTacticonFeedback::HbTacticonFeedback(HbFeedback::TacticonEffect)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbTacticonFeedback::HbTacticonFeedback(HbFeedback::TacticonEffect effect) : d(new HbTacticonFeedbackPrivate)
{
    d->cEffect = effect;
}

/*!
    Destructor.

    \deprecated HbTacticonFeedback::~HbTacticonFeedback()
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbTacticonFeedback::~HbTacticonFeedback()
{
    delete d;
}

/*!
    Assigns a copy of the feedback \a feedback to this feedback, and returns a
    reference to it.

    \deprecated HbTacticonFeedback::operator =(const HbTacticonFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbTacticonFeedback &HbTacticonFeedback::operator=(const HbTacticonFeedback & feedback)
{
    HbAbstractFeedback::operator =(feedback);
    setTacticonEffect(feedback.tacticonEffect());
    return *this;
}

/*!
    Returns true if this feedback has the same configuration as the feedback \a
    feedback; otherwise returns false.

    \deprecated HbTacticonFeedback::operator ==(const HbTacticonFeedback&) const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
bool HbTacticonFeedback::operator==(const HbTacticonFeedback &feedback) const
{
    return (rect() == feedback.rect()
            && window() == feedback.window()
            && d->cEffect == feedback.tacticonEffect());
}

/*!
    Returns true if this feedback has different configuration than the feedback \a
    feedback; otherwise returns false.

    \deprecated HbTacticonFeedback::operator !=(const HbTacticonFeedback&) const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
bool HbTacticonFeedback::operator!=(const HbTacticonFeedback &feedback) const
{
    return !(*this == feedback);
}
