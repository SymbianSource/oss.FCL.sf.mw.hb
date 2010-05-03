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

#include "hbeffectrotate_p.h"
#include "hbeffectanimation_p.h"
#include "hbeffectutils_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include <QGraphicsItem>
#include <QVariantAnimation>
#include <QtDebug>

class HbEffectRotateAnimation : public HbEffectAnimation
{
public:
    HbEffectRotateAnimation(HbEffectGroup *group,
                            Qt::Axis axis,
                            int duration );

    void setCenter(const QPointF &p);

private:
    void handleAnimationUpdate(const QVariant &value);
    void handleAnimationFinish();

public:
    Qt::Axis mAxis;
    qreal mCenterX;
    qreal mCenterY;
    qreal mCurrentRotation;
    qreal mTargetRotation;
};

HbEffectRotateAnimation::HbEffectRotateAnimation(
    HbEffectGroup *group,
    Qt::Axis axis,
    int duration ) :
        HbEffectAnimation(group),
        mAxis(axis),
        mCenterX(0),
        mCenterY(0),
        mCurrentRotation(0),
        mTargetRotation(0)
{
    setDuration(duration);
}

void HbEffectRotateAnimation::setCenter(const QPointF &p)
{
    mCenterX = p.x();
    mCenterY = p.y();
}

void HbEffectRotateAnimation::handleAnimationUpdate(const QVariant &value)
{
    if (mGroup->isRunning()) {
        mCurrentRotation = qVariantValue<qreal>(value);
        mGroup->updateItemTransform();
    }
}

void HbEffectRotateAnimation::handleAnimationFinish()
{
    mGroup->effectFinished();
}

/*
    \class HbEffectRotate

    \brief HbEffectRotate is an internal class used by effect framework.
    HbEffectRotate is an internal class used by effect framework to get rotation effect.

    Example of how to use HbEffectRotate
    \code
    // animData here is pointer to HbEffectAnimation data, gruop is pointer to HbEffectGroup, and 
    // item is the QGraphicsItem on which effect will be applied.
    HbEffectRotate *effect= new HbEffectScale(animData, item, group);
    effect->start();
    
    \endcode

    \warning This class is a part of internal library implementation and may
    be removed or modified!

    \internal
*/

HbEffectRotate::HbEffectRotate(
    const QList<HbEffectFxmlParamData> &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectAbstract(0, item, group),
        mAnimation(0),
        mCenterX(0.0),
        mCenterY(0.0),
        mPosChanged(false)
{
    // Default values of rotation effect if something is not passed in FXML
    Qt::Axis axis = Qt::ZAxis;
    qreal startAngle = 0.0;
    qreal endAngle = 0.0;
    int duration = 0;

    QEasingCurve curve = QEasingCurve::Linear;

    QList<HbKeyFrame> keyFrameList;

    const HbEffectFxmlParamData *angleParam = 0;

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, data) {
	    if (param.name() == FXML_KEYWORD_ROTATION_ANGLE) {
            keyFrameList = param.keyFrames();
            HbEffectUtils::resolveFxmlDuration(duration, param);
            HbEffectUtils::resolveFxmlCurveShape(curve, param);
            angleParam = &param;
	    }
	    else if (param.name() == FXML_KEYWORD_ROTATION_ORIGIN_X) {
            originXData = param;
	    }
	    else if (param.name() == FXML_KEYWORD_ROTATION_ORIGIN_Y) {
            originYData = param;
	    }
	    else if (param.name() == FXML_KEYWORD_ROTATION_AXIS_X) {
            bool valueOk = false;
            int value = param.getValue().toInt(&valueOk);
            if (valueOk && (bool)value) {
                axis = Qt::XAxis;
            }
        }
	    else if (param.name() == FXML_KEYWORD_ROTATION_AXIS_Y) {
            bool valueOk = false;
            int value = param.getValue().toInt(&valueOk);
            if (valueOk && (bool)value) {
                axis = Qt::YAxis;
            }
        }
        // Z axis parameter can be skipped because it's the default value.
    }

    // Create rotation animation
    HbEffectRotateAnimation *anim = new HbEffectRotateAnimation(
        group, axis, duration);

    anim->addLooping(angleParam);

    foreach( const HbKeyFrame &kf, keyFrameList ) {
	    if (HbEffectUtils::fuzzyIsNull(kf.pos)) {
		    startAngle = kf.val;
	    }
	    else if (HbEffectUtils::fuzzyIsOneOrGreater(kf.pos)) {
		    endAngle = kf.val;
	    }
        // Set keyframe in animation
        else {
            anim->setKeyValueAt(kf.pos, QVariant(kf.val));
        }
    }

    anim->mTargetRotation = endAngle;
    anim->mCurve = curve;
    mAnimation = anim;
    anim->setStartValue(QVariant(startAngle));
    anim->setEndValue(QVariant(endAngle));
}

HbEffectRotate::~HbEffectRotate()
{
    if (mAnimation) {
        mAnimation->stop();
        delete mAnimation;
    }
}

QString HbEffectRotate::name() const
{
    return HB_EFFECT_NAME_ROTATE;
}

void HbEffectRotate::init()
{
    bool valueOk(false);
    qreal value = HbEffectUtils::resolveFxmlRef(originXData, &valueOk, item(), HbEffectUtils::Center);
    if (valueOk) {
        mCenterX = value;
    } else {
        // Default to bounding rect center
        mCenterX = item()->boundingRect().center().x();
    }

    value = HbEffectUtils::resolveFxmlRef(originYData, &valueOk, item(), HbEffectUtils::Center);
    if (valueOk) {
        mCenterY = value;
    } else {
        // Default to bounding rect center
        mCenterY = item()->boundingRect().center().y();
    }
}

void HbEffectRotate::setStartState(QTransform &transform)
{
    updateItemTransform(transform);
}

/* Starts the rotate effect animation. If the effect is already started,
the result is undefined.
*/
void HbEffectRotate::start()
{  
	mAnimation->setCenter(QPointF(mCenterX, mCenterY));
    mAnimation->setCurrentTime(0);
	mAnimation->start();
}

/* Cancels the effect animation and sets the animation end state immediately.
*/
void HbEffectRotate::cancel(QTransform &transform, bool itemIsValid)
{
    Q_UNUSED(itemIsValid)

    mAnimation->stop();
    
    //This will set the endstate for the object correctly when cancelling effect
    QTransform newTransform;
    newTransform.translate(mAnimation->mCenterX, mAnimation->mCenterY);
    newTransform.rotate(mAnimation->mTargetRotation, mAnimation->mAxis);
    newTransform.translate(-mAnimation->mCenterX, -mAnimation->mCenterY);
    // Combine the new transform with the existing one
    transform *= newTransform;
}

void HbEffectRotate::updateItemTransform(QTransform &transform)
{
    if (mAnimation) {
        // Rotate the transformation matrix to reach the new rotation angle.        
        QTransform newTransform;
        newTransform.translate(mAnimation->mCenterX, mAnimation->mCenterY);
        newTransform.rotate(mAnimation->mCurrentRotation, mAnimation->mAxis);
        newTransform.translate(-mAnimation->mCenterX, -mAnimation->mCenterY);
        // Combine the new transform with the existing one
        transform *= newTransform;
    }
}

void HbEffectRotate::pause()
{
    if (mAnimation) {
        mAnimation->pause();
    }
}

void HbEffectRotate::resume()
{
    if (mAnimation) {
        mAnimation->resume();
    }
}


