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
#include "hbgraphicsscene.h"
#include "hbgraphicsscene_p.h"
#include "hbwidget.h"
#include "hbwidget_p.h"
#include "hbfocusgroup_p.h"
#include "hbpopupmanager_p.h"
#include "hbinputmethod.h"
#include "hbinputfocusobject.h"
#include "hbpopup.h"
#include "hbtooltiplabel_p.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QInputContext>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QTime>
#include <QDebug>

bool HbGraphicsScenePrivate::fpsCounterEnabled = false; // Fps counter off by default.

/*! 
    Define HB_RND_DRAW_ITEM_BORDERS to draw the graphics items' borders
    CAUTION: clipping and transformations may fail
 */
#undef HB_RND_DRAW_ITEM_BORDERS

/*!
    @beta
    @hbcore
    \class HbGraphicsScene
    \brief HbGraphicsScene implements input focus handling for graphics scene items.

    Normally Qt's QInputContext focusing mechanism works only with QWidget based objects and
    in case of the graphics view, the input context focus is attached to the graphics view widget and
    not to objects inside the scene.
    HbGraphicsScene class addresses this problem by monitoring focus activity inside the scene and
    in case an input capable item becomes focused, the input context focus is attached directly to it.

    Layout is set to hbwidget in polishEvent, this results in posting layoutrequest events which are not
    handled immediately but after first draw(This is the case when widget is added to scene first time and
    QGraphicsscene polishes it before drawing).In this case,HbGraphicsScene makes sure that polishEvents and
    any resulting layoutrequest events are handled(if any pending in event loop) before drawing.

    It does not add or change normal QGraphicsScene behaviour in any other way.
*/

HbGraphicsScenePrivate::HbGraphicsScenePrivate()
    : mPopupManager(0),
      mToolTip(0),
      mInputFocusSet(0),
      mPolishWidgets(false),
      mDrawCount(0),
      mFPS(0),
      mFPSTime(0),
      mMaxFPS(0)
{
}

HbGraphicsScenePrivate::~HbGraphicsScenePrivate()
{
}

/*!
\internal
Clear the input framework focus
*/
void HbGraphicsScenePrivate::clearInputFocus()
{
   QInputContext* inputContext = qApp->inputContext();
   if (inputContext) {
       inputContext->setFocusWidget(0);
   }
   mInputFocusSet = false;
}

//# Focus delegation between focus groups and widgets. Moves focus to next or
// previous widget within the focus group the widget belongs to or up the focus
// chain.
bool HbGraphicsScenePrivate::focusChangeEvent(const QKeyEvent *event)
{
    Q_Q(HbGraphicsScene);
    HbFocusGroup *focusGroup(0);
    QGraphicsItem *item(q->focusItem());
    bool cont(true);
    bool result(false);
    HbWidget *widget = static_cast<HbWidget *>(item);

    while (cont) {
        result = cont = false;
        while (widget && !focusGroup) {
           focusGroup = widget->d_func()->focusGroup;

            if (!focusGroup) {
                widget = static_cast<HbWidget *>(widget->parentItem());
            }
      }

        if (!focusGroup || !focusGroup->focusParameters().testFlag(HbFocusGroup::AlwaysOn)) {
            break;
        }

        bool focusNextKey = focusGroup->isFocusNextPrevKey(*event,true);
        if (focusNextKey || focusGroup->isFocusNextPrevKey(*event,false)) {
            result = focusGroup->focusNextPrevItem(widget->lastFocusedChild(), focusNextKey);

            if (result) {
                focusGroup->updateCurrentFocusChild(widget);
            }
        } else {
            if (focusGroup->isFocusModeChangeKey(*event)) {
                focusGroup->changeFocusMode();
                result = true;
            }
        }

        if (!result) {
            cont = true;
            focusGroup = 0;
            widget = static_cast<HbWidget *>(widget->parentItem());
        }
    }
    return result;
}

void HbGraphicsScenePrivate::showPopup(HbPopup *popup)
{
    popupManager()->showPopup(popup);
}

void HbGraphicsScenePrivate::hidePopup(HbPopup *popup)
{
    popupManager()->hidePopup(popup);
}

HbPopupManager* HbGraphicsScenePrivate::popupManager()
{
    Q_Q(HbGraphicsScene);
    if (!mPopupManager) {
        mPopupManager = new HbPopupManager(q,q);
    }
    return mPopupManager;
}

/*!
    Constructor
*/
HbGraphicsScene::HbGraphicsScene(QObject* parent) : QGraphicsScene(parent),
d_ptr(new HbGraphicsScenePrivate)
{
    Q_D(HbGraphicsScene);
    d->q_ptr = this;

#ifdef HB_RND_DRAW_ITEM_BORDERS // see the top of the file
    if ( QGraphicsView *view = qobject_cast<QGraphicsView*>( parent ) ) {  
        // set the IndirectPainting flag -> drawItems() is called on paint.
        view->setOptimizationFlag( QGraphicsView::IndirectPainting, true );
    } 
#endif
    setStickyFocus(true);
}

/*!
    Destructor
*/
HbGraphicsScene::~HbGraphicsScene()
{
    setProperty("destructed", true);
    delete d_ptr;
}

/*!
  \reimp
  \sa QGraphicsScene
 */
void HbGraphicsScene::focusInEvent(QFocusEvent* focusEvent)
{
    QGraphicsScene::focusInEvent(focusEvent);
}

/*!
  \reimp
  \sa QGraphicsScene
 */
void HbGraphicsScene::focusOutEvent(QFocusEvent* focusEvent)
{
    QGraphicsScene::focusOutEvent(focusEvent);
}

/*!
  \reimp
  \sa QGraphicsScene
 */
void HbGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);

    // If the focused item is not HbWidget (and not an editor) then we send an event to close the input panel
    QGraphicsItem* focusedItem = focusItem();
    if(focusedItem) {
        QGraphicsObject* focusedObject = focusedItem->toGraphicsObject();
        if(focusedObject && !focusedObject->inherits("HbWidget") &&
           ((focusedObject->flags() & QGraphicsItem::ItemAcceptsInputMethod) == 0)) {
            QInputContext *ic = qApp->inputContext();
            if (ic) {
                QEvent *closeEvent = new QEvent(QEvent::CloseSoftwareInputPanel);             
                ic->filterEvent(closeEvent);
                delete closeEvent;
            }
        }
    }
}

/*!
  \reimp
  \sa QGraphicsScene
 */
void HbGraphicsScene::drawItems(QPainter *painter, int numItems,
                                QGraphicsItem *items[],
                                const QStyleOptionGraphicsItem options[],
                                QWidget *widget)
{
#ifdef HB_RND_DRAW_ITEM_BORDERS // see the top of the file
    for (int i = 0; i < numItems; ++i) {
        // Draw the item
        painter->save();
        painter->setMatrix(items[i]->sceneMatrix(), true);
        items[i]->paint(painter, &options[i], widget);

        const QColor randColor( qrand()%255, qrand()%255, qrand()%255 );
        painter->setPen( randColor );

        painter->drawRect( items[i]->boundingRect() );
        painter->restore();
    }
#else
    QGraphicsScene::drawItems( painter, numItems, items, options, widget );
#endif

// Switch to (1) to draw the borders of the scene
#if(0)
    // Draw the scene rect
    painter->save();
    const QColor randColor( qrand()%255, qrand()%255, qrand()%255 );
    painter->setPen( randColor );
    painter->drawRect( sceneRect() );
    painter->restore();
#endif
}

void HbGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_D(HbGraphicsScene);
    if (d->mPolishWidgets) {
        // mPolishWidgets is set to true in HbWidget when a widget's scene has
        // changed. We invoke polish slot of qgraphicsscene here so that layout
        // requests resulting from polish can be handled before drawing the
        // widget. This is done more than once because most widgets try to create their
        // primitives in polish event. It will also make sure that we do not
        // have any pending layout requests in event loop.
        for (int i = 0; i < 3; ++i) {
            QMetaObject::invokeMethod(this, "_q_polishItems");
            QApplication::sendPostedEvents(0, QEvent::LayoutRequest);
        }
        d->mPolishWidgets = false;
    }
    QGraphicsScene::drawBackground(painter,rect);
}

void HbGraphicsScene::drawForeground ( QPainter * painter, const QRectF & /*rect*/ )
{
    Q_D(HbGraphicsScene);    
    if (!d->mToolTip) {
        d->mToolTip = new HbToolTipLabel();
        addItem(d->mToolTip);
    }
    if(d->fpsCounterEnabled) {
        if(!d->mFPSTime) {
            d->mFPSTime = new QTime();
            d->mFPSTime->start();
        }
        d->mDrawCount++;
        if(d->mFPSTime->elapsed() > 500) {
            int elapsed = d->mFPSTime->restart();
            qreal e = elapsed;
            if(d->mDrawCount > 2) { // just to minize problems with idle time
                d->mFPS = d->mFPS * 0.5 + 0.5 * (qreal(d->mDrawCount * 1000) / e); 
            }
            d->mDrawCount = 0;
            QTimer::singleShot(10, this, SLOT(update()));
        }
        if (d->mFPS > d->mMaxFPS)
            d->mMaxFPS = d->mFPS; 
        painter->save();
        painter->setFont(QFont("Arial", 12, QFont::Bold ));
        painter->setPen(QColor(255,0,0));
        painter->drawText(0,32,QString("FPS: %0 (%1)").arg(int(d->mFPS)).arg(int(d->mMaxFPS)));
        painter->restore();
    }
}

/*!
  \reimp
 */
void HbGraphicsScene::helpEvent ( QGraphicsSceneHelpEvent * helpEvent )
{
    Q_UNUSED(helpEvent);
    // Reimplemented to prevent desktop tool tips to be shown.
}

/*!
  \reimp
 */
bool HbGraphicsScene::event ( QEvent * event )
{
    Q_D(HbGraphicsScene);

    if (d->mToolTip) {
        d->mToolTip->eventHook(event);
    }
    if (d->mPopupManager) {
        d->mPopupManager->eventHook(event);
    }
    bool result = QGraphicsScene::event(event);

    if (event->type() == QEvent::KeyPress && !event->isAccepted()) {
        QKeyEvent *e = static_cast<QKeyEvent *>(event);
        result = d->focusChangeEvent(e);
        event->accept();
        }

    return result;
}

// End of file
