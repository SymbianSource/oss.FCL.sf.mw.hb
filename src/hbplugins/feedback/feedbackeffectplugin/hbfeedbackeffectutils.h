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

#ifndef HBFEEDBACKEFFECTUTILS_H
#define HBFEEDBACKEFFECTUTILS_H

#include <hbnamespace.h>
#include <hbfeedbacknamespace.h>

#include <QRect>
#include <QPointF>
#include <QModelIndex>

class QGraphicsItem;

class HbWidget;

class QGraphicsView;

class HbAbstractItemView;

class HbFeedbackEffectUtils
{
public:

    enum WidgetFamily {
        Undefined,
        Button,
        Slider,
        List,
        Grid,
        Tab,
        Editor,
        Popup
    };

    static int parentItemType(const HbWidget *widget);
    static WidgetFamily widgetFamily(const HbWidget *widget);
    static HbFeedback::InstantEffect instantOnPress(const HbWidget *widget, Hb::InteractionModifiers modifiers);
    static HbFeedback::InstantEffect instantOnRelease(const HbWidget *widget, Hb::InteractionModifiers modifiers);
    static HbFeedback::InstantEffect instantOnKeyRepeat(const HbWidget *widget);
    static HbFeedback::InstantEffect instantOnDrag(const HbWidget *widget, Hb::InteractionModifiers modifiers);
    static HbFeedback::InstantEffect instantOnKeyPress(const HbWidget *widget, Hb::InteractionModifiers modifiers);
    static HbFeedback::InstantEffect instantOnEditorHighlight(const HbWidget *widget, int previousCursorFocus);
    static HbFeedback::InstantEffect instantOnSelectionChanged(const HbWidget *widget, Hb::InteractionModifiers modifiers);
    static HbFeedback::InstantEffect instantOnPopupOpened(const HbWidget *widget);
    static HbFeedback::InstantEffect instantOnPopupClosed(const HbWidget *widget);
    static HbFeedback::ContinuousEffect continuousEffect(const HbWidget *widget, Hb::ContinuousInteraction interaction);
    static HbFeedback::Modalities modalities(const HbWidget *widget, Hb::InstantInteraction interaction, Hb::InteractionModifiers modifiers);
    static HbFeedback::Modalities modalities(const HbWidget *widget, Hb::ContinuousInteraction interaction, Hb::InteractionModifiers modifiers);
    static bool isFeedbackAllowedForPopup(const HbWidget *widget);
    static int intensity(const HbWidget *widget, Hb::ContinuousInteraction interaction, QPointF delta = QPointF());
    static bool isFeedbackAllowed(const HbWidget* widget);
    static bool isSliderMoveContinuous(const HbWidget *widget);
    static bool isOptionsMenuEmpty(const HbWidget *widget);
};

#endif // HBFEEDBACKEFFECTUTILS_H
