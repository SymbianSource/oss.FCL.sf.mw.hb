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

#ifndef HBDEVICEMESSAGEBOXPRIVATE_H
#define HBDEVICEMESSAGEBOXPRIVATE_H

#include "hbdevicemessagebox.h"
#include <QVariant>

class HbDeviceDialog;

class HbDeviceMessageBoxPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(HbDeviceMessageBox)

    enum PropertySelector {
        Type,
        Text,
        IconName,
        IconAlignment,
        IconVisible,
        Timeout,
        DismissPolicy,
        AnimationDefinition,
        NumProperties
    };
    enum ActionSelector {
        InvalidSelector = -1,
        AcceptButton,
        RejectButton,
        NumActions
    };
    enum PropertyFlag {
        NoFlags = 0,
        Modified = 1,
        SentToServer = 2
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

    struct Property{
        QVariant mValue;
        PropertyFlags mFlags;
    };
    struct Action{
        QAction *mAction;
        PropertyFlags mFlags;
        bool mTriggered;
    };

public:
    HbDeviceMessageBoxPrivate();
    virtual ~HbDeviceMessageBoxPrivate();

    void close();
    void exec();

    void init();
    void initProperties();
    void setAction(ActionSelector select, QAction *action);
    void sendToServer(bool show = false);
    bool propertiesModified() const;
    void clearActions();
    void setProperty(PropertySelector propertySelector, int value);
    void setProperty(PropertySelector propertySelector, const QString &value);

    static int timeoutValue(HbPopup::DefaultTimeout timeout);
    static ActionSelector actionSelector(HbDeviceMessageBox::ActionRole role);
    static HbAction *toHbAction(QAction *action);

    void scheduleUpdateEvent();
    void timerEvent(QTimerEvent *event);

public slots:
    void triggerAction(QVariantMap data);

public: // data
    HbDeviceMessageBox *q_ptr;
    HbDeviceDialog *mDeviceDialog;
    bool mVisible;
    int mUpdateTimerId;
    Property  mProperties[NumProperties];
    QAction *mDefaultActions[NumActions];
    Action mActions[NumActions];
};

#endif // HBDEVICEMESSAGEBOXPRIVATE_H
