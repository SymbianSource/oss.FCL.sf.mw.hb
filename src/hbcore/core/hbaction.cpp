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

#include "hbaction.h"
#include "hbaction_p.h"
#include "hbtoolbarextension.h"
#include "hbtoolbarextension_p.h"
#include <hbiconloader_p.h>

/*!
	@beta
    @hbcore
    \class HbAction
    \brief HbAction extends QAction with exclusiveness.

    The HbAction class provides an abstract user interface action that can be
    inserted into various widgets.

    In applications many common commands can be invoked via menus, toolbar
    buttons and shortcuts. Since the user expects each command to be performed
    in the same way, regardless of the user interface used, it is useful to
    represent each command as an action.

    An action may contain an icon, text, a shortcut and a tooltip. Most of
    these can be set in the constructor. They can also be set independently
    with setIcon(), setText(), setShortcut() and setToolTip().

    HbAction extends QAction with support of HbIcon. Future extensions are planned like for
    instance support of gestures.

    An action may contain a command role.
    These command roles are used when action is added through HbView::addAction. 
    The command role specifies the action container (options menu, toolbar) 
    to which the action is places and also the position in that container.    
    \sa QAction::MenuRole
    \sa HbView::addAction

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,1}

    \sa QAction
 */

/*!
\internal
*/
HbActionPrivate::HbActionPrivate() : menu(0), extension(0)
, commandRole(HbAction::NoRole)
{
}

/*!
\internal
*/
HbActionPrivate::~HbActionPrivate()
{
}

/*!
    Constructs a new HbAction with \a parent.

 */
HbAction::HbAction(QObject *parent)
    : QAction(parent), d_ptr(new HbActionPrivate)
{
}

/*!
    Constructs a new HbAction with \a text and \a parent.
 */
HbAction::HbAction(const QString &text, QObject *parent)
    : QAction(text, parent), d_ptr(new HbActionPrivate)
{
}

/*!
    Constructs a new HbAction with \a icon, \a text and \a parent.
 */
HbAction::HbAction(const HbIcon &icon, const QString &text, QObject *parent)
    : QAction(text, parent), d_ptr(new HbActionPrivate)
{
    Q_D(HbAction);
    d->icon = icon;
}

/*!
    Constructs a new HbAction with softkey \a action and \a parent.

    \deprecated HbAction::HbAction(Hb::SoftKeyAction, QObject *) is deprecated.
    Use HbAction::HbAction(Hb::NavigationAction, QObject *) instead.
*/
HbAction::HbAction(Hb::SoftKeyAction action, QObject* parent)
    : QAction(parent), d_ptr(new HbActionPrivate)
{
    Q_D(HbAction);
    switch (action) {
        case Hb::QuitAction:
            d->icon = HbIcon("qtg_mono_app_exit");
            break;
        case Hb::BackAction:
            d->icon = HbIcon("qtg_mono_back");
            break;
        case Hb::ConfirmAction:
            d->icon = HbIcon("qtg_mono_back");
            break;
        case Hb::DoneAction:
            d->icon = HbIcon("qtg_mono_back");
            break;
        default:
            qWarning("HbAction: unknown action: %i", action);
            break;
    }
}

/*!
    Constructs a new HbAction with the given navigation \a action and \a parent.
*/
HbAction::HbAction(Hb::NavigationAction action, QObject* parent)
    : QAction(parent), d_ptr(new HbActionPrivate)
{
    Q_D(HbAction);
    switch (action) {
        case Hb::QuitNaviAction:
            d->icon = HbIcon("qtg_mono_app_exit");
            break;
        case Hb::BackNaviAction:
            d->icon = HbIcon("qtg_mono_back");
            break;
        case Hb::ConfirmNaviAction:
            d->icon = HbIcon("qtg_mono_back");
            break;
        case Hb::DoneNaviAction:
            d->icon = HbIcon("qtg_mono_back");
            break;
        default:
            qWarning("HbAction: unknown action: %i", action);
            break;
    }
}

/*!
    Destructs the action.
 */
HbAction::~HbAction()
{
    delete d_ptr;
}

/*!
    Sets the icon of the action. Note that this icon is different from QIcon
	setIcon() and icon(). 

    \sa icon()
*/
void HbAction::setIcon(const HbIcon &icon)
{
    Q_D(HbAction);
    if (d->icon != icon) {
        d->icon = icon;
        emit changed();
    }
}

/*!
    Returns the icon of the action.

    \sa setIcon()
*/
HbIcon HbAction::icon() const
{
    Q_D(const HbAction);
    return d->icon;
}

/*!
    Sets the menu of the action. 

    \sa menu()
*/
void HbAction::setMenu(HbMenu *menu)
{
    Q_D(HbAction);
    if (d->menu != menu) {
        d->menu = menu;
    }
}

/*!
    Returns the menu of the action.

    \sa setMenu()
*/
HbMenu *HbAction::menu() const
{
    Q_D(const HbAction);
    return d->menu;
}

/*!
    Sets the toolTip for the action.

    \sa toolTip()
*/
void HbAction::setToolTip(const QString &tooltip)
{
    Q_D(HbAction);
    if (d->tooltip == tooltip)
        return;

    d->tooltip = tooltip;
    QAction::setToolTip(tooltip);
}

/*!
    Returns the toolTip of the action.

    \sa setToolTip()
*/
QString HbAction::toolTip() const
{
    Q_D(const HbAction);
    return d->tooltip;
}

/*!
    Sets the toolbar extension of the action. 

    \sa menu()
*/
void HbAction::setToolBarExtension(HbToolBarExtension *extension)
{
    Q_D(HbAction);
    if (d->extension != extension) {
        if (d->extension) {
            HbToolBarExtensionPrivate::d_ptr(d->extension)->setExtensionAction(this);
        }
        d->extension = extension;
    }
}

/*!
    Returns the tool bar extension of the action.

    \sa setMenu()
*/
HbToolBarExtension *HbAction::toolBarExtension() const
{
    Q_D(const HbAction);
    return d->extension;
}

/*!
   Sets the command role identifier of the action. 

    \sa commandRole()
*/
void HbAction::setCommandRole(CommandRole commandRole)
{
    Q_D(HbAction);
    d->commandRole = commandRole;
}

/*!
    Returns the command role identifier of the action.

    \sa setCommandRole()
*/
HbAction::CommandRole HbAction::commandRole() const
{
    Q_D(const HbAction);
    return d->commandRole;
}
