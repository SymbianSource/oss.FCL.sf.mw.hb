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

class HbMenu;
class HbMenuItem;
QT_BEGIN_NAMESPACE
class QGraphicsObject;
QT_END_NAMESPACE

class HbMenuItemPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbMenuItem)

public:
    HbMenuItemPrivate();

    virtual ~HbMenuItemPrivate();

    void createPrimitives();
    void recreatePrimitives();
    bool evaluateRecreateNecessary();

    QAction *action;
    HbMenu *menu;

    QGraphicsObject *mTextItem;
    QGraphicsObject *mArrowItem;
    QGraphicsObject *mCheckItem;
    QGraphicsObject *mSeparatorItem;
    QGraphicsObject *mFrame;

    bool             mIndicateFocus;
    bool             mRecycled;

    bool             mCheckChanged;
    bool             mTextChanged;

    bool             mOldCheckState;
    QString          mOldText;

    void _q_updateItem(bool forcedUpdate = false);

private:
    // a controlled access to the private parts
    static HbMenuItemPrivate *d_ptr(HbMenuItem *item) {
        Q_ASSERT(item);
        return item->d_func();
    }

    friend class HbMenu;
    friend class HbMenuPrivate;
    friend class HbMenuListView;
};

#endif // HBMENUITEM_P_P_H
