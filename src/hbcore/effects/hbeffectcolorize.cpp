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

#include "hbeffectcolorize_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbvgchainedeffect_p.h"
#include "hbvgcolorizeeffect_p.h"
#include "hbeffectfilter_p.h"
#include <QGraphicsItem>
#include <QtDebug>

// -----------------------------------------------------------------------------

// HbEffectColorize

HbEffectColorize::HbEffectColorize(
    const HbEffectFxmlFilterData &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
        HbEffectFilter(0, item, group),
        mAnimationO(0),
        mAnimationC(0),
        mVgColorize(0)
{
    // Default values of if something is not passed in FXML
    qreal opacity_start = 1;
    qreal opacity_end = 1;

    QColor color_start(0, 0, 0);
    QColor color_end(0, 0, 0);

    QList<HbEffectFxmlParamData> params = data.paramData();

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData &param, params) {
        if (param.name() == FXML_KEYWORD_COLORIZE_OPACITY) {
            mAnimationO = createAnimation(param, opacity_start, opacity_end, group);
        }
        else if (param.name() == FXML_KEYWORD_COLORIZE_COLOR) {
            mAnimationC = createAnimation(param, color_start, color_end, group);
        }
    }

    // Create VG effect if there were effect parameters defined
    if (mEffectDefined) {
        // Add blur effect to the filter effect chain in the effect group
        HbVgChainedEffect *chain = HbEffectAbstract::group()->vgEffect();
        
        mVgColorize = new HbVgColorizeEffect();
        mVgColorize->setCaching(true);
        chain->add(mVgColorize);

        // Set initial values for the effect
        mVgColorize->setOpacity(opacity_start);
        mVgColorize->setColor(color_start);
    }
}

HbEffectColorize::~HbEffectColorize()
{
}

QString HbEffectColorize::name() const
{
    return HB_EFFECT_NAME_COLORIZE;
}

void HbEffectColorize::updateFilterEffect()
{
    // Only update the graphics effect if the effect is running
    if (group()->isRunning()) {
        if (mAnimationO) {
            mVgColorize->setOpacity(qVariantValue<qreal>(mAnimationO->currentValue()));
        }
        if (mAnimationC) {
            mVgColorize->setColor(qVariantValue<QColor>(mAnimationC->currentValue()));            
        }
    }
}

#endif
