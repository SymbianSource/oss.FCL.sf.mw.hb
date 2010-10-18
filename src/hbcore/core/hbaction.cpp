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
    @stable
    @hbcore
    \class HbAction
    \brief The HbAction class provides a generic command object that can be inserted
    into a variety of %Hb widgets, such as toolbars, menus, and dialogs.

    Command objects are called \b actions and are represented in different ways depending 
    on which widget they are in. For example, they are presented in a list in a menu and as 
    buttons in a toolbar or a dialog. Using a generic class for commands, regardless of where 
    they are used and how they are presented, provides consistency for the programmer. It also 
    enables the same action to be used in more than one widget and to be swapped between 
    widgets, in differing circumstances.
    
    The HbAction class extends the QAction class with additional functionality, including 
    support for HbIcon, which enables icons to be themed. This means, for example, that the
    icon displayed for an action automatically changes when the theme is changed. You can 
    set the icon in the constructor or by using setIcon(const HbIcon &).
    
    You can use the HbAction constructors to create an action. You can optionally specify the 
    action's text, icon and parent. If the action is to provide a standard navigation 
    option, such as "Back", you can use the version of the constructor that lets you specify
    the type of navigation. This selects the standard icon for that type of navigation. 
    
    All classes that derive from QGraphicsWidget have \link QGraphicsWidget::addAction()
    addAction()\endlink, \link QGraphicsWidget::addActions() addActions()\endlink, 
    \link QGraphicsWidget::insertAction() insertAction()\endlink and 
    \link QGraphicsWidget::insertActions() insertActions()\endlink methods. You can use 
    these to add actions to a widget. The add-prefixed functions add the action to the 
    end of the widget's list of actions and the insert-prefixed functions add the action 
    at a specific position. 
    
    Alternatively, you can create actions and add them to a menu or toolbar by calling 
    HbMenu::addAction() or HbToolBar::addAction(), respectively. These convenience functions 
    return an HbAction pointer so that you can use the new action elsewhere in your 
    application. Alternatively, instead of adding actions directly to the options menu or 
    toolbar, you can add them to the \b view by calling HbView::addAction(). This distributes 
    them to the view's toolbar or options menu depending on the preference set, the UI 
    command distribution template, and taking into account the available space in the toolbar. 
    The menu and toolbar then order the actions according to their defined command roles 
    and the UI command container template. This approach makes it easier to create consistent 
    user interfaces and applications that work well on a variety of different devices. When 
    using this approach, you define the roles of the actions by calling setCommandRole().
    
    After you create an action, you connect its \link HbAction::triggered() triggered()\endlink
    signal to the slot that will perform the command. 
       
    The view has a navigation button in the top right corner. This is controlled by an action
    and is a Back button by default. You can set it through a call to HbView::setNavigationAction().
    
    You can use HbAction::setMenu() to associate a menu (HbMenu) with an action. This is 
    designed for adding a submenu to a menu. Similarly, you can call HbAction::setToolBarExtension()
    to associate a  toolbar extension (HbToolBarExtension) with an action. This is designed for 
    adding a popup to a toolbar (HbToolBar). The popup can contain additional actions or a widget, 
    such as a list or grid.
    
    Because HbAction is derived from QAction, you can add actions to a QActionGroup object. 
    This is useful when, for example, you want to create a group of radio-button style menu items, 
    only one of which can be selected at once. You can disable an action by calling \c setEnabled(false) 
    on the action. This property is inherited from QAction. Other properties inherited from QAction 
    enable you to specify that an action is checkable (which means it has an on/off state), to set a 
    tooltip, and so on.
    
    \section _usecases_hbaction Using the HbAction class
    
    \subsection _uc_001_hbaction Creating an action
    
    The following example demonstrates creating an action, setting its tooltip,
    adding it to a toolbar and then setting the toolbar on the view.

    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,1}
    
    \subsection _uc_002_hbaction Connecting an action's signal to a slot
    
    After creating an action, you connect its \link HbAction::triggered() triggered()\endlink
    signal to a slot on the object that will receive the signal and perform the command. For
    example:
    
    \code
    connect(mAction_Add, SIGNAL(triggered()), this, SLOT(addItem()));

    \endcode
    
    \subsection _uc_003_hbaction Adding actions to a dialog
    
    The following example demonstrates creating actions and adding them
    to a dialog. This results in the appearance of a toolbar at the bottom of the 
    dialog. This toolbar contains the actions as buttons. 
    
    \code
    // Create and set up the dialog object.
    HbDialog* dialog = new HbDialog();
    dialog->setAttribute( Qt::WA_DeleteOnClose, true );
    dialog->setDismissPolicy(HbDialog::NoDismiss);

    // Create the actions and add them to the dialog.
    HbAction *okAction = new HbAction("OK");
    HbAction *cancelAction = new HbAction("Cancel");
    dialog->addAction(okAction);
    dialog->addAction(cancelAction);

    // Create and add the content widget
    ...
    
    // Launch the dialog asynchronously.
    dialog->open(this, SLOT(handler(HbAction*)));
    \endcode

    \sa QAction, HbMenu, HbToolBar, HbToolBarExtension, HbDialog
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
    \enum HbAction::CommandRole

    Defines the role of an action for use when it is added to the view using HbView::addAction().
    The role is then used to position the action on the view's options menu or toolbar. 
    
    \sa HbMenu, HbToolBar
*/
/*!
    \var HbAction::NoRole
    The action does not have a role.
*/
/*!
    \var HbAction::OptionsRole
    The action provides access to view or application options.
*/
/*!
    \var HbAction::OpenRole
    The action opens the current item(s).
*/
/*!
    \var HbAction::NewRole
    The action creates a new item.
*/
/*!
    \var HbAction::DeleteRole
    The action deletes the current item(s).
*/
/*!
    \var HbAction::ClearRole
    The action clears the current item(s).
*/
/*!
    \var HbAction::SettingsRole
    The action provides access to view or application settings.
*/
/*!
    \var HbAction::SendRole
    The action sends the current item(s).
*/
/*!
    \var HbAction::PrintRole
    The action prints the current item(s).
*/
/*!
    \var HbAction::EditRole
    The action puts the current item into edit mode.
*/
/*!
    \var HbAction::CopyRole
    The action copies the selected item(s).
*/
/*!
    \var HbAction::PasteRole
    The action pastes the contents of the clipboard into the view.
*/
/*!
    \var HbAction::HelpRole
    The action opens the Help.
*/
/*!
    \var HbAction::QuitRole
    The action quits the application.
*/
/*!
    \var HbAction::ExitRole
    The action exits the application.
*/
/*!
    \var HbAction::OtherRole
    The action has an application-specific role.
*/


/*!
    \var HbMenu::SubMenu

    A submenu, which is a menu that has been added to another menu.
*/

/*!
    Constructs a new action with the given \a parent.

 */
HbAction::HbAction(QObject *parent)
    : QAction(parent), d_ptr(new HbActionPrivate)
{
}

/*!
    Constructs a new action with the given \a text and \a parent.
 */
HbAction::HbAction(const QString &text, QObject *parent)
    : QAction(text, parent), d_ptr(new HbActionPrivate)
{
}

/*!
    Constructs a new action with the given \a icon, \a text and \a parent.
 */
HbAction::HbAction(const HbIcon &icon, const QString &text, QObject *parent)
    : QAction(text, parent), d_ptr(new HbActionPrivate)
{
    Q_D(HbAction);
    d->icon = icon;
}

/*!
    Constructs a new action with a standard navigation icon and the given \a parent.
    
    \param action The navigation type. This must be a valid Hb::NavigationAction value.
    \param parent The parent object.
*/
HbAction::HbAction(Hb::NavigationAction action, QObject *parent)
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
        hbWarning("HbAction: unknown action: %i", action);
        break;
    }
}

/*!
    Destructor.
 */
HbAction::~HbAction()
{
    delete d_ptr;
}

/*!
    Sets the action's icon to the given HbIcon object. HbIcon is different from QIcon and 
    this function therefore differs from the \c %setIcon() function that is inherited from 
    QAction for setting the icon.

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
    Returns the action's icon.

    \sa setIcon()
*/
HbIcon HbAction::icon() const
{
    Q_D(const HbAction);
    return d->icon;
}

/*!
    Associates a menu with the action. This is designed for adding a submenu to an
    action that is within a menu.

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
    If there is a menu associated with the action, this returns that menu; otherwise it 
    returns null.
    

    \sa setMenu()
*/
HbMenu *HbAction::menu() const
{
    Q_D(const HbAction);
    return d->menu;
}

/*!
    Sets the action's tooltip.

    \sa toolTip()
*/
void HbAction::setToolTip(const QString &tooltip)
{
    Q_D(HbAction);
    if (d->tooltip == tooltip) {
        return;
    }

    d->tooltip = tooltip;
    QAction::setToolTip(tooltip);
}

/*!
    Returns the action's tooltip.

    \sa setToolTip()
*/
QString HbAction::toolTip() const
{
    Q_D(const HbAction);
    return d->tooltip;
}

/*!
    Associates a toolbar extension with the action. This is designed for adding a 
    popup to a toolbar (HbToolBar class). 

    \sa toolBarExtension()
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
    If there is a toolbar extension associated with the action, this returns that 
    toolbar extension; otherwise it returns null.

    \sa setToolBarExtension()
*/
HbToolBarExtension *HbAction::toolBarExtension() const
{
    Q_D(const HbAction);
    return d->extension;
}

/*!
   Sets the action's command role. This is used to position the action on a view's
   toolbar or options menu when the action is added to the view using HbView::addAction(). 

    \sa commandRole()
*/
void HbAction::setCommandRole(CommandRole commandRole)
{
    Q_D(HbAction);
    d->commandRole = commandRole;
}

/*!
    Returns the action's command role identifier.

    \sa setCommandRole(), HbView::addAction()
*/
HbAction::CommandRole HbAction::commandRole() const
{
    Q_D(const HbAction);
    return d->commandRole;
}
