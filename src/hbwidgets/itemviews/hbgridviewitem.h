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

#ifndef HBGRIDVIEWITEM_H
#define HBGRIDVIEWITEM_H

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbabstractviewitem.h>
          
class HbGridViewItemPrivate;
class HbStylePrimitiveData;

class QGraphicsObject;
class QPointF;

class HB_WIDGETS_EXPORT HbGridViewItem: public HbAbstractViewItem
{
Q_OBJECT

public:
    explicit HbGridViewItem(QGraphicsItem *parent = 0);
    virtual ~HbGridViewItem();

    enum
    {
        Type = Hb::ItemType_GridViewItem
    };
    int type() const;

    HbAbstractViewItem *createItem();
    void updateChildItems();


    virtual bool selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const;

public slots:
    void updatePrimitives();

protected:
    HbGridViewItem(const HbGridViewItem &source);
    HbGridViewItem &operator=(const HbGridViewItem &source);

    HbGridViewItem(HbGridViewItemPrivate &dd, QGraphicsItem *parent);
    QPainterPath shape() const;
    void polish(HbStyleParameters& params);
    void initPrimitiveData( HbStylePrimitiveData     *primitiveData, 
                            const QGraphicsObject    *primitive);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbGridViewItem)
};

#endif // HBGRIDVIEWITEM_H
