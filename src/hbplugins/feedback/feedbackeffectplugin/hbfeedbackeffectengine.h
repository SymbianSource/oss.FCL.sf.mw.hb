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

#ifndef HBFEEDBACKEFFECTENGINE_H
#define HBFEEDBACKEFFECTENGINE_H

#include <hbfeedbackengine.h>

#include <QMap>
#include <QList>

class QGraphicsItem;
class HbAbstractViewItem;
class HbWidget;

class HbFeedbackEffectEngine : public HbFeedbackEngine
{
    Q_OBJECT
        
public:
    HbFeedbackEffectEngine();
    ~HbFeedbackEffectEngine();

    void triggered(const HbWidget* widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers);
    void pressed(const HbWidget *widget);
    void released(const HbWidget *widget);
    void longPressed(const HbWidget *widget);
    void clicked(const HbWidget *widget);
    void keyRepeated(const HbWidget *widget);
    void draggedOver(const HbWidget *widget);
    void flicked(const HbWidget *widget);
    void popupOpened(const HbWidget *widget);
    void popupClosed(const HbWidget *widget);
    void boundaryReached(const HbWidget *widget);
    void rotated90Degrees(const HbWidget *widget);
    void selectionChanged(const HbWidget *widget);
    void multitouchActivated(const HbWidget *widget);
    void continuousTriggered(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta);
    void continuousStopped(const HbWidget *widget, Hb::ContinuousInteraction interaction);
        
protected:
    void playInstantFeedback(const HbWidget* widget, HbFeedback::InstantEffect effect);
    void playContinuousFeedback(const HbWidget* widget, HbFeedback::ContinuousEffect effect, int intensity);
    void cancelContinuousFeedback(const HbWidget* HbWidget);

private:
    QMap<const HbWidget*, int> continuousFeedbacks;
    QList<int> oldVisibleIndexes;
    const HbWidget* activelyScrollingItemView;
    QList<const HbWidget*> boundaryWidgets;

    int previousCursorPosition;
};

#endif // HBFEEDBACKEFFECTENGINE_H
