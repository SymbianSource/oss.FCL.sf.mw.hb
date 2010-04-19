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
#include "hbstyleoptionindicatorbutton.h"

static const char noteIndicatorType[] = {"com.nokia.hb.indicatormenu/1.0"};

HbIndicatorButtonPrivate::HbIndicatorButtonPrivate()
{

}

HbIndicatorButtonPrivate::~HbIndicatorButtonPrivate()
{

}

void HbIndicatorButtonPrivate::init()
{
    Q_Q(HbIndicatorButton);
    setBackgroundVisible(false);

     // add default actions
    defaultAction = new HbAction(HbIcon("qtg_mono_options_menu"), "IndicatorMenu", q);
    notificationAction = new HbAction(HbIcon("qtg_mono_new_event"), "IndicatorMenu", q);
    q->setAction(defaultAction);
}

void HbIndicatorButtonPrivate::showIndicatorMenu()
{
    HbDeviceDialog *deviceDialog = new HbDeviceDialog();

    QVariantMap parametersMap;
    QString noteType(noteIndicatorType);

    parametersMap.clear();
    deviceDialog->show(noteType, parametersMap);
}

HbIndicatorButton::HbIndicatorButton(QGraphicsItem *parent) 
    : HbToolButton(*new HbIndicatorButtonPrivate, parent)
{
    Q_D(HbIndicatorButton);
    d->init(); 

    createPrimitives();

    connect(this, SIGNAL(pressed()), this, SLOT(handlePress()));
    connect(this, SIGNAL(released()), this, SLOT(handleRelease()));
}

HbIndicatorButton::~HbIndicatorButton()
{

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

void HbIndicatorButton::setIcon(int count)
{
    Q_D(HbIndicatorButton);
    if (count == 0) {
        setAction(d->defaultAction);
    } else {
        setAction(d->notificationAction);
    }
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
    d->showIndicatorMenu();
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
    updatePrimitives();
}
