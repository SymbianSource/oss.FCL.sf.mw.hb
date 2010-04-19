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

#ifndef HBINDICATORINTERFACE_H
#define HBINDICATORINTERFACE_H

#include <QFlags>
#include <QObject>

#include <hbglobal.h>

class HbIndicatorInterfacePrivate;

class HB_CORE_EXPORT HbIndicatorInterface : public QObject
{
    Q_OBJECT
public:

    enum Category {
        NotificationCategory,
        ProgressCategory,
        SettingCategory
    };

    //deprecated
    enum GroupPriority{
        GroupPriorityHigh,
        GroupPriorityAverage,
        GroupPriorityLow
    };

    enum InteractionType {
        NoInteraction = 0x00,
        InteractionActivated = 0x01
    };
    Q_DECLARE_FLAGS(InteractionTypes, InteractionType)

    enum DataRole {
        IconNameRole, //deprecated
        DecorationNameRole,
        MonoDecorationNameRole,
        PrimaryTextRole,
        SecondaryTextRole,
        TextRole = PrimaryTextRole
    };

    enum RequestType {
        RequestActivate,
        RequestDeactivate
    };

    QString indicatorType() const;
    InteractionTypes interactionTypes() const;
    GroupPriority groupPriority() const; //deprecated
    Category category() const;
    void processClientRequest(RequestType type, const QVariant &parameter);
    virtual bool handleInteraction(InteractionType type);
    virtual QVariant indicatorData(int role) const = 0;
    virtual ~HbIndicatorInterface();
signals:
    void dataChanged();
    void deactivate();

protected:
    HbIndicatorInterface(const QString &indicatorType,
        GroupPriority indicatorGroup, InteractionTypes interactionTypes); //deprecated.
    HbIndicatorInterface(const QString &indicatorType,
        Category category, InteractionTypes interactionTypes);

    virtual bool handleClientRequest(RequestType type, const QVariant &parameter);
private:
    HbIndicatorInterfacePrivate * const d;
    Q_DISABLE_COPY(HbIndicatorInterface)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(HbIndicatorInterface::InteractionTypes)

#endif // HBINDICATORINTERFACE_H
