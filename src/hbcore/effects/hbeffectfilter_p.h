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

#ifndef HB_EFFECT_FILTER_P_H
#define HB_EFFECT_FILTER_P_H

#include <hbglobal.h>
#ifdef HB_FILTER_EFFECTS

#include "hbeffectabstract_p.h"
#include "hbeffectanimation_p.h"
#include <QString>
#include <QVariantAnimation>
#include <QObject>
#include <QSizeF>
#include <QVector>

class HbEffectGroup;
class HbEffectScale;
class HbEffectFilter;
class HbEffectFilterAnimation;
class HbEffectFxmlParamData;

class HB_AUTOTEST_EXPORT HbEffectFilterAnimation : public HbEffectAnimation
{
public:
    HbEffectFilterAnimation(HbEffectFilter *effect, int duration);
    ~HbEffectFilterAnimation();

private:
    void handleAnimationUpdate(const QVariant &value);
    void handleAnimationFinish();

public:
    HbEffectFilter *mEffect;
};

class HB_AUTOTEST_EXPORT HbEffectColorAnimation : public HbEffectFilterAnimation
{
public:
    HbEffectColorAnimation(HbEffectFilter *effect, int duration);
    ~HbEffectColorAnimation();

private:
    QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const;
};

class HB_AUTOTEST_EXPORT HbEffectFilter : public HbEffectAbstract
{
public:
    HbEffectFilter(int startTime, QGraphicsItem *item, HbEffectGroup *group);
    virtual ~HbEffectFilter();

public:
    void init();
    void setStartState(QTransform &transform);
    void start();
    void cancel(QTransform &transform, bool itemIsValid);
    void updateItemTransform(QTransform &transform);
    void pause();
    void resume();

public:
    virtual void updateFilterEffect() = 0;

    HbEffectFilterAnimation *createAnimation(
        const HbEffectFxmlParamData &param,
        qreal &startValue,
        qreal &endValue,
        HbEffectGroup *group);

    HbEffectColorAnimation *createAnimation(
        const HbEffectFxmlParamData &param,
        QColor &startValue,
        QColor &endValue,
        HbEffectGroup *group);

    void handleAnimationFinished();

public:
    QVector<HbEffectFilterAnimation *> mAnimations;
    bool mEffectDefined;
    bool mCanceled;
};

#endif // HB_FILTER_EFFECTS
#endif // HB_EFFECT_FILTER_P_H

