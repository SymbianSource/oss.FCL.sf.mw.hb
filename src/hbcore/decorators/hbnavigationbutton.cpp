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

#include <QApplication>

#include <hbeffect.h>
#include <hbview.h>

#include "hbnavigationbutton_p.h"
#include "hbnavigationbutton_p_p.h"
#include "hbstyleoptionnavigationbutton_p.h"
#include "hbmainwindow_p.h"

HbNavigationButtonPrivate::HbNavigationButtonPrivate() : mTouchArea(0)
{
    
}

HbNavigationButtonPrivate::~HbNavigationButtonPrivate()
{

}

void HbNavigationButtonPrivate::init()
{
    setBackgroundVisible(false);
}

HbNavigationButton::HbNavigationButton(QGraphicsItem *parent) 
    : HbToolButton(*new HbNavigationButtonPrivate, parent)
{
    Q_D(HbNavigationButton);
    d->init(); 

    createPrimitives();
}

HbNavigationButton::~HbNavigationButton()
{

}

void HbNavigationButton::delayedConstruction()
{
    connect(this, SIGNAL(pressed()), this, SLOT(handlePress()));
    connect(this, SIGNAL(released()), this, SLOT(handleRelease()));
}

void HbNavigationButton::createPrimitives()
{
    Q_D(HbNavigationButton);
    d->mTouchArea = HbStylePrivate::createPrimitive(HbStylePrivate::P_NavigationButton_toucharea, this);
    ungrabGesture(Qt::TapGesture);
    d->setBackgroundItem(HbStylePrivate::P_NavigationButton_background);
}

void HbNavigationButton::updatePrimitives()
{
    Q_D(HbNavigationButton);
    HbStyleOptionNavigationButton option;
    initStyleOption(&option);
    HbStylePrivate::updatePrimitive(backgroundItem(), HbStylePrivate::P_NavigationButton_background, &option);
    HbStylePrivate::updatePrimitive(d->mTouchArea, HbStylePrivate::P_NavigationButton_toucharea, &option);
    HbToolButton::updatePrimitives();
}

void HbNavigationButton::initStyleOption(HbStyleOptionNavigationButton *option) const
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

void HbNavigationButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LayoutDirectionChange) {
        updatePrimitives();
    }
    HbToolButton::changeEvent(event);
}

/*
  Overloaded hit detection to include touch area
 */
bool HbNavigationButton::hitButton(const QPointF &pos) const
{
    Q_D(const HbNavigationButton);
    QRectF compRect = d->mTouchArea->boundingRect();
    compRect.translate(d->mTouchArea->pos());
    return compRect.contains(pos);
}

void HbNavigationButton::handlePress()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "pressed");
#endif
    updatePrimitives();
}

void HbNavigationButton::handleRelease()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
    updatePrimitives();
}
