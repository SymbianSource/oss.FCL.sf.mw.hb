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

#ifndef HBDATEPICKERVIEWITEM_H
#define HBDATEPICKERVIEWITEM_H
#include <hblistviewitem.h>
class HbDatePickerViewItemPrivate;
class HbDatePickerViewItem : public HbListViewItem
{
    Q_OBJECT
public:
    explicit HbDatePickerViewItem(QGraphicsItem* parent =0);
    HbDatePickerViewItem(const HbDatePickerViewItem& other);
    HbAbstractViewItem *createItem();
    void updateChildItems();

    enum { Type = Hb::ItemType_DatePickerViewItem };
    virtual int type() const;

protected:
    void polish(HbStyleParameters &params);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void gestureEvent(QGestureEvent *event);
    bool event(QEvent *e);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint);
    void emitReleased(const QPointF& point);
    Q_DECLARE_PRIVATE_D(d_ptr,HbDatePickerViewItem)
};


#endif // HBDATEPICKERVIEWITEM_H
