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
#include "hbinputpredictionfactory.h"

#include <QLocale>
#include <QPluginLoader>
#include <QDir>
#include <QLibrary>
#include <QMutex>
#include <QMutexLocker>
#include <QApplication>
#include <QList>

#include "hbinputsettingproxy.h"
#include "hbinputpredictionengine.h"

/*!
@alpha
@hbcore
\class HbPredictionFactory
\brief A factory class for prediction engine abstractation layer.

A singleton factory class, which knows how to list and instantiate available input engine implementations.

\sa HbPredictionBase
\sa HbPredictionEngine
\sa HbPredictionEngineChinese
*/

/// @cond


class HbEngineData
{
public:
    HbEngineData()
        : cachedInstance(0) {
    }
    ~HbEngineData() {
        delete cachedInstance;
    }

public:
    QList<HbInputLanguage> languages;
    HbPredictionInterfaceType type;
    int handle;
    HbPredictionEngine *cachedInstance;
    QString filename;
    QString vendorId;
};

class HbPredictionFactoryPrivate
{
public:
    HbPredictionFactoryPrivate() : leadingHandle(0) {}
    ~HbPredictionFactoryPrivate();
    void initialize();
    HbPredictionEngine *predictionEngineForLanguage(const HbInputLanguage &language);
    void clearEngineData();
    HbPredictionEngine *createInstance(int index);
    int newHandle() {
        return ++leadingHandle;
    }

public:
    QList<HbPredictionPlugin *> activePlugins;
    QList<HbEngineData> engines;
    int leadingHandle;
    QMutex mutex;
};

HbPredictionFactoryPrivate::~HbPredictionFactoryPrivate()
{
    clearEngineData();

    for (int j = 0; j < activePlugins.count(); j++) {
        delete activePlugins[j];
    }
    activePlugins.clear();
}

void HbPredictionFactoryPrivate::initialize()
{
    clearEngineData();
    for (int j = 0; j < activePlugins.count(); j++) {
        delete activePlugins[j];
    }
    activePlugins.clear();

    QStringList pathList = HbInputSettingProxy::predictionEnginePaths();

    for (int kk = 0; kk < pathList.count(); kk++) {
        QDir dir(pathList[kk]);

        for (unsigned int i = 0; i < dir.count(); i++) {
            QString fileName = dir[i];

            if (!QLibrary::isLibrary(dir[i])) {
                continue;
            }

            QPluginLoader loader(dir.absoluteFilePath(fileName));
            QObject *engine_base = loader.instance();
            if (engine_base) {
                HbPredictionPlugin *plugin = qobject_cast<HbPredictionPlugin *>(engine_base);
                if (plugin) {
                    HbEngineData data;
                    data.languages = plugin->languages();
                    data.type = plugin->type();
                    data.handle = newHandle();
                    data.filename = fileName;
                    data.vendorId = plugin->vendorId();
                    engines.append(data);
                    delete plugin;
                } else {
                    qDebug("HbPredictionFactory::Initialize qobject_cast failed, probably loading ime plugin.");
                }
            } else {
                qDebug("HbPredictionFactory: Unable to create found plugin instance!");
            }
        }
    }
}

void HbPredictionFactoryPrivate::clearEngineData()
{
    engines.clear();
}

HbPredictionEngine *HbPredictionFactoryPrivate::predictionEngineForLanguage(const HbInputLanguage &language)
{
    bool anyVariant = (language.variant() == QLocale::AnyCountry);
    for (int i = 0; i < engines.count(); ++i) {
        for (int j = 0; j < engines[i].languages.count(); ++j) {
            if ((engines[i].type & HbPredInterfaceHidden) == 0) {    // Include only those that are not hidden.
                bool checkLangOnly = (anyVariant || (engines[i].languages[j].variant() == QLocale::AnyCountry));
                if ((checkLangOnly && (engines[i].languages[j].language() == language.language()))
                    || engines[i].languages[j] == language) {
                    return createInstance(i);
                }
            }
        }
    }

    return 0;
}

HbPredictionEngine *HbPredictionFactoryPrivate::createInstance(int index)
{
    if (index < engines.count()) {
        if (engines[index].cachedInstance) {
            return engines[index].cachedInstance;
        }

        QStringList pathList = HbInputSettingProxy::predictionEnginePaths();

        for (int kk = 0; kk < pathList.count(); kk++) {
            QString fullName = pathList[kk];
            fullName += QDir::separator();
            fullName += engines[index].filename;
            QPluginLoader loader(fullName);

            QObject *plugin_base = loader.instance();
            if (plugin_base) {
                HbPredictionPlugin *plugin = qobject_cast<HbPredictionPlugin *>(plugin_base);
                if (plugin) {
                    HbPredictionEngine *newEngine = plugin->createInterface();
                    if (newEngine) {
                        engines[index].cachedInstance = newEngine;
                        activePlugins.append(plugin);
                        return newEngine;
                    }
                }
            }
        }
    }

    return 0;
}

/// @endcond

/*!
Returns reference to singleton object.
*/
HbPredictionFactory *HbPredictionFactory::instance()
{
    static HbPredictionFactory myInstance;
    return &myInstance;
}

/*!
Constructs the object.
*/
HbPredictionFactory::HbPredictionFactory()
{
    d = new HbPredictionFactoryPrivate();
    d->initialize();
}

/*!
Destructs the object.
*/
HbPredictionFactory::~HbPredictionFactory()
{
}

/*!
Returns prediction engine capable of handling given language. Returns 0 if
not found. Engine is instantiated and cached. This method returns
the first suitable prediction engine it finds for given
language. There may be also others available.

\sa allPredictionEnginesForLanguage
\sa allPredictionEngines
*/
HbPredictionEngine *HbPredictionFactory::predictionEngineForLanguage(const HbInputLanguage &language)
{
    QMutexLocker locker(&d->mutex);
    return d->predictionEngineForLanguage(language);
}

/*!
Clears engine cache and frees all the resources.

\sa initialize
*/
void HbPredictionFactory::clearEngineData()
{
    QMutexLocker locker(&d->mutex);
    d->clearEngineData();
}

void HbPredictionFactory::shutDown()
{
    delete d;
    d = 0;
}


/*!
Initializes and fills cache. This is done automatically when a factory instance is
created. Can be called again to refresh the list.

\sa clearEngineData
*/
void HbPredictionFactory::initialize()
{
    // No mutex here, clearEngineData does that.
    //    clearEngineData();

    QMutexLocker locker(&d->mutex);
    d->initialize();
}

/*!
Returns all available prediction engines for given language. The returned list contains handles
which can then be used for instantiating listed engines.

\sa createEngine
\sa allPredictionEngines
*/
QList<int> HbPredictionFactory::allPredictionEnginesForLanguage(const HbInputLanguage &language)
{
    QMutexLocker locker(&d->mutex);

    QList<int> results;

    for (int i = 0; i < d->engines.count(); ++i) {
        for (int j = 0; j < d->engines[i].languages.count(); ++j) {
            if ((d->engines[i].type & HbPredInterfaceHidden) == 0) {
                if (d->engines[i].languages[j] == language) {
                    results << d->engines[i].handle;
                }
            }
        }
    }

    return QList<int>(results);
}

/*!
Returns all available prediction engines. The returned list contains handles
which can then be used for instantiating returned engines.

\sa createEngine
\sa allPredictionEnginesForLanguage
*/
QList<int> HbPredictionFactory::allPredictionEngines()
{
    QMutexLocker locker(&d->mutex);

    QList<int> results;

    for (int i = 0; i < d->engines.count(); ++i) {
        if ((d->engines[i].type & HbPredInterfaceHidden) == 0) {
            results << d->engines[i].handle;
        }
    }

    return QList<int>(results);
}

/*!
Creates and caches an instance of given language.

\sa allPredictionEnginesForLanguage
\sa allPredictionEngines
*/
HbPredictionEngine *HbPredictionFactory::createEngine(int handle)
{
    QMutexLocker locker(&d->mutex);

    for (int i = 0; i < d->engines.count(); ++i) {
        if (d->engines[i].handle == handle) {
            return d->createInstance(i);
        }
    }

    return 0;
}

/*!
Creates and caches an instance of known prediction engine. Parameter vendorIdString is matched
to HbPredictionPlugin::VendorId() string.

\sa allPredictionEnginesForLanguage
\sa allPredictionEngines
\sa HbPredictionEngine
\sa HbPredictionPlugin
*/
HbPredictionEngine *HbPredictionFactory::createEngine(const QString &vendorIdString)
{
    QMutexLocker locker(&d->mutex);

    for (int i = 0; i < d->engines.count(); ++i) {
        if (d->engines[i].vendorId == vendorIdString) {
            return d->createInstance(i);
        }
    }

    return 0;
}

// End of file
