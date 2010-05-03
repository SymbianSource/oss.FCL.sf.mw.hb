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
#ifndef HBTREEITEMCONTAINER_H
#define HBTREEITEMCONTAINER_H

#include <hbabstractitemcontainer_p.h>
#include <hblistviewitem.h>

class HbTreeItemContainerPrivate;

class HB_AUTOTEST_EXPORT HbTreeItemContainer : public HbAbstractItemContainer
{
    Q_OBJECT
    Q_PROPERTY(qreal indentation READ indentation WRITE setIndentation)

public:
    explicit HbTreeItemContainer(QGraphicsItem *parent=0);
    virtual ~HbTreeItemContainer();

    virtual void addItem(const QModelIndex &index, bool animate = false);
    virtual void removeItem(int pos, bool animate = false);

    void setIndentation(qreal indentation);
    qreal indentation() const;

    QPointF recycleItems(const QPointF &delta);
    int maxItemCount() const;

protected:

    HbTreeItemContainer(HbTreeItemContainerPrivate &dd, QGraphicsItem *parent);

    virtual void setItemModelIndex(HbAbstractViewItem *item, const QModelIndex &index);

    virtual void itemAdded(int index, HbAbstractViewItem *item, bool animate = false);
    virtual void itemRemoved(HbAbstractViewItem *item, bool animate = false);

    virtual void viewResized(const QSizeF &size);
    virtual bool event(QEvent *e);

    virtual HbAbstractViewItem *createDefaultPrototype() const;

protected slots:
    void animationFinished(const HbEffect::EffectStatus &status);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbTreeItemContainer)
    Q_DISABLE_COPY(HbTreeItemContainer)
};

#endif /* HBTREEITEMCONTAINER_H */
