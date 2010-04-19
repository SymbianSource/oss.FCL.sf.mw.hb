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

#include "hbsplashscreen_p.h"
#include <QPainter>
#include <QImage>
#include <QApplication>
#include <QWidget>
#include <QPixmap>

// To play nice with GPU resources it may be beneficial to avoid using QWidget
// for showing the splash screen. (each top-level widget results in creating a
// new window surface which consumes gpu memory) Instead, we can create & show a
// CCoeControl which draws using the traditional Symbian GC methods. (And thus
// uses the "legacy" surface which is still available currently. However if some
// day it is removed then this solution will not work anymore.)
#ifdef Q_OS_SYMBIAN
// Do not enable for now, may cause some flickering.
// The system transition effects may not like this solution anyway.
//#define HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE
#endif

#ifdef HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE
#include <coecntrl.h>
#include <fbs.h>
#include <w32std.h>
#endif

/*!
  \class HbSplashScreen

  \brief Shows a splash screen suitable for the current application, if available.

  \internal
*/

class HbSplashScreenInterface
{
public:
    virtual void start(HbSplash::Flags flags) = 0;
    virtual void release() = 0;
};

class HbSplashScreenGeneric : public QWidget, public HbSplashScreenInterface
{
public:
    HbSplashScreenGeneric();
    ~HbSplashScreenGeneric();

    void start(HbSplash::Flags flags);
    void release();

private:
    void paintEvent(QPaintEvent *event);
    void repaint();

    uchar *mImageData;
    QPixmap mContents;
};

#ifdef HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE

class HbSplashScreenSymbian : public CCoeControl, public HbSplashScreenInterface
{
public:
    HbSplashScreenSymbian();
    ~HbSplashScreenSymbian();

    void start(HbSplash::Flags flags);
    void release();

private:
    void Draw(const TRect &rect) const;

    CFbsBitmap *mContents;
};

#endif // HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE

static HbSplashScreenInterface *splashScreen = 0;

void HbSplashScreen::start(HbSplash::Flags flags)
{
    if (!splashScreen) {
        splashScreen =
#ifdef HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE
            new HbSplashScreenSymbian
#else
            new HbSplashScreenGeneric
#endif
            ;
    }
    splashScreen->start(flags);
}

void HbSplashScreen::destroy()
{
    if (splashScreen) {
        splashScreen->release();
        splashScreen = 0;
    }
}

bool HbSplashScreen::exists()
{
    return splashScreen != 0;
}

HbSplashScreenGeneric::HbSplashScreenGeneric()
    : QWidget(0, Qt::SplashScreen), mImageData(0)
{
}

HbSplashScreenGeneric::~HbSplashScreenGeneric()
{
    delete mImageData;
}

void HbSplashScreenGeneric::release()
{
    delete this;
}

void HbSplashScreenGeneric::start(HbSplash::Flags flags)
{
    try {
        if (!mImageData) {
            int w, h, bpl;
            QImage::Format fmt;
            mImageData = HbSplash::load(w, h, bpl, fmt, flags);
            if (mImageData) {
                QImage img(mImageData, w, h, bpl, fmt);
                mContents = QPixmap::fromImage(img);
                resize(mContents.size());
            }
        }
        if (!mContents.isNull()) {
#ifdef HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE
            showFullScreen();
#else
            show();
#endif
            QApplication::processEvents();
            QApplication::flush();
        }
    } catch (const std::bad_alloc &) {
    }
}

void HbSplashScreenGeneric::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(QPointF(0, 0), mContents);
}

void HbSplashScreenGeneric::repaint()
{
    QWidget::repaint();
    QApplication::flush();
}

#ifdef HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE

HbSplashScreenSymbian::HbSplashScreenSymbian()
    : mContents(0)
{
}

HbSplashScreenSymbian::~HbSplashScreenSymbian()
{
    delete mContents;
}

void HbSplashScreenSymbian::release()
{
    delete this;
}

static uchar *fbsBitmapAllocFunc(int w, int h, int bpl, QImage::Format fmt, void *param)
{
    if (fmt != QImage::Format_ARGB32_Premultiplied) {
        qWarning("HbSplash: fbsBitmapAllocFunc: unsupported format %d", fmt);
        return 0;
    }
    TDisplayMode mode = EColor16MAP;
    CFbsBitmap *bmp = static_cast<CFbsBitmap *>(param);
    if (bmp->Create(TSize(w, h), mode) == KErrNone) {
        int bmpBpl = CFbsBitmap::ScanLineLength(w, mode);
        if (bpl == bmpBpl) {
            return reinterpret_cast<uchar *>(bmp->DataAddress());
        } else {
            qWarning("HbSplash: fbsBitmapAllocFunc: bpl mismatch (%d - %d)", bpl, bmpBpl);
        }
    } else {
        qWarning("HbSplash: fbsBitmapAllocFunc: bitmap Create() failed");
    }
    return 0;
}

void HbSplashScreenSymbian::start(HbSplash::Flags flags)
{
    try {
        if (!mContents) {
            mContents = new CFbsBitmap;
            int w, h, bpl;
            QImage::Format fmt;
            if (HbSplash::load(w, h, bpl, fmt, flags, QString(), fbsBitmapAllocFunc, mContents)) {
                TRect rect(TPoint(0, 0), TSize(w, h));
                TRAPD(err, {
                        CreateWindowL();
                        RWindow *window = static_cast<RWindow *>(DrawableWindow());
                        window->SetSurfaceTransparency(ETrue);
                        SetRect(rect);
                        ActivateL(); });
                if (err == KErrNone) {
                    MakeVisible(ETrue);
                    DrawNow();
                } else {
                    qWarning("HbSplash: symbian control init failed (%d)", err);
                }
            } else {
                delete mContents;
                mContents = 0;
            }
        }
    } catch (const std::bad_alloc &) {
    }
}

void HbSplashScreenSymbian::Draw(const TRect &rect) const
{
    Q_UNUSED(rect);
    if (mContents) {
        SystemGc().BitBlt(TPoint(0, 0), mContents);
    }
}

#endif // HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE
