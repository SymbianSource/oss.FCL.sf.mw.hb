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

#include <QGraphicsSceneResizeEvent>

#include <hbeffect.h>
#include "hbsoftkey_p.h"
#include "hbsoftkey_p_p.h"
#include "hbnamespace.h"
#include "hbnamespace_p.h"
#include "hbdeviceprofile.h"
#include "hbevent.h"
#include "hbaction.h"
#include "hbtoolbutton.h"
#include "hbtoolbutton_p.h"
#include "hbinstance.h"
#include "hbiconitem.h"
#include "hbstyle.h"
#include "hbstyleoptionsoftkey.h"
#include <QDebug>
HbSoftKeyPrivate::HbSoftKeyPrivate() :
    mButton(0)
{
}

void HbSoftKeyPrivate::init(HbPrivate::SoftKeyId key)
{
    Q_Q(HbSoftKey);

    mKey = key;
    mButton = new HbToolButton(q);
    mButton->setBackgroundItem(HbStyle::P_Softkey_background);
    HbToolButtonPrivate::d_ptr(mButton)->setBackgroundVisible(false);
    updatePrimitives();
}

void HbSoftKeyPrivate::updatePrimitives()
{
    Q_Q(HbSoftKey);
    HbStyleOptionSoftKey option;
    q->initStyleOption(&option);
    if (mButton->isDown()) {
        option.mode = QIcon::Active;
    }
    if (HbDeviceProfile::profile(q).touch()) {
        q->style()->updatePrimitive(mButton->backgroundItem(), HbStyle::P_Softkey_background, &option);
    } else {
        // Hide icon & background & show text
        mButton->setEnabled(true);
        mButton->setToolButtonStyle(HbToolButton::ToolButtonText);
    }
}

/*!
	@beta
    @hbcore
    \class HbSoftKey
    \brief HbSoftKey is an Hb decorator and it implements softkeys which are usually used to execute
    various types of application and platform functionality.

    There are a number of pre-defined soft key types: OptionsKey, BackKey, SearchKey.
    It is possible to attach the softkeys to various locations on screen: top, bottom, left, right.
    HbSoftkey uses HbToolButton to show the actual widget, so HbSoftkey is merely a class that adds platform-defined
    layouting for an HbToolButton.

    HbSoftkeyGroup can be used to access the platform-defined softkeys.

    Here's how the softkey group creates the softkey and connects it to launch a menu

    \code

    HbSoftKey* primary = new HbSoftKey(HbPrivate::PrimarySoftKey, this);
    primary->setData(Hb::StateRole, "primary");

    HbAction *action = new HbAction(this);
    connect(action, SIGNAL(triggered()), d, SLOT(launchMenu()));
    primary->addAction(action);
        
    \endcode
*/

// ======== MEMBER FUNCTIONS ========


/*
    Constructor. 
    \param key Id needs to be given; the platform defined ids will place the softkey in a platform defined position.
    Custom ids can also be given but the positioning needs to be then handled by the client.

    \param parent Optional.

*/

HbSoftKey::HbSoftKey(HbPrivate::SoftKeyId key, QGraphicsItem *parent) 
    : HbDecorator(*new HbSoftKeyPrivate, SoftKey, parent)
{
    Q_D(HbSoftKey);
    d->init(key);
    connect(d->mButton, SIGNAL(pressed()), this, SLOT(buttonPressed()));
    connect(d->mButton, SIGNAL(released()), this, SLOT(buttonReleased()));
}

HbSoftKey::HbSoftKey(HbSoftKeyPrivate &dd, HbPrivate::SoftKeyId key, QGraphicsItem *parent )
    :  HbDecorator(dd, SoftKey, parent)
{
    Q_D(HbSoftKey);
    d->init(key);
}

HbSoftKey::~HbSoftKey()
{

}

HbAction *HbSoftKey::action() const
{
    Q_D(const HbSoftKey);
    return (d->mActions.isEmpty()? 0 : d->mActions.last());
}

void HbSoftKey::addAction(HbAction *action)
{
    Q_D(HbSoftKey);

    if (action) {
        d->mActions << action;
        d->mButton->setAction(action);
    }
}

void HbSoftKey::removeAction(HbAction *action)
{
    Q_D(HbSoftKey);

    if (action && !d->mActions.isEmpty()) {
        if (d->mActions.last() == action) {
            d->mActions.removeLast();
            if (!d->mActions.isEmpty()) {
                d->mButton->setAction(d->mActions.last());
            } else {
                d->mButton->setAction(0);
            }
        }
        else {
            int index = d->mActions.lastIndexOf(action);
            if (index != -1) {
                d->mActions.removeAt(index);
            }
        }
    }
}

/*!
    \reimp
 */
void HbSoftKey::updatePrimitives()
{
    Q_D(HbSoftKey);

    d->updatePrimitives();
}

void HbSoftKey::buttonPressed()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "pressed");
#endif
    updatePrimitives();
}

void HbSoftKey::buttonReleased()
{
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
    updatePrimitives();
}

void HbSoftKey::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    Q_D(HbSoftKey);
    HbDecorator::resizeEvent(event);
    
    if (d->mButton) {
        d->mButton->resize(event->newSize());
    }
}

bool HbSoftKey::event(QEvent *e)
{
    if (e->type() == HbEvent::DeviceProfileChanged) {
        updatePrimitives();
    }
    return HbDecorator::event(e);
}

HbPrivate::SoftKeyId HbSoftKey::softKeyId()
{
    Q_D(HbSoftKey);
    return d->mKey;
}

/*!
    Initializes \a option with the values from this HbSoftKey.
 */
void HbSoftKey::initStyleOption(HbStyleOptionSoftKey *option) const
{
    const Q_D(HbSoftKey);
    option->mKey = d->mKey;
    HbDecorator::initStyleOption(option);
}

