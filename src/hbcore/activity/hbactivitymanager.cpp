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

#include "hbactivitymanager.h"
#include "hbactivitymanager_p.h"

#include <QPluginLoader>
#include <QDir>

#include "hbmainwindow.h"
#include "hbinstance.h"
#include "hbactivityplugininterface_p.h"

/*!
    @beta
    @hbcore
    \class HbActivityManager
    \brief HbActivityManager is an access point for Activities features.
    
    Activities can be described as stored application states (for example bookmarks
    in web browser) or actions that can be performed using application (play next
    song, start new game).
    
    The HbActivityManager class allows to use Activities features in Hb application.
    It can be used to access, add, remove and modify activities. It also notifies the
    application about activity change requests from other applications.
*/

/*!
\internal
*/
HbActivityManagerPrivate::HbActivityManagerPrivate(HbActivityManager *q) : mActivityPlugin(0)
{
#ifdef Q_OS_SYMBIAN
    QPluginLoader *loader = new QPluginLoader("/resource/qt/plugins/hbactivityplugin.qtplugin");
    mActivityPlugin = qobject_cast<HbActivityPluginInterface*>(loader->instance());
    if (mActivityPlugin) {
        q->connect(mActivityPlugin, SIGNAL(activityRequested(QString)), SIGNAL(activityRequested(QString)));
    } else {
        qWarning("Cannot load activity plugin. Features related to activities won't be available.");
    }
#else
    Q_UNUSED(q);
#endif
}

/*!
\internal
*/
HbActivityManagerPrivate::~HbActivityManagerPrivate()
{
    delete mActivityPlugin;
}

/*!
\internal
*/
bool HbActivityManagerPrivate::addActivity(const QString &activityId, const QVariant &data, const QVariantHash &parameters)
{
    bool result(false);
    if (mActivityPlugin) {
        result = mActivityPlugin->addActivity(activityId, data, parameters);
    }
    return result;
}

/*!
\internal
*/
bool HbActivityManagerPrivate::removeActivity(const QString &activityId)
{
    bool result(false);
    if (mActivityPlugin) {
        result = mActivityPlugin->removeActivity(activityId);
    }
    return result;
}

/*!
\internal
*/
QList<QVariantHash> HbActivityManagerPrivate::activities() const
{    
    if (mActivityPlugin) {
        return mActivityPlugin->activities();
    } else {
        return QList<QVariantHash>();
    }
}

/*!
\internal
*/
bool HbActivityManagerPrivate::updateActivity(const QString &activityId, const QVariant &data, const QVariantHash &parameters)
{
    bool result(false);
    if (mActivityPlugin) {
        result = mActivityPlugin->updateActivity(activityId, data, parameters);
    }
    return result;    
}

/*!
\internal
*/
QVariant HbActivityManagerPrivate::activityData(const QString &activityId) const
{
    QVariant result;
    if (mActivityPlugin) {
        result = mActivityPlugin->activityData(activityId);
    }
    return result;
}

/*!
\internal
*/
bool HbActivityManagerPrivate::waitActivity()
{
    bool result(false);
    if (mActivityPlugin) {
        result = mActivityPlugin->waitActivity();
    }
    return result;    
}

/*!
    Constructor
    \a parent. Parent of this object.
 */
HbActivityManager::HbActivityManager(QObject *parent) : QObject(parent), d_ptr(new HbActivityManagerPrivate(this))
{   
}

/*!
    Destructor
 */
HbActivityManager::~HbActivityManager()
{
    delete d_ptr;
}

/*!
    Allows to save activity.
    \a activityId. Activity name used as identifier of activities
    \a data. Activity data that should be stored. It will allow application to restore its state later 
    \a parameters. Activity properties: screenshot, localized name, hidden flag, etc.
    Returns true if activity was succesfully saved, otherwise returns false.
 */
bool HbActivityManager::addActivity(const QString &activityId, const QVariant &data, const QVariantHash &parameters)
{
    Q_D(HbActivityManager);
    return d->addActivity(activityId, data, parameters);
}

/*!
    Allows to delete activity.
    \a activityId. Activity name used as identifier of activities
    Returns true if activity was succesfully deleted, otherwise returns false.
 */
bool HbActivityManager::removeActivity(const QString &activityId)
{
    Q_D(HbActivityManager);
    return d->removeActivity(activityId);
}

/*!
    Allows to update saved activity.
    \a activityId. Activity name used as identifier of activities
    \a data. Activity data that should be stored. It will allow application to restore its state later 
    \a parameters. Activity properties: screenshot, localized name, hidden flag, etc.
    Returns true if activity was succesfully updated, otherwise returns false.
 */
bool HbActivityManager::updateActivity(const QString &activityId, const QVariant &data, const QVariantHash &parameters)
{
    Q_D(HbActivityManager);
    return d->updateActivity(activityId, data, parameters);
}

/*!
    Returns activities list. It returns activities as name and screenshot.
 */
QList<QVariantHash> HbActivityManager::activities() const
{
    Q_D(const HbActivityManager);
    return d->activities();
}

/*!
    Returns data of activity specified by \a activityId
 */
QVariant HbActivityManager::activityData(const QString &activityId) const
{
    Q_D(const HbActivityManager);
    return d->activityData(activityId);
}

/*!
    Subscribes to activity manager
 */
bool HbActivityManager::waitActivity() 
{
    Q_D(HbActivityManager);
    return d->waitActivity();
}
