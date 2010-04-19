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
#ifndef HB_EFFECT_DEF_P_H
#define HB_EFFECT_DEF_P_H

// Effect names
#define HB_EFFECT_NAME_SCALE "scale"
#define HB_EFFECT_NAME_OPACITY "opacity"
#define HB_EFFECT_NAME_ROTATE "rotate"
#define HB_EFFECT_NAME_TRANSLATE "translate"


// FXML tags
#define FXML_LAYERS "layers"
#define FXML_LAYERGROUP "layergroup"
#define FXML_VISUAL "visual"
#define FXML_FILTER "filter"
#define FXML_BLENDING "blending"
#define FXML_BLENDING_ADDITIVE "additive"
#define FXML_PARAM "param"
#define FXML_DURATION "duration"
#define FXML_STYLE "style"
#define FXML_MARKER "marker"
#define FXML_LOOP_START "loop_start"
#define FXML_LOOP_END "loop_end"
#define FXML_KEYFRAME "keyframe"

#define FXML_PARAM_NAME "name"
#define FXML_PARAM_TYPE "type"
#define FXML_PARAM_AT "at"
#define FXML_PARAM_REF "ref"
#define FXML_COMMENT "comment"

#define FXML_KEYWORD_ANIM "anim"

#define FXML_KEYWORD_OPACITY "opacity"

#define FXML_KEYWORD_SCALE_X "scale_x"
#define FXML_KEYWORD_SCALE_Y "scale_y"
#define FXML_KEYWORD_SCALE_ORIGIN_X "scale_origin_x"
#define FXML_KEYWORD_SCALE_ORIGIN_Y "scale_origin_y"
#define FXML_KEYWORD_START "start"
#define FXML_KEYWORD_END "end"

#define FXML_KEYWORD_TRANSLATION_X "translation_x"
#define FXML_KEYWORD_TRANSLATION_Y "translation_y"

#define FXML_KEYWORD_ROTATION_ORIGIN_X "rotation_origin_x"
#define FXML_KEYWORD_ROTATION_ORIGIN_Y "rotation_origin_y"
#define FXML_KEYWORD_ROTATION_ORIGIN_Z "rotation_origin_z"
#define FXML_KEYWORD_ROTATION_ANGLE "rotation_angle"
#define FXML_KEYWORD_ROTATION_AXIS_X "rotation_axis_x"
#define FXML_KEYWORD_ROTATION_AXIS_Y "rotation_axis_y"
#define FXML_KEYWORD_ROTATION_AXIS_Z "rotation_axis_z"

// Ref tags
#define FXML_KEYWORD_VISUAL_LEFT "visual.left"
#define FXML_KEYWORD_VISUAL_RIGHT "visual.right"
#define FXML_KEYWORD_VISUAL_TOP "visual.top"
#define FXML_KEYWORD_VISUAL_BOTTOM "visual.bottom"
#define FXML_KEYWORD_VISUAL_WIDTH "visual.width"
#define FXML_KEYWORD_VISUAL_HEIGHT "visual.height"

#define FXML_KEYWORD_SCREEN_LEFT "screen.left"
#define FXML_KEYWORD_SCREEN_RIGHT "screen.right"
#define FXML_KEYWORD_SCREEN_TOP "screen.top"
#define FXML_KEYWORD_SCREEN_BOTTOM "screen.bottom"
#define FXML_KEYWORD_SCREEN_WIDTH "screen.width"
#define FXML_KEYWORD_SCREEN_HEIGHT "screen.height"

#define FXML_KEYWORD_EXTRECT_LEFT "extrect.left"
#define FXML_KEYWORD_EXTRECT_RIGHT "extrect.right"
#define FXML_KEYWORD_EXTRECT_TOP "extrect.top"
#define FXML_KEYWORD_EXTRECT_BOTTOM "extrect.bottom"
#define FXML_KEYWORD_EXTRECT_WIDTH "extrect.width"
#define FXML_KEYWORD_EXTRECT_HEIGHT "extrect.height"

// Style options
#define FXML_KEYWORD_STYLE_LINEAR "linear"
#define FXML_KEYWORD_STYLE_INQUAD "inquad"
#define FXML_KEYWORD_STYLE_OUTQUAD "outquad"
#define FXML_KEYWORD_STYLE_INOUTQUAD "inoutquad"
#define FXML_KEYWORD_STYLE_OUTINQUAD "outinquad"
#define FXML_KEYWORD_STYLE_INBACK "inback"
#define FXML_KEYWORD_STYLE_OUTBACK "outback"
#define FXML_KEYWORD_STYLE_INOUTBACK "inoutback"
#define FXML_KEYWORD_STYLE_OUTINBACK "outinback"

#ifdef HB_FILTER_EFFECTS
// For blur effect
#define HB_EFFECT_NAME_BLUR "blur"
#define FXML_KEYWORD_BLUR_OPACITY "opacity"
#define FXML_KEYWORD_BLUR_ORIGIN_X "blur_x"
#define FXML_KEYWORD_BLUR_ORIGIN_Y "blur_y"

// For brightness/contrast
#define HB_EFFECT_NAME_BRIGHTNESS_CONTRAST "brightness_contrast"
#define FXML_KEYWORD_BRIGHTNESS_CONTRAST_OPACITY "opacity"
#define FXML_KEYWORD_BRIGHTNESS_CONTRAST_BRIGHTNESS "brightness"
#define FXML_KEYWORD_BRIGHTNESS_CONTRAST_CONTRAST "contrast"

// For colorize effect
#define HB_EFFECT_NAME_COLORIZE "colorize"
#define FXML_KEYWORD_COLORIZE_OPACITY "opacity"
#define FXML_KEYWORD_COLORIZE_COLOR "color"

// For drop shadow effect
#define HB_EFFECT_NAME_DROP_SHADOW "drop_shadow"
#define FXML_KEYWORD_DROP_SHADOW_OPACITY "opacity"
#define FXML_KEYWORD_DROP_SHADOW_COLOR "color"
#define FXML_KEYWORD_DROP_SHADOW_STEEPNESS "steepness"
#define FXML_KEYWORD_DROP_SHADOW_OFFSET_X "offset_x"
#define FXML_KEYWORD_DROP_SHADOW_OFFSET_Y "offset_y"
#define FXML_KEYWORD_DROP_SHADOW_OUTLINE_X "outline_x"
#define FXML_KEYWORD_DROP_SHADOW_OUTLINE_Y "outline_y"

// For glow effect
#define HB_EFFECT_NAME_GLOW "glow"
#define FXML_KEYWORD_GLOW_OPACITY "opacity"
#define FXML_KEYWORD_GLOW_ORIGIN_X "glow_x"
#define FXML_KEYWORD_GLOW_ORIGIN_Y "glow_y"

// For HSL effect
#define HB_EFFECT_NAME_HSL "hsl"
#define FXML_KEYWORD_HSL_OPACITY "opacity"
#define FXML_KEYWORD_HSL_HUE "hue"
#define FXML_KEYWORD_HSL_SATURATION "saturation"
#define FXML_KEYWORD_HSL_LIGHTNESS "lightness"

// For outline effect
#define HB_EFFECT_NAME_OUTLINE "outline"
#define FXML_KEYWORD_OUTLINE_OPACITY "opacity"
#define FXML_KEYWORD_OUTLINE_COLOR "color"
#define FXML_KEYWORD_OUTLINE_STEEPNESS "steepness"
#define FXML_KEYWORD_OUTLINE_OFFSET_X "offset_x"
#define FXML_KEYWORD_OUTLINE_OFFSET_Y "offset_y"
#define FXML_KEYWORD_OUTLINE_OUTLINE_X "outline_x"
#define FXML_KEYWORD_OUTLINE_OUTLINE_Y "outline_y"

// For planar reflection effect
#define HB_EFFECT_NAME_PLANAR_REFLECTION "planar_reflection"
#define FXML_KEYWORD_PLANAR_REFLECTION_OPACITY "opacity"
#define FXML_KEYWORD_PLANAR_REFLECTION_COLOR "color"
#define FXML_KEYWORD_PLANAR_REFLECTION_OFFSET_X "offset_x"
#define FXML_KEYWORD_PLANAR_REFLECTION_OFFSET_Y "offset_y"
#define FXML_KEYWORD_PLANAR_REFLECTION_FADE "fade"

#endif

#endif // HB_EFFECT_DEF_P_H
