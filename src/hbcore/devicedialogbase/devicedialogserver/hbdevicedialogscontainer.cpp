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

#include <hbpopup.h>
#include <hbdevicedialogplugin.h>
#include <hbdevicedialoginterface.h>
#include <hbdevicedialogtrace_p.h>
#include <hbdevicedialogpluginmanager_p.h>
#include "hbdevicedialogscontainer_p.h"

#include <QFlags>
#include <QtGlobal>

HbDeviceDialogsContainer::Dialog::Dialog()
{
    mId = InvalidId;
    mPtr = 0;
    mFlags = NoFlags;
    mIndex = InvalidIndex;
#ifndef QT_NO_DEBUG
    mContainer = 0;
#endif // QT_NO_DEBUG
}

bool HbDeviceDialogsContainer::Dialog::operator ==(const Dialog &other) const
{
    return mId == other.mId;
}

// Verify that Dialog reference is valid
bool HbDeviceDialogsContainer::Dialog::verify() const
{
#ifndef QT_NO_DEBUG
    return isValid() && mIndex < mContainer->mDialogs.count() &&
        mId == mContainer->mDialogs.at(mIndex).mId;
#else // QT_NO_DEBUG
    return true;
#endif // QT_NO_DEBUG
}

// Constructor
HbDeviceDialogsContainer::HbDeviceDialogsContainer(HbDeviceDialogPluginManager &pluginManager) :
    mPluginManager(pluginManager), mNextId(1)
{
}

// Destructor
HbDeviceDialogsContainer::~HbDeviceDialogsContainer()
{
    int count = mDialogs.count();
    for(int i = 0; i < count; i++) {
        mPluginManager.freeWidget(mDialogs.at(i).mPtr);
    }
    mDialogs.clear();
}

// Add dialog to the container. Returns dialog reference.
HbDeviceDialogsContainer::Dialog &HbDeviceDialogsContainer::add(HbDeviceDialogInterface *widget,
    const HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo)
{
    TRACE_ENTRY
    Dialog dialog;
    dialog.mId = mNextId++;
    dialog.mPtr = widget;
    switch(deviceDialogInfo.group) {
    case HbDeviceDialogPlugin::DeviceNotificationDialogGroup:
        dialog.mFlags |= Dialog::NotificationGroup;
        break;
    case HbDeviceDialogPlugin::IndicatorGroup:
        dialog.mFlags |= Dialog::IndicatorGroup;
        break;
    case HbDeviceDialogPlugin::SecurityGroup:
        dialog.mFlags |= Dialog::SecurityGroup;
        break;
    case HbDeviceDialogPlugin::CriticalGroup:
        dialog.mFlags |= Dialog::CriticalGroup;
        break;
    default:
        dialog.mFlags |= Dialog::GenericGroup;
    }
#ifndef QT_NO_DEBUG
    dialog.mContainer = this;
#endif // QT_NO_DEBUG

    Q_ASSERT(!mDialogs.contains(dialog));
    mDialogs.append(dialog);

    Dialog &ref = mDialogs.last();
    ref.mIndex = mDialogs.count() - 1;

    TRACE_EXIT
    return ref;
}

// Search dialog by id
HbDeviceDialogsContainer::Dialog &HbDeviceDialogsContainer::find(int id)
{
    if (id == Dialog::InvalidId) {
        return mInvalidDialog;
    }

    Dialog search;
    search.mId = id;
    int i = mDialogs.indexOf(search);
    Dialog *dialog;
    if (i >= 0) {
        dialog = &mDialogs[i];
        dialog->mIndex = i;
    } else {
        dialog = &mInvalidDialog;
    }
    return *dialog;
}

// Search dialog by popup or widget signal sender
HbDeviceDialogsContainer::Dialog &HbDeviceDialogsContainer::find(const QObject *widget)
{
    int count = mDialogs.count();
    for(int i = 0; i < count; i++) {
        Dialog &dialog = mDialogs[i];
        if (dialog.mPtr->deviceDialogWidget() == widget || dialog.mPtr->signalSender() == widget) {
            dialog.mIndex = i;
            return dialog;
        }
    }
    return mInvalidDialog;
}

// Remove dialog from the container
void HbDeviceDialogsContainer::remove(Dialog &dialog)
{
    TRACE_ENTRY
    Q_ASSERT(dialog.isValid());
    Q_ASSERT(dialog.mId == mDialogs.at(dialog.mIndex).mId);
    mPluginManager.freeWidget(dialog.mPtr);
    mDialogs.removeAt(dialog.mIndex);
    TRACE_EXIT
}

// Get next dialog with matching flag pattern
HbDeviceDialogsContainer::Dialog &HbDeviceDialogsContainer::next(
    const Dialog &from, Dialog::Flags flags, Dialog::Flags mask)
{
    // With invalid from start from beginning, otherwise start from next
    int i = from.isValid() ? from.mIndex + 1 : 0;
    int count = mDialogs.count();
    if (i >= count) { // at the end
        return mInvalidDialog;
    }
    for(; i < count; i++) {
        const Dialog &current = mDialogs.at(i);
        if ((current.mFlags & mask) == flags) {
            break;
        }
    }
    if (i >= count) { // at the end
        return mInvalidDialog;
    }
    Dialog &dialog = mDialogs[i];
    dialog.mIndex = i;
    return dialog;
}

// Get next dialog with matching variable value
HbDeviceDialogsContainer::Dialog &HbDeviceDialogsContainer::next(
    const Dialog &from, Dialog::Variable variable, const QVariant &value)
{
    // With invalid from start from beginning, otherwise start from next
    int i = from.isValid() ? from.mIndex + 1 : 0;
    int count = mDialogs.count();
    if (i >= count) { // at the end
        return mInvalidDialog;
    }
    for(; i < count; i++) {
        const Dialog &current = mDialogs.at(i);
        if (current.mVariables[variable] == value) {
            break;
        }
    }
    if (i >= count) { // at the end
        return mInvalidDialog;
    }
    Dialog &dialog = mDialogs[i];
    dialog.mIndex = i;
    return dialog;
}

// check is the dialog list empty
bool HbDeviceDialogsContainer::isEmpty() const
{
    return mDialogs.isEmpty();
}
