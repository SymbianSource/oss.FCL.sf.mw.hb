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
#include "hbeffectscale_p.h"
#include "hbeffectanimation_p.h"
#include "hbeffectutils_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectscaleanimation_p.h"
#include "hbeffectdef_p.h"
#include <QGraphicsItem>
#include <QtDebug>
#include <QVariantAnimation>

HbEffectScaleAnimation::HbEffectScaleAnimation(
    HbEffectGroup *group,
    HbEffectScale *effect,
    int duration ) :
        HbEffectAnimation(group),
        mEffect(effect),
        mCenter(0),
        mCurrentScaling(1.0)
{
    setDuration(duration);
}

void HbEffectScaleAnimation::setCenter(qreal center)
{
    mCenter = center;
}

void HbEffectScaleAnimation::handleAnimationUpdate(const QVariant &value)
{
    if (mGroup->isRunning()) {
        mCurrentScaling = qVariantValue<qreal>(value);
        // Prevent negative scale factors, they crash QT.
        if (mCurrentScaling < 0) {
            mCurrentScaling = 0;
        }
        mGroup->updateItemTransform();
    }
}

void HbEffectScaleAnimation::handleAnimationFinish()
{
    // This informs the effect group when both scale_x and scale_y have finished
    mEffect->handleAnimationFinished();
}

// -----------------------------------------------------------------------------

// HbEffectScale

HbEffectScale::HbEffectScale(
    const QList<HbEffectFxmlParamData> &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectAbstract(0, item, group),
        mAnimationX(0),
        mAnimationY(0)
{
    int durationX = 0;
    int durationY = 0;
    QEasingCurve curveX = QEasingCurve::Linear;
    QEasingCurve curveY = QEasingCurve::Linear;

    // Default values if not defined in the data. Value 1.0f means the item's "identity" i.e. its normal size.
    mStartXValue = 1.0f;
    mEndXValue = 1.0f;
    mStartYValue = 1.0f;
    mEndYValue = 1.0f;
    mCenterXValue = 0;
    mCenterYValue = 0;

    const HbEffectFxmlParamData* paramX = 0;
    const HbEffectFxmlParamData* paramY = 0;

    Q_FOREACH(const HbEffectFxmlParamData &param, data) {
	    if (param.name() == FXML_KEYWORD_SCALE_X) {
            mKeyFrameListX = param.keyFrames();
			// <start> for scale_x
			mStartWidth =  param.getAttribute(FXML_KEYWORD_START);
			mStartWidthRef = param.startRef();
			// <end> for scale_x
			mEndWidth = param.getAttribute(FXML_KEYWORD_END);
			mEndWidthRef = param.endRef();
			// duration and curvepath
            HbEffectUtils::resolveFxmlDuration(durationX, param);
            HbEffectUtils::resolveFxmlCurveShape(curveX, param);

            // Only if "start ref" and "end ref" are not used, read parameter's "ref" attribute
            if (mStartWidthRef.isEmpty() && mEndWidthRef.isEmpty()) {
                mParamRefX = param.getAttribute(FXML_PARAM_REF);
            }

            paramX = &param;
	    }
	    else if (param.name() == FXML_KEYWORD_SCALE_Y) {
            mKeyFrameListY = param.keyFrames();
			// <start> for scale_y
			mStartHeight =  param.getAttribute(FXML_KEYWORD_START);
			mStartHeightRef = param.startRef();
			// <end> for scale_y
			mEndHeight = param.getAttribute(FXML_KEYWORD_END);
			mEndHeightRef = param.endRef();
            HbEffectUtils::resolveFxmlDuration(durationY, param);
            HbEffectUtils::resolveFxmlCurveShape(curveY, param);

            // Only if "start ref" and "end ref" are not used, read parameter's "ref" attribute
            if (mStartHeightRef.isEmpty() && mEndHeightRef.isEmpty()) {
                mParamRefY = param.getAttribute(FXML_PARAM_REF);
            }

            paramY = &param;
	    }
	    else if (param.name() == FXML_KEYWORD_SCALE_ORIGIN_X) {
            mCenterX = param.getValue();
            mCenterXRef = param.getAttribute(FXML_PARAM_REF);
        }                        
	    else if (param.name() == FXML_KEYWORD_SCALE_ORIGIN_Y) {
            mCenterY = param.getValue();
            mCenterYRef = param.getAttribute(FXML_PARAM_REF);
	    }
    }

    // Validate references. If start and end references are used, at least one of them must be a visual reference.
    if ((mStartWidthRef.isEmpty() || mStartWidthRef.startsWith(FXML_VISUAL) ||
         mEndWidthRef.isEmpty() || mEndWidthRef.startsWith(FXML_VISUAL)) &&
        (mStartHeightRef.isEmpty() || mStartHeightRef.startsWith(FXML_VISUAL) ||
         mEndHeightRef.isEmpty() || mEndHeightRef.startsWith(FXML_VISUAL))) {
        // references ok
    } else {
        // Invalid references - disable effect
        durationX = 0;
        durationY = 0;

        mParamRefX.clear();
        mStartWidthRef.clear();
        mStartWidth.clear();
        mEndWidthRef.clear();
        mEndWidth.clear();

        mParamRefY.clear();
        mStartHeightRef.clear();
        mStartHeight.clear();
        mEndHeightRef.clear();
        mEndHeight.clear();

        mCenterX.clear();
        mCenterXRef.clear();
        mCenterY.clear();
        mCenterYRef.clear();
    }

    if (durationX > 0) {
        mAnimationX = new HbEffectScaleAnimation(group, this, durationX);
        mAnimationX->addLooping(paramX);
        mAnimationX->mCurve = curveX;
    }
    // Create at least one animation even if both durations are zero for handling animation finish easily.
    if (durationY > 0 || !mAnimationX) {
        mAnimationY = new HbEffectScaleAnimation(group, this, durationY);
        mAnimationY->addLooping(paramY);
        mAnimationY->mCurve = curveY;
    }
}


/* deletes the effect and rewinds the animation so that next animation can star,
    rewind is necessary as the general use case is that the effect is restarted before 
    being complete, in which case the rewind would transform the item and prepare for
    the next effect.
*/
HbEffectScale::~HbEffectScale()
{
    if (mAnimationX) {
        mAnimationX->stop();
        // Applying the cancel policy so that if dynamically new effect is started HbEffect::start then this finishes
        // and the new one starts.
        // This will set the endstate for the object correctly when destroying effect
        delete mAnimationX;
    }
    if (mAnimationY) {
        mAnimationY->stop();
        delete mAnimationY;
    }
}

QString HbEffectScale::name() const
{
    return HB_EFFECT_NAME_SCALE;
}

void HbEffectScale::init()
{
    QSizeF orgSize = item()->boundingRect().size();

    // Resolve Dynamic attributes at this point
    bool valueOk;
    qreal value;

    const QRectF &extRect = group()->extRect();

    // start width
    if (!mStartWidth.isEmpty()) {
        value = HbEffectUtils::resolveFxmlRef(
            mStartWidthRef, mStartWidth, &valueOk, item(), HbEffectUtils::Size, extRect);
        if (valueOk) {
            mStartXValue = value;
        }
    }
    // start height
    if (!mStartHeight.isEmpty()) {
        value = HbEffectUtils::resolveFxmlRef(mStartHeightRef, mStartHeight, &valueOk, item(), HbEffectUtils::Size, extRect);
        if (valueOk) {
            mStartYValue = value;
        }
    }

    // end width
    if (!mEndWidth.isEmpty()) {
        value = HbEffectUtils::resolveFxmlRef(
            mEndWidthRef, mEndWidth, &valueOk, item(), HbEffectUtils::Size, extRect);

        if (valueOk) {
            mEndXValue = value;
        }
    }
    // end height
    if (!mEndHeight.isEmpty()) {
        value = HbEffectUtils::resolveFxmlRef(
            mEndHeightRef, mEndHeight, &valueOk, item(), HbEffectUtils::Size, extRect);

        if (valueOk) {
            mEndYValue = value;
        }
    }

    // CenterX
    value = HbEffectUtils::resolveFxmlRef(
        mCenterXRef, mCenterX, &valueOk, item(), HbEffectUtils::CenterMappedToTargetRect, extRect);
    
    if (valueOk) {
        mCenterXValue = value;
    }
    // CenterY
    value = HbEffectUtils::resolveFxmlRef(
        mCenterYRef, mCenterY, &valueOk, item(), HbEffectUtils::CenterMappedToTargetRect, extRect);

    if (valueOk) {
        mCenterYValue = value;
    }

    // QT's scale animations do not handle 0 value correctly so change it here.
    if (HbEffectUtils::fuzzyIsNull(mStartXValue)) {
        mStartXValue = 0.00002;
    }
    if (HbEffectUtils::fuzzyIsNull(mStartYValue)) {
        mStartYValue = 0.00002;
    }
    if (HbEffectUtils::fuzzyIsNull(mEndXValue)) {
        mEndXValue = 0.00002;
    }
    if (HbEffectUtils::fuzzyIsNull(mEndYValue)) {
        mEndYValue = 0.00002;
    }

    if (mAnimationX) {
        // Start and end values are relative to item's size so value 1 means normal size
        mAnimationX->setStartValue(QVariant(mStartXValue));
        mAnimationX->setEndValue(QVariant(mEndXValue));
        // Center value is in pixels
        mAnimationX->setCenter(mCenterXValue);

        bool startEndRefUsed = !mStartWidthRef.isEmpty() && !mEndWidthRef.isEmpty();

        qreal paramRefValueX = 0.0;
        
        // Resolve SCALE_X parameter's "ref" value only if that's needed
        if (!startEndRefUsed) {
            paramRefValueX = HbEffectUtils::resolveFxmlRef(
                mParamRefX, "1", &valueOk, item(), HbEffectUtils::Size, extRect);
        }

        // Set keyframes in animation
        foreach(const HbKeyFrame &kf, mKeyFrameListX) {

            // If start and end references are used,
            // value at given step is (1-c)*startX + c*endX where c is the keyframe coefficient value
            if (startEndRefUsed) {
                value = (1 - kf.val) * mStartXValue + kf.val * mEndXValue;
            }
            // Otherwise c defines the value
            else {
                value = kf.val;
                // Multiply keyframe's value with parameter's "ref" if it is defined
                if (valueOk) {
                    value *= paramRefValueX;
                }
            }

            mAnimationX->setKeyValueAt(kf.pos, QVariant(value));

            // Update also stored start and end values if keyframe refers to those
            if (HbEffectUtils::fuzzyIsNull(kf.pos)) {
                mStartXValue = value;
            } else if (HbEffectUtils::fuzzyIsOneOrGreater(kf.pos)) {
                mEndXValue = value;
            }
        }
    }
    if (mAnimationY) {
        // Start and end values are relative to item's size so value 1 means normal size
        mAnimationY->setStartValue(QVariant(mStartYValue));
        mAnimationY->setEndValue(QVariant(mEndYValue));
        // Center value is in pixels
        mAnimationY->setCenter(mCenterYValue);

        bool startEndRefUsed = !mStartHeightRef.isEmpty() && !mEndHeightRef.isEmpty();

        qreal paramRefValueY = 0.0;
        // Resolve SCALE_Y parameter's "ref" value only if that's needed
        if (!startEndRefUsed) {
            paramRefValueY = HbEffectUtils::resolveFxmlRef(
                mParamRefY, "1", &valueOk, item(), HbEffectUtils::Size, extRect);
        }

        // Set keyframes in animation
        foreach(const HbKeyFrame &kf, mKeyFrameListY) {
            // If start and end references are used,
            // value at given step is (1-c)*startY + c*endY where c is the keyframe coefficient value
            if (startEndRefUsed) {
                value = (1 - kf.val) * mStartYValue + kf.val * mEndYValue;
            }
            // Otherwise c defines the value
            else {
                value = kf.val;
                // Multiply keyframe's value with parameter's "ref" if it is defined
                if (valueOk) {
                    value *= paramRefValueY;
                }
            }

            mAnimationY->setKeyValueAt(kf.pos, QVariant(value));

            // Update also stored start and end values if keyframe refers to those
            if (HbEffectUtils::fuzzyIsNull(kf.pos)) {
                mStartYValue = value;
            } else if (HbEffectUtils::fuzzyIsOneOrGreater(kf.pos)) {
                mEndYValue = value;
            }
        }
    }
}

void HbEffectScale::setStartState(QTransform &transform)
{
    // Set the start state
    updateItemTransform(transform);
}

/* Starts the scale effect animation. If the effect is already started,
the result is undefined.
*/
void HbEffectScale::start()
{
    if (mAnimationX) {
        mAnimationX->stop(); // rewind the animation before starting again
        mAnimationX->setCurrentTime(0);
        mAnimationX->mFinished = false;
        mAnimationX->start();
    }
    if (mAnimationY) {
        mAnimationY->stop(); // rewind the animation before starting again
        mAnimationY->setCurrentTime(0);
        mAnimationY->mFinished = false;
        mAnimationY->start();
    }
}

/* Cancels the effect animation and sets the animation end state immediately.
*/
void HbEffectScale::cancel(QTransform &transform, bool itemIsValid)
{
    Q_UNUSED(itemIsValid)

    if (mAnimationX) {
        mAnimationX->stop();
    }
    if (mAnimationY) {
        mAnimationY->stop();
    }

    // This will set the endstate for the object correctly when cancelling effect
    QTransform newTransform;
    newTransform.translate(mCenterXValue, mCenterYValue);
    newTransform.scale(mEndXValue, mEndYValue);
    newTransform.translate(-mCenterXValue, -mCenterYValue);
    // Combine the new transform with the existing one
    transform *= newTransform;
}

void HbEffectScale::updateItemTransform(QTransform &transform)
{
    QTransform newTransform;
    // Handle centering for scaling the matrix
    newTransform.translate(mCenterXValue, mCenterYValue);
    
    // Get the current scaling factor from animation or use 1.0 if animation does not exist
    qreal currentScalingX = mAnimationX ? mAnimationX->mCurrentScaling : 1.0;
    qreal currentScalingY = mAnimationY ? mAnimationY->mCurrentScaling : 1.0;

    // Scale the transformation matrix to reach the new scaling factor.
    newTransform.scale(currentScalingX, currentScalingY);
    // Centering must be canceled after the scaling has been done in the matrix
    newTransform.translate(-mCenterXValue, -mCenterYValue);
    // Combine the new transform with the existing one
    transform *= newTransform;
}

void HbEffectScale::handleAnimationFinished()
{
    bool allFinished = true;

    if (mAnimationX && !mAnimationX->mFinished) {
        allFinished = false;
    }
    if (mAnimationY && !mAnimationY->mFinished) {
        allFinished = false;
    }

    // Inform the effect group that the whole scale effect has finished (both scaleX and scaleY)
    if (allFinished) {
        group()->effectFinished();
    }
}

void HbEffectScale::pause()
{
    if (mAnimationX) {
        mAnimationX->pause();
    }
    if (mAnimationY) {
        mAnimationY->pause();
    }
}

void HbEffectScale::resume()
{
    if (mAnimationX) {
        mAnimationX->resume();
    }
    if (mAnimationY) {
        mAnimationY->resume();
    }
}
