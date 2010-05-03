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
#include "hbeffectopacity_p.h"
#include "hbeffectutils_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include <QGraphicsItem>
#include <QRectF>
#include <QtDebug>

HbEffectOpacityAnimation::HbEffectOpacityAnimation(
    HbEffectGroup *group,
    int duration ) :
        HbEffectAnimation(group)
{
    setDuration(duration);
}

void HbEffectOpacityAnimation::handleAnimationUpdate(const QVariant &value)
{
    qreal opacity = qVariantValue<qreal>(value);
    mGroup->targetItem()->setOpacity(opacity);
}

void HbEffectOpacityAnimation::handleAnimationFinish()
{
    mGroup->effectFinished();
}

// HbEffectOpacity

HbEffectOpacity::HbEffectOpacity(
    const QList<HbEffectFxmlParamData> &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectAbstract(0, item, group),
        mAnimation(0)
{
    // Default values of if something is not passed in FXML
    mStartOpacity = 1.0f;
    mEndOpacity = 1.0f;

    int duration = 0;

    QEasingCurve curve = QEasingCurve::Linear;
    QList<HbKeyFrame> keyFrameList;

    const HbEffectFxmlParamData *opacityParam = 0;

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, data) {
	    if (param.name() == FXML_KEYWORD_OPACITY) {
            keyFrameList = param.keyFrames();
            HbEffectUtils::resolveFxmlDuration(duration, param);
            HbEffectUtils::resolveFxmlCurveShape(curve, param);

            // Get static value
            param.getValue(mStartOpacity);
            mEndOpacity = mStartOpacity;

            opacityParam = &param;
	    }
    }

    if (duration > 0) {
        mAnimation = new HbEffectOpacityAnimation(group, duration); 

        foreach( const HbKeyFrame &kf, keyFrameList ) {
	        if (HbEffectUtils::fuzzyIsNull(kf.pos)) {
		        // Start value 0.0 does not work so this trick fixes that.
                mStartOpacity = kf.val + 0.0001f;
	        }
	        else if (HbEffectUtils::fuzzyIsOneOrGreater(kf.pos)) {
		        mEndOpacity = kf.val;
	        }
            // Set keyframe in animation
            else {
                mAnimation->setKeyValueAt(kf.pos, QVariant(kf.val));
            }
        }

        mAnimation->addLooping(opacityParam);
        mAnimation->setEndValue(QVariant(mEndOpacity));
        mAnimation->setStartValue(QVariant(mStartOpacity));
        mAnimation->mCurve = curve;
    }
}

HbEffectOpacity::~HbEffectOpacity()
{
#if QT_VERSION >= 0x040500
    // Item might be deleted at this point, so it cannot be used here
    //item()->setOpacity(mEndOpacity);//performing cancel:bug fix
#endif // QT_VERSION

    delete mAnimation;
}

QString HbEffectOpacity::name() const
{
    return HB_EFFECT_NAME_OPACITY;
}

void HbEffectOpacity::init()
{
}

void HbEffectOpacity::setStartState(QTransform &/*transform*/)
{
#if !(QT_VERSION < 0x040500)
    item()->setOpacity(mStartOpacity);
#endif
}

/* Starts the opacity effect animation. If the effect is already started,
the result is undefined.
*/
void HbEffectOpacity::start()
{
#if !(QT_VERSION < 0x040500)
    item()->setOpacity(mStartOpacity);
#endif

    if (mAnimation) {
        mAnimation->stop();
        mAnimation->setCurrentTime(0);
        mAnimation->start();
    }
    // Static effect
    else {
        // Handle finish immediately, effect has been set in setStartState().
        Hb::EffectEvent reason = Hb::EffectNotStarted;
        group()->effectFinished(reason);
    }
}

/* Cancels the effect animation and sets the animation end state immediately.
*/
void HbEffectOpacity::cancel(QTransform &transform, bool itemIsValid)
{
    Q_UNUSED(transform)

    if (mAnimation) {
        mAnimation->stop();
    }

#if QT_VERSION >= 0x040500
    if (itemIsValid) {
        item()->setOpacity(mEndOpacity);
    }
#endif // QT_VERSION
}

void HbEffectOpacity::updateItemTransform(QTransform &/*transform*/)
{
    // Nothing needed because opacity does not use a transform.
}

void HbEffectOpacity::pause()
{
    if (mAnimation) {
        mAnimation->pause();
    }
}

void HbEffectOpacity::resume()
{
    if (mAnimation) {
        mAnimation->resume();
    }
}
