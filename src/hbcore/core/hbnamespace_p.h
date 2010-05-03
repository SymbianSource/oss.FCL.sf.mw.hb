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

#ifndef HBNAMESPACE_P_H
#define HBNAMESPACE_P_H

#include <QGraphicsItem>

/*
    The Hb namespace contains miscellaneous private identifiers used throughout the Hb library
*/
namespace HbPrivate
{
    // The following values are defined in units and used by HbTextItem
    // and HbMarqueeItem
    static const qreal TextTruncationFadeWidth = 6.0; //un
    static const qreal TextTruncationFadeHeight = 5.0; //un


    enum GraphicsItemType
    {
        ItemType_PopupBackGround = QGraphicsItem::UserType+1,
        ItemType_FadeItem,
        ItemType_LongPressVisualizer,
        ItemType_BatteryIndicator,
        ItemType_Decorator,
        ItemType_IndicatorGroup,
        ItemType_IndicatorMenu,
        ItemType_NaviPane, // deprecated
        ItemType_SignalIndicator,
        ItemType_SoftKey, // deprecated
        ItemType_NavigationButton,
        ItemType_IndicatorButton,
        ItemType_TitlePane,
        ItemType_MessageBoxContentWidget,
        ItemType_MessageNoteContentWidget,
        ItemType_ProgressDialogContentWidget,
        ItemType_GraphicsItemGroup,
        ItemType_BackgroundItem,
        ItemType_MenuWidget,
        ItemType_MenuItem,
        ItemType_MenuListView,
        ItemType_ProgressSliderHandle,
        ItemType_SliderPopupContentWidget,
        ItemType_SliderControl,
        ItemType_ProgressSliderControl,
        ItemType_SliderElement,
        ItemType_SliderHandle,
        ItemType_SliderTickmarks,
        ItemType_SliderTickmarksLabel,
        ItemType_ToolTipLabel,
        ItemType_ColorDialogContentWidget,
        ItemType_CQContentWidget,
        ItemType_HbInputDialogContentWidget,
        ItemType_ConfirmationQueryContentWidget,
        ItemType_ListQueryContentWidget,
        ItemType_ListDialogContentWidget,
        ItemType_ColorGridViewItem,
        ItemType_MessageQueryContentWidget,
        ItemType_SettingFormHeadingWidget,
        ItemType_SettingItem,
        ItemType_SettingGroupHeadingWidget,
        ItemType_SettingGroup,
        ItemType_DataFormHeadingWidget,
        ItemType_DataItem,
        ItemType_DataGroupHeadingWidget,
        ItemType_DataGroup,
        ItemType_GroupBoxHeadingWidget,
        ItemType_GroupBoxContentWidget,
        ItemType_TitleBar,
        ItemType_TitleBarHandle,
        ItemType_StatusBar,
        ItemType_VolumeSlider,
        ItemType_ZoomSlider,
        ItemType_EndPrivate

    };

    /*
        LibHb's predefined Z values.

        This enum describes the Z values available in LibHb.
     */
    enum HbZValues
    {
        RootItemZValue,                  /* Z value of the root parent item */
        BackgroundZValue,                /* Z value of the background layer. */
        ContentZValue,                   /* Z value of the content layer. */
        TitleBarZValue,                  /* Z value of the titlebar. */
        StatusBarZValue,                 /* Z value of the statusbar. */
        ToolBarZValue,                   /* Z value of the toolbar. */
        DockWidgetZValue,                /* Z value of the dockwidget item. */
        TitlePaneZValue,                 /* Z value of the title pane. */
        NaviPaneZValue,                  /* Z value of the navi pane. */
        SoftKeyZValue,                   /* Z value of the soft keys. */
        IndicatorGroupZValue,            /* Z value of the indicator group. */
        FadingItemZValue,                /* Z value of the fading item. */


        PopupZValueRangeStart    = 0x00004000,    /* Z value range start of the popup layer */
        PopupZValueDefaultOffset = 0x00044000,    /* Default Z value offset for popup layer */
        PopupZValueRangeEnd      = 0x00084000     /* Z value range end of the popup layer */
    };

    /*
        LibHb's predefined item change values.

        This enum describes the item change values available in LibHb.
     */
    enum HbItemChangeValues
    {
        ItemOrientationChange = 10000
    };

    //
    // Note: qreal type is used as zValue of graphics items including HbDialog objects. In some certain platforms (like in ARM)
    // qreal is defined as float. For a 32 bits float the number of significant digits is about 7.
    // It means that the Hb can have
    // at most 7 digits precision using qreal. So, the constants below has to be chosen so that the precision of Z values
    // used in popup is within the 7 digits range.
    //

    // The number of different priorities currently equals the max number can be represented on quint8 + 1
    static const int PopupPriorityCount = 256;

    // The number of popups that can have the same priority
    static const int PopupCountWithSamePriority = 64;

    static const qreal PopupWithDifferentPriorityZValueUnit  = (PopupZValueRangeEnd-PopupZValueDefaultOffset)/PopupPriorityCount;
    static const qreal PopupWithSamePriorityZValueUnit       = PopupWithDifferentPriorityZValueUnit / PopupCountWithSamePriority;
    static const qreal PopupBackgroundItemZValueUnit         = PopupWithSamePriorityZValueUnit / 2;
    static const qreal FadingItemZValueUnit                  = PopupBackgroundItemZValueUnit / 2;
    static const qreal VKBValueUnit                          = FadingItemZValueUnit / 2;
    static const qreal SelectionControlHandlesValueUnit      = VKBValueUnit / 2;

    enum SoftKeyId
    {
        PrimarySoftKey,
        SecondarySoftKey,       
        MiddleSoftKey,          

        //Custom softkey id's start here.
        CustomSoftKey = 0xFF    
    };
}

#endif // HBNAMESPACE_P_H

