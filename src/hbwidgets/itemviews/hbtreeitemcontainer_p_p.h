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
#ifndef HBTREEITEMCONTAINER_P_H
#define HBTREEITEMCONTAINER_P_H

#include "hbabstractitemcontainer_p_p.h"

class HbTreeLayout;
class HbTreeItemContainer;

class HbTreeItemContainerPrivate: public HbAbstractItemContainerPrivate
{
    Q_DECLARE_PUBLIC(HbTreeItemContainer)

public:
    HbTreeItemContainerPrivate();

    virtual ~HbTreeItemContainerPrivate();

    void init();
    int levelForItem(HbAbstractViewItem *item) const;

    HbAbstractViewItem *shiftUpItem(QPointF& delta);
    HbAbstractViewItem *shiftDownItem(QPointF& delta);
    void resolveIndentation();

    qreal itemHeight() const;
    qreal getSmallestItemHeight() const;

    int mapToLayoutIndex(int index) const;

    HbTreeLayout *mLayout;
    qreal mUserIndentation;
    qreal mStyleIndentation;

    QList< QPair<HbAbstractViewItem *, int> > mAnimatedItems;
};
#endif /* HBTREEITEMCONTAINER_H */
