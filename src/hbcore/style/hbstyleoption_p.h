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

#ifndef HBSTYLEOPTION_H
#define HBSTYLEOPTION_H

#include <hbglobal.h>
#include <QStyleOptionGraphicsItem>


class HB_CORE_PRIVATE_EXPORT HbStyleOption : public QStyleOptionGraphicsItem
{
public:
    HbStyleOption(int type = HbSO_Widget, int version = 1);
    HbStyleOption(const HbStyleOption& other);
    ~HbStyleOption();

    enum OptionType {
        HbSO_Widget = SO_CustomBase,
        HbSO_PushButton,
        HbSO_ToolButton,
        HbSO_MenuItem,
        HbSO_Menu,
        HbSO_Label,
        HbSO_Combo,
        HbSO_ScrollBar,
        HbSO_Slider,
        HbSO_SliderElement,
        HbSO_ProgressBar,
        HbSO_ProgressSlider,
        HbSO_ProgressSliderHandle,
        HbSO_ListViewItem,
        HbSO_Popup,
        HbSO_MessageBox,
        HbSO_RatingSlider,
        HbSO_ProgressDialog,
        HbSO_ToolTip,
        HbSO_AbstractViewItem,
        HbSO_ToolBarExtension,
        HbSO_InputPopup,
        HbSO_GridViewItem,
        HbSO_CheckBox,
        HbSO_TitlePane,
        HbSO_IndicatorGroup,
        HbSO_NavigationButton,
        HbSO_IndicatorButton,
        HbSO_SignalIndicator,
        HbSO_BatteryIndicator,
        HbSO_IndicatorMenu,
        HbSO_NotificationDialog,
        HbSO_TreeViewItem,
        HbSO_SelectionControl,
        HbSO_ColorGridViewItem,
        HbSO_DataForm,
        HbSO_DataItem,
        HbSO_DataFormViewItem,
        HbSO_DataGroup,
        HbSO_DataGroupHeadingWidget,
        HbSO_GroupBox,
        HbSO_IndexFeedback,
        HbSO_StatusBar,
        HbSO_InputDialog,
        // ...
        HbSO_CustomBase = 0xf0000000
    };

    enum StyleOptionType { Type = HbSO_Widget };
    enum StyleOptionVersion { Version = 1 };

    QFont font;
    QRectF boundingRect;
};

#endif // HBSTYLEOPTION_H
