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

#if defined(Q_OS_SYMBIAN)
#include <coemain.h>
#include <coecntrl.h>
#include <e32cmn.h>
#else
#include "hbindicatorwin32_p.h"
#endif // Q_OS_SYMBIAN

//local symbian helper functions
#if defined(Q_OS_SYMBIAN)
TRect QRectToTRect(const QRectF &rect)
{
    TRect trect;
    trect.SetRect(rect.topLeft().x(), rect.topLeft().y(),
               rect.bottomRight().x() + 1, rect.bottomRight().y() + 1);
    return trect;
}

QRectF MousePressCatcher::boundingRect () const
{
    return QRectF();
}

void MousePressCatcher::paint ( QPainter */*painter*/,
                          const QStyleOptionGraphicsItem */*option*/,
                          QWidget */*widget*/)
{

}

bool MousePressCatcher::sceneEventFilter( QGraphicsItem * /*watched*/, QEvent * event )
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        mDeviceDialogManger->resetWindowRegion();
    }
    return false;
}

bool RegionUpdateFilter::eventFilter(QObject* obj, QEvent *event)
	{
	if (event->type() ==  QEvent::QEvent::GraphicsSceneResize) {
		HbPopup *popup = qobject_cast<HbPopup*>(obj);
		if (popup) {			
			QRectF rect = popup->rect();
			rect.moveTo(popup->pos());
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

    TRACE_EXIT
}

/*!
    \internal
    Destructor.
*/
HbDeviceDialogManagerPrivate::~HbDeviceDialogManagerPrivate()
{
    TRACE_ENTRY
    killTimer(mHousekeeperTimerId);
#if defined(Q_OS_SYMBIAN)
    mScene->removeItem(&mMousePressCatcher);
    mWindowRegion.Close();
#endif
    delete mIndicatorPluginManager;
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

    int id = 0;

    HbDeviceDialogPlugin::DeviceDialogInfo info;
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
    id = dialog.id();

    mScene->addItem(popup);

    if (info.group == HbDeviceDialogPlugin::DeviceNotificationDialogGroup) {
        // Disable HbNotificationDialog sequential show feature. Server takes care of
        // showing sequentially.
        popup->setProperty("sequentialShow", false);
    }

    showDialogs();

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
        bool success = interface->setDeviceDialogParameters(parameters);
        if (!success) {
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
        dialog.widget()->closeDeviceDialog(byClient);
        startHousekeeperTimer();
        ret = HbDeviceDialogNoError;
    }
    TRACE_EXIT
    return ret;
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
// Move application to fore/background
void HbDeviceDialogManagerPrivate::moveToForeground(bool foreground)
{
    TRACE_ENTRY_ARGS(foreground)
    if (foreground) {
        if(!mMainWindow->isVisible()) {
            mMainWindow->showFullScreen();
            RWindowGroup &rootWindowGroup = CCoeEnv::Static()->RootWin();
            const int positionForeground(0);
            rootWindowGroup.SetOrdinalPosition(positionForeground,
                ECoeWinPriorityAlwaysAtFront);
        }
    } else {
        if(mMainWindow->isVisible()) {
            mMainWindow->hide();
            RWindowGroup &rootWindowGroup = CCoeEnv::Static()->RootWin();
            const int positionBackground(-1);
            rootWindowGroup.SetOrdinalPosition(positionBackground, ECoeWinPriorityNormal);
        }
    }
    TRACE_EXIT
}

void HbDeviceDialogManagerPrivate::updateWindowRegion() const
{
    RWindowBase *win =
        static_cast<RWindowBase*>(mMainWindow->effectiveWinId()->DrawableWindow());
    if (win) {
        win->SetShape(mWindowRegion);
    }
}

void HbDeviceDialogManagerPrivate::resetWindowRegion() const
{
    RWindowBase *win =
        static_cast<RWindowBase*>(mMainWindow->effectiveWinId()->DrawableWindow());
    if (win) {
        RRegionBuf<1> windowRegion(QRectToTRect(mMainWindow->sceneRect()));
        win->SetShape(windowRegion);
    }
}

#else
// Move application to fore/background
void HbDeviceDialogManagerPrivate::moveToForeground(bool foreground)
{
    Q_UNUSED(foreground)
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

    // Loop over not showing dialogs
    HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start, noFlags,
        showing | closeCalled);
    while(current->isValid()) {
        if (current->flags() & notificationGroup) {
            if (!showingNotification) {
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

    // Lights control. If security layer is active, only security group or critical group turn lights on.
    // Otherwise lights turned on every time a dialog is shown.
    int lightsMask = HbDeviceDialogsContainer::Dialog::SecurityGroup |
        HbDeviceDialogsContainer::Dialog::CriticalGroup;
    if (!showingSecurity) {
        lightsMask |= HbDeviceDialogsContainer::Dialog::GenericGroup |
            HbDeviceDialogsContainer::Dialog::NotificationGroup |
            HbDeviceDialogsContainer::Dialog::IndicatorGroup;
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
            // Pointer events catched only inside dialog area, other events gets passed to
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
    moveToForeground(dialogsShowing);
}

/*!
    \internal
    Create a device dialog widget
*/
HbDeviceDialogInterface *HbDeviceDialogManagerPrivate::createDeviceDialog(
    HbDeviceDialogServer::DialogParameters &parameters,
    HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo, HbPopup *&popup)
{
    parameters.mError = HbDeviceDialogNoError;

    if (!mPluginManager.loadPlugin(parameters.mType)) {
        parameters.mError = HbDeviceDialogNotFoundError;
        return 0;
    }

    const HbDeviceDialogPlugin &plugin = mPluginManager.plugin(parameters.mType);
    if (!plugin.deviceDialogInfo(parameters.mType, parameters.mData, &deviceDialogInfo)) {
        // Ensure plugin returns valid error code
        parameters.mError = checkpluginerror(plugin.error());
        mPluginManager.unloadPlugin(parameters.mType);
        return 0;
    }

    if (!checkDialogInfo(deviceDialogInfo)) {
        parameters.mError = HbDeviceDialogGeneralError;
        mPluginManager.unloadPlugin(parameters.mType);
        return 0;
    }

    // Plugin may request security check of the client
    if (deviceDialogInfo.flags & HbDeviceDialogPlugin::SecurityCheck) {
        QVariantMap credentials;
        addSecurityCredentials(parameters, credentials);
        if (!plugin.accessAllowed(parameters.mType, parameters.mData, credentials)) {
            parameters.mError = HbDeviceDialogAccessDeniedError;
            return 0;
        }
    }

    // Performance optimization for indicator menu. Device dialog widget is reused after the
    // first time it's shown.
    bool recycled = (deviceDialogInfo.group == HbDeviceDialogPlugin::IndicatorGroup);
    HbDeviceDialogInterface *deviceDialogIf =
        mPluginManager.createWidget(parameters.mType, parameters.mData, recycled,
            parameters.mError);
    // Decrease plugin reference count increased by loadPlugin() above. Unload takes place when
    // device dialog widget is deleted.
    mPluginManager.unloadPlugin(parameters.mType);
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

    if (!recycled && deviceDialogInfo.group == HbDeviceDialogPlugin::IndicatorGroup) {
        // Connect plugin manager signals to indicator menu slots, so that
        // it will get indicator updates. If the widget is reused, signals
        // are already connected.
#if defined(Q_OS_SYMBIAN)
        mIndicatorPluginManager->connectTo(popup);
#else
        HbIndicatorPrivate::pluginManager()->connectTo(popup);
#endif
    }

    // Set popup priority
    setDialogPriority(popup, deviceDialogInfo.group);

    return deviceDialogIf;
}

// Update published server status
void HbDeviceDialogManagerPrivate::updateStatus()
{
    HbDeviceDialogServerStatus::StatusFlags status = HbDeviceDialogServerStatus::NoFlags;
    const HbDeviceDialogsContainer::Dialog start;
    const HbDeviceDialogsContainer::Dialog::Flags showingFlag(
        HbDeviceDialogsContainer::Dialog::Showing);
    HbDeviceDialogsContainer::Dialog *current = &mDialogs.next(start, showingFlag, showingFlag);
    if (current->isValid()) {
        status |= HbDeviceDialogServerStatus::ShowingDialog;
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
void HbDeviceDialogManagerPrivate::setDialogPriority(HbPopup *popup, HbDeviceDialogPlugin::DeviceDialogGroup group)
{
    // For notification, security and critical layer popups, set popup priority. Others use default.
    const quint8 CriticalPopupPriority = HbPopupPrivate::VirtualKeyboard - 1;
    const quint8 SecurityPopupPriority = HbPopupPrivate::VirtualKeyboard - 2;
    const quint8 NotificationPopupPriority = HbPopupPrivate::VirtualKeyboard  - 3;

    quint8 priority = HbPopupPrivate::Default;
    switch(group) {
    case HbDeviceDialogPlugin::DeviceNotificationDialogGroup:
        priority = NotificationPopupPriority; break;
    case HbDeviceDialogPlugin::SecurityGroup:
        priority = SecurityPopupPriority; break;
    case HbDeviceDialogPlugin::CriticalGroup:
        priority = CriticalPopupPriority; break;
    case HbDeviceDialogPlugin::GenericDeviceDialogGroup:
        // fall through
    case HbDeviceDialogPlugin::IndicatorGroup:
        // fall through
    default:
        ; // fall through
    }
    HbPopupPrivate::d_ptr(popup)->setPriority(priority);
}

// Check that DeviceDialogInfo returned by plugin is valid
bool HbDeviceDialogManagerPrivate::checkDialogInfo(
    const HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo)
{
    return deviceDialogInfo.group >= HbDeviceDialogPlugin::GenericDeviceDialogGroup &&
        deviceDialogInfo.group <= HbDeviceDialogPlugin::CriticalGroup;
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
    HbDeviceDialogsContainer::Dialog &current = mDialogs.find(id);
    if (current.isValid()) {
        // If device dialog was cancelled by client or server, give reason
        int closeReason = HbDeviceDialogNoError;
        if ((current.flags() & HbDeviceDialogsContainer::Dialog::CloseCalled)) {
            if ((current.flags() & HbDeviceDialogsContainer::Dialog::ClosedByServer)) {
                closeReason = static_cast<int>(HbDeviceDialog::SystemCancelledError);
            } else {
                closeReason = static_cast<int>(HbDeviceDialog::CancelledError);
            }
        }
        emit q->deviceDialogClosed(id, closeReason);
        mDialogs.remove(current);
        removeRegionRect(id);
    }
    showDialogs();
    setupWindowRegion();
    updateStatus();
    TRACE_EXIT
}

// Called by device dialog widget to inform manager of closed device dialog
void HbDeviceDialogManagerPrivate::deviceDialogClosed()
{
    TRACE_ENTRY
    HbDeviceDialogsContainer::Dialog &current = mDialogs.find(sender());
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
    while(current->isValid() && (current->flags() & showing)) {
        current->setVariable(HbDeviceDialogsContainer::Dialog::NoClientHousekeeping,
            current->variable(HbDeviceDialogsContainer::Dialog::NoClientHousekeeping) + 1);
        // Find next one
        current = &mDialogs.next(*current, noClient, noClient);
    }

    // Loop over dialogs that have been closed and increase their housekeeping counters
    current = &mDialogs.next(start, closeCalled, closeCalled);
    while(current->isValid()) {
        current->setVariable(HbDeviceDialogsContainer::Dialog::CloseHousekeeping,
            current->variable(HbDeviceDialogsContainer::Dialog::CloseHousekeeping) + 1);
        current->setVariable(HbDeviceDialogsContainer::Dialog::NoClientHousekeeping, 0);
        // Find next one
        current = &mDialogs.next(*current, closeCalled, closeCalled);
    }

    // Close dialogs without a client that have passed time limit
    for(;;) {
        HbDeviceDialogsContainer::Dialog &dialog = mDialogs.next(start,
            HbDeviceDialogsContainer::Dialog::NoClientHousekeeping, MaxSessionlessDialogLife);
        if (dialog.isValid()) {
            dialog.setVariable(HbDeviceDialogsContainer::Dialog::NoClientHousekeeping, 0);
            closeDeviceDialog(dialog.id(), false);
        } else {
            break;
        }
    }

    // Delete closed dialogs that haven't given deviceDialogClosed() signal within a time limit
    for(;;) {
        HbDeviceDialogsContainer::Dialog &dialog = mDialogs.next(start,
            HbDeviceDialogsContainer::Dialog::CloseHousekeeping, MaxDialogClosingPeriod);
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
