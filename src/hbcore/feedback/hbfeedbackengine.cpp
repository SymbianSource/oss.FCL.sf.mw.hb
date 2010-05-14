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

#include "hbfeedbackengine.h"
#include "hbfeedbackmanager.h"
#include "hbinstantinteractionevent_p.h"
#include "hbcontinuousinteractionevent_p.h"

#include <QPointer>

/*!
    @beta
    @hbcore
    \class HbFeedbackEngine
    
    \brief Base class for implementing engines for different kinds of feedback effects.
    
    The feedback engine receives interaction information from HbFeedbackManager as interaction and continuous 
    interaction events. All events are received in method event() and forwarded to the corresponding virtual 
    methods overridden by various engine implementations.
    
    Instant interaction information is received by methods pressed(), released(), clicked(), keyRepeated(), 
    longPressed(), draggedOver(), flicked(), popupOpened(), popupClosed(), boundaryReached(), rotated90Degrees(), 
    selectionChanged() and multitouchActivated().
    
    Continuous interaction information is received by methods continuousTriggered() and continuousStopped().

    \sa HbNameSpace, HbFeedbackManager, HbFeedbackPlugin
*/

class HbFeedbackEnginePrivate
{
public: 
    HbFeedbackEnginePrivate() : currentEvent(0) {};
    ~HbFeedbackEnginePrivate() {};
public: 
    QPointer<HbFeedbackManager> m_manager;
    HbInstantInteractionEvent* currentEvent;
};

/*!
    Constructor.
*/
HbFeedbackEngine::HbFeedbackEngine() : d(new HbFeedbackEnginePrivate())
{
}

/*!
    Constructs the engine with required feedback manager. If the HbFeedbackManager object is not provided during construction,
    it needs to be provided by calling setManager().

    \param manager feedback manager object which passes interaction information to the engine
*/
HbFeedbackEngine::HbFeedbackEngine(HbFeedbackManager* manager) : d(new HbFeedbackEnginePrivate())
{
    setManager(manager);
}

/*!
    Destructor.
*/
HbFeedbackEngine::~HbFeedbackEngine()
{
    if (d->m_manager) {
        d->m_manager->setReceivesInteractions(this, false);
    }
    delete d;
}

/*!
    Sets the feedback manager responsible for passing interaction information to the engine.

    \param manager the feedback manager
*/
void HbFeedbackEngine::setManager(HbFeedbackManager* manager)
{
    // unregister engine from the old manager
    if (d->m_manager) {
        d->m_manager->setReceivesInteractions(this, false);
    }

    // setup new manager
    d->m_manager = manager;
    if (d->m_manager) {
        d->m_manager->setReceivesInteractions(this, true);
    } 
}

/*!
    Returns the feedback manager responsible for passing interaction information to the engine.
*/
HbFeedbackManager* HbFeedbackEngine::manager() const
{
    return d->m_manager;
}

/*!
    Enables the engine. When enabled, events received in method event() are forwarded to corresponding
    virtual callback functions that different feedback engines can override.

    \param enabled sets the engine status either enabled or disabled
*/
void HbFeedbackEngine::setReceivesInteractions(bool enabled)
{
    if (d->m_manager) {
        if (d->m_manager->receivesInteractions(this) != enabled) {
            d->m_manager->setReceivesInteractions(this,enabled);
            emit receivesInteractionsChanged(enabled);
        }
    }
}

/*!
    Returns true when the engine is set to receive widget interaction information.
*/
bool HbFeedbackEngine::receivesInteractions()
{
    return d->m_manager && d->m_manager->receivesInteractions(this);
}

/*!
    All events received by the QObject-based class come trough method event() and are forwarded to 
    the virtual methods corresponding to the interactions.

    \param e the interaction event
*/
bool HbFeedbackEngine::event(QEvent* e)
{
    if (d->m_manager) {
        if (e->type() == HbInstantInteractionEvent::InstantInteraction) {
            HbInstantInteractionEvent *instantEvent = static_cast<HbInstantInteractionEvent *>(e);
            if (instantEvent) {
                d->currentEvent=instantEvent;
                const HbWidget* widget = instantEvent->widget();
                if (widget) {
                    triggered(widget, instantEvent->interaction(), instantEvent->modifiers());
                }
                d->currentEvent = 0;
            }
        } else if (e->type() == HbContinuousInteractionEvent::ContinuousInteraction) {
            HbContinuousInteractionEvent *continuousEvent = static_cast<HbContinuousInteractionEvent *>(e);
            if (continuousEvent) {
                const HbWidget* widget = continuousEvent->widget();
                if (widget) {
                    continuousTriggered(widget, continuousEvent->interaction(), continuousEvent->delta());
                }
            }
        } else if (e->type() == HbContinuousInteractionEvent::ContinuousInteractionStop) {
            HbContinuousInteractionEvent *continuousEvent = static_cast<HbContinuousInteractionEvent *>(e);
            if (continuousEvent) {
                const HbWidget* widget = continuousEvent->widget();
                if (widget) {
                    continuousStopped(widget, continuousEvent->interaction());
                }
            }
        }
    }
    return QObject::event(e);
}

/*!
    During the scope of interaction triggered callbacks, feedback engines
    can query modifier values attached to the interaction. Modifier enumeration
    can be used to differentiate situations where same interaction can cause
    multiple different feedback effects depending on widget state.

    Always returns zero when called outside the scope of triggered callbacks.

    \return extra modifier to the interaction
*/

Hb::InteractionModifiers HbFeedbackEngine::modifiers()
{
    Hb::InteractionModifiers modifiers(0);
    if (d->currentEvent) {
        modifiers = d->currentEvent->modifiers();
    }
    return modifiers;
}

/*!
    Called when the engine receives interaction event from the feedback manager.
    Base class implementation forwards interaction information to dedicated interaction methods,
    remember to call HbFeedbackEngine::triggered() if you want to use the dedicated methods.

    \param widget widget being interacted with
    \param interaction the interaction
    \param modifiers extra modifiers to the interaction
*/
void HbFeedbackEngine::triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers)
{
    Q_UNUSED(modifiers);
    switch(interaction) {
        case Hb::InstantPressed: {
           pressed(widget);
        }

        break;

        case Hb::InstantReleased: {
           released(widget);
        }

        break;

        case Hb::InstantClicked: {
           clicked(widget);
        }

        break;

        case Hb::InstantKeyRepeated: {
           keyRepeated(widget);
        }

        break;

        case Hb::InstantLongPressed: {
           longPressed(widget);
        }

        break;

        case Hb::InstantDraggedOver: {
           draggedOver(widget);
        }

        break;

        case Hb::InstantFlicked: {
           flicked(widget);
        }

        break;

        case Hb::InstantPopupOpened: {
           popupOpened(widget);
        }

        break;

        case Hb::InstantPopupClosed: {
           popupClosed(widget);
        }

        break;

        case Hb::InstantBoundaryReached: {
           boundaryReached(widget);
        }

        break;

        case Hb::InstantRotated90Degrees: {
           rotated90Degrees(widget);
        }

	    break;

        case Hb::InstantSelectionChanged: {
           selectionChanged(widget);
        }
        break;

        case Hb::InstantMultitouchActivated: {
           multitouchActivated(widget);
        }
        break;

        default:
        break;
    }
}

/*!
    Called when the engine receives a press event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::pressed(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a release event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::released(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a click event from the feedback manager.

    \param widget being interacted with
*/
void HbFeedbackEngine::clicked(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a key repeat event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::keyRepeated(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a long press event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::longPressed(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a dragged over event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::draggedOver(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a flick event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::flicked(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives the information whenever a popup
    like menu, note or tooltip is opened from the feedback manager.

    \param widget the widget being interacted with

*/
void HbFeedbackEngine::popupOpened(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives the information whenever a popup like menu, 
    note or tooltip is closed from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::popupClosed(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a "boundary reached" event from the feedback manager.
    Boundary reached event should be initiated whenever user flicks or pans so that the 
    boundary of the scroll area is reached.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::boundaryReached(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a "rotated 90 degrees" event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::rotated90Degrees(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a "selection changed" event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::selectionChanged(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a "multitouch activated" event from the feedback manager.

    \param widget the widget being interacted with
*/
void HbFeedbackEngine::multitouchActivated(const HbWidget *widget)
{
    Q_UNUSED(widget);
}

/*!
    Called when the engine receives a continuous interaction event from the feedback manager.

    \param widget the widget being interacted with
    \param interaction the interaction in progress
    \param delta supplies the direction and distance of the continuous interaction.
*/
void HbFeedbackEngine::continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    Q_UNUSED(widget);
    Q_UNUSED(interaction);
    Q_UNUSED(delta);
}

/*!
    Called when the engine receives a "continuous interaction stop" event from the feedback manager.
    This method indicates when to stop previously started continuous feedback effect.

    \param widget the widget being interacted with
    \param interaction the interaction in progress
*/
void HbFeedbackEngine::continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    Q_UNUSED(widget);
    Q_UNUSED(interaction);
}
