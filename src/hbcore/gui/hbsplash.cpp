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
#include <QDir>
#include <QFile>
#include <QTime>

#ifdef Q_OS_SYMBIAN
#include <e32std.h>
#include <f32file.h>
#include "hborientationstatus_p.h"
#include "hbsplashdefs_p.h"
#endif

/*!
  \class HbSplash
  
  \brief Class with utility functions for accessing splash screens.

  \internal
*/

const qint64 image_bytes_limit = 1024 * 1024 * 4;

static QString orientationId(HbSplash::Flags flags)
{
    if (flags & HbSplash::FixedVertical) {
        return QString("prt");
    } else if (flags & HbSplash::FixedHorizontal) {
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
    HbSplash::Flags flags;
    QString appId;
    HbSplash::AllocFunc allocFunc;
    void *allocFuncParam;
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

static uchar *readSpl(File &f, const Params &params)
{
    int w = 0, h = 0, bpl = 0;
    QImage::Format fmt = QImage::Format_Invalid;
    f.read((char *) &w, sizeof(int));
    f.read((char *) &h, sizeof(int));
    f.read((char *) &bpl, sizeof(int));
    f.read((char *) &fmt, sizeof(QImage::Format));
    if (fmt != QImage::Format_ARGB32_Premultiplied) {
        qWarning("HbSplash: image format for %s is not ARGB32_PRE (is %d instead)",
                 qPrintable(f.mFullName), fmt);
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
                qint64 bytesRead = f.read((char *) data, sz);
                if (bytesRead != sz) {
                    qWarning("HbSplash: file %s is invalid", qPrintable(f.mFullName));
                    if (!params.allocFunc) {
                        delete data;
                    }
                    data = 0;
                }
            }
        } catch (const std::bad_alloc &) {
            qWarning("HbSplash: failed to allocate image buffer");
        }
    } else {
        qWarning("HbSplash: image in file %s is too big", qPrintable(f.mFullName));
    }
    *params.w = w;
    *params.h = h;
    *params.bpl = bpl;
    *params.fmt = fmt;
    return data;
}

#ifdef Q_OS_SYMBIAN

class HbSplashSrvClient : public RSessionBase
{
public:
    bool Connect();
    bool getSplash(RFile &f, const QString &ori, const QString &appId);
};

bool HbSplashSrvClient::Connect()
{
    TVersion ver(hbsplash_version_major, hbsplash_version_minor, hbsplash_version_build);
    if (CreateSession(hbsplash_server_name, ver) != KErrNone) {
        qWarning("[hbsplash] cannot connect to splashgen server");
        return false;
    }
    return true;
}

bool HbSplashSrvClient::getSplash(RFile &f, const QString &ori, const QString &appId)
{
    TPtrC oriDes(static_cast<const TUint16 *>(ori.utf16()), ori.length());
    TPtrC appIdDes(static_cast<const TUint16 *>(appId.utf16()), appId.length());
    TInt fileHandle;
    TPckg<TInt> fileHandlePckg(fileHandle);
    TIpcArgs args(&oriDes, &appIdDes, &fileHandlePckg);
    TInt fsHandle = SendReceive(HbSplashSrvGetSplash, args);
    return f.AdoptFromServer(fsHandle, fileHandle) == KErrNone;
}

static uchar *load_symbian(const Params &params)
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
    File f;
    f.mFullName = "[unavailable]";
    if (client.getSplash(f.mFile, oriStr, appIdStr)) {
        qDebug("[hbsplash] got handle from server");
        data = readSpl(f, params);
        f.mFile.Close();
    }

    client.Close();
    return data;
}

#else

static uchar *read_file_generic(const QString &name, const Params &params)
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

static uchar *load_generic(const Params &params)
{
    QString appSpecificName("splash_%1_%2.spl");
    QString genericName("splash_%1.spl");
    QDir dir("splashscreens"); // see hbsplashgenerator
    QString oriStr(orientationId(params.flags));
    uchar *data = 0;
    // Try to read the app-specific screen.
    if (!params.appId.isEmpty()) {
        data = read_file_generic(dir.filePath(appSpecificName.arg(oriStr).arg(params.appId)), params);
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

  \sa HbSplash::Flags

  \internal
*/
uchar *HbSplash::load(int &w, int &h, int &bpl, QImage::Format &fmt,
                      Flags flags, const QString &appId,
                      AllocFunc allocFunc, void *allocFuncParam)
{
    Params params;
    params.w = &w;
    params.h = &h;
    params.bpl = &bpl;
    params.fmt = &fmt;
    params.flags = flags;
    params.appId = appId;
    params.allocFunc = allocFunc;
    params.allocFuncParam = allocFuncParam;
#ifdef Q_OS_SYMBIAN
    return load_symbian(params);
#else
    return load_generic(params);
#endif
}
