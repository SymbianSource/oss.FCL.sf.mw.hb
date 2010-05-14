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

#ifndef HBSCROLLAREA_H
#define HBSCROLLAREA_H

#include <hbwidget.h>

class HbScrollAreaPrivate;
class HbScrollBar;

#ifdef HB_GESTURE_FW
class QPanGesture;
#endif


class HB_CORE_EXPORT HbScrollArea : public HbWidget
{
    Q_OBJECT

    Q_PROPERTY(Qt::Orientations scrollDirections READ scrollDirections WRITE setScrollDirections)
    Q_PROPERTY(ClampingStyle clampingStyle READ clampingStyle WRITE setClampingStyle)
    Q_PROPERTY(ScrollingStyle scrollingStyle READ scrollingStyle WRITE setScrollingStyle)
    Q_PROPERTY(bool frictionEnabled READ frictionEnabled WRITE setFrictionEnabled)
    Q_PROPERTY(bool longPressEnabled  READ longPressEnabled  WRITE setLongPressEnabled )
    Q_PROPERTY(ScrollBarPolicy verticalScrollBarPolicy READ verticalScrollBarPolicy WRITE setVerticalScrollBarPolicy)
    Q_PROPERTY(ScrollBarPolicy horizontalScrollBarPolicy READ horizontalScrollBarPolicy WRITE setHorizontalScrollBarPolicy)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool continuationIndicators READ continuationIndicators WRITE setContinuationIndicators)

    Q_ENUMS(ClampingStyle)
    Q_ENUMS(ScrollingStyle)
    Q_ENUMS(ScrollBarPolicy)

public:
    explicit HbScrollArea(QGraphicsItem *parent = 0);
    virtual ~HbScrollArea();

    enum ClampingStyle {
        StrictClamping = 0,
        BounceBackClamping,
        NoClamping
    };

    enum ScrollingStyle {
        Pan = 0,
        PanOrFlick,
        PanWithFollowOn
    };

    enum ScrollBarPolicy {        
        ScrollBarAsNeeded = Qt::ScrollBarAsNeeded,
        ScrollBarAlwaysOff = Qt::ScrollBarAlwaysOff,
        ScrollBarAlwaysOn = Qt::ScrollBarAlwaysOn,
        ScrollBarAutoHide
    };

    QGraphicsWidget *contentWidget() const;
    void setContentWidget(QGraphicsWidget *widget);    
    QGraphicsWidget *takeContentWidget();

    ClampingStyle clampingStyle() const;
    void setClampingStyle(ClampingStyle value);    

    ScrollingStyle scrollingStyle() const;
    void setScrollingStyle(ScrollingStyle value);    

    Qt::Orientations scrollDirections() const;
    void setScrollDirections(Qt::Orientations value);    

    bool frictionEnabled() const;
    void setFrictionEnabled(bool value);    

    bool longPressEnabled() const;
    void setLongPressEnabled(bool value);   

    ScrollBarPolicy verticalScrollBarPolicy() const;
    void setVerticalScrollBarPolicy(ScrollBarPolicy policy);
    HbScrollBar *verticalScrollBar() const;
    void setVerticalScrollBar(HbScrollBar *scrollBar);

    ScrollBarPolicy horizontalScrollBarPolicy() const;
    void setHorizontalScrollBarPolicy(ScrollBarPolicy policy);
    HbScrollBar *horizontalScrollBar() const;
    void setHorizontalScrollBar(HbScrollBar *scrollBar);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    bool continuationIndicators() const;
    void setContinuationIndicators(bool indication);

    bool isScrolling() const;
    bool isDragging() const;
    void ensureVisible(const QPointF &position, qreal xMargin = 0.0, qreal yMargin = 0.0);

    void scrollContentsTo (const QPointF &newPosition, int time = 0);

protected:
    HbScrollArea(HbScrollAreaPrivate &dd, QGraphicsItem *parent);    
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual bool event(QEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual bool scrollByAmount(const QPointF &delta);
    virtual void polish(HbStyleParameters &params);
    virtual bool eventFilter(QObject *obj, QEvent *event);
#ifdef HB_GESTURE_FW
    virtual void gestureEvent(QGestureEvent *event);    
#endif

signals:
    void scrollingStarted();
    void scrollingEnded();
    void scrollDirectionsChanged(Qt::Orientations newValue);
    void scrollPositionChanged(const QPointF &newPosition);

protected slots:
    virtual void upGesture(int value);
    virtual void downGesture(int value);
    virtual void leftGesture(int value);
    virtual void rightGesture(int value);
    virtual void panGesture(const QPointF &point);
    virtual void longPressGesture(const QPointF &point);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbScrollArea)
    Q_PRIVATE_SLOT(d_func(), void _q_animateScrollTimeout())
    Q_PRIVATE_SLOT(d_func(), void _q_hideScrollBars())
    Q_PRIVATE_SLOT(d_func(), void _q_thumbPositionChanged(qreal value, Qt::Orientation orientation))
    Q_PRIVATE_SLOT(d_func(), void _q_groovePressed(qreal value, Qt::Orientation orientation))
    Q_PRIVATE_SLOT(d_func(), void _q_thumbPressed())
    Q_PRIVATE_SLOT(d_func(), void _q_thumbReleased())
};

#endif // HBSCROLLAREA_H
