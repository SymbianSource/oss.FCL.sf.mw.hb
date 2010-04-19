/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbFeedback module of the UI Extensions for Mobile.
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

#include "hbhitareafeedback.h"

#include <QGraphicsItem>
#include <QGraphicsView>


class HbHitAreaFeedbackPrivate
{
public:
    HbHitAreaFeedbackPrivate() : cHitAreaType(HbFeedback::MouseButtonPress), cZValue(0) {};
    ~HbHitAreaFeedbackPrivate() {};

public:
    HbFeedback::HitAreaType cHitAreaType;
    qreal cZValue;
};

/*!

    \deprecated HbHitAreaFeedback
        is deprecated. Please use HbInstantFeedback instead.

    @hbfeedback

    \class HbHitAreaFeedback

    \brief Tool class for hit area-based feedback effects. [DEPRECATED]

    Hit area feedback is a special kind of instant feedback. Hit are feedbacks are rectangle areas registered for
    the application windows  that react to touch events before the events are reach the actual application GUI.
    Although hit areas are in many ways more cumbersome than traditional instant feedback effects initiated from
    the application code, they provide lower latency in situations where round trip to application event loop is
    considered too slow. New hit areas can be registered, old hit areas can be moved or removed from the screen
    using methods HbFeedbackPlayer::insertHitArea(), HbFeedbackPlayer::updateHitArea() and HbFeedbackPlayer::removeHitArea().

    \sa HbInstantFeedback
*/

/*!
    \fn HbFeedback::Type HbHitAreaFeedback::type() const

    Returns HbFeedback::TypeHitArea.

    \deprecated HbHitAreaFeedback::type() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

/*!
    \fn void HbHitAreaFeedback::setHitAreaType(HbFeedback::HitAreaType hitAreaType)

    Sets the hit area type for the feedback. Hit area type defines whether feedback effect
    is initiated when user touches down the screen or when user lifts the finger
    from the screen.

    \deprecated HbHitAreaFeedback::setHitAreaType(HbFeedback::HitAreaType)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

void HbHitAreaFeedback::setHitAreaType(HbFeedback::HitAreaType hitAreaType) {
    
    d->cHitAreaType = hitAreaType;
}

/*!
    \fn HbFeedback::HitAreaType HbHitAreaFeedback::hitAreaType() const

    The hit area type of the feedback. Hit area type defines whether feedback effect
    is initiated when user touches down the screen or when user lifts the finger
    from the screen.

    \deprecated HbHitAreaFeedback::hitAreaType() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

HbFeedback::HitAreaType HbHitAreaFeedback::hitAreaType() const {

    return d->cHitAreaType;
};

/*!
    \fn bool HbHitAreaFeedback::isValid() const

    Hit are feedback is valid if a proper instant effect (not HbFeedback::None), parent window
    and area rectangle (relative to the parent window) has been defined for
    the hit area feedback.

    \deprecated HbHitAreaFeedback::isValid() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

bool HbHitAreaFeedback::isValid() const {
    return isLocated() && instantEffect() != HbFeedback::None;
}

/*!
    \fn void HbHitAreaFeedback::setZValue(qreal zValue)

    Sets the z-position for the hit area. If there are multiple overlapping hit areas the feedback effect
    with highest z-position will be initiated.

    \deprecated HbHitAreaFeedback::setZValue(float)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

void HbHitAreaFeedback::setZValue(qreal zValue)
{
    d->cZValue = zValue;
}

/*!
    \fn qreal HbHitAreaFeedback::zValue() const

    The z-position for the hit area. If there are multiple overlapping hit areas the feedback effect
    with highest z-position will be initiated.

    \deprecated HbHitAreaFeedback::zValue() const
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/

qreal HbHitAreaFeedback::zValue() const
{
    return d->cZValue;
}

/*!
    Constructor.

    \deprecated HbHitAreaFeedback::HbHitAreaFeedback()
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbHitAreaFeedback::HbHitAreaFeedback() :
        d(new HbHitAreaFeedbackPrivate)
{
}

/*!
    Constructor.
    \param effect instant feedback
    \param widget used to determine the window where hit area is registered.
    \param rect rectangle in relation to the window, if null uses the widget's bounding rectangle.

    \deprecated HbHitAreaFeedback::HbHitAreaFeedback(HbFeedback::InstantEffect, const QWidget*, QRect)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbHitAreaFeedback::HbHitAreaFeedback(HbFeedback::InstantEffect effect, const QWidget* widget, QRect rect) :
        HbInstantFeedback(effect), d(new HbHitAreaFeedbackPrivate)
{
    setOwningWindow(widget);
    if (!rect.isNull()) {
        setRect(rect);
    } else {
        setRect(widget);
    }
}

/*!
    Constructor.
    \param effect instant feedback effect to be played
    \param graphicsItem graphics item used to determine the hit area rectangle, won't be automatically updated if item's position changes
    \param graphicsView sed to determine the window where hit area is registered.

    \deprecated HbHitAreaFeedback::HbHitAreaFeedback(HbFeedback::InstantEffect, const QGraphicsView*, const QGraphicsItem*)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbHitAreaFeedback::HbHitAreaFeedback(HbFeedback::InstantEffect effect, const QGraphicsView* graphicsView, const QGraphicsItem* graphicsItem) :
        HbInstantFeedback(effect), d(new HbHitAreaFeedbackPrivate)
{
    setOwningWindow(graphicsView);
    setRect(graphicsItem, graphicsView);
}

/*!
    Destructor.

    \deprecated HbHitAreaFeedback::~HbHitAreaFeedback()
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbHitAreaFeedback::~HbHitAreaFeedback()
{
    delete d;
}

/*!
    Assigns a copy of the feedback \a feedback to this feedback, and returns a
    reference to it.

    \deprecated HbHitAreaFeedback::operator =(const HbHitAreaFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
HbHitAreaFeedback &HbHitAreaFeedback::operator=(const HbHitAreaFeedback & feedback)
{
    HbInstantFeedback::operator =(feedback);
    setHitAreaType(feedback.hitAreaType());
    setZValue(feedback.zValue());
    return *this;
}

/*!
    Returns true if this feedback has the same configuration as the feedback \a
    feedback; otherwise returns false.

    \deprecated HbHitAreaFeedback::operator ==(const HbHitAreaFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
bool HbHitAreaFeedback::operator==(const HbHitAreaFeedback &feedback) const
{
    return (rect() == feedback.rect()
            && window() == feedback.window()
            && instantEffect() == feedback.instantEffect()
            && d->cHitAreaType == feedback.hitAreaType()
            && d->cZValue == feedback.zValue());
}

/*!
    Returns true if this feedback has different configuration than the feedback \a
    feedback; otherwise returns false.

    \deprecated HbHitAreaFeedback::operator !=(const HbHitAreaFeedback&)
        is deprecated. Please use HbInstantFeedback instead.

    \sa HbInstantFeedback
*/
bool HbHitAreaFeedback::operator!=(const HbHitAreaFeedback &feedback) const
{
    return !(*this == feedback);
}
