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
#include <hbwidgetfeedback.h>

#ifdef HB_EFFECTS
#include "hbeffectinternal_p.h"
#endif

#include <hbstyleoptionnotificationdialog_p.h>
#include "hbnotificationdialogcontent_p.h"

#define H_MARGIN QString("hb-param-margin-gene-middle-horizontal")
#define V_MARGIN QString("hb-param-margin-gene-middle-vertical")

// Container to encapsulate device dialog server status and sequential show
/// \cond
class SequentialShow : public HbWidgetSequentialShow
{
public:
    SequentialShow();
    static bool allowNotification(void *serverStatus);
private:
    HbDeviceDialogServerStatus mServerStatus;
};
/// \endcond

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
    bool allow = ((flags & HbDeviceDialogServerStatus::ShowingDialog) ==
        HbDeviceDialogServerStatus::NoFlags) ||
        (flags & HbDeviceDialogServerStatus::ShowingScreenSaver);
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
    @stable
    @hbwidgets

    \class HbNotificationDialog
    \brief HbNotificationDialog is a non-modal dialog for displaying application notifications.

    HbNotificationDialog is displayed at top left corner of a display. It is intended for
    applications to show notifications to user in non-intrusive way. The dialog does not
    require user input and is usually closed by timeout.

    For content, HbNotificationDialog supports two rows of text and an icon. Two text rows may
    consist either of a title spanning two lines or title and text. Setters are provided for
    setting title, text and icon. Alternatively, a custom widget can be created and set as
    content by an inherited method setContentWidget().

    HbNotificationDialog closes when tapped. A tap triggers HbNotificationDialog::activated() signal
    if enabled by enableTouchActivation().

    Notification dialog is displayed by show() or open() methods. Static helper functions
    launchDialog() can be used to show dialogs. 

    By default, notification dialogs are synchronized with device dialogs. The display of
    notification dialogs is delayed until there are no device dialogs on display.
    Notifications dialogs are also synchronized with each other. If several of them
    are shown at the same time, they are shown sequentially instead of on top of each other.
    The synchronization with device dialogs and sequential display of notification dialogs 
    can be disabled using the setSequentialShow() function.

    Following sample code sets dialog title, text, icon and shows it.
    \code
    HbNotificationDialog *dialog = new HbNotificationDialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setTitle("My title");
    dialog->setText("My text");
    dialog->setIcon(HbIcon("qtg_large_info"));
    dialog->show();
    \endcode

    Using a static helper to show a dialog.
    \code
    HbNotificationDialog::launchDialog(HbIcon("qtg_large_info"), "My title", "My text");
    \endcode

    Connecting to activated signal.
    \code
    HbNotificationDialog *dialog = new HbNotificationDialog();
    connect(dialog, SIGNAL(activated()), this, SLOT(dialogActivated()));
    dialog->enableTouchActivation(true);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setTitle("My title");
    dialog->setText("My text");
    dialog->setIcon(HbIcon("qtg_large_info"));
    dialog->show();
    \endcode
*/

/*!
    \fn void HbNotificationDialog::activated();

    This signal is emitted when the dialog is tapped and touch activation is
    enabled.
    
    \sa enableTouchActivation()
 */

/*!
    Constructs HbNotificationDialog.
 */
HbNotificationDialog::HbNotificationDialog() : HbDialog(*new HbNotificationDialogPrivate, 0)
{
    Q_D(HbNotificationDialog);
    d->q_ptr = this;
    setFocusPolicy(Qt::NoFocus);

    // Preferred position from style
    qreal hMargin = 0;
    qreal vMargin = 0;
    if ((style()->parameter(H_MARGIN, hMargin)) &&
        (style()->parameter(V_MARGIN, vMargin))) {
        setPreferredPos(QPointF(hMargin, vMargin));
    }

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
    Destructs HbNotificationDialog.
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

    \param enabled True enables activated() signal on user action.

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
    Convenience method to display HbNotificationDialog. Constructs a notification dialog and shows
    it. Constructed object is deleted on close.

    \param title Dialog title.
    \param text Dialog text.
    \param scene Scene to add the dialog into (optional).
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
    Convenience method to display HbNotificationDialog. Constructs a notification dialog and shows
    it. Constructed object is deleted on close.

    \param title Dialog title.
    \param scene Scene to add the dialog into (optional).
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
    Convenience method to display HbNotificationDialog. Constructs a notification dialog and shows
    it. Constructed object is deleted on close.

    \param icon Dialog icon.
    \param title Dialog title.
    \param text Dialog text.
    \param scene Scene to add the dialog into (optional).
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
    Returns title text. If a default content widget doesn't exist, it is created.

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
    Sets title text.

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
    Returns dialog text. If a default content widget doesn't exist, it is created.

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
    Sets dialog text. Changes also title text wrapping. If text is empty,
    sets title text wrapping to Hb::TextWordWrap, otherwise Hb::TextNoWrap. 

    \sa text(), setTitleTextWrapping()
*/
void HbNotificationDialog::setText(const QString& text)
{
    Q_D(HbNotificationDialog);
    d->checkAndCreateContentWidget();
    d->content->setText(text);
    if (text.isEmpty()) {
        d->titleTextWrapping = Hb::TextWordWrap;
    } else {
        d->titleTextWrapping = Hb::TextNoWrap;
    }
    d->setNotificationDialogContent();
}

/*!
    Returns dialog icon. If a default content widget doesn't exist, it is created.

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
    Sets dialog icon.

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
    Returns title text wrapping.
    
    The title can wrap only if dialog text is empty. The title can wrap to a maximum of two lines.
    The default is Hb::TextWordWrap.

    \sa setTitleTextWrapping(), title(), text()
*/
Hb::TextWrapping HbNotificationDialog::titleTextWrapping() const
{
    Q_D(const HbNotificationDialog);
    return d->titleTextWrapping;
}

/*!
    Sets title text wrapping. The title can wrap only if there is no text for the dialog.
    The title can wrap to a maximum of two lines. setText() also changes title text wrapping.

    \sa titleTextWrapping(), setText()
*/
void HbNotificationDialog::setTitleTextWrapping(Hb::TextWrapping wrapping)
{
    Q_D(HbNotificationDialog);
    if (d->titleTextWrapping != wrapping) {
        d->titleTextWrapping = wrapping;
        if (d->content) {
            d->content->setTitleTextWrapping(d->titleTextWrapping);
        }
        d->doLayout();
    }
}

/*!
    Enables or disables sequential display of notification dialog.

    When enabled, the dialog is synchronized with other notification dialogs. If multiple calls
    to show() occur at the same time then dialogs are displayed in sequence instead of on top
    of each other. The display of the dialog is also synchronized with device dialogs such
    that it does not appear until there are no device dialogs being displayed.

    With sequential show disabled, HbNotificationDialog behaves like other popups.

    While a dialog is waiting to be shown, setVisible(), hide() and show() have no effect.
    To remove a dialog from the wait queue, call setSequentialShow(false).

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
    Constructs HbNotificationDialog. Allows class derivation by shared d-pointer paradigm.
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
            HbWidgetFeedback::triggered(this, Hb::InstantPressed);
            d->stopTimeout();
        } else if(tap->state() == Qt::GestureFinished) {
            if (d->isTouchActivating) {
                HbWidgetFeedback::triggered(this, Hb::InstantReleased);
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
    HbDialogPrivate(), isTouchActivating(false), titleTextWrapping(Hb::TextWordWrap),
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
    setBackgroundItem(HbStylePrivate::P_NotificationDialog_frame);
}

void HbNotificationDialogPrivate::setNotificationDialogContent()
{
    Q_Q(HbNotificationDialog);
    content->enableTouchActivation(isTouchActivating);
    content->setTitleTextWrapping(titleTextWrapping);
    if (q->contentWidget() == content) {
        doLayout();
    } else {
        q->setContentWidget( content );
    }
}

