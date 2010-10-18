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

#include "hbglobalstatusbar_r.h"
#include "hbsplashdefs_p.h"
#include <QScopedPointer>

/*!
  \class HbGlobalStatusBar

  \brief Provides access and change notifications for the image (screenshot) of
  the statusbar rendered in background by the hbsplashgenerator process.

  \internal
*/

#ifdef Q_OS_SYMBIAN
#include <e32base.h>
#include <fbs.h>
#include <e32property.h>

class BitmapHandleWatcher : public CActive
{
public:
    BitmapHandleWatcher(HbSplashStatusBarKeys key, TCallBack callback);
    ~BitmapHandleWatcher();
    void DoCancel();
    void RunL();
    TInt value();

private:
    RProperty mProperty;
    TCallBack mCallback;
};

BitmapHandleWatcher::BitmapHandleWatcher(HbSplashStatusBarKeys key, TCallBack callback)
    : CActive(EPriorityStandard)
{
    CActiveScheduler::Add(this);
    if (mProperty.Attach(hbsplash_server_uid3, key) == KErrNone) {
        mCallback = callback;
        mProperty.Subscribe(iStatus);
        SetActive();
    }
}

BitmapHandleWatcher::~BitmapHandleWatcher()
{
    Cancel();
    mProperty.Close();
}

void BitmapHandleWatcher::DoCancel()
{
    mProperty.Cancel();
}

void BitmapHandleWatcher::RunL()
{
    if (iStatus != KErrCancel) {
        mProperty.Subscribe(iStatus);
        SetActive();
        if (mCallback.iFunction) {
            mCallback.CallBack();
        }
    }
}

TInt BitmapHandleWatcher::value()
{
    TInt v = 0;
    mProperty.Get(v);
    return v;
}

class HbGlobalStatusBarPrivate
{
public:
    HbGlobalStatusBarPrivate(HbGlobalStatusBar *q);
    ~HbGlobalStatusBarPrivate();
    static TInt splashImageChanged(TAny *param);
    TInt bitmapHandle(Qt::Orientation orientation);
    CFbsBitmap **storedBitmap(Qt::Orientation orientation);

    CFbsBitmap *mBmpPrt;
    CFbsBitmap *mBmpLsc;
    QScopedPointer<BitmapHandleWatcher> mPrt;
    QScopedPointer<BitmapHandleWatcher> mLsc;
};

HbGlobalStatusBarPrivate::HbGlobalStatusBarPrivate(HbGlobalStatusBar *q)
    : mBmpPrt(0), mBmpLsc(0)
{
    // Don't emit changed() twice, it's enough to have one callback because both
    // statusbars are regenerated for every change. Note however that passing
    // TCallBack() would cause a compiler warning with armcc because iPtr is
    // left uninitialized, so specify 0 explicitly.
    mPrt.reset(new BitmapHandleWatcher(HbSplashSbBitmapPrtKey, TCallBack(0, 0)));
    mLsc.reset(new BitmapHandleWatcher(HbSplashSbBitmapLscKey, TCallBack(splashImageChanged, q)));
}

HbGlobalStatusBarPrivate::~HbGlobalStatusBarPrivate()
{
    delete mBmpPrt;
    delete mBmpLsc;
}

TInt HbGlobalStatusBarPrivate::splashImageChanged(TAny *param)
{
    emit static_cast<HbGlobalStatusBar *>(param)->changed();
    return 0;
}

TInt HbGlobalStatusBarPrivate::bitmapHandle(Qt::Orientation orientation)
{
    return orientation == Qt::Vertical ? mPrt->value() : mLsc->value();
}

CFbsBitmap **HbGlobalStatusBarPrivate::storedBitmap(Qt::Orientation orientation)
{
    return orientation == Qt::Vertical ? &mBmpPrt : &mBmpLsc;
}

#endif

HbGlobalStatusBar::HbGlobalStatusBar(QObject *parent)
    : QObject(parent), d(0)
{
#ifdef Q_OS_SYMBIAN
    d = new HbGlobalStatusBarPrivate(this);
#endif
}

HbGlobalStatusBar::~HbGlobalStatusBar()
{
#ifdef Q_OS_SYMBIAN
    delete d;
#endif
}

QImage HbGlobalStatusBar::getImage(Qt::Orientation orientation)
{
#ifdef Q_OS_SYMBIAN
    QImage result;
    TInt handle = d->bitmapHandle(orientation);
    CFbsBitmap **store = d->storedBitmap(orientation);
    if (!*store || (*store)->Handle() != handle) {
        delete *store;
        *store = 0;
        CFbsBitmap *bmp = new CFbsBitmap;
        if (bmp->Duplicate(handle) == KErrNone) {
            *store = bmp;
        } else {
            qWarning("HbGlobalStatusBar: Duplicate() failed for handle %d", handle);
            delete bmp;
        }
    }
    if (*store) {
        CFbsBitmap *bmp = *store;
        // Make a deep copy because the bitmap content may change at any time.
        uchar *src = reinterpret_cast<uchar *>(bmp->DataAddress());
        TSize sz = bmp->SizeInPixels();
        int srcBpl = CFbsBitmap::ScanLineLength(sz.iWidth, bmp->DisplayMode());
        QImage image(sz.iWidth, sz.iHeight, QImage::Format_ARGB32_Premultiplied);
        uchar *dst = image.bits();
        if (srcBpl == image.bytesPerLine() && bmp->DisplayMode() == EColor16MAP) {
            qMemCopy(dst, src, sz.iHeight * image.bytesPerLine());
            result = image;
        } else {
            qWarning("HbGlobalStatusBar: Bitmap mismatch");
        }
    }
    return result;
#else
    Q_UNUSED(orientation);
    return QImage();
#endif
}
