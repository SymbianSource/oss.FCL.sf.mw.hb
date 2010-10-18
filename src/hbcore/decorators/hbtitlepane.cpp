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

#include "hbtitlepane_p.h"
#include "hbtitlepane_p_p.h"
#include "hbevent.h"
#include "hbstyle_p.h"

#include <hbstyleoptiontitlepane_p.h>
#include <hbapplication.h>
#include <hbwidgetfeedback.h>
#include <hbmainwindow.h>
#include <hbview.h>
#include <hbmenu.h>
#include <hbtapgesture.h>
#include <hbswipegesture.h>
#include <hbmarqueeitem.h>

#include <QGestureEvent>
#include <QGesture>

#include <QGraphicsSceneMouseEvent>

/*
    \class HbTitlePane
    \brief HbTitlePane represents a title pane decorator.

    The HbTitlePane class represents title pane on each application. It provides an interface for
    setting text, font alignment and options menu.
*/

HbTitlePanePrivate::HbTitlePanePrivate() :
    mText(), mTextItem(0), mIcon(0), mMode(QIcon::Normal), mTouchArea(0), mMargueeAnimation(false),
    mTapStarted(false), mSwipeStarted(false)
{

}

void HbTitlePanePrivate::delayedConstruction()
{
}

void HbTitlePanePrivate::init()
{
    Q_Q(HbTitlePane);

    createPrimitives();
    q->setAcceptedMouseButtons(Qt::LeftButton);
    q->setText(HbApplication::applicationName());
}

void HbTitlePanePrivate::createPrimitives()
{
    Q_Q(HbTitlePane);

    mTextItem = HbStylePrivate::createPrimitive(HbStylePrivate::P_TitlePane_text, q);
    mIcon = HbStylePrivate::createPrimitive(HbStylePrivate::P_TitlePane_icon, q);
    mTouchArea = HbStylePrivate::createPrimitive(HbStylePrivate::P_TitlePane_toucharea, q);
    q->ungrabGesture(Qt::TapGesture);
    q->ungrabGesture(Qt::SwipeGesture);

    setBackgroundItem(HbStylePrivate::P_TitlePane_background);
}

void HbTitlePanePrivate::updatePrimitives()
{
    Q_Q(HbTitlePane);
    HbStyleOptionTitlePane option;
    q->initStyleOption(&option);
    HbStylePrivate::updatePrimitive(q->backgroundItem(), HbStylePrivate::P_TitlePane_background, &option);
    HbStylePrivate::updatePrimitive(mTextItem, HbStylePrivate::P_TitlePane_text, &option);
    HbStylePrivate::updatePrimitive(mIcon, HbStylePrivate::P_TitlePane_icon, &option);
    HbStylePrivate::updatePrimitive(mTouchArea, HbStylePrivate::P_TitlePane_toucharea, &option);
}

void HbTitlePanePrivate::handleTap(HbTapGesture *tap)
{
    Q_Q(HbTitlePane);
    switch (tap->state()) {
    case Qt::GestureStarted:
        if (q->scene()) {
            q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(), Qt::TapGesture);
            tap->setProperty(HbPrivate::ThresholdRect.latin1(), q->mapRectToScene(q->boundingRect()).toRect());
        }
        mMode = QIcon::Active;
        updatePrimitives();
#ifdef HB_EFFECTS
        HbEffect::start(q, "decorator", "pressed");
#endif
        HbWidgetFeedback::triggered(q, Hb::InstantPressed);
        mTapStarted = true;
        break;

    case Qt::GestureCanceled: 
        cancelTap();
        break;

    case Qt::GestureFinished: {
        if (q->scene()) {
            q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(), QVariant());
        }
        mMode = QIcon::Selected;
        updatePrimitives();
#ifdef HB_EFFECTS
        HbEffect::start(q, "decorator", "latched");
#endif
        HbWidgetFeedback::triggered(q, Hb::InstantReleased);
        QPointF launchPos(q->scenePos().x() + q->boundingRect().width() / 2 + 3, 
                          q->scenePos().y() + q->boundingRect().height());
        emit q->launchPopup(launchPos);
        mTapStarted = false;
        break;
    }
    default:
        mTapStarted = false;
        break;
    }
}

void HbTitlePanePrivate::handleSwipe(HbSwipeGesture *swipe)
{
    Q_Q(HbTitlePane);
    HbWidgetFeedback::triggered(q, Hb::InstantFlicked);

    switch (swipe->state()) {
    case Qt::GestureStarted:
        mSwipeStarted = true;
        break;

    case (Qt::GestureFinished):
        if (swipe->sceneHorizontalDirection() == QSwipeGesture::Right) {
            emit q->swipeRight();
        } else if (swipe->sceneHorizontalDirection() == QSwipeGesture::Left) {
            emit q->swipeLeft();
        }

        if (mMode != QIcon::Normal) {
            mMode = QIcon::Normal;
            updatePrimitives();
        }
        mSwipeStarted = false;
        break;

    default:
        mSwipeStarted = false;
        break;
    }
}

void HbTitlePanePrivate::cancelTap()
{
    Q_Q(HbTitlePane);

    if (q->scene()) {
        q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());
    }
    HbWidgetFeedback::triggered(q, Hb::InstantReleased);

    if (mMode != QIcon::Normal) {
        mMode = QIcon::Normal;
        updatePrimitives();
    }
    mTapStarted = false;
}

// ======== MEMBER FUNCTIONS ========

/*
    Constructs a vertical title pane with \a parent.
 */

HbTitlePane::HbTitlePane(QGraphicsItem *parent)
    : HbWidget(*new HbTitlePanePrivate, parent)
{
    Q_D(HbTitlePane);
    d->init();
    setFlag(QGraphicsItem::ItemHasNoContents, true);
}

HbTitlePane::HbTitlePane(HbTitlePanePrivate &dd, QGraphicsItem *parent)
    : HbWidget(dd, parent)
{
    Q_D(HbTitlePane);
    d->init();
    setFlag(QGraphicsItem::ItemHasNoContents, true);
}

/*
    Destructor
 */
HbTitlePane::~HbTitlePane()
{

}

/*
    Delayed constructor.
 */
void HbTitlePane::delayedConstruction()
{
       Q_D(HbTitlePane);
       d->delayedConstruction();
}

/*
    \property HbTitlePane::text
    \brief This property holds the text
 */

/*
    \brief  Returns the text used in the title pane.
    \return The title pane text.
 */
QString HbTitlePane::text() const
{
    Q_D(const HbTitlePane);
    return d->mText;
}

/*
    Resets icon mode
*/
void HbTitlePane::resetMode()
{
    Q_D(HbTitlePane);
    d->mMode = QIcon::Normal;
#ifdef HB_EFFECTS
    HbEffect::start(this, "decorator", "released");
#endif
}

/*
    \brief  This method sets the text of the title pane.
    \param  text The text to be used in the title pane.
 */
void HbTitlePane::setText(const QString &text)
{
    Q_D(HbTitlePane);

    QString tmp = text;
    if (tmp.isNull()) {
        tmp = HbApplication::applicationName();
    }

    if (tmp != d->mText) {
        d->mText = tmp;
        d->mMargueeAnimation = true;
        updatePrimitives();
        d->mMargueeAnimation = false;
    }
}

/*
    \reimp
 */
void HbTitlePane::updatePrimitives()
{
    Q_D(HbTitlePane);

    d->updatePrimitives();
}

/*
    \reimp
 */
void HbTitlePane::initStyleOption(HbStyleOptionTitlePane *option) const
{
    Q_D(const HbTitlePane);

    option->caption = d->mText;
    option->mode = d->mMode;
    option->margueeAnimation = d->mMargueeAnimation;
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            option->transparent = true;
        }
    }
}

void HbTitlePane::gestureEvent(QGestureEvent *event)
{
    Q_D(HbTitlePane);
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->menu()->isEmpty()) {
            return;
        }
    }

    HbTapGesture *tap = qobject_cast<HbTapGesture*>(event->gesture(Qt::TapGesture));
    HbSwipeGesture *swipe = qobject_cast<HbSwipeGesture*>(event->gesture(Qt::SwipeGesture));

    if (tap) {
        if (!(d->mSwipeStarted && tap->state() == Qt::GestureFinished)) {
            d->handleTap(tap);
        }
    }
    if (swipe) {
        if (swipe->state() == Qt::GestureStarted && d->mTapStarted) {
             d->cancelTap();
        }
        d->handleSwipe(swipe);
    }
}

/*
    \reimp
 */
void HbTitlePane::polish(HbStyleParameters &params)
{
    Q_D(HbTitlePane);
    HbWidget::polish(params);
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->menu()->isEmpty()) {
            d->mIcon->setVisible(false);
        } else {
            d->mIcon->setVisible(true);
        }
    }
}

/*
    \reimp
 */
bool HbTitlePane::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
    Q_D(HbTitlePane);
    if (event->type() == HbEvent::WindowLayoutDirectionChanged) {
       repolish();
    } else if (event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionRemoved) {
        if (mainWindow() && mainWindow()->currentView()) {
            if (mainWindow()->currentView()->menu()->isEmpty()) {
                d->mIcon->setVisible(false);
            } else {
                d->mIcon->setVisible(true);
            }
        }
    }

    return false;
}

/*
    \reimp
*/
QVariant HbTitlePane::itemChange(GraphicsItemChange change, const QVariant & value)
{
    switch (change) {
        case ItemVisibleHasChanged: 
            {
            Q_D(const HbTitlePane);
            if (d->polished)  {
                // Cannot emit signals directly from "visibilityChanged", since it might
                // lead to e.g. item deletion that's not allowed in "itemChange".
                // Using QMetaObject to emit the signal asynchronously.
                QMetaObject::invokeMethod(this, "visibilityChanged", Qt::QueuedConnection);
            }
            break;
            }      
        default:
            break;
    }
    return HbWidget::itemChange(change, value);
}

/*!
    \reimp
*/
QGraphicsItem *HbTitlePane::primitive(const QString &itemName) const
{
    Q_D(const HbTitlePane);
    if (itemName.isEmpty()) {
        return 0;
    } else {
        if (itemName == "background") {
            return this->backgroundItem();
        } else if (itemName == "text") {
            return d->mTextItem;
        } else if (itemName == "icon") {
            return d->mIcon;
        } else if (itemName == "toucharea") {
            return d->mTouchArea;
        } else {
            return 0;
        }
    }
}
