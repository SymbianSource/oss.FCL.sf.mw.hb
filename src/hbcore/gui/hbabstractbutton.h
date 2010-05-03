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

#ifndef HBABSTRACTBUTTON_H
#define HBABSTRACTBUTTON_H

#include <hbglobal.h>
#include <hbicon.h>
#include <QSize>
#include <QString>
//#include <QKeySequence>
#include <hbwidget.h>
#include <hbnamespace.h>

class HbStyleOption;
class HbAbstractButtonPrivate;

class HB_CORE_EXPORT HbAbstractButton : public HbWidget
{
    Q_OBJECT

    Q_PROPERTY( bool checkable READ isCheckable WRITE setCheckable )
    Q_PROPERTY( bool checked READ isChecked WRITE setChecked DESIGNABLE isCheckable NOTIFY toggled USER true )
    Q_PROPERTY( bool autoRepeat READ autoRepeat WRITE setAutoRepeat )
    Q_PROPERTY( bool autoExclusive READ autoExclusive WRITE setAutoExclusive )
    Q_PROPERTY( int autoRepeatDelay READ autoRepeatDelay WRITE setAutoRepeatDelay )
    Q_PROPERTY( int autoRepeatInterval READ autoRepeatInterval WRITE setAutoRepeatInterval )
    Q_PROPERTY( bool down READ isDown WRITE setDown DESIGNABLE false )

public:
    explicit HbAbstractButton( QGraphicsItem *parent = 0 );
    virtual ~HbAbstractButton();


    //void setShortcut(const QKeySequence &key);
    //QKeySequence shortcut() const;

    void setCheckable(bool);
    bool isCheckable() const;

    bool isChecked() const;

    void setDown(bool);
    bool isDown() const;

    void setAutoRepeat(bool);
    bool autoRepeat() const;

    void setAutoRepeatDelay(int);
    int autoRepeatDelay() const;

    void setAutoRepeatInterval(int);
    int autoRepeatInterval() const;

    void setAutoExclusive(bool);
    bool autoExclusive() const;

    enum { Type = Hb::ItemType_AbstractButton };
    int type() const { return Type; }

public slots:
    void animateClick( int msec = 100 );
    void click();
    void toggle();
    void setChecked(bool);

signals:
    void pressed();
    void released();
    void clicked( bool checked = false );
    void toggled( bool checked );

protected:
    HbAbstractButton( HbAbstractButtonPrivate &dd, QGraphicsItem *parent );
    void initStyleOption(HbStyleOption *option) const;

    virtual bool hitButton( const QPointF &pos ) const;
    virtual void checkStateSet();
    virtual void nextCheckState();

    bool event( QEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
#ifndef HB_GESTURE_FW
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
#endif
    void focusInEvent( QFocusEvent *event );
    void changeEvent( QEvent *event );
    void timerEvent( QTimerEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void polish(HbStyleParameters& params);

    void gestureEvent(QGestureEvent *event);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbAbstractButton )
    Q_DISABLE_COPY( HbAbstractButton )
};

#endif // HBABSTRACTBUTTON_H
