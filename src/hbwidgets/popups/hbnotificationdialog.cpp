/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbWidgets module of the UI Extensions for Mobile.
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

#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QGraphicsScene>
#include <QCoreApplication>
#include <QtGlobal>
#include <QGesture>
#include <QGestureEvent>

#include <hbnotificationdialog.h>
#include <hbnotificationdialog_p.h>
#include <hblabel.h>
#include <hbmainwindow.h>
#include <hbwidgetsequentialshow_p.h>
#include <hbdevicedialogserverstatus_p.h>
#include <hbtapgesture.h>
#include <hbpangesture.h>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

#include <hbstyleoptionnotificationdialog_p.h>
#include "hbnotificationdialogcontent_p.h"

#define H_MARGIN QString("hb-param-margin-gene-middle-horizontal")
#define V_MARGIN QString("hb-param-margin-gene-middle-vertical")

// Container to encapsulate device dialog server status and sequential show
class SequentialShow : public HbWidgetSequentialShow
{
public:
    SequentialShow();
    static bool allowNotification(void *serverStatus);
private:
    HbDeviceDialogServerStatus mServerStatus;
};
// Constructor
SequentialShow::SequentialShow() :
    HbWidgetSequentialShow(SequentialShow::allowNotification, &mServerStatus), mServerStatus(false)
{
    connect(&mServerStatus, SIGNAL(statusChanged()), SLOT(externalStatusChanged()));
}
// Return true if notification dialog can be shown
bool SequentialShow::allowNotification(void *serverStatus)
{
    HbDeviceDialogServerStatus* srvStatus =
        reinterpret_cast<HbDeviceDialogServerStatus*>(serverStatus);
    HbDeviceDialogServerStatus::StatusFlags flags = srvStatus->status();
    bool allow = (flags & HbDeviceDialogServerStatus::ShowingDialog) ==
        HbDeviceDialogServerStatus::NoFlags;
    // Monitor changes only when notifications are not allowed
    srvStatus->enableMonitoring(!allow);
    return allow;
}

// Singleton
Q_GLOBAL_STATIC(SequentialShow, notificationDialogSequentialShowInstance)
static SequentialShow *sequentialShowInstance()
{
    return notificationDialogSequentialShowInstance();
}

/*!
    @beta
    @hbwidgets
    \class HbNotificationDialog
    \brief HbNotificationDialog can be used to notify users of system
    generated or user activated events in the UI.

    HbNotificationDialog is a non-modal dialog displayed on top of applications.
    These notifications do not require
    user input.

    Optionally, an action can be activated with a tap to the notification dialog. This is enabled
    by first enabling the touch activation with
    enableTouchActivation() and then starting the action with the signal
    HbNotificationDialog::activated().

    HbNotificationDialog is a concrete class. For the content, you can use the default content
    widgets which provides two rows of text (title spanning both lines, or title and text) and optionally an icon.
	You can use the default content widget by invoking the HbNotificationDialog with its
    static launch-methods or by using the methods setText(), setTitle() and setIcon().

    Alternatively, you can create a separate widget, and set it to the dialog with the inherited method
    HbNotificationDialog::setContentWidget().

    To display a notification dialog, show() or open() must be called. By default, notifications
    are synchronized with device dialogs. The display of notification dialogs is delayed until there
    are no device dialogs on display. Notifications are also synchronized with each other.
    If several notifications are shown at the same time with the show() function, they are shown
    sequentially instead of on top of each other. The synchronization of dialogs and sequential display of dialogs 
    can be disabled using the setSequentialShow() function.
*/

/*!
    \fn void HbNotificationDialog::activated();

    This signal is emitted when the dialog is closed with a pointer tap.
 */

/*!
    Constructor.
 */
HbNotificationDialog::HbNotificationDialog() : HbDialog(*new HbNotificationDialogPrivate, 0)
{
    Q_D(HbNotificationDialog);
    d->q_ptr = this;
    setFocusPolicy(Qt::NoFocus);
    d->timeout = HbDialog::StandardTimeout;

    // Preferred position from style
    qreal hMargin = 0;
    qreal vMargin = 0;
    if ((style()->parameter(H_MARGIN, hMargin)) &&
        (style()->parameter(V_MARGIN, vMargin))) {
        setPreferredPos(QPointF(hMargin, vMargin));
    }

    //d->setPriority(1);

    setModal(false);

    setBackgroundFaded(false);
    setDismissPolicy(HbPopup::NoDismiss);
    setTimeout(HbPopup::StandardTimeout);
    d->setBackgroundStyle();

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::TapGesture);

#ifdef HB_EFFECTS
    HbEffectInternal::add(this, "notificationdialog_appear", "appear");
    HbEffectInternal::add(this, "notificationdialog_disappear", "disappear");
#endif
}

/*!
    Destructor.
 */
HbNotificationDialog::~HbNotificationDialog()
{
    Q_D(HbNotificationDialog);
    if (d->sequentialShow) {
        sequentialShowInstance()->remove(this);
    }
}

/*!
    Enable user interaction on dialog.
    \param enabled - When enabled, the activated() signal is emitted on user action.

    \sa isTouchActivating()
*/
void HbNotificationDialog::enableTouchActivation(bool enabled)
{
    Q_D(HbNotificationDialog);
    d->isTouchActivating = enabled;
    if (d->content) {
        d->content->enableTouchActivation(enabled);
    }
}

/*!
    Returns true if user interaction is enabled.

    \sa enableTouchActivation()
*/
bool HbNotificationDialog::isTouchActivating() const
{
    Q_D(const HbNotificationDialog);
    return d->isTouchActivating;
}

/*!
    Convenience method for using HbNotificationDialog. Shows a notification dialog with
    the given parameters. The dialog is owned by HbNotificationDialog.
*/
void HbNotificationDialog::launchDialog(const QString &title, const QString &text, QGraphicsScene* scene)
{
    HbNotificationDialog *self = new HbNotificationDialog();
    if (scene) {
        scene->addItem(self);
    }
    self->setAttribute(Qt::WA_DeleteOnClose, true);
    self->setText(text);
    self->setTitle(title);
    self->show();
}

/*!
    Convenience method for using HbNotificationDialog. Shows a notification dialog with
    the given parameters. The dialog is owned by NotificationDialog.
*/
void HbNotificationDialog::launchDialog(const QString &title, QGraphicsScene* scene)
{
    HbNotificationDialog *self = new HbNotificationDialog();
    if (scene) {
        scene->addItem(self);
    }
    self->setAttribute(Qt::WA_DeleteOnClose, true);
    self->setTitle(title);
    self->show();
}

/*!
    Convenience method for using HbNotificationDialog. Shows a notification dialog with
    the given parameters. The dialog is owned by HbNotificationDialog.
*/
void HbNotificationDialog::launchDialog(const HbIcon &icon, const QString &title,
                                        const QString &text, QGraphicsScene* scene)
{
    HbNotificationDialog *self = new HbNotificationDialog();
    if (scene) {
        scene->addItem(self);
    }
    self->setAttribute(Qt::WA_DeleteOnClose, true);
    self->setIcon(icon);
    self->setText(text);
    self->setTitle(title);
    self->show();
}

/*!
    Returns the title text.

    If a default content widget doesn't exist, it is created.

    \sa setTitle()
*/
QString HbNotificationDialog::title() const
{
    Q_D(const HbNotificationDialog);
    if(d->content) {
        return d->content->title();
    } else {
        return QString();
    }
}

/*!
    Set the dialog title text.

    \sa title()
*/
void HbNotificationDialog::setTitle(const QString& title)
{
    Q_D(HbNotificationDialog);
    d->checkAndCreateContentWidget();
    d->content->setTitle(title);
    d->setNotificationDialogContent();
}

/*!
    Returns the text for the dialog.

    If a default content widget doesn't exist, it is created.

    \sa setText()
*/
QString HbNotificationDialog::text() const
{
    Q_D(const HbNotificationDialog);
    if(d->content) {
        return d->content->text();
    } else {
        return QString();
    }
}

/*!
    Set the text for the dialog.

    \sa text()
*/
void HbNotificationDialog::setText(const QString& text)
{
    Q_D(HbNotificationDialog);
    d->checkAndCreateContentWidget();
    d->content->setText( text );
    d->setNotificationDialogContent();
}

/*!
    Returns the icon for the dialog.

    If a default content widget doesn't exist, it is created.

    \sa setIcon()
*/
HbIcon HbNotificationDialog::icon() const
{
    Q_D(const HbNotificationDialog);
    if(d->content) {
        return d->content->icon();
    } else {
        return QString();
    }
}

/*!
    Set the icon.

    \sa icon()
*/
void HbNotificationDialog::setIcon(const HbIcon& icon)
{
    Q_D(HbNotificationDialog);
    d->checkAndCreateContentWidget();
    d->content->setIcon( icon );
    d->setNotificationDialogContent();
}

/*!
    Returns the style of text wrapping for the title.
    
    The title can wrap only if there is no other text for the dialog. The title can wrap to a maximum of two lines.
    The default is Hb::TextWordWrap.

    \sa setTitleTextWrapping(), HbNotificationDialog::title, HbNotificationDialog::text
*/
Hb::TextWrapping HbNotificationDialog::titleTextWrapping() const
{
    Q_D(const HbNotificationDialog);
    return d->titleWrapping;
}

/*!
    Sets whether the text for the title is wrapped.

    The title can wrap only if there is no text for the dialog. The title can wrap to a maximum of two lines.
    \sa titleTextWrapping()
*/
void HbNotificationDialog::setTitleTextWrapping(Hb::TextWrapping wrapping)
{
    Q_D(HbNotificationDialog);
    if (d->titleWrapping != wrapping) {
        d->titleWrapping = wrapping;
        if (d->content) {
            d->content->setTitleTextWrapping(d->titleWrapping);
        }
        d->doLayout();
    }
}

/*!
    Enables or disables sequential display of the Notification Dialog.

    When enabled, notification dialogs are shown sequentially. If multiple calls to show() occur at the same time then the dialogs are displayed
    in sequence instead of on top of each other. The display of the dialogs is also synchronized
    with the device dialogs such that the notification dialogs do not appear until there are no device dialogs being displayed.

    With sequential show disabled,
    HbNotificationDialog behaves like other popups. While a dialog is waiting to be shown,
    setVisible(), hide() and show() have no effect. To remove a dialog from the wait queue, call setSequentialShow(false).

    This setting is enabled by default. 

    \sa isSequentialShow()
*/
void HbNotificationDialog::setSequentialShow(bool sequentially)
{
    Q_D(HbNotificationDialog);
    if (d->sequentialShow != sequentially && !sequentially) {
        sequentialShowInstance()->remove(this);
    }
    d->sequentialShow = sequentially;
}

/*!
    Returns the sequential show setting.

    \sa setSequentialShow()
*/
bool HbNotificationDialog::isSequentialShow() const
{
    Q_D(const HbNotificationDialog);
    return d->sequentialShow;
}

/*!
 Constructor required by the shared d-pointer paradigm.
*/
HbNotificationDialog::HbNotificationDialog(HbNotificationDialogPrivate &dd, QGraphicsItem *parent) :
    HbDialog(dd, parent)
{
}

/*!
    \reimp
*/
QGraphicsItem *HbNotificationDialog::primitive(const QString &itemName) const
{
    Q_D(const HbNotificationDialog);
    
    if (itemName == "") {
        return 0;
    } else {
        if(d->content) {
            return d->content->primitive(itemName);
        } else {
             return HbWidget::primitive(itemName);
        }
    }
}

void HbNotificationDialog::gestureEvent(QGestureEvent *event)
{
    Q_D(HbNotificationDialog);
    if(HbTapGesture *tap = qobject_cast<HbTapGesture*>(event->gesture(Qt::TapGesture))) {
        if(tap->state() == Qt::GestureStarted) {
            d->stopTimeout();
        } else if(tap->state() == Qt::GestureFinished) {
            if (d->isTouchActivating) {
                emit activated();
            }
            close();
        }
    } else if( HbPanGesture *pan = qobject_cast<HbPanGesture*>(event->gesture(Qt::PanGesture))) {
        if(pan->state() == Qt::GestureFinished){
            close();
        }
    }
}

// Widget is about to hide. Closing effect has ended,
void HbNotificationDialog::hideEvent(QHideEvent *event)
{
    HbDialog::hideEvent(event);
    HbMainWindow* mainWnd = mainWindow();
    if (mainWnd) {
        disconnect(mainWnd, SIGNAL(orientationChanged(Qt::Orientation)),this, SLOT(orientationChanged(Qt::Orientation)));
    }
}

// Widget is about to show
void HbNotificationDialog::showEvent(QShowEvent *event)
{
    HbDialog::showEvent(event);
    HbMainWindow* mainWnd = mainWindow();
    if (mainWnd) {
        connect(mainWnd, SIGNAL(orientationChanged(Qt::Orientation)),this, SLOT(orientationChanged(Qt::Orientation)));
    }
}

// Item change event
QVariant HbNotificationDialog::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // If item is about to show
    if (change == QGraphicsItem::ItemVisibleChange && value.toBool()) {
        Q_D(HbNotificationDialog);
        if (d->sequentialShow) {
            HbWidgetSequentialShow *sequentialShow = sequentialShowInstance();
            sequentialShow->add(this);
            QVariant retVal(false);
            // Dialog to show now
            HbWidget *toShow = sequentialShow->toShowNow();
            if (toShow == this) { // this dialog should show now
                sequentialShow->setShowing(this);
                retVal = HbDialog::itemChange(change, value);
                if (!retVal.toBool()) {
                    sequentialShow->remove(this);
                }
            }
            // Return true if this dialog is showing. Otherwise the dialog is shown later.
            return retVal;
        }
    }
    return HbDialog::itemChange(change, value);
}

void HbNotificationDialog::orientationChanged(Qt::Orientation orientation)
{
     Q_UNUSED(orientation);

    // Preferred position from style
    qreal hMargin = 0;
    qreal vMargin = 0;
    if ((style()->parameter(H_MARGIN, hMargin)) &&
        (style()->parameter(V_MARGIN, vMargin))) {
        setPreferredPos(QPointF(hMargin, vMargin));
    }
}

HbNotificationDialogPrivate::HbNotificationDialogPrivate() :
        HbDialogPrivate(), isTouchActivating(false),
        titleWrapping(Hb::TextWordWrap),
        content(0), sequentialShow(true)
{
}

HbNotificationDialogPrivate::~HbNotificationDialogPrivate()
{
}

void HbNotificationDialogPrivate::checkAndCreateContentWidget()
{
    Q_Q(HbNotificationDialog);
    if(!content) {
        content = new HbNotificationDialogContent(q);
        HbStyle::setItemName(content, "content");
    }
}
void HbNotificationDialogPrivate::setBackgroundStyle()
{
    Q_Q(HbNotificationDialog);
    q->setBackgroundItem(HbStyle::P_NotificationDialog_frame);
}

void HbNotificationDialogPrivate::setNotificationDialogContent()
{
    Q_Q(HbNotificationDialog);
    content->enableTouchActivation(isTouchActivating);
    content->setTitleTextWrapping(titleWrapping);
    if (q->contentWidget() == content) {
        doLayout();
    } else {
        q->setContentWidget( content );
    }
}

