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
#ifndef HBDATAITEMCONTAINER_P_H
#define HBDATAITEMCONTAINER_P_H

#include <hbabstractitemcontainer_p.h>

class HbDataItemContainerPrivate;
class HbListLayout;

class HbDataItemContainer : public HbAbstractItemContainer
{
    Q_OBJECT

public:
    explicit HbDataItemContainer(QGraphicsItem *parent=0);
    virtual ~HbDataItemContainer();
    void setFormHeading(HbWidget *headingWidget);

    virtual void reset();

    enum { Type = Hb::ItemType_DataItemContainer};
    int type() const { return Type; }

    virtual void resizeContainer();

protected:
    virtual void itemAdded(int index, HbAbstractViewItem *item, bool animate = false);
    virtual void itemRemoved(HbAbstractViewItem *item, bool animate = false);
    virtual void viewResized(const QSizeF &size);
    virtual void setModelIndexes(const QModelIndex &startIndex = QModelIndex());
    virtual HbAbstractViewItem *createDefaultPrototype() const;

    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbDataItemContainer)
    Q_DISABLE_COPY(HbDataItemContainer)
    //QGraphicsLinearLayout *mLayout;
    HbListLayout *mLayout;
    HbWidget *mHeadingWidget;
};

#endif /* HBDATAITEMCONTAINER_P_H */


