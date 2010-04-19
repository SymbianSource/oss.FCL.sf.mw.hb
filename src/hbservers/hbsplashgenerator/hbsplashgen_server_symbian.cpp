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

#include "hbsplashgen_server_symbian_p.h"
#include "hbsplashgenerator_p.h"
#include "hbsplashdefs_p.h"
#include <e32base.h>
#include <f32file.h>
#include <QDir>
#include <QDebug>

#define PRE "[hbsplashgenerator] [server]"

class HbSplashGenServerSymbian : public CServer2
{
public:
    HbSplashGenServerSymbian();
    ~HbSplashGenServerSymbian();
    CSession2 *NewSessionL(const TVersion &version, const RMessage2 &message) const;

    void setSplashScreenDir(const QString &dir) { mSplashScreenDir = dir; }
    void setSplashScreenDirContents(const QStringList &entries) { mSplashScreenDirEntries = entries; }

    bool processGetSplash(const RMessage2 &message);

private:
    bool transferHandle(const RMessage2 &message, const QString &fileName);

    RFs mFs;
    QString mSplashScreenDir;
    QStringList mSplashScreenDirEntries;
};

class HbSplashGenServerSession : public CSession2
{
public:
    HbSplashGenServerSession(HbSplashGenServerSymbian *server);
    void ServiceL(const RMessage2 &message);

private:
    HbSplashGenServerSymbian *mServer;
};

HbSplashGenServer::HbSplashGenServer(HbSplashGenerator *generator)
    : mServer(new HbSplashGenServerSymbian)
{
    connect(generator, SIGNAL(outputDirContentsUpdated(QString, QStringList)),
            SLOT(onOutputDirContentsUpdated(QString, QStringList)));
}

HbSplashGenServer::~HbSplashGenServer()
{
    delete mServer;
}

void HbSplashGenServer::onOutputDirContentsUpdated(const QString &dir, const QStringList &entries)
{
    qDebug() << PRE << "splash screen dir contents received" << dir;
    qDebug() << PRE << entries;
    mServer->setSplashScreenDir(dir);
    mServer->setSplashScreenDirContents(entries);
}

HbSplashGenServerSymbian::HbSplashGenServerSymbian()
    : CServer2(CActive::EPriorityHigh)
{
    if (mFs.Connect() == KErrNone) {
        mFs.ShareProtected();
    } else {
        qWarning() << PRE << "cannot connect to file server";
        return;
    }
    TRAPD(err, StartL(hbsplash_server_name));
    if (err == KErrNone) {
        qDebug() << PRE << "server started";
    } else {
        qWarning() << PRE << "server start failed" << err;
    }
}

HbSplashGenServerSymbian::~HbSplashGenServerSymbian()
{
    mFs.Close();
}

CSession2 *HbSplashGenServerSymbian::NewSessionL(const TVersion &version, const RMessage2 &message) const
{
    Q_UNUSED(message);
    TVersion v(hbsplash_version_major, hbsplash_version_minor, hbsplash_version_build);
    if (!User::QueryVersionSupported(v, version)) {
        User::Leave(KErrNotSupported);
    }
    return new (ELeave) HbSplashGenServerSession(const_cast<HbSplashGenServerSymbian *>(this));
}

bool HbSplashGenServerSymbian::transferHandle(const RMessage2 &message, const QString &fileName)
{
    QDir splashScreenDir(mSplashScreenDir);
    QString nativeName = QDir::toNativeSeparators(splashScreenDir.filePath(fileName));
    TPtrC nativeNameDes(static_cast<const TUint16 *>(nativeName.utf16()), nativeName.length());
    RFile f;
    if (f.Open(mFs, nativeNameDes, EFileRead | EFileShareReadersOrWriters) == KErrNone) {
        TInt err = f.TransferToClient(message, 2); // completes the message with the fs handle
        f.Close();
        if (err != KErrNone) {
            // the message is not yet completed if TransferToClient() failed
            return false;
        }
    } else {
        qWarning() << PRE << "could not open" << nativeName;
        return false;
    }
    return true;
}

inline bool readParam(int param, TDes &dst, const RMessage2 &message)
{
    if (message.Read(param, dst) != KErrNone) {
        message.Panic(hbsplash_server_name, HbSplashSrvBadParam);
        return false;
    }
    return true;
}

bool HbSplashGenServerSymbian::processGetSplash(const RMessage2 &message)
{
    if (mSplashScreenDir.isEmpty() || mSplashScreenDirEntries.isEmpty()) {
        qWarning() << PRE << "generator not up yet";
        return false;
    }

    TBuf<16> orientationDes;
    if (!readParam(0, orientationDes, message)) {
        return false;
    }
    TBuf<16> appIdDes;
    if (!readParam(1, appIdDes, message)) {
        return false;
    }
    QString orientation = QString::fromUtf16(orientationDes.Ptr(), orientationDes.Length());
    QString appId = QString::fromUtf16(appIdDes.Ptr(), appIdDes.Length());
    qDebug() << PRE << "getSplash request" << orientation << appId;

    // Do not allow accessing app-specific splash screens of other applications.
    if (!appId.isEmpty()) {
        TUint32 clientId = message.SecureId().iId;
        bool ok;
        TUint32 requestedId = appId.toUInt(&ok, 16);
        if (!ok || clientId != requestedId) {
            qWarning() << PRE << "secure IDs do not match";
            return false;
        }
    }

    // First check for file existence without filesystem access by using the directory
    // listing received from the generator. This prevents wasting time with unnecessary
    // Open() calls.
    QString appSpecificName = QString("splash_%1_%2.spl").arg(orientation).arg(appId);
    bool usingAppSpecific = false;
    QString genericName = QString("splash_%1.spl").arg(orientation);
    QString name = genericName;
    if (!appId.isEmpty() && mSplashScreenDirEntries.contains(appSpecificName)) {
        name = appSpecificName;
        usingAppSpecific = true;
    } else if (!mSplashScreenDirEntries.contains(genericName)) {
        qWarning() << PRE << "no suitable splash screens found" << orientation << appId;
        return false;
    }

    bool transferred = transferHandle(message, name);
    if (!transferred) {
        // If the screens are just being regenerated then there is a chance that
        // the app-specific file is not yet ready but the generic one is already
        // there (and the directory listing checked before is out-of-date). So
        // try the generic file too.
        if (usingAppSpecific) {
            transferred = transferHandle(message, genericName);
        }
        if (!transferred) {
            qWarning() << PRE << "could not transfer file handle";
            return false;
        }
    }

    qDebug() << PRE << "file handle transfered";
    return true;
}

HbSplashGenServerSession::HbSplashGenServerSession(HbSplashGenServerSymbian *server)
    : mServer(server)
{
}

void HbSplashGenServerSession::ServiceL(const RMessage2 &message)
{
    /*
      Supported functions:

      EHbSplashSrvGetSplash
          param 0  [in] requested orientation ("prt" or "lsc")
          param 1  [in] empty or uid (currently ignored if does not match the client's secure id)
          param 2 [out] RFile handle (file is open for read)
          Request is completed with RFs handle or KErrNotFound.
     */

    //TInt requestResult = KErrNone;
    switch (message.Function()) {
    case HbSplashSrvGetSplash:
        if (!mServer->processGetSplash(message)) {
            message.Complete(KErrNotFound);
        }
        break;
    default:
        message.Panic(hbsplash_server_name, HbSplashSrvBadRequest);
        break;
    }
}
