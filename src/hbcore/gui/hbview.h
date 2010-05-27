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
#ifndef HBVIEW_H
#define HBVIEW_H

#include <hbwidget.h>
#include <hbglobal.h>
#include <hbicon.h>
#include <hbnamespace.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#endif // HB_EFFECTS

class HbMenu;
class HbToolBar;
class HbViewPrivate;
class HbAction;
class HbDockWidget;

class HB_CORE_EXPORT HbView : public HbWidget
{
    Q_OBJECT
    Q_ENUMS(HbViewFlag)
    Q_FLAGS(HbViewFlags)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(HbIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(bool contentFullScreen READ isContentFullScreen WRITE setContentFullScreen)

public:

    enum HbViewFlag
    {
        ViewFlagNone                = 0,
        ViewTitleBarMinimizable     = 0x01, 
        ViewTitleBarMinimized       = 0x02, 
        ViewTitleBarHidden          = 0x04, 
        ViewTitleBarTransparent     = 0x08,
        ViewTitleBarFloating        = 0x10,
        ViewStatusBarHidden         = 0x20,
        ViewStatusBarTransparent    = 0x40,
        ViewStatusBarFloating       = 0x80
    };
    Q_DECLARE_FLAGS(HbViewFlags, HbViewFlag)

    explicit HbView(QGraphicsItem *parent = 0);
    virtual ~HbView();

    QString title() const;

    HbIcon icon() const;
    void setIcon(const HbIcon &icon);

    HbMenu *menu() const;
    void setMenu(HbMenu *menu);
    HbMenu* takeMenu();

    HbToolBar *toolBar() const;
    void setToolBar(HbToolBar *toolBar);
    HbToolBar* takeToolBar();

    HbDockWidget *dockWidget() const;
    void setDockWidget(HbDockWidget *dockWidget);

    QGraphicsWidget *widget() const;
    void setWidget(QGraphicsWidget *widget);
    QGraphicsWidget *takeWidget();

    void showItems(Hb::SceneItems items);
    void hideItems(Hb::SceneItems items);
    Hb::SceneItems visibleItems() const;
    bool isItemVisible(Hb::SceneItem item)  const;
    void setItemVisible(Hb::SceneItem item, bool visible);

    bool isContentFullScreen() const;
    HbViewFlags viewFlags() const;
    void setTitleBarVisible(bool visible);
    void setStatusBarVisible(bool visible);

    enum { Type = Hb::ItemType_View };
    int type() const { return Type; }

    enum ActionContainer{ 
        NotSpecified,
        OptionsMenu,
        ToolBar
    };
    using HbWidget::addAction;
    void addAction(HbAction *action, ActionContainer preferredActionContainer);

    HbAction *navigationAction() const;
    void setNavigationAction(HbAction *action);

public slots:
    void setTitle(const QString &title);
    void setContentFullScreen(bool enable = true);
    void setViewFlags(HbViewFlags flags = ViewFlagNone);

signals:
    void titleChanged(const QString &title);
    void iconChanged(const HbIcon &icon);
    void toolBarChanged();
    void visibleItemsChanged();
    void contentFullScreenChanged();
    void dockWidgetChanged();

protected:
    HbView( HbViewPrivate &dd, QGraphicsItem *parent );

    bool event(QEvent *event);
    void changeEvent(QEvent *event);

private slots:
#ifdef HB_EFFECTS
    void titleBarEffectFinished(const HbEffect::EffectStatus &status);
    void statusBarEffectFinished(const HbEffect::EffectStatus &status);
#endif // HB_EFFECTS

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbView)
    Q_DISABLE_COPY(HbView)
};

#endif // HBVIEW_H
