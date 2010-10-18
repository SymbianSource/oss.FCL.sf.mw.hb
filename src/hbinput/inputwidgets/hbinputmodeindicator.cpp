/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include <hbinputmodeindicator.h>

#include <QPainter>
#include <QPointer>
#include <hbicon.h>
#include <hbinputmethod.h>
#include <hbinputeditorinterface.h>
#include <hbinputfocusobject.h>
#include <hbinputsettingproxy.h>

/// @cond

class HbInputModeIndicatorPrivate
{
public:
    HbInputModeIndicatorPrivate(HbInputButton *button);
    ~HbInputModeIndicatorPrivate();

    void updatePrediction();

public:
    HbInputButton *mButtonItem;
    QPointer<HbInputFocusObject> mFocusObject;
};

HbInputModeIndicatorPrivate::HbInputModeIndicatorPrivate(HbInputButton *button)
    : mButtonItem(button), mFocusObject(0)
{
    if (HbInputMethod::activeInputMethod()) {
        mFocusObject = HbInputMethod::activeInputMethod()->focusObject();
    }
}

HbInputModeIndicatorPrivate::~HbInputModeIndicatorPrivate()
{
}

void HbInputModeIndicatorPrivate::updatePrediction()
{
    const QString predictionOnIcon("qtg_mono_predictive_text_on");
    const QString predictionOffIcon("qtg_mono_predictive_text_off");

    QColor color;
    if (mButtonItem && !mButtonItem->icon(HbInputButton::ButtonIconIndexPrimary).isNull()) {
        color = mButtonItem->icon(HbInputButton::ButtonIconIndexPrimary).color();
    }
    HbIcon icon;
    if (HbInputSettingProxy::instance()->predictiveInputStatusForActiveKeyboard() &&
        mFocusObject && mFocusObject->editorInterface().isPredictionAllowed()) {
        icon = HbIcon(predictionOnIcon);
    } else {
        icon = HbIcon(predictionOffIcon);
    }
    icon.setColor(color);
    if (mButtonItem) {
        mButtonItem->setIcon(icon, HbInputButton::ButtonIconIndexPrimary);
    }
}

/// @endcond

/*!
@stable
@hbinput
\class HbInputModeIndicator
\brief Predictive mode indicator

Implements automatic mechanism which updates the prediction icon of keypad button based on current predictive status.

\sa HbEditorInterface
*/

HbInputModeIndicator::HbInputModeIndicator(HbInputButton *button, QGraphicsWidget *parent)
    : QObject(parent)
{
    mPrivate = new HbInputModeIndicatorPrivate(button);
    if (mPrivate->mFocusObject) {
        connect(&mPrivate->mFocusObject->editorInterface(), SIGNAL(modified()), this, SLOT(updateIndicator()));
    }
    connect(HbInputSettingProxy::instance(), SIGNAL(predictiveInputStateChanged(HbKeyboardSettingFlags, bool)),
            this, SLOT(updatePredictionStatus(HbKeyboardSettingFlags, bool)));
    updateIndicator(); // check mode of current editor
}

/*!
Destroys the widget.
*/
HbInputModeIndicator::~HbInputModeIndicator()
{
    delete mPrivate;
}

/*!
Updates the indicator.
*/
void HbInputModeIndicator::updateIndicator()
{
    if (!mPrivate->mFocusObject) {
        if (HbInputMethod::activeInputMethod()) {
            mPrivate->mFocusObject = HbInputMethod::activeInputMethod()->focusObject();
            if (mPrivate->mFocusObject) {
                connect(&mPrivate->mFocusObject->editorInterface(), SIGNAL(modified()), this, SLOT(updateIndicator()));
            } else {
                return;
            }
        } else {
            return;
        }
    }

    mPrivate->updatePrediction();
}

/*!
Updates prediction status.
*/
void HbInputModeIndicator::updatePredictionStatus(HbKeyboardSettingFlags keyboardType, bool newStatus)
{
    Q_UNUSED(keyboardType);
    Q_UNUSED(newStatus);
    mPrivate->updatePrediction();
}

// End of file

