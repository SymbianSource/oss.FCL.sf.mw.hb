/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbApps module of the UI Extensions for Mobile.
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

#include <QStringListModel>

#include <hbcombobox.h>
#include <hbdeviceprofile.h>
#include <hbdocumentloader.h>
#include <hbmainwindow.h>
#include <hbinstance.h>
#include <hbapplication.h>
#include <hbscrollarea.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <restricted/hbfeaturemanager_r.h>

#include "settingsview.h"

#if defined(Q_OS_SYMBIAN)
#include <hbs60events.h>
#include <w32std.h>
#include <hbdeviceprofile.h>
#endif

SettingsView::SettingsView(HbDocumentLoader* loader, HbMainWindow *window, HbView *infoView) :
    mWindow(window),
    mLoader(loader),
    mResolutionComboBox(0),
    mOrientationComboBox(0),
    mLayoutDirectionComboBox(0),
    mTouchAreaComboBox(0),
    mTextItemComboBox(0),
    mIconItemComboBox(0),
    mLocalizationComboBox(0),
    mTestUtilityComboBox(0),
    mFpsCounterComboBox(0),
    mInfoView(infoView)
{
    setupMenu();
    setupLayout();
    updateSelections();
    connect(mWindow, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(updateLayout()));
    connect(mWindow, SIGNAL(currentViewChanged(HbView*)), this, SLOT(updateSelections()));
    setTitle("Settings");
    HbAction *backAction = new HbAction(Hb::BackNaviAction, this);
    connect(backAction, SIGNAL(triggered()), SLOT(goBack()));
    setNavigationAction(backAction);
}

void SettingsView::goBack()
{
    mWindow->setCurrentView(mInfoView);
}
    
void SettingsView::setupMenu()
{
    HbMenu *menu = new HbMenu();
    HbAction *a;
    a = menu->addAction( "Change orientation" );
    connect(a, SIGNAL(triggered()), SLOT(changeOrientation()));
    setMenu(menu);
}


void SettingsView::setupLayout()
{
    disconnectSignals();

    HbDeviceProfile currentProfile = HbDeviceProfile::profile(mWindow);
    QString section = currentProfile.orientation() == Qt::Horizontal ? "landscape" : "portrait";
    mLoader->load(QString(":/appxml/hbs60deviceconfigapp.docml"), section);

    QGraphicsWidget *settingsWidget = mLoader->findWidget("settingsView");
    if (settingsWidget) {
        QGraphicsWidget* oldWidget = takeWidget();
        if (oldWidget)
            oldWidget->setVisible(false);
        settingsWidget->setVisible(true);
        setWidget(settingsWidget);
    }
    
    if( !mResolutionComboBox ) {
        mResolutionComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("resolutionSettingCombo"));
        mResolutionComboBox->setModel(new QStringListModel(HbDeviceProfile::profileNames()));
    }

    if( !mOrientationComboBox ) {
        mOrientationComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("orientationSettingCombo"));
        mOrientationComboBox->setModel(new QStringListModel(QStringList() << "Portrait" << "Landscape"));
    }

    if( !mLayoutDirectionComboBox ) {
        mLayoutDirectionComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("directionSettingCombo"));
        mLayoutDirectionComboBox->setModel(new QStringListModel(QStringList() << "Left to Right" << "Right to Left"));
    }

    if( !mTouchAreaComboBox ) {
        mTouchAreaComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("touchareaSettingCombo"));
        mTouchAreaComboBox->setModel(new QStringListModel(QStringList() << tr("Invisible") << tr("Visible")));
    }

    if( !mTextItemComboBox ) {
        mTextItemComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("textitemSettingCombo"));
        mTextItemComboBox->setModel(new QStringListModel(QStringList() << tr("Invisible") << tr("Visible")));
    }

    if( !mIconItemComboBox ) {
        mIconItemComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("iconitemSettingCombo"));
        mIconItemComboBox->setModel(new QStringListModel(QStringList() << tr("Invisible") << tr("Visible")));
    }
    
    if (!mFpsCounterComboBox) {
        mFpsCounterComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("fpscounterSettingCombo"));
        mFpsCounterComboBox->setModel(new QStringListModel(QStringList() << tr("Invisible") << tr("Visible")));
    }

    if( !mLocalizationComboBox ) {
        mLocalizationComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("localizationSettingCombo"));
        mLocalizationComboBox->setModel(new QStringListModel(QStringList() << tr("Disabled") << tr("Enabled") << tr("Automatic")));
    }

    int textMeasurementValue = HbFeatureManager::instance()->featureStatus( HbFeatureManager::TextMeasurement );
    if ( textMeasurementValue > 2 ) {
        textMeasurementValue = 1;
    }
    mLocalizationComboBox->setCurrentIndex( textMeasurementValue );

    if( !mTestUtilityComboBox ) {
        mTestUtilityComboBox = qobject_cast<HbComboBox *>(mLoader->findWidget("testUtilitySettingCombo"));
        mTestUtilityComboBox->setModel(new QStringListModel(QStringList() << tr("Disabled") << tr("Enabled")));
    }
    mTestUtilityComboBox->setCurrentIndex( HbFeatureManager::instance()->featureStatus( HbFeatureManager::TheTestUtility ) ? 1 : 0 );

    connectSignals();
}

void SettingsView::changeLocMode(int index)
{
    HbFeatureManager::instance()->setFeatureStatus( HbFeatureManager::TextMeasurement, index );
}

void SettingsView::changeTestUtilMode(int index)
{
    HbFeatureManager::instance()->setFeatureStatus( HbFeatureManager::TheTestUtility, index );
}

void SettingsView::changeResolution(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeDeviceProfile, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::changeOrientation(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeOrientation, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::changeLayoutDirection(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeDirection, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::changeTouchAreaVisibility(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeTouchAreaVis, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::changeTextItemVisibility(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeTextItemVis, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::changeIconItemVisibility(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeIconItemVis, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::changeFpsCounterVisibility(int index)
{
#if defined(Q_OS_SYMBIAN)
    sendEvent(KChangeFpsCounterVis, index);
#else
    Q_UNUSED(index);
#endif
}

void SettingsView::sendEvent(const int eventID, const int data)
{
#if defined(Q_OS_SYMBIAN)
    RWsSession session;
    User::LeaveIfError( session.Connect() );
    TWsEvent event;
    event.SetType(eventID);
    TUint8* dataptr = event.EventData();
    *dataptr = data;
    session.SendEventToAllWindowGroups(event);
    session.Close();
#else
    Q_UNUSED(eventID);
    Q_UNUSED(data);
#endif
}


void SettingsView::updateLayout()
{
    setupLayout();
    updateSelections();
}

void SettingsView::updateSelections()
{
    disconnectSignals();
    HbDeviceProfile currentProfile = HbDeviceProfile::profile(mWindow);
    mIconItemComboBox->setCurrentIndex(0); // TODO: Can't currently get value from private code
    mTextItemComboBox->setCurrentIndex(0); // TODO: Can't currently get value from private code
    mTouchAreaComboBox->setCurrentIndex(0); // TODO: Can't currently get value from private code
    mLayoutDirectionComboBox->setCurrentIndex(HbApplication::layoutDirection() == Qt::LeftToRight ? 0 : 1);
    mOrientationComboBox->setCurrentIndex(currentProfile.orientation() == Qt::Vertical ? 0 : 1);
    mResolutionComboBox->setCurrentIndex(HbDeviceProfile::profileNames().indexOf(currentProfile.name()));
    mFpsCounterComboBox->setCurrentIndex(0); // off by default
    connectSignals();
}

void SettingsView::connectSignals()
{
    connect(mIconItemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIconItemVisibility(int)));
    connect(mTextItemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTextItemVisibility(int)));
    connect(mTouchAreaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTouchAreaVisibility(int)));
    connect(mLayoutDirectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLayoutDirection(int)));
    connect(mOrientationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeOrientation(int)));
    connect(mResolutionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeResolution(int)));
    connect(mFpsCounterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFpsCounterVisibility(int)));
    connect(mLocalizationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLocMode(int)));
    connect(mTestUtilityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTestUtilMode(int)));
}

void SettingsView::disconnectSignals()
{
    disconnect(mIconItemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeIconItemVisibility(int)));
    disconnect(mTextItemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTextItemVisibility(int)));
    disconnect(mTouchAreaComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTouchAreaVisibility(int)));
    disconnect(mLayoutDirectionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLayoutDirection(int)));
    disconnect(mOrientationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeOrientation(int)));
    disconnect(mResolutionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeResolution(int)));
    disconnect(mFpsCounterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFpsCounterVisibility(int)));
    disconnect(mLocalizationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLocMode(int)));
    disconnect(mTestUtilityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTestUtilMode(int)));
}

void SettingsView::changeOrientation()
{
    if (mWindow)
        mWindow->setOrientation(mWindow->orientation() == Qt::Vertical ? Qt::Horizontal : Qt::Vertical);
}
