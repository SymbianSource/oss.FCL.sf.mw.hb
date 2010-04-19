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


#include "hbrepeatitem_p.h"
#include <hbframedrawer.h>
#include <QPainter>
#include <QBitmap>
#include <QGraphicsSceneResizeEvent>
#include <QApplication>


/*
    Internal class used to draw infinite progress bar graphic.
    Draws the given image repeated times and animates it.
    this is used only to show wait progressbar.
*/
HbRepeatItem::HbRepeatItem(QGraphicsItem *parent):
    HbWidgetBase(parent),
    mMode(Qt::KeepAspectRatio),
    mOffset(0),
    mRepeatPixmap(0),
	mNumberOfRepeating(5),
	mOrientation(Qt::Horizontal),
	inverted(false),
	mRectWidth(0),
	mRectHeight(0),
	mBaseIconWidth(0),
	mBaseIconHeight(0),
	mBaseResizeWidth(0),
	mBaseResizeHeight(0),
	multiFactor(0)
{
		
}

HbRepeatItem::HbRepeatItem(const QString& name,QGraphicsItem *parent):
    HbWidgetBase(parent),
    mIcon(name),
    mMode(Qt::KeepAspectRatio),
    mOffset(0),
    mRepeatPixmap(0),
	mNumberOfRepeating(5),
	mOrientation(Qt::Horizontal),
	inverted(false),
	mRectWidth(0),
	mRectHeight(0),
	mBaseIconWidth(0),
	mBaseIconHeight(0),
	mBaseResizeWidth(0),
	mBaseResizeHeight(0),
	multiFactor(0)
{
	if(!mIcon.isNull()) {
		mBaseIconWidth = mIcon.width();
		mBaseIconHeight = mIcon.height();
	}
}
void HbRepeatItem::resizeEvent ( QGraphicsSceneResizeEvent * event ) 
{
	Q_UNUSED(event);
	createPixmap(boundingRect());

}
void HbRepeatItem::createPixmap(QRectF rect) 
{
	if(!mIcon.isNull()) {
		qreal parentWidth = rect.size().width();
		qreal parentHeight = rect.size().height();
		if(parentWidth !=0) {
			qreal tempWidth = mNumberOfRepeating*mBaseIconWidth;
			multiFactor = (qreal) parentWidth  / tempWidth;
			qreal tempHeight = multiFactor * mBaseIconHeight;
			if(tempHeight > parentHeight)
			{
				multiFactor = (qreal) parentHeight  / mBaseIconHeight;
			}
			//Convert multiple factor to two digit;
			int tempFactor = (int) (10 *multiFactor);
			multiFactor = (qreal) tempFactor / 10;
			mBaseResizeWidth = (int)(mBaseIconWidth*multiFactor);
			mBaseResizeHeight = (int)(mBaseIconHeight*multiFactor);

			mIcon.setSize(QSizeF(mBaseResizeWidth,mBaseResizeHeight));
			if(mRepeatPixmap) {
				delete mRepeatPixmap;
				mRepeatPixmap =0;
			}
			mRectWidth  = mBaseResizeWidth * mNumberOfRepeating;
			mRectHeight = mBaseResizeHeight;
			if(mOrientation == Qt::Horizontal){
				mRepeatPixmap = new QPixmap(mRectWidth,mRectHeight);
				mRepeatPixmap->fill(Qt::transparent);	
				QPainter p2;
				p2.begin(mRepeatPixmap);
				if(mBaseIconWidth > 0){
					for(qreal i = 0 ; i < mRectWidth ; i += mBaseResizeWidth) {
						mIcon.paint(&p2, QRectF(i, 0, mBaseResizeWidth, mBaseResizeHeight), mMode);
					}  
				}
				p2.end();
			}
			else{
				// TODO ANZ
			}
			setGeometry(0,0,mRectWidth,mRectHeight);//set geometry of QGI
			update();
		}
	}
}
HbRepeatItem::~HbRepeatItem()
{
	if(mRepeatPixmap ) { 
		delete mRepeatPixmap;
		mRepeatPixmap =0;
	}
}
void HbRepeatItem::setName(const QString& name) 
{
	if(!mIcon.isNull()){
		if (mIcon.iconName() == name) {
			return;
		}
	}
    mIcon.setIconName(name);
	mBaseIconWidth = mIcon.width();
	mBaseIconHeight = mIcon.height();
	createPixmap(boundingRect());

}
void HbRepeatItem::setInverted(bool inversion)
{
	inverted = inversion;
}

void HbRepeatItem::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    mMode = mode;
}

void HbRepeatItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
	if(mRepeatPixmap != 0) {
		if(boundingRect().isValid()) {
			painter->drawPixmap(QPointF(0,0),*mRepeatPixmap);
		}
	}
}


void HbRepeatItem::setRepeatingNumber(int number)
{
	if(number != mNumberOfRepeating) {
		mNumberOfRepeating = number;
		createPixmap(boundingRect());
		//resizeEvent(0);
	}

}

void HbRepeatItem::setOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;
}

HbRepeatMaskItem::HbRepeatMaskItem(QGraphicsItem *parent):
HbRepeatItem(parent),mMaskValue(0),mMaximum(5)
{
	
}

HbRepeatMaskItem::HbRepeatMaskItem(const QString &name, QGraphicsItem *parent):
HbRepeatItem(name,parent),mMaskValue(0),mMaximum(5)
{
	
}
void HbRepeatMaskItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
    Q_UNUSED(widget);

	if(mRepeatPixmap != 0) {
		QPixmap mask(mRectWidth,mRectHeight);
		mask.fill(Qt::white);
		QPainter p;
		p.begin(&mask);
		p.setBrush(QBrush(Qt::black));
		if(mOrientation == Qt::Horizontal){

			QRectF test = boundingRect();
					qreal left = (qreal)boundingRect().topLeft().x();
					if(qApp->layoutDirection() == Qt::RightToLeft) {
						left = (qreal)boundingRect().width()* ((mMaximum - mMaskValue)/(qreal) mMaximum );
					}
					p.drawRect(QRectF(
						left,
						(qreal)boundingRect().topLeft().y(),
						(qreal)boundingRect().width()* (mMaskValue/(qreal) mMaximum ),
						(qreal)boundingRect().height()
					));
				}
		else {
		
			//TODO ANZ

		}

		mRepeatPixmap->setMask(mask);
		
		if(boundingRect().isValid()) {
				painter->drawPixmap(QPointF(0,0),*mRepeatPixmap);
		}
	}

}

void HbRepeatMaskItem::resizeEvent ( QGraphicsSceneResizeEvent * event ) 
{
	Q_UNUSED(event);
	createPixmap(boundingRect());

}
void HbRepeatMaskItem::setMaximum(int max)
{
	mMaximum = max;
}
void HbRepeatMaskItem::setMaskValue(int maskValue)
{
	mMaskValue = maskValue;
}

