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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "hbselectioncontrol_p.h"
#include "hbstyleoption.h"
#include "hbeffect.h"
#include "hbdialog_p.h"
#include "hbabstractedit.h"
#include "hbabstractedit_p.h"
#include "hbtoucharea.h"


#include <QTextCursor>
#include <QGraphicsItem>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneMouseEvent>
#include <QBasicTimer>
#include <QSizeF>
#include <QPointF>

#include <hbwidgetfeedback.h>

namespace {
    static const int SNAP_DELAY = 300;
}


class HbSelectionControlPrivate :public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbSelectionControl)

public:
    HbSelectionControlPrivate(HbAbstractEdit *edit);
    void init();
    void createPrimitives();
    void updateHandle(int newHandlePos,
                      Qt::AlignmentFlag handleAlignment,
                      QGraphicsItem *handle,
                      QGraphicsItem *handleTouchArea,
                      HbStyle::Primitive handlePrimitive);
    QGraphicsItem * reparent(QGraphicsItem *item);
    void reparent(QGraphicsItem *item, QGraphicsItem *newParent);
    void reparentHandles(QGraphicsItem *newParent);

public:

    HbAbstractEdit *mEdit;
    QPointF mMouseOffset;

    QGraphicsItem *mSelectionStartHandle;
    QGraphicsItem *mSelectionEndHandle;
    HbTouchArea* mSelectionStartTouchArea;
    HbTouchArea* mSelectionEndTouchArea;

    HbSelectionControl::HandleType mPressed;
    bool mHandlesDisabled;
    bool mPanInProgress;
    bool mHandlesMoved;
    QBasicTimer mWordSnapTimer;
};


HbSelectionControlPrivate::HbSelectionControlPrivate(HbAbstractEdit *edit):
    mEdit(edit),
    mSelectionStartHandle(0),
    mSelectionEndHandle(0),
    mSelectionStartTouchArea(0),
    mSelectionEndTouchArea(0),
    mPressed(HbSelectionControl::HandleType(0)),
    mHandlesDisabled(true),
    mPanInProgress(false),
    mHandlesMoved(false)
{
}

void HbSelectionControlPrivate::init()
{
    Q_Q(HbSelectionControl);
    createPrimitives();

    q->setBackgroundItem(HbStyle::P_None);
    q->setFocusPolicy(Qt::NoFocus);
    q->setTimeout(HbPopup::NoTimeout);
    q->setBackgroundFaded(false);
    q->setVisible(false);
    q->setDismissPolicy(HbPopup::NoDismiss);
    q->setModal(false);

    #ifdef HB_EFFECTS
    HbEffect::disable(q);
    #endif    

    q->setParent(mEdit);

    // Control will handle all events going to different handlers.
    q->setHandlesChildEvents(true);

    QObject::connect(mEdit, SIGNAL(cursorPositionChanged(int, int)), q, SLOT(updatePrimitives()));
    QObject::connect(mEdit, SIGNAL(selectionChanged(const QTextCursor&, const QTextCursor&)), q, SLOT(updatePrimitives()));
    QObject::connect(mEdit, SIGNAL(contentsChanged()), q, SLOT(updatePrimitives()));

    q->updatePrimitives();

}

void HbSelectionControlPrivate::createPrimitives()
{
    Q_Q(HbSelectionControl);
    if (!mSelectionStartHandle) {
        mSelectionStartHandle = mEdit->style()->createPrimitive(HbStyle::P_SelectionControl_selectionstart, q);
        mSelectionStartHandle->hide();
    }

    if (!mSelectionEndHandle) {
        mSelectionEndHandle = mEdit->style()->createPrimitive(HbStyle::P_SelectionControl_selectionend, q);
        mSelectionEndHandle->hide();
    }

    if (!mSelectionStartTouchArea) {
        mSelectionStartTouchArea = new HbTouchArea(q);
        mSelectionStartTouchArea->hide();
        HbStyle::setItemName(mSelectionStartTouchArea, "handle-toucharea");

    }

    if (!mSelectionEndTouchArea) {
        mSelectionEndTouchArea = new HbTouchArea(q);
        mSelectionEndTouchArea->hide();
        HbStyle::setItemName(mSelectionEndTouchArea, "handle-toucharea");
    }
}

/*
   Updates given handle associated with handleTouchArea to place it
   newHandlePos in the selected text.
   handlePrimitive identifies handle graphics.
*/
void HbSelectionControlPrivate::updateHandle(int newHandlePos,
                  Qt::AlignmentFlag handleAlignment,
                  QGraphicsItem *handle,
                  QGraphicsItem *handleTouchArea,
                  HbStyle::Primitive handlePrimitive)
{    
    Q_Q(HbSelectionControl);

    HbStyleOption option;

    q->initStyleOption(&option);
    mEdit->style()->updatePrimitive(handle, handlePrimitive, &option);

    QRectF rect = mEdit->rectForPosition(newHandlePos,Qt::AlignTop?QTextLine::Leading:QTextLine::Trailing);

    // Convert rect to handle's parent coordinate system
    rect = handle->parentItem()->mapRectFromItem(mEdit,rect);

    // Center handle around center point of rect
    QRectF boundingRect = handle->boundingRect();

    boundingRect.moveCenter(rect.center());

    if (handleAlignment == Qt::AlignTop) {
       boundingRect.moveBottom(rect.top());
    } else {
       boundingRect.moveTop(rect.bottom());
    }

    handle->setPos(boundingRect.topLeft());

    // Center handle touch area around center pos of handle
    QPointF centerPos = boundingRect.center();
    boundingRect = handleTouchArea->boundingRect();
    boundingRect.moveCenter(centerPos);
    handleTouchArea->setPos(boundingRect.topLeft());

    if (!mPanInProgress) {
        QGraphicsItem * newParent = reparent(handle);
        reparent(handleTouchArea, newParent);
    }

    handle->show();
    handleTouchArea->show() ;
}



/*
   Reparents item to q if item's bounding rect intersects mEdit bounding rectangle or otherwise to
   HbAbstractEditPrivate::d_ptr(d->mEdit)->canvas.
   Returns new parent.
*/
QGraphicsItem * HbSelectionControlPrivate::reparent(QGraphicsItem *item)
{
    Q_Q(HbSelectionControl);

    QGraphicsItem *newParent = HbAbstractEditPrivate::d_ptr(mEdit)->canvas;

    // Convert bounding rect to mEdit's coordinate system
    QRectF rect = item->boundingRect();
    rect = item->mapRectToItem(mEdit,rect);

    if (mEdit->contains(rect.topLeft()) || mEdit->contains(rect.bottomRight())) {
        newParent = q;
    }

    reparent(item, newParent);
    return newParent;
}

void HbSelectionControlPrivate::reparent(QGraphicsItem *item, QGraphicsItem *newParent)
{
    if (item && newParent && newParent != item->parentItem()) {

        // Reparent handle items to newParent
        QPointF pos = newParent->mapFromItem(item->parentItem(),item->pos());

        // TODO: This is a workaround for a Qt bug when reparenting from a clipping parent to a
        //       non-clipping parent
        item->setParentItem(0);
        item->setParentItem(newParent);
        item->setPos(pos);
    }
}

void HbSelectionControlPrivate::reparentHandles(QGraphicsItem *newParent)
{
    // Reparent handle items to newParent
    reparent(mSelectionStartHandle, newParent);
    reparent(mSelectionStartTouchArea, newParent);
    reparent(mSelectionEndHandle, newParent);
    reparent(mSelectionEndTouchArea, newParent);
}


HbSelectionControl::HbSelectionControl(HbAbstractEdit *edit) :
    HbPopup(*new HbSelectionControlPrivate(edit),0)

{
    Q_D(HbSelectionControl);
    d->q_ptr = this;
    d->init();
    //TODO: selection control could be singleton per main window
    //      since only one selection control is used at a time
}


void HbSelectionControl::updatePrimitives()
{
    Q_D(HbSelectionControl);
    if (!d->mHandlesDisabled && d->polished) {
        if (d->mEdit->textCursor().hasSelection() ||
            (!d->mEdit->textCursor().hasSelection() && (d->mPressed == SelectionStartHandle || d->mPressed == SelectionEndHandle))) {

            int selStartPos = qMin(d->mEdit->textCursor().anchor(),d->mEdit->textCursor().position());
            int selEndPos = qMax(d->mEdit->textCursor().anchor(),d->mEdit->textCursor().position());

            d->updateHandle(selStartPos,Qt::AlignTop,d->mSelectionStartHandle,d->mSelectionStartTouchArea,HbStyle::P_SelectionControl_selectionstart);
            d->updateHandle(selEndPos,Qt::AlignBottom,d->mSelectionEndHandle,d->mSelectionEndTouchArea,HbStyle::P_SelectionControl_selectionend);
        }
        else {
            d->mSelectionStartHandle->hide();
            d->mSelectionStartTouchArea->hide() ;
            d->mSelectionEndHandle->hide();
            d->mSelectionEndTouchArea->hide() ;
        }
    }
}

void HbSelectionControl::hideHandles()
{
    Q_D(HbSelectionControl);
    if (!d->mHandlesDisabled) {
        d->mHandlesDisabled = true;
        hide();
        d->reparentHandles(this);
    }
}

void HbSelectionControl::showHandles()
{
    Q_D(HbSelectionControl);
    if (d->mHandlesDisabled) {
        d->mHandlesDisabled = false;
        show();
    }
}

void HbSelectionControl::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(HbSelectionControl);

    QPointF editPos = d->mEdit->mapFromScene(event->scenePos());

    QRectF handleRect = d->mSelectionStartHandle->boundingRect();
    handleRect.moveTopLeft(editPos + d->mMouseOffset);

    QPointF hitTestPos = handleRect.center();

    if (d->mPressed == SelectionStartHandle) {
        hitTestPos.setY(handleRect.bottom()+1);
    } else {
        hitTestPos.setY(handleRect.top()-1);
    }

    // Hit test for the center of current selection touch area
    int hitPos = HbAbstractEditPrivate::d_ptr(d->mEdit)->hitTest(hitTestPos,Qt::FuzzyHit);
    if (hitPos == -1) {
        return;
    }

    QTextCursor cursor = d->mEdit->textCursor();

    if (hitPos != cursor.position()) {
        d->mHandlesMoved = true;
    }
    cursor.setPosition(hitPos, QTextCursor::KeepAnchor);
    if (d->mHandlesMoved) {
        if (d->mEdit) {
            HbWidgetFeedback::triggered(d->mEdit, Hb::InstantDraggedOver);
        }
        // Restart timer every time when a selection handle moved
        d->mWordSnapTimer.start(SNAP_DELAY, this);
        d->mEdit->setTextCursor(cursor);
    }

    // Ensure that the hitPos is visible
    HbAbstractEditPrivate::d_ptr(d->mEdit)->ensurePositionVisible(hitPos);
    updatePrimitives();
}

void HbSelectionControl::mousePressEvent (QGraphicsSceneMouseEvent *event)
{
    Q_D(HbSelectionControl);

    if (d->mEdit) {
        HbWidgetFeedback::triggered(d->mEdit, Hb::InstantPressed);
    }

    d->mPressed = DummyHandle;

    // Find out which handle is being moved
    if (d->mSelectionStartTouchArea->contains(mapToItem(d->mSelectionStartTouchArea, event->pos()))) {
        d->mPressed = SelectionStartHandle;
        d->mMouseOffset = d->mSelectionStartHandle->pos() - event->pos();
    }
    if (d->mSelectionEndTouchArea->contains(mapToItem(d->mSelectionEndTouchArea, event->pos()))) {
        bool useArea = true;
        if(d->mPressed != DummyHandle) {

            // The press point was inside in both of the touch areas
            // choose the touch area whose center is closer to the press point
            QRectF rect = d->mSelectionStartTouchArea->boundingRect();
            rect.moveTopLeft(d->mSelectionStartTouchArea->pos());
            QLineF  lineEventPosSelStartCenter(event->pos(),rect.center());

            rect = d->mSelectionEndTouchArea->boundingRect();
            rect.moveTopLeft(d->mSelectionEndTouchArea->pos());
            QLineF  lineEventPosSelEndCenter(event->pos(),rect.center());

            if (lineEventPosSelStartCenter.length() < lineEventPosSelEndCenter.length()) {
                useArea = false;
            }
        }
        if (useArea) {
            d->mPressed = SelectionEndHandle;
            d->mMouseOffset = d->mSelectionEndHandle->pos() - event->pos();
        }
    }

    if (d->mPressed == DummyHandle) {
        // Hit is outside touch areas, ignore
        event->ignore();
        return;
    }

    // Position cursor at the pressed selection handle

    QTextCursor cursor = d->mEdit->textCursor();
    int selStartPos = qMin(d->mEdit->textCursor().anchor(),d->mEdit->textCursor().position());
    int selEndPos = qMax(d->mEdit->textCursor().anchor(),d->mEdit->textCursor().position());

    if (d->mPressed == SelectionStartHandle) {
        cursor.setPosition(selEndPos);
        cursor.setPosition(selStartPos, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(selStartPos);
        cursor.setPosition(selEndPos, QTextCursor::KeepAnchor);
    }
    d->mEdit->setTextCursor(cursor);

}

void HbSelectionControl::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
{
    Q_D(HbSelectionControl);
    Q_UNUSED(event);

    if (d->mEdit) {
        HbWidgetFeedback::triggered(d->mEdit, Hb::InstantReleased);
    }

    if (d->mWordSnapTimer.isActive()) {

        // Snap selection to word beginning or end
        QTextCursor cursor = d->mEdit->textCursor();
        int curPos = d->mEdit->textCursor().position();
        int anchPos = d->mEdit->textCursor().anchor();
        cursor.select(QTextCursor::WordUnderCursor);

        // Snap direction depends on cursor position
        curPos = ((curPos > anchPos)?cursor.position():cursor.anchor());

        cursor.setPosition(anchPos);
        cursor.setPosition(curPos, QTextCursor::KeepAnchor);
        d->mEdit->setTextCursor(cursor);
    }

    d->mPressed = DummyHandle;
    updatePrimitives();

    if (!d->mHandlesMoved) {
        if (d->mEdit->contextMenuFlags().testFlag(Hb::ShowTextContextMenuOnSelectionClicked)) {
            d->mEdit->showContextMenu(event->scenePos());
        }
    }
    d->mHandlesMoved = false;
}

void HbSelectionControl::panStarted()
{
    Q_D(HbSelectionControl);

    if (!d->mHandlesDisabled) {
        d->mPanInProgress = true;
        // Reparent handle items to editor canvas on pan start
        d->reparentHandles(HbAbstractEditPrivate::d_ptr(d->mEdit)->canvas);
    }
}

void HbSelectionControl::panFinished()
{
    Q_D(HbSelectionControl);

    if (!d->mHandlesDisabled) {
        d->mPanInProgress = false;
        updatePrimitives();
    }
}


void HbSelectionControl::timerEvent(QTimerEvent *event)
{
    Q_D(HbSelectionControl);

    if (event->timerId() == d->mWordSnapTimer.timerId()) {
        d->mWordSnapTimer.stop();
    }
}

void HbSelectionControl::polish( HbStyleParameters& params )
{
    Q_D(HbSelectionControl);

    HbPopup::polish(params);
    QSizeF size = d->mSelectionStartTouchArea->preferredSize();
    d->mSelectionStartTouchArea->resize(size);
    d->mSelectionEndTouchArea->resize(size);
}

QVariant HbSelectionControl::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        return qVariantFromValue(QPointF(0,0));
    } else if (change == QGraphicsItem::ItemVisibleChange) {        
        updatePrimitives();
    }

    return HbPopup::itemChange(change, value);
}
