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
#include "hbstyletextprimitivedata.h"
#include "hbstyleframeprimitivedata.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QShowEvent>
#include <QHideEvent>
#include <QEventLoop>
#include <QPointer>
#include <QDebug>
#include <QApplication> // krazy:exclude=qclasses

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

/*!
    @beta
    @hbcore
    \class HbDialog
    \brief The HbDialog class is a concrete class for simple application dialogs
     and a base class for a variety of convenience application dialog and popup classes.
    
    An application dialog is a popup that opens in response to application events.
    Popup dialogs float above the top layer of the application view, usually overlapping 
    the area reserved for the application content. An application can use a dialog to 
    provide information, give warnings, and ask the user to answer a question or 
    select an option. Avoid overusing dialogs because they can interrupt the user's 
    workflow. When dialogs appear too frequently, users tend to dismiss them without 
    reading them properly, just to get them out of the way.
        
    %HbDialog is a concrete class that you can use directly in your applications. 
    However, there are a variety of convenience classes that derive from it, 
    so check whether there is one that meets your needs. You can create a custom
    dialog by deriving a class from %HbDialog and adding a custom widget to it.

    \image html hbpopup.png "A dialog with a heading widget, a list content widget, and two action buttons"

    You can add three types of content to an %HbDialog object. If you do not add any
    content, an %HbDialog object is simply a bordered rectangle. The three types of
    content are:

    - <b>A heading widget</b>. This is shown at the top of the dialog and is typically
      a simple HbLabel object that describes the purpose of the dialog. To add a heading 
      widget, call setHeadingWidget().
    
    - <b>A content widget</b>. Positioned below the heading, this contains the main 
      dialog content. To add a content widget, call setContentWidget(). 

    - \b Actions. These are generic command (HbAction) objects, which you add to the 
      dialog by calling the QGraphicsWidget::addAction() family of functions. The actions 
      appear as buttons on a toolbar at the bottom of the dialog. (The toolbar appears when 
      you add the first action to the dialog.) When the user taps one of these buttons, it 
      dismisses the dialog and the action's \link HbAction::triggered() triggered()\endlink 
      signal is emitted. Connect this signal to the slot that will perform the command. 

    %HbDialog is derived from HbPopup, which provides generic features for all popup 
    classes. These features include properties that control the dialog's dismiss policy, 
    background fade policy, timeout, frame background type, and whether it is modal.
    
    Unlike a non-modal dialog, a modal dialog stops the user interacting with anything outside
    of the dialog until it closes. You set and get the modality by calling setModal() 
    and isModal(), respectively. Typically modal dialogs use the fade background feature, 
    whereas non-modal dialogs do not. How you open a dialog depends on whether it is
    modal: if the dialog is modal use one of the open() overloads and otherwise use 
    \link QGraphicsItem::show() show()\endlink. Make sure you use the appropriate method, 
    because this ensures the correct touch event handling.

    \note Services that generate device-wide and system-wide messages use the HbDeviceDialog 
          class and not HbDialog.

    \section _usecases_hbdialog Using the HbDialog class
    
    \subsection _uc_001_hbdialog Creating and showing a simple modal dialog
    
    This example demonstrates how to create a simple modal dialog and show it. Notice
    that we show a modal dialog by calling one of the open() overloads.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,13}

    \subsection _uc_002_hbdialog Handling dialog signals

    This example continues the previous one, which connects the dialog's 
    finished(HbAction*) signal to the following slot in the call to 
    open(QObject *,const char*) on the final line. This example shows how this slot
    handles the finished(HbAction*) signal.
    
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,53}

    \subsection _uc_003_hbdialog Handling dialog signals: alternative method

    This example demonstrates an alternative way of handling dialog signals.
    This example assumes that the finished(int) overload is connected instead of 
    finished(HbAction*); for example, by using open(QObject *, const char *), like
    this:
    
    \code
    dialog->open(this, SLOT(dialogClosed(int)));
    \endcode
    
    The finished(int) signal is emitted by calling accept() and reject() or done().
    
    Here is an example of a function that handles finished(int):
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,58}

    \subsection _uc_004_hbdialog Creating and showing a non-modal dialog

    This final example shows how to create a non-modal dialog and show it.
    Notice that we display a non-modal dialog by calling \link QGraphicsItem::show() 
    show()\endlink.
    \snippet{ultimatecodesnippet/ultimatecodesnippet.cpp,26}

    \sa HbPopup, HbDeviceDialog, HbInputDialog, HbMessageBox, HbProgressDialog, 
        HbNotificationDialog, HbColorDialog, HbSelectionDialog, HbVolumeSliderPopup, 
        HbZoomSliderPopup
*/

/*!
    \enum HbDialog::DialogCode
    Indicates whether the dialog was accepted or rejected.
    
    \sa finished(int), done()
    
 */

/*!
    \var HbDialog::Rejected
    Indicates that the user selected the Cancel action (or equivalent).
 */

/*!
    \var HbDialog::Accepted
    Indicates that the user selected the OK action (or equivalent).
 */

/*!
    \fn void HbDialog::finished( int code )

    This signal is emitted when the dialog closes. \a code is an 
    HbDialog::DialogCode value.

    \sa done(), accept(), reject()
*/
/*!
    \fn void HbDialog::finished( HbAction *action )

    This signal is emitted when an action is triggered in a dialog.
    The parameter is the action that was triggered.
  */
/*!
    \fn void HbDialog::accepted( )

    This signal is emitted by a call to accept() or to \link done() 
    done(HbDialog::Accepted)\endlink. Typically this means that the user 
    has selected the OK button (or equivalent).

    \sa done(), accept(), reject()
*/
/*!
    \fn void HbDialog::rejected( )
    
    This signal is emitted by a call to reject() or to \link done() 
    done(HbDialog::Rejected)\endlink. Typically this means that the user 
    has selected the Cancel action (or equivalent).

    \sa done(), accept(), reject()
*/

/*!
    \fn int HbDialog::type() const
 */

HbDialogPrivate::HbDialogPrivate( ) :
    contentWidget(0),
    headingWidget(0),
    primaryAction(0),
    secondaryAction(0),
    closingAction(0),
    toolBar(0),
    dismissOnAction(true),
    headingFrameItem(0)
{
}

HbDialogPrivate::~HbDialogPrivate()
{
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

void HbDialogPrivate::createHeadingBackground()
{
    Q_Q(HbDialog);
    if (headingFrameItem) {
        delete headingFrameItem;
        headingFrameItem = 0;
    }
    headingFrameItem = q->style()->createPrimitive(HbStyle::PT_FrameItem, "background", q);
    HbStyleFramePrimitiveData data;
    q->initPrimitiveData(&data, headingFrameItem);
    data.fillWholeRect = true;
    data.frameType = HbFrameDrawer::ThreePiecesHorizontal;
    if (mFullScreen) {
        data.frameGraphicsName = QLatin1String("qtg_fr_fullscreen_heading");
    } else {
        data.frameGraphicsName = QLatin1String("qtg_fr_popup_heading");
    }
    q->style()->updatePrimitive(headingFrameItem, &data, q);
    headingFrameItem->update();
}

void HbDialogPrivate::_q_actionTriggered() {
    Q_Q(HbDialog);
    if (dismissOnAction) {
        q->close();
    }
}

/*
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

void HbDialogPrivate::setFullScreen(bool enable)
{
    //If the dialog is in fullscreen, normal toolbar graphics will be used
    if (enable) {
        if (toolBar) {
            HbToolBarPrivate::d_ptr(toolBar)->updateDialogToolbar(false);
        }
    } else {
        if (toolBar){
            HbToolBarPrivate::d_ptr(toolBar)->updateDialogToolbar(true);
        }
    }
    HbPopupPrivate::setFullScreen(enable);
    //If heading is available, update fullscreen heading graphics
    if (!headingText.isEmpty() || headingWidget) {
        createHeadingBackground();
    }
}

/*!
    Constructs a dialog with the given \a parent. For a popup dialog (which 
    means that it opens above all other objects, at the highest z-order), 
    set \a parent to 0. This is the primary intended use of this class.

    If \a parent is non-zero, the dialog is embedded in the parent QGraphicsItem. 
    The properties provided by HbPopup are then ignored and the aboutToClose() signal is 
    not emitted.

    \sa HbPopup::HbPopup()
*/
HbDialog::HbDialog(QGraphicsItem *parent) :
    HbPopup(*new HbDialogPrivate, parent)
{
    Q_D(HbDialog);
    d->q_ptr = this;
    d->pendingTimeout = HbPopup::NoTimeout;
}

/*!
    \internal
 */
HbDialog::HbDialog(HbDialogPrivate &dd, QGraphicsItem *parent) :
    HbPopup(dd, parent)
{
    Q_D(HbDialog);
    d->q_ptr = this;
    d->pendingTimeout = HbPopup::NoTimeout;
}

/*!
    Destructor.
*/
HbDialog::~HbDialog()
{
}

/*!
    Returns the heading widget, if one has been added to the dialog; otherwise this
    function returns null.
    \sa setHeadingWidget()
*/
QGraphicsWidget * HbDialog::headingWidget() const
{
    Q_D(const HbDialog);
    return (d->headingWidget);
}

/*!
    Adds \a headingWidget to the heading area of the dialog. This transfers ownership 
    of the heading widget to the dialog. 
    
    To remove a heading widget from a dialog, set \a headingWidget to 0.
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
    d->headingText = QString();
    if (headingWidget) {
        setProperty("heading_layout", true);
        d->headingWidget->setParentItem(this);
        d->headingWidget->setZValue(zValue()+1);
        HbStyle::setItemName(headingWidget,"heading");
    } else {
        setProperty("heading_layout", false);
    }

    if (headingWidget) {
        d->createHeadingBackground();
    } else {
        if (d->headingFrameItem) {
            delete d->headingFrameItem;
            d->headingFrameItem = 0;
        }
    }
    repolish();
}

/*!
    Returns the heading text, if one has been added to the dialog; otherwise this
    function returns empty string.
    \sa setHeadingText()
*/
QString HbDialog::headingText() const
{
    Q_D(const HbDialog);
    return d->headingText;
}

/*!
    Sets \a heading as the title of the dialog.

    To remove the title from the dialog, set an empty string as \a heading.
    \sa headingText()
*/
void HbDialog::setHeadingText(const QString &heading)
{
    Q_D(HbDialog);
    if (d->headingWidget) {
        delete d->headingWidget;
        d->headingWidget = 0;
    }

    d->headingText = heading;

    if (!heading.isEmpty()) {
        d->headingWidget = qobject_cast<QGraphicsWidget *>(style()->createPrimitive(HbStyle::PT_TextItem, "heading", this));
        d->headingWidget->setZValue(zValue()+1);
        HbStyleTextPrimitiveData data;
        initPrimitiveData(&data, d->headingWidget);
        data.text = heading;
        style()->updatePrimitive(d->headingWidget, &data, this);
        d->headingWidget->update();

        d->createHeadingBackground();

        setProperty("heading_layout", true);
    } else {
        setProperty("heading_layout", false);
        if (d->headingFrameItem) {
            delete d->headingFrameItem;
            d->headingFrameItem = 0;
        }
    }
    repolish();
}

/*!
    Returns the content widget, if one has been added to the dialog; otherwise this 
    function returns null.
    
    The content widget provides the main content for the dialog. Without a content
    widget (and a heading widget and some actions), an HbDialog object is simply a 
    bordered rectangle.
    \sa setContentWidget()
*/
QGraphicsWidget *HbDialog::contentWidget() const
{
    Q_D(const HbDialog);
    return d->contentWidget;
}

/*!
    Adds \a contentWidget to the dialog. This transfers ownership of the content 
    widget to the dialog. 
    
    To remove a content widget from a dialog, set \a headingWidget to 0.
    
    \sa contentWidget()
*/
void HbDialog::setContentWidget(QGraphicsWidget *contentWidget)
{
    Q_D(HbDialog);

    if (d->contentWidget == contentWidget)
        return;
    if (d->contentWidget)
        delete d->contentWidget;
    d->contentWidget = contentWidget;
    if (contentWidget) {
        contentWidget->setParentItem(this);
        if (contentWidget->inherits("HbAbstractItemView")) {
            setProperty("list_content", true);
        } else {
            setProperty("list_content", false);
        }
        HbStyle::setItemName(contentWidget,"content");
    }
    repolish();    
}

/*!
    \deprecated This function is deprecated. 
    
    Returns the action added by calling setPrimaryAction().
*/
HbAction* HbDialog::primaryAction() const
{
    HB_DEPRECATED("HbAction* HbDialog::primaryAction() is deprecated. Use QGraphicsWidget action api instead");
    Q_D(const HbDialog);
    return d->primaryAction;
}

/*!
    Returns the \c dismissOnAction property, which controls whether the dialog closes 
    when one of its actions is triggered.
    \sa setDismissOnAction()
*/
bool HbDialog::dismissOnAction() const
{
    Q_D(const HbDialog);
    return d->dismissOnAction;
}

/*!
    Sets the \c dismissOnAction property, which controls whether the dialog closes 
    when one of its actions is triggered.
    \sa dismissOnAction()
*/
void HbDialog::setDismissOnAction( bool dismissOnAction ) 
{
    Q_D(HbDialog);
    d->dismissOnAction = dismissOnAction;
}

/*!
    \deprecated This function is deprecated. Add actions by calling one of
    the QGraphicsWidget::addAction() family of functions instead.
    
    Adds \a action to the left side of the dialog's toolbar if the layout is left 
    to right and to the right side of the toolbar if the layout is right to left.
    
    \sa primaryAction()
*/
void HbDialog::setPrimaryAction(HbAction *action)
{
    HB_DEPRECATED("HbDialog::setPrimaryAction(HbAction *action) is deprecated. Use QGraphicsWidget action api instead");
    /* HbDialog::setPrimaryAction deprecation action coloring - begin */
    action->setProperty("invalid_addition", true);
    /* HbDialog::setPrimaryAction deprecation action coloring - end */
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
    \deprecated This function is deprecated.

     Returns the action added by calling setSecondaryAction().
*/
HbAction* HbDialog::secondaryAction() const
{
    HB_DEPRECATED("HbAction* HbDialog::secondaryAction() is deprecated. Use QGraphicsWidget action api instead");
    Q_D(const HbDialog);
    return(d->secondaryAction);
}

/*!
    \deprecated This function is deprecated. Add actions by calling one of
    the QGraphicsWidget::addAction() family of functions instead.
    
    Adds \a action to the right side of the dialog's toolbar if the layout is left 
    to right and to the left side of the toolbar if the layout is right to left.

     \sa secondaryAction()
*/
void HbDialog::setSecondaryAction(HbAction *action)
{
    HB_DEPRECATED("HbDialog::setSecondaryAction(HbAction *action) is deprecated. Use QGraphicsWidget action api instead");
    /* HbDialog::setPrimaryAction deprecation action coloring - begin */
    action->setProperty("invalid_addition", true);
    /* HbDialog::setPrimaryAction deprecation action coloring - end */
    Q_D(HbDialog);
    if (d->secondaryAction && action != d->secondaryAction) {
        removeAction(d->secondaryAction);
    }
    d->secondaryAction = action;
    addAction(action);
}

/*!
    Displays the dialog and returns immediately.
    
    Use this function to open \b modal dialogs. To open \b non-modal dialogs, 
    call \link QGraphicsItem::show() show()\endlink. 
    
    \overload
    
    \sa open(QObject*, const char*), HbPopup::isModal()
*/
void HbDialog::open()
{
    open(0,0);
}
/*!
    Displays the dialog, connects the finished(HbAction*) or finished(int) signal to a 
    receiver object's slot, and returns immediately. Disambiguation between the two 
    signals is done at runtime. The signal is disconnected from the slot when the dialog 
    closes.

    Use this function to open \b modal dialogs. To open \b non-modal dialogs, 
    call \link QGraphicsItem::show() show()\endlink. 
    
    \overload
    
    \param receiver The object that is to receive the signal. 
    \param member The slot on the receiver to which the signal is to connect. 
    
    \sa HbPopup::isModal()
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
    d->showingInProgress = true;
    show();
}
/*!
     Closes the dialog and emits the \link finished(int) finished(HbDialog::DialogCode)
     \endlink signal and either the accepted() or rejected() signal, depending on the 
     value of \a code.

     Like \link QGraphicsWidget::close() close()\endlink, this function deletes the 
     dialog if the Qt::WA_DeleteOnClose flag is set.
     
     \param code Pass HbDialog::Accepted to indicate that the user clicked the OK
            button (or equivalent) and HbDialog::Rejected to indicate that the user 
            clicked the Cancel button (or equivalent).

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
    Closes the dialog and emits the \link finished(int) finished(HbDialog::Accepted)\endlink,
    accepted(), and finished(HbAction*) signals. Typically, you call this function when the 
    user selects the OK action (or equivalent).
    
    Like \link QGraphicsWidget::close() close()\endlink, this function deletes the 
    dialog if the Qt::WA_DeleteOnClose flag is set.
     
    \b Example:
    \code
    // Create the OK action.
    mOkAction = new HbAction("OK");
    
    // Connect its triggered signal to the dialog's accept() slot.
    connect(mOkAction, SIGNAL(triggered()), this, SLOT(accept()));
    
    // Add the action to the dialog.
    addAction(mOkAction);
    \endcode

    \sa reject(), done()
*/
void HbDialog::accept()
{
    done(Accepted);
}
/*!
    Closes the dialog and emits the \link finished(int) finished(HbDialog::Rejected)\endlink,
    rejected(), and finished(HbAction*) signals. Typically, you call this function when 
    the user selects the Cancel action (or equivalent).
    
    Like \link QGraphicsWidget::close() close()\endlink, this function deletes the 
    dialog if the Qt::WA_DeleteOnClose flag is set.
    
    \b Example:
    \code
    // Create the Cancel action.
    mCancelAction = new HbAction("Cancel");
    
    // Connect its triggered signal to the dialog's reject() slot.
    connect(mCancelAction, SIGNAL(triggered()), this, SLOT(reject()));
    
    // Add the action to the dialog.
    addAction(mCancelAction);
    \endcode

    \sa accept(), done()
*/
void HbDialog::reject()
{
    done(Rejected);
}

/*!
 
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
    
 */
bool HbDialog::event(QEvent *event)
{
    Q_D(HbDialog);
    if(event->type() != QEvent::ShortcutOverride && event->type() != QEvent::GestureOverride)
        event->accept();

    if (event->type() == QEvent::ActionAdded) {
        if (!d->toolBar) {            
            d->toolBar = new HbToolBar();
            d->toolBar->setFlag(QGraphicsItem::ItemIsPanel, false);
            d->toolBar->setParentItem(this);
            HbStyle::setItemName(d->toolBar ,"controls");
            setProperty("controls_layout", true);
            d->toolBar->setOrientation(Qt::Horizontal);
            // Choose different graphics for tool buttons.
            if (!d->mFullScreen) {
                HbToolBarPrivate::d_ptr(d->toolBar)->mDialogToolBar = true;
            }
            repolish();
        }
        QActionEvent *actionEvent = static_cast<QActionEvent *>(event);
        d->toolBar->insertAction (actionEvent->before(), actionEvent->action());
        if (!parentItem()) { // only for popup without parent
            connect(actionEvent->action(), SIGNAL(triggered()), this, SLOT(_q_actionTriggered()));
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
    Reimplemented from QGraphicsWidget.
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
