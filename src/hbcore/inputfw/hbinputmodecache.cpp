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
#include "hbinputmodecache_p.h"
#include "hbinputmodecache_p_p.h"

#include <QInputContextPlugin>
#include <QLocale>
#include <QFileSystemWatcher>
#include <QLibrary>
#include <QPluginLoader>
#include <QDir>
#include <QSharedMemory>
#include <QDataStream>
#include <QBuffer>

#include "hbinpututils.h"
#include "hbinputmethod.h"
#include "hbinputcontextplugin.h"
#include "hbinputsettingproxy.h"
#include "hbinputmodeproperties.h"
#include "hbinputkeymapfactory.h"
#include "hbinputmethod_p.h"
#include "hbinputmethodnull_p.h"

/*!
@alpha
\class HbInputModeCache
\brief Input framework's internal input mode resolver class.
*/

/// @cond

void HbInputMethodListItem::setValues(QInputContextPlugin *plugin, const QString &key)
{
    if (plugin) {
        descriptor.setKey(key);
        descriptor.setDisplayName(plugin->displayName(key));

        HbInputContextPlugin *extension = qobject_cast<HbInputContextPlugin *>(plugin);
        if (extension) {
            descriptor.setDisplayNames(extension->displayNames(key));
            descriptor.setIcon(extension->icon(key));
            descriptor.setIcons(extension->icons(key));
        }
    }
}

QDataStream &operator<<(QDataStream &out, const HbInputMethodListItem &item)
{
    out << item.descriptor;
    out << item.languages;
    return out;
}

QDataStream &operator>>(QDataStream &in, HbInputMethodListItem &item)
{
    in >> item.descriptor;
    in >> item.languages;
    item.cached = 0;
    item.toBeRemoved = false;
    return in;
}

HbInputModeCachePrivate::HbInputModeCachePrivate()
  : mSharedMethodList(0),
    mMethodListModTime(0),
    mMethodListLastUpdate(0),
    mMethodsFetchedFromDisk(false),
    mShuttingDown(false)
{
    mSharedMethodList = new QSharedMemory(HbInputMethodListKey);
    // sharedMethodList is only attached when the list is updated
    mMethodListModTime = new QSharedMemory(HbInputMethodListModTimeKey);
    mMethodListModTime->attach();
}

HbInputModeCachePrivate::~HbInputModeCachePrivate()
{
    delete mSharedMethodList;
    delete mMethodListModTime;
}

void HbInputModeCachePrivate::refresh()
{
    // Shared memory data is used if available (checked every time we refresh)
    // Otherwise the methods are read from disk, but just once during modecache lifetime
    if (!readInputMethodDataFromSharedMemory() && !mMethodsFetchedFromDisk) {
        readInputMethodDataFromDisk(&mMethods);
        pruneRemovedMethods();
        mMethodsFetchedFromDisk = true;
    }
}

void HbInputModeCachePrivate::readInputMethodDataFromDisk(QList<HbInputMethodListItem>* methodList, const QDir &readPath)
{
    bool readFromSinglePath = (readPath != QDir());
    // First go through all the previously found entries and
    // tag them not refreshed. In case a directory is defined, only marks entries
    // in that directory
    for (int i = 0; i < methodList->size(); ++i) {
        if (readFromSinglePath) {
            if (methodList->at(i).descriptor.pluginNameAndPath().left(methodList->at(i).descriptor.pluginNameAndPath().lastIndexOf(QDir::separator()))
                == readPath.absolutePath()) {
                (*methodList)[i].toBeRemoved = true;
            }
        } else {
            (*methodList)[i].toBeRemoved = true;
        }
    }

    // Query plugin paths and scan the folders.
    QStringList folders = HbInputSettingProxy::instance()->inputMethodPluginPaths();
    foreach(const QString &folder, folders) {
        QDir dir(folder);
        if (!readFromSinglePath || readPath == dir) {
            for (unsigned int i = 0; i < dir.count(); i++) {
                QString path = QString(dir.absolutePath());
                if (path.right(1) != "\\" && path.right(1) != "/") {
                    path += QDir::separator();
                }
                path += dir[i];
                QInputContextPlugin *inputContextPlugin = pluginInstance(path);
                if (inputContextPlugin) {
                    HbInputMethodListItem listItem;
                    listItem.descriptor.setPluginNameAndPath(dir.absolutePath() + QDir::separator() + dir[i]);

                    // For each found plugin, check if there is already a list item for it.
                    // If not, then add one.
                    QStringList contextKeys = inputContextPlugin->keys();
                    foreach(const QString &key, contextKeys) {
                        listItem.setValues(inputContextPlugin, key);

                        int index = methodList->indexOf(listItem);
                        if (index >= 0) {
                            // The method is already in the list, the situation hasn't changed.
                            // just tag it not to be removed.
                            (*methodList)[index].toBeRemoved = false;
                        } else {
                            listItem.languages = inputContextPlugin->languages(key);
                            methodList->append(listItem);
                        }
                    }
                }
            }
        }
    }
}

bool HbInputModeCachePrivate::readInputMethodDataFromSharedMemory()
{
    // Check if the shared list has been modified
    if (!mMethodListModTime->isAttached()) {
        // Shared memory is not attached
        // Revert to in-process handling
        return false;
    }
    // No locking, since in case the value is corrupt we just need to read the list again
    // on the next run
    uint *newModTime = static_cast<uint *>(mMethodListModTime->data());
    if (*newModTime == mMethodListLastUpdate) {
        // The internal list is still in sync with the one in shared memory
        return true;
    }
    // Modifications done since last update, try to attach the method list
    // Revert to in-process handling if not successful
    if (!mSharedMethodList->attach()) {
        return false;
    }
    // Attached successfully, update the modification time
    mMethodListLastUpdate = *newModTime;

    // To start updating the list, first mark all methods for removal
    for (int k = 0; k < mMethods.count(); k++) {
        mMethods[k].toBeRemoved = true;
    }

    // Get a copy of the list from shared memory
    mSharedMethodList->lock();
    QByteArray array(static_cast<const char *>(mSharedMethodList->data())+sizeof(int), *static_cast<int *>(mSharedMethodList->data()));
    // array now has a copy of the data, so we can unlock and detach
    mSharedMethodList->unlock();
    mSharedMethodList->detach();

    // Next read the entries from the array to a temporary list
    QDataStream in(&array, QIODevice::ReadOnly);
    QList<HbInputMethodListItem> newMethodList;
    in >> newMethodList;

    // Go through the temporary list and append new methods to internal list
    // Duplicates are marked as not to be removed, the rest will be removed by pruneRemovedMethods
    foreach (const HbInputMethodListItem& item, newMethodList) {
        int index = mMethods.indexOf(item);
        if (index >= 0) {
            mMethods[index].toBeRemoved = false;
        } else {
            mMethods.append(item);
        }
    }
    pruneRemovedMethods();
    return true;
}

void HbInputModeCachePrivate::pruneRemovedMethods()
{
    // Go through the cache list and find out if some of the previous items need to be
    // removed after the refresh.
    for (int i = 0; i < mMethods.count(); i++) {
        if (mMethods.at(i).toBeRemoved) {
            if (mMethods.at(i).cached && mMethods.at(i).cached->isActiveMethod()) {
                // If the item to be removed happens to be the active one,
                // try to deal with the situation.
                mMethods.at(i).cached->forceUnfocus();
                // The active custom method is being removed.
                // Clear out related setting proxy values.
                if (mMethods.at(i).descriptor.pluginNameAndPath() == HbInputSettingProxy::instance()->preferredInputMethod(Qt::Horizontal).pluginNameAndPath()) {
                    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Horizontal, HbInputMethodDescriptor());
                }
                if (mMethods.at(i).descriptor.pluginNameAndPath() == HbInputSettingProxy::instance()->preferredInputMethod(Qt::Vertical).pluginNameAndPath()) {
                    HbInputSettingProxy::instance()->setPreferredInputMethod(Qt::Vertical, HbInputMethodDescriptor());
                }

                // Replace it with null input context.
                HbInputMethod *master = HbInputMethodNull::Instance();
                master->d_ptr->mIsActive = true;
                QInputContext *proxy = master->d_ptr->proxy();
                if (proxy != qApp->inputContext()) {
                    qApp->setInputContext(proxy);
                }
            }
            delete mMethods[i].cached;
            mMethods.removeAt(i);
            i--;
        }
    }
}

QInputContextPlugin *HbInputModeCachePrivate::pluginInstance(const QString &pluginFileName)
{
    if (QLibrary::isLibrary(pluginFileName)) {
        QPluginLoader loader(pluginFileName);
        QObject *plugin = loader.instance();
        if (plugin) {
            return qobject_cast<QInputContextPlugin *>(plugin);
        }
    }

    return 0;
}

HbInputMethod *HbInputModeCachePrivate::methodInstance(const QString &pluginFileName, const QString &key) const
{
    QInputContextPlugin *plugin = pluginInstance(pluginFileName);
    if (plugin) {
        QInputContext *instance = plugin->create(key);
        HbInputMethod *result = qobject_cast<HbInputMethod *>(instance);
        if (result) {
            QStringList languages = plugin->languages(key);
            QList<HbInputModeProperties> modeList;
            foreach(const QString &language, languages) {
                modeList.append(HbInputModeProperties::fromString(language));
            }
            result->d_ptr->mInputModes = modeList;
        }
        return result;
    }

    return 0;
}

HbInputMethod *HbInputModeCachePrivate::cachedMethod(HbInputMethodListItem &item)
{
    if (!item.cached) {
        item.cached = methodInstance(item.descriptor.pluginNameAndPath(), item.descriptor.key());
    }

    return item.cached;
}

bool HbInputModeCachePrivate::isMappedLanguage(const HbInputLanguage &language) const
{
    return (HbKeymapFactory::instance()->keymap(language) != 0);
}

/// @endcond

/*!
\internal
Returns the singleton instance.
*/
HbInputModeCache *HbInputModeCache::instance()
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
Shuts down the object safely. This is needed mainly for singleton object. There has been a lot
of problems related to randown singleton desctruction order and additional shutdown step is
needed to guarantee that it will be done safely. The slot is connected to
QCoreApplication::aboutToQuit when the framework is initialized.
*/
void HbInputModeCache::shutdown()
{
    Q_D(HbInputModeCache);
    d->mShuttingDown = true;

    foreach(HbInputMethodListItem method, d->mMethods) {
        delete method.cached;
        method.cached = 0;
    }
    d->mMethods.clear();
}

/*!
\internal
Loads given input method and caches it.
*/
HbInputMethod *HbInputModeCache::loadInputMethod(const HbInputMethodDescriptor &inputMethod)
{
    Q_D(HbInputModeCache);
    d->refresh();

    for (int i = 0; i < d->mMethods.count(); i++) {
        if (d->mMethods.at(i).descriptor.pluginNameAndPath() == inputMethod.pluginNameAndPath() &&
            d->mMethods.at(i).descriptor.key() == inputMethod.key()) {
            if (!d->mMethods.at(i).cached) {
                d->mMethods[i].cached = d->methodInstance(inputMethod.pluginNameAndPath(), inputMethod.key());
            }

            return d->mMethods[i].cached;
        }
    }

    return 0;
}

/*!
\internal
Lists all custom input methods.
*/
QList<HbInputMethodDescriptor> HbInputModeCache::listCustomInputMethods()
{
    Q_D(HbInputModeCache);
    d->refresh();

    QList<HbInputMethodDescriptor> result;

    foreach(const HbInputMethodListItem &item, d->mMethods) {
        foreach(const QString &language, item.languages) {
            HbInputModeProperties properties = HbInputModeProperties::fromString(language);
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
Lists custom input methods for given parameters.
*/
QList<HbInputMethodDescriptor> HbInputModeCache::listCustomInputMethods(Qt::Orientation orientation, const HbInputLanguage &language)
{
    Q_D(HbInputModeCache);
    d->refresh();

    QList<HbInputMethodDescriptor> result;

    for (int i = 0; i < d->mMethods.count(); i++) {
        foreach (const QString &lang, d->mMethods.at(i).languages) {
            HbInputModeProperties properties = HbInputModeProperties::fromString(lang);
            
            // Find custom methods that supports given language or any language and
            // supports given orientation
            if (properties.inputMode() == HbInputModeCustom &&
                (properties.language() == language || properties.language() == HbInputLanguage()) &&
                ((orientation == Qt::Vertical && properties.keyboard() == HbKeyboardTouchPortrait) ||
                (orientation == Qt::Horizontal && properties.keyboard() == HbKeyboardTouchLandscape))) {

                result.append(d->mMethods[i].descriptor);
                break;
            }
        }
    }

    return result;
}

/*!
\internal
Returns default input method for given orientation.
*/
HbInputMethodDescriptor HbInputModeCache::defaultInputMethod(Qt::Orientation orientation)
{
    Q_D(HbInputModeCache);
    d->refresh();

    HbInputLanguage currentLanguage = HbInputSettingProxy::instance()->globalInputLanguage();
    bool mapped = d->isMappedLanguage(currentLanguage);

    for (int i = 0; i < d->mMethods.count(); i++) {
        foreach (const QString &language, d->mMethods[i].languages) {
            HbInputModeProperties properties = HbInputModeProperties::fromString(language);

            if (properties.language().undefined()) {
                // The input method reports language range but current language is not mapped
                // language. Skip this one. 
                if (!mapped) {
                    continue; 
                }
            } else {
                // The input method reports support for specific language but it is not an exact
                // match to current language. Skip this one 
                if (properties.language() != currentLanguage) {
                    // It is not direct match either.
                    continue;
                }
            }

            // Find default method that supports given orientation
            if (properties.inputMode() == HbInputModeDefault &&
                ((orientation == Qt::Vertical && properties.keyboard() == HbKeyboardTouchPortrait) ||
                (orientation == Qt::Horizontal && properties.keyboard() == HbKeyboardTouchLandscape))) {
                return d->mMethods[i].descriptor;
            }
        }
    }

    return HbInputMethodDescriptor();
}

/*!
\internal
Find correct handler for given input state.
*/
HbInputMethod *HbInputModeCache::findStateHandler(const HbInputState &state)
{
    Q_D(HbInputModeCache);
    d->refresh();

    HbInputModeProperties stateProperties(state);
    int languageRangeIndex = -1;

    // First check if there is a method that matches excatly (ie. also specifies
    // the language).
    for (int i = 0; i < d->mMethods.count(); i++) {
        foreach(const QString &language, d->mMethods[i].languages) {
            HbInputModeProperties properties = HbInputModeProperties::fromString(language);
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

        foreach(const HbInputLanguage &language, languages) {
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
HbInputMethod *HbInputModeCache::activeMethod()
{
    Q_D(HbInputModeCache);
    d->refresh();

    foreach(const HbInputMethodListItem &item, d->mMethods) {
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
QList<HbInputLanguage> HbInputModeCache::listInputLanguages()
{
    Q_D(HbInputModeCache);
    d->refresh();

    QList<HbInputLanguage> result;

    foreach(const HbInputMethodListItem &item, d->mMethods) {
        foreach(const QString &language, item.languages) {
            HbInputModeProperties mode = HbInputModeProperties::fromString(language);
            if (mode.inputMode() != HbInputModeCustom) {
                if (mode.language().undefined()) {
                    // This is language range. Let's add everything
                    // we have key mappings for.
                    QList<HbInputLanguage> languages = HbKeymapFactory::instance()->availableLanguages();
                    foreach(const HbInputLanguage &mappedLanguage, languages) {
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
bool HbInputModeCache::acceptsState(const HbInputMethod *inputMethod, const HbInputState &state)
{
    Q_D(HbInputModeCache);
    d->refresh();

    foreach(const HbInputMethodListItem &item, d->mMethods) {
        if (item.cached == inputMethod) {
            foreach(const QString &language, item.languages) {
                HbInputModeProperties mode = HbInputModeProperties::fromString(language);
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
HbInputMethodDescriptor HbInputModeCache::descriptor(const HbInputMethod *inputMethod)
{
    Q_D(HbInputModeCache);
    d->refresh();

    foreach(const HbInputMethodListItem &item, d->mMethods) {
        if (item.cached == inputMethod) {
            return item.descriptor;
        }
    }

    return HbInputMethodDescriptor();
}


// End of file

