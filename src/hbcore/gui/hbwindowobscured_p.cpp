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

#include <qdebug.h>

#include "hbmainwindow.h"
#include "hbwindowobscured_p.h"
#include "hbinstance.h"
#include "hbevent.h"

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#include <qx11info_x11.h>
#endif // defined(Q_WS_X11)


#if defined(Q_OS_SYMBIAN)
#include <qsymbianevent.h>
#include <coecntrl.h>
#endif

bool HbWindowObscured::filterInstalled = false;
QCoreApplication::EventFilter HbWindowObscured::prevEventFilter = 0;

void HbWindowObscured::installWindowEventFilter()
{
    //Install only one eventfilter, HbMainWindow can be created multiple times.
    if (!filterInstalled) {
        prevEventFilter = qApp->setEventFilter(eventFilter);
        filterInstalled = true;
    }
}

#if defined(Q_WS_X11) //eventfilter implementation for xlib
bool HbWindowObscured::eventFilter(void *message, long *result)
{
    XEvent *xEvent = (XEvent *)message;

    if (xEvent->type == VisibilityNotify) {
        XVisibilityEvent *event = (XVisibilityEvent *)xEvent;
        QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
        int mainWindowCount = mainWindows.size();
        for (int index = 0; index < mainWindowCount; ++index) {
            HbMainWindow *window = mainWindows[index];
            if (window != 0 && window->effectiveWinId() == event->window) {
                bool obscuredState = (event->state == VisibilityFullyObscured);

                HbWindowObscuredChangedEvent obscureChangedEvent(obscuredState);
                QCoreApplication::sendEvent(window, &obscureChangedEvent);
                break;
            }
        }

    }
    //Call the previous event filter if there is one, else return false with no filtering.
    return prevEventFilter ? prevEventFilter(message, result) : false;
}
#elif defined(Q_WS_S60) //eventfilter implementation for symbian.
bool HbWindowObscured::eventFilter(void *message, long *result)
{
    QSymbianEvent *symEvent = static_cast<QSymbianEvent *>(message);
    if (symEvent->type() == QSymbianEvent::WindowServerEvent) {
        const TWsEvent *wsEvent = symEvent->windowServerEvent();
        switch (wsEvent->Type()) {
        case EEventWindowVisibilityChanged: {
            // we need to generate an HbWindowObscuredChanged event.
            CCoeControl *control = reinterpret_cast<CCoeControl *>(wsEvent->Handle());
            RDrawableWindow *rWindow = control->DrawableWindow();
            QList<HbMainWindow *> mainWindows = hbInstance->allMainWindows();
            int mainWindowCount = mainWindows.size();
            for (int index = 0; index < mainWindowCount; ++index) {
                HbMainWindow *window = mainWindows[index];
                if (window != 0
                        && window->effectiveWinId()->DrawableWindow() == rWindow) {

                    const TWsVisibilityChangedEvent *visChangedEvent = wsEvent->VisibilityChanged();
                    bool obscuredState =
                        (visChangedEvent->iFlags & TWsVisibilityChangedEvent::ENotVisible) ?
                        true : false;

                    HbWindowObscuredChangedEvent obscureChangedEvent(obscuredState);
                    QCoreApplication::sendEvent(window, &obscureChangedEvent);
                    break;
                }
            }
        }
        }
    }
    //Call the previous event filter if there is one, else return false with no filtering.
    return prevEventFilter ? prevEventFilter(message, result) : false;
}
#else //defined(Q_WS_S60)
//Generic evenfilter for non-supported platform i.e windows. Do nothing.
bool HbWindowObscured::eventFilter(void *, long *)
{
    return false;
}
#endif
