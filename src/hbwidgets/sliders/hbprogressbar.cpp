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


#include <hbprogressbar.h>
#include <hbstyleoptionprogressbar.h>
#include "hbprogressbar_p.h"

#ifdef HB_EFFECTS
#include <hbeffect.h>
#include "hbeffectinternal_p.h"
#define HB_PRGRESSBAR_ITEM_TYPE "HB_PROGRESSBAR"
#endif


/*!
    @beta
    @hbwidgets
    \class HbProgressBar

    \brief HbProgressBar widget provides a vertical and horizontal progress bar.
    An infinite progressbar is also available.

    A progress bar is used to give the user an indication of the progress of an operation and to 
    reassure them that the application is still running.

    The progress bar uses the concept of steps. User can set it up by specifying the minimum and 
    maximum possible step values, and it will display the percentage of steps that have been completed
    when you later give it the current step value. 
    
    The percentage is calculated by dividing the progress (progressValue() - minimum()) divided by maximum() - minimum().

    User can specify the minimum and maximum number of steps with setMinimum() and setMaximum() APIs. 
    The current number of steps is set with setProgressValue(). 

    If minimum and maximum both are set to 0, the bar shows a busy indicator instead of a percentage of steps.
    This is useful, for example, when using ftp or http to download items when they are unable to 
    determine the size of the item being downloaded.

    ProgressBar also supports adding text . min max text pair is also supported which is commonly 
    used for progress indication for music.
    
    \image html hbprogressbartext.png Left Aligned Text, Min Max Text.

    Progress bar provides below signal.

    \li valueChanged(int value) Emitted when the value of the progressbar is changed.
*/

/*!
    @beta
    \fn void HbProgressBar::valueChanged(int value)
    
    Emitted when the value of the progressbar is changed.
*/


/*!
    @beta
    \reimp
    \fn int HbProgressBar::type() const
 */

/*!
    @beta
    \enum HbProgressBar::ProgressBarType

    This enum defines available progress bar types.
*/


/*
    HbProgressBarPrivate
    private class constructor     
*/
HbProgressBarPrivate::HbProgressBarPrivate() :
    mFrame(0),
    mTrack(0),
    mWaitTrack(0),
    mMinTextItem(0),
    mMaxTextItem(0),
    mMinMaxTextVisible(false),
    mMaximum(100),
    mMinimum(0),
    mProgressValue(0),
    mInvertedAppearance(false),
    mMinMaxTextAlignment(Qt::AlignBottom),
    mMinText(QString()),
    mMaxText(QString()),
    mOrientation(Qt::Horizontal),
    mDelayHideInProgress(true)
{
}

/*
    HbProgressBarPrivate
    destructor     
*/
HbProgressBarPrivate::~HbProgressBarPrivate() 
{
}

/*
    \internal
    initialises progressbar
*/
void HbProgressBarPrivate::init() 
{
    Q_Q(HbProgressBar);

    HbStyle *style = qobject_cast<HbStyle*>(q->style());
    Q_ASSERT(style);

    mFrame = style->createPrimitive(HbStyle::P_ProgressBar_frame,q);
    mTrack = style->createPrimitive(HbStyle::P_ProgressBar_track,mFrame);
    mWaitTrack = style->createPrimitive(HbStyle::P_ProgressBar_waittrack,mFrame);    
    mWaitTrack->setVisible(false);

    if(q->layoutDirection() == Qt::RightToLeft) {
        q->setInvertedAppearance(true);
    }

#ifdef HB_EFFECTS
    HbEffectInternal::add(HB_PRGRESSBAR_ITEM_TYPE,"progressbar_appear", "progressbar_appear");
    HbEffectInternal::add(HB_PRGRESSBAR_ITEM_TYPE,"progressbar_disappear", "progressbar_disappear");
    HbEffectInternal::add(HB_PRGRESSBAR_ITEM_TYPE,"progressbar_progress_complete", "progressbar_progress_complete");
#endif
}

/*
    createTextPrimitives
*/
void HbProgressBarPrivate::createTextPrimitives()
{
    Q_Q(HbProgressBar);

    mMinTextItem = q->style()->createPrimitive(HbStyle::P_ProgressBar_mintext,q);
    mMaxTextItem = q->style()->createPrimitive(HbStyle::P_ProgressBar_maxtext,q);
}

/*!
    \internal
    Sets the progressbar range
*/
void HbProgressBarPrivate::setRange(int minimum, int maximum)
{
    Q_Q(HbProgressBar);
    if( minimum > maximum ){
        maximum = minimum ;
    }
    mMinimum = minimum;
    mMaximum = maximum;

    if ( mProgressValue < mMinimum){
        mProgressValue = mMinimum;
    }

    if(mProgressValue > mMaximum){
        mProgressValue = mMaximum;
    }

    if( (mMinimum == 0) && (mMaximum == 0) && (mType != HbProgressBar::RatingProgressBar) ) {
        mWaitTrack->setVisible(true);
        mTrack->setVisible(false);
    } else {
        mWaitTrack->setVisible(false);
        mTrack->setVisible(true);
    }
    q->updatePrimitives();
}

/*!
    \internal
    Sets the progressbar orientation.
    Can be either vertical or Horizontal.
*/
void HbProgressBarPrivate::setOrientation(Qt::Orientation orientation)
{
    Q_Q(HbProgressBar);
    if (mOrientation != orientation) {
        mOrientation = orientation;
        q->repolish();
        q->updatePrimitives();
    }
}

/*!
    \internal
    Private slot which delays the hiding effect.
*/
#ifdef HB_EFFECTS
void HbProgressBarPrivate::_q_delayedHide(HbEffect::EffectStatus status)
{
    Q_Q(HbProgressBar);

    if (status.reason != Hb::EffectNotStarted) {
        q->setVisible(false);
    } 
}
#endif

/*!
    Constructs a progressbar of a given \a type and \a parent. The default type is SimpleProgressBar.

    \deprecated HbProgressBar::HbProgressBar(HbProgressBar::ProgressBarType, QGraphicsItem*)
        is deprecated. Please use HbProgressBar::HbProgressBar(QGraphicsItem *parent) instead.
*/
HbProgressBar::HbProgressBar(ProgressBarType type, QGraphicsItem *parent) : 
    HbWidget(*new HbProgressBarPrivate, parent)
{    
    Q_D( HbProgressBar );
    d->q_ptr = this;
    d->init();
    d->mType = type;
}

/*!
    @beta
    Constructs a progressbar of a given \a parent.
*/
HbProgressBar::HbProgressBar(QGraphicsItem *parent) : 
    HbWidget(*new HbProgressBarPrivate, parent)
{    
    Q_D( HbProgressBar );
    d->q_ptr = this;
    d->init();
}

/*!
    \deprecated HbProgressBar::HbProgressBar(HbProgressBarPrivate&, HbProgressBar::ProgressBarType, QGraphicsItem*)
        is deprecated. Please use HbProgressBar::HbProgressBar(HbProgressBarPrivate &dd, QGraphicsItem *parent) instead.
*/
HbProgressBar::HbProgressBar(HbProgressBarPrivate &dd, ProgressBarType type, QGraphicsItem *parent) : 
    HbWidget( dd, parent)
{   
    Q_D( HbProgressBar );
    d->init();
    d->mType = type;
}


HbProgressBar::HbProgressBar(HbProgressBarPrivate &dd, QGraphicsItem *parent) : 
    HbWidget( dd, parent)
{   
    Q_D( HbProgressBar );
    d->init();
}


/*!
    @beta
    Destructor for the progressbar.
*/
HbProgressBar::~HbProgressBar() 
{    
}

/*!
    @beta
   This property holds whether or not a progress bar shows its progress inverted. The function
   returns the value of this property.

   Not implemented yet.

   \sa setInvertedAppearance()
 */
bool HbProgressBar::invertedAppearance() const 
{
    Q_D( const HbProgressBar );
    return d->mInvertedAppearance;
}

/*!
    @beta
   This property holds whether or not a progress bar shows its progress inverted. The function
   sets the property to the \a inverted value.

   Not implemented yet.

   \sa invertedAppearance()
 */
void HbProgressBar::setInvertedAppearance(bool inverted)
{
    Q_D( HbProgressBar );
    d->mInvertedAppearance=inverted;
    updatePrimitives();
}

/*!
    @beta
    Returns the maximum value of the progress bar.

    The default value is \c 100.

    \sa setMaximum()
*/
int HbProgressBar::maximum() const
{   
    Q_D( const HbProgressBar );
    return d->mMaximum;
}

/*!
    @beta
    Sets the maximum value of the progress bar.
    when setting this property, the minimum is adjusted to ensure
    that the range remains valid.

    \sa maximum(),minimum(),setMinimum()
*/
void HbProgressBar::setMaximum(int maximum)
{
    Q_D( HbProgressBar );
    d->setRange(qMin(d->mMinimum,maximum),maximum);
}

/*!
    @beta
    Returns the minimum value of the progress bar.

    The default value is \c 0.

    \sa setMinimum()
*/
int HbProgressBar::minimum() const
{
    Q_D( const HbProgressBar );
    return d->mMinimum;
}

/*!
    @beta
    Sets the minimum value of the progress bar.
    when setting this property, the maximum is adjusted to ensure
    that the range remains valid.

    \sa maximum(),minimum(),setMaximum()

*/
void HbProgressBar::setMinimum(int minimum)
{
    Q_D( HbProgressBar );
    d->setRange(minimum,qMax(d->mMaximum,minimum));
}

/*!
    @beta
    Returns the current value of the progress bar.

    The default progressValue is \c 0.
*/
int HbProgressBar::progressValue() const
{
    Q_D( const HbProgressBar );
    return d->mProgressValue;
}

/*!
    @beta
    Sets the current value of the progress bar.

    The progress bar forces the value to be within the legal range: \b
    minimum <= \c value <= \b maximum.

    \sa progressValue()
*/
void HbProgressBar::setProgressValue(int value)
{
    Q_D( HbProgressBar );
    if (d->mProgressValue == value) {
        return;
    }
    if (value >= d->mMaximum) {
        value = d->mMaximum;
#ifdef HB_EFFECTS
        HbEffect::start(d->mTrack, HB_PRGRESSBAR_ITEM_TYPE, "progressbar_progress_complete");
#endif
    }
    else if (value<d->mMinimum) {
        value = d->mMinimum;
    }
    d->mProgressValue=value;

    //redraw track
    HbStyleOptionProgressBar progressBarOption;
    initStyleOption(&progressBarOption);
    if(d->mTrack) {
        style()->updatePrimitive(d->mTrack, HbStyle::P_ProgressBar_track, &progressBarOption);
    }

    emit valueChanged(value);
}

/*!
    @beta
    This function is provided for convenience.

    Sets the progress bar's minimum to \a minimum and its maximum to \a max.

    If \a maximum is smaller than minimum, minimum becomes the only valid legal
    value.

    \sa setMinimum(), setMaximum()
*/
void HbProgressBar::setRange(int minimum, int maximum) 
{
    Q_D( HbProgressBar );
    d->setRange(minimum , maximum);
}

/*!
    @beta
    Set the \a Min text shown on the progressbar.
    \sa text()
*/
void HbProgressBar::setMinText(const QString &text)
{
    Q_D(HbProgressBar);
    if (d->mMinText != text) {
        d->mMinText = text;
        HbStyleOptionProgressBar progressBarOption;
        initStyleOption(&progressBarOption);
        style()->updatePrimitive(d->mMinTextItem,HbStyle::P_ProgressBar_mintext,&progressBarOption);
     }
}

/*!
    @beta
    Returns the Min Text of the progress bar.

    The default progressValue is \c 0.
*/
QString HbProgressBar::minText() const
{
    Q_D( const HbProgressBar );
    return d->mMinText;
}

/*!
    @beta
    Set the \a Max text shown on the progressbar.
    \sa text()
*/
void HbProgressBar::setMaxText(const QString &text)
{
  	Q_D(HbProgressBar);
    if (d->mMaxText != text) {
        d->mMaxText = text;
        HbStyleOptionProgressBar progressBarOption;
        initStyleOption(&progressBarOption);
        style()->updatePrimitive(d->mMaxTextItem,HbStyle::P_ProgressBar_maxtext,&progressBarOption);
    }
}

/*!
    @beta
    Returns the Max Text of the progress bar.

    The default progressValue is \c 0.
*/
QString HbProgressBar::maxText() const
{
    Q_D( const HbProgressBar );
    return d->mMaxText;
}

/*!
    @beta 
    Set the MinMaxtext visibility \a true for showing text,false for hiding the text.
    The default is \c false. Min Max text doesnt have a background and would have a transparent background.
    \sa isMinMaxTextVisible().
*/
void HbProgressBar::setMinMaxTextVisible(bool visible)
{
    Q_D(HbProgressBar);
    if (d->mMinMaxTextVisible != visible) {
        d->mMinMaxTextVisible = visible;

    if(d->mMinMaxTextVisible) {
        if(!d->mMinTextItem && !d->mMaxTextItem){
            d->createTextPrimitives();
        }
            d->mMinTextItem->show();
            d->mMaxTextItem->show();
    } else {
        if(d->mMinTextItem && d->mMaxTextItem){
            d->mMinTextItem->hide();
            d->mMaxTextItem->hide();
        }
    }
    repolish();
    updatePrimitives();
    }
}

/*!
    @beta
    This property holds whether the MinMax text should be displayed.
    Return the value of this property.

    \sa setMinMaxTextVisibile()
*/
bool HbProgressBar::isMinMaxTextVisible() const
{
    Q_D(const HbProgressBar);
    return d->mMinMaxTextVisible;
}

/*!
    @beta
    Sets the Min-Max text alignment
	
	Supportted alignments are (in both horizontal and vertical orientations)
	Qt::AlignTop
	Qt::AlignBottom
	Qt::AlignCenter

    In Vertical orienatation,     
    AlignTop is equivalent to Left
    AlignBottom is equivalent to Right

*/
void HbProgressBar::setMinMaxTextAlignment(Qt::Alignment alignment)
{
	Q_D(HbProgressBar);
	if( (alignment != Qt::AlignBottom) && (alignment != Qt::AlignTop) && (alignment != Qt::AlignCenter) ) {
		return;
	}
	if (d->mMinMaxTextAlignment != alignment) {
        d->mMinMaxTextAlignment = alignment;
		if (d->mMinMaxTextVisible) {
            repolish();
        }
        updatePrimitives();
    }

}
/*!
    @beta
    Returns the minmax Text alignment.
   
*/
Qt::Alignment HbProgressBar::minMaxTextAlignment() const
{
	Q_D(const HbProgressBar);
	return d->mMinMaxTextAlignment;
}

/*!
    @beta
    sets the orientation of the progressbar.It can be vertical or horizontal.
*/
void HbProgressBar::setOrientation(Qt::Orientation orientation)
{
    //TODO: Add vertical slider related info.
    Q_D(HbProgressBar);
    d->setOrientation(orientation);
}

/*!
    @beta
    Returns the orientation of the progressbar.It can be vertical or horizontal.
*/
Qt::Orientation HbProgressBar::orientation() const
{
    //TODO: Add vertical slider related info.
    Q_D(const HbProgressBar);
    return d->mOrientation;    
}

/*!
    Returns the pointer for \a primitive passed.
    Will return NULL if \a primitive passed is invalid
*/
QGraphicsItem* HbProgressBar::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbProgressBar);

    switch (primitive) {
        case HbStyle::P_ProgressBar_frame:
            return d->mFrame;
        case HbStyle::P_ProgressBar_track:
            return d->mTrack;  
        case HbStyle::P_ProgressBar_waittrack:
            return d->mWaitTrack;
        case HbStyle::P_ProgressBar_mintext:
            return d->mMinTextItem;
        case HbStyle::P_ProgressBar_maxtext:
            return d->mMaxTextItem;
        default:
            return 0;
    }
}

/*!
    \reimp
 */
void HbProgressBar::updatePrimitives()
{
    Q_D(HbProgressBar);

    if(isVisible()){
        HbStyleOptionProgressBar progressBarOption;
        initStyleOption(&progressBarOption);
        if (d->mFrame) {
            style()->updatePrimitive(d->mFrame, HbStyle::P_ProgressBar_frame, &progressBarOption);			
        }
     
        if (d->mTrack) {
                style()->updatePrimitive(d->mTrack, HbStyle::P_ProgressBar_track, &progressBarOption);
        }
        
        if (d->mWaitTrack) {
                style()->updatePrimitive(d->mWaitTrack, HbStyle::P_ProgressBar_waittrack, &progressBarOption);
        }
        
        if(d->mMinTextItem && d->mMinMaxTextVisible) {
            style()->updatePrimitive(d->mMinTextItem,HbStyle::P_ProgressBar_mintext,&progressBarOption);	
        }

        if(d->mMaxTextItem && d->mMinMaxTextVisible) {
            style()->updatePrimitive(d->mMaxTextItem,HbStyle::P_ProgressBar_maxtext,&progressBarOption);	
        }
    }
    HbWidget::updatePrimitives();
}

/*!
    Initializes \a option with the values from this HbProgressBar. This method 
    is useful for subclasses when they need a HbStyleOptionProgressBar, but don't
    want to fill in all the information themselves.
 */

void HbProgressBar::initStyleOption(HbStyleOption *hboption) const
{
    Q_D( const HbProgressBar );
    Q_ASSERT(hboption);

    HbWidget::initStyleOption(hboption);
    HbStyleOptionProgressBar *option = 0;
    if ((option = qstyleoption_cast< HbStyleOptionProgressBar *>(hboption)) != 0) {
        
        option->progressValue = d->mProgressValue;
        option->maximum = d->mMaximum;
        option->minimum = d->mMinimum;
        option->minText = d->mMinText;
        option->maxText = d->mMaxText;
        option->orientation = d->mOrientation;
        option->isSlider=d->mType == HbProgressBar::RatingProgressBar;
        option->inverted = d->mInvertedAppearance;
        option->stopWaitAnimation = false;
        option->minMaxTextAlignment = d->mMinMaxTextAlignment;
		QRect rect(d->mFrame->boundingRect().x(),d->mFrame->boundingRect().y(),d->mFrame->boundingRect().width(),
		d->mFrame->boundingRect().height());
		option->rect = rect;
    }
}

/*!
    \reimp
 */
void HbProgressBar::closeEvent ( QCloseEvent * event )
{
    Q_D(HbProgressBar);
    HbStyleOptionProgressBar progressBarOption;
    initStyleOption(&progressBarOption);
    progressBarOption.stopWaitAnimation = true;
    if (d->mWaitTrack) {
        style()->updatePrimitive(d->mWaitTrack, HbStyle::P_ProgressBar_waittrack, &progressBarOption);
    }
    event->accept();
}

/*!
    \reimp
 */
QVariant HbProgressBar::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemVisibleHasChanged && value.toBool()){
        updatePrimitives();
    }
#ifdef HB_EFFECTS
    Q_D(HbProgressBar);
    if(change == QGraphicsItem::ItemVisibleChange){
        if(value.toBool()) {

            HbEffect::start(this, HB_PRGRESSBAR_ITEM_TYPE, "progressbar_appear");

            d->mDelayHideInProgress  = false;
        }
        else if(!d->mDelayHideInProgress) {
            //parentItemVisibility check is a hack . Other wise if visibility 
            // change happends due to view switch we should not call explicit hide on 
            // progress bar.
            if(parentItem() && parentItem()->isVisible())
                if(HbEffect::start(this, HB_PRGRESSBAR_ITEM_TYPE, "progressbar_disappear",this,"_q_delayedHide")){
                    d->mDelayHideInProgress = true;
                    return true;
                }
        }
    }
#endif//HB_EFFECTS

   return HbWidget::itemChange(change,value);
}

/*!
    \reimp
 */
void HbProgressBar::changeEvent(QEvent *event)
{
    HbWidget::changeEvent(event);
    
    switch (event->type()) {
    case QEvent::LayoutDirectionChange:
	    if(layoutDirection() == Qt::RightToLeft) {
            setInvertedAppearance(true);
        }
        else {
            setInvertedAppearance(false);
        }
        break;
    default:
        break;
    }
}

#include "moc_hbprogressbar.cpp"

