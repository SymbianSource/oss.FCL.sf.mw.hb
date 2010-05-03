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

#include "hbindexfeedback.h"
#include "hbindexfeedback_p.h"

#include <hbscrollbar.h>
#include <hbabstractitemview.h>
#include <hbabstractviewitem.h>
#include <hbstyle.h>
#include <hbabstractitemcontainer.h>
#include <hbapplication.h>

#include <hbeffect.h>
#include <hbeffectinternal_p.h>

#include <hbstyleoptionindexfeedback.h>

#include <QTimer>
#include <QSize>
#include <QGraphicsScene>

// rather than having magic numbers
// defaults and constants are defined here.
namespace {
/* 
    Press timeout is the duration after the scrollbar is 
    pressed to when the index feedback will be dismissed.

    In the case that the press timeout has not yet passed
    and the timer should be fired with the release timeout,
    the timer with the press timeout will continue running.
*/
static const int DEFAULT_INDEX_FEEDBACK_PRESS_TIMEOUT = 1000;

/*
    Dwell timeout is the duration after which if there is not a change
    in the scrollbar's value the index feedback will be dismissed.
*/
static const int DEFAULT_INDEX_FEEDBACK_DWELL_TIMEOUT = 2000;

/*
    Release timeout is the duration after which the user has lifted
    their finger from the scrollbar to when the index feedback will be dismissed.
*/
static const int DEFAULT_INDEX_FEEDBACK_RELEASE_TIMEOUT = 250;

/*
    The default value for the index feedback policy.
*/
static const HbIndexFeedback::IndexFeedbackPolicy DEFAULT_INDEX_FEEDBACK_POLICY = HbIndexFeedback::IndexFeedbackSingleCharacter;

/*
    The value of the index feedback's disappear animation
*/
static const int INDEX_FEEDBACK_DISAPPEAR_DURATION = 300;
}

/*
    constructor
*/
HbIndexFeedbackPrivate::HbIndexFeedbackPrivate() :
    mIndexFeedbackPressTimeout(DEFAULT_INDEX_FEEDBACK_PRESS_TIMEOUT),
    mIndexFeedbackDwellTimeout(DEFAULT_INDEX_FEEDBACK_DWELL_TIMEOUT),
    mIndexFeedbackReleaseTimeout(DEFAULT_INDEX_FEEDBACK_RELEASE_TIMEOUT),
    mIndexFeedbackPolicy(DEFAULT_INDEX_FEEDBACK_POLICY),
    mVerticalScrollBarValue(-1.0),
    mHorizontalScrollBarValue(-1.0),
    mIndexFeedbackTimer(0),
    mDisappearTimer(0),
    mTextItem(0),
    mPopupItem(0),
    mItemView(0),
    mOneCharHeight(0.0),
    mThreeCharHeight(0.0),
    mThreeCharWidth(0.0),
    mStringOffset(0.0)
{

}

/*
    Destructor
*/
HbIndexFeedbackPrivate::~HbIndexFeedbackPrivate()
{

}

/*
    2ndary construction, called from public's constructor.
*/
void HbIndexFeedbackPrivate::init()
{
    Q_Q( HbIndexFeedback );

    //mItemView = 0; // double check that this is safe.

    HbEffect::add(HB_INDEXFEEDBACK_TYPE, "indexfeedback_appear", EFFECT_IFAPPEAR);
    if (!HbEffect::add(HB_INDEXFEEDBACK_TYPE, "indexfeedback_disappear", EFFECT_IFDISAPPEAR)) {
        mDisappearTimer = new QTimer(q);
        mDisappearTimer->setSingleShot(true);
        mDisappearTimer->setInterval(INDEX_FEEDBACK_DISAPPEAR_DURATION);
        q->connect(mDisappearTimer, SIGNAL(timeout()),
            q, SLOT(_q_hideIndexFeedbackNow()));
    }

    mIndexFeedbackTimer = new QTimer(q);
    mIndexFeedbackTimer->setSingleShot(true);
    q->connect(mIndexFeedbackTimer, SIGNAL(timeout()),
        q, SLOT(_q_hideIndexFeedback()));

    createPrimitives();
}

/*
    Update the data for, and show the index feedback (if it's not already shown)
*/
void HbIndexFeedbackPrivate::showIndexFeedback()
{
    if (!mItemView 
        || mIndexFeedbackPolicy == HbIndexFeedback::IndexFeedbackNone) {
        return;
    }

    QList <HbAbstractViewItem *> visibleItems = mItemView->visibleItems();

    QModelIndex targetIndex = visibleItems.first()->modelIndex();   
    qreal top = visibleItems.first()->mapToItem(mItemView, 0, 0).y();
    if (top < 0 
        && visibleItems.count() > 1) {
        targetIndex = visibleItems.at(1)->modelIndex();
    }

    if (targetIndex.isValid()) {
        QVariant data = targetIndex.data(Hb::IndexFeedbackRole);
        if (data.canConvert<QString>()) {

            QString testString = displayText(data);
            if (testString != mPopupContent) {
                mPopupContent = testString;
                updatePrimitives();
            }

            if (mTextItem && mTextItem->opacity() == 0.0) {
                HbEffect::start(mPopupItemList, HB_INDEXFEEDBACK_TYPE, EFFECT_IFAPPEAR);
            }
            if (mTextItem) {
                mTextItem->show();
            }

            if (mPopupItem) {
                mPopupItem->show();
            }

            if (mDisappearTimer) {
                mDisappearTimer->stop();
            }
        } else {
            _q_hideIndexFeedback();
        }
    }
}

/*
    Returns the qstring which will be the text in the index feedback.
*/
QString HbIndexFeedbackPrivate::displayText(const QVariant &data) const
{
    QString retVal = QString();

    switch (mIndexFeedbackPolicy) {
        case HbIndexFeedback::IndexFeedbackSingleCharacter:
            retVal = data.toString().left(1);
            break;

        case HbIndexFeedback::IndexFeedbackThreeCharacter:
            retVal = data.toString().left(3);
            break;

        case HbIndexFeedback::IndexFeedbackString:
            retVal = data.toString();
            break;

        default:
            qt_noop();
            break;
    }

    return retVal;
}

/*
    Handle the case of the scrollbar being pressed.

    This is show the index feedback and start the dismissal timer with the
    press timeout.
*/
void HbIndexFeedbackPrivate::scrollBarPressed()
{
    showIndexFeedback();

    // need to record the scrollbar values    

    // TODO::The values are storred here is a work around for a bug in hbscrollbar.
    // the bug is that the value changed signal is emitted when the thumb
    // is pressed, and again when it is released, regaurdless of if there was a value change.
    // once that bug is fixed, this should be removed.
    mVerticalScrollBarValue = mItemView->verticalScrollBar()->value();
    mHorizontalScrollBarValue = mItemView->horizontalScrollBar()->value();

    // and start the press timer.
    mIndexFeedbackTimer->setInterval(mIndexFeedbackPressTimeout);
    mIndexFeedbackTimer->start();
}

/*
    Handle the case of the scrollbar being released.

    This is to start the dismissal timer with the release timeout.
    ...but not if the timer is still running with the press timeout...
*/
void HbIndexFeedbackPrivate::scrollBarReleased()
{
    // record the scrollbar values in case position changed is emitted after us w/o a real change
    
    // TODO::The values are storred here is a work around for a bug in hbscrollbar.
    // the bug is that the value changed signal is emitted when the thumb
    // is pressed, and again when it is released, regaurdless of if there was a value change.
    // once that bug is fixed, this should be removed.
    mVerticalScrollBarValue = mItemView->verticalScrollBar()->value();
    mHorizontalScrollBarValue = mItemView->horizontalScrollBar()->value();

    // start this timer.
    if (!(mIndexFeedbackTimer->isActive()
         && mIndexFeedbackTimer->interval() == mIndexFeedbackPressTimeout)) {
        mIndexFeedbackTimer->setInterval(mIndexFeedbackReleaseTimeout);
        mIndexFeedbackTimer->start();
    }
}

/*
    Handle the case of the scrollbar being moved.

    This is to stop any existing timers (only if the scrollbar actually moved),
    and start a timer with the dwell timeout.

    NOTE:: this should be much simpler once the valueChanged signal from hbscrollbar
    is emitted at the correct times.
*/
void HbIndexFeedbackPrivate::_q_scrollPositionChanged(qreal value, Qt::Orientation orientation )
{
    // using 3 timers.  If the press timer is active, stop it, assuming the value actually changed.

    // TODO::The value check here is a work around for a bug in hbscrollbar.
    // the bug is that the value changed signal is emitted when the thumb
    // is pressed, and again when it is released, regaurdless of if there was a value change.
    // once that bug is fixed, This should be just setting the dwell interval,
    // starting the timer, and showing the index feedback.
    if (value != (orientation == 
            Qt::Vertical ? mVerticalScrollBarValue : mHorizontalScrollBarValue) ) {
        mIndexFeedbackTimer->setInterval(mIndexFeedbackDwellTimeout);
        mIndexFeedbackTimer->start();
        showIndexFeedback();
    }
}

/*
    The private slot for hiding the index feedback.

    If effects are active, use the disappear effect to hide the index feedback's
    primitives.  Otherwise simply hide them.
*/
void HbIndexFeedbackPrivate::_q_hideIndexFeedback()
{
    if (qFuzzyCompare(mTextItem->opacity(), (qreal) 1.0)) {
        HbEffect::start(mPopupItemList, HB_INDEXFEEDBACK_TYPE, EFFECT_IFDISAPPEAR);
    }

    if (mDisappearTimer) {
        mDisappearTimer->start();
    }
}

/*
    A private slot for actually calling hide on the index feedback.

    This should happen after the index feedback's hide animation is completed
    regardless of if it went successfully.
*/
void HbIndexFeedbackPrivate::_q_hideIndexFeedbackNow()
{
    if (mTextItem) {
        mTextItem->hide();
    }

    if (mPopupItem) {
        mPopupItem->hide();
    }
}

/*
    Do the appropriate thing if the item view we're pointing at is destroyed
*/
void HbIndexFeedbackPrivate::_q_itemViewDestroyed()
{
    mItemView = 0;
}

/*
    Update the primitives for the index feedback.
*/
void HbIndexFeedbackPrivate::updatePrimitives()
{
    Q_Q( HbIndexFeedback );

    HbStyleOptionIndexFeedback option;
    q->initStyleOption(&option);
    if (mTextItem) {
        q->style()->updatePrimitive(mTextItem, HbStyle::P_IndexFeedback_popup_text, &option);
    }
    if (mPopupItem) {
        q->style()->updatePrimitive(mPopupItem, HbStyle::P_IndexFeedback_popup_background, &option);
    }
}

/*
    Create the primitives for the index feedback.
*/
void HbIndexFeedbackPrivate::createPrimitives()
{
    Q_Q( HbIndexFeedback );

    mPopupItemList.clear();

    if (!mTextItem) {
        mTextItem = q->style()->createPrimitive(HbStyle::P_IndexFeedback_popup_text, q);
        mTextItem->hide();
        mPopupItemList.append(mTextItem);
    }

    if (!mPopupItem) {
        mPopupItem = q->style()->createPrimitive(HbStyle::P_IndexFeedback_popup_background, q);
        mPopupItem->hide();
        mPopupItemList.append(mPopupItem);
    }
}

/*
    disconnects the current item view from hbindexfeedback's slots & event filters.
*/
void HbIndexFeedbackPrivate::disconnectItemView()
{
    Q_Q( HbIndexFeedback );

    if (mItemView) {
        // disconnect from the existing itemView's scrollbars
        mItemView->disconnect(q);
        // uninstall the event filters;
        mItemView->horizontalScrollBar()->removeEventFilter(q);
        mItemView->verticalScrollBar()->removeEventFilter(q);

        mItemView->removeSceneEventFilter(q);
        if (mItemView->scene()) {
            mItemView->scene()->removeItem(q);
        }
    }

    mItemView = 0;
}

/*
    Hooks up the private slots & event filter to a scrollbar.
*/
void HbIndexFeedbackPrivate::connectScrollBarToIndexFeedback(HbScrollBar* scrollBar)
{
    Q_Q( HbIndexFeedback );

    if (scrollBar) {
        q->connect(scrollBar, SIGNAL(valueChanged(qreal, Qt::Orientation)),
            q, SLOT(_q_scrollPositionChanged(qreal, Qt::Orientation)));
        scrollBar->installEventFilter(q);
    }
}

/*
    Calculates the rects for the popup text and background.

    Does nothing if the rect is the same as the last time it was called.
*/
void HbIndexFeedbackPrivate::calculatePopupRects()
{
    Q_Q( HbIndexFeedback );

    if (!mItemView) {
        return;
    }

    QRectF contentRect = mItemView->rect();
    
    HbScrollBar *verticalScrollBar = mItemView->verticalScrollBar();
    HbScrollBar *horizontalScrollBar = mItemView->horizontalScrollBar();
    if (verticalScrollBar->isInteractive()) {
        contentRect.setWidth( contentRect.width() - verticalScrollBar->rect().width() );
        if (HbApplication::layoutDirection() == Qt::RightToLeft) {
            contentRect.setLeft( contentRect.left() + verticalScrollBar->rect().width() );
        }
    }
    
    if (horizontalScrollBar->isInteractive()) {
        contentRect.setHeight( contentRect.height() - horizontalScrollBar->rect().height() );
    }

    if (contentRect == mItemViewContentsRect) {
        return;
    }

    qreal margin = 0;
    q->style()->parameter(QLatin1String("hb-param-margin-gene-popup"), margin);

    QSizeF backgroundSize;
    QSizeF textSize;

    switch (mIndexFeedbackPolicy) {
        case HbIndexFeedback::IndexFeedbackSingleCharacter:
        case HbIndexFeedback::IndexFeedbackThreeCharacter:
            {
                textSize.setHeight( textHeight() );
                textSize.setWidth ( textWidth() );

                backgroundSize.setHeight( textSize.height() + 2 * margin );
                backgroundSize.setWidth ( textSize.width() + 2 * margin );

                mPopupBackgroundRect.setLeft( contentRect.left() + (contentRect.width() -
                    backgroundSize.width()) / 2.0 );
                mPopupBackgroundRect.setTop ( contentRect.top() + (contentRect.height() - 
                    backgroundSize.height()) / 2.0 );

                mPopupTextRect.setLeft( mPopupBackgroundRect.left() + margin );
                mPopupTextRect.setTop ( mPopupBackgroundRect.top()  + margin );

                mPopupBackgroundRect.setSize(backgroundSize);
                mPopupTextRect.setSize(textSize);
            }
            break;

        case HbIndexFeedback::IndexFeedbackString:
            {
                textSize.setHeight( textHeight() );
                backgroundSize.setHeight( textSize.height() + 2 * margin );

                backgroundSize.setWidth( contentRect.width() - 2 * mStringOffset );
                textSize.setWidth( backgroundSize.width() - 2 * margin );

                mPopupBackgroundRect.setLeft( contentRect.left() + mStringOffset );
                mPopupBackgroundRect.setTop( contentRect.top() + (contentRect.height() - 
                    backgroundSize.height()) / 2.0 );

                mPopupTextRect.setLeft( mPopupBackgroundRect.left() + margin );
                mPopupTextRect.setTop ( mPopupBackgroundRect.top() + margin );

                mPopupBackgroundRect.setSize(backgroundSize);
                mPopupTextRect.setSize(textSize);
            }
            break;

        case HbIndexFeedback::IndexFeedbackNone:
            {
                mPopupTextRect = QRectF();
                mPopupBackgroundRect = QRectF();
            }
            break;
    }
}

/*
    Returns the text height in pixels.
*/
qreal HbIndexFeedbackPrivate::textHeight() const
{
    Q_Q( const HbIndexFeedback );

    qreal retVal;

    switch (mIndexFeedbackPolicy) {
        case HbIndexFeedback::IndexFeedbackNone:
            retVal = 0.0;
            break;

        case HbIndexFeedback::IndexFeedbackSingleCharacter:
            retVal = mOneCharHeight;
            break;

        case HbIndexFeedback::IndexFeedbackThreeCharacter:
            retVal = mThreeCharHeight;
            break;

        case HbIndexFeedback::IndexFeedbackString:
            q->style()->parameter(QLatin1String("hb-param-text-height-primary"), retVal);
            break;
    }

    return retVal;
}

/*
    Returns the text width in units.
*/
qreal HbIndexFeedbackPrivate::textWidth() const
{
    qreal retVal = 0.0;

    switch (mIndexFeedbackPolicy) {
        case HbIndexFeedback::IndexFeedbackString:
        case HbIndexFeedback::IndexFeedbackNone:
            retVal = 0.0;
            break;

        case HbIndexFeedback::IndexFeedbackSingleCharacter:
            retVal = mOneCharHeight;
            break;

        case HbIndexFeedback::IndexFeedbackThreeCharacter:
            retVal = mThreeCharWidth;
            break;
    }

    return retVal;
}
