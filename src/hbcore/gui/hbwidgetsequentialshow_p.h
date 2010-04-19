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

#ifndef HBWIDGETSEQUENTIALSHOW_P_H
#define HBWIDGETSEQUENTIALSHOW_P_H

#include <hbglobal.h>

#include <QObject>
#include <QList>

class HbWidget;
class HbDeviceDialogServerStatus;

// Class that serializes showing of widgets. Instead of widgets showing on top of each other
// they are shown in sequence. Notification dialogs are shown this way but the class could
// be used for other widgets also. Class does not take ownership of widgets. It only maintains
// pointers to them in a show order.
class HB_CORE_PRIVATE_EXPORT HbWidgetSequentialShow : public QObject
{
    Q_OBJECT

public:
    typedef bool ExternalSynchorization(void *arg);
public:
    HbWidgetSequentialShow(ExternalSynchorization *externalSynchorization = 0, void *funcArg = 0);
    ~HbWidgetSequentialShow();
    void add(HbWidget *widget);
    void remove(HbWidget *widget);
    void setShowing(HbWidget *widget);
    HbWidget *toShowNow();

private:
    void showNext();
    bool externalAllows();

private slots:
    void widgetVisibleChanged();
    void externalStatusChanged();

private:
    QList<QObject*> mQueue;
    HbWidget *mWidgetShown; // widget that is currently shown
    ExternalSynchorization *mExternalSynchorization;
    void *mExternalSynchorizationArg;
};

#endif // HBWIDGETSEQUENTIALSHOW_P_H
