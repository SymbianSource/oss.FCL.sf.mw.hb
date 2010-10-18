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

#ifndef HBDEVICEDIALOGSCONTAINER_P_H
#define HBDEVICEDIALOGSCONTAINER_P_H

#include <QtGlobal>
#include <QObject>
#include <QList>
#include <QVariant>
#include <hbdevicedialogplugin.h>

class HbDeviceDialogPluginManager;
class HbDeviceDialogInterface;

// Container for device dialogs
class HbDeviceDialogsContainer
{
public:
    class Dialog {
    public:
        enum Flag{
            NoFlags = 0x00,
            // These are set from HbDeviceDialogPlugin::DeviceDialogInfo
            GenericGroup = 0x01,
            NotificationGroup = 0x02,
            IndicatorGroup  = 0x04,
            SecurityGroup  = 0x08, // screen saver / alarm
            NormalLevel = 0x10,
            SecurityLevel  = 0x20, // showing on security level
            CriticalLevel  = 0x40, // showing on critical level
            // Dialog status
            Showing = 0x80, // show() has been called
            CloseCalled = 0x100, // HbDeviceDialogInterface::closeDeviceDialog() has been called
            NoClient = 0x200, // dialog has no client (session closed or a client launched a new dialog)
            ClosedByServer = 0x400 // closed by server for some reason, not client
        };
        Q_DECLARE_FLAGS(Flags, Flag)
        enum Variable {
            ClientTag,
            CloseHousekeeping,
            NoClientHousekeeping,
            DialogType,
            NumVariables
        };
        enum {
            InvalidId = 0
        };

    private:
        enum {
            InvalidIndex = -1
        };
    public:
        Dialog();
        bool isValid() const {return mId != InvalidId;}
        HbDeviceDialogInterface *widget() const {Q_ASSERT(verify()); return mPtr;}
        int id() const {Q_ASSERT(verify()); return mId;}
        Flags flags() const {Q_ASSERT(verify()); return mFlags;}
        void setFlags(Flags flags){Q_ASSERT(verify()); mFlags |= flags;}
        void resetFlags(Flags flags){Q_ASSERT(verify()); mFlags &= ~flags;}
        qulonglong intVariable(Variable selector) const {return mVariables[selector].toULongLong();}
        QString stringVariable(Variable selector) const {return mVariables[selector].toString();}
        void setVariable(Variable selector, qulonglong value){mVariables[selector].setValue(value);}
        void setVariable(Variable selector, const QString &value){mVariables[selector].setValue(value);}
        void setReadOnly(); 
        bool operator ==(const Dialog &other) const;
        void operator =(const Dialog &other);
    private:
        bool verify() const;
        bool readOnly() const;

        int mId;
        HbDeviceDialogInterface *mPtr;
        Flags mFlags;
        QVariant mVariables[NumVariables];
        int mIndex;
#ifndef QT_NO_DEBUG
        HbDeviceDialogsContainer *mContainer; // used by verify()
        bool mReadOnly; // used by readOnly() 
#endif // QT_NO_DEBUG
        friend class HbDeviceDialogsContainer;
    };

public:
    HbDeviceDialogsContainer(HbDeviceDialogPluginManager &pluginManager);
    virtual ~HbDeviceDialogsContainer();

    Dialog &add(HbDeviceDialogInterface *widget,
        const HbDeviceDialogPlugin::DeviceDialogInfo &deviceDialogInfo);
    Dialog &find(int id);
    Dialog &find(const QObject *widget);
    void remove(Dialog &dialog);
    const Dialog &next(const Dialog &from, Dialog::Flags flags, Dialog::Flags mask) const;
    Dialog &next(const Dialog &from, Dialog::Flags flags, Dialog::Flags mask);
    Dialog &next(const Dialog &from, Dialog::Variable variable, quintptr value)
        {return next(from, variable, QVariant(static_cast<qulonglong>(value)));}
    Dialog &next(const Dialog &from, Dialog::Variable variable, const QString &value)
        {return next(from, variable, QVariant(value));}
    Dialog &next(const Dialog &from, Dialog::Variable variable, const QVariant &value);
    int count(Dialog::Flags flags, Dialog::Flags mask) const;
    bool isEmpty() const;

private:
    HbDeviceDialogPluginManager &mPluginManager;
    QList<Dialog> mDialogs;
    int mNextId;
    Dialog mInvalidDialog;

    friend class Dialog;
    Q_DISABLE_COPY(HbDeviceDialogsContainer)
};

#endif // HBDEVICEDIALOGSCONTAINER_P_H
