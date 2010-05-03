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

#ifndef HBTREEVIEWITEM_H
#define HBTREEVIEWITEM_H

#include <hblistviewitem.h>
#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbstyle.h>

class HbTreeViewItemPrivate;
class HbStyleOptionTreeViewItem;

QT_BEGIN_NAMESPACE
class QPointF;
class QGraphicsItem;
QT_END_NAMESPACE

class HB_WIDGETS_EXPORT HbTreeViewItem : public HbListViewItem
{
    Q_OBJECT

    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded)
    Q_PROPERTY(bool userExpandable READ isUserExpandable WRITE setUserExpandable)

public:

    enum StateKey
    { 
        ExpansionKey = 10
    };

    explicit HbTreeViewItem(QGraphicsItem *parent=0);
    virtual ~HbTreeViewItem();

    enum { Type = Hb::ItemType_TreeViewItem };
    virtual int type() const;

    virtual HbAbstractViewItem *createItem();

    virtual void updateChildItems();

    virtual QHash<QString, QVariant> transientState() const;
    virtual void setTransientState(const QHash<QString, QVariant> &state);

    virtual QMap<int,QVariant> state() const;
    virtual void setState(const QMap<int,QVariant> &state);

    void setExpanded(bool expanded);
    bool isExpanded() const;

    virtual QGraphicsItem *primitive(HbStyle::Primitive primitive) const;

    void setUserExpandable(bool expandable);
    bool isUserExpandable() const;

    virtual bool selectionAreaContains(const QPointF &position, SelectionAreaType selectionAreaType) const;

public slots:
    void updatePrimitives();

protected:

    HbTreeViewItem(const HbTreeViewItem &source);
    HbTreeViewItem &operator=(const HbTreeViewItem &source);

    HbTreeViewItem(HbTreeViewItemPrivate &dd, QGraphicsItem * parent);

    void initStyleOption(HbStyleOptionTreeViewItem *option) const;

private:

    Q_DECLARE_PRIVATE_D(d_ptr, HbTreeViewItem)
};

#endif // HBTREEVIEWITEM_H
