/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbthemeserverutils_p.h"

#include <QHash>
#include <QString>
#include <QFile>
#include <QDebug>

#include "hbmemoryutils_p.h"
#include "hbcssparser_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbeffectxmlparser_p.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hbthemeperf_p.h"
#include "hbcache_p.h"
#include "hbiconsource_p.h"
#include "hbwidgetloader_p.h"
#include "hbwidgetloaderactions_p.h"

//Hash of fileName-offset
typedef QHash<QString, int> HbServerCache;

Q_GLOBAL_STATIC(HbServerCache, effCache)

// cache at the server side to maintain the offsets for
// the filename,layout and section combination.
typedef QHash<QString, int> ServerHashForLayoutDefs;
Q_GLOBAL_STATIC(ServerHashForLayoutDefs, layoutDefsCache)

static const int ICON_SOURCES_MAX_SIZE = 8;
static QList<HbIconSource *> iconSources; // cache of recently used icon sources

HbIconSource *HbThemeServerUtils::getIconSource(const QString &filename)
{
    foreach(HbIconSource *source, iconSources) {
        if (source->filename() == filename) {
            return source;
        }
    }
    // Add new icon source last in the list, delete the first entry if the max size is exceeded.
    QScopedPointer<HbIconSource> tempHbIconSource(new HbIconSource(filename));
    HbIconSource *newSource = tempHbIconSource.data();

    if (iconSources.count() >= ICON_SOURCES_MAX_SIZE) {
        delete iconSources.takeFirst();
    }
    iconSources.append(newSource);
    tempHbIconSource.take();
    return newSource;
}

/**
 * formatFromPath
 */
QString HbThemeServerUtils::formatFromPath(const QString &iconPath)
{
    QString suffix = QFileInfo(iconPath).suffix().toUpper();

    if (suffix == "SVGZ") {
        return "SVG";
    }

    if (suffix == "QPIC") {
        return "PIC";
    }
    if (suffix == "XML" || suffix == "AXML" || suffix == "FXML") {
        return "BLOB";
    }
    return suffix;
}

/**
 * HbThemeServerUtils::getSharedStylesheet() parses the requested stylesheet
 * in shared memory and returns its offset. The offset could be -1 in case of
 * OOM condition or any error while parsing the css file.
 *
 * \param fileName required stylesheet (complete file path)
 * \param priority layer priority
 * \return offset of the shared style sheet in the shared memory, -1 if error
 *
 */
int HbThemeServerUtils::getSharedStylesheet(const QString &fileName,
                                            HbLayeredStyleLoader::LayerPriority priority,
                                            bool *inSharedCache)
{
    int cssOffset = -1;
    HbSharedCache *sharedCache = 0;
    if (priority == HbLayeredStyleLoader::Priority_Core) {
        sharedCache = HbSharedCache::instance();
        cssOffset = sharedCache->offset(HbSharedCache::Stylesheet, fileName);
        if (inSharedCache) {
            *inSharedCache = (cssOffset >= 0) ? true : false;
        }
    } else if (inSharedCache) {
        *inSharedCache = false;
    }

#ifdef THEME_SERVER_TRACES
    qDebug() << "In " << Q_FUNC_INFO;
#endif // THEME_SERVER_TRACES
    if (cssOffset < 0) {
        if (QFile::exists(fileName)) {
            HbCss::Parser parser;
            if (!parseCssFile(parser, fileName, cssOffset)) {
                if (parser.errorCode == HbCss::Parser::OutOfMemoryError) {
                    return OUT_OF_MEMORY_ERROR;
                }
            }
        }
        if (sharedCache) { //sharedCache valid only when priority is Priority_Core
            sharedCache->add(HbSharedCache::Stylesheet, fileName, cssOffset);
        }
    }
    return cssOffset;
}

/**
 * HbThemeServerUtils::parseCssFile()
 *
 * Returns false in case Css file has some error or there is not enough memory
 */
bool HbThemeServerUtils::parseCssFile(HbCss::Parser &parser, const QString &fileName, int &cssOffset)
{
    bool retVal = false;
    // 1. Create a styleSheet in shared memory
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory) ;
    HB_START_SHAREDMEMORY_PRINT("");
    HB_START_HEAPMEMORY_PRINT("");
    HbCss::StyleSheet *styleSheet = 0;
    try {
        cssOffset = manager->alloc(sizeof(HbCss::StyleSheet));
        styleSheet = new((char*)manager->base() + cssOffset) HbCss::StyleSheet(HbMemoryManager::SharedMemory);
    } catch (std::bad_alloc &) {
        if (cssOffset != -1) {
            // if manager->alloc in the previous try block suceeds but creation of
            // HbCss::StyleSheet on shared memory failed
            manager->free(cssOffset);
            cssOffset = -1;
        }
        return retVal;
    }
    // 2. Parse the required file into styleSheet.
    parser.init(fileName, true);
    
    retVal = parser.parse(styleSheet);
    if (!retVal) {
        //parser::parse returns false in a number of scenarios
        // 1. css file has some error
        // 2. shared memory operations on HbVector/HbString/HbVariant threw an exception
        // in either case free the memory occupied by stylesheet
        HbMemoryUtils::release<HbCss::StyleSheet>(styleSheet);
        cssOffset = -1;
    }
    HB_END_SHAREDMEMORY_PRINT("");
    HB_END_HEAPMEMORY_PRINT("");
    return retVal;
}

/*
  Returns of the offset for the given filename,layout and section name.
 */

int HbThemeServerUtils::getSharedLayoutDefinition(const QString & fileName, const QString &layout, const QString &section)
{
    int layoutDefOffset = -1;
    // check in the cache.
    QString key(fileName + layout + section);
    if (layoutDefsCache()->contains(key)) {
        layoutDefOffset = layoutDefsCache()->value(key);
        return layoutDefOffset;
    }
    HbWidgetLoader loader;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning("Unable to open file");
        return -1;
    }
#ifdef THEME_SERVER_TRACES
    qDebug() << "Trying to load: " << fileName << "::" << layout << "::" << section;
#endif // THEME_SERVER_TRACES


    HbWidgetLoader::LayoutDefinition *layoutDef(0);
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    try {
        layoutDefOffset = manager->alloc(sizeof(HbWidgetLoader::LayoutDefinition));
        layoutDef = new((char*)manager->base() + layoutDefOffset)
            HbWidgetLoader::LayoutDefinition(HbMemoryManager::SharedMemory);
    } catch (std::bad_alloc &badAlloc) {
        Q_UNUSED(badAlloc)
        if (layoutDefOffset != -1) {
            manager->free(layoutDefOffset);
            layoutDefOffset = -1;
        }
        return layoutDefOffset;
    }

    if (loader.loadLayoutDefinition(layoutDef, &file, layout, section)) {
        layoutDefsCache()->insert(key, layoutDefOffset);
        // add the filename and css offset to the shared cache.
        // no need to check if this item is already present in the
        // cache as the parsing of the file happens only once
        // in the server side.
        HbSharedCache::instance()->add(HbSharedCache::LayoutDefinition, key, layoutDefOffset);
    } else {
        // load failed
        layoutDef->~LayoutDefinition();
        manager->free(layoutDefOffset);
        layoutDefOffset = -1;
    }
    return layoutDefOffset;
}

/**
 * Creates the singleton HbDeviceProfileDatabase instance in shared memory
 *
 */
void HbThemeServerUtils::createDeviceProfileDatabase()
{
    HbDeviceProfileDatabase::instance(HbMemoryManager::SharedMemory);
}

/**
 * Removes fxml document from the shared memory and effects cache
 *
 * \param fileName of the removed fxml file
 * \return true if effect was found and removed, false otherwise
 */
bool HbThemeServerUtils::removeSharedEffect(const QString &fileName)
{
    if (effCache()->contains(fileName)) {
        effCache()->remove(fileName);
        HbSharedCache::instance()->remove(HbSharedCache::Effect, fileName);
        return true;
    }
    return false;
}

/**
 * Clears fxml documents from the shared memory and effects cache
 *
 */
void HbThemeServerUtils::clearSharedEffects()
{
    HbSharedCache *cache = HbSharedCache::instance();
    HbServerCache::const_iterator iterEnd(effCache()->constEnd());
    for (HbServerCache::const_iterator iter = effCache()->constBegin();iter != iterEnd;++iter) {
        cache->remove(HbSharedCache::Effect, iter.key());
    }
    effCache()->clear();
}

/**
 * getSharedEffect parses the requested fxml document into the shared
 * memory and returns the offset of the parsed data.
 *
 * \param fileName requested fxml file
 * \return offset of the parsed effect data in the shared memory, -1 if error
 *
 */
int HbThemeServerUtils::getSharedEffect(const QString &fileName)
{
#ifdef THEME_SERVER_TRACES
    qDebug() << "In " << Q_FUNC_INFO << fileName;
#endif // THEME_SERVER_TRACES

    int effOffset = -1;
    if (effCache()->contains(fileName)) {
        effOffset = effCache()->value(fileName);

#ifdef THEME_SERVER_TRACES
        qDebug() << "Got effect in cache: " << fileName << "offset=" << effOffset;
#endif //THEME_SERVER_TRACES

    } else {

#ifdef THEME_SERVER_TRACES
        qDebug() << "Not in cache, parsing: " << fileName;
#endif //THEME_SERVER_TRACES

        // 1. Create an effect in shared memory
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        HbEffectFxmlData *data = 0;
        try {
            effOffset = manager->alloc(sizeof(HbEffectFxmlData));
            data = new((char*)manager->base() + effOffset) HbEffectFxmlData(HbMemoryManager::SharedMemory);
        } catch (std::exception &) {
            if (effOffset != -1) {
                // if manager->alloc in the previous try block suceeds but creation of
                // HbEffectFxmlData on shared memory failed
                manager->free(effOffset);
                effOffset = -1;
            }
            return effOffset;
        }
        // 2. Parse the file.
        HbEffectXmlParser parser;

        QFile f(fileName);
        bool fail = false;
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            try {
                parser.read(&f, data);
                f.close();

                // 3. Mark an entry for this styleSheet into the table
                effCache()->insert(fileName, effOffset);
            } catch (std::exception &) {
                f.close();
                fail = true;
            }
        } else {
#ifdef THEME_SERVER_TRACES
            qWarning() << "Cannot open" << fileName;
#endif // THEME_SERVER_TRACES
            fail = true;
        }
        if (fail) {
            data->~HbEffectFxmlData();
            manager->free(effOffset);
            return -1;
        }

        // add the filename and css offset to the secondary cache.
        // no need to check if this item is already present in the
        // cache as the parsing of the file happens only once
        // in the server side.
        HbSharedCache::instance()->add(HbSharedCache::Effect, fileName, effOffset);
    }

#ifdef THEME_SERVER_TRACES
    qDebug() << "returning offset: " << effOffset;
#endif // THEME_SERVER_TRACES

    return effOffset;
}

/**
 * cleanupUnusedCss  function removes css-resources (stylesheets), whose reference count
 * is zero, it also releases the shared memory occupied by those resources.
 * \param cache server css-cache
 *
 */
void HbThemeServerUtils::cleanupUnusedCss(HbCache *cache)
{
    QList<HbCacheItem*> list = cache->lruList();
    while (!list.isEmpty()) {
        HbCacheItem* itemToRemove = list.takeFirst();
        if (itemToRemove->offset != -1) {
            HbCss::StyleSheet *styleSheet =
                HbMemoryUtils::getAddress<HbCss::StyleSheet>(HbMemoryManager::SharedMemory,
                        itemToRemove->offset);
            HbMemoryUtils::release<HbCss::StyleSheet>(styleSheet);
            itemToRemove->offset = -1;
        }
        //Since we are cleaning up css-resources whose ref-count is zero, these entries will be
        // removed from actual cache.
        delete cache->cacheHandle().take(itemToRemove->fileName);
    }
}

