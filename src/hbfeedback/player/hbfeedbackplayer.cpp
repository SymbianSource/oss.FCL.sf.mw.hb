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

#include "hbfeedbackplayer_p.h"
#include "hbfeedbackplayer_p_p.h"

#include <hbinstantfeedback.h>
#include <hbcontinuousfeedback.h>
#include <hbfeedbacksettings.h>

#ifdef FEEDBACK_TEST_EVENT
#include "hbfeedbacktestevent_p.h"
#endif

#include <QApplication>

#ifdef Q_OS_SYMBIAN
#include "hbfeedbackplayer_symbian_p.h"
#else
#include "hbfeedbackplayer_stub_p.h"
#endif

HbFeedbackPlayerPrivate::HbFeedbackPlayerPrivate(HbFeedbackPlayer* parent) : parent(parent), basePlayer(0)
{
}

HbFeedbackPlayerPrivate::~HbFeedbackPlayerPrivate()
{
    if (basePlayer) {
        delete basePlayer;
    }
    if (feedbackSettings) {
        delete feedbackSettings;
    }
}

void HbFeedbackPlayerPrivate::init()
{
    feedbackSettings = new HbFeedbackSettings(this);
    connect(feedbackSettings, SIGNAL(feedbackDisabled()),
            this, SLOT(feedbackDisabled()));

    basePlayer = new HbFeedbackBasePlayer();
}

void HbFeedbackPlayerPrivate::feedbackDisabled()
{
    if (basePlayer) {
        basePlayer->cancelContinuousFeedbacks();
    }
}

Q_GLOBAL_STATIC(HbFeedbackPlayer, feedbackPlayerGlobal);

/*!  
    \internal

    Constructor.

    Feedback player is used to initiate various haptic and sound feedback effects for the device.

    Current player supports instant feedback and continuous feedback effects. Separate HbFeedbackSettings 
    interface is reserved for applications wanting to limit or disable feedback effect playing.

    \sa HbInstantFeedback, HbContinuousFeedback, HbFeedbackSettings

*/
HbFeedbackPlayer::HbFeedbackPlayer() : d(new HbFeedbackPlayerPrivate(this))
{
    d->init();
}

/*!
    \internal
    Destructor.

    \sa HbInstantFeedback, HbContinuousFeedback
*/
HbFeedbackPlayer::~HbFeedbackPlayer()
{
    delete d;
}

/*!
    \internal
    Returns the handle to the global instance.
        
    \sa HbInstantFeedback, HbContinuousFeedback
*/
HbFeedbackPlayer* HbFeedbackPlayer::instance()
{
    return feedbackPlayerGlobal();
}

/*!
    \internal
    Returns a reference to the feedback settings interface.
        
    \sa HbFeedbackSettings

*/
HbFeedbackSettings* HbFeedbackPlayer::settings()
{
    return d->feedbackSettings;
}

/*!
    \internal
    Triggers instant feedback effects.

    \param feedback instant feedback object
    \sa HbInstantFeedback

*/
void HbFeedbackPlayer::playInstantFeedback(const HbInstantFeedback& feedback)
{
    if (feedback.isValid() && d->feedbackSettings->isFeedbackEnabled()) {
        if (d->basePlayer)  {
                d->basePlayer->playInstantFeedback(feedback);
            }
#ifdef FEEDBACK_TEST_EVENT
        HbFeedbackTestEvent te(feedback);
        qApp->sendEvent(this, &te);
#endif
    }
}

/*!
    \internal
    Starts a continuous feedback effect.

    \param feedback continuous feedback object
    \return identifier The identifier for the started effect.

    \sa HbContinuousFeedback
*/
int HbFeedbackPlayer::startContinuousFeedback(const HbContinuousFeedback& feedback)
{
    int identifier(-1);
    if (feedback.isValid() && d->feedbackSettings->isFeedbackEnabled()) {
        if (d->basePlayer)  {
            identifier = d->basePlayer->startContinuousFeedback(feedback);
#ifdef FEEDBACK_TEST_EVENT
            HbFeedbackTestEvent te(feedback, HbFeedbackTestEvent::Start, identifier);
            qApp->sendEvent(this, &te);
#endif
        }
    } else if (!feedback.window()) {
        qWarning("HbFeedbackPlayer::startContinuousFeedback: no window defined for the feedback.");

    }

    return identifier;
}

/*!
    \internal
    Updates an ongoing continuous feedback effect.

    \param identifier The identifier for the ongoing effect.
    \param feedback continuous feedback object

    \sa HbContinuousFeedback
    
*/
void HbFeedbackPlayer::updateContinuousFeedback(int identifier, const HbContinuousFeedback& feedback)
{
    if (feedback.isValid() && d->feedbackSettings->isFeedbackEnabled()) {
        if (d->basePlayer)  {
            d->basePlayer->updateContinuousFeedback(identifier, feedback);
#ifdef FEEDBACK_TEST_EVENT
            HbFeedbackTestEvent te(feedback, HbFeedbackTestEvent::Update, identifier);
            qApp->sendEvent(this, &te);
#endif
        }

    } else if (!feedback.window()) {
        qWarning("HbFeedbackPlayer::updateContinuousFeedback: no window defined for the feedback.");
    }
}

/*!
    \internal
    Cancels an ongoing continuous feedback effect.

    \param identifier The identifier for the ongoing effect.
    
    \sa HbContinuousFeedback
*/
void HbFeedbackPlayer::cancelContinuousFeedback(int identifier)
{
    if (d->basePlayer)  {
#ifdef FEEDBACK_TEST_EVENT
        if (d->basePlayer->continuousFeedbackOngoing(identifier)) {
            HbContinuousFeedback feedback;
            HbFeedbackTestEvent te(feedback, HbFeedbackTestEvent::Stop, identifier);
            qApp->sendEvent(this, &te);
        }
#endif
        d->basePlayer->cancelContinuousFeedback(identifier);
    }
}

/*!
    \internal
    Cancels all ongoing continuous feedback effects.
        
    \sa HbContinuousFeedback
*/
void HbFeedbackPlayer::cancelContinuousFeedbacks()
{
    if (d->basePlayer)  {
        d->basePlayer->cancelContinuousFeedbacks();
    }
}

/*!
    \internal
    Checks if the given continuous feedback effect is currently running.

    \param identifier The identifier for the ongoing effect.

    \return true, if the effect is ongoing.
    
    \sa HbContinuousFeedback
*/
bool HbFeedbackPlayer::continuousFeedbackOngoing(int identifier)
{
    bool feedbackOngoing(false);
    if (d->basePlayer) {
        feedbackOngoing = d->basePlayer->continuousFeedbackOngoing(identifier);
    }
    return feedbackOngoing;
}
