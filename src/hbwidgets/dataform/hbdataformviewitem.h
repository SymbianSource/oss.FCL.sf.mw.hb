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

#ifndef HBDATAFORMITEM_H
#define HBDATAFORMITEM_H

#include <hbabstractviewitem.h>

class HbDataFormModel;
class HbDataFormModelItem;
class HbStyleOptionDataFormViewItem;

class HbDataFormViewItemPrivate;

class HB_WIDGETS_EXPORT HbDataFormViewItem : public HbAbstractViewItem
{
    Q_OBJECT
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded)
    
public:



    explicit HbDataFormViewItem(QGraphicsItem *parent = 0);
    virtual ~HbDataFormViewItem();

    enum {Type = Hb::ItemType_DataFormViewItem};
    virtual int type() const {
        return Type;
    }

    virtual HbAbstractViewItem* createItem();
    virtual bool canSetModelIndex(const QModelIndex &index) const;
    virtual void updateChildItems();
    
    void setExpanded(bool expanded);
    bool isExpanded() const;
   
    HbWidget* dataItemContentWidget()const;

public slots:

    virtual void restore();
    virtual void save();

signals:
    void itemShown(const QModelIndex&);  
   
protected:
    HbDataFormViewItem(HbDataFormViewItemPrivate &dd, QGraphicsItem *parent = 0);
    HbDataFormViewItem(const HbDataFormViewItem &source);
    HbDataFormViewItem& operator=(const HbDataFormViewItem &source);
    
    virtual HbWidget* createCustomWidget();

    virtual void pressStateChanged(bool value, bool animate);
    void initStyleOption(HbStyleOptionDataFormViewItem *option) const;

    void showEvent(QShowEvent * event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDataFormViewItem)
};

#endif //HBDATAFORMITEM_H
