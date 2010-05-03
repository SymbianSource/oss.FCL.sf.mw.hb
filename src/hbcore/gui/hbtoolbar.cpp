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
    \brief HbToolBar is a toolbar decorator.

    The HbToolBar class represents an HbView toolbar. It provides the interface for adding actions
    to the toolbar.
    Toolbar actions are added using one of the addAction() methods.
    Calling addAction() adds an HbToolButton to the toolbar and triggers the action when the button
    is pressed. The image and text specified with the action are applied to the toolbar button.

    HbToolBar also provides methods for adding pop-up toolbar extensions, represented by
    HbToolBarExtension objects.

    Example usage:
    \dontinclude ultimatecodesnippet/ultimatecodesnippet.cpp
    \skip Start of snippet 1
    \until End of snippet 1

*/

/*!
    \reimp
    \fn int HbToolBar::type() const
 */

// ======== MEMBER FUNCTIONS ========

/*!
    Constructs a tool bar with \a parent.
*/

HbToolBar::HbToolBar( QGraphicsItem *parent )
    : HbWidget(*new HbToolBarPrivate, parent)
{
    Q_D(HbToolBar);
    d->q_ptr = this;
    d->init();
    setFlag(QGraphicsItem::ItemIsPanel);

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
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar.
*/
HbAction *HbToolBar::addAction( const QString &text )
{
    HbAction *action = new HbAction(text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a icon and \a text. This
    action is added to the end of the toolbar.
*/
HbAction *HbToolBar::addAction( const HbIcon &icon, const QString &text )
{
    HbAction *action = new HbAction(icon, text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar. The action's \link HbAction::triggered()
    triggered()\endlink signal is connected to \a member in \a
    receiver.
*/
HbAction *HbToolBar::addAction( const QString &text, const QObject *receiver, const char *member )
{
    HbAction *action = new HbAction(text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is added to the end of the toolbar. The action's \link
    HbAction::triggered() triggered()\endlink signal is connected to \a
    member in \a receiver.
*/
HbAction *HbToolBar::addAction( const HbIcon &icon, const QString &text, const QObject *receiver, const char *member )
{
    HbAction *action = new HbAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, member);
    addAction(action);
    return action;
}

/*!
    This convenience function adds the \a extension as an extension popup
    to the toolbar. Returns the HbAction triggering the \a extension.
*/
HbAction *HbToolBar::addExtension( HbToolBarExtension *extension )
{
    return insertExtension(0, extension);
}

/*!
    This convenience function inserts the \a extension as an extension popup
    before the action \a before.

    It appends the action if \a before is 0 or \a before is not a valid
    action for this widget.

    Returns the HbAction triggering the \a extension.
*/
HbAction *HbToolBar::insertExtension( HbAction *before, HbToolBarExtension *extension )
{    
    Q_D(HbToolBar);

    d->initToolBarExtension(extension);

    insertAction(before, extension->extensionAction());
    return extension->extensionAction();
}

/*! @beta
    \brief  Returns the orientation of the tool bar.

    \sa setOrientation
 */
Qt::Orientation HbToolBar::orientation() const
{
    Q_D(const HbToolBar);
    return d->mOrientation;
}

/*! @beta
    Sets the \a orientation of the tool bar.

    \sa orientation
 */
void HbToolBar::setOrientation( Qt::Orientation orientation )
{
    Q_D(HbToolBar);

    d->setOrientation ( orientation );
    d->minimumToolButtonSize = QSizeF();
}

/*!
    \deprecated HbToolBar::unsetOrientation()
            is deprecated.
 */
void HbToolBar::unsetOrientation()
{
    //Q_D(HbToolBar);

}

/*!
    Emits areaChanged() whenever the tool bar's visibility or position changes.
*/
QVariant HbToolBar::itemChange( GraphicsItemChange change, const QVariant &value )
{
    Q_D(HbToolBar);
    QVariant result = HbWidget::itemChange(change, value);

    switch (change) {
    case ItemVisibleChange:
        if (d->mOrientationEffectsRunning)
            return result;
        if (value.toBool()) {
            if (d->mDoLayout && d->mDoLayoutPending) {
                d->doLayout();
            }
            if (d->emitVisibilityChangeSignal && value.toBool()) {
                QMetaObject::invokeMethod(&d->core, "visibilityChanged", Qt::QueuedConnection);
                d->emitVisibilityChangeSignal = false;
            }
            if (!d->mDialogToolBar) {
                d->doLazyInit();
                d->delayedStartEffects = d->mDoLayoutPending && !d->mSuppressNextAppearEffect;
                if (!d->delayedStartEffects && d->hasEffects && !d->mSuppressNextAppearEffect) {
                    d->startAppearEffect();
                }
                d->mSuppressNextAppearEffect = false;
                d->delayedHide = d->hasEffects;
            }
        } else {
            if (d->mVisibleToolButtons.count()){
                d->emitVisibilityChangeSignal = true;
                QMetaObject::invokeMethod(&d->core, "visibilityChanged", Qt::QueuedConnection);
             }
            if(d->moreExtension && d->moreExtension->isVisible()){
               d->moreExtension->setVisible(false);
            }
            if (d->delayedHide && d->hasEffects) { // about to hide and we wanna delay hiding
                if (!d->hidingInProgress) { // Prevent reentrance
                    d->hidingInProgress = true;
                    d->startDisappearEffect();
                }
            }
            if (d->delayedHide) {
                return true;
            } else {
                d->delayedHide = d->hasEffects;
                d->hidingInProgress = false;
#ifdef HB_EFFECTS
                        HbEffect::cancel(this, QString(), true);
#endif
            }
        }
    break;
        default:
            break;
    }
    return result;
}

/*!
    Reimplemented from QGraphicsWidget::changeEvent().
 */
void HbToolBar::changeEvent( QEvent *event )
{
    Q_D(HbToolBar);
    if (event->type() == QEvent::LayoutDirectionChange) {
        d->updateToolBarExtensions();
        d->updateButtonsLayoutDirection();
    }

    if (event->type() == QEvent::ParentChange && parentItem()) {
        setPos(-1000, -1000);   // Not very nice workaround to toolbar flicker problem.
                                // We will find a better solution later.
    }

    QGraphicsWidget::changeEvent(event);
}

/*!
    Reimplemented from QGraphicsWidget::resizeEvent().
 */
void HbToolBar::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_D(HbToolBar);
    HbWidget::resizeEvent(event);
    if (isVisible() && d->mDoLayout) {
        d->updateToolBarForSizeChange();
    }
}

/*!
    \reimp
 */
void HbToolBar::hideEvent(QHideEvent *event)
{
    Q_D(HbToolBar);
    if (d->mPressedDownButton && d->mPressedDownButton->isDown()) {
        d->mPressedDownButton->setDown(false);
        d->mPressedDownButton = 0;
        d->mPreviouslyPressedDownButton = d->mPressedDownButton;
    }
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

#include "moc_hbtoolbar.cpp"
