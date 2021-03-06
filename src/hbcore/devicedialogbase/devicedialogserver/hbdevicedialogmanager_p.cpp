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

#include <hbdevicedialogpluginmanager_p.h>
#include <hbdevicedialogplugin.h>
#include <hbdevicedialoginterface.h>
#include <hbdevicedialogerrors_p.h>
#include <hbdevicedialogtrace_p.h>
#include <hbwidget.h>
#include <hbpopup_p.h>
#include "hbdevicedialogmanager_p_p.h"
#include "hbdevicedialogmanager_p.h"
#include "hbindicatorpluginmanager_p.h"
#include "hbdevicedialogscontainer_p.h"

#include <hbmainwindow.h>
#include <hbinstance.h>
#include <hbpopup.h>
#include <hbgraphicsscene.h>
#include <hbcorepskeys_r.h>
#if defined(Q_OS_SYMBIAN)
#include <coemain.h>
#include <coecntrl.h>
#include <e32cmn.h>
#else
#include "hbindicatorwin32_p.h"
#endif // Q_OS_SYMBIAN

const char* indicatorMenu = "com.nokia.hb.indicatormenu/1.0";

#ifndef Q_OS_SYMBIAN

int count = 0;
HbDeviceDialogManager *manager = 0;
QMutex mutex;

HbDeviceDialogManager* managerInstance() {
    mutex.lock();
    if (manager == 0) {
        manager = new HbDeviceDialogManager;
    }

    count++;

    mutex.unlock();
    return manager;
}

void releaseManagerInstance() {
    mutex.lock();
    count--;

    if (count == 0) {
        delete manager;
        manager = 0;
    }
    mutex.unlock();
}
#endif

#if defined(Q_OS_SYMBIAN)
TRect QRectToTRect(const QRectF &rect)
{
    TRect trect;
    trect.SetRect(rect.topLeft().x(), rect.topLeft().y(),
        rect.bottomRight().x() + 1, rect.bottomRight().y() + 1);
    return trect;
}

QRectF MousePressCatcher::boundingRect() const
{
    return QRectF();
}

void MousePressCatcher::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

bool MousePressCatcher::sceneEventFilter(QGraphicsItem*, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        // Reset region in order notification dialog to catch up event. If touch is dragged outside,
        // ui-spec. states that dialog should close.
        mDeviceDialogManger->resetWindowRegion();
    }
    return false;
}

bool RegionUpdateFilter::eventFilter(QObject* obj, QEvent *event)
    {
    if (event->type() ==  QEvent::QEvent::GraphicsSceneResize) {
        HbPopup *popup = qobject_cast<HbPopup*>(obj);
        if (popup) {
            QRectF rect = popup->mapToScene(popup->rect()).boundingRect();
            rect = mDeviceDialogManger->mMainWindow->mapFromScene(rect).boundingRect();
            HbDeviceDialogsContainer::Dialog & dialog = mDeviceDialogManger->mDialogs.find(popup);
            mDeviceDialogManger->addRegionRect(dialog.id(), rect);
        }
    }
    return false;
    }
#endif

/*!
    \internal
    Constructor.
*/
HbDeviceDialogManagerPrivate::HbDeviceDialogManagerPrivate(HbDeviceDialogManager *manager) :
    QObject(),
    q(manager),
    mIndicatorPluginManager(0),
    mDialogs(mPluginManager),
    mMainWindow(0),
    mScene(0),
    mServerStatus(true),
    mHousekeeperTimerId(0)
#if defined(Q_OS_SYMBIAN)
    ,mMousePressCatcher(this)
    ,mRegionUpdateFilter(this)
    ,mWindowRegion()
    ,mWgOrdinalPriority(ECoeWinPriorityNormal)
#endif
{
    TRACE_ENTRY

    mIndicatorPluginManager = new HbIndicatorPluginManager;
    connect(mIndicatorPluginManager, SIGNAL( indicatorActivated(const IndicatorClientInfo) ),
            this, SLOT(indicatorActivated(const IndicatorClientInfo)));
    connect(mIndicatorPluginManager, SIGNAL( indicatorRemoved(const IndicatorClientInfo) ),
            this, SLOT(indicatorRemoved(const IndicatorClientInfo)));
    connect(mIndicatorPluginManager, SIGNAL( indicatorUpdated(const IndicatorClientInfo) ),
            this, SLOT(indicatorUpdated(const IndicatorClientInfo)));
    connect(mIndicatorPluginManager, SIGNAL(indicatorUserActivated(QVariantMap)),
            q, SIGNAL(indicatorUserActivated(QVariantMap)));
    // Server publishes it's status. Applications use it to delay showing of notification
    // dialogs when server is showing.
    mServerStatus.setStatus(HbDeviceDialogServerStatus::NoFlags);
    qApp->installEventFilter(this);
    init();
#if defined(Q_OS_SYMBIAN)   
    _LIT_SECURITY_POLICY_PASS(KRdPolicy); // all pass
    _LIT_SECURITY_POLICY_S0(KWrPolicy, KHbPsForegroundAppOrientationCategoryUid.iUid); // pass device dialog server    
    
    mHbOriProperty.Define(KHbPsForegroundAppOrientationCategoryUid,
                          KHbPsForegroundAppOrientationKey,
                          RProperty::EInt, KRdPolicy, KWrPolicy);
    mHbOriProperty.Attach(KHbPsForegroundAppOrientationCategoryUid,
                          KHbPsForegroundAppOrientationKey);
#endif        
    TRACE_EXIT
}

/*!
    \internal
    Destructor.
*/
HbDeviceDialogManagerPrivate::~HbDeviceDialogManagerPrivate()
{
    TRACE_ENTRY
    if (mHousekeeperTimerId) {
        killTimer(mHousekeeperTimerId);
    }
#if defined(Q_OS_SYMBIAN)
    mScene->removeItem(&mMousePressCatcher);
    mWindowRegion.Close();
    mHbOriProperty.Close();
#endif // Q_OS_SYMBIAN
    delete mIndicatorPluginManager;
    TRACE_EXIT
}

void HbDeviceDialogManagerPrivate::init()
{
    TRACE_ENTRY
    // Performance optimization for indicator menu.
    bool recycled(true);
    int error(0);

#if defined(Q_OS_SYMBIAN)
    // Speed-up search by specifying a file name
    QString pluginBaseFileName("HbIndicatorMenuPlugin");
#else // Q_OS_SYMBIAN
    QString pluginBaseFileName;
#endif // Q_OS_SYMBIAN

    HbDeviceDialogInterface *deviceDialogIf =
        mPluginManager.createWidget(QString(indicatorMenu), QVariantMap(), pluginBaseFileName,
            QString(), recycled, error);
    if (deviceDialogIf) {
        connectIndicatorStatus(deviceDialogIf);
        mPluginManager.freeWidget(deviceDialogIf);
    }
    TRACE_EXIT
}

// Set main window. Main window is not a construct parameter as it may be
// created later than device dialog server.
void HbDeviceDialogManagerPrivate::setMainWindow(HbMainWindow *mainWindow)
{
    mMainWindow = mainWindow;
    mScene = static_cast<HbGraphicsScene*>(mainWindow->scene());
    Q_ASSERT(mScene);

    // Windows/Linux, plugin manager may outlive scene. Scene deletes all widgets when it closes
    connect(mScene, SIGNAL(destroyed(QObject*)), &mPluginManager, SLOT(allWidgetsDeleted()));

#if defined(Q_OS_SYMBIAN)
    // Scene pointer is now valid
    mScene->addItem(&mMousePressCatcher);
#endif
}

// Called by client via device dialog server to show device dialog.
int HbDeviceDialogManagerPrivate::showDeviceDialog(
    HbDeviceDialogServer::DialogParameters &parameters)
{
    TRACE_ENTRY

    parameters.mError = HbDeviceDialogNoError;
    int id = 0;

    HbDeviceDialogPlugin::DeviceDialogInfo info;
    memset(&info, 0, sizeof(info));
    HbPopup *popup = 0;
    HbDeviceDialogInterface *deviceDialogIf = createDeviceDialog(
        parameters, info, popup);

    if (!deviceDialogIf) {
        return id;
    }

    // If there are dialogs launched by the same client, mark them as having no client.
    // They need to close within a time period.
    markNoClient(parameters.mClientTag);

    HbDeviceDialogsContainer::Dialog &dialog = mDialogs.add(deviceDialogIf, info);
    dialog.setVariable(HbDeviceDialogsContainer::Dialog::ClientTag, parameters.mClientTag);
    if (info.flags & HbDeviceDialogPlugin::SingleInstance) {
        // Only single instance of the dialog should be shown, save dialog type for later comparison
        dialog.setVariable(HbDeviceDialogsContainer::Dialog::DialogType, parameters.mType);
    }
    id = dialog.id();

    // Set popup priority
    HbDeviceDialogsContainer::Dialog::Flags flags = dialog.flags();
    setDialogPriority(popup, flags);

    if (flags & HbDeviceDialogsContainer::Dialog::NotificationGroup) {
        // Disable HbNotificationDialog sequential show feature. Server takes care of
        // showing sequentially.
        popup->setProperty("sequentialShow", false);
    }

    showDialogs();
    controlForeground();
    updateStatus();

    TRACE_EXIT
    return id;
}

// Called by client via device dialog server to update device dialog.
int HbDeviceDialogManagerPrivate::updateDeviceDialog(int id, const QVariantMap &parameters)
{
    TRACE_ENTRY
    int ret = HbDeviceDialogNotFoundError;
    HbDeviceDialogsContainer::Dialog &dialog = mDialogs.find(id);
    if (dialog.isValid()) {
        HbDeviceDialogInterface *interface = dialog.widget();
        bool success = false;
        bool catched  = false;
        try {
            success = interface->setDeviceDialogParameters(parameters);
        } catch (const std::bad_alloc &) {
            catched = true;
        }
        if (catched) {
            ret = HbDeviceDialogGeneralError;
        } else if (!success) {
            ret = checkpluginerror(interface->deviceDialogError());
        } else {
            ret = HbDeviceDialogNoError;
        }
    }
    TRACE_EXIT
    return ret;
}

// Called by client via device dialog server to close device dialog
int HbDeviceDialogManagerPrivate::closeDeviceDialog(int id, bool byClient)
{
    TRACE_ENTRY
    int ret = HbDeviceDialogNotFoundError;
    HbDeviceDialogsContainer::Dialog::Flags closeCalled(
        HbDeviceDialogsContainer::Dialog::CloseCalled);
    HbDeviceDialogsContainer::Dialog &dialog = mDialogs.find(id);
    if (dialog.isValid() && !(dialog.flags() & closeCalled)) {
        dialog.setFlags(closeCalled);
        if (!byClient) {
            dialog.setFlags(HbDeviceDialogsContainer::Dialog::ClosedByServer);
        }
        ret = HbDeviceDialogNoError;
        try {
            dialog.widget()->closeDeviceDialog(byClient);
        }  catch(const std::bad_alloc &) {
            ret = HbDeviceDialogGeneralError;
        }
        startHousekeeperTimer();
    }
    TRACE_EXIT
    return ret;
}

// Publish current orientation to PS-key
int HbDeviceDialogManagerPrivate::publishOrientation(int orientation)
{
#if defined(Q_OS_SYMBIAN)
    int ret = mHbOriProperty.Set(orientation); 
    return ret;
#else
    Q_UNUSED(orientation)
    return 0;    
#endif    
}

// Client (session) is closing
void HbDeviceDialogManagerPrivate::deviceDialogClientClosing(quintptr clientTag)
{
    // Mark device dialogs launched by the client as having client (session) closed.
    // Housekeeper closes these after a timeout. Dialogs without a client are allowed but
    // they need to close within a timeout.
    markNoClient(clientTag);
}

/*!
    \internal
    Activate indicator
*/
int HbDeviceDialogManagerPrivate::activateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    // In order to prevent dialogs/indicators trowing bad_alloc during execution, amount
    // of heap available is checked before activation is allowed.
    if (!heapAvailable(NormalLevelMinHeap)) {
        return HbDeviceDialogGeneralError;        
    }
    QVariantMap credentials;
    addSecurityCredentials(parameters, credentials);
    int result = HbDeviceDialogNoError;
    mIndicatorPluginManager->addIndicator(parameters.mType, credentials, &result);
    if (result == HbDeviceDialogNoError) {
        if (!mIndicatorPluginManager->activateIndicator(
            parameters.mType, parameters.mData, credentials)) {
                result = HbDeviceDialogAccessDeniedError;
            }
    }
    return result;
}

/*!
    \internal
    Deactivate indicator
*/
int HbDeviceDialogManagerPrivate::deactivateIndicator(
    HbDeviceDialogServer::IndicatorParameters &parameters)
{
    QVariantMap credentials;
    addSecurityCredentials(parameters, credentials);
    bool success = mIndicatorPluginManager->deactivateIndicator(
        parameters.mType, parameters.mData, credentials);
    return success ? HbDeviceDialogNoError : HbDeviceDialogAccessDeniedError;
}

QList<IndicatorClientInfo> HbDeviceDialogManagerPrivate::indicatorClientInfoList() const
{
    return mIndicatorPluginManager->indicatorClientInfoList();
}

#if defined(Q_OS_SYMBIAN)

void HbDeviceDialogManagerPrivate::doMoveToForeground(bool foreground, int priority)
{
    if (priority == ECoeWinPriorityNormal) { 
        if (mMainWindow->isVisible()) {
            mMainWindow->hide();
        }
    } else {
        if (!mMainWindow->isVisible() || !mMainWindow->isActiveWindow()) {
            mMainWindow->showFullScreen();
        }
    }
    const int position = foreground ? 0 : -1;
    RWindowGroup &rootWindowGroup = CCoeEnv::Static()->RootWin();                
    rootWindowGroup.SetOrdinalPosition(position, priority);
    mWgOrdinalPriority = priority;
}

// Move application fore/background and adjust window group priorities
void HbDeviceDialogManagerPrivate::controlForeground(ForegroundControl control)
{
    const HbDeviceDialogsContainer::Dialog::Flags showing(
        HbDeviceDialogsContainer::Dialog::Showing);
    const HbDeviceDialogsContainer::Dialog start;

    int numShowing = 0;
    int numSecurity = 0;
    int numCritical = 0;

    // Loop over showing dialogs
    const HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start, showing, showing);
    while(current->isValid()) {
        HbDeviceDialogsContainer::Dialog::Flags flags = current->flags();
        numShowing++;
        numSecurity += (flags & HbDeviceDialogsContainer::Dialog::SecurityLevel) ? 1 : 0;
        numCritical += (flags & HbDeviceDialogsContainer::Dialog::CriticalLevel) ? 1 : 0;
        current = &mDialogs.next(*current, showing, showing);
    }

    const int wgBackground = ECoeWinPriorityNormal;
    const int wgForeground = ECoeWinPriorityAlwaysAtFront;
    const int wgForegroundCritical = ECoeWinPriorityAlwaysAtFront + 50;

    if (numShowing == 0) {
        doMoveToForeground(false, wgBackground);
        return;
    }

    switch(control) {
    case FgControlNone:
        doMoveToForeground(true, numCritical ? wgForegroundCritical : wgForeground);
        break;
    case FgControlSecurityCriticalClosed:
        // Security or critical dialog closed. If there is only one security dialog active,
        // Give change to other apps at alwaysAtFront priority to come foreground.
        if (numCritical == 0 && numSecurity == 1) {
            doMoveToForeground(false, wgForeground);
        } else {
            doMoveToForeground(true, numCritical ? wgForegroundCritical : wgForeground);
        }
        break;
    }
}

void HbDeviceDialogManagerPrivate::updateWindowRegion() const
{
    RWindowBase *win =
        static_cast<RWindowBase*>(mMainWindow->effectiveWinId()->DrawableWindow());
    if (win) {
#ifdef HB_RWND_HAS_POINTER_REGION
        // SetShape() will be phased out by Symbian
        win->SetPointerAcceptanceRegion(mWindowRegion);
#else // HB_RWND_HAS_POINTER_REGION
        win->SetShape(mWindowRegion);
#endif // HB_RWND_HAS_POINTER_REGION
    }
}

void HbDeviceDialogManagerPrivate::resetWindowRegion() const
{
    RWindowBase *win =
        static_cast<RWindowBase*>(mMainWindow->effectiveWinId()->DrawableWindow());
    if (win) {
        RRegionBuf<1> windowRegion(QRectToTRect(mMainWindow->rect()));
#ifdef HB_RWND_HAS_POINTER_REGION
        win->SetPointerAcceptanceRegion(windowRegion);
#else // HB_RWND_HAS_POINTER_REGION
        win->SetShape(windowRegion);
#endif // HB_RWND_HAS_POINTER_REGION
    }
}

#else
void HbDeviceDialogManagerPrivate::doMoveToForeground(bool foreground, int priority)
{
    Q_UNUSED(foreground)
    Q_UNUSED(priority)            
}

void HbDeviceDialogManagerPrivate::controlForeground(ForegroundControl control)
{
    Q_UNUSED(control)
}

void HbDeviceDialogManagerPrivate::updateWindowRegion() const
{
}

void HbDeviceDialogManagerPrivate::resetWindowRegion() const
{
}
#endif

// Show next dialog. Return true if any dialogs are showing
bool HbDeviceDialogManagerPrivate::showDialogs()
{
    // Traverse list and show dialog widgets. Notifications are shown one at a time.
    // Other dialogs are shown on top of each other.

    // Shorthands for flags
    const HbDeviceDialogsContainer::Dialog::Flags notificationGroup(
        HbDeviceDialogsContainer::Dialog::NotificationGroup);
    const HbDeviceDialogsContainer::Dialog::Flags securityGroup(
        HbDeviceDialogsContainer::Dialog::SecurityGroup);
    const HbDeviceDialogsContainer::Dialog::Flags showing(
        HbDeviceDialogsContainer::Dialog::Showing);
    const HbDeviceDialogsContainer::Dialog::Flags closeCalled(
        HbDeviceDialogsContainer::Dialog::CloseCalled);
    const HbDeviceDialogsContainer::Dialog::Flags noFlags(0);
    
    // Check if any notification dialogs are showing
    const HbDeviceDialogsContainer::Dialog start;
    bool showingNotification = mDialogs.next(start, notificationGroup|showing,
        notificationGroup|showing).isValid();

    // Initialize variables to determine display lights control
    bool showingSecurity = mDialogs.next(start, securityGroup|showing,
        securityGroup|showing).isValid();
    HbDeviceDialogsContainer::Dialog::Flags newDialogs(0);

#if defined(Q_OS_SYMBIAN)
    // The dialog's orientation must be consistent with the foreground
    // application's orientation.
    int val = 0;
    if (mWsOriListener.get(val)) {
        Qt::Orientation currentOrientation;
        // 'val' contains a TRenderOrientation value, usually one of the four explicit
        // values. Force the orientation but continue monitoring the property value and
        // change orientation also later if needed. This is different than the Hb-only
        // solution because there is no way to tell if the orientation is fixed (forced)
        // or not so continuous listening must be used, but it is more robust and supports
        // non-Hb apps too.
        if (HbOrientationStatus::mapRenderOriToOri(val, currentOrientation)) {
            mMainWindow->setOrientation(currentOrientation, false);
            mWsOriListener.start(this, "wsOrientationChanged");
        }
    } else {
        // If something goes wrong, fall back to the non-wserv-based, legacy
        // solution supporting Hb apps only.
        val = 0;
        mHbOriProperty.Get(val);
        // If the Hb application, that is in foreground, has a forced (fixed)
        // orientation (i.e. it had explicitly called setOrientation or used the
        // similar window flags) then the device dialog's orientation must use
        // that orientation to be consistent. Otherwise stick with the
        // automatic, sensor-based rotation.
        if (val & KHbFixedOrientationMask) {
            Qt::Orientation currentOrientation = (Qt::Orientation) (val & KHbOrientationMask);
            if (currentOrientation == Qt::Vertical || currentOrientation == Qt::Horizontal) {
                mMainWindow->setOrientation(currentOrientation, false);
            }
        }
    }
#endif // Q_OS_SYMBIAN
    
    // Loop over not showing dialogs
    HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start, noFlags,
        showing | closeCalled);
    while(current->isValid()) {
        if (current->flags() & notificationGroup) {
            if (!showingNotification) { // notification dialogs are shown serially
                showingNotification = true;
                current->setFlags(showing);
                HbPopup *popup = current->widget()->deviceDialogWidget();
                popup->show();
                newDialogs |= current->flags();
#if defined(Q_OS_SYMBIAN)
                popup->installSceneEventFilter(&mMousePressCatcher);
                popup->installEventFilter(&mRegionUpdateFilter);
#endif //Q_OS_SYMBIAN
            }
        } else { // generic dialog
            current->setFlags(showing);
            current->widget()->deviceDialogWidget()->show();
            newDialogs |= current->flags();
            setupWindowRegion();
        }
        // Find next one that is not showing
        current = &mDialogs.next(*current, noFlags, showing | closeCalled);
    }
    current = 0;

    // Lights control. If security layer is active, only security or critical level turn lights on.
    // Otherwise lights turned on every time a dialog is shown.
    int lightsMask = HbDeviceDialogsContainer::Dialog::SecurityLevel |
        HbDeviceDialogsContainer::Dialog::CriticalLevel;
    if (!showingSecurity) {
        lightsMask |= HbDeviceDialogsContainer::Dialog::NormalLevel;
    }

    if (newDialogs & lightsMask) {
        refreshDisplayLightsTime();
    }

    const HbDeviceDialogsContainer::Dialog &nonNotificationDialog =
        mDialogs.next(start, showing, notificationGroup|showing);
    bool dialogsShowing = showingNotification || nonNotificationDialog.isValid();

    return dialogsShowing;
}

void HbDeviceDialogManagerPrivate::setupWindowRegion()
{
    // RWindow region control.
    // Check if any non-notification dialogs are showing.
    // Shorthands for flags
    const HbDeviceDialogsContainer::Dialog::Flags notificationGroup(
        HbDeviceDialogsContainer::Dialog::NotificationGroup);
    const HbDeviceDialogsContainer::Dialog::Flags showing(
        HbDeviceDialogsContainer::Dialog::Showing);

    const HbDeviceDialogsContainer::Dialog start;
    bool showingNotification = mDialogs.next(start, notificationGroup|showing,
        notificationGroup|showing).isValid();
    const HbDeviceDialogsContainer::Dialog &nonNotificationDialog =
        mDialogs.next(start, showing, notificationGroup|showing);
    bool dialogsShowing = showingNotification || nonNotificationDialog.isValid();
    if (dialogsShowing) {
        if (!nonNotificationDialog.isValid()) {
            // Only showing (one or more) modeless, non-interruptive notification dialogs.
            // Pointer events caught only inside dialog area, other events get passed to
            // the application beneath.
            updateWindowRegion();
            enableReceiptOfFocus(false); // call before moving to foreground.
        } else {
            resetWindowRegion();
            enableReceiptOfFocus(true);
        }
    } else {
        resetWindowRegion();
        enableReceiptOfFocus(true);
    }
}

/*!
    \internal
    Create a device dialog widget
*/
HbDeviceDialogInterface *HbDeviceDialogManagerPrivate::createDeviceDialog(
    HbDeviceDialogServer::DialogParameters &parameters,
    HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo, HbPopup *&popup)
{
    // In order to prevent a dialog trowing bad_alloc when it is executing, amount
    // of heap available is checked before new dialog is allowed.
    if (!heapAvailable(SecurityLevelMinHeap)) {
        parameters.mError = HbDeviceDialogGeneralError;
        return 0;
    }

    QString pluginFilePath;
    if (!mPluginManager.loadPlugin(parameters.mType, QString(), &pluginFilePath)) {
        parameters.mError = HbDeviceDialogNotFoundError;
        return 0;
    }

    const HbDeviceDialogPlugin &plugin = mPluginManager.plugin(pluginFilePath);
    if (!plugin.deviceDialogInfo(parameters.mType, parameters.mData, &deviceDialogInfo)) {
        // Ensure plugin returns valid error code
        parameters.mError = checkpluginerror(plugin.error());
        mPluginManager.unloadPlugin(pluginFilePath);
        return 0;
    }

    if (!checkDialogInfo(deviceDialogInfo)) {
        parameters.mError = HbDeviceDialogGeneralError;
        mPluginManager.unloadPlugin(pluginFilePath);
        return 0;
    }

    // Normal level device dialogs require bigger heap reserve in order to not to
    // block higher level dialogs.
    if (deviceDialogInfo.showLevel == HbDeviceDialogPlugin::NormalLevel &&
        !heapAvailable(NormalLevelMinHeap)) {
        parameters.mError = HbDeviceDialogGeneralError;
        mPluginManager.unloadPlugin(pluginFilePath);
        return 0;
    }

    // Plugin may request security check of the client
    if (deviceDialogInfo.flags & HbDeviceDialogPlugin::SecurityCheck) {
        QVariantMap credentials;
        addSecurityCredentials(parameters, credentials);
        if (!plugin.accessAllowed(parameters.mType, parameters.mData, credentials)) {
            parameters.mError = HbDeviceDialogAccessDeniedError;
            mPluginManager.unloadPlugin(pluginFilePath);
            return 0;
        }
    }

    // If dialog is single instance, launch it only if it's not shown already
    if (deviceDialogInfo.flags & HbDeviceDialogPlugin::SingleInstance) {
        if (isShowing(parameters.mType)) {
            parameters.mError = HbDeviceDialogAlreadyExists;
            mPluginManager.unloadPlugin(pluginFilePath);
            return 0;
        }
    }

    // Performance optimization for indicator menu. Device dialog widget is reused after the
    // first time it's shown.
    bool recycled = (deviceDialogInfo.group == HbDeviceDialogPlugin::IndicatorGroup);
    HbDeviceDialogInterface *deviceDialogIf =
        mPluginManager.createWidget(parameters.mType, parameters.mData, QString(), pluginFilePath,
        recycled, parameters.mError);

    // Decrease plugin reference count increased by loadPlugin() above. Unload takes place when
    // device dialog widget is deleted.
    mPluginManager.unloadPlugin(pluginFilePath);

    if (!deviceDialogIf){
        parameters.mError = checkpluginerror(parameters.mError);
        return deviceDialogIf;
    }

    popup = deviceDialogIf->deviceDialogWidget();
    // Check that: The widget exists, it is a HbPopup and it's not visible yet (we call show())
    if (!popup || qobject_cast<HbPopup*>(popup) == 0 || popup->isVisible()) {
        parameters.mError = HbDeviceDialogGeneralError;
        mPluginManager.freeWidget(deviceDialogIf);
        deviceDialogIf = 0;
        return deviceDialogIf;
    }
    // Signals may come from the dialog widget or interface may specify a sender
    QObject *sender = deviceDialogIf->signalSender();
    if (!sender) {
        sender = popup;
    }
    // Each signal sender must by unique in order to resolve signal source to the widget
    bool duplicateSenders = mDialogs.find(sender).isValid();
    // Closed signal is mandatory
    bool connected = connect(sender, SIGNAL(deviceDialogClosed()), SLOT(deviceDialogClosed()));
    if (!connected || duplicateSenders) {
        parameters.mError = HbDeviceDialogGeneralError;
        mPluginManager.freeWidget(deviceDialogIf);
        deviceDialogIf = 0;
        return deviceDialogIf;
    }
    connect(sender, SIGNAL(deviceDialogData(QVariantMap)),
        SLOT(deviceDialogUpdate(const QVariantMap)));

    if (!recycled && (deviceDialogInfo.flags & HbDeviceDialogPlugin::ReceiveIndicatorStatus)) {
        // Connect plugin manager signals to indicator menu slots, so that
        // it will get indicator updates. If the widget is reused, signals
        // are already connected.
        connectIndicatorStatus(deviceDialogIf);
    }

    return deviceDialogIf;
}

// Update published server status
void HbDeviceDialogManagerPrivate::updateStatus()
{
    HbDeviceDialogServerStatus::StatusFlags status = HbDeviceDialogServerStatus::NoFlags;
    const HbDeviceDialogsContainer::Dialog start;
    const HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start,
        HbDeviceDialogsContainer::Dialog::Showing, HbDeviceDialogsContainer::Dialog::Showing);
    if (current->isValid()) {
        status |= HbDeviceDialogServerStatus::ShowingDialog;
        if (current->flags() & HbDeviceDialogsContainer::Dialog::SecurityGroup) {
            status |= HbDeviceDialogServerStatus::ShowingScreenSaver;
        } else {
            const HbDeviceDialogsContainer::Dialog::Flags securityShowingFlags(
                HbDeviceDialogsContainer::Dialog::Showing |
                HbDeviceDialogsContainer::Dialog::SecurityGroup);
            current = &mDialogs.next(*current, securityShowingFlags, securityShowingFlags);
            if (current->isValid()) {
                status |= HbDeviceDialogServerStatus::ShowingScreenSaver;
            }
        }
    }
    mServerStatus.setStatus(status);
}

// Ensure error code is in plugin error range.
int HbDeviceDialogManagerPrivate::checkpluginerror(int errorCode)
{
    const int errorCodeMask = ~HbDeviceDialog::ErrorTypeMask;
    if (errorCode >= static_cast<int>(HbDeviceDialog::PluginErrors) && errorCode <=
        static_cast<int>(HbDeviceDialog::PluginErrors) + errorCodeMask) {
        return errorCode;
    } else {
        return HbDeviceDialog::PluginErrors + (errorCode & errorCodeMask);
    }
}

// Add client's security credentials into variant map
void HbDeviceDialogManagerPrivate::addSecurityCredentials(
    const HbDeviceDialogServer::Parameters &parameters, QVariantMap &credentials)
{
#if defined(Q_OS_SYMBIAN)
    credentials.insert("sym-secureId",
        static_cast<quint32>(parameters.mClientCredentials.SecureId().iId));
    credentials.insert("sym-vendorId",
        static_cast<quint32>(parameters.mClientCredentials.VendorId().iId));
    quint32 caps = 0;
    const int limit = ECapability_Limit < 32 ? ECapability_Limit : 32;
    TSecurityInfo securityInfo(parameters.mClientCredentials);
    for(int i = 0; i < limit; i++) {
        if (securityInfo.iCaps.HasCapability(static_cast<TCapability>(i))) {
            caps |= 1 << i;
        }
    }
    credentials.insert("sym-caps", caps);
#else // Q_OS_SYMBIAN
    Q_UNUSED(parameters)
    Q_UNUSED(credentials)
#endif // Q_OS_SYMBIAN
}

// Set dialog (popup) priority
void HbDeviceDialogManagerPrivate::setDialogPriority(HbPopup *popup,
    HbDeviceDialogsContainer::Dialog::Flags flags)
{
    // For notification, security and critical layer popups, set popup priority. Others use default.
    const quint8 CriticalPopupPriority = HbPopupPrivate::VirtualKeyboard - 2;
    const quint8 SecurityPopupPriority = HbPopupPrivate::VirtualKeyboard - 4;
    const quint8 NotificationPopupPriority = HbPopupPrivate::VirtualKeyboard  - 6;

    quint8 priority = HbPopupPrivate::Default;
    if (flags & HbDeviceDialogsContainer::Dialog::NormalLevel) {
        if (flags & HbDeviceDialogsContainer::Dialog::NotificationGroup) {
            priority = NotificationPopupPriority;
        }
    } else {
        if (flags & HbDeviceDialogsContainer::Dialog::SecurityLevel) {
            priority = SecurityPopupPriority;
        } else {
            priority = CriticalPopupPriority;
        }
    }
    HbPopupPrivate::d_ptr(popup)->setPriority(priority);
}

// Check that DeviceDialogInfo returned by plugin is valid
bool HbDeviceDialogManagerPrivate::checkDialogInfo(
    const HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo)
{
    unsigned int tmp = deviceDialogInfo.group;
    if (tmp > HbDeviceDialogPlugin::CriticalGroup) {
        return false;
    }
    tmp = deviceDialogInfo.showLevel;
    if (tmp > HbDeviceDialogPlugin::CriticalLevel) {
        return false;
    }
    return true;
}

// Check there is minimum amount of heap available
bool HbDeviceDialogManagerPrivate::heapAvailable(int aBytes)
{
    const int MaxFragments = 16;
    const int MinFragmentSize = 0x1000; // 4 kB
    int fragmentSize = aBytes / MaxFragments;
    if (fragmentSize < MinFragmentSize) {
        fragmentSize = MinFragmentSize;
    }
    char *fragments[MaxFragments];
    memset(&fragments, 0, sizeof(fragments));
    bool success = true;
    int i = 0;
    while(success && i < MaxFragments && aBytes >= 0) {
        try {
            fragments[i] = new char[fragmentSize];
        } catch(const std::bad_alloc &) {
            success = false;
        }
        i++;
        aBytes -= fragmentSize;
    }
    for(--i; i >= 0; i--) {
        delete [] fragments[i];
    }
    return success;
}

// Called by device dialog widget to send some data to client.
void HbDeviceDialogManagerPrivate::deviceDialogUpdate(const QVariantMap &data) const
{
    TRACE_ENTRY
    const HbDeviceDialogsContainer::Dialog &current =
        const_cast<HbDeviceDialogsContainer&>(mDialogs).find(sender());
    if (current.isValid()) {
        emit q->deviceDialogUpdate(current.id(), data);
    }
    TRACE_EXIT
}

// Deleted a device dialog
void HbDeviceDialogManagerPrivate::deleteDeviceDialog(int id)
{
    TRACE_ENTRY
    HbDeviceDialogsContainer::Dialog &dialog = mDialogs.find(id);

    bool securityDialog = dialog.flags() &
        (HbDeviceDialogsContainer::Dialog::SecurityLevel|HbDeviceDialogsContainer::Dialog::CriticalLevel);

    if (dialog.isValid()) {
        // If device dialog was cancelled by client or server, give reason
        int closeReason = HbDeviceDialogNoError;
        if ((dialog.flags() & HbDeviceDialogsContainer::Dialog::CloseCalled)) {
            if ((dialog.flags() & HbDeviceDialogsContainer::Dialog::ClosedByServer)) {
                closeReason = static_cast<int>(HbDeviceDialog::SystemCancelledError);
            } else {
                closeReason = static_cast<int>(HbDeviceDialog::CancelledError);
            }
        }
        emit q->deviceDialogClosed(id, closeReason);
        disconnectDialogSignals(dialog.widget());
        mDialogs.remove(dialog);
        removeRegionRect(id);
    }
    showDialogs();
    setupWindowRegion();
    updateStatus();
    
    // Make sure there is no fixed orientation
    if (mDialogs.isEmpty()) {       
        mMainWindow->unsetOrientation(false);
        mWsOriListener.stop();
    }

    controlForeground(securityDialog ? FgControlSecurityCriticalClosed : FgControlNone);

    TRACE_EXIT
}

bool HbDeviceDialogManagerPrivate::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    // Activate to correct ordinal position. e.g. Telephone application has gone to background and
    // we are in security level.    
    if (event->type() == QEvent::ApplicationActivate) {
        controlForeground();
    }
    return QObject::eventFilter(obj, event);
}

// Called by device dialog widget to inform manager of closed device dialog
void HbDeviceDialogManagerPrivate::deviceDialogClosed()
{
    TRACE_ENTRY
    const HbDeviceDialogsContainer::Dialog &current = mDialogs.find(sender());
    deleteDeviceDialog(current.isValid() ?
        current.id() : HbDeviceDialogsContainer::Dialog::InvalidId);
    TRACE_EXIT
}

void HbDeviceDialogManagerPrivate::indicatorActivated(
        const IndicatorClientInfo &clientInfo)
{
    TRACE_ENTRY
    QList<IndicatorClientInfo> indicatorInfoList;
    indicatorInfoList.append(clientInfo);
    emit q->indicatorActivated(indicatorInfoList);
    TRACE_EXIT
}

void HbDeviceDialogManagerPrivate::indicatorUpdated(const IndicatorClientInfo &clientInfo)
{
    TRACE_ENTRY
    QList<IndicatorClientInfo> indicatorInfoList;
    indicatorInfoList.append(clientInfo);
    emit q->indicatorUpdated(indicatorInfoList);
    TRACE_EXIT
}

void HbDeviceDialogManagerPrivate::indicatorRemoved(
        const IndicatorClientInfo &clientInfo)
{
    TRACE_ENTRY
    QList<IndicatorClientInfo> indicatorInfoList;
    indicatorInfoList.append(clientInfo);
    emit q->indicatorRemoved(indicatorInfoList);
    TRACE_EXIT
}

#if defined(Q_OS_SYMBIAN)

void HbDeviceDialogManagerPrivate::addRegionRect(int widgetId,
        const QRectF &rect)
{
    TRect trect(QRectToTRect(rect));
    mWindowRegion.AddRect(trect);
    mRegionList.append(RegionMapping(widgetId, trect));
    setupWindowRegion();
}

void HbDeviceDialogManagerPrivate::removeRegionRect(int widgetId)
{
    RegionMapping search(widgetId, TRect());
    int i = mRegionList.indexOf(search);
    if (i >= 0) {
        mRegionList.removeAt(i);
        // No way to remove a rect from windowregion-list, so clear
        // and make new copy.
        mWindowRegion.Clear();
        Q_FOREACH(const RegionMapping &mapping, mRegionList) {
            mWindowRegion.AddRect(mapping.mRegion);
        }
    }
}

void HbDeviceDialogManagerPrivate::enableReceiptOfFocus(bool enable) const
{
    CCoeEnv::Static()->RootWin().EnableReceiptOfFocus(enable);
}

// Turn display lights on
void HbDeviceDialogManagerPrivate::refreshDisplayLightsTime()
{
    User::ResetInactivityTime();
}

#else
void HbDeviceDialogManagerPrivate::addRegionRect(int widgetId,
        const QRectF &rect)
{
    Q_UNUSED(widgetId);
    Q_UNUSED(rect);
}

void HbDeviceDialogManagerPrivate::removeRegionRect(int widgetId)
{
    Q_UNUSED(widgetId);
}

void HbDeviceDialogManagerPrivate::enableReceiptOfFocus(bool enable) const
{
    Q_UNUSED(enable);
}

// Turn display lights on
void HbDeviceDialogManagerPrivate::refreshDisplayLightsTime()
{
}
#endif

// Ensure that device dialogs close as expected. A dialog is deleted if does not close.
bool HbDeviceDialogManagerPrivate::doHousekeeping()
{
    const HbDeviceDialogsContainer::Dialog::Flags closeCalled(
            HbDeviceDialogsContainer::Dialog::CloseCalled);
    const HbDeviceDialogsContainer::Dialog::Flags noClient(
            HbDeviceDialogsContainer::Dialog::NoClient);
    const HbDeviceDialogsContainer::Dialog::Flags showing(
            HbDeviceDialogsContainer::Dialog::Showing);

    const HbDeviceDialogsContainer::Dialog start;

    // Loop over dialogs having their client (session) closed and increase their housekeeping counters
    HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start, noClient, noClient);
    const HbDeviceDialogsContainer::Dialog::Variable noClientCount =
        HbDeviceDialogsContainer::Dialog::NoClientHousekeeping;
    while(current->isValid() && (current->flags() & showing)) {
        current->setVariable(noClientCount, current->intVariable(noClientCount) + 1);
        // Find next one
        current = &mDialogs.next(*current, noClient, noClient);
    }

    // Loop over dialogs that have been closed and increase their housekeeping counters
    current = &mDialogs.next(start, closeCalled, closeCalled);
    const HbDeviceDialogsContainer::Dialog::Variable closedCount =
        HbDeviceDialogsContainer::Dialog::CloseHousekeeping;
    while(current->isValid()) {
        current->setVariable(closedCount, current->intVariable(closedCount) + 1);
        current->setVariable(noClientCount, 0);
        // Find next one
        current = &mDialogs.next(*current, closeCalled, closeCalled);
    }

    // Close dialogs without a client that have passed time limit
    for(;;) {
        HbDeviceDialogsContainer::Dialog &dialog = mDialogs.next(start,
            noClientCount, MaxSessionlessDialogLife);
        if (dialog.isValid()) {
            dialog.setVariable(noClientCount, 0);
            closeDeviceDialog(dialog.id(), false);
        } else {
            break;
        }
    }

    // Delete closed dialogs that haven't given deviceDialogClosed() signal within a time limit
    for(;;) {
        const HbDeviceDialogsContainer::Dialog &dialog = mDialogs.next(start,
            closedCount, MaxDialogClosingPeriod);
        if (dialog.isValid()) {
            deleteDeviceDialog(dialog.id());
        } else {
            break;
        }
    }
    
    // Return true if housekeeping needs to continue
    return mDialogs.next(start, closeCalled, closeCalled).isValid() ||
        mDialogs.next(start, noClient, noClient).isValid();
}

// Check if dialog with a type is showing (not closed yet)
bool HbDeviceDialogManagerPrivate::isShowing(const QString &type)
{
    const HbDeviceDialogsContainer::Dialog start;

    const HbDeviceDialogsContainer::Dialog::Flags closeCalled(
            HbDeviceDialogsContainer::Dialog::CloseCalled);
    const HbDeviceDialogsContainer::Dialog::Flags showing(
            HbDeviceDialogsContainer::Dialog::Showing);

    const HbDeviceDialogsContainer::Dialog::Variable dialogType =
        HbDeviceDialogsContainer::Dialog::DialogType;
    const HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start, dialogType, type);
    while(current->isValid()){
        if ((current->flags() & (closeCalled|showing)) == showing) {
            return true;
        }
        // Find next one
        current = &mDialogs.next(*current, dialogType, type);
    }
    return false;
}

// Handle timer event
void HbDeviceDialogManagerPrivate::timerEvent(QTimerEvent *event)
{
    if (mHousekeeperTimerId == event->timerId()) {
        if (!doHousekeeping()) {
            killTimer(mHousekeeperTimerId); mHousekeeperTimerId = 0;
        }
    }
}

// Mark device dialogs launched by a client with noClient flag
void HbDeviceDialogManagerPrivate::markNoClient(quintptr clientTag)
{
    // Loop over dialogs
    const HbDeviceDialogsContainer::Dialog start;
    HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start,
        HbDeviceDialogsContainer::Dialog::ClientTag, clientTag);
    while(current->isValid()) {
        // Set client tag to 0 and mark session closed
        current->setVariable(HbDeviceDialogsContainer::Dialog::ClientTag, 0);
        current->setFlags(HbDeviceDialogsContainer::Dialog::NoClient);
        startHousekeeperTimer();
        // Find next one
        current = &mDialogs.next(*current, HbDeviceDialogsContainer::Dialog::ClientTag,
            clientTag);
    }
}

// Connect indicators status (activate/deactivate) signals to device dialog (indicator menu
// and screen saver)
void HbDeviceDialogManagerPrivate::connectIndicatorStatus(HbDeviceDialogInterface *dialogInterface)
{

    QObject *receiver = dialogSignaler(dialogInterface);
    mIndicatorPluginManager->disconnect(receiver);

    // Connect indicator plugin manager signals to device dialog slots for it to get
    // indicator updates
    receiver->connect(mIndicatorPluginManager, SIGNAL(indicatorActivated(HbIndicatorInterface*)),
        SLOT(indicatorActivated(HbIndicatorInterface*)));
    receiver->connect(mIndicatorPluginManager, SIGNAL(indicatorRemoved(HbIndicatorInterface*)),
        SLOT(indicatorDeactivated(HbIndicatorInterface*)));

    mIndicatorPluginManager->signalActiveIndicators(receiver);
}

// Disconnect device dialog signals
void HbDeviceDialogManagerPrivate::disconnectDialogSignals(HbDeviceDialogInterface *dialogInterface)
{
    dialogSignaler(dialogInterface)->disconnect(this); // disconnect signals
}

// Return device dialog signals source/target
QObject *HbDeviceDialogManagerPrivate::dialogSignaler(HbDeviceDialogInterface *dialogInterface)
{
    // Plugin can specify signal source/target by signalSender() or allow default which is device
    // dialog widget
    QObject *signalSourceAndTarget = dialogInterface->signalSender();
    if (!signalSourceAndTarget) {
        signalSourceAndTarget = dialogInterface->deviceDialogWidget();
    }
    return signalSourceAndTarget;
}

void HbDeviceDialogManagerPrivate::wsOrientationChanged(int renderOrientation)
{
    // The P&S property published by wserv indicates that the foreground app
    // (better said, the app below the dialog server) has changed
    // orientation. Follow it, and use animation.
    Qt::Orientation orientation;
    if (HbOrientationStatus::mapRenderOriToOri(renderOrientation, orientation)) {
        mMainWindow->setOrientation(orientation);
    }
}
