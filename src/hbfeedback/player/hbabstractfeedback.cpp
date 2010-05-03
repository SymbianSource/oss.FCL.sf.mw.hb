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

#include "hbabstractfeedback.h"

#include <QGraphicsItem>
#include <QGraphicsView>

class HbAbstractFeedbackPrivate
{
public:
    HbAbstractFeedbackPrivate() : cWindow(0),cModalities(HbFeedback::All) {};
    ~HbAbstractFeedbackPrivate() {};
    QRect mapWidgetToWindow(const QGraphicsItem* graphicsItem, const QGraphicsView* graphicsView);
    QRect mapWidgetToWindow(const QWidget* widget);

public:
    QPointer<QWidget> cWindow;
    QRect cRect;
    HbFeedback::Modalities cModalities;
};

/*!
\internal
@{
    Maps the widget rectangle to the parent window
    co-ordinate system.
@}
*/
QRect HbAbstractFeedbackPrivate::mapWidgetToWindow(const QWidget* widget)
{
    // null rectangle
    QRect mappedRect = QRect();

    if (widget && widget->window()) {
        QRect widgetRect = widget->rect();
        // from widget to parent window coordinates
        QPoint topLeft = widget->mapTo(widget->window(),
                                       widgetRect.topLeft());
        QPoint bottomRight = widget->mapTo(widget->window(),
                                           widgetRect.bottomRight());
        mappedRect = QRect(topLeft, bottomRight);
    }

    return mappedRect;
}

/*!
\internal
@{
    Maps the graphics widget bounding rectangle trough a
    given graphics view view port to the parent window
    co-ordinate system.
 @}
*/
QRect HbAbstractFeedbackPrivate::mapWidgetToWindow(const QGraphicsItem* graphicsWidget,
        const QGraphicsView* graphicsView)
{
    // null rectangle
    QRect mappedRect = QRect();

    if (graphicsWidget && graphicsView && graphicsView->window()) {
        // belong to same scene
        if (graphicsView->scene()
                && graphicsView->scene() == graphicsWidget->scene()) {
            // from scene to graphics view coordinates
            mappedRect = graphicsView->mapFromScene(
                             graphicsWidget->mapToScene(
                                 graphicsWidget->boundingRect()
                             ).boundingRect()
                         ).boundingRect();

            // from graphics view to window coordinates
            QPoint topLeft = graphicsView->mapTo(
                                 graphicsView->window(), mappedRect.topLeft());
            QPoint bottomRight = graphicsView->mapTo(
                                     graphicsView->window(), mappedRect.bottomRight());
            mappedRect = QRect(topLeft, bottomRight);
        }
    }

    return mappedRect;
}

/*!
    @beta
    @hbfeedback
    \class HbAbstractFeedback

    \brief Abstract base class for different haptic feedback effects.

    Concrete feedback objects have to define their type HbFeedback::Type by overriding the method type().

    Provides common methods for assigning the parent window and the feedback area for the feedback effect.

    \sa HbInstantFeedback, HbContinuousFeedback, HbTacticonFeedback
*/

/*!
    \fn HbFeedback::Type HbAbstractFeedback::type() const

    Returns the haptic type of the feedback object. Every derived class has to define their a type.
*/

/*!
    \fn QWidget* HbAbstractFeedback::window() const

    Returns the owning widget, null if not defined.
*/

QWidget* HbAbstractFeedback::window() const
{
    return d->cWindow;
}

/*!
    \fn HbFeedback::Modalities HbAbstractFeedback::modalities() const

    Returns the modalities (eg. tactile, audio) used to produce the feedback.
*/

HbFeedback::Modalities HbAbstractFeedback::modalities() const
{
    return d->cModalities;
}

/*!
    \fn bool HbAbstractFeedback::isLocated() const

    Returns true if both the parent window and the feedback area rectange (in relation to the parent window) has been defined. Not all feedback objects have to be located.

*/

bool HbAbstractFeedback::isLocated() const
{
    return d->cWindow && d->cRect.isValid();
}

/*!
    \fn bool HbAbstractFeedback::isValid() const

    Whether or not all the necessary parameters have been defined for the feedback object. What constitutes as valid differs between different types of feedback.
*/

/*!
    \fn QRect HbAbstractFeedback::rect() const

    Returns the feedback area rect in relation of it's parent widget.
*/

QRect HbAbstractFeedback::rect() const
{
    return d->cRect;
}

/*!
    \fn void HbAbstractFeedback::setRect(QRect rect)

    Sets the feedback area rect in relation of it's parent widget.
*/

void HbAbstractFeedback::setRect(QRect rect)
{
    d->cRect = rect;
}

/*!
    Constructor.
*/
HbAbstractFeedback::HbAbstractFeedback() : d(new HbAbstractFeedbackPrivate)
{
}

/*!
    Destructor.
*/
HbAbstractFeedback::~HbAbstractFeedback()
{
    delete d;
}

/*!
    Inherits feedback area rect from the given widget. Position is calculated relative to the parent window.

*/
void HbAbstractFeedback::setRect(const QWidget* widget)
{
    if (widget) {
        d->cRect = d->mapWidgetToWindow(widget);
    }
}

/*!
    Inherits feedback area rect from the graphics item in relation to the parent graphics view. Position is calculated relative to the window.
*/
void HbAbstractFeedback::setRect(const QGraphicsItem* graphicsItem, const QGraphicsView* graphicsView)
{
    if (graphicsItem && graphicsView && graphicsView->window()) {
        d->cRect = d->mapWidgetToWindow(graphicsItem, graphicsView);
    }
}

/*!
    Sets window that initiates the feedback. If given QWidget is not a window,
    uses the parent window instead.
*/
void HbAbstractFeedback::setOwningWindow(const QWidget* widget)
{
    d->cWindow = 0;
    if (widget && widget->window()) {
        d->cWindow = widget->window();
    }
}

/*!
    Sets the modalities (eg. tactile, audio) used to produce the feedback.
*/
void HbAbstractFeedback::setModalities(HbFeedback::Modalities modalities)
{
    d->cModalities = modalities;
}

/*!
    Assigns a copy of the feedback \a feedback to this feedback, and returns a
    reference to it.
*/
HbAbstractFeedback &HbAbstractFeedback::operator=(const HbAbstractFeedback & feedback)
{
    setRect(feedback.rect());
    setOwningWindow(feedback.window());
    return *this;
}

/*!
    Returns true if this feedback has the same configuration as the feedback \a
    feedback; otherwise returns false.
*/
bool HbAbstractFeedback::operator==(const HbAbstractFeedback &feedback) const
{
    return (d->cRect == feedback.rect()
            && d->cWindow == feedback.window()
            && type() == feedback.type());
}

/*!
    Returns true if this feedback has different configuration than the feedback \a
    feedback; otherwise returns false.
*/
bool HbAbstractFeedback::operator!=(const HbAbstractFeedback &feedback) const
{
    return !(*this == feedback);
}
