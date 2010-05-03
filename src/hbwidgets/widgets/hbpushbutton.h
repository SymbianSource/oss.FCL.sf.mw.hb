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

#ifndef HBPUSHBUTTON_H
#define HBPUSHBUTTON_H

#include <hbabstractbutton.h>

#define HB_PUSHBUTTON_TYPE "HB_PUSHBUTTON"

class HbPushButtonPrivate;
class HbStyleOptionPushButton;
class HbFrameDrawer;

class HB_WIDGETS_EXPORT HbPushButton : public HbAbstractButton
{
    Q_OBJECT
    Q_PROPERTY( HbIcon background READ background WRITE setBackground )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( QString additionalText READ additionalText WRITE setAdditionalText )
    Q_PROPERTY( HbIcon icon READ icon WRITE setIcon )
    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment )
	Q_PROPERTY( Qt::Alignment additionalTextAlignment READ additionalTextAlignment WRITE setAdditionalTextAlignment )
    Q_PROPERTY( bool stretched READ isStretched WRITE setStretched )

public:
    explicit HbPushButton( QGraphicsItem *parent = 0 );
    explicit HbPushButton( const QString &text, QGraphicsItem *parent = 0 );
    HbPushButton( const HbIcon &icon, const QString &text, QGraphicsItem *parent = 0 );
    HbPushButton( const HbIcon &icon, const QString &text, Qt::Orientation orientation,
        QGraphicsItem *parent = 0 );
    virtual ~HbPushButton( );

    void setBackground( const HbIcon &background );
    HbIcon background( ) const;

    void setFrameBackground( HbFrameDrawer *backgroundFrameDrawer );
    HbFrameDrawer* frameBackground( ) const;

    QString text( ) const;
    QString additionalText( ) const;

    void setIcon( const HbIcon &icon );
    HbIcon icon( ) const;

    void setOrientation( Qt::Orientation orientation );
    Qt::Orientation orientation( ) const;

    void setTextAlignment( Qt::Alignment alignment );
    Qt::Alignment textAlignment ( ) const;

    void setAdditionalTextAlignment( Qt::Alignment alignment );
    Qt::Alignment additionalTextAlignment ( ) const;
	

    void setStretched( bool stretched = true );
    bool isStretched( ) const;

    QGraphicsItem* primitive( HbStyle::Primitive primitive ) const;

    enum { Type = Hb::ItemType_PushButton };
    int type() const { return Type; }

public slots:
    void setText( const QString &text );
    void setAdditionalText( const QString &additionalText );
    void recreatePrimitives();
    void updatePrimitives();

signals:
    void longPress( QPointF );

protected:
    HbPushButton( HbPushButtonPrivate &dd, QGraphicsItem *parent );
    void initStyleOption( HbStyleOptionPushButton *option ) const;

    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
#ifndef HB_GESTURE_FW
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
#endif

    void gestureEvent(QGestureEvent *event);

    void resizeEvent( QGraphicsSceneResizeEvent *event );
    virtual void polish( HbStyleParameters& params );

    void focusInEvent( QFocusEvent *event );
    void focusOutEvent( QFocusEvent *event );
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );

    bool hitButton( const QPointF &pos ) const;

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbPushButton )
    Q_DISABLE_COPY( HbPushButton )
    Q_PRIVATE_SLOT( d_func(),void _q_handleLongPress(QPointF) )
    Q_PRIVATE_SLOT( d_func(),void _q_handleLongKeyPress( ) )

};

#endif // HBPUSHBUTTON_H
