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

#ifndef HBTOOLBAR_H
#define HBTOOLBAR_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbwidget.h>

class HbToolBarPrivate;
class HbAction;
class HbIcon;
class HbToolBarExtension;

class HB_CORE_EXPORT HbToolBar : public HbWidget
{
    Q_OBJECT

    Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation)

public:
    explicit HbToolBar( QGraphicsItem *parent = 0 );
    virtual ~HbToolBar();

    using HbWidget::addAction;
    HbAction *addAction( const QString &text );
    HbAction *addAction( const HbIcon &icon, const QString &text );
    HbAction *addAction( const QString &text, const QObject *receiver, const char *member );
    HbAction *addAction( const HbIcon &icon, const QString &text, const QObject *receiver, const char *member );

    HbAction *addExtension( HbToolBarExtension *extension );
    HbAction *insertExtension( HbAction *before, HbToolBarExtension *extension );

    Qt::Orientation orientation() const;

    void updatePrimitives();
    enum { Type = Hb::ItemType_ToolBar };
    int type() const { return Type; }

public slots:
    void setOrientation(Qt::Orientation orientation);

protected:
    friend class HbView;

    HbToolBar( HbToolBarPrivate &dd, QGraphicsItem *parent = 0 );

    QVariant itemChange( GraphicsItemChange change, const QVariant &value );
    void gestureEvent(QGestureEvent *event);
    void changeEvent( QEvent *event );
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    void hideEvent(QHideEvent *event);
    bool event( QEvent *event );
    virtual void polish(HbStyleParameters &params);

private:
    Q_DECLARE_PRIVATE_D( d_ptr, HbToolBar )
    Q_DISABLE_COPY(HbToolBar)

#ifdef HB_EFFECTS
    Q_PRIVATE_SLOT(d_func(), void _q_delayedHide(HbEffect::EffectStatus status))
    //Q_PRIVATE_SLOT(d_func(), void _q_delayedShow(HbEffect::EffectStatus status))
#endif // HB_EFFECTS
};

#endif // HBTOOLBAR_H
