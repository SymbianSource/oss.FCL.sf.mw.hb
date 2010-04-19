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

#ifndef HBINSTANTINTERACTIONEVENT_P_H
#define HBINSTANTINTERACTIONEVENT_P_H

#include <hbnamespace.h>

#include <QEvent>

class HbWidget;

class HB_AUTOTEST_EXPORT HbInstantInteractionEvent : public QEvent
{
public:
    // Feedback manager specific event
    static const int InstantInteraction;

    HbInstantInteractionEvent(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers = 0);
    ~HbInstantInteractionEvent();

    inline const HbWidget* widget() const { return m_widget; };
    inline Hb::InstantInteraction interaction() const { return m_interaction; };
    inline Hb::InteractionModifiers modifiers() const { return m_modifiers; };

private:
    const HbWidget *m_widget;
    Hb::InstantInteraction m_interaction;
    Hb::InteractionModifiers m_modifiers;
};

#endif // HBINSTANTINTERACTIONEVENT_P_H
