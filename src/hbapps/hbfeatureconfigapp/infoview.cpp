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

#include <hbdocumentloader.h>
#include <hbdeviceprofile.h>
#include <hblabel.h>
#include <hbmainwindow.h>

#include <hbmenu.h>
#include <hbaction.h>
#include <hbapplication.h>
#include <hbtoolbar.h>

#include "infoview.h"

InfoView::InfoView(HbDocumentLoader* loader, HbMainWindow *window) : 
    mWindow(window), 
    mLoader(loader),
    mResolutionLabel(0),
    mOrientationLabel(0),
    mLayoutDirectionLabel(0)
{
    setupLayout();
    refreshLabels();
    connect(mWindow, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(updateLayout()));
    connect(mWindow, SIGNAL(currentViewChanged(HbView*)), this, SLOT(refreshLabels()));
    setTitle("Current values");
    
    HbToolBar *toolbar = new HbToolBar;
    HbAction *action = new HbAction("settings");
    connect(action, SIGNAL(triggered()), SLOT(goToSettingsView()));
    toolbar->addAction(action);
    setToolBar(toolbar);
}

void InfoView::setupLayout()
{
    HbDeviceProfile currentProfile = HbDeviceProfile::profile(mWindow);
    QString section = currentProfile.orientation() == Qt::Horizontal ? "landscape" : "portrait";
    mLoader->load(QString(":/appxml/hbs60deviceconfigapp.docml"), section);
    QGraphicsWidget* infoWidget = mLoader->findWidget("infoView");
    if (infoWidget) {
        QGraphicsWidget* oldWidget = takeWidget();
        if (oldWidget)
            oldWidget->setVisible(false);
        infoWidget->setVisible(true);
        setWidget(infoWidget);
    }
    
    mResolutionLabel = qobject_cast<HbLabel *>(mLoader->findWidget("resolutionInfoValue"));
    mOrientationLabel = qobject_cast<HbLabel *>(mLoader->findWidget("orientationInfoValue"));
    mLayoutDirectionLabel = qobject_cast<HbLabel *>(mLoader->findWidget("directionInfoValue"));
}

void InfoView::refreshLabels()
{
    HbDeviceProfile currentProfile = HbDeviceProfile::profile(mWindow);
    mResolutionLabel->setPlainText(currentProfile.name());
    mOrientationLabel->setPlainText(currentProfile.orientation() == Qt::Vertical ? QString("Portrait") : QString("Landscape"));
    mLayoutDirectionLabel->setPlainText(HbApplication::layoutDirection() == Qt::LeftToRight ? "Left to Right" : "Right to Left");
}

void InfoView::updateLayout()
{
    setupLayout();
    refreshLabels();
}

void InfoView::goToSettingsView()
{
    mainWindow()->setCurrentView(mSettingsView);
}
    
void InfoView::setSettingsView(HbView *view)
{
    mSettingsView = view;
}
