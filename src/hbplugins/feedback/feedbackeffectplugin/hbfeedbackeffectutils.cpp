/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#include "hbnamespace_p.h"
#include "hbfeedbackeffectutils.h"

#include <hbtoolbutton.h>
#include <hbabstractbutton.h>
#include <hbabstractedit.h>
#include <hbtextedit.h>
#include <hblineedit.h>
#include <hbabstractslidercontrol.h>
#include <hbprogressslider.h>
#include <hbscrollbar.h>
#include <hbabstractviewitem.h>
#include <hbabstractitemview.h>
#include <hbgridview.h>
#include <hbgridviewitem.h>
#include <hblistview.h>
#include <hbtreeview.h>
#include <hbtreeviewitem.h>
#include <hbdataformviewitem.h>
#include <hbpopup.h>
#include <hbmenu.h>
#include <hbview.h>
#include <hbmainwindow.h>
#include <QGraphicsScene>
#include <QPointF>
#include <QRectF>

const int continuousSliderStepLimit = 15;

#define IF_INSTANCEOF_SET_FAMILY(className, widgetFamily) \
  if (qobject_cast<const className *>(widget))  \
  {                                             \
      family = HbFeedbackEffectUtils::widgetFamily;        \
  }

/*!
    Finds out the graphics item type of the parent widget.
*/
int HbFeedbackEffectUtils::parentItemType(const HbWidget *widget)
{
    int graphicsItemType = 0;
    if (const HbWidget *parent = dynamic_cast<const HbWidget *>(widget->parentItem())) {
        graphicsItemType = parent->type();
    }
    return graphicsItemType;
}

/*!
    Returns the widget family for a widget.
*/
HbFeedbackEffectUtils::WidgetFamily HbFeedbackEffectUtils::widgetFamily(const HbWidget *widget)
{
    HbFeedbackEffectUtils::WidgetFamily family = Undefined;
    switch(widget->type()) {

        case Hb::ItemType_AbstractButton:

        case Hb::ItemType_PushButton:

        case Hb::ItemType_ToolButton:

        case Hb::ItemType_CheckBox:

        case HbPrivate::ItemType_SliderElement:

        case HbPrivate::ItemType_IndicatorGroup:

        case HbPrivate::ItemType_TitlePane:

        case Hb::ItemType_InputCharacterButton:

        case Hb::ItemType_InputFunctionButton:

        case Hb::ItemType_ComboBox:
   
            family = HbFeedbackEffectUtils::Button;
            break;

        case Hb::ItemType_MenuItem:

        case Hb::ItemType_FormViewItem:
        
        case Hb::ItemType_FormView:
        
        case Hb::ItemType_AbstractItemContainer:
        
        case Hb::ItemType_AbstractItemView:

        case Hb::ItemType_ListView:

        case Hb::ItemType_AbstractViewItem:
        
        case Hb::ItemType_ListViewItem:

        case Hb::ItemType_ItemHighlight:
        
        case Hb::ItemType_ListWidgetItem:
        
        case Hb::ItemType_ListWidgetViewItem:

        case Hb::ItemType_RadioButtonList:

        case Hb::ItemType_RoundRobinLabel:

        case Hb::ItemType_CarouselView:
        
        case HbPrivate::ItemType_MenuListView:

        case HbPrivate::ItemType_GroupBoxHeadingWidget:

        case HbPrivate::ItemType_GroupBoxContentWidget:

        case HbPrivate::ItemType_DataGroupHeadingWidget:

            family = HbFeedbackEffectUtils::List;
            IF_INSTANCEOF_SET_FAMILY(HbGridView, Grid)
            break;

        case Hb::ItemType_GridViewItem:
            family = HbFeedbackEffectUtils::Grid;
            break;
 
        case Hb::ItemType_AbstractSlider:

        case Hb::ItemType_Slider:

        case HbPrivate::ItemType_SliderControl:

        case HbPrivate::ItemType_SliderHandle:
        
        case HbPrivate::ItemType_SliderTickmarks:

        case HbPrivate::ItemType_ZoomSlider:

        case HbPrivate::ItemType_VolumeSlider:

        case Hb::ItemType_ProgressBar:

        case Hb::ItemType_ProgressSlider:

        case HbPrivate::ItemType_ProgressSliderHandle:
 
        case Hb::ItemType_ScrollBar:

            family = HbFeedbackEffectUtils::Slider;
            break;

        case Hb::ItemType_Menu:

        case HbPrivate::ItemType_IndicatorMenu:

        case Hb::ItemType_Popup:

        case Hb::ItemType_ToolBarExtension:

        case HbPrivate::ItemType_ToolTipLabel:

        case Hb::ItemType_InputVkbWidget:

        case Hb::ItemType_ColorDialog:

            family = HbFeedbackEffectUtils::Popup;
            break;

        case Hb::ItemType_LineEdit:

        case Hb::ItemType_TextEdit:

        case Hb::ItemType_AbstractEdit:

        case Hb::ItemType_VirtualTrackPoint:

        case Hb::ItemType_WritingBox:

            family = HbFeedbackEffectUtils::Editor;
            break;

        default:
            break;
    }

    // fallback if item type cannot be found
    if (family == HbFeedbackEffectUtils::Undefined) {
        IF_INSTANCEOF_SET_FAMILY(HbAbstractButton, Button)
        IF_INSTANCEOF_SET_FAMILY(HbAbstractEdit, Editor)
        IF_INSTANCEOF_SET_FAMILY(HbAbstractSliderControl, Slider)
        IF_INSTANCEOF_SET_FAMILY(HbPopup, Popup)
        IF_INSTANCEOF_SET_FAMILY(HbAbstractItemView, List)
        IF_INSTANCEOF_SET_FAMILY(HbAbstractViewItem, List)
        IF_INSTANCEOF_SET_FAMILY(HbGridView, Grid)
        IF_INSTANCEOF_SET_FAMILY(HbGridViewItem, Grid)
    }
    return family;
}

/*!
    Returns the instant feedback effect on press interaction.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnPress(const HbWidget *widget, Hb::InteractionModifiers modifiers)
{
    HbFeedback::InstantEffect effect = HbFeedback::Sensitive;

    switch (widgetFamily(widget)) {

        case HbFeedbackEffectUtils::Undefined:
            effect = HbFeedback::None;
            break;

        case HbFeedbackEffectUtils::Button:
            effect = HbFeedback::SensitiveButton;

            // latched down button special case (isChecked information required)
            if (const HbAbstractButton* button = qobject_cast<const HbAbstractButton *>(widget)) {
                if (button->type() == Hb::ItemType_PushButton) {
                    // toggleable push button
                    if (button->isCheckable()) {
                        if (button->isChecked()) {
                            effect = HbFeedback::SensitiveButton;
                        } else {
                            effect = HbFeedback::BasicButton;
                        }
                    // normal push button
                    } else {
                        effect = HbFeedback::BasicButton;
                    }
                }
            }

            if (widget->type() == HbPrivate::ItemType_NavigationButton
                // Commented out until use cases are clarified
                /*|| widget->type() == HbPrivate::ItemType_IndicatorButton*/) {
                effect = HbFeedback::BasicButton;
            }
            
            // input widget special case
            if (widget->type() == Hb::ItemType_InputCharacterButton) {
                effect = HbFeedback::SensitiveKeypad;
            } 
            else if (widget->type() == Hb::ItemType_InputFunctionButton) {
                effect = HbFeedback::BasicKeypad;
            } 
            else if (widget->type() == Hb::ItemType_CheckBox) {
                effect = HbFeedback::None; // Checkbox deferred to release
            }

            // title pane specific special case
            if (widget->type() == HbPrivate::ItemType_TitlePane) {
                if (isOptionsMenuEmpty(widget)) {
                    effect = HbFeedback::None;
                }
            }
            break;

        case HbFeedbackEffectUtils::List:
            if (modifiers & (Hb::ModifierExpandedItem | Hb::ModifierCollapsedItem)) {
                effect = HbFeedback::BasicItem;
            }
            else {
                effect = HbFeedback::SensitiveItem;
            }
            break;
        
        case HbFeedbackEffectUtils::Grid:
            effect = HbFeedback::SensitiveItem;
            break;

        case HbFeedbackEffectUtils::Slider:

            // slider area
            effect = HbFeedback::None;
            if (const HbProgressSlider *progressSlider = qobject_cast<const HbProgressSlider *>(widget)) {
                Q_UNUSED(progressSlider)
                effect = HbFeedback::BasicSlider;
            } else if (const HbScrollBar *scrollBar = qobject_cast<const HbScrollBar *>(widget)) {
                Q_UNUSED(scrollBar)
                effect = HbFeedback::SensitiveSlider;
            }

            // slider handle
            if (modifiers & Hb::ModifierSliderHandle) {
                effect = HbFeedback::SensitiveSlider;
            }

            // slider elements
            if (modifiers & Hb::ModifierSliderElement) {
                effect = HbFeedback::SensitiveButton;
            }
            break;

        case HbFeedbackEffectUtils::Popup:

            if (widget->type() == Hb::ItemType_InputVkbWidget) {
                effect = HbFeedback::SensitiveButton;
            }
            else {
                effect = HbFeedback::None;
            }
            break;

        case HbFeedbackEffectUtils::Editor:
            effect = HbFeedback::Editor;
            break;

        default:
            break;
    }

    if (widget->type() == Hb::ItemType_MenuItem) {
        if (modifiers & Hb::ModifierScrolling) {
            effect = HbFeedback::StopFlick;
        }
    }
    // item view specific special cases
    if ( const HbAbstractViewItem * viewItem = qobject_cast<const HbAbstractViewItem *>(widget)) {
        const HbAbstractItemView* itemView = viewItem->itemView();
        if (itemView) {
            // checkable item is checked with a press
            switch (itemView->selectionMode()) {
                case HbAbstractItemView::SingleSelection:
                case HbAbstractItemView::MultiSelection:
                case HbAbstractItemView::ContiguousSelection: {
                    effect = HbFeedback::None;
                    break;
                }
                case HbAbstractItemView::NoSelection:
                    if (const HbListView * listView = qobject_cast<const HbListView *>(itemView)) {
                        if (listView->arrangeMode()
                            && !(modifiers & Hb::ModifierScrolling)) {
                            effect = HbFeedback::ItemPick;
                        }
                    }
                    break;
                default:
                    break;
            }

            // radio button list works like a normal list item
            if (viewItem->type() == Hb::ItemType_RadioButtonListViewItem) {
                effect = HbFeedback::SensitiveItem;
            }
            else if(viewItem->type() == Hb::ItemType_TumbleViewItem ) {
                effect = HbFeedback::SensitiveItem;
            }

            // expandable or collapsable items give a BasicItem feedback
            if (const HbTreeViewItem* treeItem = qobject_cast<const HbTreeViewItem *>(widget)) {
                if (itemView->model()->rowCount(treeItem->modelIndex()) > 0) {
                    effect = HbFeedback::BasicItem;
                }
                else {
                    effect = HbFeedback::SensitiveItem;
                }
            }

            if (modifiers & Hb::ModifierScrolling) {
                effect = HbFeedback::StopFlick;
            }
        }
    }
    if (widget->type() == Hb::ItemType_VirtualTrackPoint) {
        effect = HbFeedback::Editor;
    }

    return effect;
}

/*!
    Returns the instant feedback effect on release interaction.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnRelease(const HbWidget *widget, Hb::InteractionModifiers modifiers)
{
    // mostly the same as when the widget is pressed
    HbFeedback::InstantEffect effect = instantOnPress(widget, modifiers);

    // clicking on a sub menu item, indicator group or title pane causes release feedback
    // even if all of those widgets open a new menu popup, which initiates IncreasingPopUp
    switch (widgetFamily(widget)) {

        case HbFeedbackEffectUtils::Undefined:
            effect = HbFeedback::None;
            break;

        case HbFeedbackEffectUtils::Button:
            effect = HbFeedback::SensitiveButton;

            // latched down button special case (isChecked information required)
            if (const HbAbstractButton* button = qobject_cast<const HbAbstractButton *>(widget)) {
                if (button->type() == Hb::ItemType_PushButton) {
                    // toggleable push button
                    if (button->isCheckable()) {
                        if (button->isChecked()) {
                            effect = HbFeedback::BasicButton;
                        } else {
                            effect = HbFeedback::None;
                        }
                    // normal push button
                    } else {
                        effect = HbFeedback::BasicButton;
                    }
                }
            }

            if (widget->type() == HbPrivate::ItemType_NavigationButton
                // Commented out until use cases are clarified
                /*|| widget->type() == HbPrivate::ItemType_IndicatorButton*/) {
                effect = HbFeedback::BasicButton;
            }

            // input widget special case
            if (widget->type() == Hb::ItemType_InputCharacterButton
             || widget->type() == Hb::ItemType_InputFunctionButton) {
                effect = HbFeedback::SensitiveKeypad;
            } else if (widget->type() == Hb::ItemType_CheckBox) {
                effect = HbFeedback::Checkbox; // deferred from press
            }

            // title pane specific special case
            if (widget->type() == HbPrivate::ItemType_TitlePane) {
                if (isOptionsMenuEmpty(widget)) {
                    effect = HbFeedback::None;
                }
            }

            if (widget->type() == Hb::ItemType_ComboBox) {
                effect = HbFeedback::PopupOpen;
            }

            break;

        case HbFeedbackEffectUtils::List:
            if (modifiers & (Hb::ModifierExpandedItem | Hb::ModifierCollapsedItem)) {
                effect = HbFeedback::BasicItem;
            }
            else {
                effect = HbFeedback::SensitiveItem;
            }
            // menu items give popop closed feedback on release
            if (widget->type() == Hb::ItemType_MenuItem) {
                effect = HbFeedback::None;
            }
            break;

        case HbFeedbackEffectUtils::Grid:
            effect = HbFeedback::SensitiveItem;
            break;
 
         case HbFeedbackEffectUtils::Slider:

            // slider area
             effect = HbFeedback::None;

            // slider handle
            if (modifiers & Hb::ModifierSliderHandle) {
                effect = HbFeedback::SensitiveSlider;
            }

            // slider elements
            if (modifiers & Hb::ModifierSliderElement) {
                effect = HbFeedback::SensitiveButton;
            }
            break;

        case HbFeedbackEffectUtils::Editor:
            effect = HbFeedback::Editor;
            break;

        default:
            break;
    }

    // item view specific special cases
    if ( const HbAbstractViewItem * viewItem = qobject_cast<const HbAbstractViewItem *>(widget)) {
        const HbAbstractItemView* itemView = viewItem->itemView();
        if (itemView) {
            switch (itemView->selectionMode()) {
                case HbAbstractItemView::SingleSelection:
                    effect = HbFeedback::Checkbox; // deferred from press
                    break;
                case HbAbstractItemView::MultiSelection: 
                case HbAbstractItemView::ContiguousSelection: {
                    effect = HbFeedback::None;
                    break;
                }

                case HbAbstractItemView::NoSelection: 
                    if (const HbListView * listView = qobject_cast<const HbListView *>(itemView)) {
                        if (listView->arrangeMode()) {
                            if(modifiers & Hb::ModifierScrolling) {
                                effect = HbFeedback::None;
                            }
                            else {
                                effect = HbFeedback::ItemDrop;
                            }
                        }
                    }
                default: {

                    break;
                }
            }

            // radio button list works like a normal list item
            if (viewItem->type() == Hb::ItemType_RadioButtonListViewItem) {
                effect = HbFeedback::SensitiveItem;
            }
            else if(viewItem->type() == Hb::ItemType_TumbleViewItem ) {
                effect = HbFeedback::SensitiveItem;
            }

            if (modifiers & Hb::ModifierExpandedItem || modifiers & Hb::ModifierCollapsedItem) {
                effect = HbFeedback::BasicItem;
            }
        }
    }
    return effect;
}

/*!
    Returns the instant feedback effect on key repeat interaction.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnKeyRepeat(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::Sensitive;

    // rule of thumb: key repeats cause sensitive version of the feedback effect
    switch (widgetFamily(widget)) {

        case HbFeedbackEffectUtils::Undefined:
            effect = HbFeedback::None;
            break;

        case HbFeedbackEffectUtils::Button:
            effect = HbFeedback::SensitiveButton;

            // input widget special case
            if (widget->type() == Hb::ItemType_InputCharacterButton
             || widget->type() == Hb::ItemType_InputFunctionButton) {
                effect = HbFeedback::SensitiveKeypad;
            }
            break;

        case HbFeedbackEffectUtils::List:
            effect = HbFeedback::SensitiveItem;
            break;
        
        case HbFeedbackEffectUtils::Grid:
            effect = HbFeedback::SensitiveItem;
            break;

        case HbFeedbackEffectUtils::Slider:
            effect = HbFeedback::SensitiveSlider;
            break;

        case HbFeedbackEffectUtils::Tab:

        case HbFeedbackEffectUtils::Popup:

        case HbFeedbackEffectUtils::Editor:
            effect = HbFeedback::None;
            break;

        default:
            break;
    }


    return effect;
}

/*!
    Returns the instant feedback effect on drag interaction.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnDrag(const HbWidget *widget, Hb::InteractionModifiers modifiers)
{
    // mostly the same as when the widget is pressed
    HbFeedback::InstantEffect effect = instantOnPress(widget, modifiers);

    if (const HbAbstractViewItem * viewItem = qobject_cast<const HbAbstractViewItem *>(widget)) {
        const HbAbstractItemView* itemView = viewItem->itemView();
        if (itemView) {
            if (itemView->selectionMode() == HbAbstractItemView::ContiguousSelection) {
                effect = HbFeedback::MultipleCheckbox;
            }
            else if (const HbListView * listView = qobject_cast<const HbListView *>(itemView)) {
                if (listView->arrangeMode()) {
                    effect = HbFeedback::ItemMoveOver;
                }
            }
        }
    }
    if (widget->type() == Hb::ItemType_VirtualTrackPoint) {
        effect = HbFeedback::Editor;
    }
    if (widget->type() == Hb::ItemType_Menu) {
        effect = HbFeedback::ItemScroll;
    }
    return effect;
}

/*!
    Returns the instant feedback effect on editor highlighting.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnEditorHighlight(const HbWidget *widget, int previousCursorPosition)
{
    HbFeedback::InstantEffect effect = HbFeedback::None;
    int dist = 0;
    bool emptyline = false;
    
    if (const HbTextEdit* textEdit = qobject_cast<const HbTextEdit*>(widget)) {

        int index = textEdit->cursorPosition();
        QTextDocument* text = textEdit->document();

        if (text->characterCount() > 0 && index < text->characterCount()) {
            dist = abs(index - previousCursorPosition);

            if (previousCursorPosition < index) {
                index--;
            }
            emptyline = text->characterAt(index).category() == QChar::Separator_Paragraph;
            QChar character = text->characterAt(index);

            if (emptyline) {
                effect = HbFeedback::EmptyLineSelection;
            }
            else if (dist > 1) {
                effect = HbFeedback::LineSelection;
            }
            else if (character.isSpace()) {
                effect = HbFeedback::BlankSelection;
            }
            else {
                effect = HbFeedback::TextSelection;
            }
        }
    }
    else if (const HbLineEdit* lineEdit = qobject_cast<const HbLineEdit*>(widget)) {

        int index = lineEdit->cursorPosition();
        QString text = lineEdit->text();

        if (!text.isEmpty() && index < text.count()) {
            if (previousCursorPosition < index) {
                index--;
            }
            QChar character = text.at(index);

            if (character.isSpace()) {
                effect = HbFeedback::BlankSelection;
            }
            else {
                effect = HbFeedback::TextSelection;
            }
        }
    }
    return effect;
}

/*!
    Checks if feedback is allowed for certain types of popup widgets.
*/
bool HbFeedbackEffectUtils::isFeedbackAllowedForPopup(const HbWidget *widget)
{
    bool feedbackAllowed(false);
    if (widgetFamily(widget) == HbFeedbackEffectUtils::Popup) {
        feedbackAllowed = true;
        if (widget->type() == HbPrivate::ItemType_ToolTipLabel
         || widget->type() == Hb::ItemType_InputCharPreviewPane
         || widget->type() == Hb::ItemType_InputVkbWidget) {
            feedbackAllowed = false;
        }
        else if (QString(widget->metaObject()->className()) == "HbSelectionControl") {
            feedbackAllowed = false;
        }
    }
    else if (QString(widget->metaObject()->className()) == "HbComboDropDown") {
        feedbackAllowed = true;
    }
    return feedbackAllowed;
}

/*!
    Returns the instant feedback effect on key press interaction.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnKeyPress(const HbWidget *widget, Hb::InteractionModifiers modifiers)
{
    // mostly the same as when the widget is pressed
    return instantOnPress(widget, modifiers);
}

/*!
    Returns the instant feedback effect on selection changed events.
*/
HbFeedback::InstantEffect HbFeedbackEffectUtils::instantOnSelectionChanged(const HbWidget *widget, Hb::InteractionModifiers modifiers)
{
    HbFeedback::InstantEffect effect = HbFeedback::None;

    if ( const HbAbstractViewItem * viewItem = qobject_cast<const HbAbstractViewItem *>(widget)) {
        const HbAbstractItemView* itemView = viewItem->itemView();
        if (itemView) {
            switch (itemView->selectionMode()) {
                case HbAbstractItemView::SingleSelection: {
                    // Single selection is handled with a release
                    effect = HbFeedback::None;
                    break;
                }
                case HbAbstractItemView::MultiSelection: {
                    effect = HbFeedback::Checkbox;
                    break;
                }
                case HbAbstractItemView::ContiguousSelection: {
                    effect = HbFeedback::MultipleCheckbox;
                    break;
                }
                default:
                    break;
            }
            if (modifiers == Hb::ModifierScrolling) {
                effect = HbFeedback::StopFlick;
            }
		}
	}

        return effect;
}

/*!
    Returns the instant continuous feedback effect for a continuous interaction.
*/
HbFeedback::ContinuousEffect HbFeedbackEffectUtils::continuousEffect(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    HbFeedback::ContinuousEffect effect = HbFeedback::ContinuousSmooth;

    switch (widgetFamily(widget)) {

        case HbFeedbackEffectUtils::Undefined:
            effect = HbFeedback::ContinuousNone;
            break;

        case HbFeedbackEffectUtils::Slider:
            effect = HbFeedback::ContinuousSlider;
            break;

        default:
            break;

    }

    if (interaction == Hb::ContinuousPinched) {
        effect = HbFeedback::ContinuousPinch;
    }
    else if (interaction == Hb::ContinuousRotated) {
        effect = HbFeedback::ContinuousSmooth;
    }

    return effect;
}

/*!
    Returns the intensity of the feedback for a continuous interaction.
*/
int HbFeedbackEffectUtils::intensity(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    Q_UNUSED(interaction);
    Q_UNUSED(delta);

    int intensity = HbFeedback::IntensityFull;

    if (const HbAbstractSliderControl *slider = qobject_cast<const HbAbstractSliderControl *>(widget)) {
        switch(parentItemType(slider)) {
            case HbPrivate::ItemType_ZoomSlider:

            case HbPrivate::ItemType_VolumeSlider:
                {
                qreal min = slider->minimum();
                qreal max = slider->maximum();
                qreal pos = slider->sliderPosition();
                qreal sliderRange = max - min;
                qreal relativePos = pos - min;
                intensity = int((qreal)(HbFeedback::IntensityFull) * (qreal)(relativePos) / (qreal)(sliderRange));
                break;
                }
            default:
                break;
       }
    }
    else if (const HbScrollBar *scrollbar = qobject_cast<const HbScrollBar *>(widget)) {
        Q_UNUSED(scrollbar);
        intensity = HbFeedback::IntensitySmooth;
    }
    else {
        // The default intensity for continuous effects
        intensity = HbFeedback::IntensityFull;
    }

    return intensity;
}

/*!
    Checks if the feedback is allowed.
*/
bool HbFeedbackEffectUtils::isFeedbackAllowed(const HbWidget* widget)
{
    // either widget is already visible or a popup about to be visible
    bool widgetVisible = widget && widget->isEnabled() && widget->isVisible() && widget->scene();
    bool popupAboutToBeVisible = widget && widget->isEnabled() && qobject_cast<const HbPopup*>(widget);
    return widgetVisible || popupAboutToBeVisible;
}

/*!
    Decides if the feedback regarding sliders should be continuous or instant.
*/
bool HbFeedbackEffectUtils::isSliderMoveContinuous(const HbWidget *widget)
{
    bool continuous(true);
    if (const HbAbstractSliderControl *slider = qobject_cast<const HbAbstractSliderControl *>(widget)) {
        if (!slider->isSliderDown()) {
            int range = slider->maximum() - slider->minimum();
            int numberOfSteps = range / slider->singleStep();
            continuous = numberOfSteps > continuousSliderStepLimit;
        }
    }
    else if (const HbProgressSlider *progressSlider = qobject_cast<const HbProgressSlider *>(widget)) {
        Q_UNUSED(progressSlider);
        continuous = true;
    }
    return continuous;
}

/*!
    Returns the status of the options menu.
*/
bool HbFeedbackEffectUtils::isOptionsMenuEmpty(const HbWidget *widget)
{
    bool menuEmpty(false);
    HbMainWindow* window = widget->mainWindow();
    if (window) {
        HbView* view = window->currentView();
        if (view) {
            HbMenu *menu = view->menu();
            if (menu && menu->isEmpty()) {
                menuEmpty = true;
            }
        }
    }
    return menuEmpty;
}

/*!
  Returns the default modalities to be used when playing feedback effect asociated with this \a widget, \a interaction, \a modifiers.
*/
HbFeedback::Modalities HbFeedbackEffectUtils::modalities(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers )
{
    Q_UNUSED(modifiers)

    HbFeedback::Modalities modalities = 0;

    switch( interaction ) {
    case Hb::InstantPressed :
    case Hb::InstantMultitouchActivated:
        modalities |= HbFeedback::Tactile;
        modalities |= HbFeedback::Audio;
        break;
    case Hb::InstantReleased:
    case Hb::InstantLongPressed:
    case Hb::InstantKeyRepeated:
    case Hb::InstantDraggedOver:
    case Hb::InstantFlicked:
    case Hb::InstantRotated90Degrees:
    case Hb::InstantPopupOpened:
    case Hb::InstantPopupClosed:
    case Hb::InstantSelectionChanged:
        modalities =  HbFeedback::Tactile;
        break;
    case Hb::InstantClicked:
        modalities = HbFeedback::Tactile;
        if(widget->type() == Hb::ItemType_CheckBox) {
            modalities |= HbFeedback::Audio;
        }
        break;
    default:
        modalities = HbFeedback::Tactile;
        break;
    }
    return modalities;
}

/*!
  Returns the default modalities to be used when playing feedback effect asociated with this \a widget, \a interaction, \a modifiers.
*/
HbFeedback::Modalities HbFeedbackEffectUtils::modalities(const HbWidget * widget, Hb::ContinuousInteraction interaction, Hb::InteractionModifiers modifiers )
{
    Q_UNUSED(widget)
    Q_UNUSED(interaction)
    Q_UNUSED(modifiers)
    return HbFeedback::Tactile;
}
