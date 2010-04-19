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
#include "hbeffectfactory_p.h"
#include "hbeffectabstract_p.h"
#include "hbeffectscale_p.h"
#include "hbeffectopacity_p.h"
#include "hbeffectrotate_p.h"
#include "hbeffecttranslate_p.h"
#ifdef HB_FILTER_EFFECTS
#include "hbeffectblur_p.h"
#include "hbeffectdropshadow_p.h"
#include "hbeffectcolorize_p.h"
#include "hbeffecthsl_p.h"
#include "hbeffectbc_p.h"
#include "hbeffectplanarreflection_p.h"
#endif
#include "hbapplication.h"
#include "hbeffectcontroller_p.h"
#include "hbeffectgroup_p.h"
#include "hbeffectdef_p.h"

inline bool isScaleParameter(const QString &param)
{
    return 
        param == FXML_KEYWORD_SCALE_X ||
        param == FXML_KEYWORD_SCALE_Y;
}

inline bool isRotateParameter(const QString &param)
{
    return
        param == FXML_KEYWORD_ROTATION_ORIGIN_X ||
        param == FXML_KEYWORD_ROTATION_ORIGIN_Y ||
        param == FXML_KEYWORD_ROTATION_ORIGIN_Z ||
        param == FXML_KEYWORD_ROTATION_ANGLE ||
        param == FXML_KEYWORD_ROTATION_AXIS_X ||
        param == FXML_KEYWORD_ROTATION_AXIS_Y ||
        param == FXML_KEYWORD_ROTATION_AXIS_Z;
}

inline bool isOpacityParameter(const QString &param)
{
    return param == FXML_KEYWORD_OPACITY;
}

inline bool isTranslateParameter(const QString &param)
{
    return
		param == FXML_KEYWORD_TRANSLATION_X ||
		param == FXML_KEYWORD_TRANSLATION_Y;
}


/*
    \class HbEffectFactory

    \brief HbEffectFactory is used for instantiating different HbEffectAbstract instances
    based on effect definition.
*/

/*
    Constructor taking controller instance..
*/
HbEffectFactory::HbEffectFactory(HbEffectController &controller) :
    mController(controller)
{
}

/*
    Destructor.
*/
HbEffectFactory::~HbEffectFactory()
{
}

HbEffectGroup *HbEffectFactory::createEffect(
    QGraphicsItem *registrationItem,
    QGraphicsItem *targetItem,
    const QString &itemType,
    const QString &effectEventType)
{
    HbEffectGroup *group = new HbEffectGroup(effectEventType, registrationItem, targetItem, itemType);

    // Try to get FXML definition
    const HbEffectFxmlData &fxmlData = mController.fetchFxmlData(
        itemType, registrationItem, effectEventType);

    if (!fxmlData.isNull()) {
        QList<HbEffectFxmlParamData> params = fxmlData.paramData();

        static const int scaleBit = 0x0001;
        static const int rotateBit = 0x0002;
        static const int opacityBit = 0x0004;
        static const int translateBit = 0x0008;
#ifdef HB_FILTER_EFFECTS
        static const int blurBit = 0x0010;
        static const int brightnessContrastBit = 0x0020;
        static const int colorizeBit = 0x0040;
        static const int dropShadowBit = 0x0080;
        static const int hslBit = 0x00100;
        static const int planarReflectionBit = 0x00200;
        // Glow is handled by blur effect, outline is handled by drop shadow effect
#endif

        // Bitmask of created effects. Used for keeping track of handling params related to the same effect only once.
        // Also implication of this is that the same effect cannot be created more than once.
        int effectsCreated = 0;

        // Handle transform effect definitions first.
        // This creates HbEffectAbstract instances from the definitions.
        // The whole list of parameters is passed to the effect
        // constructor, which skips parameters that it does not understand.
        Q_FOREACH(const HbEffectFxmlParamData &param, params) {
            QString paramName = param.name();

            HbEffectAbstract *effect = 0;

            if (!(effectsCreated & scaleBit) && isScaleParameter(paramName)) {
                // Create a scale effect
                effect = new HbEffectScale(params, targetItem, group);
                effectsCreated |= scaleBit;
            }
            if (!(effectsCreated & rotateBit) && isRotateParameter(paramName)) {
                // Create a rotate effect
                effect = new HbEffectRotate(params, targetItem, group);
                effectsCreated |= rotateBit;
            }
            if (!(effectsCreated & opacityBit) && isOpacityParameter(paramName)) {
                // Create an opacity effect
                effect = new HbEffectOpacity(params, targetItem, group);
                effectsCreated |= opacityBit;
            }

            if (!(effectsCreated & translateBit) && isTranslateParameter(paramName)) {
                // Create a translate effect
                effect = new HbEffectTranslate(params, targetItem, group);
                effectsCreated |= translateBit;
            }

            if (effect) {
                // Add created effect to the group
                group->addEffect(effect);
            }
        } // foreach

#ifdef HB_FILTER_EFFECTS

        // Handle filter definitions similarly
        QList<HbEffectFxmlFilterData> filters = fxmlData.filterData();

        Q_FOREACH(const HbEffectFxmlFilterData &filter, filters) {
            HbEffectAbstract *effect = 0;
            QString type = filter.type();

            if (!(effectsCreated & blurBit) &&
                 (type == HB_EFFECT_NAME_BLUR || type == HB_EFFECT_NAME_GLOW)) {
                // Create a blur effect
                effect = new HbEffectBlur(filter, targetItem, group);
                effectsCreated |= blurBit;
            }
            if (!(effectsCreated & brightnessContrastBit) && type == HB_EFFECT_NAME_BRIGHTNESS_CONTRAST) {
                // Create a brightness/contrast effect
                effect = new HbEffectBc(filter, targetItem, group);
                effectsCreated |= brightnessContrastBit;
            }
            if (!(effectsCreated & colorizeBit) && type == HB_EFFECT_NAME_COLORIZE) {
                // Create a colorize effect
                effect = new HbEffectColorize(filter, targetItem, group);
                effectsCreated |= colorizeBit;
            }
            if (!(effectsCreated & dropShadowBit) &&
                 (type == HB_EFFECT_NAME_DROP_SHADOW || type == HB_EFFECT_NAME_OUTLINE)) {
                // Create a drop shadow effect
                effect = new HbEffectDropShadow(filter, targetItem, group);
                effectsCreated |= dropShadowBit;
            }
            if (!(effectsCreated & hslBit) && type == HB_EFFECT_NAME_HSL) {
                // Create a hsl effect
                effect = new HbEffectHsl(filter, targetItem, group);
                effectsCreated |= hslBit;
            }
            if (!(effectsCreated & planarReflectionBit) && type == HB_EFFECT_NAME_PLANAR_REFLECTION) {
                // Create a planar reflection effect
                effect = new HbEffectPlanarReflection(filter, targetItem, group);
                effectsCreated |= planarReflectionBit;
            }
            if (effect) {
                // Add created effect to the group
                group->addEffect(effect);
            }
        } // foreach
#endif
    }

    group->fixEffectOrder();
    return group;
}

// End of File
