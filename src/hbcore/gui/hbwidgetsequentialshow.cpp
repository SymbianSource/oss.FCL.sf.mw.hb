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

#include "hbwidgetsequentialshow_p.h"
#include <hbwidget.h>

HbWidgetSequentialShow::HbWidgetSequentialShow(ExternalSynchorization *externalSynchorization,
    void *funcArg)
{
    mWidgetShown = 0;
    mExternalSynchorization = externalSynchorization; // no ownership transfer
    mExternalSynchorizationArg = funcArg;
}

HbWidgetSequentialShow::~HbWidgetSequentialShow()
{
}

// Add widget into a show queue
void HbWidgetSequentialShow::add(HbWidget *widget)
{
    // Allow only single instance of a same widget in the queue
    if (mQueue.contains(widget)) {
        return;
    }
    mQueue.append(widget);
    connect(widget, SIGNAL(visibleChanged()), SLOT(widgetVisibleChanged()));
}

// Remove widget from a show queue
void HbWidgetSequentialShow::remove(HbWidget *widget)
{
    if (mWidgetShown == widget) {
        mWidgetShown = 0;
    }
    mQueue.removeOne(widget);
    widget->disconnect(this);
    showNext();
}

// Set currently showing widget
void HbWidgetSequentialShow::setShowing(HbWidget *widget)
{
    Q_ASSERT(mWidgetShown == 0);
    mWidgetShown = widget;
}

// Return a widget that is to be shown
HbWidget *HbWidgetSequentialShow::toShowNow()
{
    HbWidget *toShow = 0;
    if (mWidgetShown == 0 && !mQueue.empty() && externalAllows()){
        toShow = static_cast<HbWidget*>(mQueue.first());
    }
    return toShow;
}

// Show next widget if appropriate
void HbWidgetSequentialShow::showNext()
{
    HbWidget *toShow = toShowNow();
    if (toShow) {
        toShow->show();
    }
}

// Return true if external synchronization allows showing of widget
bool HbWidgetSequentialShow::externalAllows()
{
    bool allows = true;
    if (mExternalSynchorization) {
        allows = mExternalSynchorization(mExternalSynchorizationArg);
    }
    return allows;
}

// Widget visibility has changed
void HbWidgetSequentialShow::widgetVisibleChanged()
{
    // Added to the queue.
    HbWidget *widget = static_cast<HbWidget*>(sender());
    if (!widget->isVisible()) {
        // Hidden widget is removed from the queue
        remove(widget);
    }
    else {
        // Widget is visible, it should be the widget currently shown. If not, remove it from
        // the queue as sequence has been broken.
        if (mWidgetShown != widget) {
            remove(widget);
        }
    }
}

// External synchronization status has changed
void HbWidgetSequentialShow::externalStatusChanged()
{
    showNext();
}
