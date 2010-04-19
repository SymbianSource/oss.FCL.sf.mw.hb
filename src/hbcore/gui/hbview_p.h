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
#ifndef HBVIEW_P_H
#define HBVIEW_P_H

#include <hbwidget_p.h>
#include <hbglobal.h>
#include <hbicon.h>
#include <hbnamespace.h>

#include <QPointer>

class HbMenu;
class HbToolBar;
class HbStackedLayout;
class HbDockWidget;

class HbViewActionManager;

class HB_CORE_PRIVATE_EXPORT HbViewPrivate : public HbWidgetPrivate
{
    Q_DECLARE_PUBLIC(HbView)

public:
    HbViewPrivate();
    virtual ~HbViewPrivate();

    HbStackedLayout *mLayout;
    QString title;
    HbIcon icon;
    QPointer<HbMenu> menu;
    QPointer<HbToolBar> toolBar;
    QPointer<HbDockWidget> dockWidget;
    QGraphicsWidget *widget;
    Hb::SceneItems mVisibleItems;
    bool mVisibleItemsSet;
    bool mFullscreen;
    bool mVisited; //to be used by HbMainWindow for performance optimization
    HbView::HbTitleBarFlags mTitleBarFlags; // Deprecated
    HbView::HbViewFlags mViewFlags;

    HbViewActionManager *actionManager;
    HbView::ActionContainer preferredActionContainer;
    HbAction *mNavigationAction;
    bool mNavigationActionSet;

private:
    static HbViewPrivate *d_ptr(HbView *view) {
        Q_ASSERT(view);
        return view->d_func();
    }

    friend class HbMainWindowPrivate;
};

#endif // HBVIEW_P_H
