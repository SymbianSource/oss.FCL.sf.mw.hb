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

#include "hbtoolbutton.h"
#include "hbtoolbutton_p.h"
#include "hbtooltip.h"
#include "hbstyleoptiontoolbutton.h"
#include "hbtoolbarextension.h"
#include "hbtoolbarextension_p.h"
#include "hbaction.h"
#include "hbaction_p.h"
#include <hbglobal.h>
#include "hbcolorscheme.h"
#include "hbtextitem.h"
#include "hbiconitem.h"
#include "hbview.h"
#include "hbmainwindow.h"

#include "hbglobal_p.h" // remove when removing HB_DEPRECATED

#include <QGraphicsSceneHelpEvent>
#include <QGraphicsSceneMouseEvent>

/*!
    @stable
    @hbcore
    \class HbToolButton
    \brief The HbToolButton class provides a quick-access button for actions.

    A tool button is a special button that provides quick-access to
    a specific action. Unlike a normal push button HbPushButton, a tool
    button represents an action. In other words, HbToolButton is
    synchronized with an instance of HbAction.

    The action may also be associated with other parts of the user interface,
    as menu items and keyboard shortcuts. Sharing actions in this way helps
    make the user interface more consistent and is often less work to implement.

    Tool buttons are normally created indirectly when actions are added to a
    toolbar with HbToolBar::addAction(). It is also possible to
    construct tool buttons directly in the same way as any other widget, and
    arrange them alongside other widgets in layouts.

    The style of a tool button is adjustable with setToolButtonStyle().
    By default a tool button shows only an icon.

    A tool button's background is set as HbIcon. This makes it possible to
    specify different images for the normal and pressed states.

    Example usage:
    \code
    HbAction *action = new HbAction(icon, name, this);
    HbToolButton *toolButton = new HbToolButton(action, this);
    layout->addItem(toolButton);
    \endcode

    \sa HbAction, HbPushButton
*/

/*!
    \enum HbToolButton::ToolButtonStyle
    \deprecated HbToolButton::ToolButtonStyle

    This enum defines available tool button styles.

    The tool button style describes how the button's text and icon should be displayed.
 */

/*!
    \var HbToolButton::ToolButtonIcon
    \deprecated HbToolButton::ToolButtonIcon

    Only display the icon.
 */

/*!
    \var HbToolButton::ToolButtonText
    \deprecated HbToolButton::ToolButtonText

    Only display the text.
 */

/*!
    \var HbToolButton::ToolButtonTextAndIcon
    \deprecated HbToolButton::ToolButtonTextAndIcon

    Display both text and icon.
 */

/*!
    \fn void HbToolButton::triggered(HbAction *action)

    This signal is emitted when the \a action is triggered.
 */

/*!
    \reimp
    \fn int HbToolButton::type() const
 */

HbToolButtonPrivate::HbToolButtonPrivate() :
    action(0),
    textItem(0),
    iconItem(0),
    frameItem(0),
    customBackground(),
    backgroundVisible(true),
    buttonStyle(HbToolButton::ToolButtonIcon),
    toolBarPosition(HbStyleOptionToolButton::TB_None),
    orientation(Qt::Vertical),
    mDialogToolBar(false),
    toolbarExtensionFrame(false),
    mButtonSize(QSizeF())
{    
}

HbToolButtonPrivate::~HbToolButtonPrivate()
{
}

void HbToolButtonPrivate::createPrimitives()
{
    Q_Q(HbToolButton);
    if (backgroundVisible) {
        if (!frameItem){
            frameItem = q->style()->createPrimitive(HbStyle::P_ToolButton_frame, q);
        }
    } else if (frameItem) {
        delete frameItem;
        frameItem = 0;
    }

    if (action && !action->text().isEmpty()) {
        if (!textItem) {
            textItem = static_cast<HbTextItem *>(q->style()->createPrimitive(HbStyle::P_ToolButton_text, q));            
            textItem->setTextWrapping(Hb::TextWordWrap);
        }
        textItem->setVisible(buttonStyle & HbToolButton::ToolButtonText);
    } else if (textItem) {
        delete textItem;
        textItem = 0;
    }

    if (action && (buttonStyle & HbToolButton::ToolButtonIcon)) {
        if (!iconItem) {
            iconItem = q->style()->createPrimitive(HbStyle::P_ToolButton_icon, q);
        }
    } else if (iconItem){
        delete iconItem;
        iconItem = 0;
    }
}

void HbToolButtonPrivate::setOrientation(Qt::Orientation orientation)
{
    if (this->orientation != orientation) {
        this->orientation = orientation;
        Q_Q(HbToolButton);        
        if (q->isVisible() && polished) {
            q->repolish();
        }
    }    
}

void HbToolButtonPrivate::setToolBarPosition(HbStyleOptionToolButton::ToolBarPosition position)
{
    Q_Q(HbToolButton);
    if (toolBarPosition != position) {
        toolBarPosition = position;
        // required for toolbar()->action()[i]->setVisible(visible)
        // to work for all cases
        if (q->isVisible() && polished) {
            q->updatePrimitives();
        }
    }    
}

void HbToolButtonPrivate::setExtensionBackgroundVisible(bool visible)
{
    Q_Q(HbToolButton);
    if (toolbarExtensionFrame != visible) {
        toolbarExtensionFrame = visible;
        // required to make extension orientation switch from
        // landscape to portrait work correctly with automatic more
        // extension.
        q->repolish();
    }
}

void HbToolButtonPrivate::setBackgroundVisible(bool visible)
{
    Q_Q(HbToolButton);
    if (backgroundVisible != visible) {
        backgroundVisible = visible;
        // required to make extension orientation switch from
        // landscape to portrait work correctly with automatic more
        // extension.
        q->repolish();
    }
}

void HbToolButtonPrivate::setLayoutProperty(const char *name, bool value)
{
    Q_Q(HbToolButton);
    q->setProperty(name, value);
    if (q->isVisible() && polished) {
        q->repolish();
    }
}

QSizeF HbToolButtonPrivate::getMinimumSize()
{
    Q_Q(HbToolButton);
    mRepolishRequested = true;
    polishPending = false;
    q->updateGeometry();
    QSizeF size = q->minimumSize();
    //Workaround (causing extra polish)
    mSizeHintPolish = false;
    //workaround ends
    return size;
}

void HbToolButtonPrivate::_q_actionTriggered()
{
    Q_Q(HbToolButton);
    emit q->triggered(action);
}

void HbToolButtonPrivate::_q_actionChanged()
{
    Q_Q(HbToolButton);
    if (!action->icon().isNull()) {
        if (orientation == Qt::Horizontal) {
            buttonStyle = HbToolButton::ToolButtonIcon;
        } else if (!action->text().isEmpty()) {
            buttonStyle = HbToolButton::ToolButtonTextAndIcon;
        } else {
            buttonStyle = HbToolButton::ToolButtonIcon;
        }
    } else {
        buttonStyle = HbToolButton::ToolButtonText;
    }
    // action text/icon may have changed,            
    if (polished) {
        q->repolish();
    }
}

void HbToolButtonPrivate::showToolTip()
{
    Q_Q(HbToolButton);
    HbToolTip::showText(q->toolTip(), q, orientation == Qt::Vertical ? Qt::AlignTop : Qt::AlignLeft);
}

/*!
    Constructs a new HbToolButton with \a parent.
*/
HbToolButton::HbToolButton(QGraphicsItem *parent) :
    HbAbstractButton(*new HbToolButtonPrivate, parent)
{
    Q_D(HbToolButton);
    d->q_ptr = this;
}

/*!
    Constructs a new HbToolButton with \a action and \a parent.

    Ownership of the \a action is not transferred to the tool button.
 */
HbToolButton::HbToolButton(HbAction *action, QGraphicsItem *parent) :
    HbAbstractButton(*new HbToolButtonPrivate, parent)
{
    Q_D(HbToolButton);
    d->q_ptr = this;
    setAction(action);
    setProperty("state", "normal");
}

/*!
    Destructs the tool button.
 */
HbToolButton::~HbToolButton()
{
}

/*!
    Returns the action of the button.

    \sa setAction()
 */
HbAction *HbToolButton::action() const
{
    Q_D(const HbToolButton);
    return d->action;
}

/*!
    Sets the \a action of the button.

    Ownership of the \a action is not transferred to the tool button.

    \sa action()
 */
void HbToolButton::setAction(HbAction *action)
{
    Q_D(HbToolButton);
    if (d->action == action) {
        return;
    }

    if (d->action) {
        disconnect(d->action, SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
        disconnect(d->action, SIGNAL(changed()), this, SLOT(_q_actionChanged()));
    }

    HbAction *oldAction = d->action;
    d->action = action;
    if (d->action) {
        connect(action, SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
        connect(action, SIGNAL(changed()), this, SLOT(_q_actionChanged()));
    }

    if (isVisible() && d->polished) {
        // If action was null then there is a chance that the iconitem is not yet created.
        // If the new action is null then we may need to get rid of the icon completely.
        if ((!oldAction && action) || (oldAction && !action)) {
            repolish(); // will call createPrimitives()
        }
        updatePrimitives();
    }
}

/*!
    Returns the background image of the button.
 */
HbIcon HbToolButton::background() const
{
    Q_D(const HbToolButton);
    return d->customBackground;
}

/*!
    Sets the \a background image of the button.
 */
void HbToolButton::setBackground(const HbIcon &background)
{
    Q_D(HbToolButton);
    if (d->customBackground != background) {
        d->customBackground = background;
        if (isVisible() || d->polished)
            updatePrimitives();
    }
}

/*!
 \deprecated HbToolButton::toolButtonStyle() const
         is deprecated.

    @beta
    Returns the tool button style.

    The default value is \b HbToolButton::ToolButtonIcon.

    \sa setToolButtonStyle()
 */
HbToolButton::ToolButtonStyle HbToolButton::toolButtonStyle() const
{
    HB_DEPRECATED("HbToolButton::toolButtonStyle() is deprecated.");

    Q_D(const HbToolButton);
    return d->buttonStyle;
}

/*!
 \deprecated HbToolButton::setToolButtonStyle(HbToolButton::ToolButtonStyle)
         is deprecated.

    @beta
    Sets the tool button style.

    \sa toolButtonStyle()
 */
void HbToolButton::setToolButtonStyle(HbToolButton::ToolButtonStyle style)
{    
    HB_DEPRECATED("HbToolButton::setToolButtonStyle(HbToolButton::ToolButtonStyle style) is deprecated.");

    Q_D(HbToolButton);
    if (d->buttonStyle != style) {
        d->buttonStyle = style;

        // action text/icon may have changed,
        // primitives might need to be created/cleaned up
        if (size() != QSize(0, 0)) {
            repolish();
        }
    }
}

/*!

    \deprecated HbToolButton::primitive(HbStyle::Primitive)
         is deprecated.

    \reimp
 */
QGraphicsItem *HbToolButton::primitive(HbStyle::Primitive primitive) const
{
    Q_D(const HbToolButton);
    switch (primitive) {
        case HbStyle::P_ToolButton_frame:
            return d->frameItem;
        case HbStyle::P_ToolButton_icon:
            return d->iconItem;
        case HbStyle::P_ToolButton_text:
            return d->textItem;
        default:
            return 0;
    }
}

/*!
    \reimp
 */
void HbToolButton::updatePrimitives()
{
    Q_D(HbToolButton);

    HbStyleOptionToolButton option;
    if (d->action) {
        setCheckable(d->action->isCheckable());
        setChecked(d->action->isChecked());
        setEnabled(d->action->isEnabled());
        HbAction *hbAction = qobject_cast<HbAction *>(d->action);
        if (hbAction && (!hbAction->toolTip().isEmpty()) && (hbAction->toolTip() != toolTip())) {
            setToolTip(hbAction->toolTip());
        } else if(!hbAction && d->action->toolTip() != toolTip()) {
            setToolTip(d->action->toolTip());
        }
        setVisible(d->action->isVisible());
    }

    initStyleOption(&option);
    setProperty("dialogtoolbar", d->mDialogToolBar);
    if (d->frameItem) {
        style()->updatePrimitive(d->frameItem, HbStyle::P_ToolButton_frame, &option);
    }
    if (d->textItem) {
        style()->updatePrimitive(d->textItem, HbStyle::P_ToolButton_text, &option);
    }
    if (d->iconItem) {
        style()->updatePrimitive(d->iconItem, HbStyle::P_ToolButton_icon, &option);
        if (d->action && d->action->icon().flags() & HbIcon::Colorized) {
            static_cast<HbIconItem *>(d->iconItem)->setFlags(HbIcon::Colorized);
        }
    }
}

/*!
    Initializes \a option with the values from this HbToolButton. This method is useful for
    subclasses when they need a HbStyleOptionToolButton, but don't want to fill in all the
    information themselves.
 */
void HbToolButton::initStyleOption(HbStyleOptionToolButton *option)
{
    Q_D(HbToolButton);
    HbAbstractButton::initStyleOption(option);

    Q_ASSERT(option);
    option->customBackground = d->customBackground;
    option->backgroundVisible = d->backgroundVisible;
    option->toolBarPosition = d->toolBarPosition;
    option->orientation = d->orientation;
    option->isCheckable = d->checkable;
    option->useSecondaryGraphics = d->mDialogToolBar;
    option->useTransparentGraphics = false;
    if (mainWindow() && mainWindow()->currentView()) {
        if (mainWindow()->currentView()->viewFlags() & HbView::ViewTitleBarTransparent) {
            option->useTransparentGraphics = true;
        }
    }

    if (d->action) {
        option->text = d->action->text();
        option->icon = d->action->icon();        
        option->isToolBarExtension = d->toolbarExtensionFrame;
    }
}

/*!
    \internal
 */
HbToolButton::HbToolButton(HbToolButtonPrivate &dd, QGraphicsItem *parent) :
    HbAbstractButton(dd, parent)
{
}

/*!
    \reimp
 */
void HbToolButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbToolButton);
    QGraphicsWidget::resizeEvent(event);
    if (event->newSize() !=  event->oldSize() && d->polished && isVisible()) {
        updatePrimitives();
    }
    if (action() && action()->toolBarExtension()) {
        HbToolBarExtensionPrivate::d_ptr(action()->toolBarExtension())->placeToolBarExtension();
    }
}

/*!
    \reimp
 */
void HbToolButton::nextCheckState()
{
    Q_D(HbToolButton);
    HbAbstractButton::nextCheckState();
    if (!d->action) {
        return;
    }
    if ( d->action->toolBarExtension() ) {
        HbToolBarExtensionPrivate::d_ptr(d->action->toolBarExtension())->mExtendedButton = this;
        d->action->toolBarExtension()->show();
    }

    d->action->trigger();
}

/*!
    \reimp
 */
bool HbToolButton::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneHelp) {
        Q_D(HbToolButton);
        // Check whether toolbutton is inside a toolbar.
        if (d->toolBarPosition != HbStyleOptionToolButton::TB_None) {
            d->showToolTip();
            event->accept();
            return true;
        }
    }
    return HbAbstractButton::sceneEvent(event);
}

/*!
    \reimp
 */
bool HbToolButton::event(QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        return true;
    }

    return HbAbstractButton::event(event);
}

void HbToolButton::polish(HbStyleParameters &params)
{
    Q_D(HbToolButton);
    setProperty("orientation", d->orientation);
    d->createPrimitives();
    updatePrimitives();
    HbAbstractButton::polish(params);
    // workaround for caching problem
    setMinimumSize(minimumSize());
    // workaround ends
}

#include "moc_hbtoolbutton.cpp"
