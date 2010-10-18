/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbInput module of the UI Extensions for Mobile.
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
#include "hbinputsettingpopup.h"
#include <hbview.h>
#include <hbinputpopupbase_p.h>
#include <hbinputregioncollector_p.h>
#include <hbpushbutton.h>
#include <QGraphicsLinearLayout>
#include <hbdataform.h>
#include <hbinputsettingwidget.h>
#include <hbmainwindow.h>

/*!
@stable
@hbinput
\class HbInputSettingPopup
\brief Popup for changing input settings in any application.

Displays the same content as control panel, but inside a popup.

\sa HbInputSettingWidget
*/

const QPoint HbSettingPopupLocation(3,3);
const QSize HbBackButtonSize(70, 45);

class HbInputSettingPopupPrivate: public HbInputPopupBasePrivate
{
    Q_DECLARE_PUBLIC(HbInputSettingPopup)

public:
    HbInputSettingPopupPrivate();
    ~HbInputSettingPopupPrivate();

public:
    HbView* mSettingView;
};

HbInputSettingPopupPrivate::HbInputSettingPopupPrivate(): mSettingView(0)
{
}

HbInputSettingPopupPrivate::~HbInputSettingPopupPrivate()
{
}

HbInputSettingPopup::HbInputSettingPopup(QGraphicsWidget *parent)
: HbInputPopupBase(*new HbInputSettingPopupPrivate(), parent)
{
    Q_D(HbInputSettingPopup);
    
    connect(mainWindow(), SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(orientationChanged()));

    HbPushButton *button = new HbPushButton();
    HbIcon backIcon = HbIcon("qtg_mono_back");
    button->setIcon(backIcon);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setObjectName("SettingsPopupBackButton");
    connect(button, SIGNAL(clicked()), this, SLOT(close()));

    d->mSettingView = new HbView(this);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    HbDataForm *dataForm = new HbDataForm();
    HbInputSettingWidget *settingWidget = new HbInputSettingWidget(dataForm, d->mSettingView);
    settingWidget->initializeWidget();

    layout->addItem(button);
    layout->setAlignment(button, Qt::AlignRight);
    button->setMaximumSize(HbBackButtonSize);
    layout->addItem(dataForm);
    d->mSettingView->setLayout(layout);
    HbInputRegionCollector::instance()->attach(d->mSettingView);

    setContentWidget(d->mSettingView);
    setModal(true);
    setPreferredPos(HbSettingPopupLocation);

    setDismissPolicy(HbPopup::NoDismiss);
    setTimeout(HbPopup::NoTimeout);
}

HbInputSettingPopup::~HbInputSettingPopup()
{
    Q_D(HbInputSettingPopup);
    HbInputRegionCollector::instance()->detach(d->mSettingView);
}

void HbInputSettingPopup::close()
{
    HbDialog::close();
    emit dialogClosed();
}

void HbInputSettingPopup::orientationChanged()
{
    repolish();
}

// End of file
