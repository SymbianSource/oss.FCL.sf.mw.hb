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

#include "hbsettingswindow_p.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"
#include "hbinstance_p.h"
#include "hbdeviceprofile.h"
#include "hbdeviceprofilemanager_p.h"
#include "hbapplication.h"
#include "hbtoucharea_p.h"
#include "hbtextitem_p.h"
#include "hbiconitem_p.h"
#include "hbgraphicsscene_p.h"

#include <QPointer>
#include <QComboBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QApplication>
#include <QSettings>

#ifdef CSS_INSPECTOR
#include <QPushButton>
#endif

static QString windowName(HbMainWindow *window)
{
    if (!window) {
        return HbSettingsWindow::tr("HbMainWindow(0x0)");
    }
    QString ptr = QString::number((qptrdiff)window, 16);
    QString name = window->metaObject()->className();
    if (!window->objectName().isEmpty()) {
        return HbSettingsWindow::tr("%1(%2, %3)").arg(name).arg(ptr).arg(window->objectName());
    }
    return HbSettingsWindow::tr("%1(%2)").arg(name).arg(ptr);
}

static HbMainWindow *getWindow(int index)
{
    HbMainWindow *window = 0;
    if (index > 0) {
        window = hbInstance->allMainWindows().value(index - 1);
    }
    return window;
}

HbSettingsWindow *HbSettingsWindow::instance()
{
    static QPointer<HbSettingsWindow> window = 0;
    if (!window) {
        window = new HbSettingsWindow;
        window->setAttribute(Qt::WA_DeleteOnClose);
    }
    return window;
}

HbSettingsWindow::HbSettingsWindow(QWidget *parent) : QWidget(parent)
{
    windowComboBox = new QComboBox(this);
    windowComboBox->hide();
    resolutionComboBox = new QComboBox(this);
    orientationComboBox = new QComboBox(this);
    directionComboBox = new QComboBox(this);
    dragToResizeComboBox = new QComboBox(this);
    mSensorComboBox = new QComboBox(this);
    mGeneralSettingsForSensorsComboBox = new QComboBox(this);
    mUnsetOrientationButton = new QPushButton(tr("&Unset orientation"), this);

    resolutionComboBox->addItems(HbDeviceProfile::profileNames());
    orientationComboBox->addItems(QStringList() << tr("Portrait") << tr("Landscape"));
    directionComboBox->addItems(QStringList() << tr("Left to right") << tr("Right to left"));
    dragToResizeComboBox->addItems(QStringList() << tr("Disabled") << tr("Enabled"));
    mSensorComboBox->addItems(QStringList() << tr("Landscape") << tr("Portrait"));
    mGeneralSettingsForSensorsComboBox->addItems(QStringList() << tr("Disabled") << tr("Enabled"));
    refresh();

    connect(windowComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeWindow(int)));
    connect(resolutionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeResolution(int)));
    connect(orientationComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeOrientation(int)));
    connect(directionComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeDirection(int)));
    connect(dragToResizeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeDragToResize(int)));
    connect(mSensorComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeSensorValue(int)));
    connect(mGeneralSettingsForSensorsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeGSettingsForSensors(int)));
    connect(mUnsetOrientationButton, SIGNAL(pressed()), SLOT(unsetOrientation()));

    QVBoxLayout *boxLayout = new QVBoxLayout(this);
    
    QGroupBox *mainGroup = new QGroupBox(this);
    QFormLayout *layout = new QFormLayout(mainGroup);
    layout->addRow(tr("&Window"), windowComboBox);
    layout->addRow(tr("&Resolution"), resolutionComboBox);
    layout->addRow(tr("&Orientation"), orientationComboBox);
    layout->addRow(tr("&Direction"), directionComboBox);
    layout->addRow(tr("&Drag to resize"), dragToResizeComboBox);

    mainGroup->setLayout(layout);
    boxLayout->addWidget(mainGroup);
	
    QGroupBox *sensorGroup = new QGroupBox(tr("Sensor handling"), this);
    QFormLayout *sensorLayout = new QFormLayout(sensorGroup);

    sensorLayout->addRow(tr("&Sensors"), mSensorComboBox);
    sensorLayout->addRow(tr("&GS sensors"), mGeneralSettingsForSensorsComboBox);
    sensorLayout->addRow(mUnsetOrientationButton);
    mainGroup->setLayout(sensorLayout);
    boxLayout->addWidget(sensorGroup);

    QGroupBox *globalGroup = new QGroupBox(tr("Global debug drawing"), this);
    QFormLayout *globalLayout = new QFormLayout(globalGroup);
    
    touchAreaComboBox = new QComboBox(this);
    touchAreaComboBox->addItems(QStringList() << tr("Invisible") << tr("Visible"));
    connect(touchAreaComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeTouchArea(int)));
	globalLayout->addRow(tr("&Touch areas"), touchAreaComboBox);
    
    textBoxesComboBox = new QComboBox(this);
    textBoxesComboBox->addItems(QStringList() << tr("Invisible") << tr("Visible"));
    connect(textBoxesComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeTextBoxes(int)));
	globalLayout->addRow(tr("&Text items"), textBoxesComboBox);
    
    iconBoxesComboBox = new QComboBox(this);
    iconBoxesComboBox->addItems(QStringList() << tr("Invisible") << tr("Visible"));
    connect(iconBoxesComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeIconBoxes(int)));
	globalLayout->addRow(tr("&Icon items"), iconBoxesComboBox);
    
    fpsCounterComboBox = new QComboBox(this);
    fpsCounterComboBox->addItems(QStringList() << tr("Invisible") << tr("Visible"));
    connect(fpsCounterComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeFpsCounter(int)));
	globalLayout->addRow(tr("&Fps counter"), fpsCounterComboBox);

    globalGroup->setLayout(globalLayout);
    boxLayout->addWidget(globalGroup);

#ifdef CSS_INSPECTOR
    QGroupBox *cssGroup = new QGroupBox(tr("CSS Debugging"), this);
    QHBoxLayout *cssLayout = new QHBoxLayout(cssGroup);

    cssWindowButton = new QPushButton(tr("Show CSS inspector"), this);
    connect(cssWindowButton, SIGNAL(pressed()), HbCssInspectorWindow::instance(), SLOT(show()));
    cssLayout->addWidget(cssWindowButton);

    cssGroup->setLayout(cssLayout);
    boxLayout->addWidget(cssGroup);
#endif
	initStartUpValues();
}

HbSettingsWindow::~HbSettingsWindow()
{
}

void HbSettingsWindow::refresh()
{
    QString text = windowComboBox->currentText();

    QStringList windows;
    windows << tr("Global");
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        windows << windowName(window);
    }

    windowComboBox->clear();
    windowComboBox->addItems(windows);

    if (!text.isNull()) {
        windowComboBox->setCurrentIndex(windowComboBox->findText(text));
    }
}

void HbSettingsWindow::initStartUpValues()
{
    changeSensorValue(3);
    changeGSettingsForSensors(3);
}

void HbSettingsWindow::changeWindow(int index)
{
    Q_UNUSED(index);
}

void HbSettingsWindow::changeResolution(int index)
{
    QStringList names = HbDeviceProfile::profileNames();
    HbDeviceProfile profile(names.value(index));

    HbMainWindow *window = getWindow(windowComboBox->currentIndex());
    if (!window) {
        // global
        
        // resize all windows
        QListIterator<HbMainWindow*> iterator(hbInstance->allMainWindows());
        while (iterator.hasNext()) {
            HbMainWindow *window = iterator.next();
            window->resize(profile.logicalSize());
            HbDeviceProfileManager::select(*window, profile);
        }
 
    } else {
        // window specific
        window->resize(profile.logicalSize());
        HbDeviceProfileManager::select(*window, profile);
    }
}

void HbSettingsWindow::changeOrientation(int index)
{
    HbMainWindow *window = getWindow(windowComboBox->currentIndex());
    if (!window) {
        // global
        HbInstancePrivate::d_ptr()->setOrientation(index == 0 ? Qt::Vertical : Qt::Horizontal, true);
    } else {
        // window specific
        if (index == 2)
            window->unsetOrientation();
        else{
            Qt::Orientation orientation = index == 0 ? Qt::Vertical : Qt::Horizontal;
            if(orientation != HbInstancePrivate::d_ptr()->orientation()){
                window->setOrientation(orientation);
            }else{
                window->unsetOrientation();
            }
        }
    }
}

void HbSettingsWindow::changeDirection(int index)
{
    HbMainWindow *window = getWindow(windowComboBox->currentIndex());
    if(!window){
        Qt::LayoutDirection dir = HbApplication::layoutDirection();
        HbApplication::setLayoutDirection(dir == Qt::LeftToRight ? Qt::RightToLeft : Qt::LeftToRight);
    }else{
        Qt::LayoutDirection dir = index == 0 ? Qt::LeftToRight : Qt::RightToLeft;
        if(dir != HbApplication::layoutDirection()){
            window->setLayoutDirection(dir);
        }else{
            window->unsetLayoutDirection();
        }
    }
}

void HbSettingsWindow::changeTouchArea(int index)
{
    HbTouchAreaPrivate::setOutlineDrawing(index > 0);
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        // Force window to redraw
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(window, &event);
    }
}

void HbSettingsWindow::changeTextBoxes(int index)
{
    HbTextItemPrivate::outlinesEnabled = index > 0;
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        // Force window to redraw
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(window, &event);
	}
}

void HbSettingsWindow::changeIconBoxes(int index)
{
    HbIconItemPrivate::outlinesEnabled = index > 0;
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        // Force window to redraw
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(window, &event);
	}
}

void HbSettingsWindow::changeFpsCounter(int index)
{
    HbGraphicsScenePrivate::fpsCounterEnabled = index > 0;
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        // Force window to redraw
        QEvent event(QEvent::WindowActivate);
        QApplication::sendEvent(window, &event);
	}
}

void HbSettingsWindow::changeDragToResize(int index)
{
    resolutionComboBox->setEnabled(index == 0);
    orientationComboBox->setEnabled(index == 0);
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        HbDeviceProfile profile = HbDeviceProfile::profile(window);
        window->resize(profile.logicalSize());
    }
    HbMainWindowPrivate::dragToResizeEnabled = index > 0;
}

void HbSettingsWindow::changeSensorValue(int index)
{
    if (mSensorComboBox->count() == 3 && (index == 0 || index == 1))
        mSensorComboBox->removeItem(2);

    QSettings mSettings( "Nokia", "HbStartUpDeskTopSensors");
    switch(index) {
        case 0:	
            HbMainWindowOrientation::instance()->mSensorListener->setSensorOrientation(Qt::Horizontal);
            mSettings.setValue("Orientation", 1);
        break;
        case 1:
            HbMainWindowOrientation::instance()->mSensorListener->setSensorOrientation(Qt::Vertical);
            mSettings.setValue("Orientation", 2);
        break;
		case 2: //Empty
        break;
        case 3: //Initialize
            if (mSettings.value("Orientation").toInt() == 1)
                mSensorComboBox->setCurrentIndex(0);
            else
                mSensorComboBox->setCurrentIndex(1);
            break;
        default: 
            break; 
    }
}

void HbSettingsWindow::changeGSettingsForSensors(int index)
{
    QSettings mSettings("Nokia", "HbStartUpDeskTopSensors");
    if (index == 3){ //Initialize
        mGeneralSettingsForSensorsComboBox->setCurrentIndex(mSettings.value("SensorsEnabled").toBool());
    } else { 
        mSettings.setValue("SensorsEnabled", (bool)index);
        HbMainWindowOrientation::instance()->mSensorListener->enableSensors(index, true);
    }
}

void HbSettingsWindow::unsetOrientation()
{
    foreach (HbMainWindow *window, hbInstance->allMainWindows()) {
        if (!HbMainWindowPrivate::d_ptr(window)->mAutomaticOrientationSwitch) {
            window->unsetOrientation();
        }
    }
}
