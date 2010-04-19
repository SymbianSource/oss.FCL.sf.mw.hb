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

#ifndef HBACTIVITYMANAGER_H
#define HBACTIVITYMANAGER_H

#include <QObject>
#include <QVariant>
#include <QVariantHash>
#include <QList>
#include <QString>

#include <hbglobal.h>

class HbActivityManagerPrivate;

class HB_CORE_EXPORT HbActivityManager : public QObject
{
    Q_OBJECT

public:
    explicit HbActivityManager(QObject *parent = 0);
    virtual ~HbActivityManager();
    
    bool addActivity(const QString &activityId, const QVariant &data, const QVariantHash &parameters);
    bool removeActivity(const QString &activityId);
    bool updateActivity(const QString &activityId, const QVariant &data, const QVariantHash &parameters);
    QList<QVariantHash> activities() const;
    QVariant activityData(const QString &activityId) const;
    bool waitActivity();
    
signals:
    void activityRequested(const QString &activityId);  
 
protected:
    HbActivityManagerPrivate *d_ptr;
    
private:
    Q_DISABLE_COPY(HbActivityManager)
    Q_DECLARE_PRIVATE_D(d_ptr, HbActivityManager)    
    
};

#endif // HBACTIVITYMANAGER_H
