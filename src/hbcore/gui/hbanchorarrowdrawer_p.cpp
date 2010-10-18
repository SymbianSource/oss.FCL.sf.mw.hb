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



#include <hbanchor.h>
#include <hbanchorarrowdrawer_p.h>
#include <hbcolorscheme.h>
#include <hbevent.h>
#include <hblayoututils_p.h>
#include <hbanchorlayout.h>
#include <QBrush>
#include <QPainter>
#include <QPen>

#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
const int ARROW_HEAD_SIZE = 2;
const int LINE_WIDTH = 1;
const int SPACER_WIDTH = 40;
const QString BOX_COLOR = "qtc_view_separator_normal";
const QString VALID_COLOR = "qtc_default_main_pane_normal";
const QString INVALID_COLOR = "qtc_view_visited_normal";
#endif

HbAnchorArrowDrawer::HbAnchorArrowDrawer(HbAnchorLayout *layout, QGraphicsItem *parent)
    : HbWidgetBase(parent),
      mLayout(layout), mDrawOutlines(true), mDrawArrows(true), mDrawSpacers(true),
      mDrawMinLength(false), mDrawPrefLength(false), mDrawMaxLength(false)

{
#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
    updateColors();
#endif
}

HbAnchorArrowDrawer::~HbAnchorArrowDrawer()
{
}

void HbAnchorArrowDrawer::changeEvent(QEvent *event)
{
#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
    if (event->type() == HbEvent::ThemeChanged)
        updateColors();
#endif
    HbWidgetBase::changeEvent(event);
}

void HbAnchorArrowDrawer::updateColors()
{
#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
    mValidColor = HbColorScheme::color(VALID_COLOR);
    mInvalidColor = HbColorScheme::color(INVALID_COLOR);
    mBoxColor = HbColorScheme::color(BOX_COLOR);
#endif
}

void HbAnchorArrowDrawer::updateFocusItem(const QGraphicsItem *item)
{
#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
    mLayout = 0;
    if (item && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item);
        setGeometry(item->sceneBoundingRect());
        QGraphicsLayout *layout = widget->layout();
        if (layout) {
            HbAnchorLayout *anchorlayout = dynamic_cast<HbAnchorLayout *>(layout);
            if (anchorlayout) {
                mLayout = anchorlayout;
            }
        }
    }
#else
    Q_UNUSED(item);
#endif
}

void HbAnchorArrowDrawer::paint(
    QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
    if(!mLayout || (!mDrawOutlines && !mDrawArrows)) {
        return;
    }
    painter->save();

    QList<HbAnchor*> anchors = mLayout->effectiveAnchors();

    // Draw boxes round anchored child items
    if (mDrawOutlines) {
        QSet<QGraphicsLayoutItem*> anchoredChildren;
        foreach (const HbAnchor *anchor, anchors) {
            anchoredChildren << anchor->startItem();
            anchoredChildren << anchor->endItem();
        }
        painter->setPen(QPen(QBrush(mBoxColor), LINE_WIDTH));
        painter->setBrush(Qt::NoBrush);
        foreach (const QGraphicsLayoutItem *item, anchoredChildren) {
            painter->drawRect(item->geometry());
        }
    }

    if (mDrawSpacers) {
        painter->save();
        foreach (HbAnchor *anchor, anchors) {
            if (anchor->sizePolicy() == QSizePolicy::Fixed) {
                // Not a "spacer"
                continue;
            }
            paintAnchor( painter, anchor, false, Spacer );
        }
        painter->restore();
    }

    // Draw anchor lines
    if (mDrawArrows) {
        bool layoutMirrored = HbLayoutUtils::visualDirection(mLayout) == Qt::RightToLeft;
        foreach (HbAnchor *anchor, anchors) {

            // Ignore frame and toucharea primitives
            if (anchor->startItem()) {
                if (QGraphicsItem *gItem = anchor->startItem()->graphicsItem()) {
                    if (gItem->isWidget()) {
                        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(gItem);
                        QString itemText(widget->metaObject()->className());
                        if (itemText == "HbFrameItem" || itemText == "HbTouchArea") {
                            continue;
                        }
                    }
                }
            }

            bool prefLonger(anchor->preferredLength() > mLayout->anchorLength(anchor));
            if ( mDrawMaxLength ) {
                paintAnchor( painter, anchor, layoutMirrored, Maximum );
            }
            if ( mDrawPrefLength && prefLonger ) {
                paintAnchor( painter, anchor, layoutMirrored, Preferred );
            }
            paintAnchor( painter, anchor, layoutMirrored, Actual );
            if ( mDrawPrefLength && !prefLonger ) {
                paintAnchor( painter, anchor, layoutMirrored, Preferred );
            }
            if ( mDrawMinLength ) {
                paintAnchor( painter, anchor, layoutMirrored, Minimum );
            }

        } // End anchors foreach loop
    }
    painter->restore();
#else
    Q_UNUSED(painter);
#endif
}

void HbAnchorArrowDrawer::paintAnchor(
    QPainter *painter,
    HbAnchor *anchor,
    bool layoutMirrored,
    ArrowType type)
{
#if defined(HB_DEVELOPER) || defined(CSS_INSPECTOR)
    // Vars from anchor
    bool positiveDirection = anchor->direction() == HbAnchor::Positive;
    Hb::Edge startEdge = anchor->startEdge();
    Hb::Edge endEdge = anchor->endEdge();
    qreal anchorLength;
    switch (type) {
        case Minimum:
            anchorLength = anchor->minimumLength();
            break;
        case Preferred:
            anchorLength = anchor->preferredLength();
            break;
        case Maximum:
            anchorLength = anchor->maximumLength();
            break;
        case Actual:
        case Spacer:
        default:
            anchorLength = mLayout->anchorLength(anchor);
            break;
    }

    // if edge is connected to parent on same edge, and if the gap is zero,
    // then don't show an arrow head
    if ( (anchor->endItem() == mLayout || anchor->startItem() == mLayout) &&
          startEdge == endEdge && anchorLength == 0) {
        return;
    }

    // Mirroring
    if (layoutMirrored) {
        if (startEdge == Hb::LeftEdge) {
            startEdge = Hb::RightEdge;
        } else if (startEdge == Hb::RightEdge) {
            startEdge = Hb::LeftEdge;
        }

        if (endEdge == Hb::LeftEdge) {
            endEdge = Hb::RightEdge;
        } else if (endEdge == Hb::RightEdge) {
            endEdge = Hb::LeftEdge;
        }
    }

    QRectF startRect = anchor->startItem()->geometry();
    QRectF endRect = anchor->endItem()->geometry();
    Hb::Edge arrowType = Hb::RightEdge;
    QPointF startPt, start2Pt, endPt, end2Pt;

    bool horizontal(true);
    // Work out the arrow line start point
    switch (startEdge) {
        case Hb::LeftEdge: startPt.rx() = startRect.left(); break;
        case Hb::RightEdge: startPt.rx() = startRect.right(); break;
        case Hb::CenterHEdge: startPt.rx() = startRect.center().x(); break;
        case Hb::TopEdge: startPt.ry() = startRect.top(); horizontal = false; break;
        case Hb::BottomEdge: startPt.ry() = startRect.bottom(); horizontal = false; break;
        case Hb::CenterVEdge: startPt.ry() = startRect.center().y(); horizontal = false; break;
        default: break;
    }
    start2Pt = startPt;

    if ( horizontal ) {
        // Set arrow end point and arrow direction
        positiveDirection ^= layoutMirrored; // XOR with layout direction
        if (positiveDirection) {
            arrowType = Hb::RightEdge;
            endPt.rx() = startPt.x() + anchorLength;
        } else {
            arrowType = Hb::LeftEdge;
            endPt.rx() = startPt.x() - anchorLength;
        }

        // Set vertical centering and staggered line point
        qreal maxTop = qMax(startRect.top(), endRect.top());
        qreal minBottom = qMin(startRect.bottom(), endRect.bottom());
        if (maxTop < minBottom) {
            startPt.ry() = (maxTop + minBottom) / 2;
            start2Pt.ry() = startPt.y();
        } else {
            const bool startAboveEnd = startRect.top() > endRect.top();
            startPt.ry() = startAboveEnd ? endRect.bottom() : endRect.top();
            start2Pt.ry() = startAboveEnd ? startRect.top() : startRect.bottom();
        }
        endPt.ry() = startPt.y();
        end2Pt.ry() = startPt.y();

        // Set end staggered point
        if (endEdge == Hb::LeftEdge) {
            end2Pt.rx() = endRect.left();
        } else if (endEdge == Hb::RightEdge) {
            end2Pt.rx() = endRect.right();
        } else {
            end2Pt.rx() = endRect.center().x();
        }
    } else { // vertical
        // Set arrow end point and arrow direction
        if (positiveDirection) {
            endPt.ry() = startPt.y() + anchorLength;
            arrowType = Hb::BottomEdge;
        } else {
            endPt.ry() = startPt.y() - anchorLength;
            arrowType = Hb::TopEdge;
        }

        // Set horizontal centering and staggered line point
        qreal maxLeft = qMax(startRect.left(), endRect.left());
        qreal minRight = qMin(startRect.right(), endRect.right());
        if (maxLeft < minRight) {
            startPt.rx() = (maxLeft + minRight) / 2;
            start2Pt.rx() = startPt.x();
        } else {
            bool startLeftOfEnd = startRect.left() > endRect.left();
            startPt.rx() = startLeftOfEnd ? endRect.right() : endRect.left();
            start2Pt.rx() = startLeftOfEnd ? startRect.left() : startRect.right();
        }
        endPt.rx() = startPt.x();
        end2Pt.rx() = startPt.x();

        // Set end staggered point
        if (endEdge == Hb::TopEdge) {
            end2Pt.ry() = endRect.top();
        } else if (endEdge == Hb::BottomEdge) {
            end2Pt.ry() = endRect.bottom();
        } else {
            end2Pt.ry() = endRect.center().y();
        }
    }

    if ( type == Spacer ) {
        QRectF layoutRect( mLayout->geometry() );
        QRectF drawArea;
        if ( horizontal ) {
            drawArea.setLeft( qMin(startPt.x(), end2Pt.x() ) );
            drawArea.setRight( qMax(startPt.x(), end2Pt.x() ) );
            drawArea.setTop( qMax(qreal(startPt.y() - SPACER_WIDTH/2.0), layoutRect.top() ) );
            drawArea.setBottom( qMin(qreal(startPt.y() + SPACER_WIDTH/2.0), layoutRect.bottom() ) );
        } else {
            drawArea.setLeft( qMax(qreal(startPt.x() - SPACER_WIDTH/2.0), layoutRect.left() ) );
            drawArea.setRight( qMin(qreal(startPt.x() + SPACER_WIDTH/2.0), layoutRect.right() ) );
            drawArea.setTop( qMin(startPt.y(), end2Pt.y() ) );
            drawArea.setBottom( qMax(startPt.y(), end2Pt.y() ) );
        }
        painter->setOpacity(0.3); // so that we can see overlapping spacers
        painter->fillRect(drawArea, QBrush(mInvalidColor, Qt::SolidPattern));
    } else  {

        // Start painting block
        QColor arrowColor = mLayout->isValid() ? mValidColor : mInvalidColor;
        switch (type) {
            case Minimum: arrowColor = Qt::blue; break; //krazy:exclude=qenums
            case Preferred: arrowColor = Qt::green; break; //krazy:exclude=qenums
            case Maximum: arrowColor = Qt::red; break; //krazy:exclude=qenums
            case Actual:
            case Spacer:
            default: break;
        }

        QColor centerColor = Qt::yellow; //krazy:exclude=qenums

        painter->setPen(QPen(arrowColor, LINE_WIDTH, Qt::DashLine));
        //painter->drawLine(start2Pt, startPt);

        painter->setPen(QPen(arrowColor, LINE_WIDTH, Qt::SolidLine));
        painter->drawLine(startPt, endPt);

        if (startEdge == Hb::CenterHEdge || startEdge == Hb::CenterVEdge) {
            painter->setBrush(centerColor);
        } else {
            painter->setBrush(arrowColor);
        }

        // Only draw the start box if the anchor is long enough to show 3 times the head size
        // (head, stalk, and tail) otherwise it turns into a mush,
        // so the best thing is to show the triangle which at least shows the direction
        if (anchorLength > ARROW_HEAD_SIZE*3) {
            painter->drawRect(QRectF(
                start2Pt.x() - ARROW_HEAD_SIZE,
                start2Pt.y() - ARROW_HEAD_SIZE,
                ARROW_HEAD_SIZE*2,
                ARROW_HEAD_SIZE*2));
        }

        // Draw arrow head
        QPointF points[3] = {
            QPointF(0.0, 0.0),
            QPointF(0.0, 0.0),
            QPointF(endPt.x(), endPt.y())
        };
        if (arrowType == Hb::RightEdge) {
            points[0] = QPointF(endPt.x()-ARROW_HEAD_SIZE*2, endPt.y()-ARROW_HEAD_SIZE);
            points[1] = QPointF(endPt.x()-ARROW_HEAD_SIZE*2, endPt.y()+ARROW_HEAD_SIZE);
        } else if (arrowType == Hb::LeftEdge) {
            points[0] = QPointF(endPt.x()+ARROW_HEAD_SIZE*2, endPt.y()-ARROW_HEAD_SIZE);
            points[1] = QPointF(endPt.x()+ARROW_HEAD_SIZE*2, endPt.y()+ARROW_HEAD_SIZE);
        } else if (arrowType == Hb::TopEdge) {
            points[0] = QPointF(endPt.x()-ARROW_HEAD_SIZE, endPt.y()+ARROW_HEAD_SIZE*2);
            points[1] = QPointF(endPt.x()+ARROW_HEAD_SIZE, endPt.y()+ARROW_HEAD_SIZE*2);
        } else { // arrowType == Hb::BottomEdge
            points[0] = QPointF(endPt.x()-ARROW_HEAD_SIZE, endPt.y()-ARROW_HEAD_SIZE*2);
            points[1] = QPointF(endPt.x()+ARROW_HEAD_SIZE, endPt.y()-ARROW_HEAD_SIZE*2);
        }
        painter->drawPolygon(points, 3);

        if ( type == Actual ) {
            // Draw invalid difference
            if (endPt != end2Pt) {
                painter->setPen(QPen(mInvalidColor, LINE_WIDTH, Qt::DashLine));
                painter->drawLine(endPt, end2Pt);
            }
        }
    }
#else
    Q_UNUSED(painter);
    Q_UNUSED(anchor);
    Q_UNUSED(layoutMirrored);
    Q_UNUSED(type);
#endif
}
