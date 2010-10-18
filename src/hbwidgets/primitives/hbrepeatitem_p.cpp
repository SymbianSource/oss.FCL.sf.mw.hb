/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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
    internal
    Constructs HbRepeatItem

    HbRepeatItem ,internal class used to draw rating slider unrated graphic.
    Draws the given image repeated times and animates it.
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

/*
    internal
    Constructs HbRepeatItem

    HbRepeatItem ,internal class used to draw rating slider unrated graphic.
    Draws the given image repeated times and animates it.
*/
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

/*
    Destructs HbRepeatItem
*/
HbRepeatItem::~HbRepeatItem()
{
    if(mRepeatPixmap ) { 
        delete mRepeatPixmap;
        mRepeatPixmap = 0;
    }
}

/*
    reimp

    create the pixmap whenever size changes
*/
void HbRepeatItem::resizeEvent ( QGraphicsSceneResizeEvent * event ) 
{
    Q_UNUSED(event);
    createPixmap(boundingRect());

}

/*
    createPixmap
*/
void HbRepeatItem::createPixmap(QRectF rect) 
{
    if(!mIcon.isNull()) {
        // Get the rect provided by the parent
        qreal parentWidth = rect.size().width();
        qreal parentHeight = rect.size().height();

        if(parentWidth != 0) {
            // If the icons default size/rect is lesser than that of the parent's size/rect,
            // then no adjustment is required.
            // Icon size adjustment is done only for the case when icon's default size is greater than
            // that of the size provided by the parent
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
            // Re-size the icon to new size calculated, so as to get accomodated in the rect 
            // provided by the parent
            mIcon.setSize(QSizeF(mBaseResizeWidth,mBaseResizeHeight));

            // clean up the pixmap, if present already
            if(mRepeatPixmap) {
                delete mRepeatPixmap;
                mRepeatPixmap = 0;
            }

            // Calculate the rect to which pixmap has to be drawn
            mRectWidth  = mBaseResizeWidth * mNumberOfRepeating;
            mRectHeight = mBaseResizeHeight;

            if(mOrientation == Qt::Horizontal){
                mRepeatPixmap = new QPixmap(mRectWidth,mRectHeight);
                mRepeatPixmap->fill(Qt::transparent);   

                QPainter pixmapPainter;
                pixmapPainter.begin(mRepeatPixmap);
                if(mBaseIconWidth > 0){
                    for(qreal i = 0 ; i < mRectWidth ; i += mBaseResizeWidth) {
                        mIcon.paint(&pixmapPainter, QRectF(i, 0, mBaseResizeWidth, mBaseResizeHeight), mMode);
                    }  
                }
                pixmapPainter.end();
            }
            
            setGeometry(0,0,mRectWidth,mRectHeight);//set geometry of QGI
            //update();
        }
    }
}
/*
    Sets the new icon name for this HbRepeatItem for \a mode and \a state. If iconName is already set in
    HbRepeatItem then it will be replaced with this new iconName.

    \param  name the icon name.
 
 */
void HbRepeatItem::setName(const QString& name) 
{
    if(!mIcon.isNull()){
        if (mIcon.iconName() == name) {
            return;
        }
    }
    mIcon.setIconName(name);

    // store the icons default width & height
    mBaseIconWidth = mIcon.width();
    mBaseIconHeight = mIcon.height();

    createPixmap(boundingRect());
}

/*
    setInverted

    If inversion is set to false, then rating slider has to be rated in the other direction (e.g. from right to left). 
    By default, the rating slider is not inverted.

    \param  inversion can be true/false.
*/
void HbRepeatItem::setInverted(bool inversion)
{
    inverted = inversion;
}

/*
    setAspectRatioMode

    Defines the width:height to be used for creating the pixmap

    \param  mode this enum type defines what happens to the aspect ratio when scaling an rectangle.    
*/
void HbRepeatItem::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    mMode = mode;
}

/*
    reimp

    Provides the item's painting implementation, using painter which
    paints the contents of item in local coordinates
*/
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

/*
    Sets the number of icons/stars to be repeated

    Creates the pixmap based on the number of repeating icons
*/
void HbRepeatItem::setRepeatingNumber(int number)
{
    if(number != mNumberOfRepeating) {
        mNumberOfRepeating = number;
        createPixmap(boundingRect());
        //resizeEvent(0);
    }
}

/*
    Sets the orientation of the pixmap to be drawn

*/
void HbRepeatItem::setOrientation(Qt::Orientation orientation)
{
    mOrientation = orientation;
}

/*
    internal
    Constructs HbRepeatItem

    HbRepeatMaskItem ,internal class used to draw rating slider rated graphic.
    Draws the given image repeated times and animates it.
    
    HbRepeatMaskItem derived from HbRepeatItem, provides paint implementation for
    rated graphic.
    
*/
HbRepeatMaskItem::HbRepeatMaskItem(QGraphicsItem *parent):
HbRepeatItem(parent),mMaskValue(0),mMaximum(5)
{
    
}

/*
    internal
    Constructs HbRepeatItem

    HbRepeatMaskItem ,internal class used to draw rating slider rated graphic.
    Draws the given image repeated times and animates it.
    
    HbRepeatMaskItem derived from HbRepeatItem, provides paint implementation for
    rated graphic.
    
*/
HbRepeatMaskItem::HbRepeatMaskItem(const QString &name, QGraphicsItem *parent):
HbRepeatItem(name,parent),mMaskValue(0),mMaximum(5)
{
    
}

/*
    Sets the orientation for pixmap to be drawn

*/
void HbRepeatMaskItem::resizeEvent ( QGraphicsSceneResizeEvent * event ) 
{
    Q_UNUSED(event);
    createPixmap(boundingRect());
}

/*
    Sets the maximum for pixmap to be drawn.

    Required to determine the pixmap rect to drawn.

*/
void HbRepeatMaskItem::setMaximum(int max)
{
    mMaximum = max;
}

/*
    Sets the mask value for pixmap to be drawn.

    Required to determine the pixmap rect to drawn.

*/
void HbRepeatMaskItem::setMaskValue(int maskValue)
{
    mMaskValue = maskValue;
}

/*
    reimp

    Provides the item's painting implementation, using painter which
    paints the contents of item in local coordinates.
*/
void HbRepeatMaskItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if(boundingRect().isValid()) {
        if(mRepeatPixmap != 0) {
            // create pixmap of height and width as mRectHeight & mRectWidth
            QPixmap mask(mRectWidth,mRectHeight);
            // Fill the pixmap with white color
            mask.fill(Qt::white);

            QPainter p;
            p.begin(&mask);
            p.setBrush(QBrush(Qt::black));

            if(mOrientation == Qt::Horizontal){
                QRectF rect = boundingRect();
                qreal left = (qreal)rect.topLeft().x();
                
                // topLeft x co-ordinate adjustment inncase of inverted
                if(inverted) {
                    left = (qreal)rect.width()* ((mMaximum - mMaskValue)/(qreal) mMaximum );
                }
                // Draw the rectangle into which pixmap need to be drawn
                p.drawRect(QRectF(
                left,
                (qreal)rect.topLeft().y(),
                (qreal)rect.width()* (mMaskValue/(qreal) mMaximum ),
                (qreal)rect.height()
                ));
            }
            // Finally draw the pixmap with mask
            mRepeatPixmap->setMask(mask);
            painter->drawPixmap(QPointF(0,0),*mRepeatPixmap);
        }
    }

}


