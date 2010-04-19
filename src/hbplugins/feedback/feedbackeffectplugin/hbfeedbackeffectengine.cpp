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

#include "hbfeedbackeffectengine.h"
#include "hbfeedbackeffectutils.h"

#include <hbfeedbackplayer.h>
#include <hbinstantfeedback.h>
#include <hbcontinuousfeedback.h>

#include <hbwidget.h>
#include <hbabstractitemview.h>
#include <hbabstractviewitem.h>
#include <hblistviewitem.h>
#include <hblistview.h>
#include <hbgridview.h>
#include <hbmainwindow.h>
#include <hbabstractedit.h>
#include "hbmenucontainer_p.h"
#include "hbinputvirtualrocker.h"
#include <hbnamespace_p.h>

/*!
    Constructor
*/
HbFeedbackEffectEngine::HbFeedbackEffectEngine() : previousCursorPosition(0)
{
    // initialize
    HbFeedbackPlayer::instance();
}

/*!
    Destructor
*/
HbFeedbackEffectEngine::~HbFeedbackEffectEngine()
{
    if (hbFeedbackPlayer) {
        foreach(int identifier, continuousFeedbacks) {
            hbFeedbackPlayer->cancelContinuousFeedback(identifier);
        }
    }

    continuousFeedbacks.clear();
    boundaryWidgets.clear();
}

/*!
    Called by the feedback manager when an interaction is triggered.
*/
void HbFeedbackEffectEngine::triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers)
{
    HbFeedback::InstantEffect widgetOverride = widget->overrideFeedback(interaction);
    if (widgetOverride != HbFeedback::NoOverride ) {
        playInstantFeedback(widget, widgetOverride);
    } else {
        HbFeedbackEngine::triggered(widget, interaction, modifiers);
    }
}

/*!
    Called by the feedback manager when a widget is pressed.
*/
void HbFeedbackEffectEngine::pressed(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedbackEffectUtils::instantOnPress(widget, modifiers());
    playInstantFeedback(widget, effect);
}

/*!
    Called by the feedback manager when a widget is released.
*/
void HbFeedbackEffectEngine::released(const HbWidget *widget)
{
    if (continuousFeedbacks.contains(widget)) {
        cancelContinuousFeedback(widget);
    }
    // slider-like widgets are a bit special
    if (HbFeedbackEffectUtils::widgetFamily(widget) == HbFeedbackEffectUtils::Slider) {
        playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnRelease(widget, modifiers()));
    }
    // lists in arrange mode react on release
    else if (const HbListViewItem *listViewItem = qobject_cast<const HbListViewItem *>(widget)) {
        const HbAbstractItemView* itemView = listViewItem->itemView();
        if (const HbListView * listView = qobject_cast<const HbListView *>(itemView)) {
            if( listView->arrangeMode() ) {
                playInstantFeedback(listViewItem,HbFeedbackEffectUtils::instantOnRelease(listViewItem, modifiers()));
            }
        }
    } else if (widget->type() == Hb::ItemType_VirtualTrackPoint) {
        playInstantFeedback(widget, HbFeedback::Editor);
    } else if (widget->type() == Hb::ItemType_WritingBox) {
        playInstantFeedback(widget, HbFeedback::Editor);
    }
    else if (widget->type() == HbPrivate::ItemType_GroupBoxHeadingWidget || widget->type() == Hb::ItemType_ComboBox) {
        playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnRelease(widget, modifiers()));
    }

    // normally touch end feedback effect is initiated with a clicked signal
}

/*!
    Called by the feedback manager when a long press is detected for a widget.
*/
void HbFeedbackEffectEngine::longPressed(const HbWidget *widget)
{
    if (widget->type() == Hb::ItemType_InputCharacterButton) {
        playInstantFeedback(widget, HbFeedback::SensitiveKeypad);
    }
}

/*!
    Called by the feedback manager when a widget is clicked.
*/
void HbFeedbackEffectEngine::clicked(const HbWidget *widget)
{
    if (continuousFeedbacks.contains(widget)) {
        cancelContinuousFeedback(widget);
    }
    playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnRelease(widget, modifiers()));
}

/*!
    Called by the feedback manager when keyrepeats are detected for a widget.
*/
void HbFeedbackEffectEngine::keyRepeated(const HbWidget *widget)
{
    playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnKeyRepeat(widget));
}

/*!
    Called by the feedback manager when a dragged over event is detected for a widget.
*/
void HbFeedbackEffectEngine::draggedOver(const HbWidget *widget)
{
    // For editor highlighting
    if (const HbAbstractEdit *edit = qobject_cast<const HbAbstractEdit *>(widget)) {
        if (edit->cursorPosition() != previousCursorPosition) {
            playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnEditorHighlight(edit, previousCursorPosition));
            previousCursorPosition = edit->cursorPosition();
        }
    }
    else if (const HbInputVirtualRocker *trackPoint = qobject_cast<const HbInputVirtualRocker *>(widget)) {
        if (trackPoint && trackPoint->mainWindow() && trackPoint->mainWindow()->scene() &&
            trackPoint->mainWindow()->scene()->focusItem()) {
            
            QGraphicsItem* graphicsItem = trackPoint->mainWindow()->scene()->focusItem();
            
            if (graphicsItem->isWidget() && 
                static_cast<QGraphicsWidget*>(graphicsItem)->inherits("HbAbstractEdit")) {
             
                if (HbAbstractEdit* edit = static_cast<HbAbstractEdit*>(graphicsItem)) {
                    if (edit->cursorPosition() != previousCursorPosition) {
                        playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnEditorHighlight(edit, previousCursorPosition));
                        previousCursorPosition = edit->cursorPosition();
                    }
                }
            }
        }
    }
    else {
        playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnDrag(widget, modifiers()));
    }
}

/*!
    Called by the feedback manager when a widget is flicked.
*/
void HbFeedbackEffectEngine::flicked(const HbWidget *widget)
{
    playInstantFeedback(widget, HbFeedback::Flick);
}

/*!
    Called by the feedback manager when a scroll area widget is flicked or dragged and the boundary reached.
*/
void HbFeedbackEffectEngine::boundaryReached(const HbWidget *widget)
{
    boundaryWidgets.append(widget);
}

/*!
    Called by the feedback manager when a rotate gesture is recognized for a widget.
*/
void HbFeedbackEffectEngine::rotated90Degrees(const HbWidget *widget)
{
    playInstantFeedback(widget, HbFeedback::RotateStep);
}

/*!
    Called by the feedback manager when a popup opens.
*/
void HbFeedbackEffectEngine::popupOpened(const HbWidget *widget)
{
    if (HbFeedbackEffectUtils::isFeedbackAllowedForPopup(widget)) {
        playInstantFeedback(widget, HbFeedback::PopupOpen);
    }
}

/*!
    Called by the feedback manager when a popup closes.
*/
void HbFeedbackEffectEngine::popupClosed(const HbWidget *widget)
{
    if (HbFeedbackEffectUtils::isFeedbackAllowedForPopup(widget)) {
        playInstantFeedback(widget, HbFeedback::PopupClose);
    }
}

/*!
    Called by the feedback manager when an item view selection has changed.
*/
void HbFeedbackEffectEngine::selectionChanged(const HbWidget *widget)
{
    playInstantFeedback(widget, HbFeedbackEffectUtils::instantOnSelectionChanged(widget));
}

/*!
    Called by the feedback manager when multitouch is activated.
*/
void HbFeedbackEffectEngine::multitouchActivated(const HbWidget *widget)
{
    playInstantFeedback(widget, HbFeedback::MultitouchActivate);
}

/*!
    Called by the feedback manager when a continuous interaction is triggered.
*/
void HbFeedbackEffectEngine::continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    bool feedbackPlayed(false);
    switch(HbFeedbackEffectUtils::widgetFamily(widget))
    {
        case HbFeedbackEffectUtils::Slider:
        {
            if (!HbFeedbackEffectUtils::isSliderMoveContinuous(widget)) {
                playInstantFeedback(widget, HbFeedback::SensitiveSlider);
                feedbackPlayed = true;
            }
            break;
        }
        case HbFeedbackEffectUtils::List:
        case HbFeedbackEffectUtils::Grid:
        {
            if (interaction == Hb::ContinuousScrolled) {
                 if (const HbAbstractItemView * itemView = qobject_cast<const HbAbstractItemView *>(widget)) {
                     feedbackPlayed = true;
                     QList<HbAbstractViewItem *> visibleItems = itemView->visibleItems();
                     bool newItemFound(false);
                     int index(-1);
                     QList<int> visibleIndexes;
                     if (widget == activelyScrollingItemView) {
                         foreach (HbAbstractViewItem * item, visibleItems) {
                             index = item->modelIndex().row();
                             if (!oldVisibleIndexes.contains(index)) {
                                 newItemFound = true;
                             }
                             visibleIndexes.append(index);
                         }
                     }
                     if (widget != activelyScrollingItemView){
                         activelyScrollingItemView = widget;
                         newItemFound = false;
                     }
                     oldVisibleIndexes.clear();
                     oldVisibleIndexes = visibleIndexes;

                    if (newItemFound) {
                        const HbListView* listView = qobject_cast<const HbListView*>(widget);
                        if (!(  listView &&
                                listView->arrangeMode() &&
                                listView->draggedItem())){
                            playInstantFeedback(widget, HbFeedback::ItemScroll);
                        }
                    }
                 }
             }
            break;
        }
        default:
        {
            break;
        }
    }

    if (interaction == Hb::ContinuousScrolled) {
        // menu widget does scroll feedback elsewhere
        if (widget->type() == HbPrivate::ItemType_MenuListView) {
            feedbackPlayed = true;
        }

        // generic scroll areas don't emit continuous feedback
        if (const HbScrollArea* scrollArea = qobject_cast<const HbScrollArea *>(widget)) {
            Q_UNUSED(scrollArea)
            feedbackPlayed = true;
        }
    }

    if (!feedbackPlayed) {
        int intensity = HbFeedbackEffectUtils::intensity(widget, interaction, delta);

        // Check if the widget has overriden feedback for this interaction
        HbFeedback::ContinuousEffect widgetOverride = widget->overrideContinuousFeedback(interaction, &intensity) ;
        if (widgetOverride != HbFeedback::NoContinuousOverride) {
            playContinuousFeedback(widget, widgetOverride, intensity);
        } else {
            playContinuousFeedback(widget, HbFeedbackEffectUtils::continuousEffect(widget, interaction), intensity);

        }
    }
}

/*!
    Called by the feedback manager when a continuous interaction is stopped.
*/
void HbFeedbackEffectEngine::continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    // determine if instant feedback should be played when a continuous interaction is stopped
    HbFeedback::InstantEffect effect = HbFeedback::None;
    if (boundaryWidgets.contains(widget)) {
        if (interaction == Hb::ContinuousScrolled) {
            effect = HbFeedback::BounceEffect;
        }
        boundaryWidgets.removeAll(widget);
    }

    // stop ongoing continuous and list scrolling feedback effects
    if (continuousFeedbacks.contains(widget)) {
       cancelContinuousFeedback(widget);
    }
    if (activelyScrollingItemView == widget) {
        activelyScrollingItemView = 0;
    }
    playInstantFeedback(widget, effect);
}

/*!
    Plays the instant feedback.
*/
void HbFeedbackEffectEngine::playInstantFeedback(const HbWidget* widget, HbFeedback::InstantEffect effect)
{
    const QGraphicsView* view = widget->mainWindow();
    if (view && HbFeedbackEffectUtils::isFeedbackAllowed(widget)) {
        HbInstantFeedback feedback(effect);
        feedback.setRect(widget, view);
        feedback.setOwningWindow(view);

        if (hbFeedbackPlayer && feedback.isLocated()) {
            hbFeedbackPlayer->playInstantFeedback(feedback);
        }
    }
}

/*!
    Plays the continuous feedback.
*/
void HbFeedbackEffectEngine::playContinuousFeedback(const HbWidget* widget, HbFeedback::ContinuousEffect effect, int intensity)
{
    const QGraphicsView* view = widget->mainWindow();
    if (view && HbFeedbackEffectUtils::isFeedbackAllowed(widget)) {
        HbContinuousFeedback feedback(effect,view);
        feedback.setRect(widget, view);
        feedback.setIntensity(intensity);

        if (hbFeedbackPlayer && feedback.isLocated()) {
            // if continuous feedback is still active and not stopped by continuous feedback timeout
            if (continuousFeedbacks.contains(widget)
                && hbFeedbackPlayer->continuousFeedbackOngoing(continuousFeedbacks.value(widget))) {
                hbFeedbackPlayer->updateContinuousFeedback(continuousFeedbacks.value(widget), feedback);
            } else {
                // if timeout has happened remove the item from the map storing continuous feedback identifiers
                if (continuousFeedbacks.contains(widget)) {
                    continuousFeedbacks.remove(widget);
                }
                // create a new continuous feedback
                continuousFeedbacks.insert(widget, hbFeedbackPlayer->startContinuousFeedback(feedback));
            }
        }
    }
}

/*!
    Cancels the playing of a continuous feedback.
*/
void HbFeedbackEffectEngine::cancelContinuousFeedback(const HbWidget* widget)
{
    if (hbFeedbackPlayer && continuousFeedbacks.contains(widget)) {
        hbFeedbackPlayer->cancelContinuousFeedback(continuousFeedbacks.take(widget));
    }
}

