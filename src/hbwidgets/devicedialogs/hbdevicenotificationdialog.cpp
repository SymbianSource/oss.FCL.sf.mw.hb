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
    \class HbDeviceNotificationDialog
    \brief HbDeviceNotificationDialog is a non-modal dialog displayed on the top of
    running applications. Functionality is based on HbNotificationDialog and it uses
    HbDeviceDialog framework.

    HbDeviceNotificationDialog is a concrete class. For the content it provides two rows of text,
    an image or an animation. Interface of HbDeviceNotificationDialog is similar to HbNotificationDialog
    with additional show(), update() and close() methods. Dialog is shown when show() is
    called.
    It is recommended that the dialog data is initialized before calling show() or update()
    methods, because those methods use interprocess communication.

    Main use of HbDeviceNotificationDialog is to show information to user without user input.

    \code
    Following code snippet creates a device notification dialog containing title, text and icon.

    QString iconName("C:/xxxx.png");

    HbDeviceNotificationDialog notificationDialog;
    notificationDialog.setIconName(iconName);
    notificationDialog.setTitle("Title");
    notificationDialog.setText("Text");
    notificationDialog.show();
    \endcode

    or equivalent dialog can be created using convenience method:

    \code
    HbDeviceNotificationDialog::notification(iconName, "Text", "Title");
    \endcode

    When using convenience methods, it is not possible to receive user interaction events,
    because the HbDeviceNotificationDialog instance is destroyed when the call returns. Ownership
    and handling of the dialog is transferred to HbDeviceDialog framework. Also dialog can not
    be closed nor updated.

    Below is an example of receiving user interaction events from device notification dialog. With
    following example user is able to receive activated and close events. Note that
    dialog is cancelled, if it goes out of scope.

    \code
    mNotificationDialog = new HbDeviceNotificationDialog;
    connect(mNotificationDialog, SIGNAL(activated()), this, SLOT(popupActivated()));
    connect(mNotificationDialog, SIGNAL(aboutToClose()), this, SLOT(release()));
    mNotificationDialog->enableTouchActivation(true);
    mNotificationDialog->setTitle("Title");
    mNotificationDialog->setText("Text");
    mNotificationDialog->show();
    \endcode

	HbDeviceNotificationDialog supports animations.
    Supported formats are the following.

    - GIF (.gif)
    - MNG (.mng)
	- Frame animations

	There is a built-in support for GIF and MNG animations.

	Frame animations can be created by following way:

	\code
	Create an animation definition file.

	<animations>
		<icon name="frame_anim_looping" playmode="loop">
			<frame duration="100">c:\icon1.svg</frame>
			<frame duration="200">c:\icon2.svg</frame>
			<frame duration="300">c:\icon3.svg</frame>
		</icon>
	</animations>

	Create HbDeviceNotificationDialog in a way described before and
	set definition file and animation's logical name.

	QString animationDefinitionXML("c:\animation.axml");
	QString logicalIconName("frame_anim_looping");

	mNotificationDialog->setAnimationDefinition(animationDefinitionXML);
    mNotificationDialog->setIconName(logicalIconName);

	\endcode
	\sa HbIconAnimationManager::addDefinitionFile
	\note Animation definition files must be stored to a place where they
	can be accessed.

    \beta
    \hbwidgets

*/

/*!
    \fn void HbDeviceNotificationDialog::aboutToClose();

    This signal is emitted when the dialog is closed with a timeout
 */

/*!
    \fn void HbDeviceNotificationDialog::activated();

    This signal is emitted when the dialog is closed with a pointer tap
*/

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

    if (mData["touchActivation"].toBool()) {
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
    Constructor.
*/
HbDeviceNotificationDialog::HbDeviceNotificationDialog(QObject *parent)
: QObject(parent), d_ptr(new HbDeviceNotificationDialogPrivate)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->q_ptr = this;

    QVariantMap data;
    data["touchActivation"] = false;
    data["titleTextWrapping"] = Hb::TextWordWrap;
    data["timeout"] = 3000;

    d->init(data);
    TRACE_EXIT
}

/*!
    Destructor.
*/
HbDeviceNotificationDialog::~HbDeviceNotificationDialog()
{
    TRACE_ENTRY
    delete d_ptr;
    TRACE_EXIT
}

/*!
    Convenience method for showing notification dialog with text and title.
    \param iconName - path and name of the icon shown on dialog.
    \param title - title shown on dialog. By default: empty.
*/
void HbDeviceNotificationDialog::notification(const QString &iconName, const QString& title)
{
    TRACE_STATIC_ENTRY
    HbDeviceNotificationDialog *self = new HbDeviceNotificationDialog;
    self->setIconName(iconName);
    self->setTitle(title);
    self->show();
    self->deleteLater();
    self = 0;
    TRACE_EXIT
}

/*!
    Convenience method for showing notification dialog with icon, text and title.
    \param iconName - path and name of the icon shown on dialog.
    \param text - text shown on dialog.
    \param title - title shown on dialog.
*/
void HbDeviceNotificationDialog::notification(const QString &iconName, const QString &text, const QString &title)
{
    TRACE_STATIC_ENTRY
    HbDeviceNotificationDialog *self = new HbDeviceNotificationDialog;
    self->setIconName(iconName);
    self->setText(text);
    self->setTitle(title);
    self->show();
    self->deleteLater();
    self = 0;
    TRACE_EXIT
}

/*!
    Sets message box icon name or animation logical name. The dialog gets updated next time ShowL() or UpdateL()
    is called.

    \param aIconName Icon name. Icon can be from Hb resources or themes. Or can be a file in
    a file system.

    \sa IconName()
*/
void HbDeviceNotificationDialog::setIconName(const QString &iconName)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["iconName"] = iconName;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Set and update text on dialog. Text is not set, if show() or update()
    is not called.
    \param text - text shown on dialog.
    \sa show(), update()
*/
void HbDeviceNotificationDialog::setText(const QString &text)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["text"] = text;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Set and update title on dialog. Title is not set, if show() or update()
    is not called.
    \param title - title shown on dialog.
    \sa show(), update()
*/
void HbDeviceNotificationDialog::setTitle(const QString &title)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["title"] = title;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Enable user interaction on dialog. Setting is not set, if show() or update()
    is not called.
    \param enable - When enabled, activated() signal is emitted on user action.

    Default value is false.
    \sa show(), update()
*/
void HbDeviceNotificationDialog::enableTouchActivation(bool enable)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["touchActivation"] = enable;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Set dialog timeout. Timeout is not set, if show() or update()
    is not called.
    \param timeout - Set timeout for dialog.

    Default value is HbPopup::StandardTimeout (3000 ms).
    \sa show(), update()
*/
void HbDeviceNotificationDialog::setTimeout(int timeout)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["timeout"] = timeout;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Set dialog title text wrapping.
    \param wrapping - Set wrapping for dialog title text.

    Default value is NoWrap.
    \sa show(), update()
*/
void HbDeviceNotificationDialog::setTitleTextWrapping(Hb::TextWrapping wrapping)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["titleTextWrapping"] = wrapping;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Set animation definition on dialog. Animation logical name has to be set
    using setIcon(). Animation is not set, if show() or update()
    is not called.
    \param animationDefinition - path and name animation definition file shown in dialog.
    \sa show(), update(), setIconName()
*/
void HbDeviceNotificationDialog::setAnimationDefinition(QString &animationDefinition)
{
    TRACE_ENTRY
    Q_D(HbDeviceNotificationDialog);
    d->mData["animationDefinition"] = animationDefinition;
    d->scheduleUpdateEvent();
    TRACE_EXIT
}

/*!
    Returns name and path of the icon shown on dialog. If not set, returns empty string.

    \sa setIconName
*/
QString HbDeviceNotificationDialog::iconName() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    const char *key = "iconName";
    QVariantMap::const_iterator i = d->mData.find(key);
    if (i != d->mData.end() && i.key() == "iconName") {
        return i.value().toString();
    }
    TRACE_EXIT
    return QString();
}

/*!
    Get text shown on dialog. If not set, returns empty string.

    \sa setText()
*/
QString HbDeviceNotificationDialog::text() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    const char *key = "text";
    QVariantMap::const_iterator i = d->mData.find(key);
    if (i != d->mData.end() && i.key() == "text") {
        return i.value().toString();
    }
    TRACE_EXIT
    return QString();
}

/*!
    Get title shown on dialog. If not set, returns empty string.

    \sa setTitle()
*/
QString HbDeviceNotificationDialog::title() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    const char *key = "title";
    QVariantMap::const_iterator i = d->mData.find(key);
    if (i != d->mData.end() && i.key() == "title") {
        return i.value().toString();
    }
    TRACE_EXIT
    return QString();
}

/*!
    Get trigger action setting.

    \sa setTriggerAction()
*/
bool HbDeviceNotificationDialog::isTouchActivating() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);
    TRACE_EXIT
    return d->mData["touchActivation"].toBool();
}

/*!
    Get timeout setting.

    \sa setTimeout()
*/
int HbDeviceNotificationDialog::timeout() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);
    TRACE_EXIT
    return d->mData["timeout"].toInt();
}

/*!
    Get title text wrapping setting.

    \sa setTitleTextWrapping()
*/
Hb::TextWrapping HbDeviceNotificationDialog::titleTextWrapping() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);
    TRACE_EXIT
    return (Hb::TextWrapping)d->mData["titleTextWrapping"].toInt();
}

/*!
    Get title animation definition file name.

    \sa setAnimationDefinition()
*/
QString HbDeviceNotificationDialog::animationDefinition() const
{
    TRACE_ENTRY
    Q_D(const HbDeviceNotificationDialog);

    const char *key = "animationDefinition";
    QVariantMap::const_iterator i = d->mData.find(key);
    if (i != d->mData.end() && i.key() == "animationDefinition") {
        return i.value().toString();
    }
    TRACE_EXIT
    return QString();
}

/*!
    Show the dialog.
    \code
    // example to show dialog.
    mNotificationDialog->setText("Dialog text");
    mNotificationDialog->show();
    \endcode
*/
void HbDeviceNotificationDialog::show()
{
    TRACE_ENTRY
    d_func()->show();
    TRACE_EXIT
}

/*!
    Update the dialog.
    \code
    // example to update already showing dialog.
    mNotificationDialog->setText("Update title");
    mNotificationDialog->update();
    \endcode
*/
void HbDeviceNotificationDialog::update()
{
    TRACE_ENTRY
    d_func()->update();
    TRACE_EXIT
}
/*!
    Close the dialog. Method has no effect if convenience methods
    are used to show device notification dialog.
*/
void HbDeviceNotificationDialog::close()
{
    TRACE_ENTRY
    d_func()->cancel();
    TRACE_EXIT
}
