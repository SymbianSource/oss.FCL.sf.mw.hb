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

#include "hbfeedbackplayer.h"
#include "hbfeedbackplayer_p.h"

#include "hbinstantfeedback.h"
#include "hbcontinuousfeedback.h"
#include "hbtacticonfeedback.h"
#include "hbhitareafeedback.h"
#include "hbfeedbackplayer_stub_p.h"

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
    feedbackSettings = new HbFeedbackSettings();
    connect(feedbackSettings, SIGNAL(feedbackDisabled()),
            this, SLOT(feedbackDisabled()));
    connect(feedbackSettings, SIGNAL(feedbackTypeDisabled(HbFeedback::Type)),
            this, SLOT(feedbackTypeDisabled(HbFeedback::Type)));

    basePlayer = new HbFeedbackBasePlayer();
}

void HbFeedbackPlayerPrivate::feedbackDisabled()
{
    if (basePlayer) {
        basePlayer->cancelContinuousFeedbacks();
        basePlayer->removeHitAreas();
    }
}

void HbFeedbackPlayerPrivate::feedbackTypeDisabled(HbFeedback::Type type)
{
    if (basePlayer) {
        switch (type) {
            case HbFeedback::TypeContinuous:
                basePlayer->cancelContinuousFeedbacks();
                break;

            case HbFeedback::TypeHitArea:
                basePlayer->removeHitAreas();
                break;
            case HbFeedback::TypeInstant:
            case HbFeedback::TypeTacticon:
            default:
                break;
        }
    }
}

/*!
    @beta
    @hbfeedback
    \class HbFeedbackPlayer

    \brief Feedback player is used to initiate various haptic and sound feedback effects for the device.

    Current player supports four kinds of effects: instant feedback, continuous feedback, tacticon feedback
    and hit area feedback effects. Separate HbFeedbackSettings interface is reserved for applications wanting
    to limit or disable feedback effects emitted by the interface. See \ref feedback "Feedback Player" for
    more information on the design of the player.

    \sa HbInstantFeedback, HbContinuousFeedback, HbTacticonFeedback, HbHitAreaFeedback, HbFeedbackSettings.
*/


Q_GLOBAL_STATIC(HbFeedbackPlayer, feedbackPlayerGlobal);

/*!
    Constructor.
*/
HbFeedbackPlayer::HbFeedbackPlayer() : d(new HbFeedbackPlayerPrivate(this))
{
    d->init();
}

/*!
    Destructor.
*/
HbFeedbackPlayer::~HbFeedbackPlayer()
{
    delete d;
}

/*!
    Returns the handle to the global instance.
*/
HbFeedbackPlayer* HbFeedbackPlayer::instance()
{
    return feedbackPlayerGlobal();
}

/*!
    Returns a reference to the feedback settings interface.
*/
HbFeedbackSettings& HbFeedbackPlayer::settings()
{
    return *d->feedbackSettings;
}

/*!
    Triggers instant feedback effects.

    \param feedback instant feedback object
    \sa HbInstantFeedback
*/
void HbFeedbackPlayer::playInstantFeedback(const HbInstantFeedback& feedback)
{
    if (feedback.isValid() && d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeInstant)) {
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
    Triggers tacticon feedback effects.

    \param feedback tacticon feedback object
    \sa HbTacticonFeedback

    \deprecated HbFeedbackPlayer::playTacticonFeedback(const HbTacticonFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
void HbFeedbackPlayer::playTacticonFeedback(const HbTacticonFeedback& feedback)
{
    if (feedback.isValid() && d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeTacticon)) {
        if (d->basePlayer)  {
            d->basePlayer->playTacticonFeedback(feedback);
#ifdef FEEDBACK_TEST_EVENT
            HbFeedbackTestEvent te(feedback);
            qApp->sendEvent(this, &te);
#endif
        }
    }
}

/*!
    Starts a continuous feedback effect.

    \param feedback continuous feedback object
    \return identifier The identifier for the started effect.

    \sa HbContinuousFeedback
*/
int HbFeedbackPlayer::startContinuousFeedback(const HbContinuousFeedback& feedback)
{
    int identifier(-1);
    if (feedback.isValid() && d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeContinuous)) {
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
    Updates an ongoing continuous feedback effect.

    \param identifier The identifier for the ongoing effect.
    \param feedback continuous feedback object

    \sa HbContinuousFeedback
*/
void HbFeedbackPlayer::updateContinuousFeedback(int identifier, const HbContinuousFeedback& feedback)
{
    if (feedback.isValid() && d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeContinuous)) {
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
    Cancels an ongoing continuous feedback effect.

    \param identifier The identifier for the ongoing effect.
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
    Cancels all ongoing continuous feedback effects.
*/
void HbFeedbackPlayer::cancelContinuousFeedbacks()
{
    if (d->basePlayer)  {
        d->basePlayer->cancelContinuousFeedbacks();
    }
}

/*!
    Checks if the given continuous feedback effect is currently running.

    \param identifier The identifier for the ongoing effect.

    \return true, if the effect is ongoing.
*/
bool HbFeedbackPlayer::continuousFeedbackOngoing(int identifier)
{
    bool feedbackOngoing(false);
    if (d->basePlayer) {
        feedbackOngoing = d->basePlayer->continuousFeedbackOngoing(identifier);
    }
    return feedbackOngoing;
}

/*!
    Inserts a hit area to the specified window.

    \param feedback hit area feedback object
    \return The identifier for the inserted hit area.
    \sa HbHitAreaFeedback

    \deprecated HbFeedbackPlayer::insertHitArea(const HbHitAreaFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
int HbFeedbackPlayer::insertHitArea(const HbHitAreaFeedback& feedback)
{
    int identifier(-1);

    if (feedback.isValid() && d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeHitArea)) {
        if (d->basePlayer) {
            identifier = d->basePlayer->insertHitArea(feedback);
#ifdef FEEDBACK_TEST_EVENT
            HbFeedbackTestEvent te(feedback, HbFeedbackTestEvent::Start, identifier);
            qApp->sendEvent(this, &te);
#endif
        }
    } else if (!feedback.isLocated()) {
        qWarning("HbFeedbackPlayer::insertHitArea: Hit area missing required parameters parent window and/or rectangle.");
    }

    return identifier;
}

/*!
    Update the specified hit area.

    \param identifier Identifier for the hit area.
    \param feedback hit area feedback object
    \sa HbHitAreaFeedback

    \deprecated HbFeedbackPlayer::updateHitArea(int, const HbHitAreaFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
void HbFeedbackPlayer::updateHitArea(int identifier, const HbHitAreaFeedback& feedback)
{
    if (feedback.isValid() && d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeHitArea)) {
        if (d->basePlayer) {
            d->basePlayer->updateHitArea(identifier, feedback);
#ifdef FEEDBACK_TEST_EVENT
            HbFeedbackTestEvent te(feedback, HbFeedbackTestEvent::Update, identifier);
            qApp->sendEvent(this, &te);
#endif
        }
    } else if (!feedback.isLocated()) {
        qWarning("HbFeedbackPlayer::updateHitArea: Hit area missing required parameters parent window and/or rectangle.");
    }
}

/*!
    Remove the specified hit area.
    \param identifier The identifer for the hit area to be removed.
    \sa HbHitAreaFeedback

    \deprecated HbFeedbackPlayer::removeHitArea(int)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
void HbFeedbackPlayer::removeHitArea(int identifier)
{
    if (d->basePlayer) {
        d->basePlayer->removeHitArea(identifier);
#ifdef FEEDBACK_TEST_EVENT
        HbHitAreaFeedback feedback;
        HbFeedbackTestEvent te(feedback, HbFeedbackTestEvent::Stop, identifier);
        qApp->sendEvent(this, &te);
#endif
    }
}

/*!
    Remove all registered hit areas.

    \deprecated HbFeedbackPlayer::removeHitAreas()
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
void HbFeedbackPlayer::removeHitAreas()
{
    if (d->basePlayer) {
        d->basePlayer->removeHitAreas();
    }
}

/*!
    Check if the specified hit area still exists.

    \param identifier The identifier for the hit area.
    \return True, if the hit area exists.

    \deprecated HbFeedbackPlayer::hitAreaExists(int)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
bool HbFeedbackPlayer::hitAreaExists(int identifier)
{
    bool hitAreaExists = false;

    if (d->feedbackSettings->isFeedbackAllowed(HbFeedback::TypeHitArea)) {
        if (d->basePlayer) {
            hitAreaExists = d->basePlayer->hitAreaExists(identifier);
        }
    }

    return hitAreaExists;
}

