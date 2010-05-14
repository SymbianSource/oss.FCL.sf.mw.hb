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

#include "hbfeedbackplayer_symbian_p.h"

#include <tacticon.h>
#include <touchfeedback.h>

#include <coecntrl.h>

#include <QList>
#include <QMap>
#include <QMapIterator>
#include <QDebug>

#include "hbinstantfeedback.h"
#include "hbcontinuousfeedback.h"

/*
    Symbian-specific feedback class that wraps Symbian feedback APIs.
*/

class HbFeedbackBasePlayerPrivate
{

public:
    HbFeedbackBasePlayerPrivate();
    ~HbFeedbackBasePlayerPrivate();

public:
    void init();
    inline int getNewIdentifier();
    inline TRect convertToSymbian(const QRect& rect);
    inline CCoeControl* convertToSymbian(QWidget* window);
    inline TTouchLogicalFeedback convertToSymbian(HbFeedback::InstantEffect effect);
    inline TTacticonType convertTacticonToSymbian(HbFeedback::InstantEffect effect);
    inline TTouchContinuousFeedback convertToSymbian(HbFeedback::ContinuousEffect effect);
    inline TTouchFeedbackType convertToSymbian(HbFeedback::Modalities modalities);
			
public:
    MTouchFeedback *iFeedback;
    RTacticon iTacticon;
    TBool iTacticonReady;
    QMap <int, QWidget*> ongoingContinuousFeedbacks;
    int slidingValue;
};

HbFeedbackBasePlayerPrivate::HbFeedbackBasePlayerPrivate() : slidingValue(0)
{
}

HbFeedbackBasePlayerPrivate::~HbFeedbackBasePlayerPrivate() {
    
    MTouchFeedback::DestroyInstance();

    iTacticon.Close();
    iTacticonReady = EFalse;
}

void HbFeedbackBasePlayerPrivate::init() {

    iFeedback = MTouchFeedback::Instance();
    if (!iFeedback) {
        TRAPD(err, iFeedback = MTouchFeedback::CreateInstanceL());
        if (err != KErrNone) {
            iFeedback = 0;
        }
    }
    TInt err = iTacticon.Connect();
    if(err) {
        iTacticonReady = EFalse;
    }
    else {
        iTacticonReady = ETrue;
    }
}

int HbFeedbackBasePlayerPrivate::getNewIdentifier() {
    
    slidingValue++;
    return slidingValue;
}


TRect HbFeedbackBasePlayerPrivate::convertToSymbian(const QRect& rect) {

    return TRect(rect.left(), rect.top(), rect.right(), rect.bottom());
}

TTouchLogicalFeedback HbFeedbackBasePlayerPrivate::convertToSymbian(HbFeedback::InstantEffect effect) {

    TTouchLogicalFeedback instantFeedbackSymbian = ETouchFeedbackBasic;
    
    switch (effect) {
    
    case HbFeedback::None:
        instantFeedbackSymbian = ETouchFeedbackNone;
        break;
    case HbFeedback::Basic:
        instantFeedbackSymbian = ETouchFeedbackBasic;
        break;
    case HbFeedback::Sensitive:
        instantFeedbackSymbian = ETouchFeedbackSensitive;
        break;
    case HbFeedback::BasicButton:
        instantFeedbackSymbian = ETouchFeedbackBasicButton;
        break;
    case HbFeedback::SensitiveButton:
        instantFeedbackSymbian = ETouchFeedbackSensitiveButton;
        break;
    case HbFeedback::BasicItem:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::SensitiveItem:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::BounceEffect:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::PopupOpen:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::PopupClose:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::BasicSlider:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::SensitiveSlider:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::StopFlick:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::Flick:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::Editor:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::TextSelection:
        instantFeedbackSymbian = ETouchFeedbackTextSelection;
        break;
    case HbFeedback::BlankSelection:
        instantFeedbackSymbian = ETouchFeedbackBlankSelection;
        break;
    case HbFeedback::LineSelection:
        instantFeedbackSymbian = ETouchFeedbackLineSelection;
        break;
    case HbFeedback::EmptyLineSelection:
        instantFeedbackSymbian = ETouchFeedbackEmptyLineSelection;
        break;
    case HbFeedback::Checkbox:
        instantFeedbackSymbian = ETouchFeedbackCheckbox;
        break;
    case HbFeedback::MultipleCheckbox:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::SensitiveKeypad:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::BasicKeypad:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::MultitouchActivate:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::RotateStep:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::ItemDrop:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::ItemMoveOver:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::ItemPick:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::ItemScroll:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    case HbFeedback::PopUp:
        instantFeedbackSymbian = ETouchFeedbackPopUp;
        break;
    case HbFeedback::LongPress:
        instantFeedbackSymbian = ETouchFeedbackBasic; // Effects changing in 10.1 are mapped to basic.
        break;
    default:
        break;
    }
    return instantFeedbackSymbian;
}

TTacticonType HbFeedbackBasePlayerPrivate::convertTacticonToSymbian(HbFeedback::InstantEffect effect) {

    TTacticonType type = ENoTacticon;

    switch (effect) {

    case HbFeedback::PositiveTacticon:
        type = EPositiveTacticon;
        break;
    case HbFeedback::NeutralTacticon:
        type = ENeutralTacticon;
        break;
    case HbFeedback::NegativeTacticon:
        type = ENegativeTacticon;
        break;
    default:
        break;
    }
    return type;
}

TTouchContinuousFeedback HbFeedbackBasePlayerPrivate::convertToSymbian(HbFeedback::ContinuousEffect effect) {

    TTouchContinuousFeedback continuousFeedbackSymbian = ETouchContinuousSmooth;
    
    switch (effect) {
        
    case HbFeedback::ContinuousSmooth:
        continuousFeedbackSymbian = ETouchContinuousSmooth;
        break;
    case HbFeedback::ContinuousSlider:
        continuousFeedbackSymbian = ETouchContinuousSlider;
        break;
    case HbFeedback::ContinuousInput:
        continuousFeedbackSymbian = ETouchContinuousInput;
        break;
    // Effects coming in 10.1 are mapped to smooth temporarily.
    case HbFeedback::ContinuousPopup:
        continuousFeedbackSymbian = ETouchContinuousSmooth;
        break;
    case HbFeedback::ContinuousPinch:
        continuousFeedbackSymbian = ETouchContinuousSmooth;
        break;
    default:
        break;
    }
    return continuousFeedbackSymbian;
}


TTouchFeedbackType HbFeedbackBasePlayerPrivate::convertToSymbian(HbFeedback::Modalities modalities)
{
    int symbianFeedbackType = 0;

    if(modalities == HbFeedback::All) {
        // enable all modalities
        symbianFeedbackType |= ETouchFeedbackAudio;
        symbianFeedbackType |= ETouchFeedbackVibra;
    } else {
        // enable individual modalities
        if(modalities & HbFeedback::Audio)   symbianFeedbackType |= ETouchFeedbackAudio;
        if(modalities & HbFeedback::Tactile) symbianFeedbackType |= ETouchFeedbackVibra;
    }
    return TTouchFeedbackType(symbianFeedbackType);
}

CCoeControl* HbFeedbackBasePlayerPrivate::convertToSymbian(QWidget* window)
{
    CCoeControl* control = 0;
    
    if ( window && window->winId()) {
        control = reinterpret_cast<CCoeControl*>(window->winId());
    }
    return control;
}

HbFeedbackBasePlayer::HbFeedbackBasePlayer() : d(new HbFeedbackBasePlayerPrivate) {

    d->init();
}

HbFeedbackBasePlayer::~HbFeedbackBasePlayer() {

    cancelContinuousFeedbacks();
    delete d;
}



void HbFeedbackBasePlayer::cancelContinuousFeedbacks() {

    // stop ongoing continuous feedbacks
    QMapIterator<int, QWidget*> i(d->ongoingContinuousFeedbacks);

    while (i.hasNext()) {
        i.next();
        cancelContinuousFeedback(i.key());
    }

    d->ongoingContinuousFeedbacks.clear();
}

void HbFeedbackBasePlayer::playInstantFeedback(const HbInstantFeedback& feedback) {
    TPointerEvent pointerEvent;

    if (d->iFeedback) {
        // If the effect is a tacticon, use the tacticon playing mechanism of the feedback player
        if (feedback.instantEffect() == HbFeedback::PositiveTacticon ||
            feedback.instantEffect() == HbFeedback::NeutralTacticon  ||
            feedback.instantEffect() == HbFeedback::NegativeTacticon) {
            if (d->iTacticonReady) {
                d->iTacticon.PlayTacticon(d->convertTacticonToSymbian(feedback.instantEffect()));
            }
        } else {
            CCoeControl* control = d->convertToSymbian(feedback.window());
            if (control) {
                d->iFeedback->InstantFeedback(control, d->convertToSymbian(feedback.instantEffect()),
                                              d->convertToSymbian(feedback.modalities()),pointerEvent);
            } else {
                d->iFeedback->InstantFeedback(d->convertToSymbian(feedback.instantEffect()));
            }
        }
    }
}

int HbFeedbackBasePlayer::startContinuousFeedback(const HbContinuousFeedback& feedback) {

    int identifier = -1;
    CCoeControl* control = d->convertToSymbian(feedback.window());
    
    if (d->iFeedback && control) {
        identifier = d->getNewIdentifier();
        if (feedback.continuousEffect() != HbFeedback::ContinuousNone) {
            d->iFeedback->StartFeedback(control, d->convertToSymbian(feedback.continuousEffect()),
                                        0, feedback.intensity(),
                                        1000*feedback.timeout());
        }
        d->ongoingContinuousFeedbacks.insert(identifier, feedback.window());
    }
    return identifier;
}

void HbFeedbackBasePlayer::updateContinuousFeedback(int identifier, const HbContinuousFeedback& feedback) {

    CCoeControl* control = d->convertToSymbian(feedback.window());

    if (d->iFeedback && control) {
        if (d->ongoingContinuousFeedbacks.contains(identifier)) {
            CCoeControl* storedControl = d->convertToSymbian(d->ongoingContinuousFeedbacks[identifier]);
            if (storedControl == control) {
                if (feedback.continuousEffect() != HbFeedback::ContinuousNone) {
                    d->iFeedback->StartFeedback( storedControl,
                                                d->convertToSymbian(feedback.continuousEffect()),
                                                0, feedback.intensity(),
                                                1000*feedback.timeout());
                } else {
                    d->iFeedback->StopFeedback(storedControl);
                }
            }
        }
    }
}

void HbFeedbackBasePlayer::cancelContinuousFeedback(int identifier) {

    if (d->iFeedback) {
        if (d->ongoingContinuousFeedbacks.contains(identifier)) {
            d->iFeedback->StopFeedback(d->convertToSymbian(d->ongoingContinuousFeedbacks[identifier]));
            d->ongoingContinuousFeedbacks.remove(identifier);
        }
    }
}

bool HbFeedbackBasePlayer::continuousFeedbackOngoing(int identifier) {

    bool feedbackOngoing(false);
    if (d->ongoingContinuousFeedbacks.contains(identifier)) {
        feedbackOngoing = true;
    }
    return feedbackOngoing;
}


