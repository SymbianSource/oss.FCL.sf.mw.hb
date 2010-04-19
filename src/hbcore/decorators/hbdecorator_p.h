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

#ifndef HBDECORATOR_H
#define HBDECORATOR_H

#include "hbnamespace_p.h"
#include <hbnamespace.h>
#include <hbwidget.h>

class HbDecoratorPrivate;
class HbStyleOptionDecorator;

class HB_CORE_PRIVATE_EXPORT HbDecorator : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(DecoratorType decoratorType READ decoratorType)

public:
    /*
        LibHb's predefined set of decorators.

        This enum describes different decorator types available in LibHb.
     */
    enum DecoratorType
    {
        SoftKey,    /*!< A soft key decorator. */
        Indicator,  /*!< An indicator decorator. */
        TitlePane,  /*!< A title pane decorator. */
        NaviPane,   /*!< A navi pane decorator. */
    };

    explicit HbDecorator(DecoratorType type, QGraphicsItem *parent = 0);
    virtual ~HbDecorator();

    DecoratorType decoratorType() const;

    enum { Type = HbPrivate::ItemType_Decorator };
    int type() const { return Type; }

    void resetMode();

signals:
    void launchPopup(const QPointF &pos);
    void visibilityChanged();

protected:
    HbDecorator(HbDecoratorPrivate &dd, DecoratorType type, QGraphicsItem *parent=0);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void initStyleOption(HbStyleOptionDecorator *option) const;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void changeEvent(QEvent* event);
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDecorator)
    Q_DISABLE_COPY(HbDecorator)
};

#endif // HBDECORATOR_H
