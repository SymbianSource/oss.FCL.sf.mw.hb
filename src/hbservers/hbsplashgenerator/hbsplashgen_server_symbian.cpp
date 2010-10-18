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
#include "hbsplashdirs_p.h"
#include "hbsplashdefs_p.h"
#include "hbsplashblacklist_p.h"
#include "hbsplash_direct_symbian_p.h"
#include "hbsplashcompositor_p.h"
#include <e32base.h>
#include <f32file.h>
#include <fbs.h>
#include <QDir>
#include <QList>
#include <QPair>

const int bitmap_cache_limit = 8; // Must be at least 1. Each bitmap consumes ~1 MB.

#define PRE "[hbsplashgenerator] [server]"

TBool HbSplashGenAppUi::FrameworkCallsRendezvous() const
{
    return EFalse;
}

HbSplashGenDocument::HbSplashGenDocument(CEikApplication &app)
    : QS60MainDocument(app)
{
}

CEikAppUi *HbSplashGenDocument::CreateAppUiL()
{
    splDeb() << PRE << "using custom appui";
    return new(ELeave) HbSplashGenAppUi;
}

CApaDocument *HbSplashGenApplication::CreateDocumentL()
{
    return new(ELeave) HbSplashGenDocument(*this);
}

class HbSplashGenServerSymbian : public CServer2
{
public:
    HbSplashGenServerSymbian(HbSplashGenerator *generator);
    ~HbSplashGenServerSymbian();
    CSession2 *NewSessionL(const TVersion &version, const RMessage2 &message) const;

    void setSplashScreenDir(const QString &dir) {
        mSplashScreenDir = dir;
    }
    void setSplashScreenDirContents(const QStringList &entries) {
        mSplashScreenDirEntries = entries;
    }
    bool startupSuccess() const {
        return mStartupSuccess;
    }
    void clearBitmapCache();
    void addCompositor(HbSplashCompositorInterface *compositor) {
        mCompositors.append(compositor);
    }
    bool processGetSplash(const RMessage2 &message);

private:
    struct BitmapCacheData {
        CFbsBitmap *mBitmap;
        int mExtraSplashFlags;
        Qt::Orientation mOrientation;
    };

    bool completeGetSplash(const RMessage2 &message, const QString &fileName, const QString &ori);
    void completeWithBitmap(const RMessage2 &message, const BitmapCacheData &data);
    const BitmapCacheData *getCachedBitmap(const QString &fileName) const;
    void cacheBitmap(const QString &key, const BitmapCacheData &data);

    HbSplashGenerator *mGen;
    bool mStartupSuccess;
    RFs mFs;
    QString mSplashScreenDir;
    QStringList mSplashScreenDirEntries;
    QList< QPair<QString, BitmapCacheData> > mBitmaps;
    QList<HbSplashCompositorInterface *> mCompositors;
};

class HbSplashGenServerSession : public CSession2
{
public:
    HbSplashGenServerSession(HbSplashGenServerSymbian *server);
    ~HbSplashGenServerSession();
    void ServiceL(const RMessage2 &message);

private:
    HbSplashGenServerSymbian *mServer;
};

HbSplashGenServer::HbSplashGenServer(HbSplashGenerator *generator)
    : mServer(new HbSplashGenServerSymbian(generator))
{
    connect(generator, SIGNAL(outputDirContentsUpdated(QString, QStringList)),
            SLOT(onOutputDirContentsUpdated(QString, QStringList)));
    // React immediately on a theme change, showing out-dated graphics
    // is never acceptable.
    connect(generator, SIGNAL(regenerateStarted()), SLOT(onRegenStarted()));
    // Clear the cache again when all the screens are regenerated to
    // make sure that only the latest ones are used.
    connect(generator, SIGNAL(finished()), SLOT(onRegenEnded()));
}

HbSplashGenServer::~HbSplashGenServer()
{
    delete mServer;
}

void HbSplashGenServer::onOutputDirContentsUpdated(const QString &dir,
        const QStringList &entries)
{
    splDeb() << PRE << "splash screen dir contents received" << dir;
    splDeb() << PRE << entries;
    mServer->setSplashScreenDir(dir);
    mServer->setSplashScreenDirContents(entries);
}

void HbSplashGenServer::onRegenStarted()
{
    mServer->clearBitmapCache();
}

void HbSplashGenServer::onRegenEnded()
{
    mServer->clearBitmapCache();
}

bool HbSplashGenServer::startupSuccess() const
{
    return mServer->startupSuccess();
}

void HbSplashGenServer::addCompositor(HbSplashCompositorInterface *compositor)
{
    mServer->addCompositor(compositor);
}

HbSplashGenServerSymbian::HbSplashGenServerSymbian(HbSplashGenerator *generator)
    : CServer2(CActive::EPriorityHigh), mGen(generator)
{
    TInt err = mFs.Connect();
    if (err == KErrNone) {
        mFs.ShareProtected();
        TRAP(err, StartL(hbsplash_server_name));
        if (err == KErrNone) {
            splDeb() << PRE << "server started";
        } else {
            qWarning() << PRE << "server start failed" << err;
        }
    } else {
        qWarning() << PRE << "cannot connect to file server";
    }
    mStartupSuccess = (err == KErrNone);
    // Now it is the right time to do the rendezvous. By default it would be
    // done too early so the custom appui disables FrameworkCallsRendezvous and
    // it is done here instead.
    RProcess::Rendezvous(err);
}

HbSplashGenServerSymbian::~HbSplashGenServerSymbian()
{
    mFs.Close();
    clearBitmapCache();
}

CSession2 *HbSplashGenServerSymbian::NewSessionL(const TVersion &version,
        const RMessage2 &message) const
{
    Q_UNUSED(message);
    TVersion v(hbsplash_version_major, hbsplash_version_minor, hbsplash_version_build);
    if (!User::QueryVersionSupported(v, version)) {
        User::Leave(KErrNotSupported);
    }
    return new(ELeave) HbSplashGenServerSession(const_cast<HbSplashGenServerSymbian *>(this));
}

void HbSplashGenServerSymbian::clearBitmapCache()
{
    for (int i = 0, ie = mBitmaps.count(); i != ie; ++i) {
        delete mBitmaps.at(i).second.mBitmap;
    }
    mBitmaps.clear();
}

const HbSplashGenServerSymbian::BitmapCacheData *HbSplashGenServerSymbian::getCachedBitmap(
    const QString &fileName) const
{
    for (int i = 0, ie = mBitmaps.count(); i != ie; ++i) {
        if (!mBitmaps.at(i).first.compare(fileName, Qt::CaseInsensitive)) {
            return &mBitmaps.at(i).second;
        }
    }
    return 0;
}

void HbSplashGenServerSymbian::cacheBitmap(const QString &key, const BitmapCacheData &data)
{
    while (mBitmaps.count() >= bitmap_cache_limit) {
        delete mBitmaps.at(0).second.mBitmap;
        mBitmaps.removeAt(0);
    }
    mBitmaps.append(QPair<QString, BitmapCacheData>(key, data));
}

void HbSplashGenServerSymbian::completeWithBitmap(const RMessage2 &message,
        const BitmapCacheData &data)
{
    foreach(HbSplashCompositorInterface * compositor, mCompositors) {
        compositor->composeToBitmap(data.mBitmap, data.mOrientation, data.mExtraSplashFlags);
    }
    TPckg<TInt> bmpHandle(data.mBitmap->Handle());
    message.Write(3, bmpHandle);
    message.Complete(KErrNone);
}

bool HbSplashGenServerSymbian::completeGetSplash(const RMessage2 &message,
        const QString &fileName,
        const QString &ori)
{
    bool wantsBitmap = message.Function() == HbSplashSrvGetSplashData;
    if (wantsBitmap) {
        const BitmapCacheData *cachedBitmapData = getCachedBitmap(fileName);
        if (cachedBitmapData) {
            splDeb() << PRE << "returning cached bitmap for" << fileName;
            completeWithBitmap(message, *cachedBitmapData);
            return true;
        }
    }
    QDir splashScreenDir(mSplashScreenDir);
    QString nativeName = QDir::toNativeSeparators(splashScreenDir.filePath(fileName));
    splDeb() << PRE << "trying to read" << nativeName;
    TPtrC nativeNameDes(static_cast<const TUint16 *>(nativeName.utf16()), nativeName.length());
    RFile f;
    if (f.Open(mFs, nativeNameDes, EFileRead | EFileShareReadersOrWriters) == KErrNone) {
        if (wantsBitmap) {
            BitmapCacheData data;
            data.mBitmap = static_cast<CFbsBitmap *>(
                               HbSplashDirectSymbian::load(&f, &data.mExtraSplashFlags));
            f.Close();
            if (data.mBitmap) {
                data.mOrientation = Qt::Vertical;
                if (!ori.compare(QLatin1String("lsc"), Qt::CaseInsensitive)) {
                    data.mOrientation = Qt::Horizontal;
                }
                cacheBitmap(fileName, data);
                completeWithBitmap(message, data);
            } else {
                qWarning() << PRE << "splash load failed";
                return false;
            }
        } else {
            TInt err = f.TransferToClient(message, 3); // completes the message with the fs handle
            f.Close();
            if (err != KErrNone) {
                // the message is not yet completed if TransferToClient() failed
                return false;
            }
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
    if (mGen->newRegenPending()) {
        qWarning() << PRE << "getSplash: processing of the new theme is pending, stop";
        return false;
    }

    TBuf<16> orientationDes;
    if (!readParam(0, orientationDes, message)) {
        return false;
    }
    TBuf<32> appIdDes;
    if (!readParam(1, appIdDes, message)) {
        return false;
    }
    TBuf<64> screenIdDes;
    if (!readParam(2, screenIdDes, message)) {
        return false;
    }
    QString orientation = QString::fromUtf16(orientationDes.Ptr(), orientationDes.Length());
    QString appId = QString::fromUtf16(appIdDes.Ptr(), appIdDes.Length());
    QString screenId = QString::fromUtf16(screenIdDes.Ptr(), screenIdDes.Length());
    splDeb() << PRE << "getSplash request" << orientation << appId << screenId;

    // Do not allow accessing app-specific splash screens of other applications.
    if (!appId.isEmpty() || !screenId.isEmpty()) {
        TUint32 clientId = message.SecureId().iId;
        bool ok;
        TUint32 requestedId = appId.toUInt(&ok, 16);
        if (!ok || clientId != requestedId) {
            qWarning() << PRE << "secure IDs do not match";
            return false;
        }
    }

    // No splash screen for blacklisted clients.
    if (hbsplash_blacklist().contains(message.SecureId().iId)) {
        qWarning() << PRE << "app is blacklisted";
        return false;
    }

    bool cachedEntryListValid = true;
    if (mSplashScreenDir.isEmpty() || mSplashScreenDirEntries.isEmpty()) {
        qWarning() << PRE << "getSplash: no contents received yet, using fallback";
        mSplashScreenDir = hbsplash_output_dir();
        cachedEntryListValid = false;
    }

    // First check for file existence without filesystem access by using the directory
    // listing received from the generator. This prevents wasting time with unnecessary
    // Open() calls.
    QString appSpecificName;
    if (!screenId.isEmpty()) {
        appSpecificName = QString("splash_%1_%2_%3.spl").arg(orientation).arg(appId).arg(screenId);
    } else {
        appSpecificName = QString("splash_%1_%2.spl").arg(orientation).arg(appId);
    }
    bool usingAppSpecific = false;
    QString genericName = QString("splash_%1.spl").arg(orientation);
    QString name = genericName;
    if (cachedEntryListValid) {
        if (!appId.isEmpty() && mSplashScreenDirEntries.contains(appSpecificName, Qt::CaseInsensitive)) {
            name = appSpecificName;
            usingAppSpecific = true;
        } else if (!mSplashScreenDirEntries.contains(genericName)) {
            qWarning() << PRE << "no suitable splash screens found" << orientation << appId;
            return false;
        }
    } else {
        // The generator has not yet sent any notification about the splash dir and its
        // contents. Therefore just try the app-specific screen first and then fall back
        // to the generic one if needed.
        name = appSpecificName;
        usingAppSpecific = true;
    }

    bool completed = completeGetSplash(message, name, orientation);
    if (!completed) {
        // If the screens are just being regenerated then there is a chance that
        // the app-specific file is not yet ready but the generic one is already
        // there (and the directory listing checked before is out-of-date). So
        // try the generic file too.
        if (usingAppSpecific) {
            completed = completeGetSplash(message, genericName, orientation);
        }
        if (!completed) {
            qWarning() << PRE << "could not complete getSplash request";
            return false;
        }
    }

    splDeb() << PRE << "getSplash request completed";
    if (!cachedEntryListValid) {
        // Set the splash dir back to empty so future invocations can also
        // recognize that the generator has not notified us yet.
        mSplashScreenDir.clear();
    }
    return true;
}

HbSplashGenServerSession::HbSplashGenServerSession(HbSplashGenServerSymbian *server)
    : mServer(server)
{
    splDeb() << PRE << "new session";
}

HbSplashGenServerSession::~HbSplashGenServerSession()
{
    splDeb() << PRE << "session destroyed";
}

void HbSplashGenServerSession::ServiceL(const RMessage2 &message)
{
    /*
      Supported functions:

      EHbSplashSrvGetSplashFile
          param 0  [in] requested orientation ("prt" or "lsc")
          param 1  [in] empty or uid (currently ignored if does not match the client's secure id)
          param 2  [in] empty or screen id
          param 3 [out] RFile handle (file is open for read)
          Request is completed with RFs handle or KErrNotFound.

      EHbSplashSrvGetSplashData
          param 0  [in] requested orientation ("prt" or "lsc")
          param 1  [in] empty or uid (currently ignored if does not match the client's secure id)
          param 2  [in] empty or screen id
          param 3 [out] CFbsBitmap handle
     */

    splDeb() << PRE << "ServiceL" << message.Function() << QString::number(message.SecureId().iId, 16);
    switch (message.Function()) {
    case HbSplashSrvGetSplashFile: // fallthrough
    case HbSplashSrvGetSplashData:
        if (!mServer->processGetSplash(message)) {
            message.Complete(KErrNotFound);
        }
        break;
    default:
        message.Panic(hbsplash_server_name, HbSplashSrvBadRequest);
        break;
    }
}
