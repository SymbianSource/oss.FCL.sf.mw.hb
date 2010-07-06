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

    An example of how to handle if finished(int) is connected instead of finished(HbAction*) in above example.
    \snippet{ultimatecodesnipped/ultimatecodesnippet.cpp,55}

    An example of how to create a non-modal dialog and show it.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,26}

*/

/*!
    \fn void HbDialog::finished( int code )

    This signal is emitted when the dialog is closed.
    This will have the HbDialog::DialogCode as the parameter code.

    \sa done(), accept(), reject()
*/
/*!
    \fn void HbDialog::finished( HbAction *action )

    This signal is emitted when an action has been triggered in a dialog.
    The parameter will be the triggered action.
  */
/*!
    \fn void HbDialog::accepted( )

    This signal is emitted when the dialog is closed and the user
    has accepted the dialog. which implies that either action has triggered
    or through function call the accept method is called, causing this signal.

    \sa done(), accept(), reject()
*/
/*!
    \fn void HbDialog::rejected( )

    This signal is emitted when the dialog is closed and the user
    has rejected the dialog. which implies that either action triggered
    or through function call the reject method is called, causing this signal.

    \sa done(), accept(), reject()
*/


/*!
    \reimp
    \fn int HbDialog::type() const
 */

HbDialogPrivate::HbDialogPrivate( ) :
    contentWidget(0),
    headingWidget(0),
    primaryAction(0),
    secondaryAction(0),
    closingAction(0),
    toolBar(0)
{
}

HbDialogPrivate::~HbDialogPrivate()
{
}

/*!
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
 Utility function removes the spaces from string if any
*/
void HbDialogPrivate::removeSpaces(QString& string)
{
    QString tempStr(string);
    string.clear();
    foreach(QChar ch, tempStr)
    {
        if(!ch.isSpace())
            string.append(ch);
    }
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
    d->timeout = HbPopupPrivate::timeoutValue(HbPopup::NoTimeout);
}

/*!
    \internal
 */
HbDialog::HbDialog(HbDialogPrivate &dd, QGraphicsItem *parent) :
    HbPopup(dd, parent)
{
    Q_D(HbDialog);
    d->q_ptr = this;
    d->timeout = HbPopupPrivate::timeoutValue(HbPopup::NoTimeout);
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
    if (d->headingWidget == headingWidget)
        return;
    if (d->headingWidget)
        delete d->headingWidget;
    d->headingWidget = headingWidget;
    if (headingWidget) {
        setProperty("heading_layout", true);
        headingWidget->setParentItem(this);
        HbStyle::setItemName(headingWidget,"heading");
    } else {
        setProperty("heading_layout", false);
    }
    repolish();
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

    if (d->contentWidget == contentWidget)
        return;
    if (d->contentWidget)
        delete d->contentWidget;
    prepareGeometryChange(); // needed to paint screen properly
    d->contentWidget = contentWidget;
    if (contentWidget) {
        contentWidget->setParentItem(this);
        HbStyle::setItemName(contentWidget,"content");
    }
    repolish();    
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
    This is a slot which shows the dialog and returns immediately.

    \sa open(QObject*,const char*)
*/
void HbDialog::open()
{
    open(0,0);
}
/*!

 Shows the dialog as modal dialog returning immediately.  

 Connects finished(HbAction*) or finished(int) signal to the slot specified by \a receiver and
 \a member. The signal will be disconnected from the slot when the
 popup is closed. disambiguation between which method to connect to is done at runtime.

 For non modal popups, use show().  
*/

void HbDialog::open( QObject* receiver, const char* member )
{
    Q_D(HbDialog);
    if ( receiver && member ) {

        QString myStr(member);
        d->removeSpaces(myStr);
        if(myStr.contains("(int)")) {
            connect( this, SIGNAL(finished(int)), receiver, member );
        }
        else {
            connect( this, SIGNAL(finished(HbAction*)), receiver, member );
        }
        d->receiverToDisconnectOnClose = receiver;
        d->memberToDisconnectOnClose = member;
    } else {
        d->receiverToDisconnectOnClose = 0;
        d->memberToDisconnectOnClose.clear();
    }
    show();
}
/*!
  Closes the dialog and emits finished ,accepted and rejected signals appropriately.

  If the dialog is accepted the code is HbDialog::Accepted, if it is rejected code
  is HbDialog::Rejected.
  As with HbWidget::close(), done() deletes the dialog if the
  Qt::WA_DeleteOnClose flag is set. 

  \sa accept(), reject()
*/
void HbDialog::done( int code )
{  
    HbAction *action=qobject_cast<HbAction*>(sender());
    if(!action) {
        close();
        //if there is no sender or if there is some sender which is not hbaction
        //then we need to close the dialog when done is called.
    }
    else if(actions().contains(action)==false) {
        close();
        //if our actions done have this HbAction. then we need to call the
        //close method explicitly.
    } //otherwise close will be called automatically due to connection in base class
    
    emit finished(code);
    if(code == Accepted) {
        emit accepted();
    }
    else if(code == Rejected) {
        emit rejected();
    }
}
/*!
  Hides the modal dialog and emits finished(HbDialog::Accepted),accepted() and finished(HbAction*) signals.

  \sa reject(), done()
*/
void HbDialog::accept()
{
    done(Accepted);
}
/*!
  Hides the modal dialog and emits finished(HbDialog::Rejected),rejected() and finished(HbAction*) signals.

  \sa accept(), done()
*/
void HbDialog::reject()
{
    done(Rejected);
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
            d->toolBar->setParentItem(this);
            HbStyle::setItemName(d->toolBar ,"controls");
            setProperty("controls_layout", true);
            d->toolBar->setOrientation(Qt::Horizontal);
            HbToolBarPrivate::d_ptr(d->toolBar)->mDialogToolBar = true;
            repolish();
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
               d->toolBar->deleteLater();
               d->toolBar = 0;
               setProperty("controls_layout", false);
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
