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
#include "hbstyleoption_p.h"
#include "hbeffect.h"
#include "hbdialog_p.h"
#include "hbabstractedit.h"
#include "hbabstractedit_p.h"
#include "hbtoucharea.h"
#include "hbpangesture.h"
#include "hbtapgesture.h"
#include "hbevent.h"
#include "hbpopup.h"
#include "hbnamespace_p.h"
#include "hbmainwindow.h"


#include <QTextCursor>
#include <QGraphicsItem>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneMouseEvent>
#include <QBasicTimer>
#include <QSizeF>
#include <QPointF>
#include <QHash>
#include <QGraphicsScene>

#include <hbwidgetfeedback.h>

typedef QHash<HbMainWindow*,HbSelectionControl*> HbSelectionControlHash;
Q_GLOBAL_STATIC(HbSelectionControlHash, globalSelectionControlHash)

namespace {
    static const int SNAP_DELAY = 300;
}


class HbSelectionControlPrivate :public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbSelectionControl)

public:
    HbSelectionControlPrivate();
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
    void tapGestureFinished (const QPointF& point);
    void panGestureStarted (HbPanGesture *gesture);
    void panGestureUpdated (HbPanGesture *gesture);
    void panGestureFinished (HbPanGesture *gesture);
    void show();
    void _q_aboutToChangeView();
    void detachEditor(bool updateAtthachedEditorState);

public:

    HbAbstractEdit *mEdit;
    QGraphicsItem *mTopLevelAncestor;

    QGraphicsItem *mSelectionStartHandle;
    QGraphicsItem *mSelectionEndHandle;
    HbTouchArea* mSelectionStartTouchArea;
    HbTouchArea* mSelectionEndTouchArea;

    HbSelectionControl::HandleType mPressed;
    bool mPanInProgress;
    QBasicTimer mWordSnapTimer;
};


HbSelectionControlPrivate::HbSelectionControlPrivate():
    mEdit(0),
    mTopLevelAncestor(0),
    mSelectionStartHandle(0),
    mSelectionEndHandle(0),
    mSelectionStartTouchArea(0),
    mSelectionEndTouchArea(0),
    mPressed(HbSelectionControl::HandleType(0)),
    mPanInProgress(false)
{    
}

void HbSelectionControlPrivate::init()
{
    Q_Q(HbSelectionControl);
    createPrimitives();

    q->setVisible(false);
    q->setFlag(QGraphicsItem::ItemIsFocusable,false);
    q->setFlag(QGraphicsItem::ItemIsPanel,true);
    q->setFocusPolicy(Qt::NoFocus);
    q->setActive(false);

    // Control will handle all events going to different handlers.
    q->setHandlesChildEvents(true);
}

void HbSelectionControlPrivate::createPrimitives()
{
    Q_Q(HbSelectionControl);
    if (!mSelectionStartHandle) {
        mSelectionStartHandle = q->style()->createPrimitive(HbStyle::P_SelectionControl_selectionstart, q);
        mSelectionStartHandle->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionStartHandle->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionStartHandle->setActive(false);
    }

    if (!mSelectionEndHandle) {
        mSelectionEndHandle = q->style()->createPrimitive(HbStyle::P_SelectionControl_selectionend, q);
        mSelectionEndHandle->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionEndHandle->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionEndHandle->setActive(false);
    }

    if (!mSelectionStartTouchArea) {
        mSelectionStartTouchArea = new HbTouchArea(q);
        mSelectionStartTouchArea->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionStartTouchArea->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionStartTouchArea->setActive(false);
        HbStyle::setItemName(mSelectionStartTouchArea, "handle-toucharea");
        mSelectionStartTouchArea->grabGesture(Qt::TapGesture);
        mSelectionStartTouchArea->grabGesture(Qt::PanGesture);
    }

    if (!mSelectionEndTouchArea) {
        mSelectionEndTouchArea = new HbTouchArea(q);
        mSelectionEndTouchArea->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionEndTouchArea->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionEndTouchArea->setActive(false);
        HbStyle::setItemName(mSelectionEndTouchArea, "handle-toucharea");
        mSelectionEndTouchArea->grabGesture(Qt::TapGesture);
        mSelectionEndTouchArea->grabGesture(Qt::PanGesture);
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
}



/*
   Reparents item to q if item's bounding rect intersects mEdit's viewPort rectangle or otherwise to
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

    QRectF scrollAreaRect = HbAbstractEditPrivate::d_ptr(mEdit)->scrollArea->geometry();

    if (rect.intersects(scrollAreaRect)) {
        newParent = q;
    }

    reparent(item, newParent);
    return newParent;
}

void HbSelectionControlPrivate::reparent(QGraphicsItem *item, QGraphicsItem *newParent)
{
    Q_Q(HbSelectionControl);

    if (item && newParent && newParent != item->parentItem()) {

        // Reparent handle items to newParent
        QPointF pos = newParent->mapFromItem(item->parentItem(),item->pos());

        // If the item is parented to other then q we have to
        // turn off the QGraphicsItem::ItemIsPanel flag because
        // otherwise the new parent loses its activeness.
        bool enablePanel = (newParent == q);

        item->setFlag(QGraphicsItem::ItemIsPanel,enablePanel);

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


void HbSelectionControlPrivate::tapGestureFinished(const QPointF &pos)
{
    if (mEdit->contextMenuFlags().testFlag(Hb::ShowTextContextMenuOnSelectionClicked)) {
        mEdit->showContextMenu(pos);
    }
}

void HbSelectionControlPrivate::panGestureStarted(HbPanGesture *gesture)
{
    Q_Q(HbSelectionControl);

    QPointF point = q->mapFromScene(gesture->sceneStartPos());
    mPressed = HbSelectionControl::DummyHandle;

    // Find out which handle is being moved
    if (mSelectionStartTouchArea->contains(q->mapToItem(mSelectionStartTouchArea, point))) {
        mPressed = HbSelectionControl::SelectionStartHandle;
    }
    if (mSelectionEndTouchArea->contains(q->mapToItem(mSelectionEndTouchArea, point))) {
        bool useArea = true;
        if(mPressed != HbSelectionControl::DummyHandle) {

            // The press point was inside in both of the touch areas
            // choose the touch area whose center is closer to the press point
            QRectF rect = mSelectionStartTouchArea->boundingRect();
            rect.moveTopLeft(mSelectionStartTouchArea->pos());
            QLineF  lineEventPosSelStartCenter(point,rect.center());

            rect = mSelectionEndTouchArea->boundingRect();
            rect.moveTopLeft(mSelectionEndTouchArea->pos());
            QLineF  lineEventPosSelEndCenter(point,rect.center());

            if (lineEventPosSelStartCenter.length() < lineEventPosSelEndCenter.length()) {
                useArea = false;
            }
        }
        if (useArea) {
            mPressed = HbSelectionControl::SelectionEndHandle;
        }
    }

    if (mPressed == HbSelectionControl::DummyHandle) {
        // Hit is outside touch areas, ignore
        return;
    }

    // Position cursor at the pressed selection handle

    QTextCursor cursor = mEdit->textCursor();
    int selStartPos = qMin(mEdit->textCursor().anchor(),mEdit->textCursor().position());
    int selEndPos = qMax(mEdit->textCursor().anchor(),mEdit->textCursor().position());

    if (mPressed == HbSelectionControl::SelectionStartHandle) {
        cursor.setPosition(selEndPos);
        cursor.setPosition(selStartPos, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(selStartPos);
        cursor.setPosition(selEndPos, QTextCursor::KeepAnchor);
    }
    mEdit->setTextCursor(cursor);

}


void HbSelectionControlPrivate::panGestureFinished(HbPanGesture *gesture)
{
    Q_Q(HbSelectionControl);
    Q_UNUSED(gesture)

    if (mWordSnapTimer.isActive()) {

        // Snap selection to word beginning or end
        QTextCursor cursor = mEdit->textCursor();
        int curPos = mEdit->textCursor().position();
        int anchPos = mEdit->textCursor().anchor();
        cursor.select(QTextCursor::WordUnderCursor);

        // Snap direction depends on cursor position
        curPos = ((curPos > anchPos)?cursor.position():cursor.anchor());

        cursor.setPosition(anchPos);
        cursor.setPosition(curPos, QTextCursor::KeepAnchor);
        mEdit->setTextCursor(cursor);
    }

    mPressed = HbSelectionControl::DummyHandle;
    q->updatePrimitives();
}


void HbSelectionControlPrivate::panGestureUpdated(HbPanGesture *gesture)
{
    Q_Q(HbSelectionControl);

    QRectF docRect = QRectF(mEdit->mapFromItem(HbAbstractEditPrivate::d_ptr(mEdit)->canvas->parentItem(),
                            HbAbstractEditPrivate::d_ptr(mEdit)->canvas->pos()),
                            HbAbstractEditPrivate::d_ptr(mEdit)->doc->size());
    
    QPointF editPos = mEdit->mapFromScene(gesture->sceneStartPos() + gesture->sceneOffset());
    QPointF origEditPos = editPos;
    bool outsideCanvas = !docRect.contains(origEditPos);

    // Constrain editPos within docRect
    editPos = QPointF(qMin(qMax(editPos.x(),docRect.left()),docRect.right()),
                      qMin(qMax(editPos.y(),docRect.top()),docRect.bottom()));

    QRectF handleRect = mSelectionStartHandle->boundingRect();

    handleRect.moveCenter(editPos);

    // Set hitTestPos based on which handle was grabbed
    QPointF hitTestPos = handleRect.center();

    if (mPressed == HbSelectionControl::SelectionStartHandle) {
        hitTestPos.setY(handleRect.bottom()+1);
    } else {
        hitTestPos.setY(handleRect.top()-1);
    }

    // Override hitTestPos if origEditPos was outside the canvas
    if (outsideCanvas) {
        if (origEditPos.y() < docRect.top()) {
            hitTestPos.setY(handleRect.bottom()+1);
        } else if (docRect.bottom() < origEditPos.y()) {
            hitTestPos.setY(handleRect.top()-1);
        }
    }

    QTextCursor cursor;
    cursor = mEdit->textCursor();
    // Hit test for the center of current selection touch area
    int hitPos = HbAbstractEditPrivate::d_ptr(mEdit)->hitTest(hitTestPos,Qt::FuzzyHit);

    // if no valid hit pos or empty selection return
    if (hitPos == -1 || hitPos == cursor.anchor()) {
        return;
    }

    bool handlesMoved(false);
    if (hitPos != cursor.position()) {
        handlesMoved = true;
    }
    cursor.setPosition(hitPos, QTextCursor::KeepAnchor);
    if (handlesMoved) {
        if (mEdit) {
            HbWidgetFeedback::triggered(mEdit, Hb::InstantDraggedOver);
        }
        // Restart timer every time when a selection handle moved
        mWordSnapTimer.start(SNAP_DELAY, q);
        mEdit->setTextCursor(cursor);
    }

    // Ensure that the hitPos is visible
    HbAbstractEditPrivate::d_ptr(mEdit)->ensurePositionVisible(hitPos);
    q->updatePrimitives();
}

void HbSelectionControlPrivate::show() {
    Q_Q(HbSelectionControl);

    // Set the z-value of the selection control above its top-level ancestor
    if (mTopLevelAncestor) {
        qreal zValue = mTopLevelAncestor->zValue() + HbPrivate::SelectionControlHandlesValueUnit;

        q->setZValue(zValue);
    }

    if (q->scene() != mEdit->scene() && mEdit->scene()) {
        mEdit->scene()->addItem(q);
    }
    q->show();    
    q->updatePrimitives();
}


void HbSelectionControlPrivate::_q_aboutToChangeView()
{
    Q_Q(HbSelectionControl);

    if (mEdit && q->isVisible()) {
        mEdit->deselect();
    }
}

void HbSelectionControlPrivate::detachEditor(bool updateAtthachedEditorState)
{
    Q_Q(HbSelectionControl);
    if (mEdit) {
        q->hideHandles();
        reparentHandles(q);
        if (updateAtthachedEditorState) {
            mEdit->disconnect(q);
            mEdit->d_func()->selectionControl = 0;
            mEdit->deselect();
        }
        mEdit = 0;
        mTopLevelAncestor = 0;
    }
}

HbSelectionControl::HbSelectionControl() : HbWidget(*new HbSelectionControlPrivate(),0)

{
    Q_D(HbSelectionControl);
    d->q_ptr = this;
    d->init();
    //TODO: selection control could be singleton per main window
    //      since only one selection control is used at a time
}

HbSelectionControl* HbSelectionControl::attachEditor(HbAbstractEdit *edit)
{
    if(!edit || !edit->mainWindow()) {
        qWarning("HbSelectionControl: attempting to attach to null editor pointer!");
    }

    HbSelectionControl *control = globalSelectionControlHash()->value(edit->mainWindow());

    if (!control) {
        control = new HbSelectionControl();
        globalSelectionControlHash()->insert(edit->mainWindow(),control);
        QObject::connect(edit->mainWindow(), SIGNAL(aboutToChangeView(HbView *, HbView *)), control, SLOT(_q_aboutToChangeView()));
    }

    HbSelectionControlPrivate *d = control->d_func();

    if (edit != d->mEdit) {
        control->detachEditor();
        d->mEdit = edit;        
        QObject::connect(d->mEdit, SIGNAL(cursorPositionChanged(int, int)), control, SLOT(updatePrimitives()));
        QObject::connect(d->mEdit, SIGNAL(contentsChanged()), control, SLOT(updatePrimitives()));

        // find first top-level ancestor of d->mEdit
        for(d->mTopLevelAncestor = d->mEdit;
            d->mTopLevelAncestor->parentItem();
            d->mTopLevelAncestor = d->mTopLevelAncestor->parentItem()){};
    }
    return control;
}

void HbSelectionControl::detachEditor()
{
    Q_D(HbSelectionControl);
    d->detachEditor(true);
}

void HbSelectionControl::detachEditorFromDestructor()
{
    Q_D(HbSelectionControl);
    d->detachEditor(false);
}

void HbSelectionControl::hideHandles()
{
    Q_D(HbSelectionControl);
    if (isVisible() && d->mEdit) {
        hide();
        d->reparentHandles(this);
    }
}

void HbSelectionControl::showHandles()
{
    Q_D(HbSelectionControl);
    if (!isVisible() && d->mEdit) {
        d->show();
    }
}

void HbSelectionControl::scrollStarted()
{
    Q_D(HbSelectionControl);

    if (isVisible() && d->mEdit) {
        d->mPanInProgress = true;
        // Reparent handle items to editor canvas on pan start
        d->reparentHandles(HbAbstractEditPrivate::d_ptr(d->mEdit)->canvas);
    }
}

void HbSelectionControl::scrollFinished()
{
    Q_D(HbSelectionControl);

    if (isVisible() && d->mEdit) {
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

    HbWidget::polish(params);
    QSizeF size = d->mSelectionStartTouchArea->preferredSize();
    d->mSelectionStartTouchArea->resize(size);
    d->mSelectionEndTouchArea->resize(size);
    updatePrimitives();
}

QVariant HbSelectionControl::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        return qVariantFromValue(QPointF(0,0));
    }

    return HbWidget::itemChange(change, value);
}

void HbSelectionControl::gestureEvent(QGestureEvent* event) {
    Q_D(HbSelectionControl);
    if(HbTapGesture *tap = qobject_cast<HbTapGesture*>(event->gesture(Qt::TapGesture))) {
        QPointF pos = event->mapToGraphicsScene(tap->position());
        switch(tap->state()) {
        case Qt::GestureStarted:
            if (d->mEdit) {
                HbWidgetFeedback::triggered(d->mEdit, Hb::InstantPressed);
            }
            break;
        case Qt::GestureUpdated:
            break;
      case Qt::GestureFinished:
            if (d->mEdit) {
                d->tapGestureFinished(pos);
                HbWidgetFeedback::triggered(d->mEdit, Hb::InstantReleased);
            }
            break;
      case Qt::GestureCanceled:
            break;
      default:
            break;
        }
    }

    if(HbPanGesture *pan = qobject_cast<HbPanGesture*>(event->gesture(Qt::PanGesture))) {
        switch(pan->state()) {
        case Qt::GestureStarted:
            if (d->mEdit) {
                d->panGestureStarted(pan);
            }
            break;
        case Qt::GestureUpdated:
            if (d->mEdit) {
                d->panGestureUpdated(pan);
            }
            break;
        case Qt::GestureFinished:
            if (d->mEdit) {
                d->panGestureFinished(pan);
                HbWidgetFeedback::triggered(d->mEdit, Hb::InstantReleased);
            }
            break;
      case Qt::GestureCanceled:
            break;
      default:
            break;
        }
    }
}

bool HbSelectionControl::event(QEvent *event)
{
    Q_D(HbSelectionControl);

    if (event->type() == HbEvent::DeviceProfileChanged && d->mEdit) {
        HbDeviceProfileChangedEvent* dpEvent = static_cast<HbDeviceProfileChangedEvent*>(event);
        if ( dpEvent->profile().alternateProfileName() == dpEvent->oldProfile().name() ) {
            updatePrimitives();
        }
    }
    return HbWidget::event(event);
}

void HbSelectionControl::updatePrimitives()
{
    Q_D(HbSelectionControl);

    if (isVisible() && d->polished && d->mEdit) {
        if (d->mEdit->textCursor().hasSelection() ) {
           
            int selStartPos = qMin(d->mEdit->textCursor().anchor(),d->mEdit->textCursor().position());
            int selEndPos = qMax(d->mEdit->textCursor().anchor(),d->mEdit->textCursor().position());

            d->updateHandle(selStartPos,Qt::AlignTop,d->mSelectionStartHandle,d->mSelectionStartTouchArea,HbStyle::P_SelectionControl_selectionstart);
            d->updateHandle(selEndPos,Qt::AlignBottom,d->mSelectionEndHandle,d->mSelectionEndTouchArea,HbStyle::P_SelectionControl_selectionend);
        }
        else {
            hide();
        }
    }
}
#include "moc_hbselectioncontrol_p.cpp"
