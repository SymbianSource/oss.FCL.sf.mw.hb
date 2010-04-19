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

#ifndef HBFEEDBACKMANAGER_H
#define HBFEEDBACKMANAGER_H

#include <hbnamespace.h>

#include <QObject>
#include <QPointF>

class HbFeedbackPluginGroup;
class HbFeedbackManagerPrivate;
class HbFeedbackEngine;
class HbWidget;

class HB_CORE_EXPORT HbFeedbackManager : public QObject
{
    Q_OBJECT
public:
    static HbFeedbackManager* instance();
    HbFeedbackManager(QObject *parent = 0);
    virtual ~HbFeedbackManager();

    void triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers = 0);
    void continuousTriggered(const HbWidget *object, Hb::ContinuousInteraction interaction, QPointF delta = QPointF());
    void continuousStopped(const HbWidget *object, Hb::ContinuousInteraction interaction);

    void setReceivesInteractions(HbFeedbackEngine* engine, bool enabled);
    bool receivesInteractions(HbFeedbackEngine* engine);

    static int registerInstantInteraction();
    static int registerContinuousInteraction();

    HbFeedbackPluginGroup& plugins();

private:
    HbFeedbackManagerPrivate * const d;
};

#endif /*HBFEEDBACKMANAGER_H*/
