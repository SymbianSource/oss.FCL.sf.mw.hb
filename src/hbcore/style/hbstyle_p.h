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

#ifndef HBSTYLE_P_H
#define HBSTYLE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QHash>
#include <QVector>
#include <hbstyle.h>
#include "hbcssparser_p.h"
#include "hblayoutparameters_p.h"

class HbWidget;
class HbWidgetBasePrivate;
static const int ItemName = 0xfffe;
class HbAnchor;


class HB_CORE_PRIVATE_EXPORT HbStylePrivate
{
    Q_DECLARE_PUBLIC( HbStyle )
public:
    HbStylePrivate();
    virtual ~HbStylePrivate();


    // HbStyle::Primitive enums are DEPRECATED
    enum Primitive {
        P_None,
        P_PushButton_icon,
        P_PushButton_text,
        P_PushButton_additionaltext,
        P_PushButton_background,
        P_PushButton_toucharea,
        P_PushButton_focus,
        P_DataGroup_background,
        P_DataGroupComboBackground,
        P_DataGroup_heading,
        P_DataGroup_description,
        P_DataForm_heading,
        P_DataForm_heading_background,
        P_DataForm_description,
        P_DataGroup_icon,
        P_ToolButton_frame,
        P_ToolButton_text,
        P_ToolButton_icon,
        P_Slider_thumb,
        P_SliderElement_text,
        P_SliderElement_icon,
        P_SliderElement_increase,
        P_SliderElement_decrease,
        P_Slider_groove,
        P_Slider_progressgroove,
        P_ItemViewItem_checkbox,
        P_ItemViewItem_radiobutton,
        P_ItemViewItem_selection,
        P_LineEdit_frame_normal,
        P_LineEdit_frame_highlight,
        P_TextEdit_frame_normal,
        P_TextEdit_frame_highlight,
        P_Edit_text,
        P_Label_text,
        P_Label_icon,
        P_ScrollBar_groove,
        P_ScrollBar_handle,
        P_ScrollBar_toucharea,
        P_Popup_background,
        P_Popup_background_weak,
        P_Popup_background_fs,
        P_Popup_heading_frame,
        P_ToolTip_background,
        P_MessageBox_icon,
        P_MessageBox_text,
        P_ItemViewItem_background,
        P_ListViewItem_text,
        P_ListViewItem_richtext,
        P_ListViewItem_icon,
        P_ItemHighlight_background,
        P_ToolBarExtension_background,
        P_GridViewItem_text,
        P_GridViewItem_icon,
        P_CheckBox_text,
        P_CheckBox_icon,
        P_CheckBox_toucharea,
        P_Fade_background,
        P_TitlePane_background,
        P_TitlePane_text,
        P_TitlePane_icon,
        P_TitlePane_toucharea,
        P_TitleBar_toucharea,
        P_SignalIndicator_icon,
        P_SignalLevel_background,
        P_SignalLevel_icon,
        P_BatteryIndicator_icon,
        P_BatteryLevel_background,
        P_BatteryLevel_icon,
        P_IndicatorGroup_icon1,
        P_IndicatorGroup_icon2,
        P_IndicatorGroup_icon3,
        P_IndicatorGroup_icon4,
        P_ProgressBar_frame,
        P_ProgressBar_track,
        P_ProgressBar_waittrack,
        P_ProgressBar_mintext,
        P_ProgressBar_maxtext,
        P_NavigationButton_background,
        P_NavigationButton_toucharea,
        P_IndicatorButton_background,
        P_IndicatorButton_handleindication,
        P_IndicatorButton_eventindication,
        P_IndicatorButton_toucharea,
        P_ItemViewItem_frame,
        P_SelectionControl_selectionstart,
        P_SelectionControl_selectionend,
        P_TreeViewItem_expandicon,
        P_Label_richtext,
        P_RatingSlider_track,
        P_RatingSlider_frame,
        P_RatingSlider_toucharea,
                P_ProgressSlider_frame,
                P_ProgressSlider_track,
        P_ProgressSlider_slidertrack,
        P_ProgressSlider_toucharea,
        P_ProgressSliderHandle_background, // deprecated
        P_ProgressSliderHandle_icon,
        P_ProgressSliderHandle_toucharea,
        P_RatingSlider_layout,
        P_ScrollArea_continuationbottom,
        P_ScrollArea_continuationtop,
        P_ScrollArea_continuationleft,
        P_ScrollArea_continuationright,
        P_ItemViewItem_focus,
        P_NotificationDialog_icon,
        P_NotificationDialog_text,
        P_NotificationDialog_title,
        P_NotificationDialog_frame,
        P_ComboBox_background,
        P_ComboBoxPopup_background,
        P_ComboBoxButton_toucharea,
        P_ComboBox_text,
        P_DataItem_background,
        P_DataItem_separator,
        P_ColorGridViewItem_colorIcon,
        P_ColorGridViewItem_borderIcon,
        P_ColorGridViewItem_checkIcon,
        P_ComboBox_button,
        P_ProgressDialog_icon,
        P_ProgressDialog_text,
        P_DataItem_label,
        P_DataItem_description,
        P_SliderElement_touchincrease,
        P_SliderElement_touchdecrease,
        P_SliderElement_touchhandle,
        P_SliderElement_touchgroove,
        P_SliderTickMark_majoricon,
        P_SliderTickMark_minoricon,
        P_SliderTickMark_majorlabel,
        P_SliderTickMark_minorlabel,
        P_QueryInputMode_image,
        P_GroupBoxHeading_icon,
        P_GroupBoxHeading_text,
        P_GroupBoxMarquee_text,
        P_GroupBoxHeading_background,
        P_GroupBoxContent_background,
        P_DataItem_icon,
        P_ItemViewItem_touchmultiselection,
        P_TumbleView_background,
        P_TumbleView_frame,
        P_TumbleView_highlight,
        P_DateTimePicker_background,
        P_DateTimePicker_frame,
        P_DateTimePicker_separator,
        P_IndexFeedback_popup_text,
        P_IndexFeedback_popup_background,
        P_SliderPopup_background,
        P_StatusBar_background,
        P_StatusBar_timetext,
        P_InputDialog_text,
        P_InputDialog_additionaltext,
        P_Last,   // Keep this always as the LAST item in the active primitives list!

        P_Deprecated = 0x000f0000,
        // Deprecated enumerations
        // These are DEPRECATED, replace or remove from widget/app sources !!
        P_ProgressBar_toucharea = P_ProgressSlider_toucharea,           // DEPRECATED
        P_ProgressBar_slidertrack = P_ProgressSlider_track,             // DEPRECATED
        P_ProgressSlider_handleicon = P_ProgressSliderHandle_icon,      // DEPRECATED
        P_InputDialog_additional_text = P_InputDialog_additionaltext,    //deprecated

        P_NotUsed = 0x00f00000,
        P_ProgressSlider_handle,    // DEPRECATED, NOT USED
        P_ProgressBar_text,          // DEPRECATED, NOT USED

        P_CustomBase = 0x0f000000
    };


    static QGraphicsItem *createPrimitive( HbStylePrivate::Primitive primitive, QGraphicsItem *parent = 0 );
    static void updatePrimitive( QGraphicsItem *item, HbStylePrivate::Primitive primitive, const QStyleOption *option );

    static QString logicalName(HbStylePrivate::Primitive primitive, const QStyleOption *option);
    static QIcon::Mode iconMode(QStyle::State state);
    static QIcon::State iconState(QStyle::State state);


    void polishItem(
        const HbVector<HbCss::StyleRule> &styleRules,
        HbWidget *widget,
        QGraphicsItem *item,
        const QString &name,
        HbDeviceProfile &profile,
        bool layoutDefined) const;
    void polishAnchor(
        const HbVector<HbCss::StyleRule> &styleRules,
        HbWidget *widget,
        HbAnchor *anchor,
        HbDeviceProfile &profile) const;
    void updateThemedItems(
        const HbVector<HbCss::StyleRule> &styleRules,
        QGraphicsItem *item,
        HbDeviceProfile &profile) const;

    void clearStyleSheetCaches();

    static HbWidgetBasePrivate *widgetBasePrivate(HbWidgetBase *widgetBase);

    bool valueFromTokens(
        const QList<int> &tokens,
        qreal &value,
        const HbDeviceProfile &profile = HbDeviceProfile()) const;

    HbStyle* q_ptr;

    mutable HbLayoutParameters layoutParameters;
    mutable QHash<QString, HbVector<HbCss::StyleRule> > styleRulesCache;

    int mLocTestMode;
};

#endif // HBSTYLE_P_H
