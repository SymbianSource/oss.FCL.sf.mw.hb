/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbCore module of the UI Extensions for Mobile.
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

#ifndef HBMENUITEM_P_P_H
#define HBMENUITEM_P_P_H

#include "hbglobal.h"
#include "hbwidget_p.h"
#include "hbstyleoptionmenuitem.h"

class HbMenu;
class HbMenuItem;
class QGraphicsItem;

class HbMenuItemPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbMenuItem)

public:
    HbMenuItemPrivate();

    virtual ~HbMenuItemPrivate();

    bool createPrimitives(HbStyleOptionMenuItem const &option);
    void updatePrimitives();

    QAction *action;
    HbMenu *menu;

    QGraphicsItem *textItem;
    QGraphicsItem *arrowItem;
    QGraphicsItem *checkItem;
    QGraphicsItem *separatorItem;    

    void _q_updateItem(bool forcedUpdate = false);

private:
    // a controlled access to the private parts
    static HbMenuItemPrivate *d_ptr(HbMenuItem *item) {
        Q_ASSERT(item);
        return item->d_func();
    }

    QGraphicsItem *mFocusItem;
    QGraphicsItem *mFrame;
    bool mChecked;
    bool mRecycled;
    friend class HbMenu;
    friend class HbMenuPrivate;
    friend class HbMenuListView;
};

#endif // HBMENUITEM_P_P_H
