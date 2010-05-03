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

#ifndef HBTOOLBUTTON_H
#define HBTOOLBUTTON_H

#include <hbglobal.h>
#include <hbabstractbutton.h>
#include <hbicon.h>

class HbAction;
class HbToolButtonPrivate;
class HbStyleOptionToolButton;

class HB_CORE_EXPORT HbToolButton : public HbAbstractButton
{
    Q_OBJECT
    Q_PROPERTY( HbIcon background READ background WRITE setBackground )
    Q_PROPERTY( ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle )
    Q_ENUMS(ToolButtonStyle)

public:
    enum ToolButtonStyle {
        ToolButtonIcon        = 0x1,
        ToolButtonText        = 0x2,
        ToolButtonTextAndIcon = 0x3
    };

    explicit HbToolButton( QGraphicsItem *parent = 0 );
    explicit HbToolButton( HbAction *action, QGraphicsItem *parent = 0 );
    virtual ~HbToolButton();

    HbAction *action() const;
    void setAction( HbAction *action );

    HbIcon background() const;
    void setBackground( const HbIcon &background );

    ToolButtonStyle toolButtonStyle() const;
    void setToolButtonStyle( ToolButtonStyle style );

    QGraphicsItem *primitive( HbStyle::Primitive primitive ) const;

    enum { Type = Hb::ItemType_ToolButton };
    int type() const { return Type; }

public slots:
    void updatePrimitives();

signals:
    void triggered( HbAction *action );

protected:
    HbToolButton( HbToolButtonPrivate &dd, QGraphicsItem *parent );

    void initStyleOption( HbStyleOptionToolButton *option );
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    void nextCheckState();
    bool event( QEvent *event );
    void polish(HbStyleParameters &params);
    bool sceneEvent(QEvent *event);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbToolButton )
    Q_DISABLE_COPY(HbToolButton)

    Q_PRIVATE_SLOT(d_func(), void _q_actionTriggered())
    Q_PRIVATE_SLOT(d_func(), void _q_actionChanged())
};

#endif // HBTOOLBUTTON_H
