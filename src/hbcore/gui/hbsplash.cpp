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

#include "hbsplash_p.h"
#include "hbsplash_direct_symbian_p.h"
#include <QDir>
#include <QFile>
#include <QTime>
#include <QScopedPointer>

#ifdef Q_OS_SYMBIAN
#include <e32std.h>
#include <f32file.h>
#include <fbs.h>
#include "hborientationstatus_p.h"
#include "hbsplashdefs_p.h"
#endif

/*!
  \class HbSplash
  
  \brief Class with utility functions for accessing splash screens.

  \internal
*/

const qint64 image_bytes_limit = 1024 * 1024 * 4;

static QString orientationId(HbSplashScreen::Flags flags)
{
    if (flags & HbSplashScreen::FixedVertical) {
        return QString("prt");
    } else if (flags & HbSplashScreen::FixedHorizontal) {
        return QString("lsc");
    }
#ifdef Q_OS_SYMBIAN
    Qt::Orientation orientation;
    if (HbOrientationStatus::currentOrientation(orientation) && orientation == Qt::Horizontal) {
        return QString("lsc");
    }
#endif
    return QString("prt");
}

struct Params
{
    int *w;
    int *h;
    int *bpl;
    QImage::Format *fmt;
    HbSplashScreen::Flags flags;
    QString appId;
    QString screenId;
    HbSplash::AllocFunc allocFunc;
    void *allocFuncParam;
    quint32 extra;
    bool forceFile;
};

struct File {
    qint64 read(char *buf, qint64 size);
    QString mFullName;
#ifdef Q_OS_SYMBIAN
    RFile mFile;
#else
    QFile mFile;
#endif
};

qint64 File::read(char *buf, qint64 size)
{
#ifdef Q_OS_SYMBIAN
    TPtr8 ptr(reinterpret_cast<unsigned char *>(buf), size);
    return mFile.Read(ptr, size) == KErrNone ? size : 0;
#else
    return mFile.read(buf, size);
#endif
}

static uchar *readSpl(File &f, Params &params)
{
    quint32 w = 0, h = 0, bpl = 0;
    QImage::Format fmt = QImage::Format_Invalid;
    // Have to read the header in one piece in order to minimize the
    // number of read() calls.
    const int headerLength = sizeof(quint32) * 5;
    char headerBuf[headerLength];
    qMemSet(headerBuf, 0, headerLength);
    f.read(headerBuf, headerLength);
    quint32 *headerPtr = reinterpret_cast<quint32 *>(headerBuf);
    w = *headerPtr++;
    h = *headerPtr++;
    bpl = *headerPtr++;
    fmt = (QImage::Format) *headerPtr++;
    params.extra = *headerPtr;
    if (fmt != QImage::Format_ARGB32_Premultiplied) {
        qWarning("[hbsplash] Image format for %s is not ARGB32_PRE (is %d instead)",
                 qPrintable(f.mFullName), fmt);
    }
    if (fmt < 0 || fmt >= QImage::NImageFormats) {
        qWarning("[hbsplash] Image format is invalid");
        return 0;
    }
    qint64 sz = h * bpl;
    uchar *data = 0;
    if (w > 0 && h > 0 && bpl > 0 && sz > 0 && sz <= image_bytes_limit) {
        try {
            if (params.allocFunc) {
                data = params.allocFunc(w, h, bpl, fmt, params.allocFuncParam);
            } else {
                data = new uchar[sz];
            }
            if (data) {
                qint64 bytesRead = f.read(reinterpret_cast<char *>(data), sz);
                if (bytesRead != sz) {
                    qWarning("[hbsplash] File %s is invalid", qPrintable(f.mFullName));
                    if (!params.allocFunc) {
                        delete[] data;
                    }
                    data = 0;
                }
            }
        } catch (const std::bad_alloc &) {
            qWarning("[hbsplash] Failed to allocate image buffer");
        }
    } else {
        qWarning("[hbsplash] Image in file %s is too big", qPrintable(f.mFullName));
    }
    *params.w = w;
    *params.h = h;
    *params.bpl = bpl;
    *params.fmt = fmt;
    return data;
}

#ifdef Q_OS_SYMBIAN

// Symbian-specific implementation to get splash screens either by
// reading from a file opened on the server side or by using bitmaps
// shared via fbserv.

class HbSplashSrvClient : public RSessionBase
{
public:
    HbSplashSrvClient();
    ~HbSplashSrvClient();
    bool Connect();

    bool getSplashFileHandle(RFile &f,
                             const QString &ori,
                             const QString &appId,
                             const QString &screenId);

    uchar *getSplashFromBitmap(const QString &ori,
                               const QString &appId,
                               const QString &screenId,
                               Params &params);

private:
    RMutex mMutex;
    bool mMutexOk;
};

HbSplashSrvClient::HbSplashSrvClient()
{
    _LIT(KHbSplashStartMutex, "hbsplstart");
    mMutexOk = mMutex.OpenGlobal(KHbSplashStartMutex) == KErrNone;
    if (!mMutexOk) {
        mMutexOk = mMutex.CreateGlobal(KHbSplashStartMutex) == KErrNone;
    }
}

HbSplashSrvClient::~HbSplashSrvClient()
{
    if (mMutexOk) {
        if (mMutex.IsHeld()) {
            mMutex.Signal();
        }
        mMutex.Close();
    }
}

bool HbSplashSrvClient::Connect()
{
    if (mMutexOk) {
        mMutex.Wait();
    }
    TVersion ver(hbsplash_version_major, hbsplash_version_minor, hbsplash_version_build);
    int maxTries = 3;
    bool ok = false;
    while (maxTries--) {
        TInt err = CreateSession(hbsplash_server_name, ver);
        qDebug("[hbsplash] CreateSession result: %d", err);
        if (err == KErrNone) {
            ok = true;
            break;
        } else if (err == KErrNotFound || err == KErrServerTerminated) {
            qDebug("[hbsplash] server not running");
            TFindServer findServer(hbsplash_server_name);
            TFullName name;
            if (findServer.Next(name) != KErrNone) {
                RProcess server;
                const TUidType uid(KNullUid, KNullUid, hbsplash_server_uid3);
                err = server.Create(hbsplash_server_exe, KNullDesC, uid);
                if (err != KErrNone) {
                    qWarning("[hbsplash] RProcess::Create failed (%d)", err);
                    break;
                }
                TRequestStatus status;
                server.Rendezvous(status);
                if (status != KRequestPending) {
                    server.Kill(0);
                } else {
                    server.Resume();
                }
                User::WaitForRequest(status);
                server.Close();
                if (status.Int() != KErrNone && status.Int() != KErrAlreadyExists) {
                    qWarning("[hbsplash] Rendezvous failed (%d)", status.Int());
                    break;
                }
                qDebug("[hbsplash] server started");
            }
        } else {
            break;
        }
    }
    if (!ok) {
        qWarning("[hbsplash] Cannot connect to splashgen server");
    }
    if (mMutexOk) {
        mMutex.Signal();
    }
    return ok;
}

bool HbSplashSrvClient::getSplashFileHandle(RFile &f,
                                            const QString &ori,
                                            const QString &appId,
                                            const QString &screenId)
{
    TPtrC oriDes(static_cast<const TUint16 *>(ori.utf16()), ori.length());
    TPtrC appIdDes(static_cast<const TUint16 *>(appId.utf16()), appId.length());
    TPtrC screenIdDes(static_cast<const TUint16 *>(screenId.utf16()), screenId.length());
    TInt fileHandle;
    TPckg<TInt> fileHandlePckg(fileHandle);
    TIpcArgs args(&oriDes, &appIdDes, &screenIdDes, &fileHandlePckg);
    TInt fsHandle = SendReceive(HbSplashSrvGetSplashFile, args);
    return f.AdoptFromServer(fsHandle, fileHandle) == KErrNone;
}

uchar *HbSplashSrvClient::getSplashFromBitmap(const QString &ori,
                                              const QString &appId,
                                              const QString &screenId,
                                              Params &params)
{
    TPtrC oriDes(static_cast<const TUint16 *>(ori.utf16()), ori.length());
    TPtrC appIdDes(static_cast<const TUint16 *>(appId.utf16()), appId.length());
    TPtrC screenIdDes(static_cast<const TUint16 *>(screenId.utf16()), screenId.length());
    TInt bitmapHandle;
    TPckg<TInt> bitmapHandlePckg(bitmapHandle);
    TIpcArgs args(&oriDes, &appIdDes, &screenIdDes, &bitmapHandlePckg);
    if (SendReceive(HbSplashSrvGetSplashData, args) == KErrNone) {
        QScopedPointer<CFbsBitmap> bmp(new CFbsBitmap);
        if (bmp->Duplicate(bitmapHandle) == KErrNone) {
            TSize size = bmp->SizeInPixels();
            TDisplayMode mode = bmp->DisplayMode();
            if (size.iWidth > 0 && size.iHeight > 0 && mode == EColor16MAP) {
                int bpl = CFbsBitmap::ScanLineLength(size.iWidth, mode);
                const QImage::Format fmt = QImage::Format_ARGB32_Premultiplied;
                int len = bpl * size.iHeight;
                uchar *data = 0;
                try {
                    if (params.allocFunc) {
                        data = params.allocFunc(size.iWidth, size.iHeight,
                                                bpl, fmt, params.allocFuncParam);
                    } else {
                        data = new uchar[len];
                    }
                    if (data) {
                        memcpy(data, bmp->DataAddress(), len);
                        *params.w = size.iWidth;
                        *params.h = size.iHeight;
                        *params.bpl = bpl;
                        *params.fmt = fmt;
                        qDebug("[hbsplash] bitmap data received");
                        return data;
                    }
                } catch (const std::bad_alloc &) {
                    qWarning("[hbsplash] Failed to allocate image buffer");
                }
            } else {
                qWarning("[hbsplash] Invalid bitmap (%d %d %d)",
                         size.iWidth, size.iHeight, mode);
            }
        } else {
            qWarning("[hbsplash] Cannot duplicate bitmap");
        }
    }
    return 0;
}

static uchar *load_symbian(Params &params)
{
    HbSplashSrvClient client;
    if (!client.Connect()) {
        return false;
    }

    QString oriStr(orientationId(params.flags));
    QString appIdStr(params.appId);
    if (appIdStr.isEmpty()) {
        RProcess process;
        appIdStr = QString::number(process.SecureId().iId, 16);
    }

    uchar *data = 0;
    if (!params.forceFile) {
        data = client.getSplashFromBitmap(oriStr, appIdStr, params.screenId, params);
    } else {
        File f;
        f.mFullName = "[unavailable]";
        if (client.getSplashFileHandle(f.mFile, oriStr, appIdStr, params.screenId)) {
            qDebug("[hbsplash] got handle from server");
            data = readSpl(f, params);
            f.mFile.Close();
        }
    }

    client.Close();
    return data;
}

#else

// Generic cross-platform implementation, reads the pixel data
// directly from files. Not suitable for Symbian due to platsec and
// performance reasons.

static uchar *read_file_generic(const QString &name, Params &params)
{
    uchar *data = 0;
    File f;
    f.mFullName = name;
    f.mFile.setFileName(name);
    if (f.mFile.open(QIODevice::ReadOnly)) {
        data = readSpl(f, params);
        f.mFile.close();
    }
    return data;
}

static uchar *load_generic(Params &params)
{
    QString appSpecificName("splash_%1_%2.spl");
    QString appAndScreenSpecificName("splash_%1_%2_%3.spl");
    QString genericName("splash_%1.spl");
    QDir dir("splashscreens"); // see hbsplashgenerator
    QString oriStr(orientationId(params.flags));
    uchar *data = 0;
    // Try to read the app-specific screen.
    if (!params.appId.isEmpty()) {
        QString name;
        if (!params.screenId.isEmpty()) {
            name = appAndScreenSpecificName.arg(oriStr).arg(params.appId).arg(params.screenId);
        } else {
            name = appSpecificName.arg(oriStr).arg(params.appId);
        }
        data = read_file_generic(dir.filePath(name), params);
    }
    // If failed then use the common one.
    if (!data) {
        data = read_file_generic(dir.filePath(genericName.arg(oriStr)), params);
    }
    return data;
}

#endif

/*!
  Tries to find a suitable splash screen and returns the pixel data. The
  ownership of the returned pointer is transferred to the caller.

  When the return value is non-null then \a w, \a h, \a bpl, and \a fmt will be
  set to the width, height, bytes-per-line, format properties of the image data.

  If no suitable splash screens could be loaded then the return value is null.

  Symbian: By default the orientation depends on the current sensor status.

  Non-Symbian platforms: By default the screen for portrait orientation is used.

  \a flags can be used to override certain settings, e.g. to explicitly request
  the splash screen for a given orientation regardless of the platform or HW
  status.

  Symbian only: If \a appId is empty (the default value) then the secure id of the
  current process will be used to look up the application-specific screen.

  Non-Symbian platforms: If \a appId is empty then no application-specific screens
  will be searched.

  All platforms: If \a appId is not empty then it will be used as the application
  id when looking up application-specific screens.

  If the \a allocFunc function pointer is given then it will be called whenever
  a buffer is needed to be allocated.  The function receives the following
  parameters: width, height, bytes-per-line, format, and the custom parameter \a
  allocFuncParam. This means that when allocFunc is not 0 the return value of
  load() is either 0 (if there was an error) or the return value of allocFunc
  (in case of success).

  \sa HbSplashScreen::Flags

  \internal
*/
uchar *HbSplash::load(int &w, int &h, int &bpl, QImage::Format &fmt,
                      HbSplashScreen::Flags flags,
                      const QString &appId, const QString &screenId,
                      AllocFunc allocFunc, void *allocFuncParam)
{
    Params params;
    params.w = &w;
    params.h = &h;
    params.bpl = &bpl;
    params.fmt = &fmt;
    params.flags = flags;
    params.appId = appId;
    params.screenId = screenId;
    params.allocFunc = allocFunc;
    params.allocFuncParam = allocFuncParam;
    params.forceFile = false; // use CFbsBitmap-based sharing on Symbian
#ifdef Q_OS_SYMBIAN
    return load_symbian(params);
#else
    return load_generic(params);
#endif
}

#ifdef Q_OS_SYMBIAN
static uchar *fbsBitmapAllocFunc(int w, int h, int bpl, QImage::Format fmt, void *param)
{
    if (fmt != QImage::Format_ARGB32_Premultiplied) {
        qWarning("[hbsplash] fbsBitmapAllocFunc: unsupported format %d", fmt);
        return 0;
    }
    TDisplayMode mode = EColor16MAP;
    CFbsBitmap *bmp = static_cast<CFbsBitmap *>(param);
    if (bmp->Create(TSize(w, h), mode) == KErrNone) {
        int bmpBpl = CFbsBitmap::ScanLineLength(w, mode);
        if (bpl == bmpBpl) {
            return reinterpret_cast<uchar *>(bmp->DataAddress());
        } else {
            qWarning("[hbsplash] fbsBitmapAllocFunc: bpl mismatch (%d - %d)", bpl, bmpBpl);
        }
    } else {
        qWarning("[hbsplash] fbsBitmapAllocFunc: bitmap Create() failed");
    }
    return 0;
}
#endif

void *HbSplashDirectSymbian::load(void *file, int *extra)
{
#ifdef Q_OS_SYMBIAN
    // Read the data directly into a CFbsBitmap. `file' is assumed to
    // be a ptr to an already Open()'ed RFile.
    QScopedPointer<CFbsBitmap> bmp(new CFbsBitmap);
    int w, h, bpl;
    QImage::Format fmt;
    Params params;
    // Everything is ignored except the alloc-func and its param.
    params.w = &w;
    params.h = &h;
    params.bpl = &bpl;
    params.fmt = &fmt;
    params.flags = HbSplashScreen::Default;
    params.allocFunc = fbsBitmapAllocFunc;
    params.allocFuncParam = bmp.data();
    File f;
    f.mFile = *static_cast<RFile *>(file);
    if (readSpl(f, params)) {
        if (extra) {
            *extra = params.extra;
        }
        return bmp.take();
    }
#else
    Q_UNUSED(file);
    Q_UNUSED(extra);
#endif
    return 0;
}
