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

#include "hbeffecthsl_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbvgchainedeffect_p.h"
#include "hbvghsleffect_p.h"
#include "hbeffectfilter_p.h"
#include <QGraphicsItem>
#include <QtDebug>

// -----------------------------------------------------------------------------

// HbEffectHsl

HbEffectHsl::HbEffectHsl(
    const HbEffectFxmlFilterData &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectFilter(0, item, group),
        mAnimationO(0),
        mAnimationH(0),
        mAnimationS(0),
        mAnimationL(0),
        mVgHsl(0)
{
    // Default values of if something is not passed in FXML
    qreal opacity_start = 1;
    qreal opacity_end = 1;
    qreal hue_start = 0;
    qreal hue_end = 0;
    qreal saturation_start = 1;
    qreal saturation_end = 1;
    qreal lightness_start = 0;
    qreal lightness_end = 0;

    QList<HbEffectFxmlParamData> params = data.paramData();

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, params) {
        if (param.name() == FXML_KEYWORD_HSL_OPACITY) {
            mAnimationO = createAnimation(param, opacity_start, opacity_end, group);
        }
        else if (param.name() == FXML_KEYWORD_HSL_HUE) {
            mAnimationH = createAnimation(param, hue_start, hue_end, group);
        }
        else if (param.name() == FXML_KEYWORD_HSL_SATURATION) {
            mAnimationS = createAnimation(param, saturation_start, saturation_end, group);
        }
        else if (param.name() == FXML_KEYWORD_HSL_LIGHTNESS) {
            mAnimationL = createAnimation(param, lightness_start, lightness_end, group);
        }
    }

    // Create VG effect if there were effect parameters defined
    if (mEffectDefined) {
        // Add blur effect to the filter effect chain in the effect group
        HbVgChainedEffect *chain = HbEffectAbstract::group()->vgEffect();
        
        mVgHsl = new HbVgHslEffect();
        mVgHsl->setCaching(true);
        
        chain->add(mVgHsl);

        // Set initial values for the effect
        mVgHsl->setOpacity(opacity_start);
        mVgHsl->setHue(hue_start);
        mVgHsl->setSaturation(saturation_start);
        mVgHsl->setLightness(lightness_start);
    }
}

HbEffectHsl::~HbEffectHsl()
{
}

QString HbEffectHsl::name() const
{
    return HB_EFFECT_NAME_HSL;
}

void HbEffectHsl::updateFilterEffect()
{
    // Only update the graphics effect if the effect is running
    if (group()->isRunning()) {
        if (mAnimationO) {
            mVgHsl->setOpacity(qVariantValue<qreal>(mAnimationO->currentValue()));
        }
        if (mAnimationH) {
            mVgHsl->setHue(qVariantValue<qreal>(mAnimationH->currentValue()));
        }
        if (mAnimationS) {
            mVgHsl->setSaturation(qVariantValue<qreal>(mAnimationS->currentValue()));
        }
        if (mAnimationL) {
            mVgHsl->setLightness(qVariantValue<qreal>(mAnimationL->currentValue()));
        }
    }
}

#endif
