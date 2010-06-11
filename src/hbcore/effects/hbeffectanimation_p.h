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

#ifndef HB_EFFECT_ANIMATION_P_H
#define HB_EFFECT_ANIMATION_P_H

#include "hbglobal.h"
#include <QVariantAnimation>

class HbEffectFxmlParamData;
class HbEffectGroup;

class HB_AUTOTEST_EXPORT HbEffectAnimation : public QVariantAnimation
{
    Q_OBJECT
public:
    HbEffectAnimation(HbEffectGroup *group);
    virtual ~HbEffectAnimation();

    void addLooping(const HbEffectFxmlParamData *param);

    virtual void handleAnimationUpdate(const QVariant &value) = 0;
    virtual void handleAnimationFinish() = 0;

protected:
    virtual QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const;

private:
    void updateCurrentValue(const QVariant &value);
    inline bool looping() const {
        return mLoopEnd > 0;
    }

private slots:
    void handleFinished();

public:
    HbEffectGroup *mGroup;
    // The curve is stored here instead of setting it to QVariantAnimation so that it uses linear curve always
    // and does not mess up with applying the curve to each keyframe separately.
    QEasingCurve mCurve;
    int mLoopStart;
    int mLoopEnd;
    bool mInactive; // This is used to temporarily prevent the animation from reacting to false update requests
    bool mFinished;
};

#endif // HB_EFFECT_ANIMATION_P_H
