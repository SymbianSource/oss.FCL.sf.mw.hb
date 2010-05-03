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
#include "hbeffecttranslate_p.h"
#include "hbeffectanimation_p.h"
#include "hbeffectutils_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include <QGraphicsItem>
#include <QtDebug>

HbEffectTranslateAnimation::HbEffectTranslateAnimation(
    HbEffectGroup *group,
    HbEffectTranslate *effect,
    int duration) :
        HbEffectAnimation(group),
        effect(effect)
{
    setDuration(duration);
}

void HbEffectTranslateAnimation::setOriginalPos(qreal pos)
{
    originalPos = pos;
}

void HbEffectTranslateAnimation::handleAnimationUpdate(const QVariant &value)
{
    if (mGroup->isRunning()) {
        currentPos = qVariantValue<qreal>(value);
        mGroup->updateItemTransform();
    }
}

void HbEffectTranslateAnimation::handleAnimationFinish()
{
    // This informs the effect group when both translate_x and translate_y have finished
    effect->handleAnimationFinished();
}

// HbEffectTranslate

HbEffectTranslate::HbEffectTranslate(
    const QList<HbEffectFxmlParamData> &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectAbstract(0, item, group),
        mAnimationX(0),
        mAnimationY(0)
{
    // Default values if something is not passed in FXML
    int durationX = 0;
    int durationY = 0;

    QEasingCurve curveShapeX = QEasingCurve::Linear;
    QEasingCurve curveShapeY = QEasingCurve::Linear;

    const HbEffectFxmlParamData* paramX = 0;
    const HbEffectFxmlParamData* paramY = 0;

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, data) {
	    if (param.name() == FXML_KEYWORD_TRANSLATION_X) {
            mKeyFrameListX = param.keyFrames();

            HbEffectUtils::resolveFxmlDuration(durationX, param);
            HbEffectUtils::resolveFxmlCurveShape(curveShapeX, param);
			mStartXRef = param.startRef();
			mEndXRef = param.endRef();
			mStartX = param.getAttribute(FXML_KEYWORD_START);
			mEndX = param.getAttribute(FXML_KEYWORD_END);

            // Only if "start ref" and "end ref" are not used, read parameter's "ref" attribute
            if (mStartXRef.isEmpty() && mEndXRef.isEmpty()) {
                mParamRefX = param.getAttribute(FXML_PARAM_REF);
            }
            paramX = &param;
	    }
	    else if (param.name() == FXML_KEYWORD_TRANSLATION_Y) {
            mKeyFrameListY = param.keyFrames();

            HbEffectUtils::resolveFxmlDuration(durationY, param);
            HbEffectUtils::resolveFxmlCurveShape(curveShapeY, param);
			mStartYRef = param.startRef();
			mEndYRef = param.endRef();
			mStartY = param.getAttribute(FXML_KEYWORD_START);
			mEndY = param.getAttribute(FXML_KEYWORD_END);

            // Only if "start ref" and "end ref" are not used, read parameter's "ref" attribute
            if (mStartYRef.isEmpty() && mEndYRef.isEmpty()) {
                mParamRefY = param.getAttribute(FXML_PARAM_REF);
            }
            paramY = &param;
	    }
    }

    if (durationX > 0) {
        mAnimationX = new HbEffectTranslateAnimation(group, this, durationX);
        mAnimationX->addLooping(paramX);
        mAnimationX->mCurve = curveShapeX;
    }

    // Create at least one animation even if both durations are zero for handling animation finish easily.
    if (durationY > 0 || !mAnimationX) {
        mAnimationY = new HbEffectTranslateAnimation(group, this, durationY);
        mAnimationY->addLooping(paramY);
        mAnimationY->mCurve = curveShapeY;
    }
}

/*  Deletes the effect and rewinds the animation so that next animation can start,
    rewind is necessary as the general use case is that the effect is restarted before 
    being complete, in which case the rewind would transform the item and prepare for
    the next effect,otherwise would result in flicker.
*/
HbEffectTranslate::~HbEffectTranslate()
{
    if (mAnimationX) {
        mAnimationX->stop();
        delete mAnimationX;
    }
    if (mAnimationY) {
        mAnimationY->stop();
        delete mAnimationY;
    }
}

QString HbEffectTranslate::name() const
{
    return HB_EFFECT_NAME_TRANSLATE;
}

void HbEffectTranslate::init()
{
    // Resolve the dynamic attributes at this point

    bool valueOk;

    const QRectF &extRect = group()->extRect();

    // StartX
    qreal value = HbEffectUtils::resolveFxmlRef(
        mStartXRef, mStartX, &valueOk, item(), HbEffectUtils::Position, extRect);
    if (valueOk) {
        mStartXValue = value;
    } else {
        // Failed to resolve parameters, use item's current position.
        mStartXValue = item()->pos().x();
    }
    
    // StartY
    value = HbEffectUtils::resolveFxmlRef(
        mStartYRef, mStartY, &valueOk, item(), HbEffectUtils::Position, extRect);
    if (valueOk) {
        mStartYValue = value;
    } else {
        // Failed to resolve parameters, use item's current position.
        mStartYValue = item()->pos().y();
    }

    // EndX
    value = HbEffectUtils::resolveFxmlRef(
        mEndXRef, mEndX, &valueOk, item(), HbEffectUtils::Position, extRect);
    if (valueOk) {
        mEndXValue = value;
    } else {
        // Failed to resolve parameters, use item's current position.
        mEndXValue = item()->pos().x();
    }

    // EndY
    value = HbEffectUtils::resolveFxmlRef(
        mEndYRef, mEndY, &valueOk, item(), HbEffectUtils::Position, extRect);
    if (valueOk) {
        mEndYValue = value;
    } else {
        // Failed to resolve parameters, use item's current position.
        mEndYValue = item()->pos().y();
    }

    if (mAnimationX) {
        mAnimationX->setOriginalPos(item()->pos().x());
        mAnimationX->setStartValue(QVariant(mStartXValue));
        mAnimationX->setEndValue(QVariant(mEndXValue));

        bool startEndRefUsed = !mStartXRef.isEmpty() && !mEndXRef.isEmpty();

        // Set keyframes in animation
        foreach(const HbKeyFrame &kf, mKeyFrameListX) {
            // If start and end references are used,
            // value at given step is (1-c)*startX + c*endX where c is the keyframe coefficient value
            if (startEndRefUsed) {
                value = (1 - kf.val) * mStartXValue + kf.val * mEndXValue;
            }
            // Otherwise c defines the value
            else {
                value = HbEffectUtils::resolveFxmlRef(
                    mParamRefX, QString().setNum(kf.val), &valueOk, item(), HbEffectUtils::Position, extRect);
                if (!valueOk) {
                    // Default to item's position if failed
                    value = item()->pos().x();
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
        mAnimationY->setOriginalPos(item()->pos().y());
        mAnimationY->setStartValue(QVariant(mStartYValue));
        mAnimationY->setEndValue(QVariant(mEndYValue));

        bool startEndRefUsed = !mStartYRef.isEmpty() && !mEndYRef.isEmpty();

        // Set keyframes in animation
        foreach(const HbKeyFrame &kf, mKeyFrameListY) {
            // If start and end references are used,
            // value at given step is (1-c)*startY + c*endY where c is the keyframe coefficient value
            if (startEndRefUsed) {
                value = (1 - kf.val) * mStartYValue + kf.val * mEndYValue;
            }
            // Otherwise c defines the relative value to parameter's reference
            else {
                value = HbEffectUtils::resolveFxmlRef(
                    mParamRefY, QString().setNum(kf.val), &valueOk, item(), HbEffectUtils::Position, extRect);
                if (!valueOk) {
                    // Default to item's position if failed
                    value = item()->pos().y();
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

void HbEffectTranslate::setStartState(QTransform &transform)
{   
    qreal translateX = mAnimationX ? mStartXValue - mAnimationX->originalPos : 0;
    qreal translateY = mAnimationY ? mStartYValue - mAnimationY->originalPos : 0;

    QTransform newTransform;
    newTransform.translate(translateX, translateY);
    // Combine the new transform with the existing one
    transform *= newTransform;
}

/*  Starts the translate effect animation. If the effect is already started,
the result is undefined.
*/
void HbEffectTranslate::start()
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

/*  Cancels the effect animation and sets the animation end state immediately.
*/
void HbEffectTranslate::cancel(QTransform &transform, bool itemIsValid)
{
    Q_UNUSED(itemIsValid)

    if (mAnimationX) {
        mAnimationX->stop();
    }
    if (mAnimationY) {
        mAnimationY->stop();
    }

    qreal translateX = mAnimationX ? mEndXValue - mAnimationX->originalPos : 0;
    qreal translateY = mAnimationY ? mEndYValue - mAnimationY->originalPos : 0;

    // This will set the endstate for the object correctly when cancelling effect
    QTransform newTransform;
    newTransform.translate(translateX, translateY);
    // Combine the new transform with the existing one
    transform *= newTransform;
}

void HbEffectTranslate::updateItemTransform(QTransform &transform)
{
    qreal translateX = mAnimationX ? mAnimationX->currentPos - mAnimationX->originalPos : 0;
    qreal translateY = mAnimationY ? mAnimationY->currentPos - mAnimationY->originalPos : 0;

    QTransform newTransform;
    newTransform.translate(translateX, translateY);
    // Combine the new transform with the existing one
    transform *= newTransform;
}

void HbEffectTranslate::handleAnimationFinished()
{
    bool allFinished = true;

    if (mAnimationX && !mAnimationX->mFinished) {
        allFinished = false;
    }
    if (mAnimationY && !mAnimationY->mFinished) {
        allFinished = false;
    }

    // Inform the effect group that the whole translate effect has finished (both translate_x and translate_y)
    if (allFinished) {
        group()->effectFinished();
    }
}

void HbEffectTranslate::pause()
{
    if (mAnimationX) {
        mAnimationX->pause();
    }
    if (mAnimationY) {
        mAnimationY->pause();
    }
}

void HbEffectTranslate::resume()
{
    if (mAnimationX) {
        mAnimationX->resume();
    }
    if (mAnimationY) {
        mAnimationY->resume();
    }
}

