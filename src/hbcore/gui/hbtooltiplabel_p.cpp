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

#include "hbtooltiplabel_p.h"
#include "hbtooltiplabel_p_p.h"
#include "hbtooltip.h"
#include "hbtextitem.h"
#include "hbnamespace_p.h"
#include "hbfontspec.h"
#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

#include <QGraphicsWidget>
#include <QPointer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QStyle> // krazy:exclude=qclasses
#include <QGraphicsLinearLayout>




namespace {
    static const int TOOLTIP_DELAY = 150;
    static const int TOOLTIP_HIDE_DELAY = 300;
}

HbToolTipLabelPrivate::HbToolTipLabelPrivate():
        label(0), aboutToHide(false),
        mHorizontalMargin(0.0), mVerticalMargin(0.0), mPolishLayoutRequest(false)
{
}

void HbToolTipLabelPrivate::init()
{
    Q_Q(HbToolTipLabel);
    q->setBackgroundItem(HbStyle::P_ToolTip_background);

    q->setFocusPolicy(Qt::NoFocus);
    q->setTimeout(HbPopup::NoTimeout);
    q->setBackgroundFaded(false);
    q->setDismissPolicy(HbPopup::TapAnywhere);
    q->setModal(false);
}

void HbToolTipLabelPrivate::addPopupEffects()
{
#ifdef HB_EFFECTS
    Q_Q(HbToolTipLabel);
    hasEffects = HbEffectInternal::add(q,"tooltip_appear", "appear");
    if (hasEffects) {
        //We load the disappear effect only if appear effect was also loaded
        hasEffects = HbEffectInternal::add(q,"tooltip_disappear", "disappear");
    }
#endif
}

/*!
  Checks if there is an item under scenePos that has tooltip. If it finds an item it sends an
  QGraphicsSceneHelpEvent event to it. If the event is not handled by the item it uses default implementation
  to display tooltip.
*/
void HbToolTipLabelPrivate::checkForToolTip(const QPointF& scenePos, const QPoint& screenPos)
{
    Q_Q(HbToolTipLabel);

    if (q->scene()) {
        // Find pressed item having tooltip
        QGraphicsItem *toolTipItem = 0;

        foreach(QGraphicsItem *item, q->scene()->items(scenePos)) {
            if (toolTipBlockItem(item)) {
                break;
            }
            if (!item->toolTip().isEmpty()) {
                toolTipItem = item;
                break;
            }
        }

        if (toolTipItem) {
            QGraphicsSceneHelpEvent helpEvent(QEvent::GraphicsSceneHelp);
            helpEvent.setScenePos(scenePos);
            helpEvent.setScreenPos(screenPos);
            helpEvent.setAccepted(false);

            static_cast<GraphicsItem*>(toolTipItem)->sceneEvent(&helpEvent);

            // if event is not processed display tooltip
            if (!helpEvent.isAccepted()) {
                // Note: Intentionally public API is used to get same behavior
                HbToolTip::showText(toolTipItem->toolTip(),toolTipItem,toolTipItem->boundingRect());
            }
        }
    }
}

void HbToolTipLabelPrivate::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    Q_Q(HbToolTipLabel);
    pressPointScreenPos = mouseEvent->screenPos();
    pressPointScenePos = mouseEvent->scenePos();

    tooltipTimer.start(TOOLTIP_DELAY, q);
    hideTimer.stop();
}

void HbToolTipLabelPrivate::mouseReleaseEvent (QGraphicsSceneMouseEvent* mouseEvent)
{
    Q_UNUSED(mouseEvent)
    tooltipTimer.stop();
    hideTimer.stop();
}

void HbToolTipLabelPrivate::mouseMoveEvent (QGraphicsSceneMouseEvent* mouseEvent)
{
    Q_Q(HbToolTipLabel);

    if (aboutToHide) {
        checkForToolTip(mouseEvent->scenePos(),mouseEvent->screenPos());
    } else {
        if (!rect.isNull() && !rect.contains(static_cast<QGraphicsSceneMouseEvent*>(mouseEvent)->scenePos())) {
            q->hideText();
        }
    }
}

void HbToolTipLabelPrivate::showText(QGraphicsItem *item, Qt::Alignment preferredAlignment)
{
    Q_Q(HbToolTipLabel);    
    QRectF itemSceneRect = item->sceneBoundingRect();    
    QRectF toolTipBoundingRect = QRectF(qreal(0.0), qreal(0.0),
                                        q->preferredSize().width(), q->preferredSize().height());
    QRectF screenRect(QPointF(qreal(0.0), qreal(0.0)), HbDeviceProfile::profile(q).logicalSize());

    Qt::Alignment currentAlignment = (preferredAlignment & Qt::AlignHorizontal_Mask)?
                                     QStyle::visualAlignment(q->layoutDirection(), preferredAlignment ): // krazy:exclude=qclasses
                                     preferredAlignment;

    bool matchingFinished = false;

    if (currentAlignment & Qt::AlignTop) {        
        if ((itemSceneRect.y() - mVerticalMargin - toolTipBoundingRect.height()) >= 0) {
            q->setPreferredPos(QPointF(itemSceneRect.x() + item->boundingRect().width()/2,
                                       itemSceneRect.y() - mVerticalMargin),
                               HbPopup::BottomEdgeCenter);
            matchingFinished = true;
        }

    }
    if (currentAlignment & Qt::AlignRight || !matchingFinished) {        
        if ((itemSceneRect.x() + item->boundingRect().width() +
             mHorizontalMargin + toolTipBoundingRect.width()) <= screenRect.width()) {
            q->setPreferredPos(QPointF(itemSceneRect.x() + item->boundingRect().width() + mHorizontalMargin,
                                       itemSceneRect.y() +  item->boundingRect().height()/2),
                               HbPopup::LeftEdgeCenter);
            matchingFinished = true;
        }
    }
    if (currentAlignment & Qt::AlignLeft || !matchingFinished) {        
        if ((itemSceneRect.x() - mHorizontalMargin - toolTipBoundingRect.width()) >= 0) {
            q->setPreferredPos(QPointF(itemSceneRect.x() - mHorizontalMargin,
                                       itemSceneRect.y() +  item->boundingRect().height()/2),
                               HbPopup::RightEdgeCenter);
            matchingFinished = true;
        }
    }
    //Using AlignTop as default
    if (!matchingFinished) {
        q->setPreferredPos(QPointF(itemSceneRect.x() + item->boundingRect().width()/2,
                                   itemSceneRect.y() - mVerticalMargin),
                           HbPopup::BottomEdgeCenter);
    }

    q->show();
}

/*!
  Returns true if item is a tooltip blocking item.
  For example HbPopup background item is tooltip blocking for modal popups.
*/
bool HbToolTipLabelPrivate::toolTipBlockItem(QGraphicsItem *item)
{
    if (item->type() == HbPrivate::ItemType_FadeItem) {
        return true;
    } else if (item->type() == HbPrivate::ItemType_PopupBackGround) {
        return static_cast<HbPopupBackGround *>(item)->isModal();
    }
    return false;
}

/*!
    \class HbToolTipLabel
    \brief HbToolTipLabel is a convenience widget that displays a small message box.
    Compared to traditional popup, a tooltip does not dim the background.
    It disappears automatically after 3 seconds.
*/
HbToolTipLabel::HbToolTipLabel(QGraphicsWidget *parent) :
    HbPopup(*new HbToolTipLabelPrivate, parent)
{
    Q_D(HbToolTipLabel);
    d->q_ptr = this;
    d->init();
}

HbToolTipLabel::~HbToolTipLabel()
{
}

/*!
  \primitives
  \primitive{background} HbFrameItem representing the tooltip background frame.
  */

/*!
  This method can be used to eventHook tool tip triggering and dismissal logic to a scene event
  flow.
 */
void HbToolTipLabel::eventHook(QEvent *event)
{
    Q_D(HbToolTipLabel);

    if (isVisible()) {
        switch (event->type()) {
                case QEvent::WindowActivate:
                case QEvent::WindowDeactivate:
                case QEvent::GraphicsSceneMousePress:
                case QEvent::GraphicsSceneMouseRelease:
                case QEvent::GraphicsSceneMouseDoubleClick:
                case QEvent::GraphicsSceneHoverEnter:
                case QEvent::GraphicsSceneHoverLeave:
                case QEvent::GraphicsSceneHoverMove:
                case QEvent::FocusIn:
                case QEvent::FocusOut:

                hideTextImmediately();
                break;
            case QEvent::GraphicsSceneMouseMove:
                d->mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
                break;
            default:
                break;
        }
    } else if (event->type() == QEvent::GraphicsSceneMousePress){
        d->mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
    } else if (event->type() == QEvent::GraphicsSceneMouseRelease){
        d->mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
    }
}


/*!
    Returns the text of the tooltip.

    \sa setText()
*/
QString HbToolTipLabel::text() const
{
    Q_D(const HbToolTipLabel);
    if (!d->label) {
        return QString();
    } else {
        return d->label->text();
    }
}

/*!
    Sets the text of the tooltip.

    \sa text()
*/
void HbToolTipLabel::setText(const QString &newText)
{
    Q_D(HbToolTipLabel);
    if (!d->label) {
        d->label = new HbTextItem(this);
        HbStyle::setItemName(d->label, "label");
        repolish();
    }
    d->label->setText(newText);
}

/*!
    If you specify a non-empty rect the tip will be hidden as soon
    as you move your cursor out of this area.

    The \a rect is in the coordinates of the scene
*/
void HbToolTipLabel::setRect(const QRectF& rect)
{
    Q_D(HbToolTipLabel);
    d->rect = rect;
}


/*!
    Displays tooltip using preferredAlignment regarding item.

    \sa hideText()
*/
void HbToolTipLabel::showText(QGraphicsItem *item, Qt::Alignment preferredAlignment)
{
    Q_D(HbToolTipLabel);
    d->aboutToHide = false;
    d->tooltipTimer.stop();
    d->hideTimer.stop();

    d->showText(item, preferredAlignment);
    d->mPreferredAlignment = preferredAlignment;
    d->mItem = item;
}

/*!
    Hides tooltip.

    \sa hideText()
*/
void HbToolTipLabel::hideText()
{
    Q_D(HbToolTipLabel);
    d->aboutToHide = true;
    d->hideTimer.start(TOOLTIP_HIDE_DELAY, this);
}


void HbToolTipLabel::hideTextImmediately()
{
    Q_D(HbToolTipLabel);
    d->aboutToHide = false;
    d->hideTimer.stop();
    d->tooltipTimer.stop();
    close();
}

/*!
    Timer event to show tooltip or hide the tool tip immediately.
*/
void HbToolTipLabel::timerEvent(QTimerEvent *event)
{
    Q_D(HbToolTipLabel);
    if (event->timerId() == d->hideTimer.timerId()){
        hideTextImmediately();
    } else if (event->timerId() == d->tooltipTimer.timerId()) {        
        d->tooltipTimer.stop();
        d->checkForToolTip(d->pressPointScenePos,d->pressPointScreenPos);
    }
}

/*!
    \reimp
*/
bool HbToolTipLabel::event(QEvent *event)
{
    Q_D(HbToolTipLabel);
    if (event->type() == QEvent::FontChange) {
        if (d->label) {
            d->label->setFont(font());
        }
    } else if (event->type() == QEvent::LayoutRequest) {
        if (isVisible() && d->mPolishLayoutRequest) {
            d->mPolishLayoutRequest = false;
            d->showText(d->mItem, d->mPreferredAlignment);
        }
    }
    return HbPopup::event(event);
}

/*!
    \reimp
*/
void HbToolTipLabel::polish(HbStyleParameters &params)
{
    Q_D(HbToolTipLabel);
    const QString HorizontalMargin = "horizontal-margin";
    const QString VerticalMargin = "vertical-margin";
    const QString ScreenMargin = "screen-margin";

    params.addParameter(HorizontalMargin);
    params.addParameter(VerticalMargin);
    params.addParameter(ScreenMargin);

    HbPopup::polish(params);

    if (params.value(HorizontalMargin).isValid()) {
        d->mHorizontalMargin = params.value(HorizontalMargin).toReal();
    }
    if (params.value(VerticalMargin).isValid()) {
        d->mVerticalMargin = params.value(VerticalMargin).toReal();
    }
    if (params.value(ScreenMargin).isValid()) {
        d->mScreenMargin = params.value(ScreenMargin).toReal();
    }
    d->mPolishLayoutRequest = true;
}

