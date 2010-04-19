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

#ifndef HBSCREEN_P_H
#define HBSCREEN_P_H

#include <hbwidget.h>

class HbContentWidget;
class HbDockWidget;
class HbToolBar;
class HbView;

class HbScreen : public HbWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation screenOrientation READ screenOrientation)
    Q_PROPERTY(Qt::Orientation toolBarOrientation READ toolBarOrientation)
    Q_PROPERTY(bool contentUnderTitleBar READ contentUnderTitleBar)
    Q_PROPERTY(bool contentUnderStatusBar READ contentUnderStatusBar)
    Q_PROPERTY(bool titleBarMinimizable READ titleBarMinimizable)

public:
    HbScreen();
    void setStackWidget(HbContentWidget *stack);
    void setToolBar(HbToolBar *tb);
    void setDockWidget(HbDockWidget *dock);

    Qt::Orientation toolBarOrientation() const;
    Qt::Orientation screenOrientation() const;
    bool contentUnderTitleBar() const;
    bool contentUnderStatusBar() const;
    bool titleBarMinimizable() const;
    void delayedConstruction();
    void setGeometry(const QRectF &rect);
    virtual bool event(QEvent *e);

public slots:
    void screenOrientationChanged(Qt::Orientation orientation);
    void toolBarOrientationChanged();
    void decoratorVisibilityChanged();
    void currentViewChanged(HbView* view);

protected:
    virtual void polish(HbStyleParameters &params);

private:
    void setToolBarOrientation(Qt::Orientation orientation);
    void setToolBarName(bool forced);
private:
    HbContentWidget *mStack;
    HbToolBar *mTb;
    HbDockWidget *mDock;

    int mToolBarOrientation;
    bool mDelayedConstructionHandled;
    bool mScreenPolished;
    Qt::Orientation mScreenOrientation;
};

#endif // HBSCREEN_P_H
