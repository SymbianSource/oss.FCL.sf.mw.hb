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

#ifndef HBCHECKBOX_H
#define HBCHECKBOX_H

#include <hbabstractbutton.h>
#define HB_CHECKBOX_TYPE "HB_CHECKBOX"

class HbCheckBoxPrivate;
class HbStyleOptionCheckBox; 

class HB_WIDGETS_EXPORT HbCheckBox : public HbAbstractButton
{
    Q_OBJECT
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate)
    Q_PROPERTY(Qt::CheckState checkState READ checkState WRITE setCheckState)

public:

    explicit HbCheckBox( QGraphicsItem *parent = 0 );
    explicit HbCheckBox( const QString &text, QGraphicsItem *parent = 0 );
    virtual ~HbCheckBox();  

    enum { Type = Hb::ItemType_CheckBox };
    int type() const { return Type; }

    void setText( const QString &text );
    QString text( ) const;

    void setTristate( bool isTristate = true );
    bool isTristate( ) const;

    Qt::CheckState checkState( ) const;

    virtual QGraphicsItem *primitive(HbStyle::Primitive primitive) const;

public slots:

    void setCheckState( Qt::CheckState state );
    virtual void updatePrimitives();

protected:
    void initStyleOption( HbStyleOptionCheckBox *option ) const;
    void resizeEvent(QGraphicsSceneResizeEvent *event);

    virtual bool hitButton( const QPointF &pos ) const;
    virtual void checkStateSet( );
    virtual void nextCheckState( );
#ifndef HB_GESTURE_FW
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
#endif
#ifdef HB_GESTURE_FW
    virtual void gestureEvent(QGestureEvent *event);
#endif
    void keyPressEvent(QKeyEvent *keyEvent);
    QVariant itemChange( GraphicsItemChange change, const QVariant &value );

signals:
    void stateChanged ( int state );

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbCheckBox)
    Q_DISABLE_COPY(HbCheckBox)
    

};

#endif  //HBCHECKBOX_H

