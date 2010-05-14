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

#ifndef HBMAINWINDOW_P_H
#define HBMAINWINDOW_P_H

#include <QList>
#include <QTimeLine>
#include <QGraphicsWidget>
#include <QPointer>
#include <QTranslator>

#include "hbglobal.h"
#include "hbmainwindow.h"
#include "hbtitlepane_p.h"
#include "hbwidget_p.h"
#include "hbwidget.h"
#include "hbnamespace_p.h"
#include "hbdeviceprofile.h"
#include "hbeffect.h"
#include "hbeffectinternal_p.h"

class HbBackgroundItem;
class HbGraphicsScene;
class HbTitleBar;
class HbStatusBar;
class HbToolBar;
class HbView;
class HbScreen;
class HbDockWidget;
class HbContentWidget;
class HbTheTestUtility;

#ifdef Q_OS_SYMBIAN
class HbNativeWindow;
#endif

class HB_CORE_PRIVATE_EXPORT HbMainWindowPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(HbMainWindow)

public:

    enum Element {
        RootItem,
        ViewportItem,
        BackgroundItem
    };

    HbMainWindowPrivate();
    virtual ~HbMainWindowPrivate();

    void init();
    void initTranslations();

    HbToolBar *toolBar() const;
    void addToolBarToLayout(HbToolBar *toolBar);
    void removeToolBarFromLayout(HbToolBar *toolBar);
    HbDockWidget *dockWidget() const;
    void addDockWidgetToLayout(HbDockWidget *dockWidget);
    void removeDockWidgetFromLayout(HbDockWidget *dockWidget);
    void updateVisibleItems();
    QRectF contentRect() const;
    void setTransformedOrientation(Qt::Orientation orientation, bool animate = true);
    qreal rotation() const;

    void select(const HbDeviceProfile &profile, HbDeviceProfile *oldGlobalProfile = 0);
    HbDeviceProfile profile() const;
    HbDeviceProfile adjustedProfile(const HbDeviceProfile &profile) const;

    void broadcastEvent(int eventType);
    void broadcastEvent(QEvent* event);

    void changeSceneSize();
    void updateRotationEffects();
    void addBackgroundItem();
    void removeBackgroundItem();

    void postIdleEvent(int eventId);

    void fadeScreen(qreal zValue);
    void unfadeScreen();

    void _q_viewReady();
        
    QGraphicsWidget *element(HbMainWindowPrivate::Element element) const;

    HbGraphicsScene *mScene;
    HbBackgroundItem *mBgItem;
    HbScreen *mClippingItem;
    HbContentWidget *mViewStackWidget;
    HbTitleBar *mTitleBar;
    HbStatusBar *mStatusBar;
    HbWidget *mFadeItem;
    QGraphicsWidget *mRootItem;
    QGraphicsWidget *mEffectItem;
    Qt::Orientation mDefaultOrientation;
    qreal mOrientationAngle;
    QList<QGraphicsItem*> mItemList;
    QList<QGraphicsItem*> mOrientationChangeEffectItems;
    bool mAutomaticOrientationSwitch;
    bool mUserOrientationSwitch;
    bool mOrientationChangeOngoing;
    bool mAnimateOrientationSwitch;
    bool mGVOrientationChangeEffectEnabled;
    Qt::Orientation mOrientation;
    Qt::Orientation mRequestedOrientation;
    HbToolBar *mCurrentToolbar;
    HbDockWidget *mCurrentDockWidget;
    Hb::SceneItems mVisibleItems;
    bool mForceSetOrientation; // for resize case
    typedef QPointer<QGraphicsWidget> QGraphicsWidgetPtr;
    struct BroadcastItem {
        QEvent *mEvent;
        QList<QGraphicsWidgetPtr> mPending;
    };
    QMap<int, BroadcastItem> mBroadcastItems;
    HbDeviceProfile mCurrentProfile;
    bool mDelayedConstructionHandled;
    HbMainWindow *q_ptr;
    HbTheTestUtility *mTheTestUtility;
    HbGVWrapperItem mGVWrapperItem;
    bool mIdleEventHandled;
    QRectF mLayoutRect;
    mutable HbDeviceProfile mAlternateProfile;
    QPointer<HbView> mMenuView;
    bool mNotifyOrientationChange;
    bool mOrientationChangeNotified;
    bool mToolbarWasAdded;
    bool mAutomaticOrientationChangeAnimation;
    QTranslator mCommonTranslator;
#ifdef Q_OS_SYMBIAN
    HbNativeWindow *mNativeWindow;
#endif

    void rootItemFirstPhaseDone(const HbEffect::EffectStatus& status);
    void rootItemFinalPhaseDone(const HbEffect::EffectStatus& status);
    void orientationEffectFinished(const HbEffect::EffectStatus& status);

    void addOrientationChangeEffects();
    void addViewEffects();
    void _q_viewChanged();
    void _q_viewRemoved(QGraphicsWidget *widget);
    void _q_viewTitleChanged(const QString &title);
    void _q_viewToolBarChanged();
    void _q_viewDockWidgetChanged();
    void _q_launchMenu(const QPointF &pos);
    void _q_viewVisibleItemsChanged();
    void _q_contentFullScreenChanged();
    void _q_themeChanged();
    void _q_continueBroadcasting(int type);
    void _q_restoreTitlePane();
    void _q_delayedConstruction();

    static bool dragToResizeEnabled;
    static bool initializeInputs;

    static const int IdleEvent;
    static const int IdleOrientationEvent;
    static const int IdleOrientationFinalEvent;

signals:
    void idleEventDispatched();

public slots:
    void menuClosed();

private:
    static HbMainWindowPrivate *d_ptr(HbMainWindow *mainWindow) {
        Q_ASSERT(mainWindow);
        return mainWindow->d_func();
    }
    friend class HbPopupManagerPrivate;
    friend class HbInstance;
    friend class HbInstancePrivate;
    friend class HbDeviceProfileManager;
    friend class HbDeviceProfile;
    friend class HbView;
    friend class HbVgEffectPrivate;
    friend class HbContentWidget;
    friend class HbSplashGenerator;
    friend class TestHbDeviceProfile;
    friend class TestHbGridView;
    friend class TestHbMainWindow;
    friend class HbMainWindowOrientation;
    friend class HbScreen;
    friend class HbSettingsWindow;
    friend class TestHbSensorOrientation;
};

#endif // HBMAINWINDOW_P_H
