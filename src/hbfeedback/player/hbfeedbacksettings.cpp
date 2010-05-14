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

    \brief Feedback setting interface for controlling the feedback playing.

    Application can choose to disable feedback effects for the application, for example a phone 
    application may want to disable the haptic and audio feedback effects during a phone call.
*/


class HbFeedbackSettingsPrivate
{

public:
    static HbFeedbackSettingsPrivate* instance();
    HbFeedbackSettingsPrivate();
    ~HbFeedbackSettingsPrivate();
    void init();

public:
    bool feedbackEnabled;
    HbFeedback::Types enabledTypes;
};

Q_GLOBAL_STATIC(HbFeedbackSettingsPrivate, feedbackSettingsPrivateGlobal);

HbFeedbackSettingsPrivate* HbFeedbackSettingsPrivate::instance()
{
    return feedbackSettingsPrivateGlobal();
}

HbFeedbackSettingsPrivate::HbFeedbackSettingsPrivate()
{
    init();
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
    enabledTypes = HbFeedback::TypeInstant | HbFeedback::TypeContinuous;
}



/*!
    Constructor.
*/
HbFeedbackSettings::HbFeedbackSettings(QObject* parent) : QObject(parent)
{
    d = HbFeedbackSettingsPrivate::instance();
}

/*!
    Destructor.
*/
HbFeedbackSettings::~HbFeedbackSettings()
{
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

