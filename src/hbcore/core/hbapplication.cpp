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

#include "hbapplication.h"
#include "hbapplication_p.h"
#include "hbsplashscreen.h"
#include "hbsplashscreen_generic_p.h"
#include "hbactivitymanager.h"
#include "hbinputcontextproxy_p.h"

#if defined(Q_OS_SYMBIAN)
#include <qwindowsstyle.h>
#include <qsymbianevent.h>
#include <e32debug.h>
#endif // Q_OS_SYMBIAN

#ifdef HB_GESTURE_FW
#include "hbgesturerecognizers_p.h"
#endif

#if QT_VERSION >= 0x040700
#include <QElapsedTimer>
#define ELAPSED_TIMER QElapsedTimer
#else
#include <QTime>
#define ELAPSED_TIMER QTime
#endif

//#define HBAPP_LOGGING

/*!
    @stable
    @hbcore
    \class HbApplication
    \brief The HbApplication class is a place for common functionality.

    HbApplication class is a place for common functionality.

    In each and every application using LibHb, the application object is
    always instantiated in main() before creating any GUI controls. Later
    when the GUI controls have been created and shown, the control is given
    to the main event loop of the application:

    \dontinclude graphicsitemdemo/main.cpp
    \skip int main(
    \until HbMainWindow
    \skip show()
    \until }

    Unless the Hb::NoSplash flag is passed, the HbApplication constructor will
    try to show a suitable splash screen for the application. On some platforms
    there will be no splash screens available at all and thus nothing will be
    shown.

    Note that even when an application uses QApplication instead of
    HbApplication, the splash screen can still be started/stopped manually via
    the HbSplashScreen class.

    If the application needs knowledge to decide whether to use splash screens
    (or which screen id to set via HbSpashScreen::setScreenId()) and this
    knowledge can only be acquired after QApplication is constructed
    (typical with service frameworks) then the standard pattern is to pass
    Hb::NoSplash to the HbApplication constructor and call
    HbSplashScreen::start() manually later.

    Note however that this will degrade the startup experience because in an
    ideal case the HbApplication constructor is able to show the splash before
    even executing the QApplication constructor. When passing Hb::NoSplash this
    will not happen and the splash is only shown after spending a possibly
    substantial time on initializing Qt and other frameworks. So use
    Hb::NoSplash only when absolutely needed (or when the splash screen will not
    be shown at all).

    On Symbian the splash will be automatically suppressed (i.e. not shown) if
    the application is started to background, that is,
    HbApplication::startedToBackground() returns true. To override this default
    behavior, pass Hb::ShowSplashWhenStartingToBackground to the HbApplication
    constructor.

    Applications that support the 'activities' concept may check the start-up
    reason like this:

    \code
    HbApplication app(argc, argv);
    if(app.activateReason() == HbApplication::activity) {
        // start-up case
    } else if (app.activateReason() == HbApplication::service) {
        // service launch
    } else {
        // normal launch
    }

    MyActivitiesEngine logic;
    // connect to the application signal
    QObject::connect(&app, SIGNAL(activate()), &logic, SLOT(openActivity()));
    \endcode

    When new activity needs to be activated signal is emitted. Application might
    observe it and start correct handling to return to saved state. Logic should
    check what is the activity id and data to return to correct state.

    \sa QApplication
*/

/*!
    \fn void HbApplication::activate()

    This signal is emitted when some activity needs to be shown.
*/

static void initSplash(Hb::ApplicationFlags flags, int argc, char *argv[])
{
    if (flags & Hb::NoSplash) {
        return;
    }

    // Show the splash screen (start() also makes sure it is really drawn before
    // continuing with anything else).

    HbSplashScreen::Flags splashFlags = HbSplashScreen::Default;
    if (flags & Hb::SplashFixedVertical) {
        splashFlags |= HbSplashScreen::FixedVertical;
    } else if (flags & Hb::SplashFixedHorizontal) {
        splashFlags |= HbSplashScreen::FixedHorizontal;
    }
    if (flags & Hb::ForceQtSplash) {
        splashFlags |= HbSplashScreen::ForceQt;
    }
    if (flags & Hb::ShowSplashWhenStartingToBackground) {
        splashFlags |= HbSplashScreen::ShowWhenStartingToBackground;
    }

#if defined(Q_OS_SYMBIAN) && defined(HBAPP_LOGGING)
    ELAPSED_TIMER t;
    t.start();
#endif

    HbSplashScreen::start(splashFlags, argc, argv);

#if defined(Q_OS_SYMBIAN) && defined(HBAPP_LOGGING)
    RDebug::Printf("[hbsplash] %d ms", (int) t.elapsed());
#endif
}

static int &preInitApp(int &argc, char *argv[], Hb::ApplicationFlags flags)
{
#if defined(Q_OS_SYMBIAN) && defined(HBAPP_LOGGING)
    RDebug::Printf("HbApplication: preInitApp");
#endif

    // This function contains code that needs to be executed before
    // the QApplication constructor.

    if (!HbSplashScreenExt::needsQt() && !flags.testFlag(Hb::ForceQtSplash)) {
        // The splash screen is capable of working without relying on Qt in any
        // way. So launch it now. This is the ideal case because we don't have
        // to wait for the potentially slow QApplication construction.
        initSplash(flags, argc, argv);
    }

#if defined(Q_OS_SYMBIAN)
    // Disable legacy screen furniture.
    QApplication::setAttribute(Qt::AA_S60DontConstructApplicationPanes);

    // Temporary solution until Hb specific style is ready.
    QApplication::setStyle(new QWindowsStyle);
#endif //Q_OS_SYMBIAN

    return argc;
}

static void handleQtBasedSplash(Hb::ApplicationFlags flags, int argc, char *argv[])
{
    if (HbSplashScreenExt::needsQt() || flags.testFlag(Hb::ForceQtSplash)) {
        // Splash needs Qt so it has not yet been started. Launch it now.
        initSplash(flags, argc, argv);
    } else {
        // Splash is already up and running, notify that Qt is now available.
        HbSplashScreenExt::doQtPhase();
    }
}

static void initialize()
{
#ifdef HB_GESTURE_FW
    QGestureRecognizer::unregisterRecognizer(Qt::TapGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::TapAndHoldGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::PanGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::SwipeGesture);

    QGestureRecognizer::registerRecognizer(new HbTapGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbTapAndHoldGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbPanGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbSwipeGestureRecognizer);
#endif

    // Installs empty input context proxy. It sits there
    // and monitors if someone wants to focus an editor
    // before the main window delayed construction is over.
    // If such a condition is detected, it sets a flag so
    // that HbInputMethod::initializeFramework knows
    // to resend the requestSoftwareInputPanel event once
    // the actual construction is over.
    qApp->setInputContext(new HbInputContextProxy(0));
}

/*!
    Constructs the application with \a argc and \a argv.
*/
HbApplication::HbApplication(int &argc, char *argv[], Hb::ApplicationFlags flags)
    : QApplication(preInitApp(argc, argv, flags), argv)
{
#if defined(Q_OS_SYMBIAN) && defined(HBAPP_LOGGING)
    RDebug::Printf("HbApplication: QApplication constructed");
#endif

    handleQtBasedSplash(flags, argc, argv); // must be the first thing we do here

    d_ptr = new HbApplicationPrivate(this);

    // No expensive operations allowed here, prefer performing such
    // initialization as part of HbMainWindow's delayed construction instead.

    initialize();
}

#if defined(Q_OS_SYMBIAN)
HbApplication::HbApplication(QApplication::QS60MainApplicationFactory factory,
                             int &argc, char *argv[], Hb::ApplicationFlags flags)
    : QApplication(factory, preInitApp(argc, argv, flags), argv)
{
#if defined(Q_OS_SYMBIAN) && defined(HBAPP_LOGGING)
    RDebug::Printf("HbApplication: QApplication constructed");
#endif

    handleQtBasedSplash(flags, argc, argv); // must be the first thing we do here

    d_ptr = new HbApplicationPrivate(this);

    // No expensive operations allowed here, prefer performing such
    // initialization as part of HbMainWindow's delayed construction instead.

    initialize();
}
#endif // Q_OS_SYMBIAN

/*!
    Destructor.
 */
HbApplication::~HbApplication()
{
    hideSplash();
}

/*!
    Hides the splash screen if it is visible. Normally this is done by
    HbMainWindow but if an application does not create any HbMainWindow
    instances then there may be a need to hide the splash screen manually.
*/
void HbApplication::hideSplash()
{
    HbSplashScreen::destroy();
}

#if defined(Q_OS_SYMBIAN)
#include <w32std.h>
#include <coecntrl.h>
#include <coemain.h>
#include <QDesktopWidget>
#include <QStringList>
#include <hbinstance.h>
#include <hbinstance_p.h>
#include <hbdeviceprofile.h>
#include <hbdeviceprofilemanager_p.h>
#include <hbs60events.h>
#include <hbtextitem_p.h>
#include <hbiconitem_p.h>
#include <hbtoucharea_p.h>
#include "hbgraphicsscene_p.h"

static void forceRefresh()
{
    foreach(HbMainWindow * window, hbInstance->allMainWindows()) {
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(window, &event);
    }
}

/*!
    Handles the S60 events.
 */
bool HbApplication::symbianEventFilter(const QSymbianEvent *event)
{
    if (event->type() != QSymbianEvent::WindowServerEvent) {
        return QApplication::symbianEventFilter(event);
    }

    // Do not add system critical functionality here (or to HbApplication in general).
    // Orbit apps may not necessarily have a HbApplication instance.
    // Some may use QApplication and all features must work in those cases too.

    const TWsEvent *aEvent = event->windowServerEvent();
    switch (aEvent->Type()) {
    case EEventScreenDeviceChanged: {
        // Resize the mainwindows when the screen device changes.
        // This is needed only to support the old S60 emulator.
        CWsScreenDevice *screenDevice = CCoeEnv::Static()->ScreenDevice();
        if (screenDevice) {
            TPixelsTwipsAndRotation params;
            int mode = screenDevice->CurrentScreenMode();
            screenDevice->GetScreenModeSizeAndRotation(mode, params);
            QSize nSize(params.iPixelSize.iWidth, params.iPixelSize.iHeight);
            QList<HbMainWindow *> windows = hbInstance->allMainWindows();
            foreach(HbMainWindow * w, windows) {
                w->resize(nSize);
            }
        }
    }
    return false; //continue handling in QApplication::s60ProcessEvent

    case KChangeDirection: {
        TUint8 *dataptr = aEvent->EventData();
        switch (*dataptr) {
        case 0:
            HbApplication::setLayoutDirection(Qt::LeftToRight);
            break;
        case 1:
            HbApplication::setLayoutDirection(Qt::RightToLeft);
            break;
        default:
            hbWarning("HbApplication::s60EventFilter: Unknown layout direction received");
            break;
        }
    }
    return false;
    case KChangeDeviceProfile: {
        TUint8 *dataptr = aEvent->EventData();
        QStringList names = HbDeviceProfile::profileNames();
        if (*dataptr > names.count() - 1) {
            hbWarning("HbApplication::s60EventFilter: Unknown device profile received");
        } else {
            HbDeviceProfile profile(names.value(*dataptr));
            HbDeviceProfileManager::select(profile);
            QList<HbMainWindow *> windows = hbInstance->allMainWindows();
            HbMainWindow *w = windows.at(0);
            w->setOrientation(profile.orientation());
        }
    }
    return false;
    case KChangeTouchAreaVis: {
        TUint8 *dataptr = aEvent->EventData();
        HbTouchAreaPrivate::setOutlineDrawing(*dataptr == 1);
        forceRefresh();
    }
    return false;
    case KChangeTextItemVis: {
        TUint8 *dataptr = aEvent->EventData();
        HbTextItemPrivate::outlinesEnabled = *dataptr == 1;
        forceRefresh();
    }
    return false;
    case KChangeIconItemVis: {
        TUint8 *dataptr = aEvent->EventData();
        HbIconItemPrivate::outlinesEnabled = *dataptr == 1;
        forceRefresh();
    }
    return false;
    case KChangeFpsCounterVis: {
        TUint8 *dataptr = aEvent->EventData();
        HbGraphicsScenePrivate::fpsCounterEnabled = *dataptr == 1;
        forceRefresh();
    }
    return false;
    default:
        return QApplication::symbianEventFilter(event);
    }
}

#endif // Q_OS_SYMBIAN

HbApplicationPrivate::HbApplicationPrivate(HbApplication *parent)
    : QObject(parent), q_ptr(parent), mActivateReason(Hb::ActivationReasonNormal)
{
    mActivityManager = new HbActivityManager(this);
    connect(mActivityManager, SIGNAL(activityRequested(QString)), this, SLOT(prepareActivityData(QString)));
    mActivityManager->parseCommandLine(qApp->arguments(), mActivateReason, mActivateId, mActivateParams);
}

HbApplicationPrivate::~HbApplicationPrivate()
{
}

QVariant HbApplicationPrivate::activateData()
{
    if (!mActivateId.isNull() && !mActivateData.isValid()) {
        mActivateData = mActivityManager->activityData(mActivateId);
    }
    return mActivateData;
}

void HbApplicationPrivate::prepareActivityData(const QString &activityId)
{
    mActivateReason = Hb::ActivationReasonActivity;
    mActivateId = activityId;
    mActivateData = QVariant();
    mActivateParams = QVariantHash();

    emit q_ptr->activate();
}

/*!
    Returns instance of class responsible for activities handling.
 */
HbActivityManager *HbApplication::activityManager()
{
    Q_D(HbApplication);
    return d->mActivityManager;
}

/*!
    Returns activation parameters parsed from activation URI.
 */
QVariantHash HbApplication::activateParams() const
{
    Q_D(const HbApplication);
    return d->mActivateParams;
}

/*!
    Returns activate reason.
 */
Hb::ActivationReason HbApplication::activateReason() const
{
    Q_D(const HbApplication);
    return d->mActivateReason;
}

/*!
    Last activated activity id.
 */
QString HbApplication::activateId() const
{
    Q_D(const HbApplication);
    return d->mActivateId;
}

/*!
    Last activated activity data.
 */
QVariant HbApplication::activateData()
{
    Q_D(HbApplication);
    return d->activateData();
}

/*!
  Returns true when the command line indicates that the application
  was started to background from the system starter list.

  When an application is launched normally, the result will be false.

  Passing argc and argv is optional, however if they are omitted,
  QCoreApplication must have already been instantiated. If neither
  argc and argv are specified nor QCoreApplication is available, a
  warning will be printed and the function will return false.
 */
bool HbApplication::startedToBackground(int argc, char *argv[])
{
    // Note: We cannot use the apa command line, even though it would be very convenient
    // to check for EApaCommandBackground instead of a custom regular command line
    // argument. Unfortunately the apa command line can only be constructed once from the
    // process environment and we have to let Qt to do that otherwise certain things
    // (e.g. window group chaining) will get broken.
    QStringList args;
    // Support argc/argv-less invocation, at least when QCoreApplication exists.
    if (!argv) {
        if (QCoreApplication::instance()) {
            args = QCoreApplication::arguments();
        } else {
            qWarning("startedToBackground(): Neither argc+argv was passed nor QCoreApplication was available, giving up.");
            return false;
        }
    } else {
        for (int i = 0; i < argc; ++i) {
            args.append(QLatin1String(argv[i]));
        }
    }
    foreach (const QString &arg, args) {
        QString targ = arg.trimmed();
        if (!targ.compare(QLatin1String("-preload"), Qt::CaseInsensitive)
            || !targ.compare(QLatin1String("-bg"), Qt::CaseInsensitive)
            || !targ.compare(QLatin1String("bg=yes"), Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
