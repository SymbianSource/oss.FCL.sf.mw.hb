/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#include <hbeffect.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbdevicedialog.h>
#include <hbaction.h>

#include "hbindicatorbutton_p.h"
#include "hbindicatorbutton_p_p.h"
#include "hbstyleoptionindicatorbutton_p.h"

#if defined(Q_OS_SYMBIAN)
#include "hbindicatorsym_p.h"
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

static const char noteIndicatorType[] = {"com.nokia.hb.indicatormenu/1.0"};

HbIndicatorButtonPrivate::HbIndicatorButtonPrivate() :
    handleIcon(0), defaultAction(0), newEventAction(0), deviceDialog(0)
{

}

HbIndicatorButtonPrivate::~HbIndicatorButtonPrivate()
{
    delete deviceDialog;
}

void HbIndicatorButtonPrivate::init()
{
    setBackgroundVisible(false);
}

void HbIndicatorButtonPrivate::showIndicatorMenu()
{
    if (mIndicators.count() > 0) {
        QVariantMap parametersMap;
        QString noteType(noteIndicatorType);

        parametersMap.clear();
        deviceDialog->show(noteType, parametersMap);
    }
}

void HbIndicatorButtonPrivate::addIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        if (clientInfo.at(i).hasMenuData) {
            mIndicators.prepend(clientInfo.at(i));
        }
    }

    updateIcon();
}

void HbIndicatorButtonPrivate::removeIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        int index = findIndicator(clientInfo.at(i));
        if (index >= 0) {
            mIndicators.removeAt(index);
        }
    }

    updateIcon();
}

int HbIndicatorButtonPrivate::findIndicator(const IndicatorClientInfo &indicator) const
{
    int index = -1;
    for (int i = 0; i < mIndicators.size(); ++i) {
        if (mIndicators.at(i).type == indicator.type) {
            index = i;
            break;
        }
    }
    return index;
}

void HbIndicatorButtonPrivate::updateIcon()
{
    Q_Q(HbIndicatorButton);
    bool newEvent(false);
    for (int i = 0; i < mIndicators.size(); ++i) {
        if (mIndicators.at(i).category == HbIndicatorInterface::NotificationCategory
            || mIndicators.at(i).category == HbIndicatorInterface::ProgressCategory) {
            newEvent = true;
            break;
        }
    }
    if (newEvent) {
        q->setAction(newEventAction);
    } else {
        q->setAction(defaultAction);
    }
}

HbIndicatorButton::HbIndicatorButton(QGraphicsItem *parent) 
    : HbToolButton(*new HbIndicatorButtonPrivate, parent)
{
    Q_D(HbIndicatorButton);
    d->init(); 

    // add default actions
    d->defaultAction = new HbAction(HbIcon("qtg_mono_options_menu"), "IndicatorMenu", this);
    d->newEventAction = new HbAction(HbIcon("qtg_mono_new_event"), "IndicatorMenu", this);
    setAction(d->defaultAction);

    createPrimitives();
}

HbIndicatorButton::~HbIndicatorButton()
{

}

void HbIndicatorButton::delayedConstruction()
{
    Q_D(HbIndicatorButton);

    connect(this, SIGNAL(pressed()), this, SLOT(handlePress()));
    connect(this, SIGNAL(released()), this, SLOT(handleRelease()));

    d->deviceDialog = new HbDeviceDialog(HbDeviceDialog::ImmediateResourceReservationFlag);
}

void HbIndicatorButton::showHandleIndication(bool show)
{
    Q_D(HbIndicatorButton);
    d->handleIcon->setVisible(show);
}

bool HbIndicatorButton::handleVisible() const
{
    bool handleVisible = false;
    if (mainWindow() && mainWindow()->currentView()) {
        handleVisible = mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarMinimizable;
    }
    return handleVisible;
}

void HbIndicatorButton::createPrimitives()
{
    Q_D(HbIndicatorButton);
    d->handleIcon = style()->createPrimitive(HbStyle::P_IndicatorButton_handleindication, this);
    d->handleIcon->setVisible(false);
    setBackgroundItem(HbStyle::P_IndicatorButton_background); // calls updatePrimitives()
}

void HbIndicatorButton::updatePrimitives()
{
    Q_D(HbIndicatorButton);
    HbStyleOptionIndicatorButton option;
    initStyleOption(&option);
    style()->updatePrimitive(backgroundItem(), HbStyle::P_IndicatorButton_background, &option);
    style()->updatePrimitive(d->handleIcon, HbStyle::P_IndicatorButton_handleindication, &option);
    HbToolButton::updatePrimitives();
}

void HbIndicatorButton::activate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->addIndicators(clientInfo);
}

void HbIndicatorButton::deactivate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->removeIndicators(clientInfo);
}

void HbIndicatorButton::initStyleOption(HbStyleOptionIndicatorButton *option) const
{
    if (isDown()) {
        option->mode = QIcon::Active;
    } else {
        option->mode = QIcon::Normal;
    }
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            option->transparent = true;
        }
    }
}

void HbIndicatorButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        updatePrimitives();
    }
    HbToolButton::changeEvent(event);
}

void HbIndicatorButton::handlePress()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "pressed");
#endif
    updatePrimitives();
}

void HbIndicatorButton::handleRelease()
{
    Q_D(HbIndicatorButton);
    if( isUnderMouse() ) {
        d->showIndicatorMenu();
    }
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
    updatePrimitives();
}


