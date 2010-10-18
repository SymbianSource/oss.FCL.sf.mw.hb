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

#include <hbnamespace.h>

/*!
    @stable
    @hbcore
    \namespace Hb
    \brief The Hb namespace contains miscellaneous identifiers used throughout the Hb library.
*/

/*!
    \enum Hb::NavigationAction
    LibHb's predefined standard navigation actions.
    This enum describes the standard Navigations actions available in LibHb.
*/

/*! \var Hb::NavigationAction Hb::QuitNaviAction
    Navigation action for quitting the application.
*/

/*! \var Hb::NavigationAction Hb::BackNaviAction
    Navigation action for back.
*/

/*! \var Hb::NavigationAction Hb::ConfirmNaviAction
    Navigation action for confirmation.
*/

/*! \var Hb::NavigationAction Hb::DoneNaviAction
    Navigation action for done.
*/

/*!
    \enum Hb::EffectEvent
    LibHb's predefined reasons for effect to finish.
    This enum describes the different reasons for a effect to finish in LibHb.
*/

/*! \var Hb::EffectEvent Hb::EffectFinished
    Effect finished normally.
*/

/*! \var Hb::EffectEvent Hb::EffectCancelled
    Effect finished because it was cancelled.
*/

/*! \var Hb::EffectEvent Hb::EffectNotStarted
    Effect was not started. If effectFinished callback has this status,
    it has been invoked synchronously in HbEffect::start.
*/

/*!
    \enum UiAttribute
        LibHb's predefined UI attributes.
        This enum describes the UI attributes available in LibHb.
*/

/*! \var Hb::UiAttribute Hb::Ui_NoAttributes
    No UI attributes.
*/

/*! \var Hb::UiAttribute Hb::Ui_VisibleInTouch
    The visibility of an UI component in touch device.
*/

/*! \var Hb::UiAttribute Hb::Ui_VisibleInNonTouch
    The visibility of an UI component in non-touch device.
*/

/*!
   \enum TouchGesture
   Enum that for different touch gestures.
*/

/*! \var Hb::TouchGesture Hb::Tap
    Tap (often referred to as "click").
*/
/*! \var Hb::TouchGesture Hb::LongPress
    Long press.
*/
/*! \var Hb::TouchGesture Hb::Pan
     Panning gesture.
*/
/*! \var Hb::TouchGesture Hb::Flick
    Flicking gesture.
*/

/*!
    \enum ItemDataRole
    LibHb's predefined item data roles.
    This enum describes the item data roles in LibHb.
*/
/*! \var Hb::ItemDataRole Hb::ItemTypeRole
    This role specifies the type of the model item. If no type is specified StandardItem type is
    used.
*/

/*! \var Hb::ItemDataRole Hb::IndexFeedbackRole
    Context of this role is an index about model item when scrolling takes place by dragging scrollbar.

    \sa HbIndexFeedback
*/

/*! \var Hb::ItemDataRole Hb::UserRole
    The first role that can be used for application-specific purposes.
*/

/*!
    \enum ModelItemType
    LibHb's predefined model item types.

    This enum describes the model item types in LibHb.
*/
/*! \var Hb::ModelItemType Hb::StandardItem
    This is the default item type. If item type is not specified this is used.
*/
/*! \var Hb::ModelItemType Hb::ParentItem
    This is the parent item type. Parent item is an item that has or can have child items.
*/
/*! \var Hb::ModelItemType Hb::SeparatorItem
    This is the separator item type. Separator is used as boundary between separate items or group of items.
*/
/*! \var Hb::ModelItemType Hb::UserItem
    The first item type that can be used for application-specific purposes.
*/

/*!
    \enum GraphicsItemType
        LibHb's graphics item types.

        This enum describes graphics item types available in LibHb.
        The type is returned by method QGraphicsItem::type().
     */

/*!
    \enum WidgetAttribute
        Hb widget attributes.

        This enum is used to specify various Hb widget attributes.
     */

/*! \var Hb::WidgetAttribute Hb::InteractionDisabled
    Indicates that the widget has interaction support disabled.
*/

/*! \var Hb::WidgetAttribute Hb::InsidePopup
    Indicates whether widget and its children (classes derived from HbWidgetBase) are inside popup.
*/


/*!  \var Hb::WidgetAttribute Hb::Widget

    Indicates that a HbWidgetBase derived item is Hb widget. Constructors of HbWidget set this flag on.
*/

/*!
   \enum ListContentType
       LibHb's predefined (list) item content types.

       This enum describes the (list) item content types available in LibHb.
    */

/*!
    \enum ListItemColumn
        LibHb's predefined (list) item columns.

        This enum describes the (list) item columns available in LibHb.
     */

/*!
    \enum ContentVisibility
        LibHb's predefined (list) item content visibility values.

        This enum describes the (list) item content visibility values available in LibHb.
     */

/*!
   \enum SceneItem
       LibHb's built-in scene items.

       This enum describes the scene items available in LibHb.
    */

/*!
   \enum ResourceType
       Resource types.

       This enum describes the available types of resources in LibHb.
    */

/*!
    \enum WindowFlag
        HbMainWindow's window flags.

        This enum describes the window flags which could be set to HbMainWindow.
 */

/*! \var Hb::WindowFlag Hb::WindowFlagNone
    No window flags (default)
*/

/*! \var Hb::WindowFlag Hb::WindowFlagTransparent
    Indicates that HbMainWindow should use alpha blending for composition.
    With this flag you can have transparent areas in UI. Setting this flag will
    cause HbMainWindow to set Qt::FramelessWindowHint to it's flags and
    Qt::WA_TranslucentBackground to it's attributes.
*/

/*! \var Hb::WindowFlag Hb::WindowFlagNoBackground
    Indicates that the background item should not be created and added to the scene.
    As a result the standard themed background will not be visible.
*/

/*! \var Hb::WindowFlag Hb::WindowFlagFixedVertical
    Indicates that the application uses vertical orientation at startup. If flag is not
    used or both WindowFlagFixedVertical and WindowFlagFixedHorizontal are used at the same time,
    both flags are ignored and the orientation is managed automatically based on hw sensors.
*/

/*! \var Hb::WindowFlag Hb::WindowFlagFixedHorizontal
    Indicates that the application uses horizontal orientation at startup. If flag is not
    used or both WindowFlagFixedVertical and WindowFlagFixedHorizontal are used at the same time,
    both flags are ignored and the orientation is managed automatically based on hw sensors.
*/

/*!
  \enum Hb::ApplicationFlag

  Flags controlling HbApplication.
*/

/*! \var Hb::ApplicationFlag Hb::DefaultApplicationFlags

    Default value for application flags. Enables the splash screen.
*/

/*! \var Hb::ApplicationFlag Hb::NoSplash

  Disables the splash screen that is shown during HbApplication construction. Note that
  the splash screen can still be launched manually using HbSplashScreen::start().
*/

/*! \var Hb::ApplicationFlag Hb::SplashFixedVertical

  Indicates that the application will force its orientation to vertical. As a result the
  splash screen will also be forced to vertical orientation.

  \sa HbSplashScreen::FixedVertical
*/

/*! \var Hb::ApplicationFlag Hb::SplashFixedHorizontal

  Indicates that the application will force its orientation to horizontal. As a result the
  splash screen will also be forced to horizontal orientation.

  \sa HbSplashScreen::FixedHorizontal
*/

/*! \var Hb::ApplicationFlag Hb::ForceQtSplash

  Forces the usage of QWidget-based splash screens, even on platforms where a more
  optimal, non-Qt dependent implementation would be available. Use this when it must be
  guaranteed that the splash screen will use the same graphics system as Qt (e.g. because
  the application forces 'raster'). The downside is a decrease in performance and user
  experience, because in this case the splash can only be shown after constructing
  QApplication.

  \sa HbSplashScreen::ForceQt
*/

/*! \var Hb::ApplicationFlag Hb::ShowSpashWhenStartingToBackground

  Does not suppress the splash when the application is started to background.
  Normally HbSplashScreen::start() will do nothing if the process command line
  indicates that the application is started to background (by specifying the
  corresponding flags in the system startup list entry or by starting to
  background as a service). This flag disables the check.

  \sa HbSplashScreen::ShowWhenStartingToBackground
*/

/*!
  \enum Hb::ViewSwitchFlag

  Flags controlling the behavior of HbMainWindow::setCurrentView().

  When the \i animate parameter of HbMainWindow::setCurrentView() is
  \i false, no effects are shown during the view switch and any
  related flags are ignored.
 */

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchDefault

  The view switch hide and show effects (if present) are applied concurrently (in
  parallel) to the view content only. In addition, the framework decides if the normal
  effect events ("hide" and "show") or the "going back" versions ("hide_back" and
  "show_back") are used. This decision is based on the index of the views in the stack,
  i.e. the order in which they were added to the mainwindow. E.g. when moving from view A,
  that is added before view B, to view B, the normal "going forward" behavior is assumed
  (ViewSwitchUseNormalAnim). On the other hand when switching from view B to A, the
  behavior is assumed to be "going backwards" (ViewSwitchUseBackAnim).
 */

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchSequential

  Executes the "show" and "hide" effects sequentially, meaning that
  first the current view gets the "hide" effect applied and the new view
  only has its "show" effect started when the "hide" has finished.
 */

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchFullScreen

  The view switch effect is applied not just to the view's content but also other
  decorators, e.g. the titlebar, the toolbar, etc. The background is not included.

  This flag automatically implies ViewSwitchSequential.
*/

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchUseNormalAnim

  Uses the standard "hide" and "show" effect events.
*/

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchUseBackAnim

  Use the alternative "hide_back" and "show_back" effect events instead of the standard
  "hide" and "show".

  This flag automatically disables ViewSwitchUseNormalAnim if that one is also present.
*/

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchUseAltEvent

  Forces the usage of the "show_alt" and "hide_alt" (also "show_alt_back" and "hide_alt_back")
  events instead of "show" and "hide" (even when combined with other flags).

  This flag is typically used when the alternative default effect is wanted, which is typically
  a flip effect (although it is not guaranteed to be that for every theme).

  The alternative effects can be overridden using HbEffect::add() just like the normal ones.

  This flag automatically implies ViewSwitchSequential. (the default effect is parallel, the alternative is sequential)
*/

/*!
  \var Hb::ViewSwitchFlag Hb::ViewSwitchCachedFullScreen

  Similar to Hb::ViewSwitchFullScreen with the following differences: It will
  use a screenshot of the source view instead of the live content and it also
  supports parallel effects, i.e. this flag does not automatically imply
  Hb::ViewSwitchSequential.

  Use this flag only when switching between inconsistent views, i.e. when one
  view has the normal statusbar and titlebar while the other is fullscreen, or
  when one view has a toolbar while the other does not.

  By default effects between such views would not look very fluid due to the
  relayouting that needs to be done due to the difference in the decorator
  visibilities. This flag makes it possible to use the standard view switch
  effects also in such cases on the expense of a minor performance hit.
*/

/*!
    \enum Hb::TextCursorVisibility
    Cursor visibility rule for text cursor inside text editors.
    This works as a boolean; all values above Hb::TextCursorHidden (which is zero) show the cursor somehow.
*/

/*! \var Hb::TextCursorVisibility Hb::TextCursorHidden
    No cursor is shown.
*/

/*! \var Hb::TextCursorVisibility Hb::TextCursorVisible
    Text cursor is shown blinking.
*/
/*!
    \var Hb::NoTicks
    HbSlider's Tick position flag

    Do not draw any tick marks.
 */

/*!
    \var Hb::TicksBothSides
    HbSlider's Tick position flag

    Draw tick marks on both sides of the groove.
 */

/*!
    \var Hb::TicksAbove
    HbSlider's Tick position flag

    Draw tick marks above the (horizontal) slider.
 */

/*!
    \var Hb::TicksBelow
    HbSlider's Tick position flag

    Draw tick marks below the (horizontal) slider.
 */

/*!
    \var Hb::TicksLeft
    HbSlider's Tick position flag

    Draw tick marks to the left of the (vertical) slider.
 */

/*!
    \var Hb::TicksRight
    HbSlider's Tick position flag

    Draw tick marks to the right of the (vertical) slider.
 */

/*!
    \var Hb::TicksAbsolute
    HbSlider's Tick position flag

    If the widget's layout direction is Qt::RightToLeft (instead of
    Qt::LeftToRight, the default), HbSlider::TicksLeft refers to the right edge
    and  HbSlider::TicksRight to the left edge. This is normally the desired
    behavior. If you want HbSlider::TicksLeft to always mean "left" and
    HbSlider::TicksRight to always mean "right", combine the flag with
    HbSlider::TicksAbsolute.
 */

/*!
    \enum Hb::TextContextMenuFlag
    Flags which indicate how context menu is behave in editors
    \sa  HbAbstractEdit::contextMenuFlags
*/

/*!
    \var Hb::TextContextMenuFlag Hb::ShowTextContextMenuOnSelectionClicked
    constext menu is shown when selection is clicked
 */

/*!
    \var Hb::TextContextMenuFlag Hb::ShowTextContextMenuOnLongPress
    constext menu is shown on long press on editor
 */

/*!
    \enum Hb::InstantInteraction
    Instant interactions enumeration covers different touch interactions
    supported by Hb widgets. This information is
    relayed by Hb widgets whenever user interacts with interface
    and used to trigger various instant feedback effects.
*/

/*! \var Hb::InstantPressed
        Sent when a widget is pressed down.
   \image html interaction_pressed.png
*/

/*! \var Hb::InstantReleased
        Sent when the pressed down is released.
   \image html interaction_released.png
*/

/*! \var Hb::InstantClicked
        Sent when a widget is pressed and released over the widget area.
   \image html interaction_clicked.png
*/

/*! \var Hb::InstantKeyRepeated
        Sent when a widget sends key repeats when being pressed down.
   \image html interaction_clicked.png
*/

/*! \var Hb::InstantLongPressed
        Sent when a widget is kept pressed for a while.
   \image html interaction_longpressed.png
*/

/*! \var Hb::InstantDraggedOver
        Sent when the finger is dragged on top of a widget.
   \image html interaction_draggedover.png
*/

/*! \var Hb::InstantFlicked
        Sent when a current widget is flicked using a flick gesture.
*/

/*! \var Hb::InstantPopupOpened
        Sent when a popup like a note, tooltip or menu is opened.
*/

/*! \var Hb::InstantPopupClosed
        Sent when a popup like a note, tooltip or menu is closed.
*/

/*! \var Hb::InstantBoundaryReached
        Sent when the boundary of a scroll area is reached.
   \image html interaction_boundaryreached.png
*/

/*! \var Hb::InstantRotated90Degrees
        Sent every time a rotate gesture area passes 90 degrees of rotation.
   \image html interaction_rotated90degrees.png
*/

/*! \var Hb::InstantSelectionChanged
        Sent went a selectable widget (like a check box) changes its selection state.
*/

/*! \var Hb::InstantAdvancedGestureActivated
        Sent when a touch gesture involving more then one finger can be performed.
        For example a second touch point is detected and a pinch gesture can be performed.
*/



/*! \var Hb::InstantInstantUser
        User-defined interaction event type.
*/

/*! \var Hb::InstantInstantMaxUser
        Last user-defined interaction event type.
*/

/*!
    \enum Hb::ContinuousInteraction
    Continuous interactions enumeration covers different touch
    gestures supported by Hb widgets. This information is
    relayed by Hb widgets whenever user interacts with interface
    and used to trigger various continuous feedback effects.
*/

/*! \var Hb::ContinuousScrolled
        Sent when scroll area (list, grid, etc.) is being panned
        or put into motion by quickly swiping the finger.
   \image html gesture_panned.png
*/

/*! \var Hb::ContinuousDragged
        Sent when user is dragging a widget like moving slider handle.
   \image html gesture_slidermoved.png
*/

/*! \var Hb::ContinuousPinched
        Sent when an widget is being zoomed or rotated with two fingers.
   \image html gesture_pinched.png
*/

/*! \var Hb::ContinuousUser
        User-defined continuous interaction event type.
*/

/*! \var Hb::ContinuousMaxUser
        Last user-defined continuous interaction event type.
*/

/*!
    \enum Hb::InteractionModifier
    Interaction modifier enumeration is used to differentiate situations where
    same interaction trigger can cause multiple different feedback effects
    depending on the widget state.
*/

/*! \var Hb::ModifierSliderHandle
        User is touching a slider handle and not the slider itself to
        drag the slider handle across the value range. Needed because
        we cannot bass private handles trough the interaction system.
*/

/*! \var Hb::ModifierSliderElement
        User is touching a slider element and not the slider itself to
        increment/decrement volume, mute/unmute or zoom in/out slider value.
        Needed because we cannot bass private elements trough the interaction
        system.
*/

/*! \var Hb::ModifierItemExpanded
        User interaction causes the widget to expand
        it's presentation, like expanding a subtree in a
        treewidget.
*/

/*! \var Hb::ModifierItemCollapsed
        User interaction causes the widget to collapse
        it's presentation, like collapsing a subtree in a
        treewidget.
*/

/*! \var Hb::ModifierScrolling
        User touches an area that is already scrolling,
        normal feedback should not be given.
*/

/*!
    \enum Hb::ActivationReason
    ActivationReason enumeration indicates the method that was used to start
    application.
*/

/*!
    \var Hb::ActivationReason Hb::ActivationReasonActivity
    Application was started through Activity Framework.
 */

/*!
    \var Hb::ActivationReason Hb::ActivationReasonService
    Application was started as QtHighway service.
 */

/*!
    \var Hb::ActivationReason Hb::ActivationReasonNormal
    Application was started by other means.
 */

/*!
    \enum Hb::BackgroundImageMode

    Controls how the background image is drawn.
*/

/*!
    \var Hb::BackgroundImageMode Hb::ScaleBackgroundToFit

    The background image is scaled to cover the entire available area. The
    aspect ratio is kept (by expanding the size if needed). The image is
    centered in the available area.
 */

/*!
    \var Hb::BackgroundImageMode Hb::ScaleBackgroundToFitWithoutExpanding

    The background image is scaled to cover most of the available area. The
    aspect ratio is kept (without expanding) so the image may not occupy the
    entire available area. The image is centered in the available area.

    Note that if the background does not fill the entire screen then there must
    be a background brush set for the scene (or transparency needs to be
    enabled), otherwise drawing problems will occur (due to certain areas not
    being painted at all).
 */

/*!
    \var Hb::BackgroundImageMode Hb::StretchBackgroundToFit

    The background image is stretched to cover the entire available area.
 */

/*!
    \var Hb::BackgroundImageMode Hb::KeepOriginalBackgroundSize

    Keeps the original size of the image, no up or downscaling occurs. The image
    is centered in the available area.

    Note that if the background does not fill the entire screen then there must
    be a background brush set for the scene (or transparency needs to be
    enabled), otherwise drawing problems will occur (due to certain areas not
    being painted at all).
 */

/*!
    \var Hb::BackgroundImageMode Hb::KeepOriginalBackgroundSizeIfSmaller

    Keeps the original size of the image only when the size is smaller than the
    available area (i.e. dynamically switches between Hb::ScaleBackgroundToFit
    and Hb::KeepOriginalBackgroundSize depending on the size of the source
    image).
 */

/*!
    \var Hb::BackgroundImageMode Hb::DoNotDrawBackground

    Does not draw the background, i.e. the paint() function of the background
    item will not do anything when this mode is set. Use this only to
    temporarily prevent the background from showing. If you need to hide the
    background item permanently then pass Hb::WindowFlagNoBackground to the
    HbMainWindow constructor because that is more efficient.

    Note that if there is no background brush set and the application does not
    enable transparency properly then setting this flag will lead to drawing
    problems because the screen will not be updated properly (because nobobdy
    paints the background).
 */

