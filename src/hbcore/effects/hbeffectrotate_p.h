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

#ifndef HB_EFFECT_ROTATE_P_H
#define HB_EFFECT_ROTATE_P_H

#include <QtGlobal>
#include "hbeffectabstract_p.h"
#include "hbeffectfxmldata_p.h"

QT_BEGIN_NAMESPACE
class QtAnimation;
class QTransform;
QT_END_NAMESPACE

class HbEffectAnimationData;
class HbEffectRotateAnimation;

class HB_AUTOTEST_EXPORT HbEffectRotate : public HbEffectAbstract
{
public:
    HbEffectRotate(
        const QList<HbEffectFxmlParamData> &data,
        QGraphicsItem *item,
        HbEffectGroup *group);

    virtual ~HbEffectRotate();

    QString name() const;
    void init();
    void setStartState(QTransform &transform);
    void start();
    void cancel(QTransform &transform, bool itemIsValid);
    void updateItemTransform(QTransform &transform);
    void pause();
    void resume();

protected:
    HbEffectRotateAnimation *mAnimation;

    qreal mCenterX;
    qreal mCenterY;
    bool mPosChanged;

    HbEffectFxmlParamData originXData;
    HbEffectFxmlParamData originYData;


};

#endif // HB_EFFECT_ROTATE_P_H

