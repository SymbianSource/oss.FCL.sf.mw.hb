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

#ifndef HBLISTWIDGET_H
#define HBLISTWIDGET_H

#include <QObject>
#include <hbglobal.h>
#include <hblistview.h>

class HbListWidgetItem;
class HbListWidgetPrivate;
class HbListViewItem;
class HbIcon;

class HB_WIDGETS_EXPORT HbListWidget : public HbListView
{
    Q_OBJECT

    Q_PROPERTY(int count READ count )
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow)
public:
    explicit HbListWidget(QGraphicsItem *parent = 0);
    virtual ~HbListWidget();
  
    enum { Type = Hb::ItemType_ListWidget };
    virtual int type() const {
        return Type;
    }

    int count() const;

    int currentRow() const;
    void setCurrentRow(int row);

    HbListWidgetItem *currentItem() const;
    void setCurrentItem(HbListWidgetItem *item);
    
    HbListWidgetItem *item(int row) const;
    int row(const HbListWidgetItem *item) const;

    void addItem(const HbIcon &icon, const QString &text);  
    void addItem(const QString &text);
    void addItem(HbListWidgetItem *item);

    void insertItem(int row, const HbIcon &icon, const QString &text); 
    void insertItem(int row, const QString &text);
    void insertItem(int row, HbListWidgetItem *item);

    void setText(int row, const QString &text);
    void setIcon(int row,const HbIcon &icon);

    HbListWidgetItem *takeItem(int row);

    virtual bool setArrangeMode(bool arrangeMode);

    void clear();

    virtual void setModel(QAbstractItemModel *model, HbAbstractViewItem *prototype = 0);

signals:
    void activated(HbListWidgetItem *item);
    void pressed(HbListWidgetItem *item);
    void released(HbListWidgetItem *item);
    void longPressed(HbListWidgetItem *item, const QPointF &coords);
   
protected:
    HbListWidget(HbListWidgetPrivate &dd, HbAbstractItemContainer *container, QGraphicsItem *parent);

    virtual void move(const QModelIndex &from, const QModelIndex &to);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbListWidget)
    Q_DISABLE_COPY(HbListWidget)
    Q_PRIVATE_SLOT(d_func(), void _q_itemActivated(const QModelIndex&))
    Q_PRIVATE_SLOT(d_func(), void _q_itemPressed(const QModelIndex&))
    Q_PRIVATE_SLOT(d_func(), void _q_itemReleased(const QModelIndex&))
    Q_PRIVATE_SLOT(d_func(), void _q_itemLongPress(HbAbstractViewItem*, const QPointF&))
};

#endif // HBLISTWIDGET_H


