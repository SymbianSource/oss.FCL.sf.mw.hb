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

#ifndef HB_EFFECT_TRANSLATE_P_H
#define HB_EFFECT_TRANSLATE_P_H

#include "hbeffectabstract_p.h"
#include "hbeffectanimation_p.h"
#include "hbeffectfxmldata_p.h"
#include <QEasingCurve>
#include <QVariantAnimation>
#include <QString>
#include <QPointF>

QT_BEGIN_NAMESPACE
class QtAnimation;
class QTransform;
QT_END_NAMESPACE

class HbEffectAnimationData;
class HbEffectTranslate;

class HB_AUTOTEST_EXPORT HbEffectTranslateAnimation : public HbEffectAnimation
{
public:
    HbEffectTranslateAnimation(HbEffectGroup *group, HbEffectTranslate *effect, int duration);
    void setOriginalPos(qreal pos);

private:
    void handleAnimationUpdate(const QVariant &value);
    void handleAnimationFinish();

public:
    qreal originalPos;
    qreal currentPos;
    HbEffectGroup *group;
    HbEffectTranslate *effect;
};


class HB_AUTOTEST_EXPORT HbEffectTranslate : public HbEffectAbstract
{
public:
    HbEffectTranslate(
        const QList<HbEffectFxmlParamData> &data,
        QGraphicsItem *item,
        HbEffectGroup *group);

    virtual ~HbEffectTranslate();

    QString name() const;
    void init();
    void setStartState(QTransform &transform);
    void start();
    void cancel(QTransform &transform, bool itemIsValid);
    void updateItemTransform(QTransform &transform);
    void pause();
    void resume();

    void handleAnimationFinished();

private:
    HbEffectTranslateAnimation *mAnimationX;
    HbEffectTranslateAnimation *mAnimationY;

    QString mStartX;
    QString mStartY;
    QString mEndX;
    QString mEndY;
    // Start and end ref values
    QString mStartXRef;
    QString mStartYRef;
    QString mEndXRef;
    QString mEndYRef;
    // Parameter's ref values
    QString mParamRefX;
    QString mParamRefY;

    QList<HbKeyFrame> mKeyFrameListX;
    QList<HbKeyFrame> mKeyFrameListY;

    // Resolved from string values when init() is called.
    qreal mStartXValue;
    qreal mStartYValue;
    qreal mEndXValue;
    qreal mEndYValue;
};


#endif // HB_EFFECT_TRANSLATE_P_H

