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

#include <hbmainwindow.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <QGraphicsLinearLayout>
#include <hbthemeutils_p.h>
#include <hbcombobox.h>
#include <hbmessagebox.h>
#include "settingsview.h"

/**
 * Constructor
 */
SettingsView::SettingsView(const QStringList &themes, HbMainWindow *mainWindow, HbView* parent): HbView(parent),
                                                                                                 mThemes(themes)
{
    mMainWindow = mainWindow;
    mParentView = parent;

    for(int i=0;i<mThemes.count();i++) {
        mThemeUiNames += QDir(mThemes[i]).dirName();
    }
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);

    this->setTitle(tr("Settings"));

    mDataForm = new HbDataForm(this);
    HbDataFormModel *model = new HbDataFormModel(this);

    mBaseThemeSetting = model->appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, tr("Base Theme"));
    mDefaultThemeSetting = model->appendDataFormItem(HbDataFormModelItem::RadioButtonListItem, tr("Default Theme"));
    mPriorityThemeSetting = model->appendDataFormItem(HbDataFormModelItem::TextItem, tr("Priority Theme"));
    mDataForm->setModel(model);

    loadSettings();

    layout->addItem(mDataForm);

    HbAction *act = new HbAction(tr("Save"));
    connect(act, SIGNAL(triggered()), this, SLOT(saveSettings()));
    this->toolBar()->addAction(act);
    act = new HbAction(tr("Cancel"));
    connect(act, SIGNAL(triggered()), this, SLOT(cancel()));
    this->toolBar()->addAction(act);
    act = new HbAction(Hb::BackNaviAction);
    connect(act, SIGNAL(triggered()), this, SLOT(cancel()));
    this->setNavigationAction(act);

    this->setLayout(layout);

    mMainWindow->addView(this);
}

/**
 * Destructor
 */
SettingsView::~SettingsView()
{
    delete mDataForm;
}

void SettingsView::loadSettings()
{
    mBaseThemeSetting->setContentWidgetData("items",mThemeUiNames);
    mBaseThemeSetting->setContentWidgetData("selected", mThemes.indexOf(HbThemeUtils::getThemeSetting(HbThemeUtils::BaseThemeSetting)));
    mDefaultThemeSetting->setContentWidgetData("items",mThemeUiNames);
    mDefaultThemeSetting->setContentWidgetData("selected", mThemes.indexOf(HbThemeUtils::getThemeSetting(HbThemeUtils::DefaultThemeSetting)));
    mPriorityThemeSetting->setContentWidgetData("text",HbThemeUtils::getThemeSetting(HbThemeUtils::OperatorNameSetting));
}

void SettingsView::saveSettings()
{
    HbThemeUtils::setThemeSetting(HbThemeUtils::BaseThemeSetting, mThemes[mBaseThemeSetting->contentWidgetData("selected").toInt()]);
    HbThemeUtils::setThemeSetting(HbThemeUtils::DefaultThemeSetting, mThemes[mDefaultThemeSetting->contentWidgetData("selected").toInt()]);
    HbThemeUtils::setThemeSetting(HbThemeUtils::OperatorNameSetting, mPriorityThemeSetting->contentWidgetData("text").toString());
    loadSettings();
#ifdef Q_OS_SYMBIAN
    HbMessageBox::information(tr("Please reboot the device to apply the changes."));
#else
    HbMessageBox::information(tr("Please restart the Hb application to apply the changes."));
#endif
    mMainWindow->setCurrentView(mParentView);
}

void SettingsView::cancel()
{
    loadSettings();
    mMainWindow->setCurrentView(mParentView);
}
