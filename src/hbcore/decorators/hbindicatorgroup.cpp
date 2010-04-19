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

#include <QTimer>

#include "hbindicatorgroup_p.h"
#include "hbindicatorgroup_p_p.h"
#include "hbstyleoptionindicatorgroup.h"

#include "hbiconitem.h"

#if defined(Q_OS_SYMBIAN)
#include "hbindicatorsym_p.h"
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

HbIndicatorGroupPrivate::HbIndicatorGroupPrivate() :
    mIndicatorType(HbIndicatorGroup::NotificationType),
    mIndicatorPrivate(0), mProgressAdded(false)
{
}

HbIndicatorGroupPrivate::~HbIndicatorGroupPrivate()
{
     mIndicatorPrivate->stopListen();     
     delete mIndicatorPrivate;

     mIndicators.clear();
}

void HbIndicatorGroupPrivate::init()
{
    Q_Q(HbIndicatorGroup);
    q->createPrimitives();

    mIndicatorPrivate = new HbIndicatorPrivate;
    mIndicatorPrivate->init();

    q->connect(mIndicatorPrivate, SIGNAL(activated(const QList<IndicatorClientInfo> &)),
        q, SLOT(activate(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(updated(const QList<IndicatorClientInfo> &)),
        q, SLOT(update(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(allActivated(const QList<IndicatorClientInfo> &)),
        q, SLOT(activateAll(const QList<IndicatorClientInfo> &)));
    q->connect(mIndicatorPrivate, SIGNAL(deactivated(const QList<IndicatorClientInfo> &)),
        q, SLOT(deactivate(const QList<IndicatorClientInfo> &)));

    QTimer::singleShot(0, q, SLOT(startListen()));
}

int HbIndicatorGroupPrivate::setIconName(HbStyleOptionIndicatorGroup &option, int index)
{
    bool ok(mProgressAdded);
    while (ok && mIndicators.count() > index) {
        if (mIndicators.at(index).category == HbIndicatorInterface::ProgressCategory) {
            ++index;
        } else {
            ok = false;
        }
    }

    if (mIndicators.count() > index) {
        if (mIndicators.at(index).category == HbIndicatorInterface::ProgressCategory) {
            if (!mProgressAdded) {
                option.iconName = "qtg_status_progress";
                mProgressAdded = true;
            }
        } else {
            option.iconName = mIndicators.at(index).iconPath;
        }
    } else {
        option.iconName = "";
    }
    return ++index;
}

void HbIndicatorGroupPrivate::addIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        const IndicatorClientInfo &indicator = clientInfo.at(i);
        if (canAddIndicator(indicator)) {
            mIndicators.prepend(indicator);
        }
    }
    emitNotificationCount();
}

void HbIndicatorGroupPrivate::updateIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        int index = findIndicator(clientInfo.at(i));
        if (index >= 0) {
            mIndicators[index].iconPath = clientInfo.at(i).iconPath;
        }
    }
}

void HbIndicatorGroupPrivate::removeIndicators(const QList<IndicatorClientInfo> &clientInfo)
{
    for (int i = 0; i < clientInfo.size(); ++i) {
        removeIndicator(clientInfo.at(i));
    }
    emitNotificationCount();
}

int HbIndicatorGroupPrivate::findIndicator(const IndicatorClientInfo &indicator) const
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

void HbIndicatorGroupPrivate::removeIndicator(const IndicatorClientInfo &indicator)
{
    int index = findIndicator(indicator);
    if (index >= 0) {
        mIndicators.removeAt(index);
    }
}

bool HbIndicatorGroupPrivate::canAddIndicator(const IndicatorClientInfo &indicator) const
{
    bool canAdd(false);

    if (indicator.category == HbIndicatorInterface::NotificationCategory 
        && mIndicatorType == HbIndicatorGroup::NotificationType) {
        canAdd = true;
    } else if (indicator.category == HbIndicatorInterface::SettingCategory 
        && mIndicatorType == HbIndicatorGroup::SettingsType) {
        canAdd = true;
    } else if (indicator.category == HbIndicatorInterface::ProgressCategory
        && mIndicatorType == HbIndicatorGroup::SettingsType) {
        canAdd = true;
    }
    return canAdd;
}

void HbIndicatorGroupPrivate::emitNotificationCount()
{
    Q_Q(HbIndicatorGroup);
    if (mIndicatorType == HbIndicatorGroup::NotificationType) {
        emit q->notificationCountChanged(mIndicators.size());
    }
}

// ======== MEMBER FUNCTIONS ========

/*
    Constructs a indicator group with \a parent.
*/
HbIndicatorGroup::HbIndicatorGroup(IndicatorType indicatorType, QGraphicsItem *parent)
    : HbWidget(*new HbIndicatorGroupPrivate, parent)
{
    Q_D(HbIndicatorGroup);
    d->init();
    d->mIndicatorType = indicatorType;
    setProperty("layout", d->mIndicatorType);
}

/*
    Destructor
 */
HbIndicatorGroup::~HbIndicatorGroup()
{
    
}

void HbIndicatorGroup::createPrimitives()
{
    Q_D(HbIndicatorGroup);
    d->mIcons.append(style()->createPrimitive(HbStyle::P_IndicatorGroup_icon1, this));
    d->mIcons.append(style()->createPrimitive(HbStyle::P_IndicatorGroup_icon2, this));
    d->mIcons.append(style()->createPrimitive(HbStyle::P_IndicatorGroup_icon3, this));
    d->mIcons.append(style()->createPrimitive(HbStyle::P_IndicatorGroup_icon4, this));
}

void HbIndicatorGroup::updatePrimitives()
{
    Q_D(HbIndicatorGroup);
    HbStyleOptionIndicatorGroup option;
    initStyleOption(&option);
    d->mProgressAdded = false;
    int index(0);
    index = d->setIconName(option, index);
    style()->updatePrimitive(d->mIcons[0], HbStyle::P_IndicatorGroup_icon1, &option);
    index = d->setIconName(option, index);
    style()->updatePrimitive(d->mIcons[1], HbStyle::P_IndicatorGroup_icon2, &option);
    index = d->setIconName(option, index);
    style()->updatePrimitive(d->mIcons[2], HbStyle::P_IndicatorGroup_icon3, &option);
    index = d->setIconName(option, index);
    style()->updatePrimitive(d->mIcons[3], HbStyle::P_IndicatorGroup_icon4, &option);
}

void HbIndicatorGroup::activate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorGroup);
    d->addIndicators(clientInfo);

    updatePrimitives();
}

void HbIndicatorGroup::update(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorGroup);
    d->updateIndicators(clientInfo);

    updatePrimitives();
}

void HbIndicatorGroup::activateAll(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorGroup);
    d->mIndicators.clear();
    d->addIndicators(clientInfo);

    updatePrimitives();
}

void HbIndicatorGroup::deactivate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorGroup);
    d->removeIndicators(clientInfo);

    updatePrimitives();
}

void HbIndicatorGroup::startListen()
{
    Q_D(HbIndicatorGroup);
    d->mIndicatorPrivate->startListen();
}

void HbIndicatorGroup::initStyleOption(HbStyleOptionIndicatorGroup *option) const
{
    HbWidget::initStyleOption(option);
}
