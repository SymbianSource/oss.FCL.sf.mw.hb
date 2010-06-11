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

#ifndef HBDEVICEDIALOGMANAGER_P_P_H
#define HBDEVICEDIALOGMANAGER_P_P_H

#include <QtGlobal>
#include <QObject>
#include <QVariantMap>
#include <QGraphicsItem>
#include <QTimerEvent>
#if defined(Q_OS_SYMBIAN)
#include <e32std.h>
#include <e32property.h>
#endif

#include "hbdevicedialogserver_p.h"
#include "hbindicatorclientinfo_p.h"
#include <hbdevicedialogplugin.h>
#include <hbdevicedialogserverstatus_p.h>
#include <hbdevicedialogpluginmanager_p.h>
#include <hbdevicedialogscontainer_p.h>

class HbDeviceDialogManager;
class HbIndicatorPluginManager;
class HbGraphicsScene;
class HbMainWindow;
class HbPopup;
class HbDeviceDialogInterface;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#if defined(Q_OS_SYMBIAN)
class HbDeviceDialogManagerPrivate;

class RegionUpdateFilter : public QObject
{
    Q_OBJECT
public:
    RegionUpdateFilter(HbDeviceDialogManagerPrivate *deviceDialogManger) :
    mDeviceDialogManger(deviceDialogManger) {}
protected:
    bool eventFilter(QObject* obj, QEvent *event);
private:	
    HbDeviceDialogManagerPrivate *mDeviceDialogManger;
};

//acts as a scene event filter to catch mouse press events.
//when caught, resets window region to full screen.
class MousePressCatcher : public QGraphicsItem
{
public:
    MousePressCatcher(HbDeviceDialogManagerPrivate *deviceDialogManger) :
        mDeviceDialogManger(deviceDialogManger) {hide();}
protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
            QWidget *widget = 0);
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
private:
    HbDeviceDialogManagerPrivate *mDeviceDialogManger; // not owned
};
#endif

class HbDeviceDialogManagerPrivate : public QObject
{
    Q_OBJECT

public:
    enum {
        HousekeeperTimerPeriod = 2000,
        MaxSessionlessDialogLife = 5, // 5 * HousekeeperTimerPeriod
        MaxDialogClosingPeriod = 2 // 2 * HousekeeperTimerPeriod
    };

    // Constructor and destructor
    explicit HbDeviceDialogManagerPrivate(HbDeviceDialogManager *manager);
    virtual ~HbDeviceDialogManagerPrivate();
    void init();
    void setMainWindow(HbMainWindow *mainWindow);

    // Device dialog client related API
    int showDeviceDialog(HbDeviceDialogServer::DialogParameters &parameters);
    int updateDeviceDialog(int id, const QVariantMap &parameters);
    int publishOrientation(int orientation);
    int closeDeviceDialog(int id, bool byClient = true);
    void deviceDialogClientClosing(quintptr clientTag);
    int activateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters);
    int deactivateIndicator(HbDeviceDialogServer::IndicatorParameters &parameters);
    int setDataToIndicator(const QString &indicatorType, const QVariant &data);
    QList<IndicatorClientInfo> indicatorClientInfoList() const;

    // Device dialog control related API
    void moveToForeground(bool foreground);
    void doMoveToForeground(bool foreground, int priority);
    void updateWindowRegion() const;
    void resetWindowRegion() const;
    bool showDialogs();

    // Utility methods
    HbDeviceDialogInterface *createDeviceDialog(HbDeviceDialogServer::DialogParameters &parameters,
        HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo, HbPopup *&popup);
    HbGraphicsScene *scene() const {return mScene;}
    void updateStatus();
    static int checkpluginerror(int errorCode);
    static void addSecurityCredentials(const HbDeviceDialogServer::Parameters &parameters,
        QVariantMap &credentials);
    static void setDialogPriority(HbPopup *popup, HbDeviceDialogPlugin::DeviceDialogGroup group);
    static bool checkDialogInfo(const HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo);
    void deleteDeviceDialog(int id);
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void deviceDialogUpdate(const QVariantMap &data) const;
    void deviceDialogClosed();
    void indicatorActivated(const IndicatorClientInfo &clientInfo);
    void indicatorUpdated(const IndicatorClientInfo &clientInfo);
    void indicatorRemoved(const IndicatorClientInfo &clientInfo);

private:
    void addRegionRect(int widgetId, const QRectF &rect);
    void removeRegionRect(int widgetId);
    void enableReceiptOfFocus(bool enable) const;
    void refreshDisplayLightsTime();
    void startHousekeeperTimer()
        {if (mHousekeeperTimerId == 0) mHousekeeperTimerId = startTimer(HousekeeperTimerPeriod);}
    bool doHousekeeping();
    bool isShowing(const QString &type);
    void timerEvent(QTimerEvent *event);
    void markNoClient(quintptr clientTag);
    void setupWindowRegion();
private:
    // Public interface
    HbDeviceDialogManager * const q;

    HbDeviceDialogPluginManager mPluginManager;
    HbIndicatorPluginManager *mIndicatorPluginManager;
    HbDeviceDialogsContainer mDialogs;

    HbMainWindow *mMainWindow;
    HbGraphicsScene *mScene;
    HbDeviceDialogServerStatus mServerStatus;
    int mHousekeeperTimerId;
#if defined(Q_OS_SYMBIAN)
    MousePressCatcher mMousePressCatcher; //acts as a scene event filter to catch mouse press events.
    friend class RegionUpdateFilter;
    RegionUpdateFilter mRegionUpdateFilter;

    struct RegionMapping
    {
        RegionMapping(int widgetId, const TRect &region) :
            mId(widgetId), mRegion(region) {}
        bool operator ==(const RegionMapping &other) const {return mId == other.mId;}

        int mId;
        TRect mRegion;
    };
    QList<RegionMapping> mRegionList;
    RRegion mWindowRegion;
    RProperty mProperty;    
#endif
};

#endif // HBDEVICEDIALOGMANAGER_P_P_H
