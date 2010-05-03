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

#include <hbglobal.h>
#include "hbeffectanimation_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbeffectfxmldata_p.h"
#include <QGraphicsItem>
#include <QtDebug>

HbEffectAnimation::HbEffectAnimation(HbEffectGroup *group) :
    mGroup(group),
    mCurve(QEasingCurve::Linear),
    mLoopStart(-1),
    mLoopEnd(-1),
    mInactive(false),
    mFinished(false)
{
    connect(this, SIGNAL(finished()), this, SLOT(handleFinished()));
}

HbEffectAnimation::~HbEffectAnimation()
{
}

void HbEffectAnimation::addLooping(const HbEffectFxmlParamData *param)
{
    if (param) {
        if (param->loopDefined()) {
            int duration = QVariantAnimation::duration();

            int loopStart = (int)(param->loopStart() * float(duration));
            int loopEnd = (int)(param->loopEnd() * float(duration));

            // Limit given values to valid range
            loopStart = qBound(0, loopStart, duration);
            loopEnd = qBound(0, loopEnd, duration);

            // Add looping range to animation if loop end is after its start
            if (loopEnd > loopStart) {
                mLoopStart = loopStart;
                mLoopEnd = loopEnd;
                mGroup->setLooping(true);
            }
        }
    }
}

void HbEffectAnimation::updateCurrentValue(const QVariant &value)
{
    if (mInactive) {
        return;
    }

    // If there is a loop defined, check whether the current time of
    // the animation is past the loop end. If it is, do not update the
    // effect with that value but instead change the current time back
    // to (loopStartTime + currentTime - LoopEndTime).

    if (looping()) {
        int current = currentTime();

        if (current > mLoopEnd) {
            int newCurrentTime = mLoopStart + current - mLoopEnd;
            
            // If the calculated new current time is again beyond the loop end,
            // change it to the loop end to avoid infinite recursion.
            if (newCurrentTime > mLoopEnd) {
                newCurrentTime = mLoopEnd;
            }

            // Set the current time of the animation according to the defined loop,
            // it will create a callback to this function again.
            setCurrentTime(newCurrentTime);
            return;
        }
    }

    // Let the derived classes handle the value change.
    handleAnimationUpdate(value);
}

void HbEffectAnimation::handleFinished()
{
    // Restart loop if the animation is looping
    if (looping()) {
        int current = currentTime();
        int newCurrentTime = mLoopStart + current - mLoopEnd;
        
        // If the calculated new current time is again beyond the loop end,
        // change it to the loop end to avoid infinite recursion.
        if (newCurrentTime > mLoopEnd) {
            newCurrentTime = mLoopEnd;
        }
        
        // Temporarily prevent the animation from reacting to update request,
        // because start() causes update with time=0 and the loop might start from a later point of time.
        mInactive = true;
        // Restart the animation
        start();
        mInactive = false;
        // Set the current time of the animation according to the defined loop.
        setCurrentTime(newCurrentTime);
    }
    // Otherwise handle finish
    else {
        mFinished = true;
        // Create a callback to derived classes
        handleAnimationFinish();
    }
}

QVariant HbEffectAnimation::interpolated(const QVariant &from, const QVariant &to, qreal progress) const
{
    // Interpolation is handled here so that we can apply curve shape to each keyframe separately.
    // QVariantAnimation would apply the curve shape to the whole animation.

    qreal fromValue = qVariantValue<qreal>(from);
    qreal toValue = qVariantValue<qreal>(to);

    return fromValue + mCurve.valueForProgress(progress) * (toValue - fromValue);
}

// End of File
