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
#include "hbsplashscreen_generic_p.h"
#include "hbsplash_p.h"
#include "hbsplashdefs_p.h"
#include "hbapplication.h"
#include <QPainter>
#include <QApplication>

#if QT_VERSION >= 0x040700
#include <QElapsedTimer>
#define ELAPSED_TIMER QElapsedTimer
#else
#include <QTime>
#define ELAPSED_TIMER QTime
#endif

#if defined(Q_OS_SYMBIAN) && defined(HB_EFFECTS_OPENVG)
// When Symbian/EGL/OpenVG is available we can use the more efficient
// implementation which is not only faster but is able to operate
// without having Qt or any other framework initialized. This means
// having truly 'instant' splash screens.
#define HB_SPLASH_DIRECT_WS
#endif

#ifdef HB_SPLASH_DIRECT_WS
#include "hbsplashscreen_symbian_vg_p.h"
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

  \sa Hb::SplashFixedVertical
*/

/*!
  \var HbSplashScreen::Flag HbSplashScreen::FixedHorizontal

  Indicates that the application will force its orientation to horizontal. As a
  result the splash screen will also be forced to horizontal orientation.

  \sa Hb::SplashFixedHorizontal
*/

/*!
  \var HbSplashScreen::Flag HbSplashScreen::ForceQt

  Forces the usage of the QWidget-based implementation even on platforms where a
  non-Qt based implementation would be available. The Qt-based version requires
  QApplication to be constructed, while non-Qt based ones do not need this and
  therefore they can be launched before instantiating QApplication or
  HbApplication, providing a better startup experience. However some special
  applications may want to stick to the Qt-based version, e.g. because they
  force the usage of the raster graphics system.

  \sa Hb::ForceQtSplash
*/

static HbSplashScreenInterface *splashScreen = 0;

struct RequestProps {
    RequestProps() : mSplashFlags(HbSplashScreen::Default) { }
    HbSplashScreen::Flags mSplashFlags;
    QString mAppId;
    QString mScreenId;
};

Q_GLOBAL_STATIC(RequestProps, requestProps)

// This static function is called when the fw wants to know if the
// splash screen can be launched before constructing QApplication.
bool HbSplashScreenExt::needsQt()
{
#ifdef HB_SPLASH_DIRECT_WS
    return false;
#else
    return true;
#endif
}

// Called (by HbApplication) when QApplication is constructed and the splash was
// launched before that. This gives a chance for the splash screen to do
// activies that need an active scheduler or a QApplication instance.
void HbSplashScreenExt::doQtPhase()
{
    // Can also be called when the splash screen was not started or is already
    // destroyed, do nothing in such cases.
    if (splashScreen) {
        splashScreen->doQtPhase();
    }
}

/*!
  Creates and shows the splash screen, if a suitable one is available for the
  current application. The splash screen is automatically destroyed by
  HbMainWindow after the window has become fully visible.

  The generic splash implementation is QWidget-based and thus needs to have the
  QApplication instance created before calling this function. On the other hand
  for Symbian there may be a Qt-independent implementation which means that
  this function can be called before constructing HbApplication or QApplication,
  i.e. right after entering main().
  
  Normally there is no need to worry about the differences because HbApplication
  is aware of this and will call this function at the most ideal time, depending
  on the underlying implementation. However when start() is called directly by
  the application, this becomes important because the application startup
  experience can be greatly improved by making sure the splash is shown as early
  as possible.

  On Symbian the splash will be automatically suppressed (i.e. not shown) if the
  application was started to background, that is,
  HbApplication::startedToBackground() returns true. To override this default
  behavior, pass HbSplashScreen::ShowWhenStartingToBackground.

  Passing argc and argv is optional, however if they are omitted,
  QCoreApplication must have already been instantiated. If neither argc and argv
  are specified nor QCoreApplication is available, the automatic splash
  suppression will not be available.
 */
void HbSplashScreen::start(Flags flags, int argc, char *argv[])
{
    Flags realFlags = flags | requestProps()->mSplashFlags;
    if (!realFlags.testFlag(ShowWhenStartingToBackground)
        && HbApplication::startedToBackground(argc, argv)) {
        splDebug("[hbsplash] app started to background, suppressing splash");
        return;
    }
    if (!splashScreen) {
#ifdef HB_SPLASH_DIRECT_WS
        // Use the Qt-based impl even on Symbian when it is forced or when there
        // is already a QApplication instance.
        if (realFlags.testFlag(ForceQt) || QApplication::instance()) {
            splashScreen = new HbSplashScreenGeneric;
        } else {
            // Ideal case: Use the native splash.
            splashScreen = new HbSplashScreenSymbianVg;
        }
#else
        splashScreen = new HbSplashScreenGeneric;
#endif
    }
    splashScreen->start(realFlags);
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

const int auto_stop_interval = 10000; // 10 sec

HbSplashScreenGeneric::HbSplashScreenGeneric()
    : QWidget(0, Qt::SplashScreen), mImageData(0)
{
}

HbSplashScreenGeneric::~HbSplashScreenGeneric()
{
    if (mImageData) {
        splDebug("[hbsplash] destroying splash screen");
        delete mImageData;
    }
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
                mContents = QImage(mImageData, w, h, bpl, fmt);
                resize(mContents.size());
            }
        }
        if (!mContents.isNull()) {
            splDebug("[hbsplash] splash screen initialized");

#ifdef Q_OS_SYMBIAN
            showFullScreen(); // krazy:exclude=qmethods
#else
            show();
#endif

            QApplication::processEvents();
            QApplication::flush();

            // The splash screen must be destroyed automatically when
            // loosing foreground.
            if (QApplication::instance()) {
                QApplication::instance()->installEventFilter(this);
            }

            // The splash screen must be destroyed automatically after
            // a certain amount of time.
            mTimerId = startTimer(auto_stop_interval);
        }
    } catch (const std::bad_alloc &) {
    }
}

void HbSplashScreenGeneric::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawImage(QPointF(0, 0), mContents);
}

void HbSplashScreenGeneric::repaint()
{
    QWidget::repaint();
    QApplication::flush();
}

void HbSplashScreenGeneric::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mTimerId) {
        qWarning("[hbsplash] timeout while splash screen is active");
        deleteLater();
        splashScreen = 0;
    } else {
        QWidget::timerEvent(event);
    }
}

bool HbSplashScreenGeneric::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ApplicationDeactivate) {
        qWarning("[hbsplash] foreground lost while splash screen is active");
        deleteLater();
        splashScreen = 0;
    }
    return QWidget::eventFilter(obj, event);
}

// Symbian (wserv + egl + openvg) implementation.

// Unlike the generic version this works also when used
// before constructing the QApplication instance.

// No mw layer frameworks (cone, uikon) must be used here
// because they may not be initialized at all and doing
// initialization here would interfere with Qt.

#ifdef HB_SPLASH_DIRECT_WS

HbSplashScreenSymbianVg::HbSplashScreenSymbianVg()
    : mInited(false), mImageData(0), mTimer(0)
{
}

HbSplashScreenSymbianVg::~HbSplashScreenSymbianVg()
{
    delete mImageData;
    delete mTimer;
    if (mInited) {
#ifdef HB_SPLASH_VERBOSE_LOGGING
        RDebug::Printf("[hbsplash] destroying splash screen");
#endif
        if (eglGetCurrentContext() == mContext
            || eglGetCurrentSurface(EGL_READ) == mSurface
            || eglGetCurrentSurface(EGL_DRAW) == mSurface)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        }
        eglDestroySurface(mDisplay, mSurface);
        eglDestroyContext(mDisplay, mContext);
        delete mScr;
        mGroup.Close();
        mWs.Close();
    }
}

void HbSplashScreenSymbianVg::release()
{
    delete this;
}

bool HbSplashScreenSymbianVg::init()
{
    // This is typically called before initializing anything, meaning
    // there is no active scheduler, cleanup stack, cone, etc.

#ifdef HB_SPLASH_VERBOSE_LOGGING
    ELAPSED_TIMER t;
    t.start();
#endif

    if (mWs.Connect() != KErrNone) { // also connects to fbserv
        return false;
    }
    try {
        mScr = new CWsScreenDevice(mWs);
    } catch (const std::bad_alloc &) {
        mWs.Close();
        return false;
    }
    mScr->Construct();
    mGroup = RWindowGroup(mWs);
    mGroup.Construct(1, ETrue);
    mWin = RWindow(mWs);
    mWin.Construct(mGroup, 2);
    mWin.SetExtent(TPoint(0, 0), mScr->SizeInPixels());
    mWin.EnableVisibilityChangeEvents();

#ifdef HB_SPLASH_VERBOSE_LOGGING
    RDebug::Printf("[hbsplash] wserv init took %d ms", (int) t.restart());
#endif

    // Do not return failure when egl/openvg calls fail. Let it go, we
    // just won't see the splash in such a case, and that's fine.
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(mDisplay, 0, 0);
    eglBindAPI(EGL_OPENVG_API);
    EGLConfig config;
    EGLint numConfigs;
    const EGLint attribList[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    eglChooseConfig(mDisplay, attribList, &config, 1, &numConfigs);
    mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, 0);
    if (mContext == EGL_NO_CONTEXT) {
        RDebug::Printf("[hbsplash] eglCreateContext failed (%d)", eglGetError());
    }
    const EGLint attribList2[] = {
        EGL_VG_ALPHA_FORMAT, EGL_VG_ALPHA_FORMAT_PRE,
        EGL_NONE
    };
    mSurface = eglCreateWindowSurface(mDisplay, config,
                                      (EGLNativeWindowType)(&mWin), attribList2);
    if (mSurface == EGL_NO_SURFACE) {
        RDebug::Printf("[hbsplash] eglCreateWindowSurface failed (%d)", eglGetError());
    }

#ifdef HB_SPLASH_VERBOSE_LOGGING
    RDebug::Printf("[hbsplash] egl+openvg init took %d ms", (int) t.elapsed());
#endif

    return true;
}

void HbSplashScreenSymbianVg::start(HbSplashScreen::Flags flags)
{
    if (!mImageData) {
        int w, h, bpl;
        QImage::Format fmt;
        RequestProps *props = requestProps();
#ifdef HB_SPLASH_VERBOSE_LOGGING
        ELAPSED_TIMER t;
        t.start();
#endif
        // Start loading the splash screen data but don't wait until it's
        // done. Instead, move on to graphics initialization.
        uchar *asyncHandle = HbSplash::load(w, h, bpl, fmt, flags,
                                            props->mAppId, props->mScreenId,
                                            0, 0, true);
#ifdef HB_SPLASH_VERBOSE_LOGGING
        RDebug::Printf("[hbsplash] async load start took %d ms", (int) t.restart());
#endif

        if (!mInited) {
            if (init()) {
                mInited = true;
            } else {
                RDebug::Printf("[hbsplash] init() failed");
                return;
            }
        }

#ifdef HB_SPLASH_VERBOSE_LOGGING
        t.restart();
#endif
        // The image data and its properties will shortly be needed so retrieve
        // the result of the load operation.
        mImageData = HbSplash::finishAsync(asyncHandle);
#ifdef HB_SPLASH_VERBOSE_LOGGING
        RDebug::Printf("[hbsplash] finishAsync() took %d ms", (int) t.restart());
#endif

        if (mImageData) {
            eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
            VGImage img = vgCreateImage(VG_sARGB_8888_PRE, w, h, VG_IMAGE_QUALITY_FASTER);
            if (img != VG_INVALID_HANDLE) {
                vgImageSubData(img, mImageData, bpl, VG_sARGB_8888_PRE, 0, 0, w, h);
            } else {
                RDebug::Printf("[hbsplash] vgCreateImage failed (%d)", vgGetError());
            }

#ifdef HB_SPLASH_VERBOSE_LOGGING
            RDebug::Printf("[hbsplash] image init took %d ms", (int) t.restart());
#endif

            mWin.Activate();
            mWin.Invalidate();
            mWin.BeginRedraw();

#ifdef HB_SPLASH_VERBOSE_LOGGING
            RDebug::Printf("[hbsplash] redraw init took %d ms", (int) t.restart());
#endif

            vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
            VGfloat mat[9];
            mat[0] = 1.0f;
            mat[1] = 0.0f;
            mat[2] = 0.0f;
            mat[3] = 0.0f;
            mat[4] = -1.0f;
            mat[5] = 0.0f;
            mat[6] = 0.0f;
            mat[7] = h;
            mat[8] = 1.0f;
            vgLoadMatrix(mat);
            vgDrawImage(img);
            vgDestroyImage(img);
            eglSwapBuffers(mDisplay, mSurface);

            mWin.EndRedraw();
            mWs.Flush();

#ifdef HB_SPLASH_VERBOSE_LOGGING
            RDebug::Printf("[hbsplash] drawing took %d ms", (int) t.elapsed());
#endif

            bool isQtAvailable = QApplication::instance() != 0;
#ifdef HB_SPLASH_VERBOSE_LOGGING
            RDebug::Printf("[hbsplash] qapplication present: %d", isQtAvailable);
#endif
            if (isQtAvailable) {
                doQtPhase();
            }
            // If there is no QApplication then there is no active scheduler,
            // cone, etc. either so defer the creation of the timer and the
            // visibility listener. doQtPhase() will be called later by
            // HbApplication when QApplication is constructed. If HbApplication
            // is not used at all and start() is called before instantiating
            // QApplication then doQtPhase() is never called but there is not
            // much we can do and it is not mandatory anyway.

        } else {
            HbSplashScreen::destroy();
        }
    }
}

TInt HbSplashScreenSymbianVg::timerCallback(TAny *param)
{
    HbSplashScreenSymbianVg *self = static_cast<HbSplashScreenSymbianVg *>(param);
    self->mTimer->Cancel();
    RDebug::Printf("[hbsplash] timeout while splash screen is active");
    HbSplashScreen::destroy();
    return 0;
}

void HbSplashScreenSymbianVg::doQtPhase()
{
#ifdef HB_SPLASH_VERBOSE_LOGGING
    RDebug::Printf("[hbsplash] HbSplashScreenSymbianVg::doQtPhase()");
#endif
    // Now there is an active scheduler.
    if (!mTimer) {
        TRAPD(err, mTimer = CPeriodic::NewL(CActive::EPriorityStandard));
        if (err == KErrNone) {
            TTimeIntervalMicroSeconds32 iv = auto_stop_interval * 1000;
            mTimer->Start(iv, iv, TCallBack(timerCallback, this));
        }
    }
    // Here we could start a listener for visibility events or similar. But it
    // is better not to, because we may get an ENotVisible event (in theory)
    // even when the incoming window has not yet got any real content so the
    // splash cannot be destroyed at that stage.
}

#endif // HB_SPLASH_DIRECT_WS
