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

#ifndef HBFEEDBACKENGINE_H
#define HBFEEDBACKENGINE_H

#include <hbnamespace.h>
#include <hbfeedbacknamespace.h>

#include <QObject>
#include <QPointF>

class QEvent;
class HbFeedbackManager;
class HbFeedbackEnginePrivate;
class HbWidget;

class HB_CORE_EXPORT HbFeedbackEngine : public QObject
{
    Q_OBJECT
public:

    HbFeedbackEngine();
    HbFeedbackEngine(HbFeedbackManager* manager);
    virtual ~HbFeedbackEngine();

    virtual void setManager(HbFeedbackManager* manager);
    HbFeedbackManager* manager() const;

    virtual void setReceivesInteractions(bool enabled);
    virtual bool receivesInteractions();

signals:
    void receivesInteractionsChanged(bool enabled);

protected:
    Hb::InteractionModifiers modifiers();

    virtual bool event(QEvent* e);
    virtual void triggered(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers);
    virtual void pressed(const HbWidget *widget);
    virtual void released(const HbWidget *widget);
    virtual void clicked(const HbWidget *widget);
    virtual void keyRepeated(const HbWidget *widget);
    virtual void longPressed(const HbWidget *widget);
    virtual void draggedOver(const HbWidget *widget);
    virtual void flicked(const HbWidget *widget);
    virtual void popupOpened(const HbWidget *widget);
    virtual void popupClosed(const HbWidget *widget);
    virtual void boundaryReached(const HbWidget *widget);
    virtual void rotated90Degrees(const HbWidget *widget);
    virtual void selectionChanged(const HbWidget *widget);
    virtual void advancedGestureActivated(const HbWidget *widget);
    virtual void continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta);
    virtual void continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction);

private:
    HbFeedbackEnginePrivate *d;
};

#endif // HBFEEDBACKENGINE_H
