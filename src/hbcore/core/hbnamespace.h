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

#ifndef HBNAMESPACE_H
#define HBNAMESPACE_H

#include <QtGlobal>
#include <QObject>
#include <QGraphicsItem>
#include <hbglobal.h>

#ifndef HB_DOXYGEN
class HB_CORE_EXPORT Hb
{
    Q_GADGET
    Q_ENUMS( SoftKeyId SoftKeyAction NavigationAction UiAttribute TouchGesture
             ItemDataRole GraphicsItemType LogicalFontName SceneItem
             InstantInteraction ContinuousInteraction InteractionModifier
             TextCursorVisibility SliderTickPosition ModelItemType TextWrapping
             ActivationReason )
    Q_FLAGS( UiAttributes TouchGestures SceneItems SliderTickPositions
             TextContextMenuFlags )

public:
#else
namespace Hb
{
#endif // HB_DOXYGEN


    static const int Key_SoftKeyPrimary    = Qt::Key_Launch0;
    static const int Key_SoftKeySecondary  = Qt::Key_Launch1;
    static const int Key_SoftKeyMiddle     = Qt::Key_Launch2;


    // DEPRECATED - DO NOT USE - These are deprecated along with deprecation of softkey functions in HbMainWindow.
    // This enumeration will become internal to Hb.
    enum SoftKeyId
    {
        PrimarySoftKey,
        SecondarySoftKey,       
        MiddleSoftKey,          

        //Custom softkey id's start here.
        CustomSoftKey = 0xFF    
    };

    // DEPRECATED - DO NOT USE - These are deprecated along with deprecation of softkey functions in HbMainWindow.
    // Use NavigationAction instead.
    enum SoftKeyAction
    {
        QuitAction,
        BackAction,
        ConfirmAction,
        DoneAction
    };

    enum NavigationAction
    {
        QuitNaviAction,
        BackNaviAction,
        ConfirmNaviAction,
        DoneNaviAction
    };
    
    enum EffectEvent
    {
        EffectFinished                = 1,
        EffectCancelled               = 2,
        EffectNotStarted              = 3
    };

    
    enum UiAttribute
    {
        Ui_NoAttributes         = 0x0000,
        Ui_VisibleInTouch       = 0x0001,
        Ui_VisibleInNonTouch    = 0x0002
    };

    Q_DECLARE_FLAGS(UiAttributes, UiAttribute)

    /*!
        Gesture types.
     */
    enum TouchGesture
    {
        TapGesture         = 0x0000001,
        LongPressGesture   = 0x0000002,
        PanGesture         = 0x0000004,
        FlickGesture       = 0x0000008
    };

    Q_DECLARE_FLAGS(TouchGestures, TouchGesture)

    
    enum ItemDataRole
    {
        ItemTypeRole = 1001,
        IndexFeedbackRole, 
        UserRole = 1500
    };

    enum ModelItemType
    {
        StandardItem,
        ParentItem,
        SeparatorItem,
        UserItem = 1000
    };
    
    enum GraphicsItemType
    {
        ItemType_NotificationDialog = QGraphicsItem::UserType+10000,
        ItemType_FrameItem,
        ItemType_GraphicsPixmapItem,
        ItemType_FormViewItem,
        ItemType_FormView,
        ItemType_AbstractItemContainer,
        ItemType_AbstractItemView,
        ItemType_ListView,
        ItemType_AbstractViewItem,
        ItemType_ListViewItem,
        ItemType_ItemHighlight,
        ItemType_ListWidgetItem,
        ItemType_ListWidgetViewItem,
        ItemType_GridViewItem,
        ItemType_Popup,
        ItemType_Dialog,
        ItemType_MessageBox,
        ItemType_ProgressNote,
        ItemType_ProgressDialog,
        ItemType_AbstractButton,
        ItemType_AbstractItem,
        ItemType_AbstractSlider,
        ItemType_Highlight,
        ItemType_IconItem,
        ItemType_Label,
        ItemType_AbstractEdit,
        ItemType_LineEdit,
        ItemType_Menu,
        ItemType_MenuItem,
        ItemType_ProgressSlider,
        ItemType_ProgressiveSlider,
        ItemType_RatingSlider,
        ItemType_ProgressBar,
        ItemType_PushButton,
        ItemType_RadioButtonList,
        ItemType_ScrollBar,
        ItemType_Slider,
        ItemType_SliderPopup,
        ItemType_StackedWidget,
        ItemType_TextEdit,
        ItemType_TextItem,
        ItemType_ToolBar,
        ItemType_ToolBarExtension,
        ItemType_ToolButton,
        ItemType_Widget,
        ItemType_View, 
        ItemType_RoundRobinLabel,
        ItemType_ConfirmationQuery,
        ItemType_CheckBox,
        ItemType_MessageQuery,
        ItemType_InputDialog,
        ItemType_TransparentWindow,
        ItemType_DateTimeEdit,
        ItemType_ColorDialog,
        ItemType_InputCharacterButton,
        ItemType_InputFunctionButton,
        ItemType_InputCloseButton,
        ItemType_InputTabButton,
        ItemType_InputVkbWidget,
        ItemType_InputCharPreviewPane,
        ItemType_VirtualTrackPoint,
        ItemType_WritingBox,
        ItemType_VolumePopup,
        ItemType_CombinedQuery,
        ItemType_ListQuery,
        ItemType_ListDialog,
        ItemType_ListDialogContentWidget,
        ItemType_ZoomPopup,
        ItemType_CarouselView,
        ItemType_TreeViewItem,
        ItemType_ComboBox,
        ItemType_TouchArea,
        ItemType_DataItemContainer,
        ItemType_DataFormViewItem,
        ItemType_DataForm,
        ItemType_RichTextItem,
        ItemType_GroupBox,
        ItemType_DockWidget,
        ItemType_RadioButtonListViewItem,
        ItemType_TumbleView,
        ItemType_TumbleViewItem,
        ItemType_DateTimePicker,
        ItemType_MarqueeItem,
        ItemType_IndexFeedback,
        ItemType_Last = QGraphicsItem::UserType + 20000
    };

    enum WidgetAttribute
    {
        InteractionDisabled   = 0x00000001,
        InsidePopup = 0x00000002,
        InputMethodNeutral = 0x00000004
    };

    Q_DECLARE_FLAGS(WidgetAttributes, WidgetAttribute)
    

    // DEPRECATED - DO NOT USE - These are deprecated along with deprecation of HbFontProvider
    // Use HbFontSpec and the font roles defines in HbFontSpec API
    enum LogicalFontName
    {
        FontUndefined = 0,
        FontPrimary,
        FontSecondary,
        FontTitle,
        FontPrimarySmall,
        FontDigital
    };

   
    enum SceneItem {
        NoItem                = 0x00000000,
        
        // Deprecated
        TitlePaneItem         = 0x00000001,
        // Deprecated
        NaviPaneItem          = 0x00000002,
        ToolBarItem           = 0x00000004,
        // Deprecated
        IndicatorItems        = 0x00000008,
        DockWidgetItem        = 0x00000010,
        TitleBarItem          = 0x00000020,
        StatusBarItem         = 0x00000040,

        // Deprecated
        PrimarySoftKeyItem    = 0x00010000,
        // Deprecated
        SecondarySoftKeyItem  = 0x00020000,
        // Deprecated
        MiddleSoftKeyItem     = 0x00040000,
        // Deprecated
        SoftKeyItems          = 0x00070000,

        AllItems              = 0xFFFFFFFF
    };
    
    /*!
       Anchor edges.
     */
    enum Edge {
        LeftEdge = 0,
        TopEdge,
        RightEdge,
        BottomEdge,
        CenterHEdge,
        CenterVEdge
    };

    enum ResourceType {
        IconResource = 0,
        ThemeResource,
        EffectResource,
        StyleSheetResource
    };

    Q_DECLARE_FLAGS(SceneItems, SceneItem)
    
    enum WindowFlag {
        WindowFlagNone            = 0x00000000,
        WindowFlagTransparent     = 0x00000001,
        WindowFlagNoBackground    = 0x00000002,
        WindowFlagFixedVertical   = 0x00000004,
        WindowFlagFixedHorizontal = 0x00000008,

        // Deprecated
        WindowFlagNoSplash        = 0x00000010
    };

    Q_DECLARE_FLAGS(WindowFlags, WindowFlag)

    enum ApplicationFlag {
        DefaultApplicationFlags = 0x00,
        NoSplash                = 0x01,
        SplashFixedVertical     = 0x02,
        SplashFixedHorizontal   = 0x04
    };

    Q_DECLARE_FLAGS(ApplicationFlags, ApplicationFlag)

    enum ViewSwitchFlag {
        ViewSwitchDefault       = 0x00,
        ViewSwitchSequential    = 0x01,
        ViewSwitchFullScreen    = 0x02,
        ViewSwitchUseNormalAnim = 0x04,
        ViewSwitchUseBackAnim   = 0x08,
        ViewSwitchUseAltEvent   = 0x10
    };

    Q_DECLARE_FLAGS(ViewSwitchFlags, ViewSwitchFlag)

    enum TextCursorVisibility
    {
        TextCursorHidden = 0,
        TextCursorVisible
    };

    enum SliderTickPosition
    {
        NoSliderTicks = 0,
        SliderTicksAbove = 1,
        SliderTicksLeft = SliderTicksAbove,
        SliderTicksBelow = 2,
        SliderTicksRight = SliderTicksBelow,
        SliderTicksBothSides = 3,
        SliderTicksAbsolute
    };

    Q_DECLARE_FLAGS(SliderTickPositions , SliderTickPosition)

    enum TextContextMenuFlag {
        ShowTextContextMenuOnSelectionClicked = 1,
        ShowTextContextMenuOnLongPress = 4
    };
    Q_DECLARE_FLAGS(TextContextMenuFlags , TextContextMenuFlag)
    
    /*!
        A type definition for text wrapping
    */
    enum TextWrapping
    {
        TextNoWrap = 0,
        TextWordWrap = 1,
        TextWrapAnywhere = 3
    };
        
    enum InstantInteraction {
        InstantPressed = 0,
        InstantReleased,
        InstantClicked,
        InstantKeyRepeated,
        InstantLongPressed,
        InstantDraggedOver,
        InstantFlicked,
        InstantPopupOpened,
        InstantPopupClosed,
        InstantBoundaryReached,
        InstantRotated90Degrees,
        InstantSelectionChanged,
        InstantMultitouchActivated,
        /* add new standard instant interactions here */
        NumberOfInstantInteractions,
        InstantUser = 1000,
        /* value range for user defined custom instant interactions */
        InstantMaxUser = 65535
    };

    enum ContinuousInteraction {
        ContinuousScrolled = 0,
        ContinuousDragged,
        ContinuousPinched,
        ContinuousRotated,
        /* add new standard continuous interactions here */
        NumberOfContinuousInteractions,
        ContinuousUser = 1000,
        /* value range for user defined custom continuous interactions */
        ContinuousMaxUser = 65535
    };

    enum InteractionModifier {
        ModifierSliderHandle = 0x1,
        ModifierSliderElement = 0x2,
        ModifierExpandedItem = 0x4,
        ModifierCollapsedItem = 0x8,
        ModifierScrolling = 0x10
    };
 
    Q_DECLARE_FLAGS(InteractionModifiers, InteractionModifier)
    

    enum ActivationReason {
        ActivationReasonActivity = 0,
        ActivationReasonService,
        ActivationReasonNormal
    };
    
};
Q_DECLARE_METATYPE(Hb::GraphicsItemType)

Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::UiAttributes)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::TouchGestures)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::SceneItems)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::WindowFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::ApplicationFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::ViewSwitchFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::SliderTickPositions)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::TextContextMenuFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Hb::InteractionModifiers)

#endif // HBNAMESPACE_H

