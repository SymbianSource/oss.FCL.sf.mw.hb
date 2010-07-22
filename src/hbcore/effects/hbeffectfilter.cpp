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
#ifdef HB_FILTER_EFFECTS

#include "hbeffectfilter_p.h" //krazy:exclude=includes
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbeffectutils_p.h"
#include "hbeffectfxmldata_p.h"
#include <QGraphicsItem>
#include <QtDebug>

HbEffectFilterAnimation::HbEffectFilterAnimation(HbEffectFilter *effect,
        int duration,
        HbEffectGroup *group)
    : HbEffectAnimation(group),
      mEffect(effect)
{
    setDuration(duration);
}

HbEffectFilterAnimation::~HbEffectFilterAnimation()
{
}

void HbEffectFilterAnimation::handleAnimationUpdate(const QVariant &value)
{
    Q_UNUSED(value);

    // Effect instance updates the vgEffect for all the filter animations
    mEffect->updateFilterEffect();
}

void HbEffectFilterAnimation::handleAnimationFinish()
{
    // This informs the effect group when all the animations have finished
    mEffect->handleAnimationFinished();
}

HbEffectColorAnimation::HbEffectColorAnimation(HbEffectFilter *effect, int duration, HbEffectGroup *group)
    : HbEffectFilterAnimation(effect, duration, group)
{
}

HbEffectColorAnimation::~HbEffectColorAnimation()
{
}

QVariant HbEffectColorAnimation::interpolated(const QVariant &from, const QVariant &to, qreal progress) const
{
    QColor ret;
    // Interpolate R,G,B and A attributes of the color separately
    QColor fromColor = qVariantValue<QColor>(from);
    QColor toColor = qVariantValue<QColor>(to);

    int fromValue = fromColor.red();
    int toValue = toColor.red();
    ret.setRed(fromValue + (int)(mCurve.valueForProgress(progress) *(toValue - fromValue)));

    fromValue = fromColor.green();
    toValue = toColor.green();
    ret.setGreen(fromValue + (int)(mCurve.valueForProgress(progress) *(toValue - fromValue)));

    fromValue = fromColor.blue();
    toValue = toColor.blue();
    ret.setBlue(fromValue + (int)(mCurve.valueForProgress(progress) *(toValue - fromValue)));

    fromValue = fromColor.alpha();
    toValue = toColor.alpha();
    ret.setAlpha(fromValue + (int)(mCurve.valueForProgress(progress) *(toValue - fromValue)));

    return ret;
}

// -----------------------------------------------------------------------------

// HbEffectFilter

HbEffectFilter::HbEffectFilter(int /*startTime*/, QGraphicsItem *item, HbEffectGroup *group)
    : HbEffectAbstract(0, item, group),
      mEffectDefined(false),
      mCanceled(false)
{
}

HbEffectFilter::~HbEffectFilter()
{
    Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
        anim->stop();
        delete anim;
    }
}

void HbEffectFilter::init()
{
}

void HbEffectFilter::setStartState(QTransform &/*transform*/)
{
}

void HbEffectFilter::start()
{
    mCanceled = false;

    // Activate openvg effect in the graphics item if there is an effect defined
    if (mEffectDefined) {
        group()->activateVgEffect();
    }

    if (mAnimations.count()) {
        Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
            anim->stop(); // rewind the animation before starting again
            anim->setCurrentTime(0);
            anim->mFinished = false;
            anim->start();
        }
    }
    // If there are no animations, handle finish immediately. This informs the effect group.
    else {
        handleAnimationFinished();
    }
}

/* Cancels the effect animation and sets the animation end state immediately.
*/
void HbEffectFilter::cancel(QTransform &transform, bool itemIsValid)
{
    Q_UNUSED(transform)
    Q_UNUSED(itemIsValid)

    mCanceled = true;

    // Put animations in their end state to reach the end state of the effect.
    Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
        anim->stop();
        anim->setCurrentTime(anim->duration());
        anim->mFinished = true;
    }
    // Update the filter effect to reflect the end state of the effect
    if (mAnimations.count()) {
        // This updates all the animated values
        mAnimations.at(0)->mEffect->updateFilterEffect();
    }
}

void HbEffectFilter::updateItemTransform(QTransform &/*transform*/)
{
    // Not a transform effect so nothing here
}

void HbEffectFilter::pause()
{
    Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
        if (anim->state() == QAbstractAnimation::Running) {
            anim->pause();
        }
    }
}

void HbEffectFilter::resume()
{
    Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
        if (anim->state() == QAbstractAnimation::Paused) {
            anim->resume();
        }
    }
}

HbEffectFilterAnimation *HbEffectFilter::createAnimation(
    const HbEffectFxmlParamData &param,
    qreal &startValue,
    qreal &endValue,
    HbEffectGroup *group)
{
    HbEffectFilterAnimation *anim = 0;

    // Replace default values with parameter's values
    if (param.getValue(startValue)) {
        mEffectDefined = true;
    }
    endValue = startValue;

    QList<HbKeyFrame> keyFrameList = param.keyFrames();

    // duration and curvepath
    int duration = 0;
    HbEffectUtils::resolveFxmlDuration(duration, param);

    QEasingCurve curve = QEasingCurve::Linear;
    HbEffectUtils::resolveFxmlCurveShape(curve, param);

    // Create animation if the duration was defined
    if (duration > 0) {
        mEffectDefined = true;
        anim = new HbEffectFilterAnimation(this, duration, group);
        mAnimations.append(anim);
        // Go through keyframes
        foreach(const HbKeyFrame & kf, keyFrameList) {
            if (HbEffectUtils::fuzzyIsNull(kf.pos)) {
                startValue = kf.val;
            } else if (HbEffectUtils::fuzzyIsOneOrGreater(kf.pos)) {
                endValue = kf.val;
            }
            // Set keyframe in animation
            else {
                anim->setKeyValueAt(kf.pos, QVariant(kf.val));
            }
        }

        anim->setStartValue(QVariant(startValue));
        anim->setEndValue(QVariant(endValue));
        anim->mCurve = curve;

        // Get loop parameters
        anim->addLooping(&param);
    }

    // Return animation instance or NULL if not created
    return anim;
}

HbEffectColorAnimation *HbEffectFilter::createAnimation(
    const HbEffectFxmlParamData &param,
    QColor &startValue,
    QColor &endValue,
    HbEffectGroup *group)
{
    HbEffectColorAnimation *anim = 0;

    // Replace default values with parameter's values
    QString colorString = param.getValue();
    if (!colorString.isEmpty()) {
        startValue.setNamedColor(colorString);
        endValue = startValue;
        if (startValue.isValid()) {
            mEffectDefined = true;
        }
    }

    QList<HbKeyFrame> keyFrameList = param.keyFrames();

    // duration and curvepath
    int duration = 0;
    HbEffectUtils::resolveFxmlDuration(duration, param);

    QEasingCurve curve = QEasingCurve::Linear;
    HbEffectUtils::resolveFxmlCurveShape(curve, param);

    // Create animation if the duration was defined
    if (duration > 0) {
        mEffectDefined = true;
        anim = new HbEffectColorAnimation(this, duration, group);
        mAnimations.append(anim);
        // Go through keyframes
        foreach(const HbKeyFrame & kf, keyFrameList) {
            if (HbEffectUtils::fuzzyIsNull(kf.pos)) {
                startValue.setNamedColor(kf.stringValue);
            } else if (HbEffectUtils::fuzzyIsOneOrGreater(kf.pos)) {
                endValue.setNamedColor(kf.stringValue);
            }
            // Set keyframe in animation
            else {
                QColor c(kf.stringValue);
                anim->setKeyValueAt(kf.pos, QVariant(c));
            }
        }

        anim->setStartValue(QVariant(startValue));
        anim->setEndValue(QVariant(endValue));
        anim->mCurve = curve;

        // Get loop parameters
        anim->addLooping(&param);
    }

    // Return animation instance or NULL if not created
    return anim;
}

void HbEffectFilter::handleAnimationFinished()
{
    bool allFinished = true;

    Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
        if (!anim->mFinished) {
            allFinished = false;
        }
    }

    // Inform the effect group that the whole filter effect has finished.
    // If the effect was canceled, the effect group already takes care of finishing the effect.
    if (allFinished && !mCanceled) {
        // Make sure animations are in their end value to reach the end state of the effect.
        // Sometimes QVariantAnimation does not update the animation with its end value before sending finished signal.
        Q_FOREACH(HbEffectFilterAnimation * anim, mAnimations) {
            anim->stop();
            anim->setCurrentTime(anim->duration());
        }

        Hb::EffectEvent reason = mAnimations.count() ? Hb::EffectFinished : Hb::EffectNotStarted;
        group()->effectFinished(reason);
    }
}

#endif
