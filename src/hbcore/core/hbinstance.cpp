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
#include <hboogmwatcher_p.h>
#include "hbmainwindow_p.h"
#include "hbdeviceprofile.h"
#include "hbglobal_p.h"

#include <hbfeedbackmanager.h>

// for testability plugin
#ifdef HB_TESTABILITY
#include <QtPlugin> 
#include <QPluginLoader>
#include <QLibraryInfo>
#include "hbtestabilityinterface_p.h"
#endif //HB_TESTABILITY
// end testability

#if defined(Q_WS_S60)
#include <QDesktopWidget> // For orientation checking in HbInstance constructor
#include <aknappui.h> // For forcing orientation to change on setOrientation
#endif

#ifdef HB_SETTINGS_WINDOW
#include <hbsettingswindow_p.h>
#include <QShortcut>
#endif

/*!
    @beta
    @hbcore
	\class HbInstance
    \brief HbInstance is the application process' global instance.

    HbInstance can be used to access objects such as style, theme or interaction manager.
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
    equivalent to the pointer returned by the QCoreApplication::instance().

    \sa QCoreApplication::instance()

*/

/*!
\internal
*/
HbInstancePrivate::HbInstancePrivate() :
    mTypefaceInfo(0),
    mStyle(0),
    mTheme(HbTheme::instance()),
    mOrientation(Qt::Vertical),
    mLibraryPaths( 0 )
#ifdef Q_OS_SYMBIAN    
    ,testabilityEnabled(false)
#endif //Q_OS_SYMBIAN
{
    // initialization of dynamics parts of feedback manager
    HbFeedbackManager::instance();

#ifdef HB_TESTABILITY
    testabilityInterface = 0;

#ifdef Q_OS_SYMBIAN   
    TRAPD( err, mRepo = CRepository::NewL( HBTESTABILITY_CREPO_ID ) );
        if( err == KErrNone ) {
            TInt value = 0;
            err = mRepo->Get( HbTestabilityKey, value );
            if(err == KErrNone && value == 1 ){
                testabilityEnabled = ETrue;
            }
        }    
#endif //Q_OS_SYMBIAN        
#endif //HB_TESTABILITY

    connect(mTheme, SIGNAL(changeFinished()), this, SLOT(updateScenes()));
    
#ifdef HB_TESTABILITY
    // Activate testability plugin if exists    
    QString testabilityPlugin = "testability/testability";
    QString testabilityPluginPostfix = ".dll";
    
    QObject *plugin = 0;
    
#ifdef Q_OS_LINUX
    testabilityPluginPostfix = ".so";
    testabilityPlugin = "testability/libtestability";
#endif //linux

#ifdef Q_OS_MAC
    testabilityPluginPostfix = ".dylib";
    testabilityPlugin = "testability/libtestability";
#endif //mac
    
    testabilityPlugin = QLibraryInfo::location(QLibraryInfo::PluginsPath) + QObject::tr("/") + testabilityPlugin + testabilityPluginPostfix;

#ifdef Q_OS_SYMBIAN

    //TEMPORARY workaround:
    //there is a defect in s60 qt port so that it does not search for plugins 
    //from all possible drives, so check for existence before loading the plugin
    //issue has been reported to port team and they will fix it at some point
        if(testabilityEnabled){
            QString flashDrive = "C:";
            QString romDrive = "Z:";   
        
            //add drive letter to plugin path and then check c and z for the plugin
            if(!testabilityPlugin.startsWith(flashDrive,Qt::CaseInsensitive) && !testabilityPlugin.startsWith(romDrive,Qt::CaseInsensitive)){
                testabilityPlugin = flashDrive + testabilityPlugin; 
            }
        
            QPluginLoader loader(testabilityPlugin.toLatin1().data());
                
            plugin = loader.instance();   
        
            if(!plugin){
                if(testabilityPlugin.startsWith(flashDrive,Qt::CaseInsensitive)){
                    testabilityPlugin.replace(flashDrive, romDrive, Qt::CaseInsensitive);
                }        
                else{
                    testabilityPlugin.replace(romDrive, flashDrive, Qt::CaseInsensitive);
        
                }
                loader.setFileName(testabilityPlugin.toLatin1().data());
                plugin = loader.instance(); 
            }
        }
        //if the file is in neither then let failure occur similarly as with other platforms    
#else    
    QPluginLoader loader(testabilityPlugin.toLatin1().data());
    plugin = loader.instance();
#endif //Q_OS_SYMBIAN        

    if (plugin) {
      testabilityInterface = qobject_cast<TestabilityInterface *>(plugin);      
      if (testabilityInterface) {
          testabilityInterface->Initialize();
      } 
    }
#endif //end testability
}

/*!
\internal
*/
HbInstancePrivate::~HbInstancePrivate()
{
	delete mTypefaceInfo;
	delete mStyle;
    delete mLibraryPaths;
    
#ifdef HB_TESTABILITY
    //remove the testability plugin if it exists
    //makes sure that all resources used by the plugin
    //are free when the application exists
    if (testabilityInterface){
        delete testabilityInterface;
        testabilityInterface = 0;
    }
#endif //HB_TESTABILITY

#ifdef Q_OS_SYMBIAN
    if(mRepo){
        delete mRepo;
        mRepo = 0;
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
    HbSettingsWindow::instance()->refresh();
#ifdef CSS_INSPECTOR
    QShortcut *cssShortcut = new QShortcut(QKeySequence("Ctrl+Alt+Shift+C"), window);
    connect(cssShortcut, SIGNAL(activated()), this, SLOT(showHideCssWindow()));
    HbCssInspectorWindow::instance()->refresh();
#endif
#endif
    HbOogmWatcher::instance()->mainWindowReady();
    emit windowAdded(window);
}

/*!
\internal
*/
bool HbInstancePrivate::removeWindow(HbMainWindow *window)
{
    bool result = mWindows.removeOne(window);
#ifdef HB_SETTINGS_WINDOW
    if (result && mWindows.isEmpty()) {
        HbSettingsWindow::instance()->close();
#ifdef CSS_INSPECTOR
        HbCssInspectorWindow::instance()->close();
#endif
    } else {
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
Qt::Orientation HbInstancePrivate::orientation() const
{
    return mCurrentProfile.orientation();
}

/*!
\internal
*/
void HbInstancePrivate::setOrientation(Qt::Orientation orientation, bool animate)
{
    if(HbDeviceProfile(mCurrentProfile.alternateProfileName()).isNull()) {
        qWarning("HbInstancePrivate::setOrientation the alternate profile is NULL");
    }

    if (mCurrentProfile.orientation() != orientation) {
        mCurrentProfile = HbDeviceProfile(mCurrentProfile.alternateProfileName());
    }

    foreach (HbMainWindow *window, mWindows) {
        // Call directly the private part of the HbMainWindow, since if calling
        // public API's setOrientation it will disable automatic orientation
        // switching.
        HbMainWindowPrivate::d_ptr(window)->setTransformedOrientation(orientation, animate);
    }
}

/*!
\internal
*/
void HbInstancePrivate::select(const HbDeviceProfile &profile)
{
    HbDeviceProfile oldProfile = mCurrentProfile;
    mCurrentProfile = profile;
    
    QListIterator<HbMainWindow*> iterator(mWindows);

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
    Q_FOREACH(HbMainWindow *window, mWindows) {
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

#ifdef CSS_INSPECTOR
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
    if ( !mLibraryPaths ) {
        mLibraryPaths = new QStringList;

#if defined(Q_OS_SYMBIAN)
        const QString pluginRelativePath("resource/plugins/");

        QFileInfoList driveInfoList = QDir::drives();

        foreach (const QFileInfo &driveInfo, driveInfoList) {
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
HbInstance* HbInstance::instance()
{
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
QList<HbMainWindow*> HbInstance::allMainWindows() const
{
    return d->mWindows;
}

/*!
    \deprecated HbInstance::orientation() const
        is deprecated. Use HbMainWindow orientation API.
    
    Returns the application's current orientation.

    \sa setOrientation() HbWidget::mainWindow() HbMainWindow::orientation()
 */
Qt::Orientation HbInstance::orientation() const
{
    HB_DEPRECATED(" HbInstance::orientation() is deprecated, use HbMainWindow orientation API!");
    return d->orientation();
}

/*!
    \deprecated HbInstance::setOrientation(Qt::Orientation, bool)
    is deprecated. Use HbMainWindow orientation API.

    Sets the \a orientation of the application. It affects all windows in
    the application, which have automatic orientation enabled 
    (no explicitly set orientation). Orientation switch animation can be
    skipped by using the boolean parameter.

    \sa orientation() HbMainWindow::setOrientation()
 */
void HbInstance::setOrientation(Qt::Orientation orientation, bool animate)
{
    HB_DEPRECATED(" HbInstance::setOrientation() is deprecated, use HbMainWindow orientation API!");
    d->setOrientation(orientation, animate);
}

/*!
	Returns the platform style object. Note that widgets can use HbWidget's style()-method to get the 
	style without using HbInstance.
*/
HbStyle *HbInstance::style() const
{
    if( !d->mStyle ){
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
    Returns the instance of HbTypefaceInfo

    \deprecated HbInstance::typefaceInfo() const
    is deprecated. Use HbFontSpec instead.
     
*/

HbTypefaceInfo *HbInstance::typefaceInfo() const
{
    return NULL;
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
    if ( !d->mLibraryPaths ) {
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
    if (path.isEmpty())
        return;

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

// end of file
