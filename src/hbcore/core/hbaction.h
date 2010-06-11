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

#ifndef HBACTION_H
#define HBACTION_H

#include <QAction> //krazy:exclude=qclasses

#include <hbnamespace.h>
#include <hbglobal.h>
#include <hbicon.h>

class HbMenu;
class HbActionPrivate;
class HbToolBarExtension;

class HB_CORE_EXPORT HbAction : public QAction
{
    Q_OBJECT
    Q_PROPERTY(HbIcon icon READ icon WRITE setIcon)

    Q_ENUMS(CommandRole)
    Q_PROPERTY(CommandRole commandRole READ commandRole WRITE setCommandRole)

public:
    explicit HbAction(QObject *parent = 0);
    explicit HbAction(const QString &text, QObject *parent = 0);
    explicit HbAction(const HbIcon &icon, const QString &text, QObject *parent = 0);
    explicit HbAction(Hb::NavigationAction action, QObject *parent = 0);
    virtual ~HbAction();

    using QAction::setIcon;
    using QAction::icon;
    void setIcon(const HbIcon &icon);
    HbIcon icon() const;
    using QAction::setMenu;
    using QAction::menu;
    void setMenu(HbMenu *menu);
    HbMenu *menu() const;
    using QAction::setToolTip;
    using QAction::toolTip;
    void setToolTip(const QString &tooltip);
    QString toolTip() const;
    void setToolBarExtension(HbToolBarExtension *extension);
    HbToolBarExtension *toolBarExtension() const;

    enum CommandRole { NoRole, OptionsRole, OpenRole, NewRole, DeleteRole, ClearRole,
                       SettingsRole, SendRole, PrintRole, EditRole, CopyRole, PasteRole,
                       HelpRole, QuitRole, ExitRole, OtherRole
                     };
    void setCommandRole(CommandRole commandRole);
    CommandRole commandRole() const;

protected:
    HbActionPrivate *const d_ptr;
    HbAction(HbActionPrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(HbAction)
    Q_DECLARE_PRIVATE_D(d_ptr, HbAction)
};

#endif // HBACTION_H
