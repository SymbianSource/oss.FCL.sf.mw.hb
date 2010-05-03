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

#include <QGraphicsGridLayout>

#include <hbaction.h>
#include <hbinputmethod.h>
#include <hbinpututils.h>
#include <hbinputvkbhost.h>
#include <hbinputsettingproxy.h>
#include <hbinputcommondialogs.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbdataform.h>

#include "hbinputvkbwidget_p.h"
#include "hbinputhwtoolcluster.h"
#include "hbinputtouchkeypadbutton.h"
#include "hbinputmodeindicator.h"

const QString HbCustomButtonObjName = "Mini VKB custom button ";

const int HbMiniVirtualKeypadNumberOfColumn = 5;
const qreal HbMiniVirtualKeypadReductionFactor = 4.5;

/*!
@proto
@hbinput
\class HbHwToolCluster
\brief Implementation of hardware tool cluster.

Implementation of hardware tool cluster.

\sa HbHwToolCluster
\sa HbTouchKeypadButton
*/

/// @cond

class HbHwToolClusterPrivate : HbInputVkbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbHwToolCluster)

public:
    HbHwToolClusterPrivate();
    ~HbHwToolClusterPrivate();
public:
    HbTouchKeypadButton *mLanguageButton;
    HbTouchKeypadButton *mInputMethodButton;
    HbTouchKeypadButton *mPredictionIndicatorButton;
};

HbHwToolClusterPrivate::HbHwToolClusterPrivate() : mLanguageButton(0),
                mInputMethodButton(0),
                mPredictionIndicatorButton(0)
{
}

HbHwToolClusterPrivate::~HbHwToolClusterPrivate()
{
}

/// @endcond

/*!
Constructs the object.
*/
HbHwToolCluster::HbHwToolCluster(HbInputMethod* owner,
                                       QGraphicsItem* parent)
                                       : HbInputVkbWidget(*new HbHwToolClusterPrivate, parent)
{
    if (0 == owner) {
        return;
    }
    Q_D(HbHwToolCluster);
    d->q_ptr = this;
    d->mOwner = owner;
}

/*!
Creates the layout of the mini touch keypad for hardware qwerty.
*/
void HbHwToolCluster::createLayout()
{
    setupToolCluster();

    Q_D(HbHwToolCluster);
    // The layout is already created. So just return.
    if ( d->mButtonLayout ) {
        return;
    }

    d->mButtonLayout = new QGraphicsGridLayout();
    setContentsMargins(0.0, 0.0, 0.0, 0.0);
    d->mButtonLayout->setContentsMargins(0.0, 0.0, 0.0, 0.0);
    d->mButtonLayout->setHorizontalSpacing(HorizontalSpacing);
    d->mButtonLayout->setVerticalSpacing(VerticalSpacing);
    d->mButtonLayout->addItem(d->mSettingsButton, 0, 0);   // Settings key
    d->mButtonLayout->addItem(d->mLanguageButton, 0, 1);   // language selection
    d->mButtonLayout->addItem(d->mPredictionIndicatorButton, 0, 2);   // prediction indicator
    d->mButtonLayout->addItem(d->mInputMethodButton, 0, 3);   //input method selection key
    d->mButtonLayout->addItem(d->mApplicationButton, 0, 4);   // Application specific key

    d->mSettingsButton->setObjectName( HbCustomButtonObjName + QString::number(1));
    d->mApplicationButton->setObjectName( HbCustomButtonObjName + QString::number(2));
    d->mLanguageButton->setObjectName( HbCustomButtonObjName + QString::number(3));
    d->mInputMethodButton->setObjectName( HbCustomButtonObjName + QString::number(4));
    d->mPredictionIndicatorButton->setObjectName( HbCustomButtonObjName + QString::number(5));

    connect(d->mInputMethodButton, SIGNAL(clicked()), this, SLOT(showMethodDialog()));
    connect(d->mLanguageButton, SIGNAL(clicked()), this, SLOT(showLanguageDialog()));
    connect(d->mPredictionIndicatorButton, SIGNAL(clicked()), HbInputSettingProxy::instance(), SLOT(togglePrediction()));
}


/*!
Creates the tools cluster for mini VKB layout.
*/
void HbHwToolCluster::setupToolCluster()
{
    Q_D(HbHwToolCluster);
    if(!d->mOwner || !d->mOwner->focusObject()) {
        return;
    }

    // Create buttons if they do not exist
    if (!d->mSettingsButton) {
        d->mSettingsButton = new HbTouchKeypadButton(this, QString(""));
        d->mSettingsButton->setIcon(HbIcon(settingsIcon));
        d->mSettingsButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        d->mSettingsButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);

        connect(d->mSettingsButton, SIGNAL(clicked()), this, SLOT(showSettingList()));
    }
    if(!d->mLanguageButton) {
        d->mLanguageButton = new HbTouchKeypadButton(this, QString(""));
        d->mLanguageButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        d->mLanguageButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    }

    if(!d->mInputMethodButton) {
        d->mInputMethodButton = new HbTouchKeypadButton(this, QString(""));
        d->mInputMethodButton->setIcon(HbIcon(inputMethodIcon));
        d->mInputMethodButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        d->mInputMethodButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
    }

    if(!d->mPredictionIndicatorButton) {
        d->mPredictionIndicatorButton = new HbTouchKeypadButton(this, QString(""));
        d->mPredictionIndicatorButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
        d->mPredictionIndicatorButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
        d->mInputModeIndicator = new HbInputModeIndicator(*d->mPredictionIndicatorButton, this);
    } else {
        d->mInputModeIndicator->updateIndicator();
    }

    if(d->mLanguageButton) {
        // update language button text
        QString langName = HbInputSettingProxy::instance()->globalInputLanguage().localisedName();
        langName.truncate(3);
        d->mLanguageButton->setText(langName);
    }

    // update prediction button status
    if (HbInputSettingProxy::instance()->predictiveInputStatusForActiveKeyboard()) {
        d->mPredictionIndicatorButton->setIcon(HbIcon(predictionOffIcon));
    } else {
        d->mPredictionIndicatorButton->setIcon(HbIcon(predictionOnIcon));
    }

    // If there's a application specific button defined, create new button with the properties
    // or update the existing one. Otherwise create an empty button or clean the properties of an existing one.
    if (!d->mOwner->focusObject()->editorInterface().actions().isEmpty()) {
        QList<HbAction*> actions = d->mOwner->focusObject()->editorInterface().actions();
        if (d->mApplicationButton) {
            d->mApplicationButton->setText(actions.first()->text());
            d->mApplicationButton->disconnect(SIGNAL(clicked()));
        } else {
            d->mApplicationButton = new HbTouchKeypadButton(this, actions.first()->text());
            d->mApplicationButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
            d->mApplicationButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
        }
        connect(d->mApplicationButton, SIGNAL(clicked()), actions.first(), SLOT(trigger()));
        d->mApplicationButton->setIcon(actions.first()->icon());
        d->mApplicationButton->setToolTip(actions.first()->toolTip());
    } else {
        if (d->mApplicationButton) {
            d->mApplicationButton->disconnect(SIGNAL(clicked()));
            d->mApplicationButton->setText(QString());
            d->mApplicationButton->setIcon(HbIcon());
            d->mApplicationButton->setToolTip(QString());
        } else {
            d->mApplicationButton = new HbTouchKeypadButton(this, QString());
            d->mApplicationButton->setButtonType(HbTouchKeypadButton::HbTouchButtonFunction);
            d->mApplicationButton->setBackgroundAttributes(HbTouchKeypadButton::HbTouchButtonReleased);
        }
    }
}

/*!
Returns keyboard type.
*/
HbKeyboardType HbHwToolCluster::keyboardType() const
{
    return HbKeyboardQwerty;
}

/*!
Destructs the object.
*/
HbHwToolCluster::~HbHwToolCluster()
{
}

/*!
This is called right before the keypad is about to open. The layout of the
keypad happens here.
*/
void HbHwToolCluster::aboutToOpen(HbVkbHost *host)
{
    Q_D(HbHwToolCluster);
    HbInputVkbWidget::aboutToOpen(host);

    QSizeF size = preferredKeyboardSize();
    qreal height = size.height() - HbCloseHandleHeight;

    qreal width = size.width() / (qreal)HbMiniVirtualKeypadNumberOfColumn;
    for (int i=0; i < HbMiniVirtualKeypadNumberOfColumn ;i++) {
        d->mButtonLayout->setColumnFixedWidth(i, width);
    }
    //There is only one row
    d->mButtonLayout->setRowFixedHeight(0, height);
}

/*!
Returns preferred keyboard size. HbVkbHost uses this information when it opens the keyboard.
*/
QSizeF HbHwToolCluster::preferredKeyboardSize()
{
    Q_D(HbHwToolCluster);
    QSizeF ret = QSizeF(0.0, 0.0);

    if (d->mCurrentHost) {
        QSizeF ret;
        //Get the available keypad area from the VKB host
        ret = d->mCurrentHost->keyboardArea();
        //Since this is a mini VKB, it has a lesser size than the available
        //area for the keypad.
        ret.setHeight(ret.height()/HbMiniVirtualKeypadReductionFactor);
        return ret;
    }

    return ret;
}

/*!
\deprecated HbHwToolCluster::showSettingsDialog()
    is deprecated. Use showSettingsView instead.

Shows the settings dialog
*/
void HbHwToolCluster::showSettingsDialog()
{
}

/*!
Shows the input method selection dialog.
*/
void HbHwToolCluster::showMethodDialog()
{
    Q_D(HbHwToolCluster);

    HbInputMethodDescriptor method
        = HbInputCommonDialogs::showCustomInputMethodSelectionDialog(HbInputSettingProxy::instance()->globalInputLanguage());
    if (!method.isEmpty() && d->mOwner) {
        d->mOwner->activateInputMethod(method);
    }
}

/*!
Shows the language selection dialog.
*/
void HbHwToolCluster::showLanguageDialog()
{
    HbInputLanguage language =
        HbInputCommonDialogs::showLanguageSelectionDialog(HbInputSettingProxy::instance()->globalInputLanguage().language());
    HbInputSettingProxy::instance()->setGlobalInputLanguage(language);
}

// End of file

