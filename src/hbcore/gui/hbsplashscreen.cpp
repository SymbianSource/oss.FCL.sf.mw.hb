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

#include "hbsplashscreen.h"
#include "hbsplash_p.h"
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
  @stable
  @hbcore
  \class HbSplashScreen

  \brief Shows a splash screen suitable for the current application.

  Normally start() and destroy() are invoked by HbApplication and HbMainWindow
  so applications do not have to care about this class at all. However if
  HbApplication is not used then it may be necessary to call start() manually,
  very early in the application's main() function.
*/

/*!
  \enum HbSplashScreen::Flag

  Flags controlling the splash screen.
*/

/*!
  \var HbSplashScreen::Flag HbSplashScreen::Default

  Default value for splash screen flags. By default the orientation for the
  splash screen is determined based on the current device orientation
  (e.g. based on sensor data).
*/

/*!
  \var HbSplashScreen::Flag HbSplashScreen::FixedVertical

  Indicates that the application will force its orientation to vertical. As a
  result the splash screen will also be forced to vertical orientation.
*/

/*!
  \var HbSplashScreen::Flag HbSplashScreen::FixedHorizontal

  Indicates that the application will force its orientation to horizontal. As a
  result the splash screen will also be forced to horizontal orientation.
*/

class HbSplashScreenInterface
{
public:
    virtual ~HbSplashScreenInterface() {}
    virtual void start(HbSplashScreen::Flags flags) = 0;
    virtual void release() = 0;
};

class HbSplashScreenGeneric : public QWidget, public HbSplashScreenInterface
{
public:
    HbSplashScreenGeneric();
    ~HbSplashScreenGeneric();

    void start(HbSplashScreen::Flags flags);
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

    void start(HbSplashScreen::Flags flags);
    void release();

private:
    void Draw(const TRect &rect) const;

    CFbsBitmap *mContents;
};

#endif // HB_SPLASH_USE_SYMBIAN_LEGACY_SURFACE

static HbSplashScreenInterface *splashScreen = 0;

struct RequestProps {
    RequestProps() : mSplashFlags(HbSplashScreen::Default) { }
    HbSplashScreen::Flags mSplashFlags;
    QString mAppId;
    QString mScreenId;
};

Q_GLOBAL_STATIC(RequestProps, requestProps)

/*!
  Creates and shows the splash screen, if a suitable one is available for the
  current application. The splash screen is automatically destroyed by
  HbMainWindow after the window has become fully visible.
 */
void HbSplashScreen::start(Flags flags)
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
    splashScreen->start(flags | requestProps()->mSplashFlags);
}

/*!
    Hides and destroys the splash screen.  Has no effect if the splash screen
    has not been started.  This is called automatically by HbMainWindow after
    the window is fully constructed and visible.
 */
void HbSplashScreen::destroy()
{
    if (splashScreen) {
        splashScreen->release();
        splashScreen = 0;
    }
}

/*!
    Returns true if the splash screen exists (i.e. start(), but not destroy(),
    has been called)
 */
bool HbSplashScreen::exists()
{
    return splashScreen != 0;
}

/*!
  Sets the flags that will be combined with the flags passed to start().  If
  start() is called directly by the application then this function is not needed
  because the flags can be passed directly to start(). However if start() is not
  called by the application itself then, without this function, the application
  would have no way to pass flags to it. Hence the existence of this function.
 */
void HbSplashScreen::setFlags(Flags flags)
{
    requestProps()->mSplashFlags = flags;
}

/*!
  Overrides the application id. If used then this must be called before
  instantiating HbApplication or calling start().

  No prefixes are allowed, e.g. on Symbian \a appid is typically a secure id,
  however the 0x prefix must not be used here.

  On Symbian the request will typically be ignored because in general it is not
  allowed to use another application's splash screen.

  If not set then appid defaults to an empty string, the interpretation of which
  depends on the platform. For example on Symbian the current process' secure id
  will be used (which will match splash screens for which the splashml document
  had specified the same uid in the appid element).

  \sa setScreenId()
 */
void HbSplashScreen::setAppId(const QString &appId)
{
    requestProps()->mAppId = appId;
}

/*!
  Sets the requested screen id. If a screen id need to be specified then this
  function must be called before instantiating HbApplication or before invoking
  start().

  Splash screens are normally identified based on the appid, however if one
  application needs more than one screen (specific to that application) then the
  screenid can be used to distinguish between the different screens. This string
  will then be matched to the screenid element text from the splashml documents
  in order to find the proper splash screen.

  If not set then the screenid defaults to an empty string that matches splash
  screens for which the screenid was not specified in the splashml document.

  If set but no matching screen is found then the generic splash screen will be
  used.

  \sa setAppId()
 */
void HbSplashScreen::setScreenId(const QString &screenId)
{
    requestProps()->mScreenId = screenId;
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

void HbSplashScreenGeneric::start(HbSplashScreen::Flags flags)
{
    try {
        if (!mImageData) {
            int w, h, bpl;
            QImage::Format fmt;
            RequestProps *props = requestProps();
            mImageData = HbSplash::load(w, h, bpl, fmt, flags,
                                        props->mAppId, props->mScreenId);
            if (mImageData) {
                QImage img(mImageData, w, h, bpl, fmt);
                mContents = QPixmap::fromImage(img);
                resize(mContents.size());
            }
        }
        if (!mContents.isNull()) {
#ifdef Q_OS_SYMBIAN
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

void HbSplashScreenSymbian::start(HbSplashScreen::Flags flags)
{
    try {
        if (!mContents) {
            mContents = new CFbsBitmap;
            int w, h, bpl;
            QImage::Format fmt;
            RequestProps *props = requestProps();
            if (HbSplash::load(w, h, bpl, fmt, flags,
                               props->mAppId, props->mScreenId,
                               fbsBitmapAllocFunc, mContents))
            {
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
