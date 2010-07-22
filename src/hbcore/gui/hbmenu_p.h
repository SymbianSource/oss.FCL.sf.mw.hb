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

#ifndef HBMENU_P_H
#define HBMENU_P_H

#include "hbmenu.h"
#include "hbview.h"
#include "hbpopup_p.h"

class HbMenuListView;
class HbMenuItem;

class HbMenuPrivate : public HbPopupPrivate
{
    Q_DECLARE_PUBLIC(HbMenu)

public:
    HbMenuPrivate();
    ~HbMenuPrivate();

    void init();
    void addPopupEffects();
    static bool isFocusable(QAction *action);// krazy:exclude=qclasses
    bool isEmpty() const;

    void openSubmenu(HbMenuItem *activeItem = 0);
    void closeSubmenu();
    void createMenuView();
    void delayedLayout();
    void setSubMenuPosition();
    void changeToOptionsMenu();
    HbMenuItem *subMenuItem();
    void setSubMenuItem(HbMenuItem *menuItem);
    void closeMenu();
    void closeMenuRecursively(HbAction* menuAction);

    void _q_triggerAction(HbMenuItem *currentItem);
    void _q_onActionTriggered();
    void _q_subMenuItemTriggered(HbAction *action);
    void _q_subMenuTimedOut();
    void _q_handleMenuClose();
    void actionAdded(QActionEvent *actionEvent);
    void actionRemoved(QActionEvent *actionEvent);
    void actionChanged(QActionEvent *actionEvent);
    HbAction *activeAction(HbMenuItem *&activeItem) const;
    HbAction *activeAction() const;
    HbMenuItem *menuItem(QAction *action); // krazy:exclude=qclasses

    HbMenuListView *menuItemView;
    HbAction *subMenuAction;
    QPointer<HbMenu> activeSubMenu;
    bool actionTriggered;
    bool menuTimedOut(HbMenu* menu);

    HbMenu::MenuType menuType;
    QPointer<HbMenuItem> mSubMenuItem;

    qreal mRightMargin;
    qreal mDownMargin;
    bool delayMenuConstruction;

    QPointer<QObject> receiverToDisconnectOnClose;
    QByteArray memberToDisconnectOnClose;
    int mNumberOfColumns;

private:
    static bool menuEffectsLoaded;
    static HbMenuPrivate *d_ptr(HbMenu *menu) {
        Q_ASSERT(menu);
        return menu->d_func();
    }

    friend class HbMenuItem;
    friend class HbMenuListView;
    friend class HbView;
    friend class HbMenuContainer;
};

#endif // MENU_P_H
