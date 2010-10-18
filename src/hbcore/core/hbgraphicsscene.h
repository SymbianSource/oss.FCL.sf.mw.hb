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

#ifndef HBGRAPHICSSCENE_H
#define HBGRAPHICSSCENE_H

#include <QGraphicsScene> //krazy:exclude=qclasses
#include <QStyleOptionGraphicsItem>
#include <QGraphicsItem>
#include <hbglobal.h>


class HbGraphicsScenePrivate;
class HbPopup;

class HB_CORE_EXPORT HbGraphicsScene : public QGraphicsScene //krazy:exclude=qclasses
{
    Q_OBJECT

public:
    HbGraphicsScene(QObject *parent = 0);
    ~HbGraphicsScene();

    QVariant inputMethodQuery ( Qt::InputMethodQuery query ) const;

protected:
    void focusInEvent(QFocusEvent *focusEvent);
    void focusOutEvent(QFocusEvent *focusEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

    void drawItems(QPainter *painter, int numItems,
                   QGraphicsItem *items[],
                   const QStyleOptionGraphicsItem options[],
                   QWidget *widget = 0);
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);
    void helpEvent(QGraphicsSceneHelpEvent *helpEvent);
    bool event(QEvent *event);

private:
    HbGraphicsScenePrivate *const d_ptr;
    Q_DECLARE_PRIVATE_D(d_ptr, HbGraphicsScene)
    friend class HbToolTip;
    friend class HbPopup;
    friend class HbWidget;
};

#endif // HBGRAPHICSSCENE_H
