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



#include <hbanchor_p.h>
#include <hbanchorarrowdrawer_p.h>
#include <hbcolorscheme.h>
#include <hbcssinspector_p.h>
#include <hbevent.h>
#include <hblayoututils_p.h>
#include <hbmeshlayout_p.h>
#include <hbmeshlayoutdebug_p.h>
#include <QBrush>
#include <QPainter>
#include <QPen>

const int ARROW_HEAD_SIZE = 2;
const int LINE_WIDTH = 1;
const QString BOX_COLOR = "qtc_view_separator_normal";
const QString VALID_COLOR = "qtc_default_main_pane_normal";
const QString INVALID_COLOR = "qtc_view_visited_normal";

HbAnchorArrowDrawer::HbAnchorArrowDrawer(HbMeshLayout* mesh, QGraphicsItem *parent)
    : HbWidgetBase(parent), mLayout(mesh), mDrawOutlines(true), mDrawArrows(true), mDrawSpacers(true)
{
#ifdef BUILD_HB_INTERNAL
    updateColors();
#endif
}

HbAnchorArrowDrawer::~HbAnchorArrowDrawer()
{
}

void HbAnchorArrowDrawer::changeEvent(QEvent *event)
{
#if defined(BUILD_HB_INTERNAL) || defined(CSS_INSPECTOR)
    if (event->type() == HbEvent::ThemeChanged)
        updateColors();
#endif
    HbWidgetBase::changeEvent(event);
}

void HbAnchorArrowDrawer::updateColors()
{
#if defined(BUILD_HB_INTERNAL) || defined(CSS_INSPECTOR)
    mValidColor = HbColorScheme::color(VALID_COLOR);
    mInvalidColor = HbColorScheme::color(INVALID_COLOR);
    mBoxColor = HbColorScheme::color(BOX_COLOR);
#endif
}

void HbAnchorArrowDrawer::updateFocusItem(const QGraphicsItem *item)
{
#if defined(BUILD_HB_INTERNAL) || defined(CSS_INSPECTOR)
    mLayout = 0;
    if (item && item->isWidget()) {
        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget*>(item);
        setGeometry(item->sceneBoundingRect());
        QGraphicsLayout *layout = widget->layout();
        if (layout) {
            HbMeshLayout *mesh = dynamic_cast<HbMeshLayout *>(layout);
            if (mesh) {
                mLayout = mesh;
            }
        }
    }
#else
    Q_UNUSED(item);
#endif
}

void HbAnchorArrowDrawer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

#if defined(BUILD_HB_INTERNAL) || defined(CSS_INSPECTOR)
    if(!mLayout || (!mDrawOutlines && !mDrawArrows)) {
        return;
    }
    painter->save();
    
    QList<HbAnchor> anchors = HbMeshLayoutDebug::getAnchors(mLayout);

    // Draw boxes round anchored child items
    if (mDrawOutlines) {
        QList<QGraphicsLayoutItem*> anchoredChildren;
        foreach (const HbAnchor &anchor, anchors) {
            if (!anchoredChildren.contains(anchor.mStartItem)) {
                anchoredChildren.append(anchor.mStartItem);
            }
            if (!anchoredChildren.contains(anchor.mEndItem)) {
                anchoredChildren.append(anchor.mEndItem);
            }
        }
        painter->setPen(QPen(QBrush(mBoxColor), LINE_WIDTH));
        painter->setBrush(Qt::NoBrush);
        foreach (const QGraphicsLayoutItem *item, anchoredChildren) {
            painter->drawRect(item->geometry());
        }
    }

    if (mDrawSpacers) {
        painter->save();
        for (int li=0 ; li<mLayout->count(); li++) {
            QGraphicsLayoutItem *layoutItem = mLayout->itemAt(li);
            //if (!layoutItem->graphicsItem()) {
            QRectF rectArea = layoutItem->geometry();
            if (rectArea.width() == 0 || rectArea.height() == 0) {
                if (rectArea.height() == 0 && rectArea.width() > 0) {
                    rectArea.setHeight(contentsRect().height());
                    bool overridden = false;
                    // find all the horizontal anchors
                    foreach(HbAnchor anchor, anchors) {
                        if (anchor.mStartEdge == Hb::LeftEdge || anchor.mStartEdge == Hb::RightEdge ||anchor.mStartEdge == Hb::CenterHEdge) {
                            QGraphicsLayoutItem *other = 0;
                            if (anchor.mStartItem == layoutItem && anchor.mEndItem != mLayout) {
                                other = anchor.mEndItem;
                            } else if (anchor.mEndItem == layoutItem && anchor.mStartItem != mLayout) {
                                other = anchor.mStartItem;
                            }
                            if (other) {
                                if (overridden) {
                                    rectArea.setTop(qMin(rectArea.top(), other->geometry().top()));
                                    rectArea.setBottom(qMax(rectArea.bottom(), other->geometry().bottom()));
                                    break;
                                } else { 
                                    rectArea.setTop(other->geometry().top());
                                    rectArea.setHeight(other->geometry().height());
                                    overridden = true;  
                                }
                            }
                        }
                    }
                 
                }
            
                if (rectArea.width() == 0 && rectArea.height() > 0) {
                    rectArea.setWidth(contentsRect().width());
                    bool overridden = false;
                    // find all the vertical anchors
                    foreach(HbAnchor anchor, anchors) {
                        if (anchor.mStartEdge == Hb::TopEdge || anchor.mStartEdge == Hb::BottomEdge ||anchor.mStartEdge == Hb::CenterVEdge) {
                            QGraphicsLayoutItem *other = 0;
                            if (anchor.mStartItem == layoutItem && anchor.mEndItem != mLayout) {
                                other = anchor.mEndItem;
                            } else if (anchor.mEndItem == layoutItem && anchor.mStartItem != mLayout) {
                                other = anchor.mStartItem;
                            }
                            if (other) {
                                if (overridden) {
                                    rectArea.setLeft(qMin(rectArea.left(), other->geometry().left()));
                                    rectArea.setRight(qMax(rectArea.right(), other->geometry().right()));
                                    break;
                                } else {
                                    rectArea.setLeft(other->geometry().left());
                                    rectArea.setWidth(other->geometry().width());
                                    overridden = true; 
                                }
                            }
                        }
                    }
                }

                painter->fillRect(rectArea, QBrush(mInvalidColor, Qt::BDiagPattern));
            }
        }
        painter->restore();
    } // End spacers


    // Draw anchor lines
    if (mDrawArrows) {
        Qt::LayoutDirection dir = HbLayoutUtils::visualDirection(mLayout);
        for (int i=0; i<anchors.count(); i++) {
            Hb::Edge arrowType = Hb::RightEdge;
            QPointF start, start2, end, end2;
            HbAnchor anchor(anchors.at(i));

            // Ignore some primitives
            if (anchor.mStartItem) {
                if (QGraphicsItem *asGraphicsItem = anchor.mStartItem->graphicsItem()) {
                    if (asGraphicsItem->isWidget()) {
                        const QGraphicsWidget *widget = static_cast<const QGraphicsWidget *>(asGraphicsItem);
                        QString itemText(widget->metaObject()->className());
                        if (itemText == "HbFrameItem"
                            || itemText == "HbTouchArea") {
                            continue;
                        }
                    }
                }
            }
            // if edge is connected to parent on same edge, and if the gap is zero, then don't show an arrow head
            if(anchor.mEndItem->isLayout() 
                && anchor.mStartEdge == anchor.mEndEdge
                && anchor.mValue == 0) {
                    continue;
            }
            // Mirroring
            if (dir == Qt::RightToLeft) {
                if (anchor.mStartEdge == Hb::LeftEdge) {
                    anchor.mStartEdge = Hb::RightEdge;
                    anchor.mValue = -(anchor.mValue);
                } else if (anchor.mStartEdge == Hb::RightEdge) {
                    anchor.mStartEdge = Hb::LeftEdge;
                    anchor.mValue = -(anchor.mValue);
                } else if (anchor.mStartEdge == Hb::CenterHEdge) {
                    anchor.mValue = -(anchor.mValue);
                }

                if (anchor.mEndEdge == Hb::LeftEdge) {
                    anchor.mEndEdge = Hb::RightEdge;
                } else if (anchor.mEndEdge == Hb::RightEdge) {
                    anchor.mEndEdge = Hb::LeftEdge;
                }
            }

            QRectF startRect = anchor.mStartItem->geometry();
            QRectF endRect = anchor.mEndItem->geometry();

            // Fix non-pinned spacers issue
            if (startRect.left() == 0 && startRect.width() == 0) {
                startRect.adjust(endRect.left(), 0, endRect.left() + endRect.width(), 0);
            } else if (startRect.top() == 0 && startRect.height() == 0) {
                startRect.adjust(0, endRect.top(), 0, endRect.top() + endRect.height());
            }
            if (endRect.left() == 0 && endRect.width() == 0) {
                endRect.adjust(startRect.left(), 0, startRect.left() + startRect.width(), 0);
            } else if (endRect.top() == 0 && endRect.height() == 0) {
                endRect.adjust(0, startRect.top(), 0, startRect.top() + startRect.height());
            }

            // Work out the arrow line start point
            switch (anchor.mStartEdge) {
                case Hb::LeftEdge: start.rx() = startRect.left(); break;
                case Hb::RightEdge: start.rx() = startRect.right(); break;
                case Hb::CenterHEdge: start.rx() = startRect.center().x(); break;
                case Hb::TopEdge: start.ry() = startRect.top(); break;
                case Hb::BottomEdge: start.ry() = startRect.bottom(); break;
                case Hb::CenterVEdge: start.ry() = startRect.center().y(); break;
            }
            start2 = start;

            switch (anchor.mStartEdge) {
                case Hb::LeftEdge:
                case Hb::RightEdge:
                case Hb::CenterHEdge:
                {
                    // Set arrow end point
                    end.rx() = start.x() + anchor.mValue;

                    // Set arrow direction
                    arrowType = anchor.mValue < 0
                        ? Hb::LeftEdge
                        : Hb::RightEdge;

                    // Set vertical centering and staggered line point
                    qreal maxTop = qMax(startRect.top(), endRect.top());
                    qreal minBottom = qMin(startRect.bottom(), endRect.bottom());
                    if (maxTop < minBottom) {
                        start.ry() = (maxTop + minBottom) / 2;
                        start2.ry() = start.y();
                    } else {
                        const bool startAboveEnd = startRect.top() > endRect.top();
                        start.ry() = startAboveEnd ? endRect.bottom() : endRect.top();
                        start2.ry() = startAboveEnd ? startRect.top() : startRect.bottom();
                    }
                    end.ry() = start.y();
                    end2.ry() = start.y();

                    // Set end staggered point
                    if (anchor.mEndEdge == Hb::LeftEdge) {
                        end2.rx() = endRect.left();
                    } else if (anchor.mEndEdge == Hb::RightEdge) {
                        end2.rx() = endRect.right();
                    } else { 
                        end2.rx() = endRect.center().x();
                    }
                }
                break;

                case Hb::TopEdge:
                case Hb::BottomEdge:
                case Hb::CenterVEdge:
                {
                    // Set arrow end point
                    end.ry() = start.y() + anchor.mValue;

                    // Set arrow direction
                    arrowType = anchor.mValue < 0
                        ? Hb::TopEdge
                        : Hb::BottomEdge;

                    // Set horizontal centering and staggered line point
                    qreal maxLeft = qMax(startRect.left(), endRect.left());
                    qreal minRight = qMin(startRect.right(), endRect.right());
                    if (maxLeft < minRight) {
                        start.rx() = (maxLeft + minRight) / 2;
                        start2.rx() = start.x();
                    } else {
                        bool startLeftOfEnd = startRect.left() > endRect.left();
                        start.rx() = startLeftOfEnd ? endRect.right() : endRect.left();
                        start2.rx() = startLeftOfEnd ? startRect.left() : startRect.right();
                    }
                    end.rx() = start.x();
                    end2.rx() = start.x();

                    // Set end staggered point
                    if (anchor.mEndEdge == Hb::TopEdge) {
                        end2.ry() = endRect.top();
                    } else if (anchor.mEndEdge == Hb::BottomEdge) {
                        end2.ry() = endRect.bottom();
                    } else {
                        end2.ry() = endRect.center().y();
                    }
                }
                break;
            }

            // Start painting block
            QPen myPen;
            QColor arrowColor = mLayout->isValid()
                ? mValidColor
                : mInvalidColor;
            QColor centerColor = Qt::yellow;

            myPen.setWidth(LINE_WIDTH);
            myPen.setColor(arrowColor);
            myPen.setStyle(Qt::DashLine);
            painter->setPen(myPen);
            painter->setBrush(arrowColor);
            painter->drawLine(start2, start);

            myPen.setStyle(Qt::SolidLine);
            painter->setPen(myPen);
            painter->drawLine(start, end);

            if (anchor.mStartEdge == Hb::CenterHEdge || anchor.mStartEdge == Hb::CenterVEdge) {
                painter->setBrush(centerColor);
            }

            // Only draw the start box if the anchor is long enough to show 3 times the head size 
            // (head, stalk, and tail) otherwise it turns into a mush, 
            // so the best thing is to show the triangle which at least shows the direction
            if (qAbs(anchor.mValue) > ARROW_HEAD_SIZE*3) {
                painter->drawRect(QRectF(
                    start2.x() - ARROW_HEAD_SIZE,
                    start2.y() - ARROW_HEAD_SIZE,
                    ARROW_HEAD_SIZE*2,
                    ARROW_HEAD_SIZE*2));
            }

            // Draw arrow head
            QPointF points[3] = {
                QPointF(0.0, 0.0),
                QPointF(0.0, 0.0),
                QPointF(end.x(), end.y())
            };
            if (arrowType == Hb::RightEdge) {
                points[0] = QPointF(end.x()-ARROW_HEAD_SIZE*2, end.y()-ARROW_HEAD_SIZE);
                points[1] = QPointF(end.x()-ARROW_HEAD_SIZE*2, end.y()+ARROW_HEAD_SIZE);
            } else if (arrowType == Hb::LeftEdge) {
                points[0] = QPointF(end.x()+ARROW_HEAD_SIZE*2, end.y()-ARROW_HEAD_SIZE);
                points[1] = QPointF(end.x()+ARROW_HEAD_SIZE*2, end.y()+ARROW_HEAD_SIZE);
            } else if (arrowType == Hb::TopEdge) {
                points[0] = QPointF(end.x()-ARROW_HEAD_SIZE, end.y()+ARROW_HEAD_SIZE*2);
                points[1] = QPointF(end.x()+ARROW_HEAD_SIZE, end.y()+ARROW_HEAD_SIZE*2);
            } else {
                points[0] = QPointF(end.x()-ARROW_HEAD_SIZE, end.y()-ARROW_HEAD_SIZE*2);
                points[1] = QPointF(end.x()+ARROW_HEAD_SIZE, end.y()-ARROW_HEAD_SIZE*2);
            }
            painter->drawPolygon(points, 3);

            // Draw invalid difference
            if (end != end2) {
                myPen.setColor(mInvalidColor);
                myPen.setStyle(Qt::DashLine);
                painter->setPen(myPen);
                painter->drawLine(end, end2);
            }

        } // End anchors for loop
    }
    painter->restore();
#else
    Q_UNUSED(painter);
#endif
}

