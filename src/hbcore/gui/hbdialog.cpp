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

#include "hbdialog.h"
#include "hbdialog_p.h"
#include "hbinstance.h"
#include "hbaction.h"
#include "hbstyleoptionpopup_p.h"
#include "hbdeviceprofile.h"
#include "hbevent.h"
#include "hbtoolbar_p.h"
#include "hbglobal_p.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QShowEvent>
#include <QHideEvent>
#include <QEventLoop>
#include <QPointer>
#include <QDebug>
#include <QGraphicsLinearLayout>
#include <QApplication> // krazy:exclude=qclasses

#include <hbfeedbackmanager.h>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#define HB_POPUP_ITEM_TYPE "HB_POPUP"
#endif

/*!
    @beta
    @hbcore
    \class HbDialog
    \brief HbDialog is a base class for different dialogs in Hb library.

    \image html hbpopup.png "A dialog with a header widget, a list as a content widget, and two action buttons."

    HbDialog is a concrete class. The content for a custom dialog is
    implemented in a separate widget, which is set to the dialog with
    method setContentWidget().


    An example of how to create a simple modal dialog and show it.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,13}

    An example of how to handle dialog signals from previous example.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,53}

    An example of how to create a non-modal dialog and show it.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,26}

*/

/*!
    \reimp
    \fn int HbDialog::type() const
 */

HbDialogPrivate::HbDialogPrivate( ) :
    contentWidget(0),
    headingWidget(0),
    mainLayout(new QGraphicsLinearLayout(Qt::Vertical)),
    primaryAction(0),
    secondaryAction(0),
    closingAction(0),
    toolBar(0)
{
}

HbDialogPrivate::~HbDialogPrivate()
{
}

void HbDialogPrivate::init()
{
    Q_Q(HbDialog);

    // Content is responsible of adding spacings & margins
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0,0,0,0);

    q->setLayout(mainLayout);
    mainLayout->setMinimumSize(0, 0);    
}

void HbDialogPrivate::setWidget(int layoutIndex, QGraphicsWidget *&destWidget, QGraphicsWidget *widget)
{
    Q_Q(HbDialog);
    if (destWidget != widget) {
        if (destWidget) {
            mainLayout->removeItem(destWidget);
            delete destWidget;
            destWidget = 0;
        }
        if (widget) {
            destWidget = widget;
            destWidget->setParentItem(q);
            mainLayout->insertItem(layoutIndex, widget);
            mainLayout->setAlignment(widget, Qt::AlignCenter);
        }

        doLayout();
    }
}

/*
  Relayouts the popup. If expandSize is true it the new calculated size of the popup
  cannot be smaller than the current size.
*/
void HbDialogPrivate::doLayout()
{
    Q_Q(HbDialog);
    if(q->mainWindow() && contentWidget)
    {
        if ( q->mainWindow()->layoutDirection() != contentWidget->layoutDirection() ) {
           contentWidget->setLayoutDirection(q->mainWindow()->layoutDirection());
        }
    }

    if (q->mainWindow() && headingWidget) {
        if (q->mainWindow()->layoutDirection() != headingWidget->layoutDirection()) {
            headingWidget->setLayoutDirection(q->mainWindow()->layoutDirection());
        }
    }

    q->updateGeometry();
}


/*!
 Constructs a dialog with given  \a parent graphics item.\n
 Note: dialogs with \a parent set as 0 are behaving as real popups. 
 This is actually the intended use. \sa HbPopup::HbPopup
*/
HbDialog::HbDialog(QGraphicsItem *parent) :
    HbPopup(*new HbDialogPrivate, parent)
{
    Q_D(HbDialog);
    d->q_ptr = this;
    d->init();
}

/*!
    \internal
 */
HbDialog::HbDialog(HbDialogPrivate &dd, QGraphicsItem *parent) :
    HbPopup(dd, parent)
{
    Q_D(HbDialog);
    d->q_ptr = this;
    d->init();
}

/*!
 Destroys the popup.
*/
HbDialog::~HbDialog()
{
}

/*!
 Returns the widget which is being added to the heading area
 \sa setHeadingWidget()
*/
QGraphicsWidget * HbDialog::headingWidget() const
{
    Q_D(const HbDialog);
    return (d->headingWidget);
}

/*!
 Adds \a widget to the heading area. Ownership of the widget is transferred
 to popup. If \a headingWidget is 0 the heading widget is removed.
 \sa headingWidget()
*/
void HbDialog::setHeadingWidget(QGraphicsWidget *headingWidget)
{
    Q_D(HbDialog);
    HbStyle::setItemName(headingWidget,"heading");
    d->setWidget(0, d->headingWidget, headingWidget);
}

/*!
 Returns the content widget property of the popup.
 HbDialog only draws a bordered rect, the rest is drawn by the content widget.
 \sa setContentWidget()
*/
QGraphicsWidget *HbDialog::contentWidget() const
{
    Q_D(const HbDialog);
    return d->contentWidget;
}

/*!
 Sets the content widget property of the popup.
 HbDialog only draws a bordered rect, the rest is drawn by the content widget.
 Ownership of the widget is transferred
 to popup. If \a contentWidget is 0 the content widget is removed.
 \sa contentWidget()
*/
void HbDialog::setContentWidget(QGraphicsWidget *contentWidget)
{
   Q_D(HbDialog);
   HbStyle::setItemName(contentWidget,"content");
   d->setWidget((d->headingWidget?1:0), d->contentWidget, contentWidget);
}

/*!
 \deprecated HbDialog::primaryAction() const
       is deprecated.
 It returns the primary action added to the control area
 \sa setPrimaryAction()
*/
HbAction* HbDialog::primaryAction() const
{
    HB_DEPRECATED("HbAction* HbDialog::primaryAction() is deprecated. Use QGraphicsWidget action api instead");
    Q_D(const HbDialog);
    return d->primaryAction;
}

/*!
 \deprecated HbDialog::setPrimaryAction(HbAction*)
           is deprecated. Please use QGraphicsWidget::addAction() family of functions instead.
 It adds the given action to the control area.
 It is added to the left side of the control area if the layout direction of the application
 is left-to-right and in the vice-versa if the layout direction of the application
 is right-to-left.
 \sa primaryAction()
*/
void HbDialog::setPrimaryAction(HbAction *action)
{
    HB_DEPRECATED("HbDialog::setPrimaryAction(HbAction *action) is deprecated. Use QGraphicsWidget action api instead");
    Q_D(HbDialog);
    if (d->primaryAction && action != d->primaryAction) {
        removeAction(d->primaryAction);
    }
    d->primaryAction = action;
    QAction *before = 0;        //krazy:exclude=qclasses
    if (actions().count()) {
        before = actions().first();
    }
    insertAction(before, action);
}

/*!
 \deprecated HbDialog::secondaryAction() const
            is deprecated.
 It returns the secondary action added to the control area
 \sa setSecondaryAction()
*/
HbAction* HbDialog::secondaryAction() const
{
    HB_DEPRECATED("HbAction* HbDialog::secondaryAction() is deprecated. Use QGraphicsWidget action api instead");
    Q_D(const HbDialog);
    return(d->secondaryAction);
}

/*!
 \deprecated HbDialog::setSecondaryAction(HbAction*)
           is deprecated. Please use QGraphicsWidget::addAction() family of functions instead.
 It adds the given action to the control area.
 It is added to the right side of the control area if the layout direction of the application
 is left-to-right and in the vice-versa if the layout direction of the application
 is right-to-left.
 \sa secondaryAction()
*/
void HbDialog::setSecondaryAction(HbAction *action)
{
    HB_DEPRECATED("HbDialog::setSecondaryAction(HbAction *action) is deprecated. Use QGraphicsWidget action api instead");
    Q_D(HbDialog);
    if (d->secondaryAction && action != d->secondaryAction) {
        removeAction(d->secondaryAction);
    }
    d->secondaryAction = action;
    addAction(action);
}

/*!

 Shows the dialog as modal dialog returning immediately.  

 Connects finished(HbAction*) signal to the slot specified by \a receiver and
 \a member. The signal will be disconnected from the slot when the
 popup is closed.

 For non modal popups, use show().  
*/

void HbDialog::open( QObject* receiver, const char* member )
{
    Q_D(HbDialog);
    if ( receiver && member ) {
        connect( this, SIGNAL(finished(HbAction*)), receiver, member );
        d->receiverToDisconnectOnClose = receiver;
        d->memberToDisconnectOnClose = member;
    } else {
        d->receiverToDisconnectOnClose = 0;
        d->memberToDisconnectOnClose.clear();
    }
    show();
}

/*!
 \reimp
*/
//
// Sets the focus to its content widget.
//
void HbDialog::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    if(contentWidget()) {
        contentWidget()->setFocus();
    }
}

/*!
    \reimp
 */
void HbDialog::closeEvent ( QCloseEvent * event )
{
    Q_D(HbDialog);

    HbAction *closingAction = qobject_cast<HbAction *>(sender());
    if (closingAction && actions().contains(closingAction)) {
        d->closingAction = closingAction;
        emit finished( d->closingAction );
    } else {
        HbAction* nullAction(0);
        emit finished( nullAction );
    }
    if (d->receiverToDisconnectOnClose) {
        if (disconnect(this, SIGNAL(finished(HbAction*)),
                       d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose)) {
            d->receiverToDisconnectOnClose = 0;
            d->memberToDisconnectOnClose.clear();
        }
    }

    HbPopup::closeEvent(event);
}

/*!
    \reimp
 */
void HbDialog::changeEvent(QEvent *event)
{
    Q_D(HbDialog);
    if (event->type() == QEvent::StyleChange ||
        event->type() == QEvent::LayoutDirectionChange) {
        d->doLayout();
    }

    HbPopup::changeEvent(event);
}

/*!
    \reimp
 */
bool HbDialog::event(QEvent *event)
{
    Q_D(HbDialog);
    if(event->type() != QEvent::ShortcutOverride && event->type() != QEvent::GestureOverride)
        event->accept();

    if (event->type() == QEvent::ActionAdded) {
        if (!d->toolBar) {
            // TODO: HbToolBar private interface should make it possible to choose
            // different graphics for tool buttons.            
            d->toolBar = new HbToolBar();
            HbStyle::setItemName(d->toolBar ,"controls");
            d->toolBar->setParentItem(this);
            d->toolBar->setOrientation(Qt::Horizontal);
            HbToolBarPrivate::d_ptr(d->toolBar)->mDialogToolBar = true;
            // prevent stretching buttons, should the content be small
            // but dialog size forcibly large
            d->mainLayout->addStretch();
            d->mainLayout->addItem(d->toolBar);
        }
        QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
        d->toolBar->insertAction (actionEvent->before(), actionEvent->action());
        if (!parentItem()) { // only for popup without parent
            connect(actionEvent->action(), SIGNAL(triggered()), this, SLOT(close()));
        }
        d->doLayout();
        return true;

    } else if (event->type() == QEvent::ActionRemoved) {

        QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
        // remove primary or secondary action if set
        if (actionEvent->action() == d->primaryAction) {
            d->primaryAction = 0;
        } else if (actionEvent->action() == d->secondaryAction) {
            d->secondaryAction = 0;
        }
        disconnect(actionEvent->action(), 0, this, 0);

        if (d->toolBar) {
           d->toolBar->removeAction(actionEvent->action());
           if (!d->toolBar->actions().count()) {
               d->mainLayout->removeItem(d->toolBar);
               d->toolBar->deleteLater();
               d->toolBar = 0;
           }
        }
        d->doLayout();
        return true;
 
    } else if (event->type() == HbEvent::LayoutRequest) {
        if (d->toolBar
            && mainWindow()
            && d->toolBar->layoutDirection() != mainWindow()->layoutDirection()
            && !d->toolBar->testAttribute(Qt::WA_SetLayoutDirection)) {
            d->toolBar->setLayoutDirection(mainWindow()->layoutDirection());
            d->toolBar->setAttribute(Qt::WA_SetLayoutDirection, false);
        }
        d->doLayout();
    }
    return HbPopup::event(event);
}

/*!
    \reimp
*/
QSizeF HbDialog::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF hint = HbPopup::sizeHint(which, constraint);

    if (which == Qt::PreferredSize) {
        Q_D(const HbDialog);
        if (d->contentWidget) {
            QSizePolicy policy = d->contentWidget->sizePolicy();
            if (policy.horizontalPolicy() & QSizePolicy::ExpandFlag) {
                hint.setWidth(QWIDGETSIZE_MAX);
            }

            if (policy.verticalPolicy() & QSizePolicy::ExpandFlag) {
                hint.setHeight(QWIDGETSIZE_MAX);
            }
        }
    }

    return hint;
}

#include "moc_hbdialog.cpp"
