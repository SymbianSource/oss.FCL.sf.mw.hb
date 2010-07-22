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

#include "hbstyle_p.h"
#include "hbstyleoptionslider_p.h"
#include "hbstyleoptionsliderelement_p.h"
#include "hbstyleoptionpushbutton_p.h"
#include "hbstyleoptiontoolbutton_p.h"
#include "hbstyleoptiontooltip_p.h"
#include "hbstyleoptionprogressbar_p.h"
#include "hbstyleoptionabstractviewitem_p.h"
#include "hbstyleoptionlistviewitem_p.h"
#include "hbstyleoptionmenuitem_p.h"
#include "hbstyleoptionlabel_p.h"
#include "hbstyleoptionscrollbar_p.h"
#include "hbstyleoptiongridviewitem_p.h"
#include "hbstyleoptioncheckbox_p.h"
#include "hbiconloader_p.h"
#include "hbstyleoptiontitlepane_p.h"
#include "hbstyleoptionstatusbar_p.h"
#include "hbstyleoptionindicatorgroup_p.h"
#include "hbstyleoptionprogressdialog_p.h"
#include "hbstyleoptionnotificationdialog_p.h"
#include "hbstyleoptiontreeviewitem_p.h"
#include "hbstyleoptioncolorgridviewitem_p.h"
#include "hbstyleoptionmessagebox_p.h"
#include "hbstyleoptionnavigationbutton_p.h"
#include "hbstyleoptionindicatorbutton_p.h"
#include "hbstyleoptionsignalindicator_p.h"
#include "hbstyleoptionbatteryindicator_p.h"
#include "hbstyleloader.h"
#include "hbwidgetloader_p.h"
#include "hbwidget_p.h"
#include "hbstyleoptionratingslider_p.h"
#include "hbstyleoptiondataformviewitem_p.h"
#include "hbstyleoptiondatagroup_p.h"
#include "hbstyleoptiondatagroupheadingwidget_p.h"
#include "hbstyleoptiondataform_p.h"
#include "hbstyleoptiongroupbox_p.h"
#include "hbstyleoptionindexfeedback_p.h"
#include "hbstyleoptioncombobox_p.h"
#include "hbstyleoptioninputdialog_p.h"
#include "hbstyleoptionprogresssliderhandle_p.h"
#include "hbstyleoptionprogressslider_p.h"

#include <hbicon.h>
#include <hbstyle.h>
#include <hbframedrawer.h>
#include <hbiconitem.h>
#include <hbiconitem_p.h>
#include <hbframeitem.h>
#include <hbframedrawer.h>
#include <hbframedrawer_p.h>
#include <hbframebackground.h>
#include <hbprogresstrackitem_p.h>
#include <hbslidertrackitem_p.h>
#include <hbinstance.h>
#include <hbtextitem.h>
#include <hbtextitem_p.h>
#include <hbmarqueeitem.h>
#include <hbmarqueeitem_p.h>
#include <hbrichtextitem.h>
#include <hbrichtextitem_p.h>
#include <hbtoucharea.h>
#include <hbindicatorleveliconitem_p.h>

#include <hbglobal.h>
#include <hbstyleparameters.h>

#include "hbstyleselector_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbwidgetstyleloader_p.h"
#include "hbcssparser_p.h"
#include "hbrepeaticonitem_p.h"
#include "hbnamespace_p.h"


#include "hbanchorlayout.h"
#include <hbanchor.h>

#include <QGraphicsWidget>
#include <hbwidget.h>
#include <hbwidgetbase.h>
#include "hbdeviceprofile.h"
#include "hbrepeatitem_p.h"

#include <QDebug>
#include <QMetaClassInfo>

Q_DECLARE_METATYPE(QGraphicsLayout*)

//Uncomment next define(s) in order to get more debug prints.
//Similar define exists also in the engine side.
//#define HBSTYLE_DEBUG

/*!
    @beta
    @hbcore
    \class HbStyle
    \brief HbStyle is the style management class for widgets

    HbStyle is used for the style management of widgets.
    It offers methods to create and update styled primitives. Briefly, styled primitives
    are HbWidgetBase-derived (which is a QGraphicsWidget) classes which draw a certain part of the user interface (for example
    the button background, or the slider thumb).

    A widget that chooses to be styled needs to create its primitives
    by using the createPrimitive method.

    Whenever the widget's state changes in a way that needs changes in drawing the primitive
    (such as button pressed down -> change the button background image) then the widget needs to
    call the updatePrimitive method.

    Generally primitives should be updated only when a state change occurs. When a widget uses primitives to construct
    itself it does not need a paint() method at all since primitives (widget's children) are doing the drawing. 
    Painting for the primitives occurs from the graphics scene.

    HbStyle has some caching functionality and thus it should not be instantiated explicitly, but 
    accessed only through HbWidget::style() or HbInstance::style() APIs.

*/

/*!

	    \deprecated P_PushButton_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_PushButton_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_PushButton_additionaltext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_PushButton_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_PushButton_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_PushButton_focus
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataGrou\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataGroupComboBackground
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataGrou\deprecated P_heading
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataGrou\deprecated P_description
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataForm_heading
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataForm_heading_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataForm_description
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataGrou\deprecated P_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ToolButton_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ToolButton_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ToolButton_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Slider_thumb
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_increase
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_decrease
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Slider_groove
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Slider_progressgroove
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_checkbox
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_radiobutton
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_selection
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_LineEdit_frame_normal
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_LineEdit_frame_highlight
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TextEdit_frame_normal
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TextEdit_frame_highlight
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Edit_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Label_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Label_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MenuItem_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MenuItem_focus
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MenuItem_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MenuItem_submenuindicator
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MenuItem_checkindicator
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MenuItem_separator
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollBar_groove
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollBar_handle
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollBar_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Popu\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Popu\deprecated P_background_weak
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Popu\deprecated P_heading_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ToolTi\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MessageBox_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ListViewItem_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ListViewItem_richtext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ListViewItem_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemHighlight_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ToolBarExtension_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GridViewItem_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GridViewItem_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_CheckBox_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_CheckBox_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_CheckBox_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Fade_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TitlePane_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TitlePane_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TitlePane_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TitleBar_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_signalicon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_batteryicon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SignalIndicator_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SignalLevel_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SignalLevel_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_BatteryIndicator_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_BatteryLevel_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_BatteryLevel_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_icon1
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_icon2
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_icon3
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorGrou\deprecated P_icon4
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressBar_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressBar_track
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressBar_waittrack
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressBar_mintext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressBar_maxtext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Softkey_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_NavigationButton_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorButton_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndicatorButton_handleindication
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SelectionControl_selectionstart
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SelectionControl_selectionend
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TreeViewItem_expandicon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_Label_richtext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_RatingSlider_track
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_RatingSlider_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_RatingSlider_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


				\deprecated P_ProgressSlider_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


				\deprecated P_ProgressSlider_track
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSlider_slidertrack
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSlider_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSliderHandle_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSliderHandle_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSliderHandle_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_RatingSlider_layout
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollArea_continuationbottom
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollArea_continuationtop
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollArea_continuationleft
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ScrollArea_continuationright
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_focus
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_NotificationDialog_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_NotificationDialog_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_NotificationDialog_title
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_NotificationDialog_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ComboBox_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ComboBoxPopu\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ComboBoxButton_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ComboBox_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataItem_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataItem_separator
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ColorGridViewItem_colorIcon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ColorGridViewItem_borderIcon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ColorGridViewItem_checkIcon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ComboBox_button
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressDialog_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressDialog_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataItem_label
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataItem_description
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_touchincrease
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_touchdecrease
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_touchhandle
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderElement_touchgroove
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderTickMark_majoricon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderTickMark_minoricon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderTickMark_majorlabel
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderTickMark_minorlabel
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_QueryInputMode_image
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GroupBoxHeading_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GroupBoxHeading_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GroupBoxMarquee_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GroupBoxHeading_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_GroupBoxContent_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DataItem_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ItemViewItem_touchmultiselection
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TumbleView_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TumbleView_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_TumbleView_highlight
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DateTimePicker_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DateTimePicker_frame
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_DateTimePicker_separator
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndexFeedback_popu\deprecated P_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_IndexFeedback_popu\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_SliderPopu\deprecated P_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_StatusBar_background
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_StatusBar_timetext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_InputDialog_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_InputDialog_additionaltext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSlider_toucharea
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSlider_track
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSliderHandle_icon
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_InputDialog_additionaltext
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_MessageBox_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressSlider_handle
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).


        \deprecated P_ProgressBar_text
        is deprecated. HbStyle::Primitive enums are deprecated. Primitives can be accessed by item names using HbWidget::primitive(const QString).
*/

// TODO: margins should be defined in layout data once layout specification exists.
static const int ItemName = 0xfffe;
static const QString STYLE_LOCATION = QLatin1String(":/themes/style/hbdefault/rules/widgets/%w/%w");
static const QString COLOR_STYLE_LOCATION = QLatin1String(":/themes/style/hbdefault/rules/widgets/%w/%w_color");
static const int TOUCHAREA_ZVALUE = 1000;

static const QString GLOBAL_PARAMETERS_LOCATION = QLatin1String(":/themes/style/hbdefault/variables/layout/zoom/0/hbglobalparameters.css");

static const char* LAYOUT_PTR_PROPERTY = "HbStyle_layout_ptr";
static const char* LAYOUT_NAME_PROPERTY = "HbStyle_layout_name";

inline void overrideSpacing( HbAnchorLayout *layout, const QString &name, Hb::Edge edge, qreal val )
{
    if( name.isNull() ) {
        return;
    }
    QList<HbAnchor*> list = layout->anchors();
    for( int i = 0; i < list.size(); i++ ) {
        HbAnchor *anchor = list.at(i);
        if( ( anchor->startNodeId() == name ) && ( anchor->startEdge() == edge ) ) {
            anchor->setDirection( val < 0 ? HbAnchor::Negative : HbAnchor::Positive );
            anchor->setPreferredLength( qAbs(val) );
            if (anchor->anchorId().isEmpty()) {
                // assuming it's a fixed anchor
                anchor->setSizePolicy( QSizePolicy::Fixed );
            } else {
                // assuming it's a "spacer" and we want to override the minimum length
                anchor->setMinimumLength( qAbs(val) );
            }
        }
    }
}

/*!
Constructor

Should not be called explicitly. Use HbWidget::style() or HbInstance::style() instead.
*/
HbStyle::HbStyle() :
    d_ptr(new HbStylePrivate)
{
    Q_D( HbStyle );
    d->q_ptr = this;
}

/*!
Destructor
*/
HbStyle::~HbStyle()
{
    delete d_ptr;
}


/*!

  \deprecated HbStyle::createPrimitive(HbStyle::Primitive, QGraphicsItem*)
  is deprecated. This method will be replaced with an altered version which will use new base primitive enumerations.

  Creates instances of primitive graphics items. This method should be used by all widgets that support styling.
  When changing the style the returned primitives can be replaced with an altered version of the primitives, or with a completely
  different primitive, to create a custom appearance. This shouldn't cause changes to the widget if the functional design
  remains the same.

  This method returns HbWidgetBase objects. A widget can store the returned items as HbWidgetBase. If a widget
  does not require any functionality from HbWidgetBase then it should store the primitives as QGraphicsItems.

  \note This method is used to replace the conventional Qt-style paint indirection via drawPrimitive.
  \sa HbStyle::updatePrimitive method which is to be used in conjunction with this method
  \sa HbWidgetBase
  \param primitive, to identify the primitive to create
  \param parent of the item
  \return HbWidgetBase is returned.

 */
QGraphicsItem *HbStyle::createPrimitive( HbStyle::Primitive primitive, QGraphicsItem *parent ) const
{

        switch (primitive){
            case P_MenuItem_submenuindicator:
                {
                HbIconItem *item = new HbIconItem(parent);
                setItemName(item, QLatin1String("submenu-indicator"));
                return item;
                }

            case P_MenuItem_checkindicator:
                {
                HbIconItem *item = new HbIconItem(parent);
                setItemName(item, QLatin1String("check-indicator"));
                return item;
                }

            case P_MenuItem_separator:
                {
                HbIconItem *item = new HbIconItem(parent);
                setItemName(item, QLatin1String("separator"));
                return item;
                }
            case P_ToolButton_icon:
            case P_PushButton_icon:
            case P_ProgressDialog_icon: {
                HbIconItem *iconItem = new HbIconItem(HbIcon(),parent);
                setItemName(iconItem, QLatin1String("icon"));
                return iconItem;
                }

            case P_CheckBox_icon:
            case P_GroupBoxHeading_icon:
            case P_Label_icon:
            case P_GridViewItem_icon: {
                HbIconItem *item = q_check_ptr(new HbIconItem(HbIcon(),parent));
                setItemName(item, QLatin1String("icon"));
                return item;
                }
            case P_DataGroup_icon:
                {
                HbIconItem *item = new HbIconItem(HbIcon(), parent);
                setItemName(item, QLatin1String("dataGroup_Icon"));
                return item;
                }
            case P_DataItem_icon:
                {
                HbIconItem *item = new HbIconItem(HbIcon(), parent);
                setItemName(item, QLatin1String("dataItem_Icon"));
                return item;
                }
            case P_ComboBox_text:
                {
                    HbTextItem *textItem = new HbTextItem(parent);
                    setItemName(textItem, QLatin1String("combobox_labelfield"));
                    return  textItem;
                }
            case P_ToolButton_text:
            case P_CheckBox_text:
            case P_ProgressDialog_text:
                {
                    HbTextItem *textItem = new HbTextItem(parent);
                    setItemName(textItem, QLatin1String("text"));
                    return  textItem;
                }
            case P_PushButton_text:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("text"));
                return  ti;
                }
            case P_DataGroup_heading:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataGroup_HeadingLabel"));
                return  ti;
                }
            case P_DataGroup_description:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataGroup_Description"));
                return  ti;
                }
            case P_DataForm_heading:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataForm_Heading"));
                //ti->setZValue(2);
                return  ti;
                }
            case P_DataForm_description:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataForm_Desc"));
                return  ti;
                }
            case P_PushButton_additionaltext:
                {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("additional-text"));
                return  ti;
                }
            case P_ProgressSlider_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                ta->setZValue(TOUCHAREA_ZVALUE);
                return ta;
                }
            case P_ProgressSliderHandle_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                ta->setZValue(TOUCHAREA_ZVALUE);
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }

            case P_ProgressSliderHandle_icon:
                {
                HbIconItem *item = new HbIconItem(QLatin1String("qtg_graf_progslider_handle_normal"), parent);
                item->setAspectRatioMode(Qt::IgnoreAspectRatio);
                return item;
                }
            case P_PushButton_toucharea: // Generic implementation, can add other cases here
            case P_CheckBox_toucharea:
            case P_RatingSlider_toucharea:
            case P_SliderElement_touchhandle:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }
            case P_ScrollBar_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlags(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                // NOTE:: This is a temporary fix
                //if(parent){
                //    parent->setHandlesChildEvents(true);
                //}
                return ta;
                }
            case P_ComboBoxButton_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("combobox_button_toucharea"));
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }
             case P_TitleBar_toucharea: 
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                return ta;
                }
             case P_IndicatorButton_toucharea:
             case P_TitlePane_toucharea:
             case P_NavigationButton_toucharea:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                setItemName(ta, QLatin1String("toucharea"));
                if (parent){
                    parent->setHandlesChildEvents(true);
                }
                return ta;
                }
             case P_SliderElement_touchdecrease:
             case P_SliderElement_touchincrease:
             case P_SliderElement_touchgroove:
                {
                HbTouchArea *ta = new HbTouchArea(parent);
                ta->setFlag(QGraphicsItem::ItemIsFocusable);
                if(parent){
                    parent->setFiltersChildEvents(true);
                }
                return ta;
                }
            case P_SliderElement_text:
            case P_ProgressBar_text:
            {
                HbTextItem *ti = new HbTextItem(parent);
                return  ti;
            }
            case P_DataItem_label:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataItem_Label"));
                return  ti;
            }
            case P_DataItem_description:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("dataItem_Description"));
                return  ti;
            }
            case P_ProgressBar_mintext:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("min-text"));
                return  ti;
            }
            case P_ProgressBar_maxtext:
            {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("max-text"));
                return  ti;
            }
            case P_Label_text: {
                HbTextItem *ti = new HbTextItem(parent);
                setItemName(ti, QLatin1String("text"));
                ti->setAlignment(Qt::AlignCenter);
                return ti;
            }

            case P_Label_richtext:
            {
                HbRichTextItem *rti = new HbRichTextItem(parent);
                setItemName(rti, QLatin1String("text"));
                return rti;
            }

            case P_MenuItem_text:
                {
                HbTextItem *item = new HbTextItem(parent);
                setItemName(item, QLatin1String("text"));

                return item;
                }

            case P_Slider_thumb:
                return new HbIconItem(parent);

            case P_MenuItem_frame:
            case P_MenuItem_focus:
            case P_ScrollBar_groove:
            case P_ScrollBar_handle:
            case P_Slider_groove:
            case P_Popup_background:
            case P_Popup_background_weak:
            case P_Popup_heading_frame:
            case P_NotificationDialog_frame:
            case P_ToolTip_background:
            case P_PushButton_focus:
            case P_ComboBox_background:
            case P_ToolBarExtension_background:
            case P_SliderPopup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                return n;
            }

            case P_ProgressBar_frame:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                n->frameDrawer().setFillWholeRect(true);
                setItemName(n, "frame");
                return n;
            }

            case P_ComboBoxPopup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n , "dropdown_background");
                return n;
            }

            case P_TumbleView_background:
            case P_TumbleView_frame:
            case P_DateTimePicker_background:
            case P_DateTimePicker_frame:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-5);
                return n;
            }
            case P_TumbleView_highlight:
            {
                return new HbFrameItem(parent);//TODO:make it icon once iconitem setGeomoetry works for tumbleview
            }

            case P_DateTimePicker_separator:{
                HbFrameItem *frameItem= new HbFrameItem(parent);//TODO:make it icon once iconitem setGeomoetry works for tumbleview
                return frameItem;
            }
            case P_DataGroup_background :
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("dataGroup_Background"));
                return n;
            }
            case P_DataGroupComboBackground:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("dataGroup_ComboBackground"));
                return n;
            }
            case P_DataForm_heading_background :
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("dataFormHeading_Background"));
                return n;
            }
            case P_DataItem_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-2);
                setItemName(n, QLatin1String("dataItem_Background"));
                return n;
            }
            case P_GroupBoxContent_background:
            case P_GroupBoxHeading_background:
            case P_PushButton_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_ProgressBar_track:
            {
                HbProgressTrackItem *n = new HbProgressTrackItem(parent);
                n->frameDrawer().setFillWholeRect(true);
                n->setZValue(-2);
                return n;
            }
            case P_ProgressSlider_slidertrack:
            {
                HbProgressTrackItem *n = new HbProgressTrackItem(parent);
                n->setZValue(-1);
                return n;
            }
            case P_Slider_progressgroove:
            {
                HbSliderTrackItem *n=new HbSliderTrackItem(parent);
                n->setZValue(0);
                return n;
            }
            case P_ToolButton_frame:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_NavigationButton_background:
            case P_IndicatorButton_background:
            {
                HbIconItem *n = new HbIconItem(HbIcon(), parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_IndicatorButton_handleindication:
            {
                HbIconItem *n = new HbIconItem(parent);
                setItemName(n, QLatin1String("handleindication"));
                return n;
            }
            case P_IndicatorButton_eventindication:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("eventindication"));
                return n;
            }
            case P_IndicatorGroup_icon1:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon1"));
                return n;
            }
            case P_IndicatorGroup_icon2:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon2"));
                return n;
            }
            case P_IndicatorGroup_icon3:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon3"));
                return n;
            }
            case P_IndicatorGroup_icon4:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon4"));
                return n;
            }
            case P_SignalIndicator_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_SignalLevel_background:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_SignalLevel_icon:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                setItemName(n, QLatin1String("levelicon"));
                return n;
            }
            case P_BatteryIndicator_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_BatteryLevel_background:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_BatteryLevel_icon:
            {
                HbIndicatorLevelIconItem *n = new HbIndicatorLevelIconItem(parent);
                setItemName(n, QLatin1String("levelicon"));
                return n;
            }
            case P_TitlePane_background:
                {
                    HbFrameItem *n = new HbFrameItem(parent);
                    n->setZValue(-1);
                    setItemName(n, QLatin1String("background"));
                    return n;
                }
            case P_LineEdit_frame_normal:
            case P_LineEdit_frame_highlight:
            case P_TextEdit_frame_normal:
            case P_TextEdit_frame_highlight:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                return n;
            }
            case P_GroupBoxMarquee_text:
            case P_TitlePane_text:
            {
                HbMarqueeItem *n = new HbMarqueeItem(parent);
                setItemName(n, QLatin1String("text"));
                return n;
            }
            case P_TitlePane_icon:
            {
                HbIconItem *n = new HbIconItem(parent);
                n->setFlags(HbIcon::Colorized);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_StatusBar_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->setZValue(-1);
                setItemName(n, QLatin1String("background"));
                return n;
            }
            case P_StatusBar_timetext:
            {
                HbTextItem *textItem = new HbTextItem(parent);
                setItemName(textItem, QLatin1String("timetext"));
                textItem->setAlignment(Qt::AlignCenter);
                return textItem;
            }
            case P_Fade_background:
                return new HbIconItem(QString(), parent);
            case P_SliderElement_icon:
            case P_SliderElement_increase:
            case P_SliderElement_decrease:
                return new HbIconItem(QString(), parent);

            case P_SliderTickMark_majoricon: {
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem,QLatin1String("tickmark-majoricon"));
                return iconItem;
            }
            case P_SliderTickMark_minoricon: {
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem,QLatin1String("tickmark-minoricon"));
                return iconItem;
            }
            case P_SliderTickMark_majorlabel: {
                HbTextItem *textItem = new HbTextItem(parent);
                setItemName(textItem,QLatin1String("tickmark-majorlabel"));
                return textItem;
            }
            case P_SliderTickMark_minorlabel: {
                HbTextItem *textItem = new HbTextItem(parent);
                setItemName(textItem,QLatin1String("tickmark-minorlabel"));
                return textItem;
            }

            case P_ProgressBar_waittrack:
                return new HbRepeatIconItem(QLatin1String("qtg_graf_progbar_h_wait"), parent);

            case P_RatingSlider_frame:
                return  new HbRepeatItem(parent);

            case P_RatingSlider_track:
                return new HbRepeatMaskItem(parent);

            case P_RatingSlider_layout: {
                HbWidgetBase *layout = new HbWidgetBase(parent);
                setItemName(layout, "frame");
                return layout;
            }

            case P_ItemViewItem_checkbox: {
                qDebug() << "Primitive P_ItemViewItem_checkbox is deprecated and will cease to exist in the near future.";
                HbIconItem *iconItem = new HbIconItem(parent);
                setItemName(iconItem, QLatin1String("checkbox-icon"));
                return iconItem;
            }

            case P_ItemViewItem_radiobutton:
            case P_ItemViewItem_selection: {
                HbIconItem *iconItem = q_check_ptr(new HbIconItem(parent));
                setItemName(iconItem, QLatin1String("selection-icon"));
                return iconItem;
            }

            case P_Edit_text:{
                return 0;
            }
            case P_GroupBoxHeading_text:
            case P_NotificationDialog_text:{
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("text"));
                n->setMinimumLines(1);
                n->setMaximumLines(1);
                n->setTextWrapping(Hb::TextNoWrap);
                return n;
            }
            case P_NotificationDialog_title:{
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("title"));
                return n;
            }
            case P_NotificationDialog_icon:{
                HbIconItem *n = new HbIconItem(QString(), parent);
                setItemName(n, QLatin1String("icon"));
                return n;
            }
            case P_MessageBox_text:{    //deprecated
                HbTextItem *rti = new HbTextItem(parent);
                return rti;
            }
            case P_MessageBox_icon:{
                HbIconItem *n = new HbIconItem(QString(), parent);
                return n;
            }

            case P_ItemViewItem_background: {
                HbIconItem *iconItem = q_check_ptr(new HbIconItem(parent));
                iconItem->setZValue(-3.0);
                setItemName(iconItem, QLatin1String("background"));
                return iconItem;
            }

            case P_ListViewItem_text: {
                HbTextItem *textItem = q_check_ptr(new HbTextItem(parent));

                return textItem;
            }

            case P_ListViewItem_richtext: {
                HbRichTextItem *textItem = q_check_ptr(new HbRichTextItem(parent));
                return textItem;
            }

            case P_GridViewItem_text: {
                HbTextItem *textItem = q_check_ptr(new HbTextItem(parent));
                // caching do not work properly - text is not refreshed immediatelly
                //textItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
                setItemName(textItem, QLatin1String("text"));
                return textItem;
            }

            case P_ListViewItem_icon:
                return q_check_ptr(new HbIconItem(parent));

            case P_ColorGridViewItem_colorIcon:
                return new HbIconItem("qtg_graf_colorpicker_mask", parent);
            case P_ColorGridViewItem_borderIcon:
                return new HbIconItem("qtg_graf_colorpicker_filled", parent);
            case P_ColorGridViewItem_checkIcon:
                return new HbIconItem("qtg_small_tick", parent);

            case P_TreeViewItem_expandicon: {
                HbIconItem *iconItem = q_check_ptr(new HbIconItem(parent));
                setItemName(iconItem, QLatin1String("subitem-indicator"));
                return iconItem;
            }

            case P_SelectionControl_selectionstart://fallthrough
            case P_SelectionControl_selectionend://fallthrough
            {
                HbIconItem *iconItem = new HbIconItem(parent);
                switch (primitive) {
                    case P_SelectionControl_selectionstart:
                        iconItem->setIconName(QLatin1String("qtg_graf_editor_handle_begin"));
                        break;
                    case P_SelectionControl_selectionend:
                        iconItem->setIconName(QLatin1String("qtg_graf_editor_handle_end"));
                        break;
                    default:
                        qWarning("Unknown HbSelectionControl primitive %i", primitive);
                        break;
                }
                setItemName(iconItem, QLatin1String("handle-icon"));
                return iconItem;
            }
            case P_ComboBox_button: {
                HbIconItem *n = new HbIconItem(QString(), parent);
                return n;
            }
            case P_ItemViewItem_focus: {
                HbFrameItem *frameItem = q_check_ptr(new HbFrameItem(parent));
                frameItem->setZValue(-1.0);
                return frameItem;
            }
            case P_ItemHighlight_background:
                return new HbFrameItem(parent);

            case P_ItemViewItem_frame: {
                HbFrameItem *item = q_check_ptr(new HbFrameItem(parent));
                item->setZValue(-4.0);
                setItemName(item,"frame");
                return item;
            }
            case P_ScrollArea_continuationbottom:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-bottom"));
                n->setZValue(-1);
                return n;
            }
            case P_ScrollArea_continuationtop:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-top"));
                n->setZValue(-1);
                return n;
            }
            case P_ScrollArea_continuationleft:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-left"));
                n->setZValue(-1);
                return n;
            }
            case P_ScrollArea_continuationright:
                {
                HbFrameItem *n = new HbFrameItem(parent);
                setItemName(n, QLatin1String("continuation-indicator-right"));
                n->setZValue(-1);
                return n;
            }
            case P_ItemViewItem_touchmultiselection:
            {
                HbTouchArea *area = q_check_ptr(new HbTouchArea(parent));
                setItemName(area, "multiselection-toucharea");
                return area;
            }
            case P_IndexFeedback_popup_text:
            {
                HbTextItem *textItem = new HbTextItem(parent);
                textItem->setAlignment(Qt::AlignCenter);
                textItem->setTextWrapping(Hb::TextNoWrap);
                setItemName(textItem, QLatin1String("index-text"));

                //TODO:: make this a sane value
                textItem->setZValue(4);
                return textItem;
            }
            case P_IndexFeedback_popup_background:
            {
                HbFrameItem *n = new HbFrameItem(parent);
                n->frameDrawer().setFrameGraphicsName("qtg_fr_popup_preview");
                n->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                qreal cornerPieceSize = 0;
                parameter(QLatin1String("hb-param-background-popup-preview"),cornerPieceSize);
                n->frameDrawer().setBorderWidths(cornerPieceSize, cornerPieceSize);
                setItemName(n, QLatin1String("index-background"));
                n->setZValue(3);
                return n;
            }
            case P_InputDialog_text:
            {
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("label-1"));
                return n;
            }
            case P_InputDialog_additionaltext:
            {
                HbTextItem *n = new HbTextItem(parent);
                setItemName(n, QLatin1String("label-2"));
                return n;
            }
            default:
                return 0;
        }
}

/*!

  \deprecated HbStyle::updatePrimitive(QGraphicsItem*, HbStyle::Primitive, const QStyleOption*)
  is deprecated. This method will be made private and finally removed since primitive updating will be done in the widgets.

  Updates the state and content of widget's child primitives. Update for a styled primitive should happen always when
  a state change that affects drawing occurs. Such a situation can be for example pressing of a button (change background image), or
  changing a text for a widget having text content. The updatePrimitive() implementation for each primitive element can be considered
  as a part of widget's implementation. Note that it's up to the widget to decide what the styleoption contains and what the updatePrimitive() method
  uses the styleoption data for.

  \sa HbStyle::createPrimitive
  \param item Primitive graphicsitem.
  \param primitive To identify the primitive to create.
  \param option Style option, contains all the information needed to update the primitive, this
  information is widget specific and each widget usually has a styleoption. Styleoption may include
  information about the widget's state, content etc.
 */
void HbStyle::updatePrimitive( QGraphicsItem *item, HbStyle::Primitive primitive, const QStyleOption *option ) const
{
    Q_D( const HbStyle );

    switch(primitive){
            case P_PushButton_icon:
                {
                    if (const HbStyleOptionPushButton *opt =
                        qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        //iconItem->setIconName(opt->icon.iconName());
                        iconItem->setIcon(opt->icon); // with this call iconitem refresh issue is there
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                        iconItem->setAlignment(Qt::AlignCenter);
                    }
                break;
                }
            case P_GroupBoxHeading_icon:
                {
                    if (const HbStyleOptionGroupBox *opt =
                        qstyleoption_cast<const HbStyleOptionGroupBox*>(option)) {
                            HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                            if (opt->collapsed) {
                                iconItem->setIconName(QLatin1String("qtg_small_expand"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_small_collapse"));
                            }
                    }
                    break;
                }
            case P_DataGroup_icon:
                {
                    if (const HbStyleOptionDataGroupHeadingWidget *opt =
                            qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        if (opt->expanded) {
                            iconItem->setIconName(QLatin1String("qtg_small_collapse"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_small_expand"));
                        }
                    }
                break;
                }
            case P_DataItem_icon:
                {
                    const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option);
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    break;
                }
            case P_Label_icon:
                if (const HbStyleOptionLabel *opt = qstyleoption_cast<const HbStyleOptionLabel*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    if (!opt->icon.isNull()) {
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                        iconItem->setAspectRatioMode(opt->aspectRatioMode);
                        iconItem->setAlignment(opt->alignment);
                    }
                }
                break;
            case P_ToolButton_icon:
                if (const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    iconItem->setMode(d->iconMode(opt->state));
                    iconItem->setState(d->iconState(opt->state));
                }
                break;
            case P_ComboBox_text:
                if (const HbStyleOptionComboBox *opt = qstyleoption_cast<const HbStyleOptionComboBox*>(option)) {
                    HbTextItem *comboTextItem = static_cast<HbTextItem*>(item);
                    comboTextItem->setText(opt->text);
                    comboTextItem->setTextWrapping(Hb::TextNoWrap);
                }
                break;
            case P_PushButton_text:
                {
                    if (const HbStyleOptionPushButton *opt =
                        qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->text);
                        //default alignment will come from css,
                        //if api flag is set ,then alignment from style is taken.
                        if (opt->hasTextAlignment) {
                            textItem->setAlignment( opt->textAlignment );
                        }
                        textItem->setTextWrapping(Hb::TextWordWrap);
                    }
                }
                break;
            case P_ProgressDialog_text:
                {
                    if (const HbStyleOptionProgressDialog *opt =
                        qstyleoption_cast<const HbStyleOptionProgressDialog*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->text);
                    }
                break;
                }
            case P_PushButton_additionaltext:
                {
                    if (const HbStyleOptionPushButton *opt =
                            qstyleoption_cast<const HbStyleOptionPushButton*>(option)) {
                        HbTextItem *additionalTextItem = static_cast<HbTextItem*>(item);
                        additionalTextItem->setText(opt->additionalText);
                        //default alignment will come from css,
                        //if api flag is set ,then alignment from style is taken.
                        if (opt->hasAdditionalTextAlignment) {
                            additionalTextItem->setAlignment( opt->additionalTextAlignment );
                        }
                        additionalTextItem->setTextWrapping(Hb::TextWordWrap);
                    }
                }
                break;
            case P_PushButton_toucharea:  // Generic implementation, can add other cases here
            case P_ComboBoxButton_toucharea:
            case P_CheckBox_toucharea:
            case P_TitleBar_toucharea:
            case P_IndicatorButton_toucharea:
            case P_TitlePane_toucharea:
            case P_NavigationButton_toucharea:
            case P_SliderElement_touchdecrease:
            case P_SliderElement_touchincrease:
            case P_SliderElement_touchgroove:
            case P_SliderElement_touchhandle:
            case P_ScrollBar_toucharea:
                {
                    HbTouchArea *touchArea = static_cast<HbTouchArea*>(item);
                    touchArea->setZValue(TOUCHAREA_ZVALUE);
                }
                break;
            case P_DataGroup_heading:
                {
                   if (const HbStyleOptionDataGroupHeadingWidget *opt =
                        qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->heading);
                        textItem->setTextWrapping(Hb::TextNoWrap);
                    }
                break;
                }
            case P_DataGroup_description:
                {
                   if (const HbStyleOptionDataGroupHeadingWidget *opt =
                        qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->description);
                    }
                break;
                }
            case P_DataForm_heading:
                {
                   if (const HbStyleOptionDataForm *opt =
                        qstyleoption_cast<const HbStyleOptionDataForm*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setText(opt->heading);
                    }
                break;
                }
            case P_DataForm_description:
                {
                   if (const HbStyleOptionDataForm *opt =
                        qstyleoption_cast<const HbStyleOptionDataForm*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        HbFontSpec spec(HbFontSpec::Secondary);
                        QFont descriptionFont = spec.font();
                        descriptionFont.setItalic(true);
                        textItem->setFont(descriptionFont);
                        textItem->setText(opt->description);
                    }
                break;
                }

            case P_DataForm_heading_background: {
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                if(!frameItem){
                    return;
                }
                frameItem->setZValue(-1.0);
                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_form_heading"));
                frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                break;
            }
            case P_ToolButton_text:
                if (const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setText(opt->text);
                }
                break;
            case P_ProgressBar_text: {
                  if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar*>(option)) {
                      HbTextItem *textItem = static_cast<HbTextItem*>(item);
                      textItem->setAlignment(Qt::AlignCenter);
                      textItem->setZValue(100);
                      textItem->setText(opt->text);
                  }
                  break;
              }
            case P_DataGroup_background: {
                if (const HbStyleOptionDataGroupHeadingWidget *opt =
                    qstyleoption_cast<const HbStyleOptionDataGroupHeadingWidget *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    if(!frameItem){
                        return;
                    }
                    QString frameGraphicsName;
                    QIcon::Mode mode = QIcon::Disabled;
                    if (opt->state & QStyle::State_Enabled) {
                        mode = QIcon::Normal;
                    }
                    if (opt->state & QStyle::State_Active) {
                        mode = QIcon::Active;
                    } 
                    if (opt->state & QStyle::State_Selected) {
                        mode = QIcon::Selected;
                    }
                    frameItem->setZValue(-1.0);
                    if(mode == QIcon::Disabled) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_disabled"));
                    }                   
                    else if(opt->pressed ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_pressed"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_parent_normal"));
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->setGeometry(opt->rect);
                }
                break;
            }
            case P_DataGroupComboBackground: {
                if (const HbStyleOptionDataGroup *opt = qstyleoption_cast<const HbStyleOptionDataGroup *>(option)) {
                    QString frameGraphicsName;
                    QIcon::Mode mode = QIcon::Disabled;
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    if(!frameItem){
                        return;
                    }
                    if (opt->state & QStyle::State_Enabled) {
                        mode = QIcon::Normal;
                    }
                    if (opt->state & QStyle::State_Active) {
                        mode = QIcon::Active;
                    }
                    if (opt->state & QStyle::State_Selected) {
                        mode = QIcon::Selected;
                    }
                    if (mode == QIcon::Disabled ) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_disabled");
                        } else  {
                            frameGraphicsName = QLatin1String("qtg_fr_list_parent_normal");
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
                break;
            }

            case P_GroupBoxHeading_background: {
                if (const HbStyleOptionGroupBox *opt = qstyleoption_cast<const HbStyleOptionGroupBox *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );

                    if(!frameItem){
                        return;
                    }
                    QString frameGraphicsName;
                    if(opt->groupBoxType == HbStyleOptionGroupBox::GroupBoxSimpleLabel)
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox");
                    else if(opt->groupBoxType == HbStyleOptionGroupBox::GroupBoxCollapsingContainer){
                        if (opt->state == QStyle::State_On) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_pressed");
                        } else if (opt->state == QStyle::State_Off) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_parent_normal");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_list_highlight");
                        }
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                }
                break;
            }

            case P_GroupBoxContent_background: {
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );

                    if(!frameItem){
                        return;
                    }

                    QString frameGraphicsName;
                    if (opt->state == QStyle::State_On) {
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox_pressed");
                    } else if (opt->state == QStyle::State_Off) {
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox_normal");
                    } else {
                        frameGraphicsName = QLatin1String("qtg_fr_groupbox_highlight");
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                }
                break;
            }

            case P_DataItem_background: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    QString frameGraphicsName;
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    if(!frameItem){
                        return;
                    } 
                    QIcon::Mode mode = QIcon::Disabled;
                    if (opt->state & QStyle::State_Enabled) {
                        mode = QIcon::Normal;
                    }
                    if (opt->state & QStyle::State_Active) {
                        mode = QIcon::Active;
                    }
                    if (opt->state & QStyle::State_Selected) {
                        mode = QIcon::Selected;
                    }
                    if (mode == QIcon::Disabled ) {
                            frameGraphicsName = QLatin1String("qtg_fr_list_disabled");
                        } else  {
                            frameGraphicsName = QLatin1String("qtg_fr_list_parent_normal");
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
                break;
            }
            case P_DataItem_label: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    HbTextItem *textitem = static_cast<HbTextItem*>( item );
                    textitem->setText(opt->label);
                    textitem->setTextWrapping(Hb::TextNoWrap);
                }
                break;
            }
            case P_DataItem_description: {
                if (const HbStyleOptionDataFormViewItem *opt = qstyleoption_cast<const HbStyleOptionDataFormViewItem *>(option)) {
                    HbTextItem *textitem = static_cast<HbTextItem*>( item );
                    textitem->setText(opt->description);
                }
                break;
            }
            case P_PushButton_background: {
                if (const HbStyleOptionPushButton *opt =
                    qstyleoption_cast<const HbStyleOptionPushButton *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    QString frameGraphicsName;
                    QIcon::Mode mode = QIcon::Disabled;
                    QIcon::State state = QIcon::Off;
                    if (opt->state & QStyle::State_Enabled)
                        mode = QIcon::Normal;
                    if (opt->state & QStyle::State_Active)
                        mode = QIcon::Active;
                    if (opt->state & QStyle::State_Selected)
                        mode = QIcon::Selected;
                    if (opt->state & QStyle::State_On)
                        state = QIcon::On;

                    frameItem->setZValue(-1.0);
                    if(opt->backgroundFrameDrawer &&!(opt->backgroundFrameDrawer->isNull()))
                    {
                        frameItem->setFrameDrawer(opt->backgroundFrameDrawer);
                    }
                    else if (!opt->background.isNull()) {
                        QString customName = opt->background.iconName(mode, state);
                        frameItem->frameDrawer().setFrameGraphicsName(customName);
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        break;
                    }
                    else{
                        if (mode == QIcon::Disabled && state == QIcon::Off) {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_disabled");
                        } else if (mode == QIcon::Normal && state == QIcon::On) {
                            if(!opt->isCheckable){
                                frameGraphicsName = QLatin1String("qtg_fr_btn_pressed");
                            } else {
                                frameGraphicsName = QLatin1String("qtg_fr_btn_latched");
                            }
                        } else if (mode == QIcon::Selected && state == QIcon::Off) {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_highlight");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_normal");
                        }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    }
                }
                break;
            }

            case P_TitlePane_background:{
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);

                if (opt) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    if (opt->mode == QIcon::Active) {
                        if (opt->transparent) {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_trans_pressed"));
                        } else {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_pressed"));
                        }
                    } else if (opt->mode == QIcon::Selected) {
                        if (opt->transparent) {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_trans_latched"));
                        } else {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_latched"));
                        }
                    } else {
                        if (opt->transparent) {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_trans_normal"));
                        } else {
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_titlebar_normal"));
                        }
                    }
                    frameItem->frameDrawer().setFileNameSuffixList(QStringList() << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_cr"));
                }
                break;
            }
            case P_StatusBar_background: {
                const HbStyleOptionStatusBar *opt = qstyleoption_cast<const HbStyleOptionStatusBar *>(option);
                if (opt) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    if (opt->transparent) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_statusbar_trans"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_statusbar"));
                    }
                }
                break;
            }
            case P_StatusBar_timetext: {
                if (const HbStyleOptionStatusBar *opt =
                    qstyleoption_cast<const HbStyleOptionStatusBar*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setText(opt->timeText);
                    }
                break;
            }

            case P_IndicatorGroup_icon1:
            case P_IndicatorGroup_icon2:
            case P_IndicatorGroup_icon3:
            case P_IndicatorGroup_icon4:
                {
                const HbStyleOptionIndicatorGroup *opt = qstyleoption_cast<const HbStyleOptionIndicatorGroup *>(option);
                if (opt) {
                    HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                    iconItem->setIconName(opt->iconName);
                }
                break;
            }

            case P_IndicatorButton_handleindication: {
                const HbStyleOptionIndicatorButton *opt = qstyleoption_cast<const HbStyleOptionIndicatorButton *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_graf_titlebar_minimize");
                }
                break;
            }

            case P_IndicatorButton_eventindication: {  
                const HbStyleOptionIndicatorButton *opt = qstyleoption_cast<const HbStyleOptionIndicatorButton *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if (opt->twoIcons) {
                        iconItem->setIconName("qtg_mono_new_event");
                        iconItem->setVisible(true);
                    } else {
                        iconItem->setVisible(false);
                    }
                }
                break;
            }

            case P_SignalIndicator_icon: {
                const HbStyleOptionSignalIndicator *opt = qstyleoption_cast<const HbStyleOptionSignalIndicator *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    switch (opt->networkMode) {
                        case 0:
                            iconItem->setIconName("qtg_mono_offline");
                            break;
                        case 1:
                            iconItem->setIconName("qtg_status_signal");
                            break;
                        case 2:
                            iconItem->setIconName("qtg_status_wcdma");
                            break;
                        case 3:
                            iconItem->setIconName("qtg_status_wcdma");
                            break;
                        case 5:
                            iconItem->setIconName("qtg_mono_wlan");
                            break;
                        default:
                            iconItem->setIconName("qtg_mono_offline");
                            break;
                    }
                }
                break;
            }
            case P_SignalLevel_background: {
                const HbStyleOptionSignalIndicator *opt = qstyleoption_cast<const HbStyleOptionSignalIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    iconItem->setIconName("qtg_graf_signal_bg");
                    iconItem->setValue(100 - opt->signalValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::RightToLeft);
                    iconItem->update();
                }
                break;
            }
            case P_SignalLevel_icon: {
                const HbStyleOptionSignalIndicator *opt = qstyleoption_cast<const HbStyleOptionSignalIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    switch (opt->signalLevel) {
                        case HbStyleOptionSignalIndicator::Zero:
                            iconItem->setIconName("qtg_graf_signal_bg");
                            break;
                        case HbStyleOptionSignalIndicator::Low:
                            iconItem->setIconName("qtg_graf_signal_low");
                            break;
                        case HbStyleOptionSignalIndicator::Medium:
                            iconItem->setIconName("qtg_graf_signal_medium");
                            break;
                        case HbStyleOptionSignalIndicator::Full:
                            iconItem->setIconName("qtg_graf_signal_full");
                            break;
                        default:
                            iconItem->setIconName("qtg_graf_signal_bg");
                            break;
                    }
                    iconItem->setValue(opt->signalValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::LeftToRight);
                    iconItem->update();
                }
                break;
            }
            case P_BatteryIndicator_icon: {
                if (option) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_status_battery");
                }
                break;
            }
            case P_BatteryLevel_background: {
                const HbStyleOptionBatteryIndicator *opt = qstyleoption_cast<const HbStyleOptionBatteryIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    iconItem->setIconName("qtg_graf_battery_bg");
                    iconItem->setValue(100 - opt->batteryValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::LeftToRight);
                    iconItem->update();
                }
                break;
            }
            case P_BatteryLevel_icon: {
                const HbStyleOptionBatteryIndicator *opt = qstyleoption_cast<const HbStyleOptionBatteryIndicator *>(option);
                if (opt) {
                    HbIndicatorLevelIconItem *iconItem = static_cast<HbIndicatorLevelIconItem*>(item);
                    switch (opt->batteryLevel) {
                        case HbStyleOptionBatteryIndicator::Zero:
                            iconItem->setIconName("qtg_graf_battery_bg");
                            break;
                        case HbStyleOptionBatteryIndicator::Low:
                            iconItem->setIconName("qtg_graf_battery_low");
                            break;
                        case HbStyleOptionBatteryIndicator::Medium:
                            iconItem->setIconName("qtg_graf_battery_medium");
                            break;
                        case HbStyleOptionBatteryIndicator::Full:
                            iconItem->setIconName("qtg_graf_battery_full");
                            break;
                        default:
                            iconItem->setIconName("qtg_graf_battery_bg");
                            break;
                    }
                    iconItem->setValue(opt->batteryValue);
                    iconItem->setDirection(HbIndicatorLevelIconItem::RightToLeft);
                    iconItem->update();
                }
                break;
            }

            case P_NavigationButton_background: {
                const HbStyleOptionNavigationButton *opt = qstyleoption_cast<const HbStyleOptionNavigationButton *>(option);
                    if (opt) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                        iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                        if (opt->transparent) {
                            if (opt->mode == QIcon::Active) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_r"));
                            }
                        } else {
                            if (opt->mode == QIcon::Active) {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_r"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_r"));
                            }
                        }
                    }
                break;
            }

            case P_IndicatorButton_background: {
                const HbStyleOptionIndicatorButton *opt = qstyleoption_cast<const HbStyleOptionIndicatorButton *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                    if (opt->mode == QIcon::Active) {
                        if (opt->transparent) {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_pressed_l"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_pressed_l"));
                        }
                    } else if (opt->mode == QIcon::Selected) {
                        if (opt->transparent) {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_latched_l"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_latched_l"));
                        }
                    } else {
                        if (opt->transparent) {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_trans_normal_l"));
                        } else {
                            iconItem->setIconName(QLatin1String("qtg_fr_titlebar_normal_l"));
                        }
                    }
                }
                break;
            }

            case P_TitlePane_text: {
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);

                if (opt) {
                    HbMarqueeItem *marqueeItem = static_cast<HbMarqueeItem*>( item );
                    if (marqueeItem) {
                        marqueeItem->setText(opt->caption);
                    }
                }
                break;
            }

            case P_TitlePane_icon: {
                const HbStyleOptionTitlePane *opt = qstyleoption_cast<const HbStyleOptionTitlePane *>(option);
                if (opt) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIconName("qtg_mono_options_menu");
                }
                break;
            }
            case P_GroupBoxMarquee_text: {
                if (const HbStyleOptionGroupBox *opt = qstyleoption_cast<const HbStyleOptionGroupBox *>(option)) {
                    HbMarqueeItem *marqueeItem = static_cast<HbMarqueeItem*>( item );
                    if(marqueeItem){
                        marqueeItem->setText(opt->heading);
                        if(opt->marqueeEnabled){
                            if(!marqueeItem->isAnimating()){
                                marqueeItem->setLoopCount(-1);
                                marqueeItem->startAnimation();
                            }
                        }else
                        {
                            marqueeItem->stopAnimation();
                        }
                    }
                }
                break;
            }
            case P_GroupBoxHeading_text: {
                if (const HbStyleOptionGroupBox *opt = qstyleoption_cast<const HbStyleOptionGroupBox *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>( item );
                    if(textItem){
                        textItem->setText(opt->heading);
                    }
                }
                break;
            }
            case P_PushButton_focus: {
                if (const HbStyleOptionPushButton *opt =
                        qstyleoption_cast<const HbStyleOptionPushButton *>(option)) {
                     HbFrameItem *iconItem  = static_cast<HbFrameItem*>(item);
                     iconItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_btn_highlight"));
                     int margin = 4;
                     QRectF focusRect  = opt->rect.adjusted(-margin , -margin , margin , margin);
                     iconItem->setGeometry(focusRect);
                     iconItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                 }
                break;
            }
            case P_ToolButton_frame:{
                const HbStyleOptionToolButton *opt = qstyleoption_cast<const HbStyleOptionToolButton *>(option);
                HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                frameItem->frameDrawer().setFillWholeRect(true);
                if (opt->orientation == Qt::Vertical) {
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                } else {
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                }

                QStringList list;
                QString frameGraphicsFooter;
                QIcon::Mode mode = d->iconMode(opt->state);
                QIcon::State state = d->iconState(opt->state);

                if (!opt->customBackground.isNull()) {
                    QString customName = opt->customBackground.iconName(mode, state);
                    frameItem->frameDrawer().setFrameGraphicsName(customName);
                    break;
                }

                QString frameGraphicsName;
                if(opt->isToolBarExtension) {
                   if (mode == QIcon::Normal && state == QIcon::On) {
                        if(!opt->isCheckable){
                            frameGraphicsName = QLatin1String("qtg_fr_popup_grid_pressed");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_tb_ext");
                        }
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    return;
                }
                if (!opt->toolBarPosition) {
                    if (mode == QIcon::Disabled && state == QIcon::Off) {
                        frameGraphicsName = QLatin1String("qtg_fr_btn_disabled");
                    } else if (mode == QIcon::Normal && state == QIcon::On) {
                        if(!opt->isCheckable){
                            frameGraphicsName = QLatin1String("qtg_fr_btn_pressed");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_btn_latched");
                        }
                    } else if (mode == QIcon::Selected && state == QIcon::Off) {
                        frameGraphicsName = QLatin1String("qtg_fr_btn_highlight");
                    } else {
                        frameGraphicsName = QLatin1String("qtg_fr_btn_normal");
                    }
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    break;
                }
                // For toolbar:

                QString frameGraphicsHeader;
                if (!opt->useSecondaryGraphics){
                    if (opt->useTransparentGraphics) {
                        frameGraphicsHeader = opt->orientation == Qt::Vertical ?
                                              QLatin1String("qtg_fr_tb_trans_h_"):
                                              QLatin1String("qtg_fr_tb_trans_v_");
                    } else {
                        frameGraphicsHeader = opt->orientation == Qt::Vertical ?
                                              QLatin1String("qtg_fr_tb_h_"):
                                              QLatin1String("qtg_fr_tb_v_");
                    }
                }else {
                    frameGraphicsHeader = QLatin1String("qtg_fr_popup_sk_");
                    frameItem->frameDrawer().setMirroringMode(HbIcon::LayoutDirection);
                }

                switch (opt->toolBarPosition) {
                    case HbStyleOptionToolButton::TB_OnlyOne:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_l") << QLatin1String("_c") << QLatin1String("_r");
                        } else {
                            list << QLatin1String("_t") << QLatin1String("_c") << QLatin1String("_b");
                        }
                        break;
                    case HbStyleOptionToolButton::TB_Beginning:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_l") << QLatin1String("_c") << QLatin1String("_cr");
                        } else {
                            list << QLatin1String("_t") << QLatin1String("_c") << QLatin1String("_cb");
                        }
                        break;
                    case HbStyleOptionToolButton::TB_Middle:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_cr");
                        } else {
                            list << QLatin1String("_ct") << QLatin1String("_c") << QLatin1String("_cb");
                        }
                        break;
                    case HbStyleOptionToolButton::TB_End:
                        if (opt->orientation == Qt::Vertical) {
                            list << QLatin1String("_cl") << QLatin1String("_c") << QLatin1String("_r");
                        } else {
                            list << QLatin1String("_ct") << QLatin1String("_c") << QLatin1String("_b");
                        }
                        break;
                    default:

                        break;
                }
                frameItem->frameDrawer().setFileNameSuffixList(list);
                if (mode == QIcon::Disabled && state == QIcon::Off) {
                    frameGraphicsFooter = QLatin1String("disabled");
                } else if (mode == QIcon::Normal && state == QIcon::On) {
                    if(!opt->isCheckable) {
                        frameGraphicsFooter = QLatin1String("pressed");
                    } else {
                        frameGraphicsFooter = QLatin1String("latched");
                    }
                } else if (mode == QIcon::Selected && state == QIcon::Off) {
                    frameGraphicsFooter = QLatin1String("highlight");
                } else {
                    frameGraphicsFooter = QLatin1String("normal");
                }

                frameItem->frameDrawer().setFrameGraphicsName(QString ("%0%1").arg(frameGraphicsHeader).arg(frameGraphicsFooter));
                frameItem->frameDrawer().setMirroringMode(HbIcon::LayoutDirection);

                break;
            }



            case P_SliderTickMark_majoricon: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                        iconItem->setMode(d->iconMode(opt->state));
                        if ( opt->orientation ==Qt::Horizontal ){
                            iconItem->setIconName( "qtg_graf_slider_h_tick_major" );
                        } else {
                            iconItem->setIconName ( "qtg_graf_slider_v_tick_major" );
                        }
                        iconItem->setState(d->iconState(opt->state));
                    }
                 break;

             }

            case P_SliderTickMark_minoricon: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item);
                        iconItem->setMode(d->iconMode(opt->state));
                        if ( opt->orientation ==Qt::Horizontal ){
                            iconItem->setIconName( "qtg_graf_slider_h_tick_minor" );
                        } else {
                            iconItem->setIconName ( "qtg_graf_slider_v_tick_minor" );
                        }
                        iconItem->setState(d->iconState(opt->state));
                    }
                 break;

             }

            case P_SliderTickMark_majorlabel: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbTextItem *textItem = qgraphicsitem_cast<HbTextItem*>(item);

                        textItem->setText( opt->text );

                    }
                 break;

             }

            case P_SliderTickMark_minorlabel: {
                 if (const HbStyleOptionSlider *opt =
                        qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbTextItem *textItem = qgraphicsitem_cast<HbTextItem*>(item);
                         textItem->setText( opt->text );
                    }
                 break;

             }

            case P_Slider_progressgroove: {
                if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option))  {
                    HbSliderTrackItem* frameItem = static_cast<HbSliderTrackItem*>(item);
                    if(!frameItem->isVisible()) {
                        break;
                    }
                    if(opt->orientation == Qt::Horizontal){
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_slider_h_filled"));
                        frameItem->setInverted(opt->upsideDown);
                    }
                    else{
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_slider_v_filled"));
                        frameItem->setInverted(!opt->upsideDown);
                        frameItem->frameDrawer().setMirroringMode(HbIcon::Prevented);
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->sliderValue);
                    frameItem->setOrientation(opt->orientation);
                    frameItem->setSpan( opt->span );
                    frameItem->update();
                }
                break;
            }
            case P_Slider_groove:{

                if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    HbFrameDrawer::FrameType type(HbFrameDrawer::Undefined);
                    QString name;

                    if (opt->orientation == Qt::Horizontal) {
                        type = HbFrameDrawer::ThreePiecesHorizontal;
                        if (opt->state & QStyle::State_Enabled) {
                            if(!opt->groovePressed){
                                name = QLatin1String("qtg_fr_slider_h_frame_normal");
                            }
                            else {
                                name = QLatin1String("qtg_fr_slider_h_frame_pressed");
                            }
                        } else {
                                name = QLatin1String("qtg_fr_slider_h_frame_disabled");
                        }
                    } else if (opt->orientation == Qt::Vertical) {
                        if (opt->state & QStyle::State_Enabled) {
                            if(!opt->groovePressed){
                                name = QLatin1String("qtg_fr_slider_v_frame_normal");
                            }
                            else {
                                name = QLatin1String("qtg_fr_slider_v_frame_pressed");
                            }
                        } else {
                                name = QLatin1String("qtg_fr_slider_v_frame_disabled");
                        
                        }
                        type = HbFrameDrawer::ThreePiecesVertical;
                    }

                    frameItem->frameDrawer().setFrameGraphicsName(name);
                    frameItem->frameDrawer().setFrameType(type);
                    frameItem->frameDrawer().setFillWholeRect(true);
                    break;
                }
            }

            case P_Slider_thumb: {
                 if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    // Set pixmap name and bounding rect
                    QString thumbPath;
                    if(opt->thumbPath.isNull())
                    {
                        thumbPath=d->logicalName(HbStyle::P_Slider_thumb, option);
                    }
                    else
                    {
                        thumbPath=opt->thumbPath;
                        iconItem->setAlignment(Qt::AlignCenter);
                    }
                    iconItem->setIconName(thumbPath);
                    iconItem->setAspectRatioMode(Qt::KeepAspectRatio);
                 }
                break;
            }
            case HbStyle::P_SliderElement_increase:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                        if ( !opt->sliderElementIcon.isNull() ){
                            iconItem->setIcon(opt->sliderElementIcon);
                        } else if ( opt->sliderType == HbStyleOptionSlider::VolumeType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_vol_up"));
                        } else if (opt->sliderType == HbStyleOptionSlider::ZoomType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_zoom_in"));
                        }
                    }
                break;
                }
            case HbStyle::P_SliderElement_decrease:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                        if ( !opt->sliderElementIcon.isNull() ){
                            iconItem->setIcon(opt->sliderElementIcon);
                        } else if ( opt->sliderType == HbStyleOptionSlider::VolumeType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_vol_down"));
                        } else if (opt->sliderType == HbStyleOptionSlider::ZoomType) {
                            iconItem->setIconName(QLatin1String("qtg_mono_zoom_out"));
                        }
                    }
                break;
                }
            case HbStyle::P_SliderElement_icon:
                {
                    if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                        iconItem->setZValue(1.0);
                        iconItem->setMode(d->iconMode(opt->state));
                        iconItem->setState(d->iconState(opt->state));
                        if ( !opt->sliderElementIcon.isNull() ){
                            iconItem->setIcon(opt->sliderElementIcon);
                        } else if ( opt->sliderType == HbStyleOptionSlider::VolumeType) {
                            if( opt->state & QStyle::State_Sunken) {
                                iconItem->setIconName(QLatin1String("qtg_mono_speaker_off"));
                            } else {
                                iconItem->setIconName(QLatin1String("qtg_mono_speaker"));
                            }
                        }
                    }
                break;
                }
            case P_SliderElement_text:
                {
                   if (const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        textItem->setZValue(1.0);
                        textItem->setText(opt->text);
                    }
                break;
                }


            case P_CheckBox_text:
                if (const HbStyleOptionCheckBox *opt = qstyleoption_cast<const HbStyleOptionCheckBox*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setTextWrapping(Hb::TextWordWrap);
                    textItem->setText(opt->text);
                }
                break;

            case P_CheckBox_icon: {
                if (const HbStyleOptionCheckBox *opt = qstyleoption_cast<const HbStyleOptionCheckBox *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem *>(item);
                    if (opt->state.testFlag(QStyle::State_Selected) ) {
                        if (opt->state.testFlag(QStyle::State_On)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_highlight"));
                        } else if (opt->state.testFlag(QStyle::State_Off)) {
                            iconItem->setState(QIcon::Off);
                            iconItem->setIconName(QLatin1String("qtg_small_unselected_highlight"));
                        } else if (opt->state.testFlag(QStyle::State_NoChange)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_partial"));
                        }
                    } else {
                        if (opt->state.testFlag(QStyle::State_On)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected"));
                        } else if (opt->state.testFlag(QStyle::State_Off)) {
                            iconItem->setState(QIcon::Off);
                            iconItem->setIconName(QLatin1String("qtg_small_unselected"));
                        } else if (opt->state.testFlag(QStyle::State_NoChange)) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_partial"));
                        }
                    }
                }
                break;
            }
            case P_ItemViewItem_radiobutton:
            case P_ItemViewItem_checkbox:
            case P_ItemViewItem_selection:{
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)){
                    HbIconItem *iconItem = static_cast<HbIconItem *>(item);
                    iconItem->setAlignment(Qt::AlignCenter);

                    if (opt->viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                        if (opt->checkState == Qt::Checked) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_radio_selected"));
                        } else {
                            iconItem->setState(QIcon::Off);
                            iconItem->setIconName(QLatin1String("qtg_small_radio_unselected"));
                        }
                    } else {
                        if (opt->checkState == Qt::Checked) {
                            iconItem->setState(QIcon::On);
                            if (opt->singleSelectionMode) {
                                iconItem->setIconName(QLatin1String("qtg_small_tick"));
                            }
                            else {
                                iconItem->setIconName(QLatin1String("qtg_small_selected"));
                            }
                        } else if (opt->checkState == Qt::PartiallyChecked) {
                            iconItem->setState(QIcon::On);
                            iconItem->setIconName(QLatin1String("qtg_small_selected_partial"));
                        } else {
                            iconItem->setState(QIcon::Off);
                            if (opt->singleSelectionMode) {
                                iconItem->setIconName(QLatin1String(""));
                            }
                            else {
                                iconItem->setIconName(QLatin1String("qtg_small_unselected"));
                            }
                        }
                    }
                }
                break;
            }

            case P_LineEdit_frame_normal:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_normal"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_LineEdit_frame_highlight:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_lineedit_highlight"));
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setGeometry(opt->boundingRect);
                    }
                }
            break;
            }

            case P_TextEdit_frame_normal:{
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_normal"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                }
            break;
            }

            case P_TextEdit_frame_highlight:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_textedit_highlight"));
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setGeometry(opt->boundingRect);
                    }
                }
            break;
            }

            case P_ComboBox_background:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>( item );
                    QString frameGraphicsName;
                    if (opt->state & QStyle::State_Enabled) {
                        if (opt->state & QStyle::State_Sunken) {
                            frameGraphicsName = QLatin1String("qtg_fr_combobox_pressed");
                        } else {
                            frameGraphicsName = QLatin1String("qtg_fr_combobox_normal");
                        }
                    } else {
                        frameGraphicsName = QLatin1String("qtg_fr_combobox_disabled");
                    }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->frameDrawer().setFrameGraphicsName(frameGraphicsName);
                    frameItem->frameDrawer().setMirroringMode ( HbIcon::LayoutDirection );

                }
            break;
            }
            case P_ComboBox_button:{
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    QString iconGraphicsName;
                    if (opt->state & QStyle::State_Enabled) {
                        if (opt->state & QStyle::State_Sunken) {
                            iconGraphicsName = QLatin1String("qtg_graf_combobox_button_pressed");
                        } else {
                            iconGraphicsName = QLatin1String("qtg_graf_combobox_button_normal");
                        }
                    } else {
                        iconGraphicsName = QLatin1String("qtg_graf_combobox_button_disabled");
                    }
                    iconItem->setIconName(iconGraphicsName);
                    iconItem->setMirroringMode ( HbIcon::LayoutDirection );
                    iconItem->setAspectRatioMode( Qt::IgnoreAspectRatio);
                }
            break;
            }

            case P_Edit_text:{
                break;
            }

            case P_MenuItem_frame:{
                        HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_list_normal"));
                    break;
                }
            case P_MenuItem_focus:{
                        HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_list_pressed"));
                    break;
                }

            case P_Label_text:
                 if (const HbStyleOptionLabel *opt = qstyleoption_cast<const HbStyleOptionLabel*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->alignment);
                    textItem->setElideMode(opt->elideMode);
                    textItem->setTextWrapping(opt->textWrapMode);
                    if(!opt->fontSpec.isNull()) {
                        textItem->setFontSpec(opt->fontSpec);
                    }
                    if (opt->color.isValid()) {
                        textItem->setTextColor(opt->color);
                    }
                    textItem->setText(opt->text);
                 }
                 break;
            case P_Label_richtext:
                if (const HbStyleOptionLabel *opt = qstyleoption_cast<const HbStyleOptionLabel*>(option)) {
                    HbRichTextItem *textItem = static_cast<HbRichTextItem*>(item);
                    textItem->setAlignment(opt->alignment);
                    textItem->setTextWrapping(opt->textWrapMode);
                    if(!opt->fontSpec.isNull()) {
                        textItem->setFontSpec(opt->fontSpec);
                    }
                    if (opt->color.isValid()) {
                        textItem->setTextDefaultColor(opt->color);
                    }
                    textItem->setText(opt->text);
                }
                break;

            case P_MenuItem_submenuindicator:
                if (const HbStyleOptionMenuItem *opt = qstyleoption_cast<const HbStyleOptionMenuItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    Q_UNUSED(opt)
                    iconItem->setIconName(QLatin1String("qtg_mono_options_menu"));
                }
                break;

            case P_MenuItem_checkindicator:
                if (const HbStyleOptionMenuItem *opt = qstyleoption_cast<const HbStyleOptionMenuItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if (opt->checked) {
                        iconItem->setIconName(QLatin1String("qtg_small_tick"));
                    } else {
                        iconItem->setIcon(HbIcon());
                    }
                }
                break;

            case P_MenuItem_separator:
                if (const HbStyleOptionMenuItem *opt = qstyleoption_cast<const HbStyleOptionMenuItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    Q_UNUSED(opt)
                    iconItem->setAspectRatioMode(Qt::IgnoreAspectRatio);
                    iconItem->setAlignment(Qt::AlignCenter);
                    iconItem->setIconName(QLatin1String("qtg_graf_popup_separator"));
                }
                break;

            case P_ScrollBar_groove:
                if (const HbStyleOptionScrollBar *opt = qstyleoption_cast<const HbStyleOptionScrollBar *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    if (opt->interactive) {
                        if (opt->groovePressed) {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_frame_pressed"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_frame_pressed"));
                            }
                        } else {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_frame_normal"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_frame_normal"));
                            }
                        }
                    } else {
                        if (opt->orientation == Qt::Vertical) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_frame"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_frame"));
                        }
                    }

                    frameItem->frameDrawer().setFillWholeRect(true);
                }
                break;

            case P_ScrollBar_handle:
                if (const HbStyleOptionScrollBar *opt = qstyleoption_cast<const HbStyleOptionScrollBar *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    if (opt->interactive) {
                        if (opt->thumbPressed) {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_handle_pressed"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_handle_pressed"));
                            }
                        } else {
                            if (opt->orientation == Qt::Vertical) {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_active_handle_normal"));
                            } else {
                                frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_active_handle_normal"));
                            }
                        }
                    } else {
                        if (opt->orientation == Qt::Vertical) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_v_handle"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_scroll_h_handle"));
                        }
                    }
                    frameItem->frameDrawer().setFillWholeRect(true);
                }
                break;

            case P_Popup_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;
            case P_Popup_background_weak:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_secondary"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_Popup_heading_frame:
                if (const HbStyleOptionPopup *opt = qstyleoption_cast<const HbStyleOptionPopup *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                    frameItem->setGeometry(opt->boundingRect);
                    if (opt->headingMode == QIcon::Active) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_heading_pressed"));
                    } else if (opt->headingMode == QIcon::Selected) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_heading_highlight"));
                    } else {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_heading_normal"));
                    }
                }
                break;

            case P_ComboBoxPopup_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                        if(!frameItem){
                           return;
                        }
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_secondary"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_ToolTip_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_preview"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_NotificationDialog_frame:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_preview"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_Fade_background:
               if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>( item );
                    iconItem->setIconName(QLatin1String("qtg_graf_dimming_image"));
                    iconItem->setAspectRatioMode (Qt::IgnoreAspectRatio);
                    iconItem->setGeometry(opt->boundingRect);
                }
                break;

            case P_NotificationDialog_text:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->textAlignment);
                    textItem->setFontSpec(HbFontSpec(HbFontSpec::Secondary));
                    textItem->setTextWrapping(opt->textTextWrapping);
                    textItem->setText(opt->text);
                }
                break;
            case P_NotificationDialog_title:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->titleAlignment);
                    textItem->setFontSpec(HbFontSpec(HbFontSpec::Primary));
                    textItem->setTextWrapping(opt->titleTextWrapping);
                    if (opt->titleTextWrapping == Hb::TextNoWrap) {
                        textItem->setElideMode(Qt::ElideNone);
                    } else {
                        textItem->setElideMode(Qt::ElideRight);
                    }
                    textItem->setText(opt->title);
                }
                break;
            case P_NotificationDialog_icon:
                if (const HbStyleOptionNotificationDialog *opt =
                        qstyleoption_cast<const HbStyleOptionNotificationDialog *>(option)) {
                    HbIconItem *icon = static_cast<HbIconItem*>(item);

                    icon->setIcon(opt->icon);
                    icon->setAlignment(opt->iconAlignment);
                }
                break;

            case P_ProgressDialog_icon:
                if (const HbStyleOptionProgressDialog *opt = static_cast<const HbStyleOptionProgressDialog *>(option)) {
                    HbIconItem *icon = static_cast<HbIconItem*>(item);
                    icon->setIcon(opt->icon);
                }
                break;

            case P_MessageBox_text:    //deprecated
                if (const HbStyleOptionMessageBox *opt = static_cast<const HbStyleOptionMessageBox *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(opt->textAlignment);

                    if (opt->textWrapping) {
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    } else {
                        textItem->setTextWrapping(Hb::TextNoWrap);
                    }
                    textItem->setText(opt->text);
                }
                break;
            case P_MessageBox_icon:
                if (const HbStyleOptionMessageBox *opt = static_cast<const HbStyleOptionMessageBox *>(option)) {
                    HbIconItem *icon = static_cast<HbIconItem*>(item);
                    HbIcon noteIcon = opt->icon;
                    if(noteIcon.isNull()) {
                        switch (opt->messageBoxType) {
                        case HbStyleOptionMessageBox::MessageTypeInformation:
                            icon->setIcon(HbIcon(QLatin1String("qtg_large_info")));
                            break;
                        case HbStyleOptionMessageBox::MessageTypeQuestion:
                            icon->setIcon(HbIcon(QLatin1String("qtg_large_help")));
                            break;
                        case HbStyleOptionMessageBox::MessageTypeWarning:
                            icon->setIcon(HbIcon(QLatin1String("qtg_large_warning")));
                            break;
                        default:
                            break;
                        }
                    } else {
                        icon->setIcon(opt->icon);
                    }
                }
                break;
            case P_ListViewItem_text:
                if (const HbStyleOptionListViewItem *opt = qstyleoption_cast<const HbStyleOptionListViewItem *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);

                    if (opt->index == 1) {
                        if (opt->multilineSecondaryTextSupported) {
                            if (opt->minimumLines != -1) {
                                // min & max secondary text row counts set by app
                                Hb::TextWrapping wrapping = Hb::TextNoWrap;
                                if (    opt->minimumLines != 1
                                    ||  opt->maximumLines != 1) {
                                    wrapping = Hb::TextWordWrap;
                                }
                                textItem->setTextWrapping(wrapping);
                                textItem->setMinimumLines(opt->minimumLines);
                                textItem->setMaximumLines(opt->maximumLines);
                            } else {
                                // min & max secondary text row counts not set by app. Allow setting those from .css
                                // Needed when multilineSecondaryTextSupported changed from FALSE to TRUE and
                                // min & max secondary text row counts has not bee set by app
                                HbWidgetBasePrivate *widgetBasePrivate = d->widgetBasePrivate(textItem);
                                widgetBasePrivate->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
                                widgetBasePrivate->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
                                widgetBasePrivate->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
                            }
                        } else {
                            // min & max secondary text row counts must always be 1. They cannot be overridden by .css
                            textItem->setTextWrapping(Hb::TextNoWrap);
                            textItem->setMinimumLines(1);
                            textItem->setMaximumLines(1);
                        }
                    } // else - default values from .css still in use
                    setItemName(textItem, QLatin1String("text-") + QString::number(opt->index + 1));
                    textItem->setText(opt->content.value<QString>());
                }
                break;
            case P_ListViewItem_richtext:
                if (const HbStyleOptionListViewItem *opt = qstyleoption_cast<const HbStyleOptionListViewItem *>(option)) {
                    HbRichTextItem *label = static_cast<HbRichTextItem*>(item);
                    label->setText(opt->content.value<QString>());

                    setItemName(label, QLatin1String("text-") + QString::number(opt->index + 1));
                }
                break;
            case P_ListViewItem_icon:
                if (const HbStyleOptionListViewItem *opt = qstyleoption_cast<const HbStyleOptionListViewItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);

                    setItemName(iconItem, QLatin1String("icon-") + QString::number(opt->index + 1));
                    if (iconItem->zValue() != opt->index + 1) {
                        iconItem->setZValue(opt->index + 1);
                    }

                    if (opt->content.canConvert<HbIcon>()){
                        iconItem->setIcon(opt->content.value<HbIcon>());
                    } if (opt->content.canConvert<QIcon>()){
                        iconItem->setIcon(opt->content.value<QIcon>());
                    }
                }
                break;
            case P_GridViewItem_icon:
                if (const  HbStyleOptionGridViewItem*opt = qstyleoption_cast<const HbStyleOptionGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setIcon(opt->icon);
                    iconItem->setAlignment(Qt::AlignCenter);
                    iconItem->setAspectRatioMode(Qt::KeepAspectRatio);
                }
                break;

            case P_ColorGridViewItem_colorIcon:
                if (const  HbStyleOptionColorGridViewItem* opt = qstyleoption_cast<const HbStyleOptionColorGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if(!iconItem)
                        return;
                    iconItem->setColor(opt->color);
                    iconItem->setZValue(iconItem->parentItem()->zValue()+2);
                }
                break;
            case P_ColorGridViewItem_borderIcon:
                if (const  HbStyleOptionColorGridViewItem* opt = qstyleoption_cast<const HbStyleOptionColorGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if(!iconItem)
                        return;
                    iconItem->setIconName(opt->borderIcon);
                    iconItem->setColor(opt->borderColor);
                    iconItem->setZValue(iconItem->parentItem()->zValue()+1);
                }
                break;
            case P_ColorGridViewItem_checkIcon:
                if (const  HbStyleOptionColorGridViewItem* opt = qstyleoption_cast<const HbStyleOptionColorGridViewItem*>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if(!iconItem)
                        return;
                    Q_UNUSED(opt);
                    iconItem->setZValue(iconItem->parentItem()->zValue()+3);
                }
                break;


            case P_GridViewItem_text:
                if (const  HbStyleOptionGridViewItem*opt = qstyleoption_cast<const HbStyleOptionGridViewItem*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setAlignment(Qt::AlignCenter);
                    textItem->setText(opt->text);
                }
                break;

            case P_ItemHighlight_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->setZValue(-2.0);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_highlight"));
                    frameItem->setGeometry(opt->boundingRect);
                }
                break;
            case P_ToolBarExtension_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    Q_UNUSED(opt);
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_trans"));
                }
                break;
            case P_SliderPopup_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption *>(option)) {
                    Q_UNUSED(opt)
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_popup_trans"));
                }
                break;
            case P_ProgressBar_frame:
                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        if(opt->orientation == Qt::Horizontal){
                       
                                frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_h_frame"));
                         }
                      
                        else{
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_v_frame"));
                        }
                  //  frameItem->frameDrawer().setFillWholeRect(true);
                }
                break;
            case P_ProgressBar_track:

                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                    HbProgressTrackItem* frameItem = static_cast<HbProgressTrackItem*>(item);
                    if(!frameItem->isVisible()) {
                        break;
                    }
                    
                    if(opt->orientation == Qt::Horizontal){
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_h_filled"));
                     }
                     else{
                               frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesVertical);
                               frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progbar_v_filled"));
                     }
                    
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->progressValue);
                    frameItem->setInverted(opt->inverted);
                    //frameItem->setGeometry(opt->rect);
                    frameItem->setOrientation(opt->orientation);
                    frameItem->update();
                }
                break;
            case P_ProgressBar_waittrack:
                if (const HbStyleOptionProgressBar *opt = qstyleoption_cast<const HbStyleOptionProgressBar *>(option)) {
                    HbRepeatIconItem *iconItem = static_cast<HbRepeatIconItem*>(item);
                    iconItem->setOrientation(opt->orientation);
                    if( !iconItem->isVisible() ) {
                        break;
                    }
                    if(opt->orientation == Qt::Horizontal){
                        iconItem->setName(QLatin1String("qtg_graf_progbar_h_wait"));
                    }
                    else{
                        iconItem->setName(QLatin1String("qtg_graf_progbar_v_wait"));
                    }
                    if(opt->stopWaitAnimation){
                        iconItem->stopAnimation();
                    }
                    }
                    break;

            case P_ProgressSlider_frame:
                if (const HbStyleOptionProgressSlider *opt = qstyleoption_cast<const HbStyleOptionProgressSlider *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                   	if (opt->disableState ) {
                        frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_frame_disabled"));
					}
					else {
					     if(opt->pressedState) {
                               frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_frame_pressed"));
                         }
                         else {
                               frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_frame_normal"));
                         }
					}
                    frameItem->frameDrawer().setFillWholeRect(true);
                    //frameItem->update();
                    }
                    break;
            case P_ProgressSlider_track: // The ProgressValue Mask
                if (const HbStyleOptionProgressSlider *opt = qstyleoption_cast<const HbStyleOptionProgressSlider *>(option)) {
                    HbProgressTrackItem* frameItem = static_cast<HbProgressTrackItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);

					if (opt->disableState ) {
                         frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_loaded_disabled"));
					}
					else {
                         frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_loaded"));
					}
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->progressValue);
                    frameItem->setInverted(opt->inverted);
                    frameItem->setOrientation(opt->orientation);
                    frameItem->update();
                    }
                    break;

            case P_ProgressSlider_slidertrack: // The Slider Position Mask
                if (const HbStyleOptionProgressSlider *opt = qstyleoption_cast<const HbStyleOptionProgressSlider *>(option)) {
                    HbProgressTrackItem* frameItem = static_cast<HbProgressTrackItem*>(item);
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
					if (opt->disableState ) {
                         frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_played_disabled"));
					}
					else {
                         frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_progslider_played"));
					}
                    frameItem->frameDrawer().setFillWholeRect(true);
                    frameItem->setMaximum(opt->maximum);
                    frameItem->setMinimum(opt->minimum);
                    frameItem->setValue(opt->progressSliderValue);
                    frameItem->setInverted(opt->inverted);
                    frameItem->setOrientation(opt->orientation);
                    frameItem->update();
                    }
                    break;
            case P_ProgressBar_mintext: {
                 if (const HbStyleOptionProgressBar *opt =
                        qstyleoption_cast<const HbStyleOptionProgressBar*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        if(!item) {
                            return;
                        }
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                        textItem->setText(opt->minText);
                    }
                 break;
            }

            case P_ProgressBar_maxtext: {
                if (const HbStyleOptionProgressBar *opt =
                        qstyleoption_cast<const HbStyleOptionProgressBar*>(option)) {
                        HbTextItem *textItem = static_cast<HbTextItem*>(item);
                        if(!item) {
                            return;
                        }
                        textItem->setTextWrapping(Hb::TextWrapAnywhere);
                        textItem->setText(opt->maxText);
                    }
                break;
            }

            case P_RatingSlider_frame:{
                if (const HbStyleOptionRatingSlider *opt = qstyleoption_cast<const HbStyleOptionRatingSlider *>(option)) {
                    HbRepeatItem *repeatItem = static_cast<HbRepeatItem*>(item);
                    repeatItem->setRepeatingNumber(opt->noOfStars);
                    if (!opt->unRatedGraphicsName.isEmpty()) {
                        repeatItem->setName(opt->unRatedGraphicsName);
                    }
                    else {
                        if(opt->disableState) {
                             repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated_disabled"));

                        }
                        else {
							if(opt->pressedState) {

								repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated_pressed"));
							}
							else {
                            
								repeatItem->setName(QLatin1String("qtg_graf_ratingslider_unrated"));
							}
                        }
                    }
                    repeatItem->setGeometry(opt->boundingRect);
                    repeatItem->update();
                }
                break;
            }

            case P_RatingSlider_track:{
                if (const HbStyleOptionRatingSlider *opt = qstyleoption_cast<const HbStyleOptionRatingSlider *>(option)) {
                    HbRepeatMaskItem *repeatItem = static_cast<HbRepeatMaskItem*>(item);
                    repeatItem->setMaskValue(opt->progressValue);
                    repeatItem->setMaximum(opt->noOfIntervals);
                    repeatItem->setInverted(opt->inverted);
                    repeatItem->setRepeatingNumber(opt->noOfStars);
                    if (!opt->ratedGraphicsName.isEmpty()) {
                        repeatItem->setName(opt->ratedGraphicsName);
                    }
                    else {
                        if(opt->disableState) {
                            repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated_disabled"));


                        }
                        else {

							if(opt->pressedState) {

								repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated_pressed"));
							}
							else {
								repeatItem->setName(QLatin1String("qtg_graf_ratingslider_rated"));
							}
                        }
                    }
                    repeatItem->setGeometry(opt->boundingRect);
                    repeatItem->update();
                }
                break;
            }
        case P_ProgressSliderHandle_icon:
                    if (const HbStyleOptionProgressSliderHandle *opt = qstyleoption_cast<const HbStyleOptionProgressSliderHandle *>(option)) {
                        HbIconItem *iconItem = static_cast<HbIconItem*>(item);

                        if(!opt->handleIcon.isNull()) {
                            iconItem->setIconName(opt->handleIcon.iconName());
                        }
                        else {
                            if(opt->pressedState) {
                                iconItem->setIconName(QLatin1String("qtg_graf_progslider_handle_pressed"));
                            }
                            else {
                                iconItem->setIconName(QLatin1String("qtg_graf_progslider_handle_normal"));
                            }
                        }
                    }
                    break;
            case P_QueryInputMode_image:

             /*   if (const HbStyleOptionInputPopup *opt = qstyleoption_cast<const HbStyleOptionInputPopup *>(option)){
                    if (HbIconItem *iconItem = static_cast<HbIconItem*>(item)) {
                       iconItem->setIcon(*opt->modeIcon);
                       iconItem->setPos(opt->editRect.width() - opt->modeIcon->width() - 6,opt->editRect.top()-3);
                    }
                }*/
                break;
            case P_ItemViewItem_background:
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    iconItem->setGeometry(opt->boundingRect);
                    if (opt->background.canConvert<HbIcon>()){
                        iconItem->setIcon(opt->background.value<HbIcon>());
                    } else if (opt->background.canConvert<QBrush>()){
                        iconItem->setBrush(opt->background.value<QBrush>());
                    }
                }

                break;
            case P_ItemViewItem_frame:
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->setGeometry(opt->boundingRect);

                    if (opt->background.canConvert<HbFrameBackground>()) {
                        HbFrameBackground frame = opt->background.value<HbFrameBackground>();
                        frameItem->frameDrawer().setFrameType(frame.frameType());
                        frameItem->frameDrawer().setFrameGraphicsName(frame.frameGraphicsName());
                    } else if (opt->viewItemType == Hb::ItemType_TreeViewItem) {
                        if (opt->modelItemType == Hb::ParentItem) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ?
                                QLatin1String("qtg_fr_popup_list_parent_normal") : QLatin1String("qtg_fr_list_parent_normal"));
                        } else if (opt->modelItemType == Hb::SeparatorItem) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_separator"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ?
                                QLatin1String("qtg_fr_popup_list_normal") : QLatin1String("qtg_fr_list_normal"));
                        }
                    } else if ( opt->viewItemType == Hb::ItemType_ListViewItem
                                || opt->viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                        if (opt->modelItemType == Hb::SeparatorItem) {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_fr_list_separator"));
                        } else {
                            frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                            frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ?
                                QLatin1String("qtg_fr_popup_list_normal") : QLatin1String("qtg_fr_list_normal"));
                        }
                    } else if (opt->viewItemType == Hb::ItemType_GridViewItem
                                || opt->viewItemType == HbPrivate::ItemType_ColorGridViewItem) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ?
                            QLatin1String("qtg_fr_popup_grid_normal") : QLatin1String("qtg_fr_grid_normal"));
                    }
                    else{
                        frameItem->frameDrawer().setFrameGraphicsName(QString());
                    }
                }
                break;
            case P_ItemViewItem_focus:
                if (const HbStyleOptionAbstractViewItem *opt = qstyleoption_cast<const HbStyleOptionAbstractViewItem *>(option)) {
                    HbFrameItem *frameItem = static_cast<HbFrameItem*>(item);
                    frameItem->setGeometry(opt->boundingRect);

                    if (opt->viewItemType == Hb::ItemType_TreeViewItem
                        || opt->viewItemType == Hb::ItemType_ListViewItem
                        || opt->viewItemType == Hb::ItemType_RadioButtonListViewItem) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName( opt->insidePopup ?
                            QLatin1String("qtg_fr_popup_list_pressed") : QLatin1String("qtg_fr_list_pressed"));
                    } else if (     opt->viewItemType == Hb::ItemType_GridViewItem
                                ||  opt->viewItemType == HbPrivate::ItemType_ColorGridViewItem) {
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->frameDrawer().setFrameGraphicsName(opt->insidePopup ?
                            QLatin1String("qtg_fr_popup_grid_pressed") : QLatin1String("qtg_fr_grid_pressed"));
                    } else {
                        setItemName(frameItem,QString());
                        frameItem->frameDrawer().setFrameGraphicsName(QString());
                    }
                }
                break;
            case P_SelectionControl_selectionstart://fallthrough
            case P_SelectionControl_selectionend://fallthrough
            {
                if (HbIconItem *iconItem = qgraphicsitem_cast<HbIconItem*>(item)) {
                    iconItem->setSize(iconItem->preferredSize());
                }
                break;
            }
            case P_TreeViewItem_expandicon:
                if (const HbStyleOptionTreeViewItem *opt = qstyleoption_cast<const HbStyleOptionTreeViewItem *>(option)) {
                    HbIconItem *iconItem = static_cast<HbIconItem*>(item);
                    if (opt->expanded) {
                        iconItem->setIconName(opt->insidePopup ? QLatin1String("qtg_mono_collapse") : QLatin1String("qtg_small_collapse"));
                    } else {
                        iconItem->setIconName(opt->insidePopup ? QLatin1String("qtg_mono_expand") : QLatin1String("qtg_small_expand"));
                    }
                }
                break;
            case P_ScrollArea_continuationbottom:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_b"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;
            case P_ScrollArea_continuationtop:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_t"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;
            case P_ScrollArea_continuationright:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_r"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;
            case P_ScrollArea_continuationleft:
                if (HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>( item ) ) {
                    frameItem->frameDrawer().setFrameGraphicsName(QLatin1String("qtg_graf_list_mask_l"));
                    frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                    frameItem->setZValue(1);
                }
                break;

            case P_TumbleView_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {

                        //Temporary source to avoid introducing new style primitive for selection dialog mark widget background item.
                        QGraphicsItem *parent = frameItem->parentItem();
                        const QMetaObject *obj = parent->toGraphicsObject()->metaObject();

                        QString className;
                        if(obj){
                            className = obj->className();
                        }
                        ///////////////////////////////////////////////////////////
                        
                        if( !className.compare("HbTumbleView") ){
                            frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_bg");
                        }
                        else if( !className.compare("HbSelectionDialogMarkWidget") )
                        {
                            frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_groupbox");
                        }
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-5);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;
            case P_TumbleView_frame:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_overlay");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-4);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;

            case P_TumbleView_highlight:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_highlight_pri");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
                        frameItem->setZValue(3);
                        //TODO:temp fix, issue with css rule picking in derived class

                        //frameItem->setGeometry(0,(opt->boundingRect.height()-frameItem->boundingRect().height())/2,opt->boundingRect.width(),opt->boundingRect.height());
                        Q_UNUSED(opt);
                    }

                }
                break;

            case P_IndexFeedback_popup_text:
                if (const HbStyleOptionIndexFeedback *opt = qstyleoption_cast<const HbStyleOptionIndexFeedback *>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    textItem->setFontSpec(opt->fontSpec);
                    textItem->setGeometry(opt->textRect);
                    textItem->setText(opt->text);
                }
                break;
            case P_IndexFeedback_popup_background:
                if (const HbStyleOptionIndexFeedback *opt = qstyleoption_cast<const HbStyleOptionIndexFeedback *>(option)) {
                    if (HbFrameItem* frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->setGeometry(opt->popupRect);
                    }
                }
                break;
            case P_DateTimePicker_background:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_bg");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(-5);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;
            case P_DateTimePicker_frame:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_fr_tumbler_overlay");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::NinePieces);
                        frameItem->setZValue(1);
                        //TODO:temp fix, issue with css rule picking in derived class
                        //frameItem->setGeometry(opt->boundingRect);
                        Q_UNUSED(opt);
                    }
                }
                break;
            case P_DateTimePicker_separator:
                if (const HbStyleOption *opt = qstyleoption_cast<const HbStyleOption*>(option)) {
                    if(HbFrameItem *frameItem = qgraphicsitem_cast<HbFrameItem*>(item)) {
                        frameItem->frameDrawer().setFrameGraphicsName("qtg_graf_tumbler_divider");
                        frameItem->frameDrawer().setFrameType(HbFrameDrawer::OnePiece);
                        frameItem->setZValue(2);
                        //TODO:temp fix, issue with css rule picking in derived class

                        //frameItem->setGeometry(0,(opt->boundingRect.height()-frameItem->boundingRect().height())/2,opt->boundingRect.width(),opt->boundingRect.height());
                        Q_UNUSED(opt);
                    }

                }
                break;
            case P_InputDialog_text:
                if (const HbStyleOptionInputDialog *opt =
                    qstyleoption_cast<const HbStyleOptionInputDialog*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    if(!item) {
                        return;
                    }
                    textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    textItem->setText(opt->text);
                }
                break;
             case P_InputDialog_additionaltext:
                if (const HbStyleOptionInputDialog *opt =
                    qstyleoption_cast<const HbStyleOptionInputDialog*>(option)) {
                    HbTextItem *textItem = static_cast<HbTextItem*>(item);
                    if(!item) {
                        return;
                    }
                    textItem->setTextWrapping(Hb::TextWrapAnywhere);
                    textItem->setText(opt->additionalText);
                }
                break;

             default:
                return;
    };
}

/*!
  From qstylesheetstyle.cpp (declarations(...))
  \internal
*/
static HbVector<HbCss::Declaration> declarations(
    const HbVector<HbCss::StyleRule> &styleRules,
    const QString &part,
    HbWidget* widget,
    const HbDeviceProfile &profile)
{
    quint64 pseudoClass(0);

    if ( profile.orientation() == Qt::Horizontal ) {
        pseudoClass |= HbCss::PseudoClass_Landscape;
    } else {
        pseudoClass |= HbCss::PseudoClass_Portrait;
    }

    HbMainWindow *mainWindow = 0;
    if (widget) {
        mainWindow = widget->mainWindow();
    } else {
        QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
        if (!mainWindows.isEmpty()) {
            mainWindow = mainWindows.at(0);
        }
    }
    if (mainWindow && mainWindow->isRightToLeft()) {
        pseudoClass |= HbCss::PseudoClass_RightToLeft;
    } else {
        pseudoClass |= HbCss::PseudoClass_LeftToRight;
    }

    HbVector<HbCss::Declaration> decls;
    for (int i = 0; i < styleRules.count(); i++) {
        const HbCss::Selector& selector = styleRules.at(i).selectors.at(0);
        // Rules with pseudo elements don't cascade. This is an intentional
        // diversion for CSS
        if (part.compare(selector.pseudoElement(), Qt::CaseInsensitive) != 0)
            continue;
        quint64 negated = 0;
        quint64 cssClass = selector.pseudoClass(&negated);
        if ((pseudoClass == HbCss::PseudoClass_Any) || (cssClass == HbCss::PseudoClass_Unspecified)
            || ((((cssClass & pseudoClass) == cssClass)) && ((negated & pseudoClass) == 0)))
            decls += styleRules.at(i).declarations;
    }
    return decls;
}

/*!
  used by the polish() method
  \internal
*/
void HbStylePrivate::polishItem(
    const HbVector<HbCss::StyleRule> &styleRules,
    HbWidget *widget,
    QGraphicsItem *item,
    const QString &name,
    HbDeviceProfile &profile,
    bool layoutDefined) const
{
    if (name.isEmpty() && widget != item) {
        return;
    }

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- --";
    if ( widget == item ) {
        qDebug() << "HbStyle::polishItem : -- item name: " << widget->metaObject()->className();
    } else if ( item ) {
        qDebug() << "HbStyle::polishItem : -- item name: " << name << "(existing item)";
    } else {
        qDebug() << "HbStyle::polishItem : -- item name: " << name << "(missing item)";
    }
#endif

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, name, widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polishItem : -- Number of matching CSS declarations: " << decl.count();
#endif
    HbCss::ValueExtractor extractor(decl, layoutParameters, profile);
    HbCss::KnownProperties prop;

    if ( !extractor.extractKnownProperties(prop) ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishItem : -- No polish overrides found";
#endif
        return;
    }

    if ( item ) {
        if (prop.mFlags & HbCss::ExtractedZValue) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting zvalue: " << prop.mZ;
#endif
            item->setZValue(prop.mZ);
        }
    }

    QGraphicsWidget *gWidget = (item && item->isWidget())
        ? static_cast<QGraphicsWidget*>(item)
        : 0;
    if ( gWidget ) {
        if (prop.mFlags & HbCss::ExtractedLayoutDir) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting layout direction: " << prop.mLayoutDir;
#endif
            if (prop.mLayoutDir == HbCss::LayoutDirection_LeftToRight) {
                gWidget->setLayoutDirection(Qt::LeftToRight);
            } else if (prop.mLayoutDir == HbCss::LayoutDirection_RightToLeft) {
                gWidget->setLayoutDirection(Qt::RightToLeft);
            } else {
                gWidget->unsetLayoutDirection();
            }
        }
    }

    HbWidgetBase *hbWidget = qobject_cast<HbWidgetBase*>(gWidget);
    if ( hbWidget ) {
        HbWidgetBasePrivate* hbWidget_p = HbWidgetBasePrivate::d_ptr(hbWidget);
        if ( prop.mFlags & HbCss::ExtractedFontSpec ) {
            if ( !(prop.mFlags & HbCss::ExtractedFont) ) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting fontspec: " << prop.mFontSpec.role() << prop.mFontSpec.textHeight();
#endif
                hbWidget->setFontSpec(prop.mFontSpec);
            } else {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting fontspec with overrides: "
                         << prop.mFontSpec.role() << prop.mFontSpec.textHeight() << prop.mFont;
#endif
                hbWidget->setFont(prop.mFontSpec.font().resolve(prop.mFont));
            }
        } else if ( prop.mFlags & HbCss::ExtractedFont ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting explicit font: " << prop.mFont;
#endif
            hbWidget->setFont(prop.mFont);
        }

        HbIconItem* icon = qobject_cast<HbIconItem*>(hbWidget);
        if (icon) {
            if (prop.mFlags & HbCss::ExtractedAspectRatioMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting aspect ratio mode: " << prop.mAspectRatioMode;
#endif
                icon->setAspectRatioMode(prop.mAspectRatioMode);
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconAspectRatioMode, false);
            }
            if ( prop.mFlags & HbCss::ExtractedAlignment
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_IconAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting icon alignment: " << prop.mAlignment;
#endif
                icon->setAlignment( prop.mAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_IconAlign, false);
            }
        }

        HbTextItem* text = qobject_cast<HbTextItem*>(hbWidget);
        if (text) {
            if ( prop.mFlags & HbCss::ExtractedMinLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text min lines: " << prop.mMinLines;
#endif
                text->setMinimumLines( prop.mMinLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
            }
            if ( prop.mFlags & HbCss::ExtractedMaxLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text max lines: " << prop.mMaxLines;
#endif
                text->setMaximumLines( prop.mMaxLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
            }
            if ( prop.mFlags & HbCss::ExtractedAlignment
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text alignment: " << prop.mAlignment;
#endif
                text->setAlignment( prop.mAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, false);
            }
            if ( prop.mFlags & HbCss::ExtractedWrapMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting wrap mode : " << prop.mTextWrapMode;
#endif
                text->setTextWrapping( prop.mTextWrapMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
            }
        }

        HbRichTextItem* richtext = qobject_cast<HbRichTextItem*>(hbWidget);
        if (richtext) {
            if ( prop.mFlags & HbCss::ExtractedAlignment
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text alignment: " << prop.mAlignment;
#endif
                richtext->setAlignment( prop.mAlignment );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextAlign, false);
            }
            if ( prop.mFlags & HbCss::ExtractedWrapMode
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting wrap mode : " << prop.mTextWrapMode;
#endif
                richtext->setTextWrapping( prop.mTextWrapMode );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextWrapMode, false);
            }
        }

        HbFrameItem *frame = qobject_cast<HbFrameItem*>(hbWidget);
        if (frame && !frame->frameDrawer().d->testBorderApiProtectionFlag()) {
            if (prop.mFlags & HbCss::ExtractedBorderWidths) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting border widths (l,t,r,b):"
                    << prop.mBorderWidths[HbCss::LeftEdge]
                    << prop.mBorderWidths[HbCss::TopEdge]
                    << prop.mBorderWidths[HbCss::RightEdge]
                    << prop.mBorderWidths[HbCss::BottomEdge];
#endif
                frame->frameDrawer().setBorderWidths(
                    prop.mBorderWidths[HbCss::LeftEdge],
                    prop.mBorderWidths[HbCss::TopEdge],
                    prop.mBorderWidths[HbCss::RightEdge],
                    prop.mBorderWidths[HbCss::BottomEdge]);
                frame->frameDrawer().d->setBorderApiProtectionFlag(false);
            }
        }
        if ( hbWidget->inherits( "HbLineEdit" ) ) {
            if ( prop.mFlags & HbCss::ExtractedMinLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text min lines: " << prop.mMinLines;
#endif
                hbWidget->setProperty( "minRows", prop.mMinLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin, false);
            }
            if ( prop.mFlags & HbCss::ExtractedMaxLines
                && !hbWidget_p->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMin)) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting text max lines: " << prop.mMaxLines;
#endif
                hbWidget->setProperty( "maxRows", prop.mMaxLines );
                hbWidget_p->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextLinesMax, false);
            }
        }
    }

    QGraphicsLayoutItem* lItem = (item && item->isWidget())
        ? (QGraphicsLayoutItem*)static_cast<QGraphicsWidget*>(item)
        : 0;
    if ( lItem ) {
        if ( prop.mFlags & HbCss::ExtractedMinW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting minimum width: " << prop.mMinW;
#endif
            lItem->setMinimumWidth( prop.mMinW );
        }
        if ( prop.mFlags & HbCss::ExtractedMinH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting minimum height: " << prop.mMinH;
#endif
            lItem->setMinimumHeight( prop.mMinH );
        }
        if ( prop.mFlags & HbCss::ExtractedPrefW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting preferred width: " << prop.mPrefW;
#endif
            lItem->setPreferredWidth( prop.mPrefW );
        }
        if ( prop.mFlags & HbCss::ExtractedPrefH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting preferred height: " << prop.mPrefH;
#endif
            lItem->setPreferredHeight( prop.mPrefH );
        }
        if ( prop.mFlags & HbCss::ExtractedMaxW ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting maximum width: " << prop.mMaxW;
#endif
            lItem->setMaximumWidth( prop.mMaxW );
        }
        if ( prop.mFlags & HbCss::ExtractedMaxH ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting maximum height: " << prop.mMaxH;
#endif
            lItem->setMaximumHeight( prop.mMaxH );
        }
        QSizePolicy itemPol = lItem->sizePolicy();
        if ( prop.mFlags & HbCss::ExtractedPolHor ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting horizontal size policy: " << prop.mSizePolicy.horizontalPolicy();
#endif
            itemPol.setHorizontalPolicy(prop.mSizePolicy.horizontalPolicy());
        }
        if ( prop.mFlags & HbCss::ExtractedPolVer ) {
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polishItem : -- Setting vertical size policy: " << prop.mSizePolicy.verticalPolicy();
#endif
            itemPol.setVerticalPolicy(prop.mSizePolicy.verticalPolicy());
        }
        lItem->setSizePolicy(itemPol);
    }

    if (layoutDefined) {
        HbAnchorLayout *layout = static_cast<HbAnchorLayout*>(widget->layout());
        if ( layout ) {
            if (prop.mFlags & HbCss::ExtractedLeft) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting left override: " << prop.mLeft;
#endif
                overrideSpacing(layout, name, Hb::LeftEdge, prop.mLeft);
            }
            if (prop.mFlags & HbCss::ExtractedRight) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting right override: " << prop.mRight;
#endif
                overrideSpacing(layout, name, Hb::RightEdge, prop.mRight);
            }
            if (prop.mFlags & HbCss::ExtractedTop) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting top override: " << prop.mTop;
#endif
                overrideSpacing(layout, name, Hb::TopEdge, prop.mTop);
            }
            if (prop.mFlags & HbCss::ExtractedBottom) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting bottom override: " << prop.mBottom;
#endif
                overrideSpacing(layout, name, Hb::BottomEdge, prop.mBottom);
            }
            if (prop.mFlags & HbCss::ExtractedCenterH) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting centerh override: " << prop.mCenterH;
#endif
                overrideSpacing(layout, name, Hb::CenterHEdge, prop.mCenterH);
            }
            if (prop.mFlags & HbCss::ExtractedCenterV) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "HbStyle::polishItem : -- Setting centerv override: " << prop.mCenterV;
#endif
                overrideSpacing(layout, name, Hb::CenterVEdge, prop.mCenterV);
            }
        }
    }
}

void HbStylePrivate::polishAnchor(
    const HbVector<HbCss::StyleRule> &styleRules,
    HbWidget *widget,
    HbAnchor *anchor,
    HbDeviceProfile &profile) const
{
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- --";
    qDebug() << "HbStyle::polishAnchor : -- anchor id: " << anchor->anchorId();
#endif

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, anchor->anchorId(), widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polishAnchor : -- Number of matching CSS declarations: " << decl.count();
#endif
    HbCss::ValueExtractor extractor(decl, layoutParameters, profile);
    HbCss::KnownProperties prop;

    if ( !extractor.extractKnownProperties(prop) ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- No polish overrides found";
#endif
        return;
    }


    if ( prop.mFlags & HbCss::ExtractedMinH || prop.mFlags & HbCss::ExtractedMinW ) {
        qreal minLength = prop.mFlags & HbCss::ExtractedMinH ? prop.mMinH : prop.mMinW;
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting minimum length: " << minLength;
#endif
        anchor->setMinimumLength( minLength );
    }
    if ( prop.mFlags & HbCss::ExtractedPrefH || prop.mFlags & HbCss::ExtractedPrefW ) {
        qreal prefLength = prop.mFlags & HbCss::ExtractedPrefH ? prop.mPrefH : prop.mPrefW;
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting preferred length: " << prefLength;
#endif
        anchor->setPreferredLength( prefLength );
    }
    if ( prop.mFlags & HbCss::ExtractedMaxH || prop.mFlags & HbCss::ExtractedMaxW ) {
        qreal maxLength = prop.mFlags & HbCss::ExtractedMaxH ? prop.mMaxH : prop.mMaxW;
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting maximum length: " << maxLength;
#endif
        anchor->setMaximumLength( maxLength );
    }
    if ( prop.mFlags & HbCss::ExtractedPolVer || prop.mFlags & HbCss::ExtractedPolHor ) {
        QSizePolicy::Policy policy = prop.mFlags & HbCss::ExtractedPolVer
            ? prop.mSizePolicy.verticalPolicy() : prop.mSizePolicy.horizontalPolicy();
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting size policy: " << policy;
#endif
        anchor->setSizePolicy( policy );
    }
    if ( prop.mFlags & HbCss::ExtractedAnchorDir ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polishAnchor : -- Setting anchor direction: " << prop.mAnchorDir;
#endif
        anchor->setDirection( prop.mAnchorDir );
    }
}



#define NODEPTR_N(x) HbCss::StyleSelector::NodePtr n = {n.ptr = (void *)x};
/*!
    Polishes the appearance of the given \a widget.

    This method should be called only by HbWidget (base class). The inherited
    classes should not call this method directly, but use the HbWidget's
    polish loop instead.

    This method reads the CSS and WidgetML definition for a given widget and positions
    the sub-elements inside it. Note you need to set the names for the sub-elements
    with HbStyle::setItemName method before this method is called.

    Note that this method is heavy on the system resources and should be called
    absolutely when necessary by the widget.

    \param widget, HbWidget to be polished
    \param params, style parameters to be returned to the caller
*/
void HbStyle::polish(HbWidget *widget, HbStyleParameters &params)
{
    Q_D( HbStyle );
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : -- -- -- --";
#endif
    if( !widget ) {
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Polishing" << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : HbLayeredStyleLoader returned a null pointer.";
#endif
        return;
    }
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    NODEPTR_N(widget);

    HbVector<HbCss::StyleRule> styleRules;
    QVariant widgetStyleCache = widget->property( HbStyleRulesCacheId::hbStyleRulesForNodeCache );

    if(widgetStyleCache.isValid()) {
        QString widgetStyleCacheId = widgetStyleCache.toString();
        if (d->styleRulesCache.contains(widgetStyleCacheId)) {
            styleRules = d->styleRulesCache.value(widgetStyleCacheId);
        } else {
            styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
            d->styleRulesCache[widgetStyleCacheId] = styleRules;
        }
    } else {
        styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
    }

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Number of style rules:" << styleRules.count();
#endif
    if (!styleRules.count()) {
        return;
    }
    const HbVector<HbCss::Declaration> decl = declarations(styleRules, "", widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::polish : Number of matching CSS declarations: " << decl.count();
#endif
    d->ensureLayoutParameters(profile);

    HbCss::ValueExtractor extractor(decl, d->layoutParameters, profile);
    QString layoutName;
    QString sectionName;

    if ( params.count() ) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::polish : Extracting custom properties.";
#endif
        extractor.extractCustomProperties( params.keys(), params.values() );
    }

    bool layoutDefined = extractor.extractLayout(layoutName, sectionName);
#ifdef HBSTYLE_DEBUG
    if (!layoutDefined) {
        qDebug() << "HbStyle::polish : Couldn't find layout name for the widget.";
    } else {
        qDebug() << "HbStyle::polish : Layout name: " << layoutName;
    }
#endif

    QStringList nodeIds;
    HbAnchorLayout *anchorLayout(0);
    if (layoutDefined) {

        // check that we do not override the user defined layout
        const QVariant layoutPtrByStyleV = widget->property( LAYOUT_PTR_PROPERTY );
        const QGraphicsLayout *layoutPtrByStyle = layoutPtrByStyleV.isValid() ? layoutPtrByStyleV.value<QGraphicsLayout*>() : 0;
        const QGraphicsLayout *existingLayoutPtr = widget->layout();

        if ( existingLayoutPtr && (existingLayoutPtr!=layoutPtrByStyle) ) {
            // widget has a layout, but it is not created by the style -> we cannot override.
#ifdef HBSTYLE_DEBUG
            qDebug() << "HbStyle::polish : layout overridden by user";
#endif // HBSTYLE_DEBUG
        } else {
            const QVariant existingLayoutNameV = widget->property( LAYOUT_NAME_PROPERTY );
            const QString existingLayoutName = existingLayoutNameV.isValid() ? existingLayoutNameV.toString() : QString();
            const bool cached = (existingLayoutName == layoutName );
            if ( !cached ) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "LayoutName cache miss.";
#endif
                HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
                if ( !loader->loadWidgetML(widget, layoutName, sectionName)) {
#ifdef HBSTYLE_DEBUG
                    qDebug() << "HbStyle::polish : Failed to load WidgetML";
#endif
                    return;
                }
                const QVariant ptrVariant = QVariant::fromValue( widget->layout() );
                widget->setProperty( LAYOUT_PTR_PROPERTY, ptrVariant );
                widget->setProperty( LAYOUT_NAME_PROPERTY, QVariant( layoutName ) );
            }

            anchorLayout = static_cast<HbAnchorLayout*>(widget->layout());
            if (cached) {
#ifdef HBSTYLE_DEBUG
                qDebug() << "LayoutName cache hit.";
#endif
                anchorLayout->removeMappings();
            }

            anchorLayout->setMapping(anchorLayout, "");
            nodeIds = anchorLayout->nodeIds();
        }
    }

    // polish widget and subitems
    d->polishItem(styleRules, widget, widget, "", profile, false);
    QList<QGraphicsItem*> list = widget->childItems();
    foreach (QGraphicsItem* item, list) {
        QString name = HbStyle::itemName(item);
        if ( anchorLayout && !name.isEmpty() ) {
            // Assuming that all items with "itemName" are widgets.
            anchorLayout->setMapping(static_cast<QGraphicsWidget*>(item), name);
            // Remove from "nodeIds" so that we don't call polishItem
            // twice for this item.
            nodeIds.removeAll(name);
        }
        d->polishItem(styleRules, widget, item, name, profile, layoutDefined);
    }
    foreach (QString nodeId, nodeIds) {
        // These are the "missing" anchor items. Need to call polishItem
        // for them, too, for getting the anchor spacings right.
        // if there are anchor node ids, layoutDefined is always true.
        if ( !nodeId.isEmpty() ) {
            d->polishItem(styleRules, widget, 0, nodeId, profile, true);
        }
    }
    if ( anchorLayout ) {
        QList<HbAnchor*> anchors = anchorLayout->anchors();
        foreach (HbAnchor* anchor, anchors) {
            if ( !anchor->anchorId().isEmpty() ) {
                d->polishAnchor(styleRules, widget, anchor, profile);
            }
        }
    }
}

/*!
    updateThemedItems updates themable children items of widget

    \param styleRules, style-rules of the widget
    \param variableRules, variable-rules of the widget
    \param widget, widget whose themed parameters are supposed to be updated
    \param item, graphics item whose themed parameters are supposed to be updated
*/
void HbStylePrivate::updateThemedItems(
    const HbVector<HbCss::StyleRule> &styleRules,
    QGraphicsItem *item,
    HbDeviceProfile &profile) const
{

    QString name = HbStyle::itemName(item);
    if (name.isEmpty() ) {
        return;
    }

    HbTextItem* text = qgraphicsitem_cast<HbTextItem*>( item );
    HbRichTextItem* richtext = qgraphicsitem_cast<HbRichTextItem*>( item );
    HbIconItem* iconItem = qgraphicsitem_cast<HbIconItem*>( item );
    HbMarqueeItem* marqueeItem = qgraphicsitem_cast<HbMarqueeItem*>( item );
    if(! (text || richtext || iconItem || marqueeItem ) ){
        return;
    }

    const HbVector<HbCss::Declaration> decl = declarations(styleRules, name, 0, profile);

#ifdef HBSTYLE_DEBUG
   qDebug() << "HbStyle::updateThemedItems : -- Number of matching CSS declarations: " << decl.count();
#endif

   HbCss::ValueExtractor extractor(decl, profile);

    QColor col;
    bool extracted = extractor.extractColor( col );
    if (!extracted || !col.isValid()) {
        // Setting non black or white default color to make it visisble in black or white theme
        col.setRgb(255,0,255);
    }
   //apply the themed color to text-item
    if(text) {
#ifdef HBSTYLE_DEBUG
        if ( !extracted ) {
            qDebug() << "HbStyle::getColor : -- No color information found";
        }
#endif
        if ( !HbWidgetBasePrivate::d_ptr(text)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor ) ){
            text->setTextColor(col);
            HbWidgetBasePrivate::d_ptr(text)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
        }
    }

    if(richtext) {
#ifdef HBSTYLE_DEBUG
        if ( !extracted ) {
            qDebug() << "HbStyle::getColor : -- No color information found";
        }
#endif
        if ( !HbWidgetBasePrivate::d_ptr(richtext)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor ) ){
            richtext->setTextDefaultColor(col);
            HbWidgetBasePrivate::d_ptr(richtext)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
        }
    }

    if (iconItem) {
        // Applying color to mono-colorised icons from theme. Using setColor()
        // here would be wrong. It would lead to loosing the user-supplied color
        // in some cases so use the special setThemedColor() instead.
        HbIconItemPrivate::d_ptr(iconItem)->setThemedColor( col );
    }
    if(marqueeItem){
        //applying color to the marquee-item from theme
        if(!HbWidgetBasePrivate::d_ptr(marqueeItem)->testApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor)) {
            marqueeItem->setTextColor( col );
            HbWidgetBasePrivate::d_ptr(marqueeItem)->setApiProtectionFlag(HbWidgetBasePrivate::AC_TextColor, false);
        }
    }
}

/*!
    HbStyle::updateThemedParams()

    Updates the themable parameters of widget (specially those which are specified in css files)

    \param widget, widget whose themed parameters are supposed to be updated
*/
void HbStyle::updateThemedParams(HbWidget *widget)
{
    Q_D( HbStyle );
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::updateThemedParams : -- -- -- --";
#endif
    if( !widget ) {
        return;
    }
    if(!widget->childItems().count()){
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::updateThemedParams : " << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::updateThemedParams : HbLayeredStyleLoader returned a null pointer.";
#endif
        return;
    }
    NODEPTR_N(widget);
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    HbVector<HbCss::StyleRule> styleRules = styleLoader->styleRulesForNode(n, profile.orientation());
    if (!styleRules.count()) {
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::updateThemedParams : No style rules found.";
#endif
        return;
    }

    // update themed items
    QList<QGraphicsItem*> list = widget->childItems();
    foreach (QGraphicsItem* item, list) {
        d->updateThemedItems(styleRules, item, profile);
    }
}

/*!
    Checkes whether given widget has orienation specific style
    rules. This can be used for optimizing orientation switch.
*/
bool HbStyle::hasOrientationSpecificStyleRules(HbWidget *widget)
{
    HbLayeredStyleLoader *styleLoader =
        HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    NODEPTR_N(widget);
    return styleLoader->hasOrientationSpecificStyleRules(n);
}

/*!
    For clearing caches.
*/
void HbStyle::widgetDestroyed(QObject* obj)
{
    Q_UNUSED( obj );
}

/*!

    Sets the item name of a graphics item. Item name is not the same as object name.
    It's a textual identifier that uniquely identifies QGraphicsItem's child items.
    This method is used to map HbWidget's subelements to sub-elements defined in
    CSS/WidgetML layouts.

    Note that polish() method requires widget's children to be set with a correct item name.
    \sa HbStyle::polish()

    This method uses QGraphicsItem::setData() with id 0xfffe.

    \param item, graphics item
    \param name, name to be set
*/
void HbStyle::setItemName( QGraphicsItem *item, const QString &name )
{
    QString originalName = itemName(item);
    if (item && originalName != name) {
        item->setData( ItemName, name );
        QGraphicsItem* parent = item->parentItem();
        QGraphicsLayoutItem* lItem = (item->isWidget()) ? (QGraphicsLayoutItem*)static_cast<QGraphicsWidget*>(item) : 0;
        if (lItem && parent && parent->isWidget()) {
            QGraphicsWidget* parentW = static_cast<QGraphicsWidget*>(parent);
            const QVariant layoutPtrByStyleV = parentW->property( LAYOUT_PTR_PROPERTY );
            const QGraphicsLayout *layoutPtrByStyle = layoutPtrByStyleV.isValid() ? layoutPtrByStyleV.value<QGraphicsLayout*>() : 0;
            if ( parentW->layout() && (parentW->layout()==layoutPtrByStyle)) {
                HbAnchorLayout* layout = static_cast<HbAnchorLayout*>(parentW->layout());
                if ( layout->indexOf(lItem) != -1 ) {
                    if( name.isEmpty() ) {
                        layout->removeMapping(originalName);
                    } else {
                        if ( !originalName.isEmpty() ) {
                            layout->removeMapping(originalName);
                        }
                        layout->setMapping(lItem, name);
                    }
                }
            }
        }
    }
}

/*!

    Returns the item name of a graphics item. Item name is not the same as object name.
    It's a textual identifier that uniquely identifies QGraphicsItem's child items.

    \param item, graphics item
    \return item name or an empty string if name's not set
*/
QString HbStyle::itemName( const QGraphicsItem *item )
{
    return item
        ? item->data( ItemName ).toString()
        : QString();
}

/*!
    Returns a value of a global style parameter. The value is returned in pixels.

    Available parameters can be found from hbglobalparameters.css. By using these
    parameters applications get consistent look.

    \param param Name of the global parameter.
    \param value Returns value of the global parameter.
    \param profile Device profile of the used HbMainWindow. Primary display's.
            device profile HbDeviceProfile::current() is used if the value is omitted.
    \return true if the variable was found.
*/
bool HbStyle::parameter(const QString& param, qreal& value, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }
    Q_D( const HbStyle );
    d->ensureLayoutParameters(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(d->layoutParameters, true, effectiveProfile);
    // todo: parsing variable/expression is done here so that there is no need to change API
    // also parameters method not changed (this change is done for docml/widgetml parsing)
    if (param.startsWith(QLatin1String("var(")) && param.endsWith(QLatin1String(")"))) {
        return valueExtractor.extractVariableValue(param.mid(4,param.length()-5), value);
    } else if (param.startsWith(QLatin1String("-var(")) && param.endsWith(QLatin1String(")"))) {
        bool retVal = valueExtractor.extractVariableValue(param.mid(5,param.length()-6), value);
        value = -value;
        return retVal;
    } else if (param.startsWith(QLatin1String("expr(")) && param.endsWith(QLatin1String(")"))) {
        QString expressionString = param.mid(5,param.length()-6);
        return valueExtractor.extractExpressionValue(expressionString, value);
    } else if (param.startsWith(QLatin1String("-expr(")) && param.endsWith(QLatin1String(")"))) {
        QString expressionString = param.mid(6,param.length()-7);
        bool retVal = valueExtractor.extractExpressionValue(expressionString, value);
        value = -value;
        return retVal;
    }

    return valueExtractor.extractVariableValue(param, value);
}

/*!
    Returns copy of all global style parameters. Both names and values
    of the parameters are returned. The values are returned in pixels.

    Available parameters can be found from hbglobalparameters.css. By using these
    parameters applications get consistent look. Usage of this API (instead of parameter)
    is recommended if an application needs to fetch several parameters in one place.

    \param params Contains names and values of all global style parameters.
    \param profile Device profile of the used HbMainWindow. Primary display's
            device profile HbDeviceProfile::current() is used if the value is omitted.
*/

void HbStyle::parameters(HbStyleParameters &params, const HbDeviceProfile &profile) const
{
    HbDeviceProfile effectiveProfile = profile;
    if ( effectiveProfile.isNull() ) {
        effectiveProfile = HbDeviceProfile::current();
    }

    Q_D( const HbStyle );
    d->ensureLayoutParameters(effectiveProfile);
    HbCss::ValueExtractor valueExtractor(d->layoutParameters, true, effectiveProfile);
    qreal value = 0;
    QHash<QString, HbCss::Declaration>::const_iterator i = d->layoutParameters.constBegin();
    while (i != d->layoutParameters.constEnd()) {
        if (valueExtractor.extractVariableValue(i.key(), value)) {
            params.addParameter(i.key(), value);
        }
        ++i;
    }
}

/*!
    Returns values for widget specific style parameters. The names of the parameters
    are passed in with \a params.

    This method should be used only if you need to access widget specific parameters out
    of the polish loop. It is more efficient to use HbWidget::polish(HbStyleParameters &params)
    if you don't need to access the parameters before the (first) polish event.

    \param parameters, style parameters to be returned to the caller
    \param widget, HbWidget to be polished
    \sa HbStyle::polish, HbStyle::parameters and HbWidget::polish
*/
void HbStyle::widgetParameters(HbStyleParameters &params, HbWidget* widget) const
{
    Q_D( const HbStyle );
    if( !widget || !params.count() ) {
        return;
    }
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::widgetParameters : Parameters for" << widget->metaObject()->className();
#endif

    HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Layouts);
    if(!styleLoader){
#ifdef HBSTYLE_DEBUG
        qDebug() << "HbStyle::widgetParameters : HbLayeredStyleLoader returned a null pointer.";
#endif
        return;
    }
    HbDeviceProfile profile(HbDeviceProfile::profile(widget));
    NODEPTR_N(widget);

    HbVector<HbCss::StyleRule> styleRules = styleLoader->styleRulesForNode(n, profile.orientation());

#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::widgetParameters : Number of style rules:" << styleRules.count();
#endif
    if (!styleRules.count()) {
        return;
    }
    const HbVector<HbCss::Declaration> decl = declarations(styleRules, "", widget, profile);
#ifdef HBSTYLE_DEBUG
    qDebug() << "HbStyle::widgetParameters : Number of matching CSS declarations: " << decl.count();
#endif
    d->ensureLayoutParameters(profile);

    HbCss::ValueExtractor extractor(decl, d->layoutParameters, profile);
    extractor.extractCustomProperties( params.keys(), params.values() );
}


/*!
\internal
*/
HbStylePrivate::HbStylePrivate()
{
    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    if(loader){
        loader->addFilePath(STYLE_LOCATION, HbLayeredStyleLoader::Concern_Layouts,
                HbLayeredStyleLoader::Priority_Core);
        loader->addFilePath(COLOR_STYLE_LOCATION, HbLayeredStyleLoader::Concern_Colors,
                HbLayeredStyleLoader::Priority_Core);
    }
}

/*!
\internal
*/
HbStylePrivate::~HbStylePrivate()
{
    layoutParameters.clear();

    HbWidgetStyleLoader *loader = HbWidgetStyleLoader::instance();
    if(loader){
        loader->removeFilePath(STYLE_LOCATION, HbLayeredStyleLoader::Concern_Layouts,
                HbLayeredStyleLoader::Priority_Core);
        loader->removeFilePath(COLOR_STYLE_LOCATION, HbLayeredStyleLoader::Concern_Colors,
                HbLayeredStyleLoader::Priority_Core);
    }
}

/*!
\internal
*/
QString HbStylePrivate::logicalName(HbStyle::Primitive primitive, const QStyleOption *option) const
{
    switch (primitive) {

    case HbStyle::P_Slider_thumb: {
        const HbStyleOptionSlider *opt = qstyleoption_cast<const HbStyleOptionSlider *>(option);
        if (opt) {
            QString iconPath;
            switch (opt->orientation) {

            case Qt::Horizontal:
                if (opt->state & QStyle::State_Sunken)
                    iconPath = "qtg_graf_slider_h_handle_pressed";
                else
                    iconPath = "qtg_graf_slider_h_handle_normal";
                return iconPath;

            case Qt::Vertical:
                if (opt->state & QStyle::State_Sunken)
                    iconPath = "qtg_graf_slider_v_handle_pressed";
                else
                    iconPath = "qtg_graf_slider_v_handle_normal";
                return iconPath;

            default:
                break;

            }
        }
    }

    default:
        break;

    }

    return QString();
}

/*!
\internal
*/
QIcon::Mode HbStylePrivate::iconMode(QStyle::State state) const
{
    QIcon::Mode mode = QIcon::Disabled;
    if (state & QStyle::State_Enabled)
        mode = QIcon::Normal;
    if (state & QStyle::State_Active)
        mode = QIcon::Active;
    if (state & QStyle::State_Selected)
        mode = QIcon::Selected;
    return mode;
}

/*!
\internal
*/
QIcon::State HbStylePrivate::iconState(QStyle::State state) const
{
    QIcon::State icon = QIcon::Off;
    if (state & QStyle::State_On)
        icon = QIcon::On;
    return icon;
}


/*!
\internal
*/
void HbStylePrivate::ensureLayoutParameters(const HbDeviceProfile &profile) const
{
    bool firstParse(layoutParameters.isEmpty());
    bool addSpecialParams(firstParse);
    if (firstParse) {
        HbCss::Parser parser;
        parser.init(GLOBAL_PARAMETERS_LOCATION, true);
        HbCss::StyleSheet *styleSheet = HbMemoryUtils::create<HbCss::StyleSheet>(HbMemoryManager::HeapMemory);
        parser.parse(styleSheet);

        HbStyleSelector selector;
        selector.styleSheets.append(styleSheet);
        selector.variableRuleSets(&layoutParameters);
    }
    if (!firstParse && (layoutParametersProfileName != profile.name())) {
        addSpecialParams = true;
    }
    if (addSpecialParams) {
        layoutParametersProfileName = profile.name();
        QSizeF pSize = profile.logicalSize();

        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-width";
            decl.propertyId = HbCss::Property_Unknown;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)pSize.width(),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.insert(decl.property, decl);
        }
        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-height";
            decl.propertyId = HbCss::Property_Unknown;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)pSize.height(),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.insert(decl.property, decl);
        }
        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-short-edge";
            decl.propertyId = HbCss::Property_Unknown;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)qMin(pSize.height(),pSize.width()),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.insert(decl.property, decl);
        }
        {
            HbCss::Declaration decl;
            decl.property = "hb-param-screen-long-edge";
            decl.propertyId = HbCss::Property_Unknown;
            HbCss::Value val;
            val.type = HbCss::Value::Number;
            val.variant = HbVariant((double)qMax(pSize.height(),pSize.width()),HbMemoryManager::HeapMemory);
            decl.values.append(val);
            layoutParameters.insert(decl.property, decl);
        }
    }
}

/*!
\internal
*/
void HbStylePrivate::clearStyleSheetCaches()
{
    styleRulesCache.clear();
}

HbWidgetBasePrivate *HbStylePrivate::widgetBasePrivate(HbWidgetBase *widgetBase) const
{
    return HbWidgetBasePrivate::d_ptr(widgetBase);
}

#include "moc_hbstyle.cpp"
