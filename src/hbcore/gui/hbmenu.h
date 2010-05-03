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

#ifndef HBMENU_H
#define HBMENU_H

#include <hbpopup.h>

class HbAction;
class HbMenuPrivate;

class HB_CORE_EXPORT HbMenu : public HbPopup
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(MenuType menuType READ menuType)

    Q_ENUMS(MenuType)

public:

    explicit HbMenu(QGraphicsItem *parent = 0);
    explicit HbMenu(const QString &title, QGraphicsItem *parent = 0);
    virtual ~HbMenu();

    enum MenuType {
        ContextMenu = 0,
        OptionsMenu,
        SubMenu
    };

    HbAction *exec(const QPointF &pos, HbAction *action = 0);

    using HbPopup::addAction;
    HbAction *addAction(const QString &text);
    HbAction *addAction(const QString &text, const QObject *receiver, const char *member);

    HbAction *addMenu(HbMenu *menu);       
    HbMenu *addMenu(const QString &title); 
    HbAction *menuAction() const;
    HbAction *insertMenu(HbAction *before, HbMenu *menu);

    HbAction *addSeparator();
    HbAction *insertSeparator(HbAction *before);

    HbAction *activeAction() const;
    void setActiveAction(HbAction *action);

    bool isEmpty() const;

    void setTitle(const QString &title);
    QString title() const;

    HbMenu::MenuType menuType() const;

    enum { Type = Hb::ItemType_Menu };
    int type() const { return Type; }
    QPainterPath shape() const;

public slots:
    HbAction *exec(HbAction *action = 0);
    void open(QObject *receiver = 0, const char *member = 0);

signals:
    void triggered(HbAction *action);

protected:
    HbMenu(HbMenuPrivate &dd, QGraphicsItem *parent);

    bool event(QEvent *event);
    void polish(HbStyleParameters &params);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void showEvent(QShowEvent *event);

private:
    Q_DECLARE_PRIVATE_D(d_ptr, HbMenu)
    Q_DISABLE_COPY(HbMenu)

    Q_PRIVATE_SLOT(d_func(), void _q_onActionTriggered())
    Q_PRIVATE_SLOT(d_func(), void _q_subMenuItemTriggered(HbAction *action))
    Q_PRIVATE_SLOT(d_func(), void _q_subMenuTimedOut())
    Q_PRIVATE_SLOT(d_func(), void _q_handleMenuAfterOrientationChange())
};

#endif // HBMENU_H
