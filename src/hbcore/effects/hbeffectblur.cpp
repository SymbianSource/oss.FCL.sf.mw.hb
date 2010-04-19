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

#include "hbeffectblur_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbvgchainedeffect_p.h"
#include "hbvgblureffect_p.h"
#include "hbvggloweffect_p.h"
#include "hbeffectfilter_p.h"
#include <QGraphicsItem>
#include <QtDebug>

// -----------------------------------------------------------------------------

// HbEffectBlur

HbEffectBlur::HbEffectBlur(
    const HbEffectFxmlFilterData &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectFilter(0, item, group),
        mAnimationO(0),
        mAnimationX(0),
        mAnimationY(0),
        mVgBlur(0),
        mType(Blur)
{
    // If it is a glow filter, change effect type
    if (data.type() == HB_EFFECT_NAME_GLOW) {
        mType = Glow;
    }

    // Default values of if something is not passed in FXML
    qreal opacity_start = 1;
    qreal opacity_end = 1;
    qreal blur_x_start = 1;
    qreal blur_x_end = 1;
    qreal blur_y_start = 1;
    qreal blur_y_end = 1;

    QList<HbEffectFxmlParamData> params = data.paramData();

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, params) {
        if (param.name() == FXML_KEYWORD_BLUR_OPACITY) {
            mAnimationO = createAnimation(param, opacity_start, opacity_end, group);
        }
        else if (param.name() == FXML_KEYWORD_BLUR_ORIGIN_X) {
            mAnimationX = createAnimation(param, blur_x_start, blur_x_end, group);
        }
        else if (param.name() == FXML_KEYWORD_BLUR_ORIGIN_Y) {
            mAnimationY = createAnimation(param, blur_y_start, blur_y_end, group);
        }
    }

    // Additive blending mode changes blur effect to glow effect.
    if (data.blending() == FXML_BLENDING_ADDITIVE) {
        mType = HbEffectBlur::Glow;
    }

    // Create VG effect if there were effect parameters defined
    if (mEffectDefined) {
        // Add blur effect to the filter effect chain in the effect group
        HbVgChainedEffect *chain = HbEffectAbstract::group()->vgEffect();
        
        if (mType == Blur) {
            mVgBlur = new HbVgBlurEffect();
        }
        else {
            mVgBlur = new HbVgGlowEffect();
        }
        mVgBlur->setCaching(true);
        chain->add(mVgBlur);

        // Set initial radius for the blur effect
        mVgBlur->setRadius(blur_x_start, blur_y_start);
        mVgBlur->setOpacity(opacity_start);
    }
}

HbEffectBlur::~HbEffectBlur()
{
}

QString HbEffectBlur::name() const
{
    return mType == Blur ? HB_EFFECT_NAME_BLUR : HB_EFFECT_NAME_GLOW;
}

void HbEffectBlur::updateFilterEffect()
{
    // Only update the graphics effect if the effect is running
    if (group()->isRunning()) {
        QPointF blurRadius = mVgBlur->radius();
        if (mAnimationX) {
            blurRadius.setX(qVariantValue<qreal>(mAnimationX->currentValue()));
        }
        if (mAnimationY) {
            blurRadius.setY(qVariantValue<qreal>(mAnimationY->currentValue()));
        }

        mVgBlur->setRadius(blurRadius);

        if (mAnimationO) {
            mVgBlur->setOpacity(qVariantValue<qreal>(mAnimationO->currentValue()));
        }

#if 0 // Enable for debugging parameters given to the openVG filter
        QString opacityString;
        if (mAnimationO) {
            opacityString.setNum(qVariantValue<qreal>(mAnimationO->currentValue()));
        }
        else {
            opacityString = "N/A";
        }

        qDebug() << "HbEffectBlur: Updating filter with radius = (" << blurRadius.x() << "," << blurRadius.y()
                 << ") - Opacity =" << opacityString;
#endif
    }
}

#endif
