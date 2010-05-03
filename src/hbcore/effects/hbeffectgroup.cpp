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

#include "hbeffectgroup_p.h"
#include "hbeffectabstract_p.h"
#include "hbeffect.h"
#include "hbtimer_p.h"
#include "hbeffectdef_p.h"
#include "hbmainwindow.h"
#include "hbinstance.h"

#include <qglobal.h>
#include <QMetaObject>
#include <QTransform>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QGraphicsView>

#ifdef HB_FILTER_EFFECTS
#include "hbvgeffect_p.h"
#include "hbvgchainedeffect_p.h"
#endif

HbEffectGroup::HbEffectGroup(
    const QString &effectEventType, 
    QGraphicsItem *registrationItem, 
    QGraphicsItem *targetItem,
    const QString &itemType)
    : mRegistrationItem(registrationItem),
      mTargetItem(targetItem),
      mEffectEventType(effectEventType),
      mItemType(itemType),
      mDirty(false),
      mVgEffect(0),
      mVgEffectActivated(false),
      mFinishedCount(0),
      mObserver(0),
      mRunningState(NotRunning),
      mLooping(false),
      mView(0),
      mEffectFlags(HbEffectInternal::Normal),
      mRegItemHidden(false),
      mTargetItemHidden(false)
{
}

HbEffectGroup::~HbEffectGroup()
{
    qDeleteAll(mEffects);

#ifdef HB_FILTER_EFFECTS
    // Delete the vg effect if its ownership has not been transferred to the graphics item
    if (!mVgEffectActivated && !mVgEffectGuard.isNull()) {
        delete mVgEffect;
    }
#endif
}

QString HbEffectGroup::effectEventType() const
{
    return mEffectEventType;
}

QString HbEffectGroup::itemType() const
{
    return mItemType;
}

QGraphicsItem *HbEffectGroup::registrationItem() const
{
    return mRegistrationItem;
}

QGraphicsItem *HbEffectGroup::targetItem() const
{
    return mTargetItem;
}

void HbEffectGroup::addEffect(HbEffectAbstract *effect)
{
    mEffects.append(effect);
}

void HbEffectGroup::removeEffect(HbEffectAbstract *effect)
{
    mEffects.removeAll(effect);
}

/*
* Fixes the order of the effects to be optimal for item transformations.
* E.g. translate needs to be after scale because translating a scaled matrix would not lead to the wanted result.
* Also there are some problems with rotation effect if it's done after scale effect
*/
void HbEffectGroup::fixEffectOrder()
{
    // If there is only one effect, no need to change order
    if (mEffects.count() > 1) {
        int last = mEffects.count() - 1;

        for (int i = last; i >= 0; --i) {
            HbEffectAbstract *effect = mEffects[i];
            if (effect->name() == HB_EFFECT_NAME_TRANSLATE) {
                // Move translate effect last in the effect list
                mEffects.takeAt(i);
                mEffects.append(effect);
            }
        }

        for (int i = last; i >= 0; --i) {
            HbEffectAbstract *effect = mEffects[i];
            if (effect->name() == HB_EFFECT_NAME_SCALE) {
                // Move scale effect second last in the effect list
                mEffects.takeAt(i);
                mEffects.insert(mEffects.size()-1, effect);
            }
        }

    }
}

void HbEffectGroup::setObserver(QObject *observer, const QString &effectFinishedSlotName)
{
    mObserver = observer;
    mEffectFinishedSlotName = effectFinishedSlotName;
}

void HbEffectGroup::updateItemTransform()
{
    QGraphicsView *gv(0); 
    // support for graphics view transforms
    if (mTargetItem->type() == HbGVWrapperItemType) {
        HbGVWrapperItem *gvw = static_cast<HbGVWrapperItem*>(mTargetItem);
        if (gvw)
            gv = gvw->mainWindow();
    }
    QTransform transform;

    foreach (HbEffectAbstract *effect, mEffects) {
        if (effect)
            effect->updateItemTransform(transform);
    }
    if (!gv) {
        mTargetItem->setTransform(transform);
    } else {
        gv->setTransform(transform);
    }
}

bool HbEffectGroup::dirty() const
{
    return mDirty;
}

void HbEffectGroup::setDirty(bool dirty)
{
    mDirty = dirty;
}

int HbEffectGroup::effectCount() const
{
	return mEffects.count();
}

bool HbEffectGroup::isRunning() const
{
    return mRunningState != NotRunning;
}

void HbEffectGroup::setLooping(bool looping)
{
    mLooping = looping;
}

bool HbEffectGroup::isLooping() const
{
    return mLooping;
}

void HbEffectGroup::pause()
{
    foreach (HbEffectAbstract *effect, mEffects) {
        effect->pause();
    }
}

void HbEffectGroup::resume()
{
    foreach (HbEffectAbstract *effect, mEffects) {
        effect->resume();
    }
}

const QVariant &HbEffectGroup::userData() const
{
    return mUserData;
}

void HbEffectGroup::setUserData(const QVariant &userData)
{
    mUserData = userData;
}

QRectF HbEffectGroup::extRect() const
{
    return mExtRect;
}

void HbEffectGroup::setExtRect(const QRectF &extRect)
{
    mExtRect = extRect;

    // This is needed to make scaling work from an extrect that has an equal size with the item's rect
    if (mTargetItem) {
        QRectF itemRect = mTargetItem->boundingRect();
        qreal width = itemRect.width();
        if (qFuzzyCompare(width, mExtRect.width())) {
            mExtRect.setWidth(width + 0.00001);
        }
        qreal height = itemRect.height();
        if (qFuzzyCompare(height, mExtRect.height())) {
            mExtRect.setHeight(height + 0.00001);
        }
    }
}

HbView *HbEffectGroup::view() const
{
    return mView;
}

void HbEffectGroup::setView(HbView *view)
{
    mView = view;
}

#ifdef HB_FILTER_EFFECTS

HbVgChainedEffect *HbEffectGroup::vgEffect()
{
    if (!mVgEffect) {
        mVgEffect = new HbVgChainedEffect;
    }
    return mVgEffect;
}

void HbEffectGroup::activateVgEffect()
{
    if (!mVgEffectActivated) {
        mVgEffectGuard = QPointer<QGraphicsEffect>();
        vgEffect()->install(mTargetItem);
        mVgEffectActivated = true;
    }
}

/**
* Removes the VG effect from the graphics item without deleting it.
* The ownership is moved back to the effect group.
*/
void HbEffectGroup::deactivateVgEffect()
{
    if (mVgEffectActivated) {
        // This does not delete the effect so ownership is transferred back to
        // the effect group.  However this is believed to be a bug in Qt 4.6.0
        // so use a QPointer to make sure that we do not do double deletion in
        // case Qt starts deleting the effect correctly in the future.
        mVgEffectGuard = QPointer<QGraphicsEffect>(mTargetItem->graphicsEffect());
        mTargetItem->setGraphicsEffect(0);
        mVgEffectActivated = false;
    }
}

#endif // HB_FILTER_EFFECTS

void HbEffectGroup::startAll()
{
    if (!mEffects.empty()) {
        mRunningState = Running;
        mFinishedCount = 0;
    }

    // First resolve parameters and set the start states for all the effects.
    // This is done before starting the effect animations to avoid screen flickering.
    QTransform transform;
    foreach (HbEffectAbstract *effect, mEffects) {
        // Resolve parameters etc.
        effect->init();
        if (effect->interval() == 0) {
            // Set start state if effect starts immediately
            effect->setStartState(transform);
        }
    }
    mTargetItem->setTransform(transform);

    // Make the target item visible, if needed, now that the start state is set
    // for all the effects.
    if (mEffectFlags.testFlag(HbEffectInternal::ShowItemOnFirstUpdate)) {
        // In case of a view switch the registration item may be the HbView
        // itself and the target item is just the view's content widget. Make
        // sure both are visible.
        mTargetItem->setVisible(true);
        if (mRegistrationItem != mTargetItem) {
            mRegistrationItem->setVisible(true);
        }
    }

    mRegItemHidden = false;
    mTargetItemHidden = false;

    if (mEffects.empty()) {
        // No effect exists but user wants notification when effect finishes. 
        // Let the user do whatever he wanted to do when effect finishes.
        invokeObserver(Hb::EffectNotStarted);
    } else {
        // Start state has been set for all the effects,
        // next step is to start the effect animations.
        // Before that, resolve the view where the effect belongs if the effect is looping.
        // This is needed for being able to pause looping effects when their view is inactive.
        if (isLooping()) {
            resolveView();
        }
        foreach (HbEffectAbstract *effect, mEffects) {
            // If the starttime is zero, start effect immediately
            if (effect->interval() == 0) {
                effect->start(); // This may call group's effectFinished if the effect was empty.
            } else {
                //Else register the effect to timeline to wait its turn.
                HbTimer::instance()->registerEntry(effect);
            }
        }
    }
}

void HbEffectGroup::resolveView()
{
    if (!mView && mTargetItem) {
        QGraphicsScene *scene = mTargetItem->scene();
        if (scene) {
            // Resolve the main window having the same scene that the item belongs to
            QList<HbMainWindow *> windowList = hbInstance->allMainWindows();
            foreach (const HbMainWindow *window, windowList) {
                if (window->scene() == scene) {
                    mView = window->currentView();
                    break;
                }
            }
        }
    }
}

bool HbEffectGroup::hasTranslateEffect() const
{
    foreach (HbEffectAbstract *effect, mEffects) {
        if (effect->name() == HB_EFFECT_NAME_TRANSLATE) {
            return true;
        }
    }
    return false;
}

bool HbEffectGroup::hasRotateEffect() const
{
    foreach (HbEffectAbstract *effect, mEffects) {
        if (effect->name() == HB_EFFECT_NAME_ROTATE) {
            return true;
        }
    }

    return false;
}

bool HbEffectGroup::hasScaleEffect() const
{
    foreach (HbEffectAbstract *effect, mEffects) {
        if (effect->name() == HB_EFFECT_NAME_SCALE) {
            return true;
        }
    }
    return false;
}

bool HbEffectGroup::hasOpacityEffect() const
{
    foreach (HbEffectAbstract *effect, mEffects) {
        if (effect->name() == HB_EFFECT_NAME_OPACITY) {
            return true;
        }
    }
    return false;
}

void HbEffectGroup::doClearEffect(const QTransform *transform, bool opacityEffectUsed)
{
    // Hide registration item if needed.
    if (mEffectFlags.testFlag(HbEffectInternal::HideRegItemBeforeClearingEffect)) {
        // We get here also from cancelAll() when starting a new effect so there
        // must be some guarding to do the hiding only once.
        if (!mRegItemHidden) {
            mRegItemHidden = true;
            // This flag affects the registration item only.
            mRegistrationItem->setVisible(false);
        }
    }
    // Same for the target item.
    if (mEffectFlags.testFlag(HbEffectInternal::HideTargetItemBeforeClearingEffect)) {
        if (!mTargetItemHidden) {
            mTargetItemHidden = true;
            mTargetItem->setVisible(false);
        }
    }
    // Reset the transformation.
    mTargetItem->setTransform(transform ? *transform : QTransform());
    // Reset opacity.
    if (opacityEffectUsed) {
        // Hide opacity effect by setting item fully opaque regardless of what
        // its opacity value was before the effect.
        mTargetItem->setOpacity(1.0f);
    }
    // Reset filter effects.
#ifdef HB_FILTER_EFFECTS            
    deactivateVgEffect();
#endif            
}

void HbEffectGroup::cancelAll(bool sendCallback, bool itemIsValid, bool clearEffect, const QTransform &initialItemTransform)
{
    // No checks for running state here. The cancellation (esp. the clearing of
    // the effect (if needed)) must be done always, regardless of the effect's
    // state.

    QTransform transform;
    bool opacityEffectUsed = false;

    foreach (HbEffectAbstract *effect, mEffects) {
        if (effect) {
            HbTimer::instance()->unregisterEntry(effect);
            effect->cancel(transform, itemIsValid);
            if (effect->name() == HB_EFFECT_NAME_OPACITY) {
                opacityEffectUsed = true;
            }
        }
    }

    if (itemIsValid) {
        // If effect needs to be removed, reset transform matrix and deactivate VG effect
        if (clearEffect || mEffectFlags.testFlag(HbEffectInternal::ClearEffectWhenFinished)) {
            doClearEffect(&initialItemTransform, opacityEffectUsed);
        } else { // Otherwise set transform corresponding to the end state of the effect
            mTargetItem->setTransform(initialItemTransform * transform);
        }
    }

    // Do not set this to NotRunning before effect->cancel has been called because filter
    // effects cancel does stuff that requires group->isRunning() return true.
    mRunningState = NotRunning;

    // Invoke observer with cancel signal
    if (sendCallback) {
        invokeObserver(Hb::EffectCancelled);
    }
}

void HbEffectGroup::effectFinished(Hb::EffectEvent reason)
{
    // Inform the animated item when the whole effect group has finished.
    if (++mFinishedCount == mEffects.count()) {
        mFinishedCount = 0;

        // The animation framework funnily enough sends the finished signal
        // before updating the animation with the final value, so here we set
        // running state to NotRunning asynchronously so the effect's final
        // value gets still updated.
        mRunningState = FinishInProgress;
        QMetaObject::invokeMethod(this, "clearEffectRunning", Qt::QueuedConnection);

        // Send callback if observer has been provided. Note that with the
        // EffectFinished reason the observer will be invoked asynchronously.
        invokeObserver(reason);
    }

    // The effect group object is not deleted here,
    // because it would need to be removed from the list of the effect groups in
    // the effect engine as well.
    // It will be deleted and a new effect group created when the same effect is started again.
}

void HbEffectGroup::clearEffectRunning()
{
    // Comes here when effect has finished and running state is set to NotRunning asynchronously.
    // Only set running state to 'NotRunning' if the finish is still in progress, i.e. the effect
    // has not been restarted meanwhile.
    if (mRunningState == FinishInProgress) {
        mRunningState = NotRunning;
        // We are finished either normally or with EffectNotStarted. It is now
        // the time to get rid of all the "effects" caused by the effects in
        // this group, if needed.
        if (mEffectFlags.testFlag(HbEffectInternal::ClearEffectWhenFinished)) {
            doClearEffect(0, hasOpacityEffect());
        }
    }
}

void HbEffectGroup::invokeObserver(Hb::EffectEvent reason)
{
    // Send callback if observer has been provided
    if (mRegistrationItem && mTargetItem && mObserver && !mEffectFinishedSlotName.isEmpty()) {
        HbEffect::EffectStatus status;
        status.item = mRegistrationItem;
        status.effectEvent = mEffectEventType;
        status.userData = mUserData;
        status.reason = reason;

        QObject *observer = mObserver;    
    
        // Clear the observer to make sure it is not sent more than once.
        // This is done before invokeMethod to avoid crash if the callback
        // deletes this object.
        mObserver = 0;

        // Send callback if observer has been provided. Use queued connection if
        // the effect finished normally, because otherwise deleting the effect
        // during the callback would cause crash because this function finally
        // returns back to animation framework code which assumes the effect
        // objects are alive.
        QMetaObject::invokeMethod(
            observer,
            mEffectFinishedSlotName.toAscii().data(),
            reason == Hb::EffectFinished ? Qt::QueuedConnection : Qt::AutoConnection,
            QGenericReturnArgument(),
            Q_ARG(HbEffect::EffectStatus, status));

        // Do not access member variables after invoking the callback since it might
        // have deleted this object.
    }
}

void HbEffectGroup::setEffectFlags(HbEffectInternal::EffectFlags flags)
{
    mEffectFlags = flags;
}

// End of File
