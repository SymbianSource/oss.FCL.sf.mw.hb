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

#ifndef HB_COLORGRIDVIEWITEM_H
#define HB_COLORGRIDVIEWITEM_H

#include "hbnamespace_p.h"
#include <hbgridviewitem.h>

class HbColorGridViewItemPrivate;
class HbStyleOptionColorGridViewItem;

class HbColorGridViewItem : public HbGridViewItem
{
    Q_OBJECT

public:
    explicit HbColorGridViewItem(QGraphicsItem *parent = 0);
    virtual ~HbColorGridViewItem();

    HbAbstractViewItem* createItem();
    void updateChildItems();
    void updatePrimitives();

    QGraphicsItem * primitive(HbStyle::Primitive primitive) const;
    void initStyleOption(HbStyleOptionColorGridViewItem *option) const;
    enum { Type = HbPrivate::ItemType_ColorGridViewItem};
    int type() const { return Type; }
    enum { ColorRole = Qt::UserRole + 1 };
protected:
    HbColorGridViewItem(HbColorGridViewItemPrivate &dd, QGraphicsItem *parent);
    HbColorGridViewItem(const HbColorGridViewItem &source);
    virtual void resizeEvent ( QGraphicsSceneResizeEvent * event );

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbColorGridViewItem)
};

#endif // HB_COLORGRIDVIEWITEM_H

