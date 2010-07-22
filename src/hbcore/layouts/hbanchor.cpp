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

#include "hbanchor.h"
#include "hbanchor_p.h"
#include "hbanchorlayout.h"

#ifndef HB_BOOTSTRAPPED
#include <QWidget> // QWIDGETSIZE_MAX
#else
#define QWIDGETSIZE_MAX ((1<<24)-1)
#endif

/*!
    \class HbAnchor
    \brief HbAnchor class represents anchors used by HbAnchorLayout.

    HbAnchor instance has own size hints and size policy. It can have
    minimum/maximum/preferred sizes and any of size policies supported by Qt.
    By default size policy is QSizePolicy::Fixed and anchor size is zero.

    HbAnchor cannot have negative size hints, instead you can use
    HbAnchor::Direction for turning anchor to opposite side.

    Note: when you pass negative number to size hint, direction is
    changed automatically to HbAnchor::Negative, and absolute value
    is for requested size hint.

    \stable
*/

/*!
    internal
*/
HbAnchorPrivate::HbAnchorPrivate()
    : mStartItem(0), mStartId(), mEndItem(0), mEndId(), mMinValue(0), mPrefValue(0), mMaxValue(QWIDGETSIZE_MAX),
      mPolicy(QSizePolicy::Fixed), mDir(HbAnchor::Positive), mParent(0), mAnchorId()
    {
    }


/*!
    internal
*/
void HbAnchorPrivate::setInitialLength( qreal length )
{
    mDir = (length < 0) ? HbAnchor::Negative : HbAnchor::Positive;
    mPrefValue = qAbs(length);
}

/*!
    Constructor.
*/
HbAnchor::HbAnchor( const QString& startNodeId, Hb::Edge startEdge, const QString& endNodeId, Hb::Edge endEdge, qreal length ) : d_ptr( new HbAnchorPrivate )
{
    Q_D( HbAnchor );
    d->mStartEdge = startEdge;
    d->mStartId = startNodeId;
    d->mEndEdge = endEdge;
    d->mEndId = endNodeId;

    d->setInitialLength( length );
}

/*!
    Constructor.
*/
HbAnchor::HbAnchor( const QString& startNodeId, Hb::Edge startEdge, QGraphicsLayoutItem *endItem, Hb::Edge endEdge, qreal length ) : d_ptr( new HbAnchorPrivate )
{
    Q_D( HbAnchor );
    d->mStartId = startNodeId;
    d->mStartEdge = startEdge;
    d->mEndEdge = endEdge;
    d->mEndItem = endItem;

    d->setInitialLength( length );
}

/*!
    Constructor.
*/
HbAnchor::HbAnchor( QGraphicsLayoutItem *startItem, Hb::Edge startEdge, const QString& endNodeId, Hb::Edge endEdge, qreal length ) : d_ptr( new HbAnchorPrivate )
{
    Q_D( HbAnchor );
    d->mStartEdge = startEdge;
    d->mStartItem = startItem;
    d->mEndEdge = endEdge;
    d->mEndId = endNodeId;

    d->setInitialLength( length );
}

/*!
    Constructor.
*/
HbAnchor::HbAnchor( QGraphicsLayoutItem *startItem, Hb::Edge startEdge, QGraphicsLayoutItem *endItem, Hb::Edge endEdge, qreal length ) : d_ptr( new HbAnchorPrivate )
{
    Q_D( HbAnchor );
    d->mStartEdge = startEdge;
    d->mStartItem = startItem;
    d->mEndEdge = endEdge;
    d->mEndItem = endItem;

    d->setInitialLength( length );
}


/*!
    Destructor.
*/
HbAnchor::~HbAnchor()
{
    delete d_ptr;
}


/*!
    Returns reference to HbAnchorLayout, where this anchor is set. If this anchor has not
    been set to any layout yet, then this function returns zero.

    \return reference to HbAnchorLayout, or zero.
*/
HbAnchorLayout *HbAnchor::parent() const
{
    Q_D( const HbAnchor );
    return d->mParent;
}

/*!
    Assigns other to this anchor and returns a reference to this anchor.
*/
HbAnchor &HbAnchor::operator=(const HbAnchor &other)
{
    Q_D( HbAnchor );
    if (this != &other) {
        d->mStartItem = other.startItem();
        d->mStartId = other.startNodeId();
        d->mStartEdge = other.startEdge();
        d->mEndItem = other.endItem();
        d->mEndId = other.endNodeId();
        d->mEndEdge = other.endEdge();
        d->mMinValue = other.minimumLength();
        d->mPrefValue = other.preferredLength();
        d->mMaxValue = other.maximumLength();
        d->mPolicy = other.sizePolicy();
        setDirection( other.direction() );
        d->mParent = other.parent();
    }
    return *this;
}


/*!
    Sets size policy to this anchor.

    \param policy size policy to be set
*/
void HbAnchor::setSizePolicy( QSizePolicy::Policy policy )
{
    Q_D( HbAnchor );
    if ( d->mPolicy != policy ) {
        d->mPolicy = policy;
        if( d->mParent ) {
            d->mParent->updateGeometry();
        }
    }
}


/*!
    Returns current size policy. By default size policy is QSizePolicy::Fixed

    \return current size policy
*/
QSizePolicy::Policy HbAnchor::sizePolicy() const
{
    Q_D( const HbAnchor );
    return d->mPolicy;
}

/*!
    Sets minimum length to this anchor.

    Note: only non-negative values accepted.

    \param length minimum length to be set
*/
void HbAnchor::setMinimumLength( qreal length )
{
    Q_D( HbAnchor );
    Q_ASSERT( length >= 0 );
    if ( d->mMinValue != length ) {
        d->mMinValue = length;
        if( d->mParent ) {
            d->mParent->updateGeometry();
        }
    }
}

/*!
    Sets preferred length to this anchor.

    Note: only non-negative values accepted.

    \param length preferred length to be set
*/
void HbAnchor::setPreferredLength( qreal length )
{
    Q_D( HbAnchor );
    Q_ASSERT( length >= 0 );
    if ( d->mPrefValue != length ) {
        d->mPrefValue = length;
        if( d->mParent ) {
            d->mParent->updateGeometry();
        }
    }
}

/*!
    Sets maximum length to this anchor.

    Note: only non-negative values accepted.

    \param length maximum length to be set
*/
void HbAnchor::setMaximumLength( qreal length )
{
    Q_D( HbAnchor );
    Q_ASSERT( length >= 0 );
    if ( d->mMaxValue != length ) {
        d->mMaxValue = length;
        if( d->mParent ) {
            d->mParent->updateGeometry();
        }
    }
}

/*!
    Returns minimum length of this anchor.

    \return minimum length of this anchor
*/
qreal HbAnchor::minimumLength() const
{
    Q_D( const HbAnchor );
    return d->mMinValue;
}

/*!
    Returns preferred length of this anchor.

    \return preferred length of this anchor
*/
qreal HbAnchor::preferredLength() const
{
    Q_D( const HbAnchor );
    return d->mPrefValue;
}

/*!
    Returns maximum length of this anchor.

    \return maximum length of this anchor
*/
qreal HbAnchor::maximumLength() const
{
    Q_D( const HbAnchor );
    return d->mMaxValue;
}

/*!
    Returns length hint of this anchor.

    \param which here you specify which size hint to return
    \return value of requested length hint
*/
qreal HbAnchor::lengthHint( Qt::SizeHint which ) const
{
    Q_D( const HbAnchor );
    if (which == Qt::MinimumSize) {
        return d->mMinValue;
    } else if (which == Qt::PreferredSize ) {
        return d->mPrefValue;
    } else {
        return d->mMaxValue;
    }

}

/*!
    Returns current start item, or zero if it is not set.

    \return reference to start item
*/
QGraphicsLayoutItem *HbAnchor::startItem() const
{
    Q_D( const HbAnchor );
    return d->mStartItem;
}

/*!
    Returns current start node id, or null-string if it is not set.

    \return start node id
*/
QString HbAnchor::startNodeId() const
{
    Q_D( const HbAnchor );
    return d->mStartId;
}

/*!
    Returns start edge of this anchor.

    \return start edge
*/
Hb::Edge HbAnchor::startEdge() const
{
    Q_D( const HbAnchor );
    return d->mStartEdge;
}

/*!
    Returns current end item, or zero if it is not set.

    \return reference to end item
*/
QGraphicsLayoutItem *HbAnchor::endItem() const
{
    Q_D( const HbAnchor );
    return d->mEndItem;
}

/*!
    Returns current end node id, or null-string if it is not set.

    \return start end id
*/
QString HbAnchor::endNodeId() const
{
    Q_D( const HbAnchor );
    return d->mEndId;
}

/*!
    Returns end edge of this anchor.

    \return end edge
*/
Hb::Edge HbAnchor::endEdge() const
{
    Q_D( const HbAnchor );
    return d->mEndEdge;
}

/*!
    Returns current direction of this anchor.

    \return direction
*/
HbAnchor::Direction HbAnchor::direction() const
{
    Q_D( const HbAnchor );
    return d->mDir;
}

/*!
    Sets direction to this anchor.

    \param dir direction to set
*/
void HbAnchor::setDirection( HbAnchor::Direction dir )
{
    Q_D( HbAnchor );

    if ( d->mDir != dir ) {
        d->mDir = dir;
        if( d->mParent ) {
            d->mParent->updateGeometry();
        }
    }
}

/*!
    Returns anchor id of this anchor, or null-string if it is not set. 

    \return anchor id
*/
QString HbAnchor::anchorId() const
{
    Q_D( const HbAnchor );
    return d->mAnchorId;
}

/*!
    Sets anchor id for this anchor.

    \param anchorId id to be set
*/
void HbAnchor::setAnchorId( const QString &anchorId )
{
    Q_D( HbAnchor );
    d->mAnchorId = anchorId;
}


