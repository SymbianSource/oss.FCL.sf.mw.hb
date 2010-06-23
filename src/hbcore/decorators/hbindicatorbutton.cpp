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
#include <hbiconanimationmanager.h>
#include <hbiconitem.h>
#include <hbiconanimator.h>

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
    mHandleIcon(0), mDefaultAction(0), mNewEventAction(0), mProgressAction(0), mDeviceDialog(0), 
    mProgressAnimationFound(false), mNewEventIcon(0), mNewEvent(false), mStyle(0), mIndicatorMenuOpen(false)
{

}

HbIndicatorButtonPrivate::~HbIndicatorButtonPrivate()
{
    delete mDeviceDialog;
}

void HbIndicatorButtonPrivate::init()
{
    Q_Q(HbIndicatorButton);
    setBackgroundVisible(false);
    mProgressAnimationFound = HbIconAnimationManager::global()->addDefinitionFile(
        "qtg_anim_mono_loading.axml");
    
    // add default actions
    mDefaultAction = new HbAction(HbIcon("qtg_mono_options_menu"), "IndicatorMenu", q);
    mNewEventAction = new HbAction(HbIcon("qtg_mono_new_event"), "IndicatorMenu", q);

    QString iconName("qtg_anim_mono_loading_1");
    if (mProgressAnimationFound) {
        iconName = "qtg_anim_mono_loading";
    }
    HbIcon icon(iconName);
    icon.setFlags(HbIcon::Colorized);
    mProgressAction = new HbAction(icon, "IndicatorMenu", q);
}

void HbIndicatorButtonPrivate::showIndicatorMenu()
{
    QVariantMap parametersMap;
    QString noteType(noteIndicatorType);

    parametersMap.clear();
    mDeviceDialog->show(noteType, parametersMap);
    mNewEvent = false;
    mIndicatorMenuOpen = true;

    updateIcon();
}

void HbIndicatorButtonPrivate::addIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        if (clientInfo.at(i).hasMenuData) {
            mIndicators.prepend(clientInfo.at(i));
            if (clientInfo.at(i).category == HbIndicatorInterface::NotificationCategory) {
                mNewEvent = true;
            }
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

    setStyle();
    switch (mStyle)
    {
    case 0:
        q->setAction(mDefaultAction);
        q->setProperty("layout", 1);
        break;
    case 1:
        q->setAction(mNewEventAction);
        q->setProperty("layout", 1);
        break;
    case 2:
        q->setAction(mProgressAction);
        q->setProperty("layout", 1);
        break;
    case 3:
        q->setAction(mProgressAction);
        q->setProperty("layout", 2);
        break;
    default:
        q->setAction(mDefaultAction);
        q->setProperty("layout", 1);
        break;
    }
    q->repolish();
}

void HbIndicatorButtonPrivate::setStyle()
{
    bool newEvent(false);
    bool progress(false);
    for (int i = 0; i < mIndicators.size(); ++i) {
        if (mIndicators.at(i).category == HbIndicatorInterface::NotificationCategory && mNewEvent) {
            newEvent = true;
        }
        if (mIndicators.at(i).category == HbIndicatorInterface::ProgressCategory) {
            progress = true;
        }
    }

    if (!newEvent && !progress) {
        mStyle = 0;
    } else if (newEvent && !progress){
        mStyle = 1;
    } else if (!newEvent && progress){
        mStyle = 2;
    } else {
        mStyle = 3;
    }
}

HbIndicatorButton::HbIndicatorButton(QGraphicsItem *parent) 
    : HbToolButton(*new HbIndicatorButtonPrivate, parent)
{
    Q_D(HbIndicatorButton);
    setProperty("layout", 1);
    d->init(); 

    setAction(d->mDefaultAction);

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

    d->mDeviceDialog = new HbDeviceDialog(HbDeviceDialog::ImmediateResourceReservationFlag);
    connect(d->mDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(resetBackground()));
}

void HbIndicatorButton::showHandleIndication(bool show)
{
    Q_D(HbIndicatorButton);
    d->mHandleIcon->setVisible(show);
}

bool HbIndicatorButton::handleVisible() const
{
    bool handleVisible = false;
    if (mainWindow() && mainWindow()->currentView()) {
        handleVisible = mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarMinimizable;
    }
    return handleVisible;
}

int HbIndicatorButton::buttonStyle() const
{
    Q_D(const HbIndicatorButton);
    return d->mStyle;
}

void HbIndicatorButton::currentViewChanged(HbView *view)
{
    Q_D(HbIndicatorButton);
    HbIconItem *item = dynamic_cast<HbIconItem *>(d->iconItem);
    if (item) {
        item->animator().setOwnerView(view);
    }
}

void HbIndicatorButton::createPrimitives()
{
    Q_D(HbIndicatorButton);
    d->mHandleIcon = style()->createPrimitive(HbStyle::P_IndicatorButton_handleindication, this);
    d->mHandleIcon->setVisible(false);
    d->mNewEventIcon = style()->createPrimitive(HbStyle::P_IndicatorButton_eventindication, this);
    setBackgroundItem(HbStyle::P_IndicatorButton_background); // calls updatePrimitives()
}

void HbIndicatorButton::updatePrimitives()
{
    Q_D(HbIndicatorButton);
    HbStyleOptionIndicatorButton option;
    initStyleOption(&option);
    style()->updatePrimitive(backgroundItem(), HbStyle::P_IndicatorButton_background, &option);
    style()->updatePrimitive(d->mHandleIcon, HbStyle::P_IndicatorButton_handleindication, &option);
    style()->updatePrimitive(d->mNewEventIcon, HbStyle::P_IndicatorButton_eventindication, &option);
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

void HbIndicatorButton::activateAll(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorButton);
    d->mIndicators.clear();
    d->addIndicators(clientInfo);
}

void HbIndicatorButton::resetBackground()
{
    Q_D(HbIndicatorButton);
    d->mIndicatorMenuOpen = false;
    updatePrimitives();
}

void HbIndicatorButton::initStyleOption(HbStyleOptionIndicatorButton *option) const
{
    Q_D(const HbIndicatorButton);
    if (isDown()) {
        option->mode = QIcon::Active;
    } else if (d->mIndicatorMenuOpen) {
        option->mode = QIcon::Selected;
    } else {
        option->mode = QIcon::Normal;
    }
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            option->transparent = true;
        }
    }
    option->twoIcons = (d->mStyle == 3);
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
    if (isUnderMouse()) {
        d->showIndicatorMenu();
    }
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
    updatePrimitives();
}

