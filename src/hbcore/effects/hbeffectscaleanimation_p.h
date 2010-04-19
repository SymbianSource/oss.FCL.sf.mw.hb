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

#ifndef HB_EFFECT_SCALE_ANIMATION_P_H
#define HB_EFFECT_SCALE_ANIMATION_P_H

#include "hbglobal.h"
#include "hbeffectanimation_p.h"
#include <QString>
#include <QVariantAnimation>
#include <QSizeF>

class HbEffectGroup;
class HbEffectScale;

// A scale animation instance animates either width or height.
// Two animations are used to animate both to be able to set parameters separately to the animations
// as specified in FXML.
class HB_AUTOTEST_EXPORT HbEffectScaleAnimation : public HbEffectAnimation
{
public:
    HbEffectScaleAnimation(HbEffectGroup *group, HbEffectScale *effect, int duration);
    void setCenter(qreal center);

private:
    void handleAnimationUpdate(const QVariant &value);
    void handleAnimationFinish();

public:
    HbEffectGroup *mGroup;
    HbEffectScale *mEffect;

    qreal mCenter;
    qreal mCurrentScaling;
};

#endif // HB_EFFECT_SCALE_ANIMATION_P_H

