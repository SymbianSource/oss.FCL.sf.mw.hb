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

#include "hbprogressiveslider.h"
#include "hbprogressiveslider_p.h"
#include "hbprogressslidercontrol_p.h"
#include "hbstyleoption.h"

HbProgressiveSliderPrivate::HbProgressiveSliderPrivate()
    :mControl(0)
{
}


HbProgressiveSliderPrivate::~HbProgressiveSliderPrivate()
{
}


void HbProgressiveSliderPrivate::init()
{
}

void HbProgressiveSliderPrivate::setOrientation(Qt::Orientation orientation)
{
    mControl->setOrientation(orientation);
    HbProgressBarPrivate::setOrientation(orientation);
}

HbProgressiveSlider::HbProgressiveSlider(QGraphicsItem *parent) :
    HbProgressBar(*new HbProgressiveSliderPrivate,HbProgressBar::SimpleProgressBar,parent)
{
    Q_D( HbProgressiveSlider );
    d->q_ptr = this;
    d->init();
}

/*!
    @deprecated
    \class HbProgressiveSlider
    \sa HbRatingSlider
*/

/*!
    Constructs a progressslider  of a given \a parent.
*/
HbProgressiveSlider::HbProgressiveSlider(HbProgressiveSliderPrivate &dd,QGraphicsItem *parent) : 
    HbProgressBar( dd,HbProgressBar::SimpleProgressBar,parent)
{
    Q_D( HbProgressiveSlider );
    d->init();
}


/*!
    Destructor for the progressslider.
*/
HbProgressiveSlider::~HbProgressiveSlider()
{
}


void HbProgressiveSlider::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    HbProgressBar::resizeEvent(event);
}


/*!
    Sets the current value of the progress slider.

    The progress slider forces the value to be within the legal range: \b
    minimum <= \c value <= \b maximum.

    \sa value()
*/
void HbProgressiveSlider::setSliderValue(int value)
{
    Q_D( HbProgressiveSlider );
    d->mControl->setValue(value);
}

/*!
    Returns the current slider value . 

    The default value is \c 0.

    \sa setSliderValue()
*/
int HbProgressiveSlider::sliderValue() const
{
    Q_D(const HbProgressiveSlider );
    return d->mControl->value();
}


/*!
    Returns \c true whether the slider is pressed down.
*/

bool HbProgressiveSlider::isSliderDown() const
{
    return true; //TODO:do sometheing here
}


/*!
    set the tooltip text . 
    \sa handleToolTip()
*/
void HbProgressiveSlider::setHandleToolTip(const QString &text)
{
    Q_D(HbProgressiveSlider);
    d->mControl->setToolTip(text);
}


/*!
    Returns the current tooltip text value . 
    \sa setHandleToolTip()
*/
QString HbProgressiveSlider::handleToolTip() const
{
    Q_D(const HbProgressiveSlider);
    return d->mControl->toolTip();
}


void HbProgressiveSlider::setInvertedAppearance(bool inverted)
{
    Q_D( HbProgressiveSlider );
    if(d->mControl->invertedAppearance() != inverted) {
        d->mControl->setInvertedAppearance(inverted);
    }
    HbProgressBar::setInvertedAppearance(inverted);
}


/*!
    Sets the Icon for the progressslider thumb.
*/
void HbProgressiveSlider::setThumbIcon(const HbIcon &icon)
{
    Q_D( HbProgressiveSlider );
    d->mControl->setHandleIcon(icon.iconName());
}

HbIcon HbProgressiveSlider::thumbIcon() const
{
    Q_D( const HbProgressiveSlider );
    return HbIcon(d->mControl->handleIcon());
}

/*!
    Returns \c true whether slider tracking is enabled.

    The default value is \c true.

    If tracking is enabled, the slider emits the
    valueChanged( ) signal while the slider is being dragged. If
    tracking is disabled, the slider emits the valueChanged( ) signal
    only when the user releases the slider.

    \sa HbSlider::setTracking( )
*/
bool HbProgressiveSlider::hasTracking( ) const
{
    Q_D( const HbProgressiveSlider );
    return d->mControl->hasTracking( );
}

/*!
    Sets whether the slider tracking is enabled.

    \sa HbSlider::hasTracking( )
*/
void HbProgressiveSlider::setTracking( bool enable )
{
    Q_D( HbProgressiveSlider );
    d->mControl->setTracking( enable );
}

void HbProgressiveSlider::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_UNUSED(event);//TODO:check this
}


void HbProgressiveSlider::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{
    Q_UNUSED(event);
}

void HbProgressiveSlider::setGeometry(const QRectF & rect)
{
    Q_D(HbProgressiveSlider);
    HbProgressBar::setGeometry(rect);
    d->mControl->setGeometry(QRectF(0,0,rect.width(),rect.height()));
}


/*void HbProgressiveSlider::initStyleOption(HbStyleOption *hboption) const
{
    HbProgressBar::initStyleOption(hboption);    
}*/


void HbProgressiveSlider::updatePrimitives()
{
    HbProgressBar::updatePrimitives();

    Q_D(HbProgressiveSlider);
    d->mControl->updatePrimitives();
    
    
}

/*!
    Sets the Thumbwidget for the progressslider.If passes NULL it uses previously set thumbwidget
    \a Pointer to the widget to set as thumbitem.Ownership is transferred to slider.
    The thumb item can be hidden by setting QGraphicsItem::ItemIsFocusable flag to false.

    \sa thumbItem() 
*/
void HbProgressiveSlider::setThumbItem(QGraphicsWidget* thumbItem)
{
    Q_D(HbProgressiveSlider);

    if (HbWidget *widget = qobject_cast<HbWidget *>(thumbItem)) {
        widget->setAttribute(Hb::InteractionDisabled, true);
    }
    d->mControl->setHandleItem(thumbItem);
    if(!flags().testFlag(ItemIsFocusable)) {
        d->mControl->setHandleVisible(false);
    }
}

/*!
    Returns  the Thumbwidget set as the thumbItem.Returns 0 if no thumb item set by user.
    \sa setThumbItem()
*/
QGraphicsWidget* HbProgressiveSlider::thumbItem() const
{
    Q_D(const HbProgressiveSlider);

    return static_cast<QGraphicsWidget*>(d->mControl->handleItem());
}


void HbProgressiveSlider::showEvent( QShowEvent * event )
{
    HbProgressBar::showEvent(event);
}

QVariant HbProgressiveSlider::itemChange(GraphicsItemChange change,const QVariant & value)
{ 
    return HbProgressBar::itemChange(change, value);
}
 
