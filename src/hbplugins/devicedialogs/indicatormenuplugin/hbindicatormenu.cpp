/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#include <QVariant>
#include <QTimer>
#include <hbmainwindow.h>

#include "hbindicatormenucontent_p.h"
#include "hbindicatormenu_p.h"
#include "hbindicatormenupluginerrors_p.h"
#include "hbdialog_p.h"

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

static const int HB_UNIVERSAL_INDICATOR_POPUP_TIMEOUT = 6000; // ms

// Constructor
HbIndicatorMenu::HbIndicatorMenu(const QVariantMap &parameters)
{
    mLastError = NoError;
    mShowEventReceived = false;
    resetProperties();
    constructMenu(parameters);
    setBackgroundFaded(false);

#ifdef HB_EFFECTS
    HbEffectInternal::add(this, "universalindicatormenu_appear", "appear");
    HbEffectInternal::add(this, "universalindicatormenu_disappear", "disappear");
#endif
}

// Return device dialog group
HbDeviceDialogPlugin::DeviceDialogGroup HbIndicatorMenu::deviceDialogGroup() const
{
    return HbDeviceDialogPlugin::IndicatorGroup;
}

// Return device dialog priority
int HbIndicatorMenu::deviceDialogPriority() const
{
    return 0;
}

// Return device dialog flags
HbDeviceDialogPlugin::DeviceDialogFlags HbIndicatorMenu::deviceDialogFlags() const
{
    return HbDeviceDialogPlugin::NoDeviceDialogFlags;
}

// Set parameters
bool HbIndicatorMenu::setDeviceDialogParameters(const QVariantMap &parameters)
{
    mLastError = NoError;
    bool ret(false);
    if (checkProperties(parameters)) {
        setProperties(parameters);
        ret = true;
    } else {
        mLastError = ParameterError;
        ret = false;
    }
    return ret;
}

// Get error
int HbIndicatorMenu::deviceDialogError() const
{
    return mLastError;
}

// Close device dialog
void HbIndicatorMenu::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    close();
    // If show event has been received, close is signalled from hide event. If not,
    // hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
}

// Return display widget
HbPopup *HbIndicatorMenu::deviceDialogWidget() const
{
    return const_cast<HbIndicatorMenu*>(this);
}

// Construct note
bool HbIndicatorMenu::constructMenu(const QVariantMap &parameters)
{
    bool ret(false);
    // Check that parameters are valid
    if (!checkProperties(parameters)) {
        mLastError = ParameterError;
        ret = false;
    } else {
        setProperties(parameters);
        ret = true;
    }

    doMenuLayout();
    setTimeout(HB_UNIVERSAL_INDICATOR_POPUP_TIMEOUT);

    HbIndicatorMenuContent *content = new HbIndicatorMenuContent();
    HbStyle::setItemName(content, "content");
    setContentWidget(content);
    connect(content, SIGNAL(aboutToClose()), this, SLOT(close()));
    connect(content, SIGNAL(userActivity()), this, SLOT(userActivity()));
    connect(content, SIGNAL(userActivityStarted()), this, SLOT(userActivityStarted()));
    connect(content, SIGNAL(userActivityEnded()), this, SLOT(userActivityEnded()));       

    return ret;
}

// Check that device dialog parameters are valid
bool HbIndicatorMenu::checkProperties(const QVariantMap &parameters)
{
    // Currently no parameters are used.
    Q_UNUSED(parameters);
    return true;
}

// Set properties
void HbIndicatorMenu::setProperties(const QVariantMap &parameters)
{
    // Currently no parameters are used.
    Q_UNUSED(parameters);
    return;
}

// Reset properties to default values
void HbIndicatorMenu::resetProperties()
{
    setDismissPolicy(HbDialog::TapAnywhere);
    setModal(true);
    return;
}

// Widget is about to hide. Closing effect has ended,
void HbIndicatorMenu::hideEvent(QHideEvent *event)
{
    HbDialog::hideEvent(event);
    emit deviceDialogClosed();
}

// Widget is about to show
void HbIndicatorMenu::showEvent(QShowEvent *event)
{
    HbMainWindow* mainWnd = mainWindow();
    Q_ASSERT(mainWnd);
    if (mainWnd && !mShowEventReceived) {        
        // To have a consistant user experience the indicator menu's behaviour
        // got aligned to the options menu's behaviour.
        // The menu is now closed before a view/orientation change happens.
        connect(mainWnd, SIGNAL(aboutToChangeOrientation()),
                this, SLOT(close()));
        connect(mainWnd, SIGNAL(aboutToChangeView(HbView *,HbView *)),
                this, SLOT(close()));
    }

    HbIndicatorMenuContent *menuContent =
        qobject_cast<HbIndicatorMenuContent*>(contentWidget());
    menuContent->handleAboutToShow();

    HbDialog::showEvent(event);
    mShowEventReceived = true;
}

void HbIndicatorMenu::doMenuLayout()
{
    qreal chromeHeight = 0;
    qreal xPos = 0;
    // Position menu below titlebar
    if (style()->parameter("hb-param-widget-chrome-height", chromeHeight)
        && style()->parameter("hb-param-margin-gene-middle-horizontal", xPos)){
        setPreferredPos(QPointF(xPos, chromeHeight));
    }
    if (mShowEventReceived) {
        HbIndicatorMenuContent *menuContent =
            qobject_cast<HbIndicatorMenuContent*>(contentWidget());
        menuContent->updatePrimitives();
    }
}

void HbIndicatorMenu::userActivity()
{
    //Q_D(HbDialog);
    //d->timeoutTimeLine->setCurrentTime(0);
}

void HbIndicatorMenu::userActivityStarted()
{
    Q_D(HbDialog);
    d->stopTimeout();
}

void HbIndicatorMenu::userActivityEnded()
{
    Q_D(HbDialog);
    d->startTimeout();
}

void HbIndicatorMenu::indicatorsActivated(
        QList<HbIndicatorInterface*> activatedIndicators)
{
    //forward to content widget.
    HbIndicatorMenuContent *menuContent =
            qobject_cast<HbIndicatorMenuContent*>(contentWidget());
    menuContent->indicatorsActivated(activatedIndicators);
}

void HbIndicatorMenu::indicatorActivated(
        HbIndicatorInterface *activatedIndicator)
{
    //forward to content widget.
    HbIndicatorMenuContent *menuContent =
            qobject_cast<HbIndicatorMenuContent*>(contentWidget());
    menuContent->indicatorActivated(activatedIndicator);
}

void HbIndicatorMenu::indicatorRemoved(
        HbIndicatorInterface *indicatorRemoved)
{
    //forward to content widget.
    HbIndicatorMenuContent *menuContent =
            qobject_cast<HbIndicatorMenuContent*>(contentWidget());
    menuContent->indicatorRemoved(indicatorRemoved);
}
