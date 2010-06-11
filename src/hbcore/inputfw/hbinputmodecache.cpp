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
#include <QInputContextPlugin>
#include <QLocale>
#include <QFileSystemWatcher>
#include <QLibrary>
#include <QPluginLoader>
#include <QDir>

#include "hbinputmodecache_p.h"
#include "hbinpututils.h"
#include "hbinputmethod.h"
#include "hbinputsettingproxy.h"
#include "hbinputmodeproperties.h"
#include "hbinputkeymapfactory.h"
#include "hbinputmethod_p.h"
#include "hbinputmethodnull_p.h"

/*!
@alpha
@hbcore
\class HbInputModeCache
\brief Input framework's internal input mode resolver class.
*/

/// @cond

class HbInputMethodListItem
{
public:
    HbInputMethodListItem() : cached(0), toBeRemoved(false) {}
    bool operator==(const HbInputMethodListItem &item) const {
        return (descriptor.pluginNameAndPath() == item.descriptor.pluginNameAndPath() &&
                descriptor.key() == item.descriptor.key());
    }

public:
    HbInputMethodDescriptor descriptor;
    QStringList languages;
    HbInputMethod *cached;
    bool toBeRemoved;
};

class HbInputModeCachePrivate
{
public:
    HbInputModeCachePrivate() : mWatcher(new QFileSystemWatcher()), mShuttingDown(false) {}
    ~HbInputModeCachePrivate() {}
    void refresh(const QString &directory = QString());
    QInputContextPlugin *pluginInstance(const QString& pluginFileName) const;
    HbInputMethod *methodInstance(const QString &pluginFileName, const QString &key) const;
    HbInputModeProperties propertiesFromString(const QString &entry) const;
    HbInputModeProperties propertiesFromState(const HbInputState &state) const;
    HbInputMethod *cachedMethod(HbInputMethodListItem &item);
    void updateMonitoredPaths();
    bool isMappedLanguage(const HbInputLanguage &language) const;

public:
    QFileSystemWatcher *mWatcher;
    QList<HbInputMethodListItem> mMethods;
    bool mShuttingDown;
};

QInputContextPlugin* HbInputModeCachePrivate::pluginInstance(const QString& pluginFileName) const
{
    if (QLibrary::isLibrary(pluginFileName)) {
        QPluginLoader loader(pluginFileName);
        QObject* plugin = loader.instance();
        if (plugin) {
            return qobject_cast<QInputContextPlugin*>(plugin);
        }
    }

    return 0;
}

HbInputMethod *HbInputModeCachePrivate::methodInstance(const QString &pluginFileName, const QString &key) const
{
    QInputContextPlugin *plugin = pluginInstance(pluginFileName);
    if (plugin) {
        QInputContext *instance = plugin->create(key);
        HbInputMethod *result = qobject_cast<HbInputMethod*>(instance);
        if (result) {
            QStringList languages = plugin->languages(key);
            QList<HbInputModeProperties> modeList;
            foreach (QString language, languages) {
                  modeList.append(propertiesFromString(language));
            }
            result->d_ptr->mInputModes = modeList;
        }
        return result;
    }

    return 0;
}

void HbInputModeCachePrivate::refresh(const QString &directory)
{
    Q_UNUSED(directory);
    // optimize later so that if the directory is given, only changes concerning
    // it are taken into account.

    // First go through all the previously found entries and
    // tag them not refreshed.
    for (int k = 0; k < mMethods.count(); k++) {
        mMethods[k].toBeRemoved = true;
    }

    // Query plugin paths and scan the folders.
    QStringList folders = HbInputSettingProxy::instance()->inputMethodPluginPaths();
    foreach (QString folder, folders) {
        QDir dir(folder);
        for (unsigned int i = 0; i < dir.count(); i++) {
            QString path = QString(dir.absolutePath());
            if (path.right(1) != "\\" && path.right(1) != "/") {
                path += QDir::separator();
            }
            path += dir[i];
            QInputContextPlugin* inputContextPlugin = pluginInstance(path);
            if (inputContextPlugin) {
                HbInputMethodListItem listItem;
                listItem.descriptor.setPluginNameAndPath(dir.absolutePath() + QDir::separator() + dir[i]);

                // For each found plugin, check if there is already a list item for it.
                // If not, then add one.
                QStringList contextKeys = inputContextPlugin->keys();
                foreach (QString key, contextKeys) {
                    listItem.descriptor.setKey(key);
                    listItem.descriptor.setDisplayName(inputContextPlugin->displayName(key));

                    int index = mMethods.indexOf(listItem);
                    if (index >= 0) {
                        // The method is already in the list, the situation hasn't changed.
                        // just tag it not to be removed.
                        mMethods[index].toBeRemoved = false;
                    } else {
                        listItem.languages = inputContextPlugin->languages(key);
                        mMethods.append(listItem);
                    }
                }
            }
        }
    }

    // Go through the cache list and find out if some of the previous items need to be
    // removed after the refresh.
    for (int i = 0; i < mMethods.count(); i++) {
        if (mMethods[i].toBeRemoved) {
            if (mMethods[i].cached && mMethods[i].cached->isActiveMethod()) {
                // If the item to be removed happens to be the active one,
                // try to deal with the situation.
                mMethods[i].cached->forceUnfocus();
                // The active custom method is being removed.
                // Clear out related setting proxy values.
                if (mMethods[i].descriptor.pluginNameAndPath() == HbInputSettingProxy::instance()->preferredInputMethod(Qt::Horizontal).pluginNameAndPath()) {
                    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, HbInputMethodDescriptor());
                }
                if (mMethods[i].descriptor.pluginNameAndPath() == HbInputSettingProxy::instance()->preferredInputMethod(Qt::Vertical).pluginNameAndPath()) {
                    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, HbInputMethodDescriptor());
                }

                // Replace it with null input context.
                HbInputMethod *master = HbInputMethodNull::Instance();
                master->d_ptr->mIsActive = true;
                QInputContext* proxy = master->d_ptr->proxy();
                if (proxy != qApp->inputContext())
                    qApp->setInputContext(proxy);
            }
            delete mMethods[i].cached;
            mMethods.removeAt(i);
            i--;
        }
    }
}

HbInputModeProperties HbInputModeCachePrivate::propertiesFromString(const QString &entry) const
{
    HbInputModeProperties result;

    QStringList parts = entry.split(" ");
    if (parts.count() == 4) {
        // See HbInputModeProperties::toString() for details,
        QString languageStr = parts[0] + QString(" ") + parts[1];
        HbInputLanguage language;
        language.fromString(languageStr);
        result.setLanguage(language);
        result.setInputMode((HbInputModeType)parts[2].toLong());
        result.setKeyboard((HbKeyboardType)parts[3].toLong());
    }

    return result;
}

HbInputModeProperties HbInputModeCachePrivate::propertiesFromState(const HbInputState &state) const
{
    HbInputModeProperties result;

    result.setLanguage(state.language());
    result.setKeyboard(state.keyboard());
    result.setInputMode(state.inputMode());

    return HbInputModeProperties(result);
}

HbInputMethod *HbInputModeCachePrivate::cachedMethod(HbInputMethodListItem &item)
{
    if (!item.cached) {
        item.cached = methodInstance(item.descriptor.pluginNameAndPath(), item.descriptor.key());
    }

    return item.cached;
}

void HbInputModeCachePrivate::updateMonitoredPaths()
{
    QStringList watchedDirs = mWatcher->directories();
    if (!watchedDirs.isEmpty()) {
        mWatcher->removePaths(watchedDirs);
    }

    QStringList paths = HbInputSettingProxy::instance()->inputMethodPluginPaths();
    foreach (QString path, paths) {
        QDir dir(path);
        if (!dir.exists() && path.left(1) == "f") {
            mWatcher->addPath(QString("f:") + QDir::separator());
        } else {
            mWatcher->addPath(path);
        }
    }
}

bool HbInputModeCachePrivate::isMappedLanguage(const HbInputLanguage &language) const
{
    if (language.defined()) {
        QList<HbInputLanguage> languages = HbKeymapFactory::instance()->availableLanguages();
        foreach (const HbInputLanguage mappedLanguage, languages) {
            if (mappedLanguage == language) {
                return true;
            }
        }
    }

    return false;
}

/// @endcond

/*!
\internal
Returns the singleton instance.
*/
HbInputModeCache* HbInputModeCache::instance()
{
    static HbInputModeCache theCache;
    return &theCache;
}

/*!
\internal
Construct the object.
*/
HbInputModeCache::HbInputModeCache() : d_ptr(new HbInputModeCachePrivate())
{
    Q_D(HbInputModeCache);

    // Start to monitor file system for changes.
    d->updateMonitoredPaths();
    connect(d->mWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(directoryChanged(const QString &)));

    d->refresh();
}

/*!
\internal
Destruct the object.
*/
HbInputModeCache::~HbInputModeCache()
{
    delete d_ptr;
}

/*!
\internal
This slot is called whenever a change in input method plugin file system is detected and
the list needs to be refreshed.
*/
void HbInputModeCache::directoryChanged(const QString &directory)
{
    Q_D(HbInputModeCache);

    d->updateMonitoredPaths();

    if (!d->mShuttingDown) {
        d->refresh(directory);
    }
}

/*!
\internal
Shuts down the object safely. This is needed mainly for singleton object. There has been a lot
of problems related to randown singleton desctruction order and additional shutdown step is
needed to guarantee that it will be done safely. The slot is connected to
QCoreApplication::aboutToQuit when the framework is initialized.
*/
void HbInputModeCache::shutdown()
{
    Q_D(HbInputModeCache);
    d->mShuttingDown = true;

    foreach (HbInputMethodListItem method, d->mMethods) {
        delete method.cached;
        method.cached = 0;
    }
    d->mMethods.clear();
    delete d->mWatcher;
    d->mWatcher = 0;
}

/*!
\internal
Loads given input method and caches it.
*/
HbInputMethod* HbInputModeCache::loadInputMethod(const HbInputMethodDescriptor &inputMethod)
{
    Q_D(HbInputModeCache);

    for (int i = 0; i < d->mMethods.count(); i++) {
        if (d->mMethods[i].descriptor.pluginNameAndPath() == inputMethod.pluginNameAndPath() &&
            d->mMethods[i].descriptor.key() == inputMethod.key()) {
            if (!d->mMethods[i].cached) {
                d->mMethods[i].cached = d->methodInstance(inputMethod.pluginNameAndPath(), inputMethod.key());
            }

            return d->mMethods[i].cached;
        }
    }

    return 0;
}

/*!
\internal
Lists custom input methods.
*/
QList<HbInputMethodDescriptor> HbInputModeCache::listCustomInputMethods()
{
    Q_D(HbInputModeCache);

    QList<HbInputMethodDescriptor> result;

    foreach (HbInputMethodListItem item, d->mMethods) {
        foreach (QString language, item.languages) {
            HbInputModeProperties properties = d->propertiesFromString(language);
            if (properties.inputMode() == HbInputModeCustom) {
                result.append(item.descriptor);
                break;
            }
        }
    }

    return result;
}

/*!
\internal
Find correct handler for given input state.
*/
HbInputMethod* HbInputModeCache::findStateHandler(const HbInputState& state)
{
    Q_D(HbInputModeCache);

    HbInputModeProperties stateProperties = d->propertiesFromState(state);
    int languageRangeIndex = -1;    

    // First check if there is a method that matches excatly (ie. also specifies
    // the language).
    for (int i = 0; i < d->mMethods.count(); i++) {
        foreach (QString language, d->mMethods[i].languages) {
            HbInputModeProperties properties = d->propertiesFromString(language);
            if (properties.language().undefined() &&
                properties.keyboard() == stateProperties.keyboard() &&
                properties.inputMode() == stateProperties.inputMode()) {
                // Remember the index, we'll need this in the next phase if no exact
                // match is found.
                languageRangeIndex = i;
            }

            if (properties.inputMode() != HbInputModeCustom) {
                if (properties == stateProperties) {
                    return d->cachedMethod(d->mMethods[i]);
                }
            }
        }
    }

    // No exact match found. Then see if there was a method that matches to language
    // range, meaning that the language is left unspecified in which case we'll
    // use key mapping factory for matching.
    if (languageRangeIndex >= 0) {
        QList<HbInputLanguage> languages = HbKeymapFactory::instance()->availableLanguages();

        foreach(HbInputLanguage language, languages) {
            // exact match is returned If the country variant is specified in state language,
            // otherwise a method that matches to only language range is returned.
            bool exactMatchFound = (stateProperties.language().variant() != QLocale::AnyCountry) ?
                (language == stateProperties.language()) :
                (language.language() == stateProperties.language().language());
            if (exactMatchFound) {
                return d->cachedMethod(d->mMethods[languageRangeIndex]);
            }
        }
    }

    return 0;
}

/*!
\internal
Returns the active input method.

\sa HbInputMethod
*/
HbInputMethod* HbInputModeCache::activeMethod() const
{
    Q_D(const HbInputModeCache);

    foreach (HbInputMethodListItem item, d->mMethods) {
        if (item.cached && item.cached->isActiveMethod()) {
            return item.cached;
        }
    }

    return 0;
}

/*!
\internal
Lists available input languages.
*/
QList<HbInputLanguage> HbInputModeCache::listInputLanguages() const
{
    Q_D(const HbInputModeCache);

    QList<HbInputLanguage> result;

    foreach (HbInputMethodListItem item, d->mMethods) {
        foreach (QString language, item.languages) {
            HbInputModeProperties mode = d->propertiesFromString(language);
            if (mode.inputMode() != HbInputModeCustom) {
                if (mode.language().undefined()) {
                    // This is language range. Let's add everything
                    // we have key mappings for.
                    QList<HbInputLanguage> languages = HbKeymapFactory::instance()->availableLanguages();
                    foreach (HbInputLanguage mappedLanguage, languages) {
                        if (!result.contains(mappedLanguage)) {
                            result.append(mappedLanguage);
                        }
                    }
                } else {
                    if (!result.contains(mode.language())) {
                        result.append(mode.language());
                    }
                }
            }
        }
    }

    return result;
}

/*!
\internal
Returns true if given input method is able to handle given input state.
*/
bool HbInputModeCache::acceptsState(const HbInputMethod *inputMethod, const HbInputState &state) const
{
    Q_D(const HbInputModeCache);

    foreach (const HbInputMethodListItem item, d->mMethods) {
        if (item.cached == inputMethod) {           
            foreach (const QString language, item.languages) {
                HbInputModeProperties mode = d->propertiesFromString(language);
                // Check if keyboard type matches.
                if (mode.keyboard() == state.keyboard()) {
                    // Check if input mode matches or it is a custom input method but
                    // state's mode is not numeric.
                    if (mode.inputMode() == state.inputMode() ||
                        ((mode.inputMode() == HbInputModeCustom) &&
                         (state.inputMode() != HbInputModeNumeric))) {
                        // Check if language matches or input method supports
                        // all mapped languages and state's language is among them.
                        if (mode.language() == state.language() ||
                            (mode.language().undefined() && d->isMappedLanguage(state.language()))) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

/*!
\internal
Returns input method descriptor for given input method. Returns empty descriptor if the framework
doesn't recognize given input method.
*/
HbInputMethodDescriptor HbInputModeCache::descriptor(const HbInputMethod *inputMethod) const
{
    Q_D(const HbInputModeCache);

    foreach (HbInputMethodListItem item, d->mMethods) {
        if (item.cached == inputMethod) {
            return item.descriptor;
        }
    }

    return HbInputMethodDescriptor();
}


// End of file

