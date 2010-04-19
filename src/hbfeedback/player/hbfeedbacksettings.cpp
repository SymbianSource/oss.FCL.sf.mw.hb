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

#include "hbfeedbacksettings.h"

#ifdef Q_OS_SYMBIAN
#include <touchfeedback.h>
#endif

/*!
    @beta
    @hbfeedback

    \class HbFeedbackSettings

    \brief %Feedback Settings API for Qt application development.

    Application can choose to disable feedback effects for the application, for example
    phone application may want to disable vibration and sound feedback effects during a phone call.

    Feedback effects can also be disabled based on the type of feedback. There are currently four
    supported types of feedback: instant fire&forget feedback, continuous feedback that needs to
    be started and ended separately, hit area feedback that uses pre-registered feedback areas and
    finally tacticon feedback.
*/

class HbFeedbackSettingsPrivate
{

public:
    HbFeedbackSettingsPrivate(HbFeedbackSettings* parent);
    ~HbFeedbackSettingsPrivate();
    void init();

public:
    bool feedbackEnabled;
    HbFeedback::Types enabledTypes;
    HbFeedbackSettings* parent;
};

HbFeedbackSettingsPrivate::HbFeedbackSettingsPrivate(HbFeedbackSettings* parent) : parent(parent)
{
}

HbFeedbackSettingsPrivate::~HbFeedbackSettingsPrivate()
{
}

void HbFeedbackSettingsPrivate::init()
{
    feedbackEnabled = true;
#ifdef Q_OS_SYMBIAN
    MTouchFeedback* touchFeedbackPlayer = MTouchFeedback::Instance();

    if (touchFeedbackPlayer) {
        feedbackEnabled = touchFeedbackPlayer->FeedbackEnabledForThisApp();
    }

#endif
    // all types are enabled by default
    enabledTypes = HbFeedback::TypeInstant
                   | HbFeedback::TypeContinuous
                   | HbFeedback::TypeHitArea
                   | HbFeedback::TypeTacticon;
}

/*!
    Constructor.
*/
HbFeedbackSettings::HbFeedbackSettings(QObject* parent) : QObject(parent),
        d(new HbFeedbackSettingsPrivate(this))
{
    d->init();
}

/*!
    Destructor.
*/
HbFeedbackSettings::~HbFeedbackSettings()
{
    delete d;
}

/*!
    True if the device supports feedback effects.
*/
bool HbFeedbackSettings::feedbackSupported()
{
    bool feedbackSupported = false;
#ifdef Q_OS_SYMBIAN
    MTouchFeedback* touchFeedbackPlayer = MTouchFeedback::Instance();

    if (touchFeedbackPlayer) {
        feedbackSupported = touchFeedbackPlayer->TouchFeedbackSupported();
    }

#else
    feedbackSupported = true;

#endif
    return feedbackSupported;
}

/*!
    Enables haptic and sound feedback effects for the application.
*/
void HbFeedbackSettings::enableFeedback()
{
    if (!d->feedbackEnabled) {
#ifdef Q_OS_SYMBIAN
        MTouchFeedback* touchFeedbackPlayer = MTouchFeedback::Instance();

        if (touchFeedbackPlayer) {
            touchFeedbackPlayer->SetFeedbackEnabledForThisApp(ETrue);
        }

        d->feedbackEnabled = touchFeedbackPlayer->FeedbackEnabledForThisApp();

#else
        d->feedbackEnabled = true;
#endif
        emit feedbackEnabled();
    }
}

/*!
    Disables feedback effects for the application.
*/
void HbFeedbackSettings::disableFeedback()
{
    if (d->feedbackEnabled) {
#ifdef Q_OS_SYMBIAN
        MTouchFeedback* touchFeedbackPlayer = MTouchFeedback::Instance();

        if (touchFeedbackPlayer) {
            touchFeedbackPlayer->SetFeedbackEnabledForThisApp(EFalse);
        }

        d->feedbackEnabled = touchFeedbackPlayer->FeedbackEnabledForThisApp();

#else
        d->feedbackEnabled = false;
#endif

        if (!d->feedbackEnabled) {
            emit feedbackDisabled();
        }
    }
}

/*!
    Returns true if haptic and sound feedback effects has been enabled in the application.
*/
bool HbFeedbackSettings::isFeedbackEnabled()
{
#ifdef Q_OS_SYMBIAN
    MTouchFeedback* touchFeedbackPlayer = MTouchFeedback::Instance();

    if (touchFeedbackPlayer) {
        d->feedbackEnabled = touchFeedbackPlayer->FeedbackEnabledForThisApp();
    }

#endif
    return d->feedbackEnabled;
}

/*!
    Enables given type of feedback effects in the application.
    All feedback types are enabled by default.
    Emits signal typeEnabled(HbFeedback::Type type) if
    previously disabled feedback type has been enabled.
*/
void HbFeedbackSettings::enableType(HbFeedback::Type type)
{
    if (!isTypeEnabled(type)) {
        d->enabledTypes |= type;
        emit feedbackTypeEnabled(type);
    }
}

/*!
    Disables given type of feedback effect mechanism in the application.

    Emits signal typeDisabled(HbFeedback::Type type) if
    previously enabled feedback type has been disabled.

    \param type type of feedback effect
*/
void HbFeedbackSettings::disableType(HbFeedback::Type type)
{
    if (isTypeEnabled(type)) {
        d->enabledTypes &= ~type;
        emit feedbackTypeDisabled(type);
    }
}

/*!
    Returns true if a particular type of feedback effect mechanism has been 
    enabled for the application.
*/
bool HbFeedbackSettings::isTypeEnabled(HbFeedback::Type type)
{
    return d->enabledTypes & type;
}

/*!
    True if feedback effects and the particular feedback type of feedback 
    effect mechanism is enabled, false if not.
*/
bool HbFeedbackSettings::isFeedbackAllowed(HbFeedback::Type type)
{
    return d->feedbackEnabled && isTypeEnabled(type);
}
