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

#ifndef HBINDICATORPLUGINMANAGER_H
#define HBINDICATORPLUGINMANAGER_H

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QVariantMap>

#include <hbglobal.h>
#include <hbindicatorplugininterface.h>
#include <hbpluginnamecache_p.h>

#include "hbindicatorclientinfo_p.h"

class HB_AUTOTEST_EXPORT HbIndicatorPluginManager : public QObject
{
Q_OBJECT
private:
    struct IndicatorInfo
    {
        IndicatorInfo(HbIndicatorInterface *indicator = 0) :
                indicator(indicator), activated(false) {}
        IndicatorInfo(const IndicatorInfo &other) :
                indicator(other.indicator),
                activated(other.activated),
                statusAreaIconPath(other.statusAreaIconPath) {}

        HbIndicatorInterface *indicator; //not owned
        bool activated;
        QString statusAreaIconPath;
    };

    struct PluginInfo {
        PluginInfo() : mLoader(0) {}
        ~PluginInfo(){
            if (mLoader) {
                mLoader->unload(); delete mLoader; mLoader = 0;
            }
        }

        HbIndicatorPluginInterface *plugin() const {
            return qobject_cast<HbIndicatorPluginInterface*>(mLoader->instance());
        }

        QStringList mTypes; // notification types implemented by the plugin
        QVector<IndicatorInfo> mAddedIndicators;
        HbLockedPluginLoader *mLoader; // loaded plugin
    };

public:
    explicit HbIndicatorPluginManager(QObject *parent = 0);
    virtual ~HbIndicatorPluginManager();

    HbIndicatorInterface *addIndicator(const QString &indicatorType,
        const QVariantMap &securityCredentials, int *errorCode = 0);
    bool removeIndicator(const QString &indicatorType);
    QList<IndicatorClientInfo> indicatorClientInfoList() const;
    QList<HbIndicatorInterface*> indicators() const;
    void connectTo(QObject *widget);
signals:
    void indicatorActivated(HbIndicatorInterface *addedIndicator);
    void indicatorActivated(const IndicatorClientInfo &clientInfo);
    void indicatorUpdated(const IndicatorClientInfo &clientInfo);
    void indicatorRemoved(HbIndicatorInterface *indicatorRemoved);
    void indicatorRemoved(const IndicatorClientInfo &clientInfo);

public slots:
    bool activateIndicator(const QString &indicatorType, const QVariant &parameter,
        const QVariantMap &securityCredentials);
    bool deactivateIndicator(const QString &indicatorType, const QVariant &parameter,
        const QVariantMap &securityCredentials);
    void deactivateIndicator();
    void indicatorDataChanged();
private:
    bool checkAccess(int index, const QString &indicatorType,
        const QVariantMap &securityCredentials);    
    int loadPlugin(const QString &indicatorType);
    int loadPlugin(const QString &indicatorType, const QString &filePath);
    int scanPlugins(const QString &indicatorType);
    int findPlugin(const QString &indicatorType) const;
    int findPlugin(const QString &indicatorType,
                   IndicatorInfo **indicatorInfo, int *infoIndex = 0);
    QString statusAreaIconPath(
            const HbIndicatorInterface *indicator) const;

    void updateCachePath(const QString &path);
    static QStringList pluginPathList();
    static QString pluginFileNameFilter();
    static QStringList pluginKeys(QObject *pluginInstance);

private:
    Q_DISABLE_COPY(HbIndicatorPluginManager)

    QList<PluginInfo> mPlugins;
    HbPluginNameCache mNameCache;
};

#endif // HBINDICATORPLUGINMANAGER_H
