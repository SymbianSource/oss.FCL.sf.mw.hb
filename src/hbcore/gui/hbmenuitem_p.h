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

#ifndef HBMENUITEM_P_H
#define HBMENUITEM_P_H

#include "hbnamespace_p.h"

#include <hbwidget.h>
#include <hbmenu.h>
#include <hbnamespace.h>
#include <hbglobal.h>

class HbAction;
class HbMenuItemPrivate;

class HB_AUTOTEST_EXPORT HbMenuItem : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(HbMenu::MenuType menuType READ menuType)
    Q_PROPERTY(bool checkbox_exists READ checkboxExists)
    Q_PROPERTY(bool submenu_exists READ submenuExists)
    Q_PROPERTY(bool separator_exists READ separatorExists)

public:
    explicit HbMenuItem(HbMenu *menu, QGraphicsItem *parent = 0);
    virtual ~HbMenuItem();

    QAction *action() const;
    HbMenu *menu() const;
    void setAction(QAction *action);
    void pressStateChanged(bool value);
    void recycleItem();
    HbMenu::MenuType menuType() const;

    enum { Type = Hb::ItemType_MenuItem };
    int type() const { return Type; }

    bool checkboxExists();
    bool submenuExists();
    bool separatorExists();

public slots:
    void recreatePrimitives();
    void updatePrimitives();

protected:
    void initPrimitiveData(HbStylePrimitiveData *primitiveData, const QGraphicsObject *primitive);
    void changeEvent(QEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

#ifdef HB_GESTURE_FW
    void gestureEvent(QGestureEvent *event);
#endif

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbMenuItem)
    Q_PRIVATE_SLOT(d_func(), void _q_updateItem())

    friend class HbMenuPrivate;
};

#endif // HBMENUITEM_P_H
