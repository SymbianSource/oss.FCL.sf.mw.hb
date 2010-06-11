/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#ifndef HBSPLASHGEN_SERVER_SYMBIAN_P_H
#define HBSPLASHGEN_SERVER_SYMBIAN_P_H

#include <QObject>
#include <QStringList>
#include <qs60mainappui.h>
#include <qs60maindocument.h>
#include <qs60mainapplication.h>

class HbSplashGenerator;
class HbSplashGenServerSymbian;
class HbSplashCompositorInterface;

class HbSplashGenServer : public QObject
{
    Q_OBJECT

public:
    HbSplashGenServer(HbSplashGenerator *generator);
    ~HbSplashGenServer();
    bool startupSuccess() const;
    void addCompositor(HbSplashCompositorInterface *compositor);

private slots:
    void onOutputDirContentsUpdated(const QString &dir, const QStringList &entries);
    void dropCachedData();

private:
    HbSplashGenServerSymbian *mServer;
};

class HbSplashGenAppUi : public QS60MainAppUi
{
public:
    TBool FrameworkCallsRendezvous() const;
};

class HbSplashGenDocument : public QS60MainDocument
{
public:
    HbSplashGenDocument(CEikApplication &app);
    CEikAppUi *CreateAppUiL();
};

class HbSplashGenApplication : public QS60MainApplication
{
protected:
    CApaDocument *CreateDocumentL();
};

#endif
