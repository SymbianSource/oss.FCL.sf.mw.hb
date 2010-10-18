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

#ifndef HBSCROLLBAR_H
#define HBSCROLLBAR_H

#include <hbwidget.h>

class HbScrollBarPrivate;

class HB_CORE_EXPORT HbScrollBar : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY( qreal value READ value WRITE setValue )
    Q_PROPERTY( qreal pageSize READ pageSize WRITE setPageSize )
    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( bool interactive READ isInteractive WRITE setInteractive )
public:
    explicit HbScrollBar( QGraphicsItem *parent = 0 );
    explicit HbScrollBar( Qt::Orientation orientation, QGraphicsItem *parent = 0 );
    virtual ~HbScrollBar();

    qreal value() const;
    qreal pageSize() const;
    Qt::Orientation orientation() const;

    enum {
        Type = Hb::ItemType_ScrollBar
    };
    int type() const { return Type; }

    bool isInteractive() const;
    void setInteractive( bool enabled );    

    QRectF boundingRect() const;
public slots:
    void setValue( qreal value );
    void setPageSize( qreal size );
    void setOrientation( Qt::Orientation orientation );

    virtual void updatePrimitives();

signals:
    void valueChanged( qreal value, Qt::Orientation orientation );
    void valueChangeRequested( qreal value, Qt::Orientation orientation );

protected:
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void timerEvent( QTimerEvent *event );
    bool event(QEvent *event);
    void updateGeometry();
    QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
    void polish( HbStyleParameters& params );
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void gestureEvent(QGestureEvent* event);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbScrollBar )
    Q_DISABLE_COPY(HbScrollBar)
};

#endif //HBSCROLLBAR_H
