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

#ifndef HBDEVICEPROGRESSDIALOGPRIVATE_H
#define HBDEVICEPROGRESSDIALOGPRIVATE_H

#include <hbprogressdialog.h>
#include <QVariantMap>

#include <hbdevicedialog.h>
#include "hbdeviceprogressdialog.h"

class HbDeviceDialog;
class QAction;

class HbDeviceProgressDialogPrivate : public QObject
{
    Q_OBJECT

public:
    enum PropertySelector {
        ProgressType,
        Maximum,
        Minimum,
        Value,
        AutoClose,
        Text,
        IconName,
        AnimationDefinition,
        NumProperties
    };
    enum ActionSelector {
        InvalidSelector = -1,
        CancelButton,
        NumActions
    };
    enum PropertyFlag {
        NoFlags = 0,
        Modified = 1,
        SentToServer = 2
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

    struct Property {
        QVariant mValue;
        PropertyFlags mFlags;
    };
    struct Action {
        QAction *mAction;
        PropertyFlags mFlags;
        bool mTriggered;
    };

public:
    HbDeviceProgressDialogPrivate();
    virtual ~HbDeviceProgressDialogPrivate();

    void initProperties(HbProgressDialog::ProgressDialogType type);
    void sendToServer(bool show = false);
    bool propertiesModified() const;
    void clearActions();
    void setProperty(PropertySelector propertySelector, int value);
    void setProperty(PropertySelector propertySelector, const QString &value);

    void cancel();
    void init(HbProgressDialog::ProgressDialogType type);

    void scheduleUpdateEvent();
    void timerEvent(QTimerEvent *event);

    static ActionSelector actionSelector(HbDeviceProgressDialog::ActionRole role);

public slots:
    void aboutToClose();
    void dataReceived(QVariantMap data);

public: // data
    friend class HbDeviceProgressDialog;

    HbDeviceProgressDialog *q;
    HbDeviceDialog mDeviceDialog;
    bool mVisible;
    int mUpdateTimerId;
    Property  mProperties[NumProperties];
    QAction *mDefaultActions[NumActions];
    Action mActions[NumActions];
};

#endif // HBDEVICEPROGRESSDIALOGPRIVATE_H
