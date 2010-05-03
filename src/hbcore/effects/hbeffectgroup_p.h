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

#ifndef HB_EFFECT_GROUP_P_H
#define HB_EFFECT_GROUP_P_H

#include <hbnamespace.h>
#include <hbglobal.h>
#include <hbeffectinternal_p.h>
#include <QList>
#include <QString>
#include <QObject>
#include <QVariant>
#include <QPointer>
#include <QTransform>

QT_BEGIN_NAMESPACE

class QGraphicsItem;
class QGraphicsEffect;
class QVariant;

QT_END_NAMESPACE

class HbEffectAbstract;
class HbEffectController;
class HbEffect;
class HbVgChainedEffect;
class HbView;

class HB_AUTOTEST_EXPORT HbEffectGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString effectEventType READ effectEventType)

public:
    HbEffectGroup(const QString &effectEventType,
                  QGraphicsItem *registrationItem,
                  QGraphicsItem *targetItem,
                  const QString &itemType);
    ~HbEffectGroup();

    QString effectEventType() const;
    QString itemType() const;
    QGraphicsItem *registrationItem() const;
    QGraphicsItem *targetItem() const;

    void addEffect(HbEffectAbstract *effect);
    void removeEffect(HbEffectAbstract *effect);
    void fixEffectOrder();
    
    void setObserver(QObject *observer, const QString &effectFinishedSlotName);
    void updateItemTransform();

    bool dirty() const;
    void setDirty(bool dirty);
    int effectCount() const;

    bool isRunning() const;
    void setLooping(bool looping);
    bool isLooping() const;
    void pause();
    void resume();

    const QVariant &userData() const;
    void setUserData(const QVariant &userData);

    QRectF extRect() const;
    void setExtRect(const QRectF &extRect);

    HbView *view() const;
    void setView(HbView *view);

    void setEffectFlags(HbEffectInternal::EffectFlags flags);

#ifdef HB_FILTER_EFFECTS
    HbVgChainedEffect *vgEffect();
    void activateVgEffect();
    void deactivateVgEffect();
#endif // HB_FILTER_EFFECTS

    bool hasTranslateEffect() const;
    bool hasRotateEffect() const;
    bool hasScaleEffect() const;
    bool hasOpacityEffect() const;

private:
    void resolveView();
    void doClearEffect(const QTransform *transform, bool opacityEffectUsed);

public slots:
    void startAll();
    void cancelAll(bool sendCallback, bool itemIsValid = true, bool clearEffect = false, const QTransform &initialItemTransform = QTransform());
    void effectFinished(Hb::EffectEvent reason = Hb::EffectFinished);

private slots:
    void clearEffectRunning();
    void invokeObserver(Hb::EffectEvent reason = Hb::EffectFinished);

private:
    QGraphicsItem *mRegistrationItem;
    QGraphicsItem *mTargetItem;
    QString mEffectEventType;
    QString mItemType;
    bool mDirty;

    QList<HbEffectAbstract *> mEffects;
    
    HbVgChainedEffect *mVgEffect;
    bool mVgEffectActivated;
    QPointer<QGraphicsEffect> mVgEffectGuard;

    int mFinishedCount;
    QObject *mObserver;
    QString mEffectFinishedSlotName;

    enum RunningState {
        NotRunning = 0,
        Running = 1,
        FinishInProgress = 2
    };

    RunningState mRunningState;
    QVariant mUserData;
    QRectF mExtRect;
    bool mLooping;

    HbView *mView;
    HbEffectInternal::EffectFlags mEffectFlags;
    bool mRegItemHidden;
    bool mTargetItemHidden;
};

#endif // HB_EFFECT_GROUP_P_H
