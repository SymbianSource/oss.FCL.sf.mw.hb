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

#ifndef HBEFFECTDROPSHADOW_H
#define HBEFFECTDROPSHADOW_H

#include <hbglobal.h>
#ifdef HB_FILTER_EFFECTS

#include "hbeffectabstract_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbeffectfilter_p.h"
#include <QtGlobal>
#include <QVariantAnimation>
#include <QGraphicsEffect>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QtAnimation;
class QTransform;
QT_END_NAMESPACE

class HbEffectDropShadow;
class HbVgOutlineEffect;
class HbEffectAnimationData;

/**
* This effect class implements both drop_shadow and outline filter effects.
* If translation (offset) parameters are defined, it is a drop_shadow effect,
* otherwise it is an outline effect.
*/
class HB_AUTOTEST_EXPORT HbEffectDropShadow : public HbEffectFilter
{
public:

    enum type {
        dropShadow = 0x01,
        outline = 0x02
    };
    HbEffectDropShadow(
        const HbEffectFxmlFilterData &data,
        QGraphicsItem *item,
        HbEffectGroup *group);

    virtual ~HbEffectDropShadow();

    QString name() const;
    void updateFilterEffect();

protected:
    HbEffectFilterAnimation *mAnimationO;           // opacity
    HbEffectFilterAnimation *mAnimationS;           // steepness
    HbEffectFilterAnimation *mAnimationOutlineX;    // outline_x
    HbEffectFilterAnimation *mAnimationOutlineY;    // outline_y
    HbEffectFilterAnimation *mAnimationOffsetX;     // offset_x
    HbEffectFilterAnimation *mAnimationOffsetY;     // offset_y
    HbEffectColorAnimation *mAnimationC;            // color

    QPointer<HbVgOutlineEffect> mVgOutline;
    int mType;
};

#endif
#endif // HBEFFECTDROPSHADOW_H

