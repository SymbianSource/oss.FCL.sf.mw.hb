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

#ifndef HBEFFECTBLUR_P_H
#define HBEFFECTBLUR_P_H

#include <hbglobal.h>
#ifdef HB_FILTER_EFFECTS

#include "hbeffectabstract_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbeffectfilter_p.h"
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QtAnimation;
class QTransform;
class QGraphicsItem;
QT_END_NAMESPACE

class HbVgBlurEffect;
class HbEffectFilterAnimation;
class HbEffectFxmlParamData;
class HbEffectGroup;

class HB_AUTOTEST_EXPORT HbEffectBlur : public HbEffectFilter
{
public:
    enum Type
    {
        Blur = 0x01,
        Glow = 0x02
    };

    HbEffectBlur(
        const HbEffectFxmlFilterData &data,
        QGraphicsItem *item,
        HbEffectGroup *group);

    virtual ~HbEffectBlur();

    QString name() const;
    void updateFilterEffect();

protected:
    HbEffectFilterAnimation *mAnimationO;
    HbEffectFilterAnimation *mAnimationX;
    HbEffectFilterAnimation *mAnimationY;

    HbVgBlurEffect *mVgBlur;
    Type mType;
};

#endif
#endif // HBEFFECTBLUR_P_H
