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

#include "hbfeedbackplayer_stub_p.h"
#include <QList>
#include "hbinstantfeedback.h"
#include "hbcontinuousfeedback.h"

/*
    Test player is used to display feedback effects in desktop environment.
*/

class HbFeedbackBasePlayerPrivate
{

public:
    HbFeedbackBasePlayerPrivate();
    ~HbFeedbackBasePlayerPrivate();

    int getNewContinuousIdentifier();

public:
    QList<int> continuousIdentifiers;
    int slidingValueContinuous;
};

HbFeedbackBasePlayerPrivate::HbFeedbackBasePlayerPrivate() : slidingValueContinuous(0)
{
}

HbFeedbackBasePlayerPrivate::~HbFeedbackBasePlayerPrivate()
{
    continuousIdentifiers.clear();
}

int HbFeedbackBasePlayerPrivate::getNewContinuousIdentifier()
{
    slidingValueContinuous++;
    return slidingValueContinuous;
}

HbFeedbackBasePlayer::HbFeedbackBasePlayer()
        : d(new HbFeedbackBasePlayerPrivate())
{
}

HbFeedbackBasePlayer::~HbFeedbackBasePlayer()
{
    delete d;
}

void HbFeedbackBasePlayer::playInstantFeedback(const HbInstantFeedback& feedback)
{
    Q_UNUSED(feedback)
}

int HbFeedbackBasePlayer::startContinuousFeedback(const HbContinuousFeedback& feedback)
{
    Q_UNUSED(feedback)
    int identifier = d->getNewContinuousIdentifier();
    d->continuousIdentifiers.append(identifier);
    return identifier;
}
 
void HbFeedbackBasePlayer::updateContinuousFeedback(int identifier, const HbContinuousFeedback& feedback)
{
  Q_UNUSED(identifier)
  Q_UNUSED(feedback)
}

void HbFeedbackBasePlayer::cancelContinuousFeedback(int identifier)
{
    if (d->continuousIdentifiers.contains(identifier)) {
        d->continuousIdentifiers.removeAll(identifier);
    }
}

bool HbFeedbackBasePlayer::continuousFeedbackOngoing(int identifier)
{
    return d->continuousIdentifiers.contains(identifier);
}

void HbFeedbackBasePlayer::cancelContinuousFeedbacks()
{
    d->continuousIdentifiers.clear();
}
