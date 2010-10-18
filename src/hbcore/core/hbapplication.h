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

#ifndef HBAPPLICATION_H
#define HBAPPLICATION_H

#include <QApplication>
#include <hbglobal.h>
#include <hbnamespace.h>

class HbApplicationPrivate;
class HbActivityManager;

class HB_CORE_EXPORT HbApplication : public QApplication
{
    Q_OBJECT

public:
    HbApplication(int &argc,
                  char *argv[],
                  Hb::ApplicationFlags flags = Hb::DefaultApplicationFlags);

#if defined(Q_OS_SYMBIAN)
    HbApplication(QApplication::QS60MainApplicationFactory factory,
                  int &argc,
                  char *argv[],
                  Hb::ApplicationFlags flags = Hb::DefaultApplicationFlags);
#endif // Q_OS_SYMBIAN

    ~HbApplication();

#if defined(Q_OS_SYMBIAN)
    virtual bool symbianEventFilter(const QSymbianEvent *event);
#endif // Q_OS_SYMBIAN

signals:
    void activate();

public:
    HbActivityManager *activityManager();
    Hb::ActivationReason activateReason() const;
    QVariantHash activateParams() const;
    QString activateId() const;
    QVariant activateData();

    void hideSplash();

    static bool startedToBackground(int argc = 0, char *argv[] = 0);

protected:
    HbApplicationPrivate *d_ptr;

private:
    Q_DISABLE_COPY(HbApplication)
    Q_DECLARE_PRIVATE_D(d_ptr, HbApplication)

};

#endif // HBAPPLICATION_H
