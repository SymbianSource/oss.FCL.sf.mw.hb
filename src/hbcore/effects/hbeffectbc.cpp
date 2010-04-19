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

#include "hbeffectbc_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbvgchainedeffect_p.h"
#include "hbvgbceffect_p.h"
#include "hbeffectfilter_p.h"
#include <QGraphicsItem>
#include <QtDebug>

// -----------------------------------------------------------------------------

// HbEffectBc

HbEffectBc::HbEffectBc(
    const HbEffectFxmlFilterData &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectFilter(0, item, group),
        mAnimationO(0),
        mAnimationB(0),
        mAnimationC(0),
        mVgBc(0)
{
    // Default values of if something is not passed in FXML
    qreal opacity_start = 1;
    qreal opacity_end = 1;
    qreal brightness_start = 0;
    qreal brightness_end = 0;
    qreal contrast_start = 1;
    qreal contrast_end = 1;

    QList<HbEffectFxmlParamData> params = data.paramData();

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, params) {
        if (param.name() == FXML_KEYWORD_BRIGHTNESS_CONTRAST_OPACITY) {
            mAnimationO = createAnimation(param, opacity_start, opacity_end, group);
        }
        else if (param.name() == FXML_KEYWORD_BRIGHTNESS_CONTRAST_BRIGHTNESS) {
            mAnimationB = createAnimation(param, brightness_start, brightness_end, group);
        }
        else if (param.name() == FXML_KEYWORD_BRIGHTNESS_CONTRAST_CONTRAST) {
            mAnimationC = createAnimation(param, contrast_start, contrast_end, group);
        }
    }

    // Create VG effect if there were effect parameters defined
    if (mEffectDefined) {
        // Add effect to the filter effect chain in the effect group
        HbVgChainedEffect *chain = HbEffectAbstract::group()->vgEffect();
        
        mVgBc = new HbVgBcEffect();
        mVgBc->setCaching(true);
        
        chain->add(mVgBc);

        // Set initial values for the effect
        mVgBc->setOpacity(opacity_start);
        mVgBc->setBrightness(brightness_start);
        mVgBc->setContrast(contrast_start);
    }
}

HbEffectBc::~HbEffectBc()
{
}

QString HbEffectBc::name() const
{
    return HB_EFFECT_NAME_BRIGHTNESS_CONTRAST;
}

void HbEffectBc::updateFilterEffect()
{
    // Only update the graphics effect if the effect is running
    if (group()->isRunning()) {
        if (mAnimationO) {
            mVgBc->setOpacity(qVariantValue<qreal>(mAnimationO->currentValue()));
        }
        if (mAnimationB) {
            mVgBc->setBrightness(qVariantValue<qreal>(mAnimationB->currentValue()));
        }
        if (mAnimationC) {
            mVgBc->setContrast(qVariantValue<qreal>(mAnimationC->currentValue()));
        }
    }
}

#endif
