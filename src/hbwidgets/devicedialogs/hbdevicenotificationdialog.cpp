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

#include "hbdevicenotificationdialog_p.h"
#include "hbdevicenotificationdialog.h"

#include <hbdevicedialog.h>
#include <hbdevicedialogtrace_p.h>
#include <QTimer>
#include <QTimerEvent>

/*!
    @stable
    @hbwidgets

    \class HbDeviceNotificationDialog

    \brief HbDeviceNotificationDialog is a non-modal dialog for displaying device-wide notifications.

    HbDeviceNotificationDialog is device dialog version of HbNotificationDialog and offers
    similar API.

    The dialog is shown by device dialog service which HbNotificationDialog is a client of.
 
    The dialog is displayed on top of all applications. It appears at top left corner of a display.
    It is intended for applications to show device-wide notifications to user in non-intrusive way.
    It can also be used by server components which don't have HbMainWindow.

    HbDeviceNotificationDialog does not require user input and is usually closed by timeout.
    For content, it supports two rows of text and an icon or animation. Two text rows may consist
    either of a title spanning two lines or title and text. Setters are provided for setting
    title, text and icon.

    An asynchronous show() method launches a dialog. Device dialog framework decides when the
    dialog is actually shown. There may be a delay untill dialog appears on display. close()
    closes a dialog.

    After dialog has been lauched, properties may be updated by setters and calling update().
    Calling update() is optional as setters schedule an automatic update event which
    updates dialog parameters next time event loop is entered.

    It is recommended that dialog properties are set before calling show() as updating them after
    causes interprocess communication.

    The dialog closes when tapped. A tap triggers activated() signal if enabled by
    enableTouchActivation().

    If there is no need to update or receive activation from a launched dialog,
    HbDeviceNotificationDialog object can be deleted after show() returns. Device
    dialog framework takes care of displaying the dialog.
 
    Device notifications dialogs are synchronized with each other. If several of them
    are to be shown at the same time, they are shown sequentially instead of on top of each other.

    In place of an icon, notification dialog may contain an animation. Supported icon animation
    formats are:
    - GIF (.gif)
    - MNG (.mng)
    - Frame animations (.axml)

    \section _platform_spec Platform-specific implementation notes for HbDeviceNotificationDialog

    \subsection _nonsymbian Non-Symbian
    Device dialog service is implemented only for the Symbian platform. On other platforms device 
    notification dialogs are displayed on client's main window.
 

    \section _usecases HbDeviceNotificationDialog use cases

    Use cases in this section:
    - \ref _uc2
    - \ref _uc2_001
    - \ref _uc2_002

    \subsection _uc2 Creating a device notification dialog

    The following code snippet creates a device notification dialog containing title, text and icon.

    \code
    HbDeviceNotificationDialog notificationDialog;
    notificationDialog.setIconName("qtg_large_info");
    notificationDialog.setTitle("Title");
    notificationDialog.setText("Text");
    notificationDialog.show();
    \endcode

    or equivalent dialog can be created using convenience method:

    \code
    HbDeviceNotificationDialog::notification("qtg_large_info", "Text", "Title");
    \endcode

    When using convenience methods, it is not possible to receive user interaction events,
    because the HbDeviceNotificationDialog instance is destroyed when the call returns. Ownership
    and handling of the dialog is transferred to HbDeviceDialog framework. In this case the dialog
    can neither be closed nor updated programmatically.

    \subsection _uc2_001 Receiving user interactions events

    Below is an example of receiving user interaction events from device notification dialog. With
    following example user is able to receive activated and close events. Note that in this case the
    dialog is closed by device dialog framework if HbDeviceNotificationDialog object is deleted.

    \code
    mNotificationDialog = new HbDeviceNotificationDialog;
    connect(mNotificationDialog, SIGNAL(activated()), this, SLOT(popupActivated()));
    connect(mNotificationDialog, SIGNAL(aboutToClose()), this, SLOT(release()));
    mNotificationDialog->enableTouchActivation(true);
    mNotificationDialog->setTitle("Title");
    mNotificationDialog->setText("Text");
    mNotificationDialog->show();
    \endcode

    \subsection _uc2_002 Using animations in a device notification dialog

    Create an animation definition file:

    \code
    <animations>
        <icon name="frame_anim_looping" playmode="loop">
            <frame duration="100">c:\icon1.svg</frame>
            <frame duration="200">c:\icon2.svg</frame>
            <frame duration="300">c:\icon3.svg</frame>
        </icon>
    </animations>
    \endcode

    Create a HbDeviceNotificationDialog as described above and
    set the definition file and the logical name of the animation:

    \code
    QString animationDefinitionXML("c:\animation.axml");
    QString logicalIconName("frame_anim_looping");

    mNotificationDialog->setAnimationDefinition(animationDefinitionXML);
    mNotificationDialog->setIconName(logicalIconName);
    mNotificationDialog->show();
    \endcode


    \sa HbNotificationDialog, HbDeviceDialog, CHbDeviceNotificationDialogSymbian
*/

/*!
    \fn void HbDeviceNotificationDialog::aboutToClose();

    This signal is emitted when notification dialog has closed.

    \sa show()
 */

/*!
    \fn void HbDeviceNotificationDialog::activated();

    This signal is emitted when the dialog is tapped and touch activation is
    enabled.
    
    \sa enableTouchActivation()
*/

static const char keyTimeout[] = "timeout";
static const char keyIconName[] = "iconName";
static const char keyText[] = "text";
static const char keyTitle[] = "title";
static const char keyTouchActivation[] = "touchActivation";
static const char keyTitleTextWrapping[] = "titleTextWrapping";
static const char keyAnimationDefinition[] = "animationDefinition";

HbDeviceNotificationDialogPrivate::HbDeviceNotificationDialogPrivate()
: QObject(), mDeviceDialog(0), mUpdateTimerId(0), mShowing(false)
{
    TRACE_ENTRY
    mData.clear();
    TRACE_EXIT
}

HbDeviceNotificationDialogPrivate::~HbDeviceNotificationDialogPrivate()
{
    TRACE_ENTRY
    // If no signals of notification dialog are connected, disconnect device dialog client signals.
    // This keeps server from deleting the device dialog when client is deleted.
    if (q_ptr->receivers(SIGNAL(aboutToClose())) == 0 &&
        q_ptr->receivers(SIGNAL(activated())) == 0) {
        mDeviceDialog->disconnect();
        if (mUpdateTimerId != 0) {
            // There's a pending update
            update(); // this also kills the timer
        }
    }
    delete mDeviceDialog;
    TRACE_EXIT
}

void HbDeviceNotificationDialogPrivate::init(const QVariantMap &map)
{
    TRACE_ENTRY
    mDeviceDialog = new HbDeviceDialog;
    mData = map;
    TRACE_EXIT
}

void HbDeviceNotificationDialogPrivate::show()
{
    TRACE_ENTRY
    Q_ASSERT(mDeviceDialog);

    killTimer(mUpdateTimerId);
    mUpdateTimerId = 0;

    connect(mDeviceDialog, SIGNAL(deviceDialogClosed()), this, SLOT(deviceDialogClosed()));

    if (q_func()->isTouchActivating()) {
        connect(mDeviceDialog, SIGNAL(dataReceived(QVariantMap)), this, SLOT(dataReceived(QVariantMap)));
    }
    if (!mDeviceDialog->show("com.nokia.hb.devicenotificationdialog/1.0", mData)) {
        // Failed to show the device dialog. Start a one shot to emit aboutToClose() signal.
        QTimer::singleShot(0, this, SLOT(deviceDialogClosed()));
    }
    mShowing = true;
    TRACE_EXIT
}

void HbDeviceNotificationDialogPrivate::cancel()
{
    TRACE_ENTRY
    Q_ASSERT(mDeviceDialog);
    mDeviceDialog->cancel();
    mData.clear();
    TRACE_EXIT
}

void HbDeviceNotificationDialogPrivate::update()
{
    TRACE_ENTRY
    Q_ASSERT(mDeviceDialog);
    killTimer(mUpdateTimerId);
    mUpdateTimerId = 0;
    mDeviceDialog->update(mData);
    TRACE_EXIT
}

// Schedule event to update changed properties to device dialog server. update() is not
// called after each time a property is set. Instead an event is scheduled in order to
// update all changed properties in one shot.
void HbDeviceNotificationDialogPrivate::scheduleUpdateEvent()
{
    if (mShowing && mUpdateTimerId == 0) {
        mUpdateTimerId = startTimer(0);
    }
}

void HbDeviceNotificationDialogPrivate::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimerId) {
        update();
    }
}

void HbDeviceNotificationDialogPrivate::dataReceived(QVariantMap data)
{
    TRACE_ENTRY
    QString temp = data.value("result").toString();
    if (!temp.isEmpty()) {
        emit q_func()->activated();
    }
    TRACE_EXIT
}

void HbDeviceNotificationDialogPrivate::deviceDialogClosed()
{
    TRACE_ENTRY
    mShowing = false;
    emit q_func()->aboutToClose();
    TRACE_EXIT
}

/*!
    Constructs HbDeviceNotificationDialog with \a parent.
*/
HbDeviceNotificationDialog::HbDeviceNotificationDialog(QObject *parent)
: QObject(parent), d_ptr(new HbDeviceNotificationDialogPrivate)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->q_ptr = this;

    QVariantMap data;

    d->init(data);
    TRACE_EXIT
}

/*!
    Destructs HbDeviceNotificationDialog. The dialog launched by show() is closed if aboutToClose()
    or activated() signals are connected to by an application. Otherwise the dialog is left
    executing and closes itself by a timeout.
*/
HbDeviceNotificationDialog::~HbDeviceNotificationDialog()
{
    TRACE_ENTRY
    delete d_ptr;
    TRACE_EXIT
}

/*!
    Convenience method for showing notification dialog with text and title.

    \param iconName Path and name of the icon shown on dialog.
    \param title Title shown on dialog. Default is empty.
*/
void HbDeviceNotificationDialog::notification(const QString &iconName, const QString& title)
{
    TRACE_STATIC_ENTRY
    HbDeviceNotificationDialog dialog;
    dialog.setIconName(iconName);
    dialog.setTitle(title);
    dialog.show();
    TRACE_EXIT
}

/*!
    Convenience method for showing notification dialog with icon, text and title.

    \param iconName Path and name of the icon shown on dialog.
    \param text Text shown on dialog.
    \param title Title shown on dialog.
*/
void HbDeviceNotificationDialog::notification(const QString &iconName, const QString &text, const QString &title)
{
    TRACE_STATIC_ENTRY
    HbDeviceNotificationDialog dialog;
    dialog.setIconName(iconName);
    dialog.setText(text);
    dialog.setTitle(title);
    dialog.show();
    TRACE_EXIT
}

/*!
    Sets icon name or animation logical name.

    \param iconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa iconName(), show(), update() 
*/
void HbDeviceNotificationDialog::setIconName(const QString &iconName)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyIconName] = iconName;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Sets dialog text. Changes also title text wrapping. If text is empty,
    sets title text wrapping to Hb::TextWordWrap, otherwise Hb::TextNoWrap.

    \param text Dialog text.

    \sa text(), show(), update()
*/
void HbDeviceNotificationDialog::setText(const QString &text)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyText] = text;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Sets title text.

    \param title Title text.

    \sa title(), show(), update()
*/
void HbDeviceNotificationDialog::setTitle(const QString &title)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyTitle] = title;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Enables user interaction on dialog.

    \param enable True enableds activated() signal on user action.

    \sa isTouchActivating(), show(), update()
*/
void HbDeviceNotificationDialog::enableTouchActivation(bool enable)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyTouchActivation] = enable;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Sets dialog timeout.

    \param timeout Timeout is milliseconds.

    \sa timeout(), show(), update()
*/
void HbDeviceNotificationDialog::setTimeout(int timeout)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyTimeout] = timeout;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Sets title text wrapping. The title can wrap only if there is no text for the dialog.
    The title can wrap to a maximum of two lines. setText() also changes title text wrapping.

    \param wrapping Title text wrapping.

    \sa titleTextWrapping(), setText(), show(), update()
*/
void HbDeviceNotificationDialog::setTitleTextWrapping(Hb::TextWrapping wrapping)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyTitleTextWrapping] = wrapping;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Set animation definition. Animation logical name has to be set
    using setIcon(). Animation definition files must be stored to a place where they
    can be accessed by device dialog service.

    Supported animation formats are following:
    - GIF (.gif)
    - MNG (.mng)
        - Frame animations

    \param animationDefinition Path and name of the animation definition file.

    \sa setIconName(), animationDefinition(), HbIconAnimationManager::addDefinitionFile(), show(), update()
*/
void HbDeviceNotificationDialog::setAnimationDefinition(QString &animationDefinition)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData[keyAnimationDefinition] = animationDefinition;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Returns icon or animation file name.

    \sa setIconName()
*/
QString HbDeviceNotificationDialog::iconName() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    TRACE_EXIT
    return d->mData.value(keyIconName).toString();
}

/*!
    Returns dialog text.

    \sa setText()
*/
QString HbDeviceNotificationDialog::text() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    TRACE_EXIT
    return d->mData.value(keyText).toString();
}

/*!
    Returns title text.

    \sa setTitle()
*/
QString HbDeviceNotificationDialog::title() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    TRACE_EXIT
    return d->mData.value(keyTitle).toString();
}

/*!
    Returns whether touch activation is enabled. Default value is false.

    \sa enableTouchActivation()
*/
bool HbDeviceNotificationDialog::isTouchActivating() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);
    TRACE_EXIT
    return d->mData.value(keyTouchActivation).toBool();
}

/*!
    Returns timeout.

    Default value is HbPopup::StandardTimeout (3000 ms).

    \sa setTimeout()
*/
int HbDeviceNotificationDialog::timeout() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);
    TRACE_EXIT
    const QVariant defaultValue(3000); // HbPopup::StandardTimeout
    return d->mData.value(keyTimeout, defaultValue).toInt();
}

/*!
    Returns title text wrapping.
    
    The title can wrap only if dialog text is empty. The title can wrap to a maximum of two lines.
    The default is Hb::TextWordWrap.

     \sa setTitleTextWrapping()
 */
Hb::TextWrapping HbDeviceNotificationDialog::titleTextWrapping() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);
    TRACE_EXIT
    const QVariant defaultValue(
        static_cast<int>(text().isEmpty() ? Hb::TextWordWrap : Hb::TextNoWrap));
    return static_cast<Hb::TextWrapping>(d->mData.value(keyTitleTextWrapping,
        defaultValue).toInt());
}

/*!
    Returns animation definition file name.

    \sa setAnimationDefinition()
*/
QString HbDeviceNotificationDialog::animationDefinition() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    TRACE_EXIT
    return d->mData.value(keyAnimationDefinition).toString();
}

/*!
    Shows a notification dialog and returns immediately without waiting for it to close.
    Closing of the dialog is indicated by aboutToClose() signal. Tapping of dialog is
    indicated by activated() signal. Dialog can be updated while showing by property
    setters.

    \sa update(), aboutToClose(), activated()

*/
void HbDeviceNotificationDialog::show()
{
    TRACE_ENTRY
    d_func()->show();
    TRACE_EXIT
}

/*!
    Updates changed properties to a showing notification dialog via interprocess
    communication. Has no effect if show() has not been called or the dialog has
    closed already. Calling update() is optional as setting any property schedules
    an event and the showing notification is updated next time Qt event loop executes.

    \sa show()
*/
void HbDeviceNotificationDialog::update()
{
    TRACE_ENTRY
    d_func()->update();
    TRACE_EXIT
}
/*!
    Closes a device notification dialog.
*/
void HbDeviceNotificationDialog::close()
{
    TRACE_ENTRY
    d_func()->cancel();
    TRACE_EXIT
}
