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

#ifndef HBMENUCONTAINER_P_H
#define HBMENUCONTAINER_P_H

#include "hbnamespace_p.h"
#include "hbmenu.h"
#include "hbview.h"
#include "hbscrollarea.h"
#include "hbscrollarea_p.h"
#include "hbactionmanager_p.h"
#include <QGraphicsGridLayout>
#include <QGesture>

class HbMenuContainer;
class HbMenuContainerPrivate;
class HbMenuListView;
class HbMenuItem;
class HbMenuListViewPrivate;

class HbMenuListView : public HbScrollArea
{
    Q_OBJECT

public:
    explicit HbMenuListView(HbMenu *menu,QGraphicsItem *parent = 0);
    virtual ~HbMenuListView() {}
    virtual bool scrollByAmount(const QPointF &delta);
    void addActionItem(QAction *action);
    void removeActionItem(QAction *Action);
    void updateActionItem(QAction *action);
    void doDelayedLayout();
    void updateContainer();
    HbMenuItem* currentItem() const;
    void setCurrentItem(HbAction *action);
    enum { Type = HbPrivate::ItemType_MenuListView };
    int type() const { return Type; }

protected:
    virtual void gestureEvent(QGestureEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void changeEvent(QEvent *event);
private:
    void updateCurrentItem();
    bool isFocusable(QAction *action);
protected:
    friend class HbMenuPrivate;
private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbMenuListView)
};

class HbMenuListViewPrivate : public HbScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(HbMenuListView)

    public:
    HbMenuListViewPrivate();
    virtual ~HbMenuListViewPrivate() { }
    void updateScrollMetrics();
    void doLazyInit();
    HbMenuContainer *mContainer;
    HbMenuItem *mCurrentItem;
    int mCurrentIndex;
    bool mWasScrolling;
};

class HbMenuContainer : public HbWidget
{
    Q_OBJECT
public:

    explicit HbMenuContainer(HbMenu *menu,QGraphicsItem *parent = 0);
    virtual ~HbMenuContainer() {}
    void updateActionItem(QAction *action);
    QList<HbMenuItem *> items() const;
    void delayedLayout();
    void addItem(QAction *action, HbMenuItem* item = 0);
    void removeActionItem(QAction *Action);
    void visibleItemsChanged();
    void updateVisibleActionList();
    void reLayout();

protected:
    void polish(HbStyleParameters &params);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void changeEvent(QEvent *event);
private:
    int actionMenuPos(QAction *action);
    Q_DECLARE_PRIVATE_D(d_ptr, HbMenuContainer)
};

class HbMenuContainerPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbMenuContainer)

public:
    explicit HbMenuContainerPrivate(HbMenu *menu);
    void init();
    virtual ~HbMenuContainerPrivate();
    QGraphicsGridLayout *mLayout;
    HbMenu *menu;
    HbActionManager *actionManager;
    QList<HbMenuItem*> mItems;
    QMap<QAction*, HbMenuItem*>  mBufferItems;
    QList<HbMenuItem*>  mOldVisibleItems;
    QList<QAction*> mVisibleActions;
    bool mDelayLayout;
};

#endif // MENU_P_H
