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
#ifndef HBLISTITEMCONTAINER_P_H
#define HBLISTITEMCONTAINER_P_H

#include <hbeffect.h>
#include <hbabstractitemcontainer_p.h>

class HbListItemContainerPrivate;

class HbListItemContainer : public HbAbstractItemContainer
{
    Q_OBJECT

public:
    explicit HbListItemContainer(QGraphicsItem *parent=0);
    virtual ~HbListItemContainer();

    void removeItem(const QModelIndex &index, bool animate);

protected:

    HbListItemContainer(HbListItemContainerPrivate &dd, QGraphicsItem *parent);

    virtual void itemAdded(int index, HbAbstractViewItem *item, bool animate = false);
    virtual void itemRemoved(HbAbstractViewItem *item, bool animate = false);

    virtual void viewResized(const QSizeF &viewSize);

    QPointF recycleItems(const QPointF &delta);
    int maxItemCount() const;

    virtual HbAbstractViewItem *createDefaultPrototype() const;

    virtual void resizeContainer();

protected slots:

    void animationFinished(const HbEffect::EffectStatus &status);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbListItemContainer)
    Q_DISABLE_COPY(HbListItemContainer)
};

#endif /* HBLISTITEMCONTAINER_P_H */
