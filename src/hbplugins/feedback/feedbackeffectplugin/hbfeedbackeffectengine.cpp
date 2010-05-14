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

/*
    Constructor
*/
HbFeedbackEffectEngine::HbFeedbackEffectEngine() : activelyScrollingItemView(0), previousCursorPosition(0)
{
    instantFeedback = new HbInstantFeedback(HbFeedback::None);
}

/*
    Destructor
*/
HbFeedbackEffectEngine::~HbFeedbackEffectEngine()
{
    if(instantFeedback)
        delete instantFeedback;
    foreach(HbContinuousFeedback *feedback, continuousFeedbacks) {
        feedback->stop();
        delete feedback;
    }

    continuousFeedbacks.clear();
    boundaryWidgets.clear();
}

QString HbFeedbackEffectEngine::effectOverrideProperty(Hb::InstantInteraction interaction)
{
    QString propertyName;
    switch (interaction) {
    case Hb::InstantPressed:
        propertyName = "pressedFeedbackEffect";
        break;
    case Hb::InstantReleased:
        propertyName = "releasedFeedbackEffect";
        break;
    case Hb::InstantClicked:
        propertyName = "clickedFeedbackEffect";
        break;
    case Hb::InstantKeyRepeated:
        propertyName = "keyRepeatedFeedbackEffect";
        break;
    case Hb::InstantLongPressed:
        propertyName = "longPressedFeedbackEffect";
        break;
    case Hb::InstantDraggedOver:
        propertyName = "draggedOverFeedbackEffect";
        break;
    case Hb::InstantFlicked:
        propertyName = "flickedFeedbackEffect";
        break;
    case Hb::InstantPopupOpened:
        propertyName = "popupOpenedFeedbackEffect";
        break;
    case Hb::InstantPopupClosed:
        propertyName = "popupClosedFeedbackEffect";
        break;
    case Hb::InstantBoundaryReached:
        propertyName = "boundaryReachedFeedbackEffect";
        break;
    case Hb::InstantRotated90Degrees:
        propertyName = "rotated90DegreesFeedbackEffect";
        break;
    case Hb::InstantSelectionChanged:
        propertyName = "selectionChangedFeedbackEffect";
        break;
    case Hb::InstantMultitouchActivated:
        propertyName = "multitouchActivatedFeedbackEffect";
        break;
    default:
        propertyName = "defaultFeedbackEffect";
        break;
    }
    return propertyName;
}

QString HbFeedbackEffectEngine::effectOverrideProperty(Hb::ContinuousInteraction interaction)
{
    QString propertyName;
    switch (interaction) {
    case Hb::ContinuousScrolled:
        propertyName = "scrolledFeedbackEffect";
        break;
    case Hb::ContinuousDragged:
        propertyName = "draggedFeedbackEffect";
        break;
    case Hb::ContinuousPinched:
        propertyName = "pinchedFeedbackEffect";
        break;
    case Hb::ContinuousRotated:
        propertyName = "rotatedFeedbackEffect";
        break;
    default:
        propertyName = "defaultFeedbackEffect";
        break;
    }
    return propertyName;
}

QString HbFeedbackEffectEngine::modalitiesOverrideProperty(Hb::InstantInteraction interaction)
{
    QString propertyName;
    switch (interaction) {
    case Hb::InstantPressed:
        propertyName = "pressedFeedbackModalities";
        break;
    case Hb::InstantReleased:
        propertyName = "releasedFeedbackModalities";
        break;
    case Hb::InstantClicked:
        propertyName = "clickedFeedbackModalities";
        break;
    case Hb::InstantKeyRepeated:
        propertyName = "keyRepeatedFeedbackModalities";
        break;
    case Hb::InstantLongPressed:
        propertyName = "longPressedFeedbackModalities";
        break;
    case Hb::InstantDraggedOver:
        propertyName = "draggedOverFeedbackModalities";
        break;
    case Hb::InstantFlicked:
        propertyName = "flickedFeedbackModalities";
        break;
    case Hb::InstantPopupOpened:
        propertyName = "popupOpenedFeedbackModalities";
        break;
    case Hb::InstantPopupClosed:
        propertyName = "popupClosedFeedbackModalities";
        break;
    case Hb::InstantBoundaryReached:
        propertyName = "boundaryReachedFeedbackModalities";
        break;
    case Hb::InstantRotated90Degrees:
        propertyName = "rotated90DegreesFeedbackModalities";
        break;
    case Hb::InstantSelectionChanged:
        propertyName = "selectionChangedFeedbackModalities";
        break;
    case Hb::InstantMultitouchActivated:
        propertyName = "multitouchActivatedFeedbackModalities";
        break;
    default:
        propertyName = "defaultFeedbackModalities";
        break;
    }
    return propertyName;
}

QString HbFeedbackEffectEngine::modalitiesOverrideProperty(Hb::ContinuousInteraction interaction)
{
    QString propertyName;
    switch (interaction) {
    case Hb::ContinuousScrolled:
        propertyName = "scrolledFeedbackModalities";
        break;
    case Hb::ContinuousDragged:
        propertyName = "draggedFeedbackModalities";
        break;
    case Hb::ContinuousPinched:
        propertyName = "pinchedFeedbackModalities";
        break;
    case Hb::ContinuousRotated:
        propertyName = "rotatedFeedbackModalities";
        break;
    default:
        propertyName = "defaultFeedbackModalities";
        break;
    }
    return propertyName;
}


bool HbFeedbackEffectEngine::widgetOverridesEffect(const HbWidget *widget, Hb::InstantInteraction interaction) {
    return (widget == overrider.widget && overrider.overridesEffect && overrider.instantInteraction == interaction);
}

bool HbFeedbackEffectEngine::widgetOverridesEffect(const HbWidget *widget, Hb::ContinuousInteraction interaction) {
    return (widget == overrider.widget && overrider.overridesEffect && overrider.continuousInteraction == interaction);
}

bool HbFeedbackEffectEngine::widgetOverridesModalities(const HbWidget *widget,Hb::InstantInteraction interaction) {
    return (widget == overrider.widget && overrider.overridesModalities && overrider.instantInteraction == interaction);
}

bool HbFeedbackEffectEngine::widgetOverridesModalities(const HbWidget *widget,Hb::ContinuousInteraction interaction) {
    return (widget == overrider.widget && overrider.overridesModalities && overrider.continuousInteraction == interaction);
}


/*
    Called by the feedback manager when an interaction is triggered.
*/
void HbFeedbackEffectEngine::triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers)
{
    
    overrider.widget = widget;
    overrider.instantInteraction = interaction;
    overrider.overridesEffect = false;
    overrider.overridesModalities = false;

    QString effectProperty = effectOverrideProperty(interaction);
    QVariant veffect = widget->property(effectProperty.toLatin1());
    if(veffect.isValid() && veffect.type() == QVariant::Int) {
        overrider.overridesEffect = true;
        overrider.newInstantEffect = HbFeedback::InstantEffect(veffect.toInt());
    }
    
    QString modalitiesProperty = modalitiesOverrideProperty(interaction);
    QVariant vmodalities = widget->property(modalitiesProperty.toLatin1());
    if(vmodalities.isValid() && vmodalities.type() == QVariant::Int) {
        overrider.overridesModalities = true;
        overrider.newModalities = HbFeedback::Modalities(vmodalities.toInt());
    }

    if(overrider.overridesEffect && overrider.overridesModalities) {
        // play specified effect via the specified modality
        playInstantFeedback(widget,overrider.newInstantEffect,overrider.newModalities);

    } else {
        // use default values for unspecified effect/modalities
        HbFeedbackEngine::triggered(widget,interaction,modifiers);
    }
}

/*
    Called by the feedback manager when a widget is pressed.
*/
void HbFeedbackEffectEngine::pressed(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None;
    HbFeedback::Modalities modalities = 0;
    Hb::InstantInteraction interaction = Hb::InstantPressed;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedbackEffectUtils::instantOnPress(widget, modifiers());
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, Hb::InstantPressed, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when a widget is released.
*/
void HbFeedbackEffectEngine::released(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None;
    HbFeedback::Modalities modalities = 0;
    Hb::InstantInteraction interaction = Hb::InstantReleased;

    if (continuousFeedbacks.contains(widget)) {
        cancelContinuousFeedback(widget);
    }

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        // use default effect
        // slider-like widgets are a bit special
        if (HbFeedbackEffectUtils::widgetFamily(widget) == HbFeedbackEffectUtils::Slider) {
            effect = HbFeedbackEffectUtils::instantOnRelease(widget, modifiers()) ;
        }
        // lists in arrange mode react on release
        else if (const HbListViewItem *listViewItem = qobject_cast<const HbListViewItem *>(widget)) {
            const HbAbstractItemView* itemView = listViewItem->itemView();
            if (const HbListView * listView = qobject_cast<const HbListView *>(itemView)) {
                if( listView->arrangeMode() ) {
                    effect = HbFeedbackEffectUtils::instantOnRelease(listViewItem, modifiers()) ;
                }
            }
        }
        else if (HbFeedbackEffectUtils::widgetFamily(widget) == HbFeedbackEffectUtils::Editor) {
            effect = HbFeedbackEffectUtils::instantOnRelease(widget, modifiers());
        } else if (widget->type() == HbPrivate::ItemType_GroupBoxHeadingWidget || widget->type() == Hb::ItemType_ComboBox) {
            effect = HbFeedbackEffectUtils::instantOnRelease(widget, modifiers()) ;
        }
        else if (widget->type() == HbPrivate::ItemType_GroupBoxHeadingWidget
                 || widget->type() == HbPrivate::ItemType_GroupBoxContentWidget
                 || widget->type() == HbPrivate::ItemType_DataGroupHeadingWidget
                 || widget->type() == Hb::ItemType_ComboBox) {
            effect = HbFeedbackEffectUtils::instantOnRelease(widget, modifiers());
        }
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, Hb::InstantReleased, modifiers());
    }

    playInstantFeedback(widget,effect, modalities);

    // normally touch end feedback effect is initiated with a clicked signal
}

/*
    Called by the feedback manager when a long press is detected for a widget.
*/
void HbFeedbackEffectEngine::longPressed(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None;
    HbFeedback::Modalities modalities = 0;
    Hb::InstantInteraction interaction = Hb::InstantLongPressed;

    if(widgetOverridesEffect(widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedback::None;
        if (widget->type() == Hb::ItemType_InputButtonGroup) {
            effect = HbFeedback::SensitiveKeypad;
        }
        else {
            effect = HbFeedback::LongPress;
        }
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);

}

/*
    Called by the feedback manager when a widget is clicked.
*/
void HbFeedbackEffectEngine::clicked(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantClicked;

    if (continuousFeedbacks.contains(widget)) {
        cancelContinuousFeedback(widget);
    }

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedbackEffectUtils::instantOnRelease(widget, modifiers());
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when keyrepeats are detected for a widget.
*/
void HbFeedbackEffectEngine::keyRepeated(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantKeyRepeated;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedbackEffectUtils::instantOnKeyRepeat(widget);
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when a dragged over event is detected for a widget.
*/
void HbFeedbackEffectEngine::draggedOver(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantDraggedOver;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        // For editor highlighting
        if (const HbAbstractEdit *edit = qobject_cast<const HbAbstractEdit *>(widget)) {
            if (edit->cursorPosition() != previousCursorPosition) {
                effect = HbFeedbackEffectUtils::instantOnEditorHighlight(edit, previousCursorPosition) ;
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
                            effect = HbFeedbackEffectUtils::instantOnEditorHighlight(edit, previousCursorPosition);
                            previousCursorPosition = edit->cursorPosition();
                        }
                    }
                }
            }
        } else {
            effect = HbFeedbackEffectUtils::instantOnDrag(widget, modifiers());
        }
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when a widget is flicked.
*/
void HbFeedbackEffectEngine::flicked(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantFlicked;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedback::Flick ;
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when a scroll area widget is flicked or dragged and the boundary reached.
*/
void HbFeedbackEffectEngine::boundaryReached(const HbWidget *widget)
{
    boundaryWidgets.append(widget);
}

/*
    Called by the feedback manager when a rotate gesture is recognized for a widget.
*/
void HbFeedbackEffectEngine::rotated90Degrees(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantRotated90Degrees;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedback::RotateStep;

    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when a popup opens.
*/
void HbFeedbackEffectEngine::popupOpened(const HbWidget *widget)
{
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantPopupOpened;

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    if(widgetOverridesEffect( widget, interaction)) {
        playInstantFeedback(widget, overrider.newInstantEffect, modalities);
    } else {
        if(HbFeedbackEffectUtils::isFeedbackAllowedForPopup(widget)) {
            playInstantFeedback(widget, HbFeedback::PopupOpen, modalities);
        }
    }
}

/*
    Called by the feedback manager when a popup closes.
*/
void HbFeedbackEffectEngine::popupClosed(const HbWidget *widget)
{
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantPopupClosed;

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    if(widgetOverridesEffect( widget, interaction)) {
        playInstantFeedback(widget, overrider.newInstantEffect, modalities);
    } else {
        if(HbFeedbackEffectUtils::isFeedbackAllowedForPopup(widget)) {
            playInstantFeedback(widget, HbFeedback::PopupClose, modalities);
        }
    }
}

/*
    Called by the feedback manager when an item view selection has changed.
*/
void HbFeedbackEffectEngine::selectionChanged(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantSelectionChanged;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedbackEffectUtils::instantOnSelectionChanged(widget, modifiers());
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when multitouch is activated.
*/
void HbFeedbackEffectEngine::multitouchActivated(const HbWidget *widget)
{
    HbFeedback::InstantEffect effect = HbFeedback::None ;
    HbFeedback::Modalities modalities = 0 ;
    Hb::InstantInteraction interaction = Hb::InstantMultitouchActivated;

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
    } else {
        effect = HbFeedback::MultitouchActivate;
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    playInstantFeedback(widget, effect, modalities);
}

/*
    Called by the feedback manager when a continuous interaction is triggered.
*/
void HbFeedbackEffectEngine::continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    HbFeedback::ContinuousEffect effect = HbFeedback::ContinuousNone ;
    HbFeedback::Modalities modalities = 0 ;

    overrider.widget = widget;
    overrider.continuousInteraction = interaction;
    overrider.overridesEffect = false;
    overrider.overridesModalities = false;

    QString effectProperty = effectOverrideProperty(interaction);
    QVariant veffect = widget->property(effectProperty.toLatin1());
    if(veffect.isValid() && veffect.type() == QVariant::Int) {
        overrider.overridesEffect = true;
        overrider.newContinuousEffect = HbFeedback::ContinuousEffect(veffect.toInt());
    }

    QString modalitiesProperty = modalitiesOverrideProperty(interaction);
    QVariant vmodalities = widget->property(modalitiesProperty.toLatin1());
    if(vmodalities.isValid() && vmodalities.type() == QVariant::Int) {
        overrider.overridesModalities = true;
        overrider.newModalities = HbFeedback::Modalities(vmodalities.toInt());
    }

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedbackEffectUtils::modalities(widget, interaction, modifiers());
    }

    if(widgetOverridesEffect( widget, interaction)) {
        // use overridden effect
        effect = overrider.newContinuousEffect;
        int intensity = HbFeedbackEffectUtils::intensity(widget, interaction, delta);
        playContinuousFeedback(widget, HbFeedback::ContinuousEffect(effect), intensity, modalities);
        
    } else {
        // use default effect
        bool feedbackPlayed(false);

        switch(HbFeedbackEffectUtils::widgetFamily(widget))
        {
        case HbFeedbackEffectUtils::Slider:
            {
                if (!HbFeedbackEffectUtils::isSliderMoveContinuous(widget)) {
                    if(!widgetOverridesModalities(widget,interaction)) {
                        modalities = HbFeedback::Audio | HbFeedback::Tactile;
                    }
                    playInstantFeedback(widget, HbFeedback::SensitiveSlider, modalities);
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
                        // To prevent the uninitialized list to cause false new item detections
                        if (oldVisibleIndexes.empty()) {
                            newItemFound = false;
                        }
                        oldVisibleIndexes.clear();
                        oldVisibleIndexes = visibleIndexes;
                        
                        if (newItemFound) {
                            const HbListView* listView = qobject_cast<const HbListView*>(widget);
                            if (!(  listView &&
                                    listView->arrangeMode() &&
                                    listView->draggedItem())){
                                if(!widgetOverridesModalities(widget,interaction)) {
                                    modalities = HbFeedback::Audio | HbFeedback::Tactile;
                                }
                                playInstantFeedback(widget, HbFeedback::ItemScroll, modalities);
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
            playContinuousFeedback(widget, HbFeedbackEffectUtils::continuousEffect(widget, interaction), intensity, modalities);

        }
    }
}

/*
    Called by the feedback manager when a continuous interaction is stopped.
*/
void HbFeedbackEffectEngine::continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    HbFeedback::InstantEffect effect = HbFeedback::None;
    HbFeedback::Modalities modalities = 0;

    if(widgetOverridesModalities(widget,interaction)) {
        modalities = overrider.newModalities ;
    } else  {
        modalities = HbFeedback::Tactile;
    }

    if(widgetOverridesEffect( widget, interaction)) {
        effect = overrider.newInstantEffect;
        playInstantFeedback(widget, overrider.newInstantEffect, modalities);
    } else {
        // determine if instant feedback should be played when a continuous interaction is stopped
        effect = HbFeedback::None;
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
        playInstantFeedback(widget, effect, modalities);
    }
}

/*
    Plays the instant feedback.
*/
void HbFeedbackEffectEngine::playInstantFeedback(const HbWidget* widget, HbFeedback::InstantEffect effect, HbFeedback::Modalities modalities)
{
    const QGraphicsView* view = widget->mainWindow();
    if (view && HbFeedbackEffectUtils::isFeedbackAllowed(widget) && instantFeedback) {
        instantFeedback->setInstantEffect(effect);
        instantFeedback->setRect(widget, view);
        instantFeedback->setOwningWindow(view);
        instantFeedback->setModalities(modalities);
        
        if (instantFeedback->isLocated()) {
            instantFeedback->play();
        }
    }
}

/*
    Plays the continuous feedback.
*/
void HbFeedbackEffectEngine::playContinuousFeedback(const HbWidget* widget, HbFeedback::ContinuousEffect effect, int intensity, HbFeedback::Modalities modalities)
{
    const QGraphicsView* view = widget->mainWindow();

    // if feedback can be played
    if (view && HbFeedbackEffectUtils::isFeedbackAllowed(widget)) {

        // if this widget has been playing
        if (continuousFeedbacks.contains(widget)) {
            HbContinuousFeedback* feedback = continuousFeedbacks.value(widget);

            // if this feedback is already playing then only its effect and intensity are updated
            feedback->setModalities(modalities);
            feedback->setOwningWindow(view);
            feedback->setRect(widget, view);
            feedback->setContinuousEffect(effect);
            feedback->setIntensity(intensity);
            // if this feedback is not being played, play it
            if (!feedback->isPlaying()) {
                feedback->play();
            }
        } else {
            // this widget has not played anything before
            HbContinuousFeedback *feedback = new HbContinuousFeedback();
            feedback->setModalities(modalities);
            feedback->setOwningWindow(view);
            feedback->setRect(widget, view);
            feedback->setContinuousEffect(effect);
            feedback->setIntensity(intensity);
            continuousFeedbacks.insert(widget, feedback);
            feedback->play();
        }
    }
}

/*
    Cancels the playing of a continuous feedback.
*/
void HbFeedbackEffectEngine::cancelContinuousFeedback(const HbWidget* widget)
{
    if (continuousFeedbacks.contains(widget)) {
        HbContinuousFeedback *feedback = continuousFeedbacks.take(widget);
        feedback->stop();
        delete feedback;
    }
}

