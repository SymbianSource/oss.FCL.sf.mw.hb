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

#include "hbeffectplanarreflection_p.h" //krazy:exclude=includes
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"
#include "hbvgchainedeffect_p.h"
#include "hbvgreflectioneffect_p.h"
#include "hbeffectfilter_p.h"
#include <QGraphicsItem>
#include <QtDebug>

// -----------------------------------------------------------------------------

// HbEffectPlanarReflection

HbEffectPlanarReflection::HbEffectPlanarReflection(
    const HbEffectFxmlFilterData &data,
    QGraphicsItem *item,
    HbEffectGroup *group) :
    HbEffectFilter(0, item, group),
    mAnimationO(0),
    mAnimationC(0),
    mAnimationX(0),
    mAnimationY(0),
    mAnimationF(0),
    mVgReflection(0)
{
    // Default values of if something is not passed in FXML
    qreal opacity_start = 1;
    qreal opacity_end = 1;

    // Default values for color should be "undefined" so using default constructor,
    // which constructs invalid QColor.
    QColor color_start;
    QColor color_end;

    qreal offset_x_start = 0;
    qreal offset_x_end = 0;
    qreal offset_y_start = 0;
    qreal offset_y_end = 0;
    qreal fade_start = 0;
    qreal fade_end = 0;

    QList<HbEffectFxmlParamData> params = data.paramData();

    // Handle FXML parameters
    Q_FOREACH(const HbEffectFxmlParamData & param, params) {
        if (param.name() == FXML_KEYWORD_PLANAR_REFLECTION_OPACITY) {
            mAnimationO = createAnimation(param, opacity_start, opacity_end, group);
        } else if (param.name() == FXML_KEYWORD_PLANAR_REFLECTION_COLOR) {
            mAnimationC = createAnimation(param, color_start, color_end, group);
        } else if (param.name() == FXML_KEYWORD_PLANAR_REFLECTION_OFFSET_X) {
            mAnimationX = createAnimation(param, offset_x_start, offset_x_end, group);
        } else if (param.name() == FXML_KEYWORD_PLANAR_REFLECTION_OFFSET_Y) {
            mAnimationY = createAnimation(param, offset_y_start, offset_y_end, group);
        } else if (param.name() == FXML_KEYWORD_PLANAR_REFLECTION_FADE) {
            mAnimationF = createAnimation(param, fade_start, fade_end, group);
        }
    }

    // Create VG effect if there were effect parameters defined
    if (mEffectDefined) {
        // Add the effect to the filter effect chain in the effect group
        HbVgChainedEffect *chain = HbEffectAbstract::group()->vgEffect();

        mVgReflection = new HbVgReflectionEffect();
        mVgReflection->setCaching(true);

        chain->add(mVgReflection);

        // Set initial values for the effect
        mVgReflection->setOpacity(opacity_start);
        if (color_start.isValid()) {
            mVgReflection->setColor(color_start);
        }
        mVgReflection->setOffset(QPointF(offset_x_start, offset_y_start));
        mVgReflection->setFade(fade_start);
    }
}

HbEffectPlanarReflection::~HbEffectPlanarReflection()
{
}

QString HbEffectPlanarReflection::name() const
{
    return HB_EFFECT_NAME_PLANAR_REFLECTION;
}

void HbEffectPlanarReflection::updateFilterEffect()
{
    // Only update the graphics effect if the effect is running
    if (group()->isRunning()) {
        if (mAnimationO) {
            mVgReflection->setOpacity(qVariantValue<qreal>(mAnimationO->currentValue()));
        }
        if (mAnimationC) {
            mVgReflection->setColor(qVariantValue<QColor>(mAnimationC->currentValue()));
        }

        QPointF offset = mVgReflection->offset();

        if (mAnimationX) {
            offset.setX(qVariantValue<qreal>(mAnimationX->currentValue()));
        }
        if (mAnimationY) {
            offset.setY(qVariantValue<qreal>(mAnimationY->currentValue()));
        }

        mVgReflection->setOffset(offset);

        if (mAnimationF) {
            mVgReflection->setFade(qVariantValue<qreal>(mAnimationF->currentValue()));
        }
    }
}

#endif
