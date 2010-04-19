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

// internal
#include "hbfeedbackmanager.h"
#include "hbfeedbackplugingroup.h"
#include "hbfeedbackengine.h"
#include "hbinstantinteractionevent_p.h"
#include "hbcontinuousinteractionevent_p.h"
#include "hbwidget.h"

// Qt related
#include <QtDebug>
#include <QList>
#include <QCoreApplication>

static int instantRunningIndex;
static int continuousRunningIndex;

/*!
    @beta
    @hbcore

    \class HbFeedbackManager

    \brief Feedback manager forwards interaction information from the widgets to feedback engine plugins.
    Feedback engine plugins decide what kind of feedback effects will be played. Widgets don't use 
    HbFeedbackManager directly but through HbWidgetFeedback convenience class.
*/

Q_GLOBAL_STATIC(HbFeedbackManager, feedbackManagerGlobal);

class HbFeedbackManagerPrivate : public QObject
{

public:
    HbFeedbackManagerPrivate();
    ~HbFeedbackManagerPrivate();
public:
    HbFeedbackPluginGroup* pluginGroup;
    QList <HbFeedbackEngine*> engines;
};

HbFeedbackManagerPrivate::HbFeedbackManagerPrivate()
{
}


HbFeedbackManagerPrivate::~HbFeedbackManagerPrivate()
{
    delete pluginGroup;
    engines.clear();
}

void removePlugins() {
    HbFeedbackManager* manager = HbFeedbackManager::instance();
    if (manager) {
        manager->plugins().removePlugins();
    }
}

/*!
    Constructor.
*/
HbFeedbackManager::HbFeedbackManager(QObject *parent) : QObject(parent),
        d(new HbFeedbackManagerPrivate)
{
    d->pluginGroup = new HbFeedbackPluginGroup(*this);
    d->pluginGroup->loadPlugins();
    instantRunningIndex = Hb::InstantUser;
    continuousRunningIndex = Hb::ContinuousUser;
    // delete the plugins before the application and widgets are destroyed
    qAddPostRoutine(removePlugins);
}

/*!
    Destructor.
*/
HbFeedbackManager::~HbFeedbackManager()
{
    delete d;
}

/*!
    Returns feedback manager singleton object responsible for initiating feedback effects.

    \return HbFeedbackManager returns feedback manager singleton object
*/
HbFeedbackManager* HbFeedbackManager::instance()
{
    return feedbackManagerGlobal();
}

/*!
    Mediates interaction triggers from widgets to all active feedback engine plugins.

    \param widget the widget being interacted with
    \param interaction the interaction
    \param modifiers extra specifiers to the interaction
*/
void HbFeedbackManager::triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers)
{
    if (widget) {
        if (!widget->testAttribute(Hb::InteractionDisabled)) {
            HbInstantInteractionEvent event(widget, interaction, modifiers);
            foreach (HbFeedbackEngine* engine, d->engines) {
                QCoreApplication::sendEvent(engine, &event);
            }
        }
    } else {
        qWarning("HbFeedbackManager::triggered: Attempt to report an interaction trigger with null widget pointer");
    }
}

/*!
    Mediates continuous interaction triggers to all active feedback plugins.

    \param widget the widget being interacted with
    \param interaction the continuous interaction in progress
    \param delta the direction and distance of the continuous interaction
*/
void HbFeedbackManager::continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta)
{
    if (widget) {
        if (!widget->testAttribute(Hb::InteractionDisabled)) {
            HbContinuousInteractionEvent event(HbContinuousInteractionEvent::ContinuousInteraction, widget, interaction, delta);
            foreach (HbFeedbackEngine* engine, d->engines) {
                QCoreApplication::sendEvent(engine, &event);
            }
        }
    } else {
        qWarning("HbFeedbackManager::continuousTriggered: Attempt to stop a continuous interaction trigger with null widget pointer");
    }
}

/*!
    Mediates a "continuous interaction stop" triggers to all active feedback engine plugins.
    This methods is needed for knowing when to stop continuous feedback effects started by 
    the continuous interaction.

    \param widget the widget being interacted with
    \param interaction the continuous interaction in progress
*/
void HbFeedbackManager::continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction)
{
    if (widget) {
        if (!widget->testAttribute(Hb::InteractionDisabled)) {
            HbContinuousInteractionEvent event(HbContinuousInteractionEvent::ContinuousInteractionStop, widget, interaction);
            foreach (HbFeedbackEngine* engine, d->engines) {
                QCoreApplication::sendEvent(engine, &event);
            }
        }
    } else {
        qWarning("HbFeedbackManager::continuousStopped: Attempt to stop a c trigger with \
                  null widget pointer");
    }
}

/*!
    Registers and returns a unique value (between Hb::InstantUser and Hb::InstantMaxUser) for custom 
    interactions that cannot be covered with standard ones defined in namepace Hb. Returns -1 if all
    custom interactions have been reserved.
*/
int HbFeedbackManager::registerInstantInteraction()
{
    if (instantRunningIndex < Hb::InstantMaxUser) {
        instantRunningIndex++;
        return instantRunningIndex;
    } 
    else {
        return -1;
    }
}

/*!
    Registers and returns an unique value (between Hb::ContinuousUser and Hb::ContinuousMaxUser) for custom 
    continuous interactions that cannot be covered with standard ones defined in namepace Hb. Returns -1 if 
    all custom interactions have been reserved.
*/
int HbFeedbackManager::registerContinuousInteraction()
{
    if (continuousRunningIndex < Hb::ContinuousMaxUser) {
        continuousRunningIndex++;
        return continuousRunningIndex;
    } 
    else {
        return -1;
    }
}

/*!
    Adds or removes HbFeedbackEngine from the list of feedback engines that receive
    interaction events from the feedback manager.

    \param engine the feedback engine to be added or removed
    \param enabled the flag indicating whether the feedback engine is to be added or removed
*/

void HbFeedbackManager::setReceivesInteractions(HbFeedbackEngine* engine, bool enabled)
{
    if (engine) {
        if (enabled) {
            if (!d->engines.contains(engine)) {
                d->engines.append(engine);
            }
        } else {
            int index = d->engines.indexOf(engine);
            if (index != -1) {
                d->engines.removeAt(index);
            }
        }
    } else {
        if (enabled) {
            qWarning("HbFeedbackManager::setReceivesInteractions: Attempt to register engine by passing a null pointer");
        } else {
            qWarning("HbFeedbackManager::setReceivesInteractions: Attempt to unregister engine using a null pointer");
        }
    }
}

/*!
    Returns true if HbFeedbackEngine has already been registered to the manager.

    \param engine the feedback engine
*/
bool HbFeedbackManager::receivesInteractions(HbFeedbackEngine* engine)
{
    bool contains(false);
    if (engine) {
        if (d->engines.contains(engine)) {
            contains = true;
        }
    }
    return contains;
}

/*!
    Returns the group of feedback engine plugins.

    \sa HbFeedbackPluginGroup
*/
HbFeedbackPluginGroup& HbFeedbackManager::plugins()
{
    return *d->pluginGroup;
}

