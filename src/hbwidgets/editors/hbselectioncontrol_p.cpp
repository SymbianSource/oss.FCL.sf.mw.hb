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
#include "hblineedit.h"
#include "hbtoucharea.h"
#include "hbpangesture.h"
#include "hbtapgesture.h"
#include "hbevent.h"
#include "hbpopup.h"
#include "hbmagnifier_p.h"
#include "hbnamespace_p.h"
#include "hbmainwindow.h"
#include "hbdeviceprofile.h"
#include "hbiconitem.h"


#include <QTextCursor>
#include <QTextBlock>
#include <QGraphicsItem>
#include <QGraphicsWidget>
#include <QAbstractTextDocumentLayout>
#include <QGraphicsSceneMouseEvent>
#include <QBasicTimer>
#include <QSizeF>
#include <QPointF>
#include <QHash>
#include <QGraphicsScene>

#include <hbwidgetfeedback.h>


#define HB_DEBUG_PAINT_INFO 0

typedef QHash<HbMainWindow*,HbSelectionControl*> HbSelectionControlHash;
Q_GLOBAL_STATIC(HbSelectionControlHash, globalSelectionControlHash)

namespace {
    static const int SNAP_DELAY = 300;
    static const int MAGNIFIER_OPEN_DELAY = 200;
    static const qreal MAGNIFIER_SCALE_FACTOR = 1.5;
}

class MyEditor: public HbAbstractEdit
{
public:
    using HbAbstractEdit::drawContents;
};


class HbMagnifierDelegateEditor : public HbMagnifierDelegate
{
public:
    HbMagnifierDelegateEditor(HbAbstractEdit * editor) {this->editor = editor;}
    virtual void drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option)
    {
        if (option && editor) {
            static_cast<MyEditor*>(editor)->drawContents(painter,*option);
        }
    }
    void setEditor(HbAbstractEdit * editor){this->editor = editor;}
private:
    HbAbstractEdit * editor;
};


class HbSelectionControlPrivate :public HbDialogPrivate
{
    Q_DECLARE_PUBLIC(HbSelectionControl)

public:
    enum InteractionMode {
        Selection,
        CursorPositioning
    };

    enum HandleType {
        DummyHandle,
        SelectionStartHandle,
        SelectionEndHandle
    };


    HbSelectionControlPrivate();
    ~HbSelectionControlPrivate();
    void init();
    void createPrimitives();
    void updateHandle(int cursorPos,
                      int newHandlePos,
                      Qt::AlignmentFlag handleAlignment,
                      QGraphicsItem *handle,
                      QGraphicsItem *handleTouchArea);
    void updateMagnifier();
    void initMagnifier();

    QGraphicsItem * reparent(QGraphicsItem *item);
    void reparent(QGraphicsItem *item, QGraphicsItem *newParent);
    void reparentHandles(QGraphicsItem *newParent);
    QPointF constrainHitTestPoint(const QPointF& point);
    HbSelectionControlPrivate::HandleType handleForPoint(const QPointF& point);
    void gestureStarted(const QPointF &point);
    void tapGestureStarted(HbTapGesture *gesture);
    void tapGestureFinished();
    void delayedTapFinished();
    void panGestureStarted (HbPanGesture *gesture);
    void panGestureUpdated (HbPanGesture *gesture);
    void panGestureFinished (HbPanGesture *gesture);
    void panGestureCanceled();
    void show();
    void _q_aboutToChangeView();
    void detachEditor(bool updateAtthachedEditorState);

#if HB_DEBUG_PAINT_INFO
    void updateDebugPaintInfo();
#endif


public:

    HbAbstractEdit *mEdit;
    HbMagnifierDelegateEditor *mMagnifierDelegate; // Owned by mMagnifier
    HbMagnifier* mMagnifier;

    QGraphicsItem *mTopLevelAncestor;    
    // The offset between the gesture start point and first hit test point
    QPointF mTouchOffsetFromHitTestPoint;

    HbIconItem *mSelectionStartHandle;
    HbIconItem *mSelectionEndHandle;
    HbTouchArea* mSelectionStartTouchArea;
    HbTouchArea* mSelectionEndTouchArea;

    InteractionMode mInteractionMode;
    HandleType mPressed;
    bool mScrollInProgress;
    QPointF mHitTestPoint;                 // HbSelectionControl's coordinate system
    QPointF mTouchPoint;                   // HbSelectionControl's coordinate system
    QPointF mStartHandleHitTestPoint;      // HbSelectionControl's coordinate system
    QPointF mEndHandleHitTestPoint;        // HbSelectionControl's coordinate system
    QBasicTimer mWordSnapTimer;
    QBasicTimer mDelayedTapTimer;

    bool mMagnifierEnabled;
    qreal mLastCursorHeight;
    qreal mHandleMarginFromLine;
    qreal mMagnifierMarginFromLine;
    qreal mMagnifierLeftRightMarginFromHandle;
    qreal mMagnifierMaxDescent;
    qreal mVerticalScreenMargin;
    qreal mTouchYOffsetFromMagnifierReferenceLine; // HbSelectionControl's coordinate system
    qreal mStartHandleMagnifierReferenceLine;     // HbSelectionControl's coordinate system
    qreal mEndHandleMagnifierReferenceLine;       // HbSelectionControl's coordinate system
    qreal mMagnifierMaxYOffsetFromTouchPoint;
    qreal mMagnifierMinYOffsetFromTouchPoint;
    qreal mMaxHitTestPointOffsetYFromLine;
    bool mIsPanActive;

#if HB_DEBUG_PAINT_INFO
    QRectF  mDocRectDebug;      // HbSelectionControl's coordinate system
#endif
};


HbSelectionControlPrivate::HbSelectionControlPrivate():
    mEdit(0),
    mMagnifierDelegate(0),
    mMagnifier(0),
    mTopLevelAncestor(0),
    mSelectionStartHandle(0),
    mSelectionEndHandle(0),
    mSelectionStartTouchArea(0),
    mSelectionEndTouchArea(0),
    mInteractionMode(InteractionMode(0)),
    mPressed(HandleType(0)),
    mScrollInProgress(false),
    mMagnifierEnabled(true),
    mLastCursorHeight(0.0),
    mHandleMarginFromLine(0.0),
    mMagnifierMarginFromLine(0.0),
    mMagnifierLeftRightMarginFromHandle(0.0),
    mMagnifierMaxDescent(0.0),
    mVerticalScreenMargin(0.0),
    mTouchYOffsetFromMagnifierReferenceLine(0.0),
    mStartHandleMagnifierReferenceLine(0.0),
    mEndHandleMagnifierReferenceLine(0.0),
    mMagnifierMaxYOffsetFromTouchPoint(0.0),
    mMagnifierMinYOffsetFromTouchPoint(0.0),
    mMaxHitTestPointOffsetYFromLine(0.0),
    mIsPanActive(false)
{    
}

HbSelectionControlPrivate::~HbSelectionControlPrivate()
{
}

void HbSelectionControlPrivate::init()
{
    Q_Q(HbSelectionControl);

    // Set the size of the control to 0
    q->resize(0,0);

#if HB_DEBUG_PAINT_INFO
    // Override 0 size to be able to paint paint
    q->resize(1,1);
#endif

    q->style()->parameter(QLatin1String("hb-param-margin-gene-middle-vertical"), mVerticalScreenMargin);

    createPrimitives();

    q->setVisible(false);
    QGraphicsItem::GraphicsItemFlags itemFlags = q->flags();
#if QT_VERSION >= 0x040600
    itemFlags |=  QGraphicsItem::ItemSendsGeometryChanges;
#endif
    itemFlags &= ~QGraphicsItem::ItemIsFocusable;
    itemFlags |=  QGraphicsItem::ItemIsPanel;
    q->setFlags(itemFlags);
    q->setFocusPolicy(Qt::NoFocus);
    q->setActive(false);
}

void HbSelectionControlPrivate::createPrimitives()
{
    Q_Q(HbSelectionControl);
    if (!mSelectionStartHandle) {

        mSelectionStartHandle = new HbIconItem(q);
        mSelectionStartHandle->setIconName(QLatin1String("qtg_graf_editor_handle_begin"));
        HbStyle::setItemName(mSelectionStartHandle, QLatin1String("handle-icon"));
        mSelectionStartHandle->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionStartHandle->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionStartHandle->setActive(false);
    }

    if (!mSelectionEndHandle) {
        mSelectionEndHandle = new HbIconItem(q);
        mSelectionEndHandle->setIconName(QLatin1String("qtg_graf_editor_handle_end"));
        HbStyle::setItemName(mSelectionEndHandle, QLatin1String("handle-icon"));
        mSelectionEndHandle->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionEndHandle->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionEndHandle->setActive(false);
    }

    if (!mSelectionStartTouchArea) {
        mSelectionStartTouchArea = new HbTouchArea(q);
        mSelectionStartTouchArea->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionStartTouchArea->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionStartTouchArea->setActive(false);
        HbStyle::setItemName(mSelectionStartTouchArea, QLatin1String("handle-toucharea"));
        mSelectionStartTouchArea->grabGesture(Qt::TapGesture);
        mSelectionStartTouchArea->grabGesture(Qt::PanGesture);
        mSelectionStartTouchArea->installEventFilter(q);
    }

    if (!mSelectionEndTouchArea) {
        mSelectionEndTouchArea = new HbTouchArea(q);
        mSelectionEndTouchArea->setFlag(QGraphicsItem::ItemIsPanel);
        mSelectionEndTouchArea->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mSelectionEndTouchArea->setActive(false);
        HbStyle::setItemName(mSelectionEndTouchArea, QLatin1String("handle-toucharea"));
        mSelectionEndTouchArea->grabGesture(Qt::TapGesture);
        mSelectionEndTouchArea->grabGesture(Qt::PanGesture);
        mSelectionEndTouchArea->installEventFilter(q);
    }
    if(!mMagnifier) {
        mMagnifier = new HbMagnifier(q);
        mMagnifier->setFlag(QGraphicsItem::ItemIsPanel);
        mMagnifier->setFlag(QGraphicsItem::ItemIsFocusable,false);
        mMagnifier->setActive(false);
        HbStyle::setItemName(mMagnifier, QLatin1String("magnifier"));
        mMagnifierDelegate = new HbMagnifierDelegateEditor(mEdit);
        mMagnifier->setContentDelegate(mMagnifierDelegate);
        mMagnifier->hide();
        initMagnifier();
    }
}

/*
   Updates given handle associated with handleTouchArea to place it
   newHandlePos in the selected text.
*/
void HbSelectionControlPrivate::updateHandle(
                  int cursorPos,
                  int newHandlePos,
                  Qt::AlignmentFlag handleAlignment,
                  QGraphicsItem *handle,
                  QGraphicsItem *handleTouchArea)
{    
    Q_Q(HbSelectionControl);

    QRectF rect = mEdit->rectForPosition(newHandlePos,(Qt::AlignTop||mInteractionMode == CursorPositioning)?
                                                       QTextLine::Leading:QTextLine::Trailing);
    qreal lineHeight = rect.height();

    if(cursorPos == newHandlePos) {
        mLastCursorHeight = lineHeight;
    }

    // Store the current hit test point for the given handle
    // Convert rect to HbSelectionControl's coordinate system
    QRectF rectInSelectionControlCoord = q->mapRectFromItem(mEdit,rect);

    if (handleAlignment == Qt::AlignTop) {
        mStartHandleHitTestPoint = rectInSelectionControlCoord.center();
        mStartHandleHitTestPoint.setY(qMin(mStartHandleHitTestPoint.y(),
                                           rectInSelectionControlCoord.top()+mMaxHitTestPointOffsetYFromLine));
        mStartHandleMagnifierReferenceLine = rectInSelectionControlCoord.top();
    } else {
        mEndHandleHitTestPoint = rectInSelectionControlCoord.center();
        mEndHandleHitTestPoint.setY(qMax(mEndHandleHitTestPoint.y(),
                                         rectInSelectionControlCoord.bottom()-mMaxHitTestPointOffsetYFromLine));
        mEndHandleMagnifierReferenceLine = rectInSelectionControlCoord.bottom();
    }

    // Convert rect to handle's parent coordinate system
    rect = handle->parentItem()->mapRectFromItem(mEdit,rect);

    // Center handle around center point of rect
    QRectF boundingRect = handle->boundingRect();

    bool positionHandlesTouchEachOther = (boundingRect.height()*2 > lineHeight);

    boundingRect.moveCenter(rect.center());

    // Position handle either on top of rect using margin mHandleMarginFromLine or so that
    // the two handles touch eachother
    if (handleAlignment == Qt::AlignTop) {
        if (positionHandlesTouchEachOther) {
            boundingRect.moveBottom(rect.center().y());
        } else {
            boundingRect.moveTop(rect.top()-mHandleMarginFromLine);
        }
    } else {
        if (positionHandlesTouchEachOther) {
            boundingRect.moveTop(rect.center().y());
        } else {
            boundingRect.moveBottom(rect.bottom()+mHandleMarginFromLine);
        }
    }

    handle->setPos(boundingRect.topLeft());

    // Position handle touch area around center-top of handle
    QPointF centerPos = boundingRect.center();
    rect = boundingRect;
    boundingRect = handleTouchArea->boundingRect();
    boundingRect.moveCenter(centerPos);

    if (handleAlignment != Qt::AlignTop) {
        boundingRect.moveTop(rect.top());
    }

    handleTouchArea->setPos(boundingRect.topLeft());

    if (!mScrollInProgress) {
        QGraphicsItem * newParent = reparent(handle);
        reparent(handleTouchArea, newParent);
    }
}


void HbSelectionControlPrivate::updateMagnifier()
{
    Q_Q(HbSelectionControl);

    if (mMagnifier->isVisible()) {
        QPointF canvasHitTestPos = q->mapToItem(HbAbstractEditPrivate::d_ptr(mEdit)->canvas,mHitTestPoint);
        QRectF canvasCursorRect = HbAbstractEditPrivate::d_ptr(mEdit)->rectForPositionInCanvasCoords(mEdit->textCursor().position(),QTextLine::Leading);
        QPointF centerOfMagnification = canvasCursorRect.center();
        centerOfMagnification.setX(canvasHitTestPos.x());
        canvasCursorRect.moveCenter(centerOfMagnification);

        // Set the visible content to be magnified
        qreal magnifierHeight = mMagnifier->boundingRect().height();

        // Check if the line fits vertically to the magnifier if not align the bottom of the line with the bottom of the
        // magnifier constraining a maximum line descent.
        if (magnifierHeight < canvasCursorRect.height()*mMagnifier->contentScale()) {
            const QTextCursor cursor = mEdit->textCursor();
            const QTextBlock block = cursor.block();
            const int positionInBlock = cursor.position() - block.position();
            QTextLine line = block.layout()->lineForTextPosition(positionInBlock);
            qreal lineDescent = line.descent();
            qreal lineBottom  = canvasCursorRect.bottom();
            if (mMagnifierMaxDescent < lineDescent) {
                lineBottom-= lineDescent-mMagnifierMaxDescent;
            }
            // Compensate for rounding error by adding +1
            centerOfMagnification.setY(lineBottom-magnifierHeight/(2*mMagnifier->contentScale())+1);
        }

        mMagnifier->centerOnContent(centerOfMagnification);

        // -- Position magnifier --

        // Convert cursorRect to HbSelectionControl's coordinate system
        QRectF rect = q->mapRectFromItem(HbAbstractEditPrivate::d_ptr(mEdit)->canvas,canvasCursorRect);

        QRectF boundingRect = mMagnifier->boundingRect();
        boundingRect.moveCenter(rect.center());

        const qreal KMagnifierBottomUpperBound = rect.top()-mMagnifierMarginFromLine+mTouchYOffsetFromMagnifierReferenceLine;
        const qreal KMagnifierBottomLowerBound = rect.bottom()-mMagnifierMarginFromLine+mTouchYOffsetFromMagnifierReferenceLine;

        qreal newMagnifierBottom = (mPressed == SelectionStartHandle?KMagnifierBottomUpperBound:KMagnifierBottomLowerBound);

        // -- Constrain vertical position --

        // Constrain maximum distance of bottom of the magnifier from touch point
        newMagnifierBottom = qMax(newMagnifierBottom,mTouchPoint.y()-mMagnifierMaxYOffsetFromTouchPoint);

        // Constrain minimum distance of bottom of the magnifier from touch point
        newMagnifierBottom = qMin(newMagnifierBottom,mTouchPoint.y()-mMagnifierMinYOffsetFromTouchPoint);

        // Constrain the bottom of the magnifier to be within the feasible bounds
        newMagnifierBottom = qMin(qMax(newMagnifierBottom,KMagnifierBottomUpperBound),KMagnifierBottomLowerBound);

        boundingRect.moveBottom(newMagnifierBottom);
        boundingRect.moveTop(qMax(mVerticalScreenMargin,boundingRect.top()));

        // Readjust magnifier position if there is not enough space at the top of the screen
        if (mTouchPoint.y()-mMagnifierMinYOffsetFromTouchPoint < boundingRect.bottom()) {

            boundingRect.moveRight(mTouchPoint.x()-mMagnifierLeftRightMarginFromHandle);

            if(boundingRect.left() < 0) {
                boundingRect.moveLeft(mTouchPoint.x()+mMagnifierLeftRightMarginFromHandle);
            }
        }
        mMagnifier->setPos(boundingRect.topLeft());
#if HB_DEBUG_PAINT_INFO
    updateDebugPaintInfo();
#endif
    }

}

void HbSelectionControlPrivate::initMagnifier()
{   
    mMagnifier->setContentScale(MAGNIFIER_SCALE_FACTOR);
    mMagnifier->setZValue(100); 
    mMagnifier->setMask(QLatin1String("qtg_fr_editor_magnifier_mask"));
    mMagnifier->setOverlay(QLatin1String("qtg_fr_editor_magnifier_overlay"));
    updateMagnifier();
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
    if (item && newParent && newParent != item->parentItem()) {

        // Reparent handle items to newParent
        QPointF pos = newParent->mapFromItem(item->parentItem(),item->pos());

        // ----------
        // Workaround for Qt bug: Missing repaint when QGraphicsItem is reparented to another
        // item with flag QGraphicsItem::ItemClipsChildrenToShape is set.
        // ----------
        // Remove when the bug is fixed
        if(item->isWidget()) {
            if (qobject_cast<HbIconItem *>(static_cast<QGraphicsWidget *>(item))) {
                item->scene()->invalidate(item->sceneBoundingRect());
            }
        }
        // ----------------------------------------------------------------------------------

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


/*
    Returns the constrained hit test point calculated from point.
    point has to be in HbSelectionControl's coordinate system.
    The returned pos is also in HbSelectionControl's coordinate system.
*/
QPointF HbSelectionControlPrivate::constrainHitTestPoint(const QPointF& point)
{
    Q_Q(HbSelectionControl);

    QRectF docRect = QRectF(q->mapFromItem(HbAbstractEditPrivate::d_ptr(mEdit)->canvas->parentItem(),
                            HbAbstractEditPrivate::d_ptr(mEdit)->canvas->pos()),
                            HbAbstractEditPrivate::d_ptr(mEdit)->doc->size());

    // Constrain hitTestPos within docRect with mLastCursorHeight/2 top/bottom margins.
    QPointF hitTestPos = QPointF(qMin(qMax(point.x(),docRect.left()),docRect.right()),
                         qMin(qMax(point.y(),docRect.top()+mLastCursorHeight/2),docRect.bottom()-mLastCursorHeight/2));

#if HB_DEBUG_PAINT_INFO
    mDocRectDebug = docRect;
#endif

    return hitTestPos;
}


/*
    Returns the handle whose touch area contains point.
    point has to be in HbSelectionControl's coordinate system.
*/
HbSelectionControlPrivate::HandleType HbSelectionControlPrivate::handleForPoint(const QPointF& point) {

    Q_Q(HbSelectionControl);

    HandleType pressed = DummyHandle;

    // Find out which handle is being moved
    if (mSelectionStartTouchArea->isVisible() &&
        mSelectionStartTouchArea->contains(q->mapToItem(mSelectionStartTouchArea, point))) {
        pressed = SelectionStartHandle;
    }
    if (mSelectionEndTouchArea->contains(q->mapToItem(mSelectionEndTouchArea, point))) {
        bool useArea = true;
        if(pressed != DummyHandle) {

            // The press point was inside in both of the touch areas
            // choose the touch area whose center is closer to the press point
            QRectF rect = mSelectionStartHandle->boundingRect();
            rect.moveTopLeft(mSelectionStartHandle->pos());
            QLineF  lineEventPosSelStartCenter(point,rect.center());

            rect = mSelectionEndHandle->boundingRect();
            rect.moveTopLeft(mSelectionEndHandle->pos());
            QLineF  lineEventPosSelEndCenter(point,rect.center());

            if (lineEventPosSelStartCenter.length() < lineEventPosSelEndCenter.length()) {
                useArea = false;
            }
        }
        if (useArea) {
            pressed = SelectionEndHandle;
        }
    }

    return pressed;
}

void HbSelectionControlPrivate::gestureStarted(const QPointF &point)
{
    mPressed = handleForPoint(point);
    mTouchPoint = point;

    if (mPressed == DummyHandle) {
        // Hit is outside touch areas, ignore
        return;
    }

    // Calculate touch offsets
    mHitTestPoint = mStartHandleHitTestPoint;
    qreal magnifierReferenceLine = mStartHandleMagnifierReferenceLine;
    if (mPressed == SelectionEndHandle) {
        mHitTestPoint = mEndHandleHitTestPoint;
        magnifierReferenceLine = mEndHandleMagnifierReferenceLine;
    }

    mTouchYOffsetFromMagnifierReferenceLine = point.y() - magnifierReferenceLine;
    mTouchOffsetFromHitTestPoint = mHitTestPoint - point;

    // Position cursor at the pressed selection handle

    QTextCursor cursor = mEdit->textCursor();
    int selStartPos = qMin(mEdit->textCursor().anchor(),mEdit->textCursor().position());
    int selEndPos = qMax(mEdit->textCursor().anchor(),mEdit->textCursor().position());

    if (mPressed == SelectionStartHandle) {
        cursor.setPosition(selEndPos);
        cursor.setPosition(selStartPos, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(selStartPos);
        cursor.setPosition(selEndPos, QTextCursor::KeepAnchor);
    }
    mEdit->setTextCursor(cursor);
}

void HbSelectionControlPrivate::tapGestureStarted(HbTapGesture *gesture)
{
    Q_Q(HbSelectionControl);

    mMagnifier->hideWithEffect();
    if (!mDelayedTapTimer.isActive()) {
        mDelayedTapTimer.start(MAGNIFIER_OPEN_DELAY,q);
    }
    QPointF point = q->mapFromScene(gesture->sceneStartPos());
    gestureStarted(point);
}

void HbSelectionControlPrivate::tapGestureFinished()
{
    mDelayedTapTimer.stop();
    if (!mIsPanActive) {
        mSelectionStartTouchArea->show();
        mSelectionStartHandle->show();
        mMagnifier->hideWithEffect();
    }
}

void HbSelectionControlPrivate::delayedTapFinished()
{
    Q_Q(HbSelectionControl);

    // Reset gesture override to have enable more responsive pan
    q->scene()->setProperty(HbPrivate::OverridingGesture.latin1(),QVariant());

    if (mPressed == HbSelectionControlPrivate::SelectionEndHandle && mInteractionMode == HbSelectionControlPrivate::CursorPositioning) {
        mSelectionStartTouchArea->hide();
        mSelectionStartHandle->hide();
    }
    if (mMagnifierEnabled) {
        mMagnifier->showWithEffect();
        q->updatePrimitives();
    }
}



void HbSelectionControlPrivate::panGestureStarted(HbPanGesture *gesture)
{
    Q_Q(HbSelectionControl);

    mIsPanActive = true;
    QPointF point = q->mapFromScene(gesture->sceneStartPos());
    gestureStarted(point);
    if (mPressed == SelectionEndHandle && mInteractionMode == HbSelectionControlPrivate::CursorPositioning) {
        mSelectionStartTouchArea->hide();
        mSelectionStartHandle->hide();
    }
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

    mSelectionStartTouchArea->show();
    mSelectionStartHandle->show();
    // This has to be set before updatePrimitives call
    mIsPanActive = false;
    q->updatePrimitives();
    mMagnifier->hideWithEffect();

    // This has to be set at last
    mPressed = DummyHandle;
}

void HbSelectionControlPrivate::panGestureCanceled()
{
    mMagnifier->hideWithEffect();
    mIsPanActive = false;
}


void HbSelectionControlPrivate::panGestureUpdated(HbPanGesture *gesture)
{
    Q_Q(HbSelectionControl);

    // Calculate new hittest point
    QPointF point = q->mapFromScene(gesture->sceneStartPos() + gesture->sceneOffset());
    mTouchPoint = point;
    mHitTestPoint = (point + mTouchOffsetFromHitTestPoint);
    mHitTestPoint = constrainHitTestPoint(mHitTestPoint);

#if HB_DEBUG_PAINT_INFO
    updateDebugPaintInfo();
#endif

    QTextCursor cursor;
    cursor = mEdit->textCursor();

    // Hit test for the center of current selection touch area
    int hitPos = HbAbstractEditPrivate::d_ptr(mEdit)->hitTest(q->mapToItem(mEdit,mHitTestPoint),Qt::FuzzyHit);

    // if no valid hit pos or empty selection in read-only mode return
    if (hitPos == -1 || (mEdit->isReadOnly() && hitPos == cursor.anchor() && mInteractionMode == Selection)) {
        return;
    }

    bool isCursorMoved = false;
    if (hitPos != cursor.position()) {
        isCursorMoved = true;
    }

    cursor.setPosition(hitPos, ((mInteractionMode==Selection||mPressed == SelectionStartHandle)
                               ?QTextCursor::KeepAnchor:QTextCursor::MoveAnchor));

    if (isCursorMoved) {
        if (mEdit) {
            HbWidgetFeedback::triggered(mEdit, Hb::InstantDraggedOver);
        }
        if (mInteractionMode==Selection) {
            // Restart timer every time when a selection handle moved
            mWordSnapTimer.start(SNAP_DELAY, q);
        }
        mEdit->setTextCursor(cursor);
    }

    // Ensure that the hitPos is visible
    HbAbstractEditPrivate::d_ptr(mEdit)->ensurePositionVisible(hitPos);
    if (mMagnifierEnabled) {
        mMagnifier->showWithEffect();
    }
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
        q->hideHandles();
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

#if HB_DEBUG_PAINT_INFO
void HbSelectionControlPrivate::updateDebugPaintInfo()
{
    Q_Q(HbSelectionControl);
    mEdit->update();
    q->update();
}
#endif


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
        d->mMagnifierDelegate->setEditor(d->mEdit);


        // Set the background of magnifier to the background of HbLineEdit or HbTextEdit
        QLatin1String magnifierBackground("qtg_fr_lineedit_normal_c");
        if (!qobject_cast<HbLineEdit*>(d->mEdit)) {
            magnifierBackground = QLatin1String("qtg_fr_textedit_normal_c");

        }
        d->mMagnifier->setBackground(magnifierBackground);

        // find first top-level ancestor of d->mEdit
        for(d->mTopLevelAncestor = d->mEdit;
            d->mTopLevelAncestor->parentItem();
            d->mTopLevelAncestor = d->mTopLevelAncestor->parentItem()){

            // Workaround for Qt bug: QTBUG-13473
            // This line could be removed after Qt fixes this bug.
            d->mTopLevelAncestor->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

        };
    }
    return control;
}

void HbSelectionControl::detachEditor(HbAbstractEdit *edit)
{
    if(!edit || !edit->mainWindow()) {
        qWarning("HbSelectionControl: attempting to detach to null editor pointer!");
    }

    HbSelectionControl *control = globalSelectionControlHash()->value(edit->mainWindow());
    if (control) {
        control->detachEditor();
    }
}

void HbSelectionControl::detachEditor()
{
    Q_D(HbSelectionControl);
    d->mMagnifierDelegate->setEditor(0);
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
    // selection start handles might be hidden
    d->mSelectionStartTouchArea->show();
    d->mSelectionStartHandle->show();
}

void HbSelectionControl::scrollStarted()
{
    Q_D(HbSelectionControl);

    if (isVisible() && d->mEdit) {
        d->mScrollInProgress = true;
        // Reparent handle items to editor canvas on pan start
        d->reparentHandles(HbAbstractEditPrivate::d_ptr(d->mEdit)->canvas);
        d->mMagnifier->hideWithEffect();
        // Note: This call is not needed if it is guaranteed that this method
        // will be called before the scrolling started.
        updatePrimitives();
    }
}

void HbSelectionControl::scrollFinished()
{
    Q_D(HbSelectionControl);

    if (isVisible() && d->mEdit) {
        d->mScrollInProgress = false;
        updatePrimitives();
    }
}


void HbSelectionControl::timerEvent(QTimerEvent *event)
{
    Q_D(HbSelectionControl);

    if (event->timerId() == d->mWordSnapTimer.timerId()) {
        d->mWordSnapTimer.stop();
    } else  if (event->timerId() == d->mDelayedTapTimer.timerId()) {
        d->mDelayedTapTimer.stop();
        d->delayedTapFinished();
    }
}

void HbSelectionControl::polish( HbStyleParameters& params )
{
    Q_D(HbSelectionControl);

    if (isVisible()) {

        const QLatin1String KHandleMarginFromLine("handle-margin-from-line");
        const QLatin1String KMagnifierMarginFromLine("magnifier-margin-from-line");
        const QLatin1String KMagnifierLeftRightMarginFromHandle("magnifier-left-right-margin-from-handle");
        const QLatin1String KMagnifierMaxDescent("magnifier-max-descent");

        params.addParameter(KHandleMarginFromLine);
        params.addParameter(KMagnifierMarginFromLine);
        params.addParameter(KMagnifierLeftRightMarginFromHandle);
        params.addParameter(KMagnifierMaxDescent);

        HbWidget::polish(params);

        // Set size of handles
        QSizeF size = d->mSelectionStartHandle->preferredSize();
        d->mSelectionStartHandle->setSize(size);
        d->mSelectionEndHandle->setSize(size);

        // Set max y offset for hit test point
        // TODO: consider setting this value from css
        d->mMaxHitTestPointOffsetYFromLine = size.height();

        // Set size of touch areas
        size = d->mSelectionEndTouchArea->preferredSize();
        d->mSelectionEndTouchArea->resize(size);


        // Increase the height of the touch area of the start selection handle
        size.setHeight(size.height()*2-d->mSelectionStartHandle->size().height());
        d->mSelectionStartTouchArea->resize(size);



        // Set size of magnifier
        d->mMagnifier->resize(d->mMagnifier->preferredSize());

        if (params.value(KHandleMarginFromLine).isValid()) {
            d->mHandleMarginFromLine = params.value(KHandleMarginFromLine).toReal();
        }

        if (params.value(KMagnifierMarginFromLine).isValid()) {
            d->mMagnifierMarginFromLine = params.value(KMagnifierMarginFromLine).toReal();
        }
        if (params.value(KMagnifierLeftRightMarginFromHandle).isValid()) {
            d->mMagnifierLeftRightMarginFromHandle = params.value(KMagnifierLeftRightMarginFromHandle).toReal();
        }

        if (params.value(KMagnifierMaxDescent).isValid()) {
            d->mMagnifierMaxDescent = params.value(KMagnifierMaxDescent).toReal();
        }

        // Set the min/max magnifier touch offsets
        // TODO: consider setting this value from css
        d->mMagnifierMaxYOffsetFromTouchPoint = d->mMagnifierMarginFromLine*2;
        d->mMagnifierMinYOffsetFromTouchPoint = d->mMagnifierMarginFromLine/2;

        updatePrimitives();
    } else {
        HbWidget::polish(params);
    }
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
        if (d->mEdit) {
            // GestureFinshed is only delegated while the mDelayedTapTimer is active
            if (tap->state() != Qt::GestureFinished || d->mDelayedTapTimer.isActive()) {
                HbAbstractEditPrivate::d_ptr(d->mEdit)->gestureEvent(event,this);
            }
        }

        switch(tap->state()) {

        case Qt::GestureStarted:
            d->tapGestureStarted(tap);
            break;
        case Qt::GestureCanceled:
        case Qt::GestureFinished:
            d->tapGestureFinished();
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
                HbWidgetFeedback::triggered(this, Hb::InstantReleased);
            }
            break;
      case Qt::GestureCanceled:
            if (d->mEdit) {
                d->panGestureCanceled();
                HbWidgetFeedback::triggered(d->mEdit, Hb::InstantReleased);
            }
            break;
      default:
            break;
        }
    }
}

bool HbSelectionControl::eventFilter(QObject * watched, QEvent *event)
{
    Q_UNUSED(watched)

    // Filter gesture events and delegate to gestureEvent()
    if (event->type() == QEvent::Gesture || event->type() == QEvent::GestureOverride) {
        gestureEvent(static_cast<QGestureEvent*>(event));
        return true;
    }
    return false;
}


bool HbSelectionControl::event(QEvent *event)
{
    Q_D(HbSelectionControl);

    if (event->type() == HbEvent::DeviceProfileChanged && d->mEdit) {
        HbDeviceProfileChangedEvent* dpEvent = static_cast<HbDeviceProfileChangedEvent*>(event);
        if ( dpEvent->profile().alternateProfileName() == dpEvent->oldProfile().name() ) {
            d->initMagnifier();
            updatePrimitives();
        }
    }
    return HbWidget::event(event);
}


void HbSelectionControl::setMagnifierEnabled(bool enable)
{
    Q_D(HbSelectionControl);
    d->mMagnifierEnabled = enable;
}

bool HbSelectionControl::isMagnifierEnabled() const
{
    Q_D(const HbSelectionControl);
    return d->mMagnifierEnabled;
}

void HbSelectionControl::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)    
    Q_UNUSED(painter)

#if HB_DEBUG_PAINT_INFO
    Q_D(HbSelectionControl);
    painter->save();

    // draw mHitTestPoint
    painter->setPen(Qt::yellow);
    painter->setBrush(Qt::yellow);
    painter->drawEllipse(d->mHitTestPoint, 3,3);

    // draw mDocRectDebug
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(d->mDocRectDebug);

    // draw line representing mMagnifierMaxYOffsetFromTouchPoint
    painter->setPen(Qt::red);
    qreal magnifierMaxYOffsetLine = d->mTouchPoint.y()-d->mMagnifierMaxYOffsetFromTouchPoint;
    painter->drawLine(QPointF(0,magnifierMaxYOffsetLine),QPointF(500,magnifierMaxYOffsetLine));

    // draw line representing mMagnifierMinYOffsetFromTouchPoint
    painter->setPen(Qt::green);
    qreal magnifierMinYOffsetLine = d->mTouchPoint.y()-d->mMagnifierMinYOffsetFromTouchPoint;
    painter->drawLine(QPointF(0,magnifierMinYOffsetLine),QPointF(500,magnifierMinYOffsetLine));

    // draw line representing magnifierStartYOffsetLine
    painter->setPen(Qt::black);
    qreal magnifierStartYOffsetLine = d->mStartHandleMagnifierReferenceLine - d->mMagnifierMarginFromLine;
    painter->drawLine(QPointF(0,magnifierStartYOffsetLine),QPointF(500,magnifierStartYOffsetLine));

    // draw line representing magnifierStartYOffsetLine
    painter->setPen(Qt::magenta);
    qreal magnifierEndYOffsetLine = d->mEndHandleMagnifierReferenceLine - d->mMagnifierMarginFromLine;
    painter->drawLine(QPointF(0,magnifierEndYOffsetLine),QPointF(500,magnifierEndYOffsetLine));


    painter->drawRect(boundingRect());

    painter->restore();
#endif

}


void HbSelectionControl::updatePrimitives()
{
    Q_D(HbSelectionControl);

    if (isVisible() && d->polished && d->mEdit) {

        const bool hasSelection = d->mEdit->textCursor().hasSelection();

        // The interaction mode can be change only when there is no pan in progress
        if (!d->mIsPanActive) {
            d->mInteractionMode = (hasSelection?HbSelectionControlPrivate::Selection:
                                                HbSelectionControlPrivate::CursorPositioning);
        }

        const int cursorPos = d->mEdit->textCursor().position();
        int selStartPos = cursorPos;
        int selEndPos = cursorPos;
        d->mSelectionEndHandle->setIconName(QLatin1String("qtg_graf_editor_handle_finetune"));

        if (hasSelection) {
            selStartPos = qMin(d->mEdit->textCursor().anchor(),cursorPos);
            selEndPos = qMax(d->mEdit->textCursor().anchor(),cursorPos);
            d->mSelectionEndHandle->setIconName(QLatin1String("qtg_graf_editor_handle_end"));
        }

        d->updateHandle(cursorPos,selStartPos,Qt::AlignTop,d->mSelectionStartHandle,d->mSelectionStartTouchArea);
        d->updateHandle(cursorPos,selEndPos,Qt::AlignBottom,d->mSelectionEndHandle,d->mSelectionEndTouchArea);
        d->updateMagnifier();
    }
}
#include "moc_hbselectioncontrol_p.cpp"

