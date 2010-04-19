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

#include "hbdevice_p.h"
#include <QGraphicsLinearLayout>
#include <hbdeviceprofile.h>

HbDeviceView::HbDeviceView()
{
}

HbDeviceView::HbDeviceView(HbMainWindow *window, QWidget *parent) :
QGraphicsView(parent), mMainWindow(window), mAngle(0), mLastPos(0,0), 
mMouseMove(false)
{
    Q_UNUSED(parent);

    //Set background
    setBackgroundBrush(QBrush(QColor(0,0,0),Qt::SolidPattern/*Qt::VerPattern*/));
    //Take scene from window
    setScene(mMainWindow->scene());

    //Get real values from device profile
    HbDeviceProfile profile = HbDeviceProfile::profile(mMainWindow);
    qreal x = mMainWindow->rect().x();
    qreal y = mMainWindow->rect().y();
    int h = profile.logicalSize().height();
    int w = profile.logicalSize().width();
    
    //Centralize
    setSceneRect(x, y, w, h);

    setDragMode(QGraphicsView::NoDrag/*QGraphicsView::ScrollHandDrag*/);
    connect(mMainWindow, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(orientationChanged(Qt::Orientation)));    
}

HbDeviceView::~HbDeviceView()
{
}

void HbDeviceView::rotateDevice(int angle)
{
    //Store angle and block turning more than 90 or -90 degrees
    mAngle = mAngle+angle;
 if ( mAngle < -90 || mAngle > 90 ){
        if (mAngle < -90) {
            mAngle = -90;
        } else {
 mAngle = 90;
        }
        return;
    }

    rotate(angle);

 // For special purpose, if needed..
        if (mAngle == 0) {
        }else if (mAngle == 90) {
        }else if (mAngle == -90) {
    }
}

void HbDeviceView::orientationChanged(Qt::Orientation orientation)
{
    Q_UNUSED(orientation);

    HbDeviceProfile profile = HbDeviceProfile::profile(mMainWindow);
    int h = profile.logicalSize().height();
    int w = profile.logicalSize().width();

    //This is needed to centralize mMainWindow
    setSceneRect(0, 0, w, h);
    if(!mMouseMove) {
        resize(w+5, h+5);
    }

}

//Remove black "frames"
void HbDeviceView::compressDevice()
{
    if(mAngle <0 ) {
        rotate(+(-mAngle)); 
        mAngle = 0;
    } else {
 rotate(-(+mAngle));
 mAngle = 0;
    }

    HbDeviceProfile profile = HbDeviceProfile::profile(mMainWindow);
    int h = profile.logicalSize().height();
    int w = profile.logicalSize().width();

    setSceneRect(0, 0, w, h);
    resize(w+5, h+5);
}

//Catch mouse move event and rotate this view
void HbDeviceView::mouseMoveEvent(QMouseEvent *event)
{	
    if (!mMouseMove || itemAt(event->pos())) {
        return;
     }
     if ((event->buttons() & Qt::LeftButton)) {
        if (event->pos().x() < mLastPos.x()) {
            rotateDevice(-10);
        }else if (event->pos().x() > mLastPos.x()) {
            rotateDevice(10);
        }
    mLastPos = event->pos();
    }
}

void HbDeviceView::keyPressEvent(QKeyEvent *event)
{
    //If "Ctrl+D" is pressed, enable or disable rotate
    if(event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_D )
    {
        if(!mMouseMove) {
            mMouseMove = true;
            enableDevice(true);
        } else {
            mMouseMove = false;
            enableDevice(false);
        }
    }
}

void HbDeviceView::enableDevice(bool enable)
{
    if(enable) {
        resize(900, 900);
    } else {
        compressDevice();
    }
}


#include "moc_hbdevice_p.cpp"

