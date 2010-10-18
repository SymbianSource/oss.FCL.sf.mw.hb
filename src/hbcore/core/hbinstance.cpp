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

#include <QList>
#include <QDebug>
#include <QDir>
#include <hbinstance.h>
#include <hbinstance_p.h>
#include <hbtheme.h>
#include <hbstyle.h>
#include <hbtypefaceinfo_p.h>
#include "hbmainwindow_p.h"
#include "hbdeviceprofile.h"
#include "hbglobal_p.h"
#include "hblocalechangenotifier_p.h"

#include <hbfeedbackmanager.h>

// for testability plugin
#include <QtPlugin>
#include <QPluginLoader>
#include <QLibraryInfo>
#include "hbtestabilityinterface_p.h"
// end testability

#ifdef HB_SETTINGS_WINDOW
#include <hbsettingswindow_p.h>
#include <QShortcut>
#endif

#ifdef HB_GESTURE_FW
#include "hbgesturerecognizers_p.h"
#endif

#ifdef HB_CSS_INSPECTOR
#include "hbcssinspector_p.h"
#endif

#ifdef HB_TEXT_MEASUREMENT_UTILITY
#include <QTimer>
#include "hbtextmeasurementutility_r.h"
#endif


#ifdef HB_TEXT_MEASUREMENT_UTILITY
static const int LOCALIZATION_MEASUREMENT_DELAY = 500; // Delay in milliseconds.
#endif

/*!
    @stable
    @hbcore
    \class HbInstance
    \brief HbInstance manages global settings and objects in the application.

    HbInstance can be used to access objects such as style, theme or interaction
    manager.  It can be used both with and without HbApplication,
    i.e. applications instantiating QApplication instead of HbApplication can
    still use HbInstance to access various Hb-specific features.

    Note however that instantiating either QApplication or HbApplication is
    still mandatory before calling HbInstance::instance() (or using
    hbInstance).

    The example below shows how hbInstance global pointer can be used to access theme name:

    \dontinclude ultimatecodesnippet/main.cpp
    \skip int main(
    \until setApplicationName
    \skip HbMainWindow
    \until }

    \sa QApplication
    \sa HbApplication

*/

/*!
    \var hbInstance
    \relates HbInstance

    A global pointer referring to the unique application object. It is
    equivalent to the pointer returned by the HbInstance::instance().

    \sa HbInstance::instance()
*/

/*!
\internal
*/
HbInstancePrivate::HbInstancePrivate() :
    mTypefaceInfo(0),
    mStyle(0),
    mTheme(HbTheme::instance()),
    mOrientation(Qt::Vertical),
    mLibraryPaths(0),
    mDropHiddenIconData(false)
#ifdef Q_OS_SYMBIAN
    , testabilityEnabled(false)
    , mSts(0)
#endif //Q_OS_SYMBIAN
    , mLocaleChangeNotifier(0)
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    , mLocalizationMetricsProfile(0), 
    mLocalizationMeasurementPending(false)
#endif
{
    // initialization of dynamics parts of feedback manager
    HbFeedbackManager::instance();

    testabilityInterface = 0;

#ifdef Q_OS_SYMBIAN
    TRAPD(err, mRepo = CRepository::NewL(HBTESTABILITY_CREPO_ID));
    if (err == KErrNone) {
        TInt value = 0;
        err = mRepo->Get(HbTestabilityKey, value);
        if (err == KErrNone && value == 1) {
            testabilityEnabled = true;
        }
    }
#endif //Q_OS_SYMBIAN        

    connect(mTheme, SIGNAL(changeFinished()), this, SLOT(updateScenes()));

    // Activate testability plugin if exists
    QObject *plugin = 0;

#if defined(Q_OS_WIN) || defined(Q_OS_SYMBIAN)
    QString testabilityPluginPostfix = ".dll";
    QString testabilityPlugin = "testability/testability";
#elif defined(Q_OS_MAC)
    QString testabilityPluginPostfix = ".dylib";
    QString testabilityPlugin = "testability/libtestability";
#else
    QString testabilityPluginPostfix = ".so";
    QString testabilityPlugin = "testability/libtestability";
#endif

    testabilityPlugin = QLibraryInfo::location(QLibraryInfo::PluginsPath) + QObject::tr("/") + testabilityPlugin + testabilityPluginPostfix;

#ifdef Q_OS_SYMBIAN
    //TEMPORARY workaround:
    //there is a defect in s60 qt port so that it does not search for plugins
    //from all possible drives, so check for existence before loading the plugin
    //issue has been reported to port team and they will fix it at some point
    if (testabilityEnabled) {
        QString flashDrive = "C:";
        QString romDrive = "Z:";

        //add drive letter to plugin path and then check c and z for the plugin
        if (!testabilityPlugin.startsWith(flashDrive, Qt::CaseInsensitive) && !testabilityPlugin.startsWith(romDrive, Qt::CaseInsensitive)) {
            testabilityPlugin = flashDrive + testabilityPlugin;
        }

        QPluginLoader loader(testabilityPlugin.toLatin1().data());

        plugin = loader.instance();

        if (!plugin) {
            if (testabilityPlugin.startsWith(flashDrive, Qt::CaseInsensitive)) {
                testabilityPlugin.replace(flashDrive, romDrive, Qt::CaseInsensitive);
            } else {
                testabilityPlugin.replace(romDrive, flashDrive, Qt::CaseInsensitive);

            }
            loader.setFileName(testabilityPlugin.toLatin1().data());
            plugin = loader.instance();
        }
    }
    //if the file is in neither then let failure occur similarly as with other platforms
#else
    // No enabled-disabled check on other platforms, try to load the plug-in always.
    QPluginLoader loader(testabilityPlugin.toLatin1().data());
    plugin = loader.instance();
#endif //Q_OS_SYMBIAN        

    if (plugin) {
        testabilityInterface = qobject_cast<TestabilityInterface *>(plugin);
        if (testabilityInterface) {
            testabilityInterface->Initialize();
        }
    }
//end testability

    mLocaleChangeNotifier = q_check_ptr(new HbLocaleChangeNotifier());

#ifdef HB_GESTURE_FW
    QGestureRecognizer::unregisterRecognizer(Qt::TapGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::TapAndHoldGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::PanGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::SwipeGesture);
    QGestureRecognizer::unregisterRecognizer(Qt::PinchGesture);

    QGestureRecognizer::registerRecognizer(new HbTapGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbTapAndHoldGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbPanGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbSwipeGestureRecognizer);
    QGestureRecognizer::registerRecognizer(new HbPinchGestureRecognizer);
#endif
}

/*!
\internal
*/
HbInstancePrivate::~HbInstancePrivate()
{
    delete mTypefaceInfo;
    delete mStyle;
    delete mLibraryPaths;

    delete mLocaleChangeNotifier;
    mLocaleChangeNotifier = 0;

    //remove the testability plugin if it exists
    //makes sure that all resources used by the plugin
    //are free when the application exists
    if (testabilityInterface) {
        delete testabilityInterface;
        testabilityInterface = 0;
    }

#ifdef Q_OS_SYMBIAN
    if (mRepo) {
        delete mRepo;
        mRepo = 0;
    }

    if (mSts) {
        CSystemToneService::Delete(mSts);
        mSts = 0;
    }

#endif //Q_OS_SYMBIAN

}

/*!
\internal
*/
void HbInstancePrivate::addWindow(HbMainWindow *window)
{
    mWindows.append(window);
#ifdef HB_SETTINGS_WINDOW
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Alt+Shift+S"), window);
    connect(shortcut, SIGNAL(activated()), this, SLOT(showHideSettingsWindow()));
    if (HbSettingsWindow::exists()) {
        HbSettingsWindow::instance()->refresh();
    }
#ifdef HB_CSS_INSPECTOR
    QShortcut *cssShortcut = new QShortcut(QKeySequence("Ctrl+Alt+Shift+C"), window);
    connect(cssShortcut, SIGNAL(activated()), this, SLOT(showHideCssWindow()));
    if (HbCssInspectorWindow::exists()) {
        HbCssInspectorWindow::instance()->refresh();
    }
#endif
#endif
    emit windowAdded(window);
}

/*!
\internal
*/

/*!
\internal
*/
CSystemToneService *HbInstancePrivate::systemTone()
{
#ifdef Q_OS_SYMBIAN

    if (mSts == 0) {
        mSts = CSystemToneService::Create();
    }
    return mSts;

#else
    return  0;
#endif

}

bool HbInstancePrivate::removeWindow(HbMainWindow *window)
{
    bool result = mWindows.removeOne(window);
#ifdef HB_SETTINGS_WINDOW
    if (result && mWindows.isEmpty()) {
        if (HbSettingsWindow::exists()) {
            HbSettingsWindow::instance()->close();
        }
#ifdef HB_CSS_INSPECTOR
        if (HbCssInspectorWindow::exists()) {
            HbCssInspectorWindow::instance()->close();
        }
#endif
    } else if (HbSettingsWindow::exists()) {
        HbSettingsWindow::instance()->refresh();
    }
#endif
    if (result) {
        emit windowRemoved(window);
    }
    return result;
}

/*!
\internal
*/
void HbInstancePrivate::select(const HbDeviceProfile &profile)
{
    HbDeviceProfile oldProfile = mCurrentProfile;
    mCurrentProfile = profile;

    QListIterator<HbMainWindow *> iterator(mWindows);

    while (iterator.hasNext()) {
        HbMainWindow *window = iterator.next();
        // "true" - global profile change
        HbMainWindowPrivate::d_ptr(window)->select(mCurrentProfile, &oldProfile);
    }
}

/*!
\internal
*/
HbDeviceProfile HbInstancePrivate::profile()
{
    return mCurrentProfile;
}

void HbInstancePrivate::updateScenes()
{
    QGraphicsScene *updatedScene = 0; //krazy:exclude=qclasses

    // Update graphics scenes
    Q_FOREACH(HbMainWindow * window, mWindows) {
        QGraphicsScene *scene = window->scene(); //krazy:exclude=qclasses
        if (scene && scene != updatedScene) {
            scene->update();
            updatedScene = scene;
        }
    }
}

#ifdef HB_SETTINGS_WINDOW
void HbInstancePrivate::showHideSettingsWindow()
{
    if (HbSettingsWindow::instance()->isVisible()) {
        HbSettingsWindow::instance()->hide();
    } else {
        HbSettingsWindow::instance()->show();
    }
}
#endif

#ifdef HB_CSS_INSPECTOR
void HbInstancePrivate::showHideCssWindow()
{
    if (HbCssInspectorWindow::instance()->isVisible()) {
        HbCssInspectorWindow::instance()->hide();
    } else {
        HbCssInspectorWindow::instance()->show();
    }
}
#endif

/*!
    Returns the instance of HbTypefaceInfo
*/

HbTypefaceInfo *HbInstancePrivate::typefaceInfo() const
{
    if (!mTypefaceInfo) {
        mTypefaceInfo = new HbTypefaceInfo();
    }
    return mTypefaceInfo;
}

/*!
\internal
*/
void HbInstancePrivate::initLibraryPaths()
{
    if (!mLibraryPaths) {
        mLibraryPaths = new QStringList;

#if defined(Q_OS_SYMBIAN)
        const QString pluginRelativePath("resource/plugins/");

        QFileInfoList driveInfoList = QDir::drives();

        foreach(const QFileInfo & driveInfo, driveInfoList) {
            QFileInfo pathInfo(driveInfo.absolutePath() + pluginRelativePath);
            if (pathInfo.exists()) {
                *mLibraryPaths << pathInfo.absolutePath();
            }
        }
#elif defined(Q_OS_WIN32) || defined(Q_OS_UNIX)
        QString pluginsCanonicalPath = QDir(HB_PLUGINS_DIR).canonicalPath();
        mLibraryPaths->append(pluginsCanonicalPath);

        QString applicationCanonicalPath = QDir(qApp->applicationDirPath()).canonicalPath();
        mLibraryPaths->append(applicationCanonicalPath);
#endif
    }
}

void HbInstancePrivate::startLocalizationMeasurement() 
{
#ifdef HB_TEXT_MEASUREMENT_UTILITY
    if ( !mLocalizationMeasurementPending ) {
        mLocalizationMeasurementPending = true;
        QTimer::singleShot(LOCALIZATION_MEASUREMENT_DELAY, this, SLOT(doLocalizationMeasurements()));
    }
#endif
    return;
}

void HbInstancePrivate::doLocalizationMeasurements()
{
#ifdef HB_TEXT_MEASUREMENT_UTILITY     
    QList<HbMainWindow*> mainWindows;
    mainWindows = hbInstance->allMainWindows();

    HbTextMeasurementUtility *measureUtility = HbTextMeasurementUtility::instance();

    // NOTE: Localization text measurements are done only for one main window.
    // In case of multiple main window support this code needs to be modified.
    HbDeviceProfile profile = HbDeviceProfile::profile(mainWindows.first());
    if (profile.name() != mLocalizationMetricsProfile.name()) {
        measureUtility->reset();
        mLocalizationMetricsProfile = profile;
    }

    QFileInfo info(QCoreApplication::applicationFilePath());
    measureUtility->readReport(profile, info.baseName());
    measureUtility->measureItems();
    measureUtility->writeReport(profile, info.baseName());

    mLocalizationMeasurementPending = false;
#endif // HB_TEXT_MEASUREMENT_UTILITY

    return;
}

/*!
    Constructor
 */
HbInstance::HbInstance() :
    d(new HbInstancePrivate)
{
    QStringList profiles = HbDeviceProfile::profileNames();
    d->mCurrentProfile = HbDeviceProfile(profiles.value(0));
}

/*!
    Deconstructor
 */
HbInstance::~HbInstance()
{
    delete d;
}

/*!
    Returns static instance
 */
HbInstance *HbInstance::instance()
{
#ifndef Q_OS_SYMBIAN
    if (!QCoreApplication::instance()) {
        qWarning("HbInstance: No application instance present.");
    }
#endif // Q_OS_SYMBIAN

    static HbInstance theInstance;
    return &theInstance;
}

/*!
    Returns the list of all main windows in the application.

    In a multi-display device there might be a window per display,
    but in normal cases the list contains only once display which
    equals to the primary window.

    \sa HbWidget::mainWindow()
 */
QList<HbMainWindow *> HbInstance::allMainWindows() const
{
    return d->mWindows;
}


/*!
    Returns the platform style object. Note that widgets can use HbWidget's style()-method to get the
    style without using HbInstance.
*/
HbStyle *HbInstance::style() const
{
    if (!d->mStyle) {
        d->mStyle = new HbStyle();
    }
    return d->mStyle;
}

/*!
    Returns the platform theme object.
*/
HbTheme *HbInstance::theme() const
{
    return d->mTheme;
}

/*!
    Sets the list of directories to search when loading libraries to
    \a paths. All existing paths will be deleted and the path list
    will consist of the paths given in \a paths.

    \sa HbInstance::removeLibraryPath()
    \sa HbInstance::addLibraryPath()
    \sa HbInstance::libraryPaths()
 */
void HbInstance::setLibraryPaths(const QStringList &paths)
{
    if (!d->mLibraryPaths) {
        d->mLibraryPaths = new QStringList;
    }
    *d->mLibraryPaths = paths;
}

/*!
    Prepends \a path to the beginning of the library path list. The paths
    will be search in order, so the \a path is the first.

    The default path list consists of

    \li desktop environments: the installation directory for plugins
        and application execution directory. The default installation
        directory for plugins is INSTALL/plugins, where INSTALL is the
        directory where Hb was installed.
    \li Symbian: \c resource/plugins/ directory on each drive found
        from the device

    \sa HbInstance::removeLibraryPath()
    \sa HbInstance::libraryPaths()
    \sa HbInstance::setLibraryPaths()
*/
void HbInstance::addLibraryPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    d->initLibraryPaths();

    QString canonicalPath = QDir(path).canonicalPath();
    if (!canonicalPath.isEmpty() && !d->mLibraryPaths->contains(canonicalPath)) {
        d->mLibraryPaths->prepend(canonicalPath);
    }
}

/*!
    Removes path from the library path list. If path is empty or not in the path list,
    the list is not changed.

    See also addLibraryPath(), libraryPaths(), and setLibraryPaths().
*/
void HbInstance::removeLibraryPath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }

    d->initLibraryPaths();

    QString canonicalPath = QDir(path).canonicalPath();
    d->mLibraryPaths->removeAll(canonicalPath);
}


/*!
    Returns a list of paths that the application will search when dynamically
    loading libraries.

    \sa HbInstance::removeLibraryPath()
    \sa HbInstance::addLibraryPath()
    \sa HbInstance::setLibraryPaths()
*/
QStringList HbInstance::libraryPaths() const
{
    const_cast<HbInstancePrivate *>(d)->initLibraryPaths();
    return *d->mLibraryPaths;
}

/*!
  When enabled, the icons shown by HbIconItem and HbFrameItem primitives are
  automatically unloaded when the primitives become hidden from the graphics
  view's point of view.

  The data is not lost, the icons will be reloaded when the primitive is painted
  next time.

  This is disabled by default and enabling it causes a heavy performance hit in
  many cases.

  It should only be enabled by applications with extreme graphics memory needs,
  i.e. in cases where storing the graphics for invisible widgets and standard UI
  decorators is not acceptable and the application wants to make sure that it
  does not keep such graphics referenced on themeserver side.

  When using SgImage this does not ensure that the graphics data is really
  dropped from GPU or any other cache of the themeserver. It just ensures that
  an unload request is made for such icons, i.e. the reference count is
  decreased whenever the widget that renders them becomes hidden.

  For icons backed by a QPixmap (typical on desktop platforms) clearing the icon
  data results in destroying the underlying pixmap, thus freeing all possible
  system/gpu memory used by the icon.

  This setting has no effect on icons constructed from QIcon.
 */
void HbInstance::enableUnloadingHiddenIconData(bool enable)
{
    d->mDropHiddenIconData = enable;
}

// end of file
