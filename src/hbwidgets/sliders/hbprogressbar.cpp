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
#include <hbstyleoptionprogressbar_p.h>
#include "hbprogressbar_p.h"
#include "hbglobal_p.h"

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
    
    \image html progressbar.png  "A Progress Bar with Min-Max text at bottom"
    \image html infiniteprogressbar.png  "An infinite progres bar"

    The HbProgressBar widget provides a horizontal or vertical progress bar.

    A progress bar is used to give the user an indication of the progress of an operation and to reassure them that 
    the application is still running. The progress bar uses the concept of steps. You set it up by specifying the 
    minimum and maximum possible step values, and it will display the percentage of steps that have been completed 
    when you later give it the current step value. The percentage is calculated by dividing the 
    progress (value() - minimum()) divided by maximum() - minimum().
    
    By default the min value is 0 and max value is 100.If minimum and maximum both are set to 0, the bar shows a busy indicator 
    instead of a percentage of steps.The ProgressBar is always non interactive.

    ProgressBar also supports adding text . Min-Max text pair is also supported which is commonly 
    used for progress indication for music. 
    
    Progress bar provides below signal.

    \li valueChanged(int value) Emitted when the value of the progressbar is changed.


    Example code for creating normal ProgressBar:
    \code
    HbProgressBar *pb = new HbProgressBar();
    pb->setMinimum(0);
    pb->setMaximum(500);
    pb->setProgressValue(175);
    \endcode

    Example code for creating infinite ProgressBar:
    \code
    HbProgressBar *pb = new HbProgressBar();
    pb->setMinimum(0);
    pb->setMaximum(0);
    \endcode

    Example code for creating normal ProgressBar with Min-Max text at Top:
    \code
    HbProgressBar *pb = new HbProgressBar();
    pb->setMinimum(0);
    pb->setMaximum(500);
    pb->setProgressValue(175);
    pb->setMinMaxTextVisible(true);
    pb->setMinMaxTextAlignment(Qt::AlignTop);// The possible options are Qt::AlignTop ,Qt::AlignBottom ,Qt::AlignCenter
    pb->setminText("0");
    pb->setmaxText("500");

    \endcode

    Example code for creating vertical normal ProgressBar:
    \code
    HbProgressBar *pb = new HbProgressBar();
    pb->setOrientation(Qt::Vertical);
    pb->setMinimum(0);
    pb->setMaximum(500);
    pb->setProgressValue(175);
    \endcode
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
    mDelayHideInProgress(true),
    mShowEffectInProgress(false)
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

void HbProgressBarPrivate::setProgressValue(int value)
{
   Q_Q(HbProgressBar);
   if (mProgressValue == value) {
        return;
    }
    if (value >= mMaximum) {
        value = mMaximum;
#ifdef HB_EFFECTS
        HbEffect::start(mTrack, HB_PRGRESSBAR_ITEM_TYPE, "progressbar_progress_complete");
#endif
    }
    else if (value < mMinimum) {
        value = mMinimum;
    }
    
    mProgressValue=value;

    //redraw track
    HbStyleOptionProgressBar progressBarOption;
    q->initStyleOption(&progressBarOption);
    if(mTrack) {
        q->style()->updatePrimitive(mTrack, HbStyle::P_ProgressBar_track, &progressBarOption);
    }

    emit q->valueChanged(value);
    
}

/*
    \internal
    Sets the progressbar enabling/disabling
*/
void HbProgressBarPrivate::setEnableFlag(bool flag)
{
    Q_Q(HbProgressBar);
    if(!flag) {
        q->setProgressValue(q->minimum());

    }
}

/*
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

    if( (mMinimum == 0) && (mMaximum == 0) ) {
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

/*!
    \internal
    Private slot which delays the show effect.
*/
void HbProgressBarPrivate::_q_delayedShow(HbEffect::EffectStatus status)
{
    Q_UNUSED(status);
    mShowEffectInProgress = false;
}
#endif

/*!
    @beta
   Constructs a progress bar with the given parent.
   By default, the minimum step value is set to 0, and the maximum to 100.
   \param parent The parent of ProgressBar

*/
HbProgressBar::HbProgressBar(QGraphicsItem *parent) : 
    HbWidget(*new HbProgressBarPrivate, parent)
{    
    Q_D( HbProgressBar );
    d->q_ptr = this;
    d->init();
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
    Return the inverted appearence property. 
    \sa setInvertedAppearance()
*/
bool HbProgressBar::invertedAppearance() const 
{
    Q_D( const HbProgressBar );
    return d->mInvertedAppearance;
}

/*!
    @beta
    Sets the inverted appearence. If this is true progress grows from right to 
    left otherwise left to right.

    \param inverted true or false.

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
    Returns the maximum value of the progressbar. By default it is 100. 

    \sa setMaximum()
*/
int HbProgressBar::maximum() const
{   
    Q_D( const HbProgressBar );
    return d->mMaximum;
}

/*!
    @beta
    Sets the maximum value of the progressbar. By default it is 100. 

    \param maximum the maximum value

    \sa maximum()
*/
void HbProgressBar::setMaximum(int maximum)
{
    Q_D( HbProgressBar );
    d->setRange(qMin(d->mMinimum,maximum),maximum);
}

/*!
    @beta
    Returns the minimum value of the progressbar. By default it is 0. 

    \sa setMinimum()
*/
int HbProgressBar::minimum() const
{
    Q_D( const HbProgressBar );
    return d->mMinimum;
}

/*!
    @beta
    Sets the minimum value of the progressbar. By default it is 0. 

    \param minimum the minimum value

    \sa minimum()
*/
void HbProgressBar::setMinimum(int minimum)
{
    Q_D( HbProgressBar );
    d->setRange(minimum,qMax(d->mMaximum,minimum));
}

/*!
    @beta
    Returns the current value of the progress bar.
    \sa setProgressValue()
*/
int HbProgressBar::progressValue() const
{
    Q_D( const HbProgressBar );
    return d->mProgressValue;
}

/*!
    @beta
    Sets the progress value of the progressbar. 

    \param value the progress value

    \sa progressValue()
*/
void HbProgressBar::setProgressValue(int value)
{
    Q_D( HbProgressBar );
    d->setProgressValue(value);
}

/*!
    @beta
    This function is provided for convenience.
    Sets the progress bar's minimum and its maximum.

    If  maximum is smaller than minimum, minimum becomes the only valid legal
    value.

    \param minimum the minimum value
    \param maximum the maximum value
*/
void HbProgressBar::setRange(int minimum, int maximum) 
{
    Q_D( HbProgressBar );
    d->setRange(minimum , maximum);
}

/*!
    @beta
    A text can be shown at top,bottom or left-right of the progressbar near minimum and maximum.
    This will set the text near the minimum point.

    \param text mintext string

    \sa minText()
*/
void HbProgressBar::setMinText(const QString &text)
{
    Q_D(HbProgressBar);
    if (d->mMinText != text) {
        d->mMinText = text;
        HbStyleOptionProgressBar progressBarOption;
        progressBarOption.minText = d->mMinText;
        style()->updatePrimitive(d->mMinTextItem,HbStyle::P_ProgressBar_mintext,&progressBarOption);
     }
}

/*!
    @beta
    Returns the Min Text of the progress bar.
    \sa setMinText()
*/
QString HbProgressBar::minText() const
{
    Q_D( const HbProgressBar );
    return d->mMinText;
}

/*!
    @beta
    A text can be shown at top,bottom or left-right of the progressbar near minimum and maximum.
    This will set the text near the minimum point.

    \param text max text string

    \sa maxText()
*/
void HbProgressBar::setMaxText(const QString &text)
{
    Q_D(HbProgressBar);
    if (d->mMaxText != text) {
        d->mMaxText = text;
        HbStyleOptionProgressBar progressBarOption;
        progressBarOption.maxText = d->mMaxText;
        style()->updatePrimitive(d->mMaxTextItem,HbStyle::P_ProgressBar_maxtext,&progressBarOption);
    }
}

/*!
    @beta
    Returns the Max Text of the progress bar.
    \sa setMaxText()
*/
QString HbProgressBar::maxText() const
{
    Q_D( const HbProgressBar );
    return d->mMaxText;
}

/*!
    @beta 
    Set the MinMaxtext visibility. true for showing text,false for hiding the text.
    The default is  false. Min Max text doesnt have a background and would have a transparent background.
    \param visible true or false.
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
    Returns the MinMax visibility.
    \sa setMinMaxTextVisible()
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

    \param alignment alignement for the min max text
    \sa isMinMaxTextVisible().

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
    \sa setMinMaxTextAlignment().
   
*/
Qt::Alignment HbProgressBar::minMaxTextAlignment() const
{
    Q_D(const HbProgressBar);
    return d->mMinMaxTextAlignment;
}

/*!
    @beta
    sets the orientation of the progressbar.It can be vertical or horizontal.
    \param orientation Horizontal or Vertical
    \sa orientation().
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
    \sa setOrientation().
*/
Qt::Orientation HbProgressBar::orientation() const
{
    //TODO: Add vertical slider related info.
    Q_D(const HbProgressBar);
    return d->mOrientation;    
}

/*!
    \deprecated HbProgressBar::primitive(HbStyle::Primitive)
        is deprecated.
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
    \reimp
*/

void HbProgressBar::initStyleOption(HbStyleOptionProgressBar *option) const
{
    Q_D( const HbProgressBar );

    HbWidget::initStyleOption(option);

    option->progressValue = d->mProgressValue;
    option->maximum = d->mMaximum;
    option->minimum = d->mMinimum;
    option->minText = d->mMinText;
    option->maxText = d->mMaxText;
    option->orientation = d->mOrientation;
    option->inverted = d->mInvertedAppearance;
    option->stopWaitAnimation = false;
    option->minMaxTextAlignment = d->mMinMaxTextAlignment;
    QRect rect((int)d->mFrame->boundingRect().x(),(int)d->mFrame->boundingRect().y(),(int)d->mFrame->boundingRect().width(),
    (int)d->mFrame->boundingRect().height());
    option->rect = rect;
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

            if(!d->mShowEffectInProgress) {
                HbEffect::start(this, HB_PRGRESSBAR_ITEM_TYPE, "progressbar_appear",this,"_q_delayedShow");
                d->mShowEffectInProgress = true;
            }

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
    Q_D(HbProgressBar);
    switch (event->type()) {
        case QEvent::LayoutDirectionChange:
            if(layoutDirection() == Qt::RightToLeft) {
                setInvertedAppearance(true);
            }
            else {
                setInvertedAppearance(false);
            }
            break;
        case QEvent::EnabledChange:
            {
                if (!isEnabled()) {
                   d->setEnableFlag(false);
                }
                else
                {
                    d->setEnableFlag(true);
                }
            }
        default:
            break;
    }

    HbWidget::changeEvent(event);
}

#include "moc_hbprogressbar.cpp"
