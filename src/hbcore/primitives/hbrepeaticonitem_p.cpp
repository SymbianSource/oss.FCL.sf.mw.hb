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


#include "hbrepeaticonitem_p.h"
#include <hbframedrawer.h>

#include <QPainter>
#include <QTimer>
#include <QBitmap>
#include <QGraphicsSceneResizeEvent>
#include <QApplication>

QTimer HbRepeatIconItem::mTimer;

/*
    Internal class used to draw infinite progress bar graphic.
    Draws the given image repeated times and animates it.
    this is used only to show wait progressbar.
*/
HbRepeatIconItem::HbRepeatIconItem(QGraphicsItem *parent):
    HbWidgetBase(parent),
    mMode(Qt::KeepAspectRatio),
    mMask(QPixmap()),
    mOffset(0),
    mRepeatPixmap(0),
    mOrientation(Qt::Horizontal),
	mConnected(false)
{

	connect(&mTimer,SIGNAL(timeout()),this,SLOT(updateAnimation()));
	
}


HbRepeatIconItem::HbRepeatIconItem(const QString& name,QGraphicsItem *parent):
    HbWidgetBase(parent),
    mIcon(name),
    mMode(Qt::KeepAspectRatio),
    mMask(QPixmap()),
    mOffset(0),
    mRepeatPixmap(0),
	mConnected(false)
{
   connect(&mTimer,SIGNAL(timeout()),this,SLOT(updateAnimation()));
}

HbRepeatIconItem::~HbRepeatIconItem()
{
    if(mRepeatPixmap) {
        delete mRepeatPixmap;
    }
}


void HbRepeatIconItem::setName(const QString& name) 
{
    mIcon.setIconName(name);
    resizeEvent(0);
}


void HbRepeatIconItem::setMask(QPixmap mask)
{
    mMask = mask;
}


void HbRepeatIconItem::setIconHeight(qreal height)
{
    mIcon.setHeight(height);
}


void HbRepeatIconItem::setIconWidth(int width)
{
    mIcon.setWidth(width);
}


void HbRepeatIconItem::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    mMode = mode;
}


/*
    This updates the offset to show the image.
    this is called on timer timeout so this is called no. of times/s.
*/
void HbRepeatIconItem::updateAnimation()
{
	mConnected = true;
    if(mIcon.width()>0 && mIcon.height()>0){
        if(mOrientation == Qt::Horizontal){
            mOffset = (mOffset+2) % (int)mIcon.width();
        }
        else{
            mOffset = (mOffset+2) % (int)mIcon.height();
        }
        update(boundingRect());
    }
}


void HbRepeatIconItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

  //  if(boundingRect().isValid()) { // There is no geomtry set at any point ? 
		if(!mConnected) {
			connect(&mTimer,SIGNAL(timeout()),this,SLOT(updateAnimation()));
		}
		// Chec the status of timer
		if (!mTimer.isActive()) {
			mTimer.start(122);
		}
				
		if(mParentRect != parentItem()->boundingRect()) {
            resizeEvent(0); // need to regenerate optimized pixmap.
        }
        QPixmap newblit(boundingRect().size().toSize());
        newblit.fill(Qt::white);
        QPainter p;
        p.begin(&newblit);
        if(mOrientation == Qt::Horizontal){
            QSizeF pixmapSize(boundingRect().size().width(),mIcon.height());
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                p.drawPixmap(QPointF(0,0), *mRepeatPixmap, QRectF(QPointF(mIcon.width()-mOffset,0),pixmapSize));
            } else {
                p.drawPixmap(QPointF(0,0), *mRepeatPixmap, QRectF(QPointF(mOffset,0),pixmapSize));
            }
        }
        else{
            QSizeF pixmapSize(mIcon.width(),boundingRect().size().height());
            p.drawPixmap(QPointF(0,0), *mRepeatPixmap, QRectF(QPointF(0,mOffset),pixmapSize));
        }
        p.end();
        if(!mMask.isNull()) {                 
            newblit.setMask(mMask);
        }
        painter->drawPixmap(QPointF(0,0),newblit);
  //  }
}


/*
    This is done when infinite progressbar is switched to normal progressbar.
*/
void HbRepeatIconItem::stopAnimation()
{
        mTimer.stop();
}


void HbRepeatIconItem::setOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;
}

/*
    This regenrate optimize pixmap.
    there is single squire image which is repeated n no. of times to genrate new pixmap
    this new pixmap is translated to move horizontaly or verticaly depending on orientation.
*/
void HbRepeatIconItem::resizeEvent ( QGraphicsSceneResizeEvent * event ) 
{
    Q_UNUSED(event);
    mParentRect = parentItem()->boundingRect();
    HbFrameDrawer drawer;
    if(mOrientation == Qt::Horizontal){
        drawer.setFrameGraphicsName("qtg_fr_progbar_h_mask");
        drawer.setFrameType(HbFrameDrawer::ThreePiecesHorizontal);
        setIconHeight(mParentRect.size().toSize().height());//set height of the image drawer
    }
    else{
        drawer.setFrameGraphicsName("qtg_fr_progbar_v_mask");
        drawer.setFrameType(HbFrameDrawer::ThreePiecesVertical);
        setIconWidth(mParentRect.size().toSize().width());//set width of the image drawer
    }
    drawer.setFillWholeRect(true);
    QPixmap track(mParentRect.size().toSize());
    track.fill(Qt::white);
    QPainter p;
    p.begin(&track);
    drawer.paint(&p, QRectF(QPointF(0,0),mParentRect.size() ));
    p.end();
    
    setGeometry(QRectF(QPointF(0,0),mParentRect.size() ));//set geometry of QGI
    
    QImage i=track.toImage();
    //i.invertPixels();
    setMask(QPixmap::fromImage(i));
    
    if(mRepeatPixmap){
        delete mRepeatPixmap;
        mRepeatPixmap = 0;
    }

    qreal rectWidth = mParentRect.size().width(); //newSize.width();
    qreal iconWidth = mIcon.width();
    qreal rectHeight = mParentRect.size().height();
    qreal iconHeight = mIcon.height();


    if(mOrientation == Qt::Horizontal){
        mRepeatPixmap = new QPixmap(int(rectWidth + iconWidth), (int)iconHeight);
        mRepeatPixmap->fill(Qt::white);
        QPainter p2;
        p2.begin(mRepeatPixmap);
        if(iconWidth > 0){
            for(qreal i = 0 ; i < (rectWidth + iconWidth) ; i += iconWidth) {
                mIcon.paint(&p2, QRectF(i, 0, iconWidth, iconHeight), mMode);
            }  
        }
        p2.end();
    }
    else{
        mRepeatPixmap = new QPixmap((int)iconWidth, int(rectHeight + iconHeight));
        mRepeatPixmap->fill(Qt::white);
        QPainter p2;
        p2.begin(mRepeatPixmap);
        if( iconHeight > 0 ){
            for(qreal i = 0 ; i < (rectHeight + iconHeight) ; i += iconHeight) {
                mIcon.paint(&p2, QRectF(0, i, iconWidth, iconHeight), mMode);
            }  
        }
        p2.end();
    }
}

