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
#include "hbsplashscreen_p.h"
#include "hbactivitymanager.h"
#include <QTime>
#include <QUrl>

#if defined(Q_OS_SYMBIAN)
#include <qwindowsstyle.h>
#include <qsymbianevent.h>
#endif // Q_OS_SYMBIAN

// ### TODO remove this and do it in mainwindow_p once QGestureManager problems are fixed
#ifdef HB_GESTURE_FW
#include "hbmousepangesturerecognizer_p.h"
#include "hbswipegesturerecognizer_p.h"
#include "hbtapgesturerecognizer_p.h"
#include "hbtapandholdgesturerecognizer_p.h"
#endif

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

    To support Hb-widgets with QApplication the actual implementation
    of HbApplication is available in HbInstance.

    Unless the Hb::NoSplash flag is passed, the HbApplication constructor will
    try to show a suitable splash screen for the application. On some platforms
    there will be no splash screens available at all and thus nothing will be
    shown.

    Applications that support the 'activities' concept may check the start-up
    reason like this:
    
    \code
    HbApplication app(argc, argv);
    if(app.activateReason() == HbApplication::activity) {
        // start-up case
    } else if (app.activateReason() == HbApplication::service) {
        // service lauch 
    } else {
        // normal launch
    }

    MyActivitiesEngine logic;
    // connect to the application signal
    QObject::connect(&app, SIGNAL(activate()), &logic, SLOT(openActivity())); 
    \endcode
    
    When new activity needs to be activated signal is emited. Application might
    observe it and start correct handling to return to saved state. Logic should
    check what is the activity id and data to return to correct state.
    
    \sa QApplication
*/

/*!
    \fn void HbApplication::activate()
    
    This signal is emitted when some activity needs to be shown.
*/

static int& preInitApp(int &argc)
{
    // This function contains code that needs to be executed before
    // the QApplication constructor.

#if defined(Q_OS_SYMBIAN)
    // Disable legacy screen furniture.
    QApplication::setAttribute(Qt::AA_S60DontConstructApplicationPanes);

    // Temporary solution until Hb specific style is ready.
    QApplication::setStyle(new QWindowsStyle);
#endif //Q_OS_SYMBIAN

    return argc;
}

static void initSplash(Hb::ApplicationFlags flags)
{
    if (flags & Hb::NoSplash) {
        return;
    }

    // Show the splash screen (start() also makes sure it is really drawn before
    // continuing with anything else).

    HbSplash::Flags splashFlags = HbSplash::Default;
    if (flags & Hb::SplashFixedVertical) {
        splashFlags |= HbSplash::FixedVertical;
    } else if (flags & Hb::SplashFixedHorizontal) {
        splashFlags |= HbSplash::FixedHorizontal;
    }

#ifdef Q_OS_SYMBIAN
    QTime t;
    t.start();
#endif

    HbSplashScreen::start(splashFlags);

#ifdef Q_OS_SYMBIAN
    qDebug("[hbsplash] %d ms", t.elapsed());
#endif
}

static void initialize()
{
// ### TODO remove this and enable HbMainWindowPrivate::initGestures once
// the QGestureManager problems are fixed.
#ifdef HB_GESTURE_FW
    QGestureRecognizer::unregisterRecognizer(Qt::TapGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::TapAndHoldGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::PanGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::SwipeGesture);

    QGestureRecognizer::registerRecognizer(new HbTapGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbTapAndHoldGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbMousePanGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbSwipeGestureRecognizer);
#endif
}

/*!
    Constructs the application with \a argc and \a argv.
*/
HbApplication::HbApplication(int &argc, char *argv[], Hb::ApplicationFlags flags)
    : QApplication(preInitApp(argc), argv)
{
    initSplash(flags); // must be the first thing we do here

    d_ptr = new HbApplicationPrivate(this);

    // No expensive operations allowed here, prefer performing such
    // initialization as part of HbMainWindow's delayed construction instead.

    initialize();
}

#if defined(Q_WS_S60)
HbApplication::HbApplication(QApplication::QS60MainApplicationFactory factory,
                             int &argc, char *argv[], Hb::ApplicationFlags flags)
    : QApplication(factory, preInitApp(argc), argv)
{
    initSplash(flags); // must be the first thing we do here

    d_ptr = new HbApplicationPrivate(this);

    // No expensive operations allowed here, prefer performing such
    // initialization as part of HbMainWindow's delayed construction instead.

    initialize();
}
#endif // Q_WS_S60

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

#if defined(Q_WS_S60)
#include <w32std.h>
#include <coecntrl.h>
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

#ifdef BUILD_HB_INTERNAL
static void forceRefresh()
{
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(window, &event);
    }
}
#endif

/*!
    Handles the S60 events.
 */
bool HbApplication::symbianEventFilter(const QSymbianEvent *event)
{
    if (event->type() != QSymbianEvent::WindowServerEvent) {
        return QApplication::symbianEventFilter(event);
    }
    const TWsEvent *aEvent = event->windowServerEvent();
    switch (aEvent->Type()) {
         // In case of EEventScreenDeviceChanged-event, the current screen
         // ratio is checked and orientation is set accordingly. 
        case EEventScreenDeviceChanged:{

        QList<HbMainWindow*> windows = hbInstance->allMainWindows();
        RWindow *win = static_cast<RWindow *>(windows.at(0)->effectiveWinId()->DrawableWindow());
               
       TSize rWinSize;
       if (win)
           rWinSize = win->Size();
             
        // fix for emulator / changing modes
        QSize nSize( (int)rWinSize.iWidth, (int)rWinSize.iHeight );
        foreach (HbMainWindow* w, windows) {
                    w->resize(nSize);
                }
            

        }
            return false; //continue handling in QApplication::s60ProcessEvent
		case KChangeDirection:{
			TUint8* dataptr = aEvent->EventData();
			switch(*dataptr){
				case 0:
					HbApplication::setLayoutDirection(Qt::LeftToRight);
					break;
				case 1:
					HbApplication::setLayoutDirection(Qt::RightToLeft);
					break;
				default:
					qWarning("HbApplication::s60EventFilter: Unknown layout direction received");
					break;
				}
			}
			return false;
		case KChangeOrientation:{
			TUint8* dataptr = aEvent->EventData();
			switch(*dataptr){
				case 0:
					hbInstance->setOrientation(Qt::Vertical);
					break;
				case 1:
					hbInstance->setOrientation(Qt::Horizontal);
					break;
				default:
					qWarning("HbApplication::s60EventFilter: Unknown orientation received");
					break;
				}
			}
			return false;
		case KChangeDeviceProfile:{
			TUint8* dataptr = aEvent->EventData();
			QStringList names = HbDeviceProfile::profileNames();
			if(*dataptr > names.count() - 1){
				qWarning("HbApplication::s60EventFilter: Unknown device profile received");
			}else{
				HbDeviceProfile profile(names.value(*dataptr));
				HbDeviceProfileManager::select(profile);
				HbInstancePrivate::d_ptr()->setOrientation(profile.orientation(),false);
			}
			}
			return false;
#ifdef BUILD_HB_INTERNAL
        case KChangeTouchAreaVis:{
                TUint8* dataptr = aEvent->EventData();
                HbTouchAreaPrivate::setOutlineDrawing(*dataptr == 1);
                forceRefresh();
            }
            return false;
        case KChangeTextItemVis:{
                TUint8* dataptr = aEvent->EventData();
                HbTextItemPrivate::outlinesEnabled = *dataptr == 1;
                forceRefresh();
            }
            return false;
        case KChangeIconItemVis:{
                TUint8* dataptr = aEvent->EventData();
                HbIconItemPrivate::outlinesEnabled = *dataptr == 1;
                forceRefresh();
            }
            return false;
        case KChangeFpsCounterVis:{
                TUint8* dataptr = aEvent->EventData();
                HbGraphicsScenePrivate::fpsCounterEnabled = *dataptr == 1;
                forceRefresh();
            }
            return false;
#endif
        default:
            return QApplication::symbianEventFilter(event);
        }
}

#endif // Q_WS_S60

HbApplicationPrivate::HbApplicationPrivate(HbApplication *parent)
    : QObject(parent), q_ptr(parent), mActivateReason(Hb::ActivationReasonNormal)
{
    QStringList commandLineArguments = qApp->arguments();
    int activityMarkerIndex = commandLineArguments.indexOf("-activity");
    if (activityMarkerIndex != -1 && commandLineArguments.count() - 1 > activityMarkerIndex) {
        QUrl activityUri(commandLineArguments.at(activityMarkerIndex+1));        
        if (activityUri.scheme() == "appto") {
            typedef QPair<QString, QString> ParamType;
            QList<ParamType> parameters = activityUri.queryItems();            
            foreach (const ParamType &param, parameters) {
                mActivateParams.insert(param.first, param.second);
            }
            if (mActivateParams.contains("activityname") && !mActivateParams.value("activityname").toString().isEmpty()) {
                // all necessary data is present
                mActivateReason = Hb::ActivationReasonActivity;
                mActivateId = mActivateParams.value("activityname").toString();
            } else {
                mActivateParams.clear();
            }
        }
    }
    
    mActivityManager = new HbActivityManager(this);
    connect(mActivityManager, SIGNAL(activityRequested(QString)), this, SLOT(prepareActivityData(QString)));
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
