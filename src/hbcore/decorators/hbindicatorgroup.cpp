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

#include <hbiconanimationmanager.h>

#include "hbindicatorgroup_p.h"
#include "hbindicatorgroup_p_p.h"
#include "hbstyleoptionindicatorgroup_p.h"

#include "hbiconitem.h"

#if defined(Q_OS_SYMBIAN)
#include "hbindicatorsym_p.h"
#else
#include "hbindicatorwin32_p.h"
#endif // defined(Q_OS_SYMBIAN)

HbIndicatorGroupPrivate::HbIndicatorGroupPrivate() :
    mIndicatorType(HbIndicatorGroup::NotificationType),
    mProgressAdded(false), mIndicatorAdded(false), mProgressAnimationFound(false)
{
}

HbIndicatorGroupPrivate::~HbIndicatorGroupPrivate()
{
     mIndicators.clear();
}

void HbIndicatorGroupPrivate::delayedConstruction()
{
#ifdef HB_EFFECTS
    HbEffect::add(
        QStringList() << "indicator" << "indicator" << "indicator" << "indicator",
        QStringList() << "indicator_appear" <<  "indicator_disappear" << "indicator_move_right" << "indicator_move_left",
        QStringList() << "appear" << "disappear" <<  "move_right" << "move_left");
#endif

    mProgressAnimationFound = HbIconAnimationManager::global()->addDefinitionFile("qtg_status_progress.axml");
}

void HbIndicatorGroupPrivate::init()
{
    Q_Q(HbIndicatorGroup);
    q->createPrimitives();
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
                if (mProgressAnimationFound) {
                    option.iconName = "qtg_status_progress";
                } else {
                    option.iconName = "qtg_status_progress_1";
                }
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
            mIndicatorAdded = true;
        }
    }
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
    mRemovedIndicators.clear();
    for (int i = 0; i < clientInfo.size(); ++i) {
        removeIndicator(clientInfo.at(i));
    }
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
        if (mIndicators.at(index).category == HbIndicatorInterface::ProgressCategory) {
            if (mProgressAnimationFound) {
                mRemovedIndicators.append("qtg_status_progress");
            } else {
                mRemovedIndicators.append("qtg_status_progress_1");
            }
        } else {
            mRemovedIndicators.append(mIndicators.at(index).iconPath);
        }
        mIndicators.removeAt(index);
    }
}

bool HbIndicatorGroupPrivate::canAddIndicator(const IndicatorClientInfo &indicator) const
{
    bool canAdd(false);

    if (indicator.iconPath.isEmpty()) {
        return canAdd;
    }

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

void HbIndicatorGroupPrivate::startAddingEffect()
{
#ifdef HB_EFFECTS
    Q_Q(HbIndicatorGroup);
    if (mIndicatorAdded) {
        // get the original positions for the first time
        if (mOriginalPos.count() == 0) {
            for (int i = 0; i < mIcons.size(); ++i) {
                mOriginalPos.append(mIcons[i]->pos());
            }
        }
        // Move positions one to left
        if (mIndicatorType == HbIndicatorGroup::NotificationType) {
            for (int i = mIcons.size() - 1; i > 0; --i) {
                mIcons[i]->setPos(mIcons[i - 1]->pos());
             }
        }

        // Start the effects
        HbEffect::start(mIcons[0], "indicator", "appear");
        if (mIndicatorType == HbIndicatorGroup::NotificationType) {
            HbEffect::start(mIcons[1], "indicator", "move_right");
            HbEffect::start(mIcons[2], "indicator", "move_right");
            HbEffect::start(mIcons[3], "indicator", "move_right", q, "moveEffectFinnished");
        } else {
            HbEffect::start(mIcons[1], "indicator", "move_left");
            HbEffect::start(mIcons[2], "indicator", "move_left");
            HbEffect::start(mIcons[3], "indicator", "move_left", q, "moveEffectFinnished");
        }
    }
#endif
}

void HbIndicatorGroupPrivate::startRemovingEffect()
{
#ifdef HB_EFFECTS
    Q_Q(HbIndicatorGroup);
    for (int i = 0; i < mRemovedIndicators.size(); ++i) {   
        int index = findIndicatorIcon(mRemovedIndicators[i]);
        if (index >= 0) {
            HbEffect::start(mIcons[index], "indicator", "disappear", q, "disappearEffectFinnished");
            if (mIndicatorType == HbIndicatorGroup::NotificationType) {
                 for (int i = mIcons.size() - 1; i > index; --i) {
                    mIcons[i]->setPos(mIcons[i - 1]->pos());
                 }
                for (int i = index + 1; i < mIcons.size(); ++i) {
                    HbEffect::start(mIcons[i], "indicator", "move_left");
                }
            } else {
                for (int i = index + 1; i < mIcons.size(); ++i) {
                    HbEffect::start(mIcons[i], "indicator", "move_right");
                }
            }
        }
    }
#else 
    updatePrimitives();
#endif
}

int HbIndicatorGroupPrivate::findIndicatorIcon(const QString &iconPath) const
{
    int index = -1;

    for (int i = 0; i < mIcons.size(); ++i) {
        if (static_cast<HbIconItem*>(mIcons[i])->iconName() == iconPath) {
            index = i;
            break;
        }
    }

    return index;
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

/*
    Delayed constructor.
 */
void HbIndicatorGroup::delayedConstruction()
{
   Q_D(HbIndicatorGroup);
   d->delayedConstruction();
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

    if (d->mIndicatorAdded) {
        d->startAddingEffect();
    }
}

void HbIndicatorGroup::activate(const QList<IndicatorClientInfo> &clientInfo)
{
    Q_D(HbIndicatorGroup);
    d->addIndicators(clientInfo);

    updatePrimitives();
    d->mIndicatorAdded = false;
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

    if (d->mRemovedIndicators.count() > 0) {
        d->startRemovingEffect();
    }
}

void HbIndicatorGroup::initStyleOption(HbStyleOptionIndicatorGroup *option) const
{
    HbWidget::initStyleOption(option);
}

void HbIndicatorGroup::moveEffectFinnished(const HbEffect::EffectStatus &status)
{
    Q_D(HbIndicatorGroup);
    if (status.reason == Hb::EffectFinished || status.reason == Hb::EffectCancelled
        || status.reason == Hb::EffectNotStarted) {
        // Reset the positions and transformations
        for (int i = 0; i < d->mIcons.size(); ++i) {
            d->mIcons[i]->resetTransform();
            d->mIcons[i]->setPos(d->mOriginalPos[i]);
        }
    }
}

void HbIndicatorGroup::disappearEffectFinnished(const HbEffect::EffectStatus &status)
{
    Q_D(HbIndicatorGroup);
    if (status.reason == Hb::EffectFinished || status.reason == Hb::EffectCancelled
        || status.reason == Hb::EffectNotStarted) {
            // Reset the positions and transformations and opacity
        for (int i = 0; i < d->mIcons.size(); ++i) {
            d->mIcons[i]->resetTransform();
            d->mIcons[i]->setOpacity(1.0);
            d->mIcons[i]->setPos(d->mOriginalPos[i]);
        }
        updatePrimitives();
    }
}
