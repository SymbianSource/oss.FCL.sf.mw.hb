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

#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbeffectdropshadow_p.h" //krazy:exclude=includes
#include "hbeffectfilter_p.h"
#include "hbvgchainedeffect_p.h"
#include "hbvgshadoweffect_p.h"
#include "hbvgoutlineeffect_p.h"
#include <QGraphicsItem>
#include <QtDebug>

// -----------------------------------------------------------------------------

// HbEffectDropShadow

HbEffectDropShadow::HbEffectDropShadow(
    const HbEffectFxmlFilterData &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
    HbEffectFilter(0, item, group),
    mAnimationO(0),
    mAnimationS(0),
    mAnimationOutlineX(0),
    mAnimationOutlineY(0),
    mAnimationOffsetX(0),
    mAnimationOffsetY(0),
    mAnimationC(0),
    mVgOutline(0),
    mType(HbEffectDropShadow::outline)
{
    // Default values from FXML spec
    qreal opacity_start = 1;
    qreal opacity_end = 1;

    qreal steepness_start = 20.0f;
    qreal steepness_end = 20.0f;

    qreal outline_x_start = 1.0f;
    qreal outline_x_end = 1.0f;
    qreal outline_y_start = 1.0f;
    qreal outline_y_end = 1.0f;

    qreal offset_x_start = 0.0f;
    qreal offset_x_end = 0.0f;
    qreal offset_y_start = 0.0f;
    qreal offset_y_end = 0.0f;

    QColor color_start(0, 0, 0);
    QColor color_end(0, 0, 0);

    QList<HbEffectFxmlParamData> params = data.paramData();

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData & param, params) {
        if (param.name() == FXML_KEYWORD_DROP_SHADOW_OPACITY) {
            mAnimationO = createAnimation(param, opacity_start, opacity_end, group);
        } else if (param.name() == FXML_KEYWORD_DROP_SHADOW_COLOR) {
            mAnimationC = createAnimation(param, color_start, color_end, group);
        } else if (param.name() == FXML_KEYWORD_DROP_SHADOW_OFFSET_X) {
            mAnimationOffsetX = createAnimation(param, offset_x_start, offset_x_end, group);
            // If offset parameters are used, it is a drop shadow effect.
            mType = dropShadow;
        } else if (param.name() == FXML_KEYWORD_DROP_SHADOW_OFFSET_Y) {
            mAnimationOffsetY = createAnimation(param, offset_y_start, offset_y_end, group);
            // If offset parameters are used, it is a drop shadow effect.
            mType = dropShadow;
        } else if (param.name() == FXML_KEYWORD_DROP_SHADOW_STEEPNESS) {
            mAnimationS = createAnimation(param, steepness_start, steepness_end, group);
        } else if (param.name() == FXML_KEYWORD_DROP_SHADOW_OUTLINE_X) {
            mAnimationOutlineX = createAnimation(param, outline_x_start, outline_x_end, group);
        } else if (param.name() == FXML_KEYWORD_DROP_SHADOW_OUTLINE_Y) {
            mAnimationOutlineY = createAnimation(param, outline_y_start, outline_y_end, group);
        }
    }

    // Create VG effect if there were effect parameters defined
    if (mEffectDefined) {
        // Add effect to the filter effect chain in the effect group
        HbVgChainedEffect *chain = HbEffectAbstract::group()->vgEffect();

        if (mType == dropShadow) {
            mVgOutline = new HbVgShadowEffect();
        } else {
            mVgOutline = new HbVgOutlineEffect();
        }

        mVgOutline->setCaching(true);

        chain->add(mVgOutline);

        // Set initial values for the effect
        mVgOutline->setOpacity(opacity_start);
        mVgOutline->setOutline(outline_x_start, outline_y_start);
        mVgOutline->setColor(color_start);
        mVgOutline->setSteepness(steepness_start);
        mVgOutline->setOffset(offset_x_start, offset_y_start);
    }
}

HbEffectDropShadow::~HbEffectDropShadow()
{
}

QString HbEffectDropShadow::name() const
{
    return mType == dropShadow ? HB_EFFECT_NAME_DROP_SHADOW : HB_EFFECT_NAME_OUTLINE;
}

void HbEffectDropShadow::updateFilterEffect()
{
    // Only update the graphics effect if the effect is running
    if (mVgOutline && group()->isRunning()) {
        QPointF outline = mVgOutline->outline();
        QPointF offset = mVgOutline->offset();

        if (mAnimationOutlineX) {
            outline.setX(qVariantValue<qreal>(mAnimationOutlineX->currentValue()));
        }
        if (mAnimationOutlineY) {
            outline.setY(qVariantValue<qreal>(mAnimationOutlineY->currentValue()));
        }
        if (mAnimationOffsetX) {
            offset.setX(qVariantValue<qreal>(mAnimationOffsetX->currentValue()));
        }
        if (mAnimationOffsetY) {
            offset.setY(qVariantValue<qreal>(mAnimationOffsetY->currentValue()));
        }

        mVgOutline->setOutline(outline);
        mVgOutline->setOffset(offset);

        if (mAnimationS) {
            mVgOutline->setSteepness(qVariantValue<qreal>(mAnimationS->currentValue()));
        }
        if (mAnimationC) {
            mVgOutline->setColor(qVariantValue<QColor>(mAnimationC->currentValue()));
        }
        if (mAnimationO) {
            mVgOutline->setOpacity(qVariantValue<qreal>(mAnimationO->currentValue()));
        }
    }
}

#endif
