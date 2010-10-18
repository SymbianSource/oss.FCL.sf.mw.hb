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

#include "hbtoolbar.h"
#include "hbtoolbar_p.h"
#include "hbaction.h"
#include "hbtoolbutton.h"
#include "hbtoolbutton_p.h"
#include "hbapplication.h"
#include "hbnamespace_p.h"
#include "hbtoolbarextension.h"
#include "hbtoolbarextension_p.h"
#include "hbwidget_p.h"
#include "hbinstance.h"
#include "hbactionmanager_p.h"
#include "hbmainwindow_p.h"
#include "hbcolorscheme.h"
#include "hbevent.h"

#include <hbwidgetfeedback.h>

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneResizeEvent>
#include <QWidget> // for QWIDGETSIZE_MAX
#include <QActionEvent>
#include <QDebug>
#include <QGesture>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

/*!
    @beta
    @hbcore
    \class HbToolBar
    \brief The HbToolBar class provides a widget that gives quick access to commands
    associated with the view.

    A toolbar provides similar functions to an options menu (HbMenu class). Like an options
    menu, a toolbar represents commands that apply to the entire view and not to individual
    items in the view. However, there are some important differences. A toolbar shows the
    options as buttons (each of which can have an image and a label) whereas a menu shows the
    options in a simple list. Typically the toolbar gives access to the view's most important
    commands (called first order commands) and menus provide access to second order commands.
    In an application that has multiple views (HbView class), typically the toolbar also
    enables the user to switch quickly between views.

    \image html toolbar.png A toolbar that has five buttons

    Toolbars contain \b actions, which are objects of class HbAction. A toolbar can contain
    actions that represent a button and actions that open a popup, called a toolbar extension
    (HbToolBarExtension class). This can contain additional actions or a widget, such as a list.

    Use addAction() to create an action and add it to the toolbar. There are several overloads
    of this function, which allow you to specify both a text and image or just a text and also to
    connect the action's \link HbAction::triggered() triggered()\endlink signal to a slot on
    a receiver object. The image and text, if specified, are applied to the toolbar button.
    Using an image is recommended. Use the insertAction(), addActions() and insertActions() methods
    (which are inherited from QGraphicsWidget) to add existing actions to the toolbar. Use
    clearActions() to clear all of the actions from a toolbar and removeAction() to remove
    individual actions.

    Typically a toolbar can accommodate up to five actions. If you add more actions than can
    fit, the additional actions are hidden behind the endmost button, which activates
    a popup containing the extra actions. However, usability guidelines suggest that this
    should be avoided. The order of the actions within the toolbar controls the order of the
    buttons that the user sees.

    When you add the actions directly to the toolbar, addAction() and addActions() append
    the actions to the end of the toolbar and insertAction() and insertActions() enable you to
    specify the required position. However, there is an alternative approach to ordering the
    action items. That is to call HbView::addAction() to add actions to the \b view and
    let the view distribute them to the options menu or toolbar, depending on the preference
    set, the UI command distribution template, and taking into account the available space in
    the toolbar. The menu and toolbar then order the actions according to their defined roles
    and the UI command container template. This approach makes it easier to create consistent
    user interfaces and applications that work well on a variety of different devices.

    After you add an action item to a toolbar, you can connect its \link HbAction::triggered()
    triggered()\endlink signal to a slot on a receiver object. Alternatively you can use one of
    the addAction() overloads that allow you to add an action and specify a receiver slot at
    the same time. The receiver is notified when the action is \link HbAction::triggered()
    triggered()\endlink, which means that the user has selected the button; for example, by
    tapping it.

    By default, the toolbar is horizontal and positioned at the bottom of the view. Use
    setOrientation() to change the orientation to vertical. The toolbar is then positioned on
    the right of the view.

    You add the toolbar to the view by calling HbView::setToolBar(). If you want to hide the
    toolbar, it is recommended that you call HbView::setItemVisible() rather than calling
    \link QGraphicsItem::hide() hide()\endlink or \link QGraphicsItem::setVisible()
    setVisible()\endlink on the toolbar itself.

    You can disable a toolbar button by calling \c setEnabled(false) on the action. This
    property is inherited from QAction. Other properties inherited from QAction enable
    you to specify that an action is checkable (which means it has an on/off state), to
    set a keyboard shortcut, and so on.

    \b Note: When you add an action to the toolbar, the HbToolBar implementation creates an
    internal HbToolButton object. This is not accessible and is an implementation detail that
    might change in the future.

    \section _usecases_hbtoolbar Using the HbToolBar class

    \subsection _uc_001_hbtoolbar Creating a toolbar

    The following example demonstrates creating a toolbar and adding an action
    to it and then adding the toolbar to the view:

    \code
    // Create the toolbar object.
    HbToolBar *toolBar = new HbToolBar();

    HbAction *closeAction = toolBar->addAction(tr("Close"));

    // Add the toolbar to the view.
    myView->setToolBar(toolBar);
    \endcode

    \subsection _uc_002_hbtoolbar Connecting a toolbar action's signal to a slot

    You must connect the action's \link HbAction::triggered() triggered()\endlink
    signal to a suitable slot on the receiver object that is to carry out the command.
    For example:

    \code
    QObject::connect(closeAction, SIGNAL(triggered(bool)), this, SLOT(closeAccount(bool)));
    \endcode

    Alternatively you can add the action and connect its signal to a slot by using one of the
    the convenience \link HbToolBar::addAction(const QString &, const QObject *, const char *)
    addAction() \endlink overloads that let you add the action to the toolbar and connect its
    signal to a suitable slot in one call. For example:

    \code
    toolBar->addAction(tr("Open"), d, SLOT(openAccount(bool)));
    \endcode

    \subsection _uc_003_hbtoolbar Adding actions to the view

    As mentioned above, there are advantages in terms of consistency and portability to
    adding toolbar actions to the view rather than to the toolbar itself. The following example
    demonstrates this. It creates two action items, specifies their command roles, and then
    adds them to the view, specifying the toolbar as the preferred container. The view takes
    this preference into account but may place them in the options menu if, for example, the
    toolbar is already full. The menu and toolbar order the actions according to their defined
    command roles.

    \code
    HbAction *actionCircle = new HbAction();
    actionCircle->setToolTip("Circle");
    actionCircle->setIcon(HbIcon("circle.png"));
    actionCircle->setCommandRole(HbAction::OtherRole);

    HbAction *actionStar = new HbAction();
    actionStar->setToolTip("Star");
    actionStar->setIcon(HbIcon("star.png"));
    actionStar->setCommandRole(HbAction::OtherRole);

    // Add actions to the view.
    myView->addAction(actionStar, HbView::ToolBar);
    myView->addAction(actionCircle, HbView::ToolBar);
    \endcode

    \subsection _uc_004_hbtoolbar Hiding the toolbar

    The next example demonstrates calling HbView::setItemVisible() to hide the
    toolbar. This is the recommended way of hiding a toolbar:
    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip Start of snippet 59
    \until End of snippet 59

    \sa HbToolBarExtension, HbView, HbMenu, HbAction
*/

/*!
    \fn int HbToolBar::type() const
*/

// ======== MEMBER FUNCTIONS ========

/*!
    Constructs a toolbar with the given \a parent.
*/

HbToolBar::HbToolBar( QGraphicsItem *parent )
    : HbWidget(*new HbToolBarPrivate, parent)
{
    Q_D(HbToolBar);
    d->q_ptr = this;
    d->init();
    setFlag(QGraphicsItem::ItemIsPanel);
    setFlag(QGraphicsItem::ItemHasNoContents, true);
}

/*!
    Protected constructor.
*/
HbToolBar::HbToolBar( HbToolBarPrivate &dd, QGraphicsItem *parent )
    : HbWidget(dd, parent)
{
    Q_D(HbToolBar);
    d->q_ptr = this;
    d->init();
}

/*!
    Destructor
 */
HbToolBar::~HbToolBar()
{    
    if (!scene() || !scene()->property("destructed").isValid()) {
        foreach (QAction *action, actions()) {// krazy:exclude=qclasses
            HbAction* hbAction = qobject_cast<HbAction *>(action);
            if (hbAction){
                if (hbAction->toolBarExtension()){
                    hbAction->toolBarExtension()->deleteLater();
                }
                hbAction->setToolBarExtension(0);
            }
        }
    }
}

/*!
    Creates a new action with the given \a text and adds the action to
    the end of the toolbar. The toolbar takes ownership of the new action.

    \overload
    \return The new action.
*/
HbAction *HbToolBar::addAction( const QString &text )
{
    HbAction *action = new HbAction(text, this);
    addAction(action);
    return action;
}

/*!
    Creates a new action with the given \a icon and \a text and adds the
    action to the end of the toolbar. The toolbar takes ownership of the new action.

    \overload
    \return The new action.
*/
HbAction *HbToolBar::addAction( const HbIcon &icon, const QString &text )
{
    HbAction *action = new HbAction(icon, text, this);
    addAction(action);
    return action;
}

/*!
    Creates a new action with the given \a text, adds the action to the
    end of the toolbar, and connects the action's \link HbAction::triggered()
    triggered()\endlink signal to a receiver object's slot. The toolbar takes ownership
    of the new action.

    \overload
    \param text The text for the new action.
    \param receiver The object that is to receive the new action's signal.
    \param member The slot on the receiver to which the action's signal is to connect.
    \return The new action.
*/
HbAction *HbToolBar::addAction( const QString &text, const QObject *receiver, const char *member )
{
    HbAction *action = new HbAction(text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    Creates a new action with the given \a icon and \a text, adds the action
    to the end of the toolbar, and connects the action's \link HbAction::triggered()
    triggered()\endlink signal to a receiver object's slot. The toolbar takes ownership
    of the new action.

    \overload
    \param icon The image for the new action.
    \param text The text for the new action.
    \param receiver The object that is to receive the new action's signal.
    \param member The slot on the receiver to which the action's signal is to connect.
    \return The new action.
*/
HbAction *HbToolBar::addAction( const HbIcon &icon, const QString &text, const QObject *receiver, const char *member )
{
    HbAction *action = new HbAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    Adds \a extension as a toolbar extension at the end of the toolbar.

    \return The action that opens the new extension.

    \sa insertExtension()
*/
HbAction *HbToolBar::addExtension( HbToolBarExtension *extension )
{
    return insertExtension(0, extension);
}

/*!
    Inserts \a extension as a toolbar extension in front of the \a before action, provided
    it is a valid toolbar action. If \a before is 0 or is not valid, this function adds
    the extension to the end of the toolbar.

    \return The action that opens the new extension.

    \sa addExtension()
*/
HbAction *HbToolBar::insertExtension( HbAction *before, HbToolBarExtension *extension )
{    
    Q_D(HbToolBar);

    d->initToolBarExtension(extension);

    insertAction(before, extension->extensionAction());
    return extension->extensionAction();
}

/*!
    Returns the orientation of the toolbar.

    @beta

    \sa setOrientation()
*/
Qt::Orientation HbToolBar::orientation() const
{
    Q_D(const HbToolBar);
    return d->mOrientation;
}

/*!
    Sets the orientation of the toolbar.

    @beta

    \b Example:

    \code
    HbToolBar *toolBar = new HbToolBar();
    toolBar->setOrientation(Qt::Vertical);
    \endcode

    \sa orientation()
*/
void HbToolBar::setOrientation( Qt::Orientation orientation )
{
    Q_D(HbToolBar);

    d->setOrientation ( orientation );
    d->minimumToolButtonSize = QSizeF();
   
}

/*!
    \reimp
 */
QVariant HbToolBar::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D(HbToolBar);
    QVariant result = HbWidget::itemChange(change, value);

    switch (change) {
    case ItemVisibleHasChanged:
        if (d->emitVisibilityChangeSignal && value.toBool()) {
            QMetaObject::invokeMethod(&d->core, "visibilityChanged", Qt::DirectConnection);
            d->emitVisibilityChangeSignal = false;
        }
        break;
    case ItemVisibleChange:
        if (d->mOrientationEffectsRunning)
            return result;
        if (value.toBool()) {
            if (d->mDoLayoutPending && d->polished) {
                d->doLayout();
            }
            if (!d->mDialogToolBar) {
                d->doLazyInit();
                d->delayedStartEffects = d->mDoLayoutPending && !d->mSuppressNextAppearEffect;
                if (!d->delayedStartEffects && d->hasEffects && !d->mSuppressNextAppearEffect) {
                    d->startAppearEffect();
                }
                d->delayedHide = d->hasEffects;
            }
        } else {
            d->mSuppressNextAppearEffect = false;
            if(d->moreExtension && d->moreExtension->isVisible()){
               d->moreExtension->setVisible(false);
            }
            bool hideDelayed = d->delayedHide;
            if (d->delayedHide && d->hasEffects) { // about to hide and we wanna delay hiding
                // This check solves the situation where toolbar is hidden by its parent.
                // Delayed hide changes explicit bit in qgraphicsitem which makes
                // toolbar to stay hidden when parent becomes visible again.
                // There is a small misbehaviour when parentItem is hidden and hide was explicitely
                // called for toolbar. In this case toolbar hides without the effect.
                if ((!parentItem() || (parentItem() && parentItem()->isVisible()))) {
                    if (!d->hidingInProgress) { // Prevent reentrance
                        d->hidingInProgress = true;
                        d->startDisappearEffect();
                        d->delayedHide = false;
                    }
                } else {
                    d->delayedHide = false;
                }
            }
            if (hideDelayed) {
                return true;
            } else {
                d->delayedHide = d->hasEffects;
                d->hidingInProgress = false;
#ifdef HB_EFFECTS
                HbEffect::cancel(this, QString(), true);
#endif
            }
            if (d->mVisibleToolButtons.count()){
                d->emitVisibilityChangeSignal = true;
                QMetaObject::invokeMethod(&d->core, "visibilityChanged", Qt::QueuedConnection);
            }
        }
    break;
        default:
            break;
    }
    return result;
}

/*!
    \reimp
 */
void HbToolBar::changeEvent( QEvent *event )
{
    Q_D(HbToolBar);
    if (event->type() == QEvent::LayoutDirectionChange) {
        d->updateToolBarExtensions();
        d->updateButtonsLayoutDirection();
    } else if (event->type() == HbEvent::ThemeChanged) {        
        // forward change event to toolbuttons
        if (d->moreExtensionButton) {
            d->moreExtensionButton->event(event);
        }
        foreach(HbToolButton *button, d->mToolButtons) {
            button->event(event);
        }
    }

    QGraphicsWidget::changeEvent(event);
}

/*!
    \reimp
 */
void HbToolBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbToolBar);
    HbWidget::resizeEvent(event);
    if (d->polished && isVisible()) {
        d->updateToolBarForSizeChange();
    }
}

/*!
    \reimp
 */
void HbToolBar::hideEvent(QHideEvent *event)
{
    HbWidget::hideEvent(event);
}

/*!
    \reimp
 */
bool HbToolBar::event(QEvent *event)
{
    Q_D(HbToolBar);   
    switch( event->type() ) {        
    case QEvent::ActionAdded:
        d->actionAdded(static_cast<QActionEvent*>(event));
        return true;
    case QEvent::ActionRemoved:
        d->actionRemoved(static_cast<QActionEvent*>(event));
        return true;
    case QEvent::ActionChanged:
        // happens at least when action->setVisible(bool visible) is called
        if (d->polished && isVisible()) {
            d->resetVisibleButtonsList();
            d->doLayout();
        } else {
            d->mDoLayoutPending = true;
        }
        return true;
        default:
            return HbWidget::event(event);
    }    
}

/*!
    \reimp
 */
void HbToolBar::gestureEvent(QGestureEvent *)
{

}

/*!
    \reimp
 */
void HbToolBar::updatePrimitives()
{
    Q_D(HbToolBar);
    for (int i = 0; i < d->mVisibleToolButtons.count(); i++) {
        d->mVisibleToolButtons.at(i)->updatePrimitives();
    }
    if (d->moreExtensionButton) {
        d->moreExtensionButton->updatePrimitives();
    }
}

/*!
    \reimp
 */
void HbToolBar::polish(HbStyleParameters &params)
{
    Q_D(HbToolBar);
    bool resize = d->mDialogToolBar || testAttribute(Qt::WA_Resized) ||
                  !parentItem() || (parentLayoutItem() && !parentLayoutItem()->isLayout())
                  || (parentLayoutItem() && parentLayoutItem()->isLayout() && static_cast<QGraphicsLayout*>(parentLayoutItem())->isActivated())
                  || d->polished;
    if (d->mDoLayoutPending && isVisible() && resize) {
        d->initialButtonsPolish = false;
        d->doLayout();
    }
    HbWidget::polish(params);
}

#include "moc_hbtoolbar.cpp"
