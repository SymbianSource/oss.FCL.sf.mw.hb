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

#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSvgRenderer>

#include "hbmemoryutils_p.h"
#include "hbcssparser_p.h"
#include "hbeffectfxmldata_p.h"
#include "hbeffectxmlparser_p.h"
#include "hbdeviceprofiledatabase_p.h"
#include "hbthemeperf_p.h"
#include "hbiconsource_p.h"
#include "hbwidgetloader_p.h"
#include "hbwidgetloaderactions_p.h"
#include "hbthemeindex_p.h"

//Hash of fileName-offset
typedef QHash<QString, int> HbServerCache;

Q_GLOBAL_STATIC(HbServerCache, effCache)

// cache at the server side to maintain the offsets for
// the filename,layout and section combination.
typedef QHash<QString, int> ServerHashForLayoutDefs;
Q_GLOBAL_STATIC(ServerHashForLayoutDefs, layoutDefsCache)

// Offset for the cache of the missed CSS files for classes starting 'hb'
static int missedHbCssFilesOffset = -1;
static const QString HB_CLASSNAME_PREFIX("hb");

static const int ICON_SOURCES_MAX_SIZE = 8;
static QList<HbIconSource *> iconSources; // cache of recently used icon sources


const int PIXEL_DEPTH = 4;
const int TILE_SIZE_IN_PIXELS = 32 * 32;
const int SGIMAGE_PIXEL_OVERHEAD = 200;

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
                                            bool &fileExists,
                                            bool *inSharedCache)
{
    qptrdiff cssOffset = -1;
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

    if (cssOffset < 0) {
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "Stylesheet: " << fileName << " not found in shared cache.";
        HbCss::Parser parser;
        if (!parseCssFile(parser, fileName, cssOffset, fileExists)) {
            if (parser.errorCode == HbCss::Parser::OutOfMemoryError) {
                return OUT_OF_MEMORY_ERROR;
            }
        }
        if (sharedCache) { //sharedCache valid only when priority is Priority_Core
            sharedCache->add(HbSharedCache::Stylesheet, fileName, cssOffset);
        }
    }
    return cssOffset;
}

/*
 * Returns the offset of the missed CSS files for widgets starting with 'hb'
 */
int HbThemeServerUtils::getMissedHbCssFilesOffset()
{
    if (missedHbCssFilesOffset < 0) {
        initMissedHbCssFilesList();
    }
    return missedHbCssFilesOffset;
}

void HbThemeServerUtils::initMissedHbCssFilesList()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    int offset = -1;
    try {
        offset = manager->alloc(sizeof(HbVector<uint>));
        HbVector<uint> *list = new((char*)manager->base() + offset)
                HbVector<uint>(HbMemoryManager::SharedMemory);
    } catch (std::bad_alloc &) {
        if (offset >= 0) {
            manager->free(offset);
            offset = -1;
        }
    }
    missedHbCssFilesOffset = offset;
}

/**
 * HbThemeServerUtils::parseCssFile()
 *
 * Returns false in case Css file has some error or there is not enough memory
 */
bool HbThemeServerUtils::parseCssFile(HbCss::Parser &parser, const QString &fileName,
                                      qptrdiff &cssOffset, bool &fileExists)
{
    bool retVal = false;
    // 1. Create a styleSheet in shared memory
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory) ;
    HB_START_SHAREDMEMORY_PRINT("");
    HB_START_HEAPMEMORY_PRINT("");
    HbCss::StyleSheet *styleSheet = 0;
    try {
        cssOffset = manager->alloc(sizeof(HbCss::StyleSheet));
        styleSheet = new(static_cast<char *>(manager->base()) + cssOffset)
                     HbCss::StyleSheet(HbMemoryManager::SharedMemory);
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
    fileExists = parser.init(fileName, true);
    if (!fileExists) {
        if (QFileInfo(fileName).fileName().startsWith(HB_CLASSNAME_PREFIX)) {
            HbVector<uint> *list = HbMemoryUtils::getAddress<HbVector<uint> >(
                HbMemoryManager::SharedMemory, getMissedHbCssFilesOffset());
            if (list) {
                list->append(qHash(fileName));
            }
        }
        if (cssOffset != -1) {
            manager->free(cssOffset);
            cssOffset = -1;
        }
        return retVal;
    }
    
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

int HbThemeServerUtils::getSharedLayoutDefinition(const QString &fileName, const QString &layout,
                                                  const QString &section, bool &fileExists)
{
    qptrdiff layoutDefOffset = -1;
    QStringRef nameKey(&fileName);
    if (nameKey.at(0) == ':') {
        //use only filename as a key.
        int index = fileName.lastIndexOf('/');
        if (index >= 0) {
            nameKey = fileName.rightRef((fileName.size() - 1) - index);
        }
    }
    // check in the cache.
    QString key;
    QChar separator('\0');
    key.reserve(nameKey.length() + 2 //separators
                + layout.length() + section.length());
    key.append(nameKey)
       .append(separator)
       .append(layout)
       .append(separator)
       .append(section);

    if (layoutDefsCache()->contains(key)) {
        layoutDefOffset = layoutDefsCache()->value(key);
        return layoutDefOffset;
    }
    HbWidgetLoader loader;

    QFile file(fileName);
    fileExists = file.open(QFile::ReadOnly | QFile::Text);
    if (!fileExists) {
        THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "Unable to open the file: " << fileName;
        return -1;
    }
    THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "Trying to load: " << fileName << "::" << layout << "::" << section;

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
        HbSharedCache::instance()->addLayoutDefinition(fileName, layout, section,
                                                       layoutDefOffset);
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
    THEME_GENERIC_DEBUG() << Q_FUNC_INFO << "called with fileName: " << fileName;

    qptrdiff effOffset = -1;
    if (effCache()->contains(fileName)) {
        effOffset = effCache()->value(fileName);
        THEME_GENERIC_DEBUG() << "in cache: " << fileName << "offset=" << effOffset;
    } else {
        THEME_GENERIC_DEBUG() << "Not in cache, parsing: " << fileName;
        
        // 1. Create an effect in shared memory
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        HbEffectFxmlData *data = 0;
        try {
            effOffset = manager->alloc(sizeof(HbEffectFxmlData));
            data = new(static_cast<char*>(manager->base()) + effOffset)
                   HbEffectFxmlData(HbMemoryManager::SharedMemory);
        } catch (std::exception &) {
            if (effOffset != -1) {
                // if manager->alloc in the previous try block suceeds but creation of
                // HbEffectFxmlData on shared memory failed
                manager->free(effOffset);
                effOffset = -1;
            }
            THEME_GENERIC_DEBUG() << "Exception when allocating shared memory.";
            return effOffset;
        }
        // 2. Parse the file.
        HbEffectXmlParser parser;

        QBuffer fxmlBuffer;
        QFile f(fileName);
        bool fail = false;

        if (f.open(QIODevice::ReadOnly)) {
            try {
                QByteArray arr(f.readAll());
                fxmlBuffer.setBuffer(&arr);
                if (fxmlBuffer.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    parser.read(&fxmlBuffer, data);
                } else {
                    THEME_GENERIC_DEBUG() << "Cannot open fxmlBuffer from file:" << fileName;
                    fail = true;
                }

                // 3. Mark an entry for this styleSheet into the table
                if (!fail) {
                    effCache()->insert(fileName, effOffset);
                }
            } catch (std::exception &) {
                fail = true;
            }
        } else {
            THEME_GENERIC_DEBUG() << "Cannot open:" << fileName;
            fail = true;
        }
        if (fail) {
            data->~HbEffectFxmlData();
            manager->free(effOffset);
            THEME_GENERIC_DEBUG() << "Failed to parse effect.";
            return -1;
        }

        // These will be open, if we got here
        f.close();
        fxmlBuffer.close();

        // add the filename and css offset to the secondary cache.
        // no need to check if this item is already present in the
        // cache as the parsing of the file happens only once
        // in the server side.
        HbSharedCache::instance()->add(HbSharedCache::Effect, fileName, effOffset);
    }
    
    THEME_GENERIC_DEBUG() << "returning offset: " << effOffset;
    return effOffset;
}

void HbThemeServerUtils::createThemeIndex(const QString &themePath, const HbThemeType &themetype)
{
    THEME_INDEX_DEBUG() <<  "ThemeIndex: createThemeIndex" << themePath.toUtf8();
    QDir themeBasePath(themePath);
    // Path is like "C:/resource/hb/themes/icons/sfwhitetheme"
    QString themeName = themeBasePath.dirName();
    
    QString filename(themeBasePath.absolutePath());
    int cutindex = filename.lastIndexOf('/',filename.lastIndexOf('/')-1)+1;
    filename = filename.left(cutindex);
    themeBasePath.setPath(filename);
    filename.append(themeName);
    filename.append(".themeindex");

    if (themePath == ":\\themes\\icons\\hbdefault") {
        themeBasePath.setPath(":/themes");
        filename = ":/themes/hbdefault.themeindex";
        themeName = "hbdefault";
    }
    
    QFile indexFile(filename);

    bool indexOK = false;

    if (indexFile.open(QIODevice::ReadOnly)) {
        indexOK = true;
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);

        HbSharedChunkHeader *chunkHeader = (HbSharedChunkHeader*) manager->base();        
        qint64 byteSize = indexFile.size();
        THEME_INDEX_DEBUG() <<  "ThemeIndex: " << themeName.toUtf8() << " index file size:" << byteSize;

        int offset = manager->alloc(byteSize);
        if (offset >= 0) {
            THEME_INDEX_DEBUG() <<  "ThemeIndex: memory allocated for theme: " << themeName.toUtf8();

            // Read the theme index in the shared chunk
            char *address = HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, offset);

            indexFile.read(address, byteSize);
            indexFile.close();
            THEME_INDEX_DEBUG() <<  "ThemeIndex: Reading themeindex for theme" << themeName.toUtf8() << "... Done!";

            // Verify theme index contents if it is not located in ROM,
            // so that it does not have over-indexing offsets which might
            // crash all the clients trying to read from it.

            if (themePath[0] != 'z' && themePath[0] != 'Z') {
                THEME_INDEX_DEBUG() <<  "ThemeIndex: Validating themeindex for theme" << themeName.toUtf8();
                HbThemeIndex index(address);
                indexOK = index.validateItems(byteSize);
                THEME_INDEX_DEBUG() <<  "ThemeIndex: Validating themeindex for theme" << themeName.toUtf8() << " done! Result: " << indexOK;
            }

            if (indexOK) {
                // Allocate theme path string from shared memory
                QByteArray themePathArray = themeBasePath.absolutePath().toLatin1();
                quint32 themePathOffset = manager->alloc(themePathArray.size()+1);
                memcpy(HbMemoryUtils::getAddress<char>(
                           HbMemoryManager::SharedMemory,
                           themePathOffset),
                           themePathArray.data(),
                           themePathArray.size()+1); // +1 for '/0'

                // Allocate theme name string from shared memory
                QByteArray themeNameArray = themeName.toLatin1();
                quint32 themeNameOffset = manager->alloc(themeNameArray.size()+1);
                memcpy(HbMemoryUtils::getAddress<char>(
                           HbMemoryManager::SharedMemory,
                           themeNameOffset),
                           themeNameArray.data(),
                           themeNameArray.size()+1); // +1 for '/0'

                // Store offset to the index in chunk
                switch (themetype) {
                case BaseTheme:
                    // Release previously allocated content
                    manager->free(chunkHeader->baseThemePathOffset);
                    manager->free(chunkHeader->baseThemeNameOffset);
                    manager->free(chunkHeader->baseThemeIndexOffset);
                    // Base theme offsets
                    chunkHeader->baseThemePathOffset = themePathOffset;
                    chunkHeader->baseThemeNameOffset = themeNameOffset;
                    chunkHeader->baseThemeIndexOffset = offset;
                    break;
                case OperatorC:
                    // Release previously allocated content
                    manager->free(chunkHeader->operatorThemeDriveCPathOffset);
                    manager->free(chunkHeader->operatorThemeDriveCNameOffset);
                    manager->free(chunkHeader->operatorThemeDriveCIndexOffset);
                    // Operator theme in C-drive offsets
                    chunkHeader->operatorThemeDriveCPathOffset = themePathOffset;
                    chunkHeader->operatorThemeDriveCNameOffset = themeNameOffset;
                    chunkHeader->operatorThemeDriveCIndexOffset = offset;
                    break;
                case OperatorROM:
                    // Release previously allocated content
                    manager->free(chunkHeader->operatorThemeRomPathOffset);
                    manager->free(chunkHeader->operatorThemeRomNameOffset);
                    manager->free(chunkHeader->operatorThemeRomIndexOffset);
                    // Operator theme in ROM offsets
                    chunkHeader->operatorThemeRomPathOffset = themePathOffset;
                    chunkHeader->operatorThemeRomNameOffset = themeNameOffset;
                    chunkHeader->operatorThemeRomIndexOffset = offset;
                    break;
                case ActiveTheme:
                    // Release previously allocated content
                    manager->free(chunkHeader->activeThemePathOffset);
                    manager->free(chunkHeader->activeThemeNameOffset);
                    manager->free(chunkHeader->activeThemeIndexOffset);
                    // Active theme offsets
                    chunkHeader->activeThemePathOffset = themePathOffset;
                    chunkHeader->activeThemeNameOffset = themeNameOffset;
                    chunkHeader->activeThemeIndexOffset = offset;
                    break;
                default:
                    break;
                }
            } else {
                // If the index contents were not OK, remove the index from the chunk
                manager->free(offset);
            }
        }
    }
}

int HbThemeServerUtils::computeCpuCost(const HbIconKey & key, HbIconFormatType iconType,
                                        bool isConsolidatedIcon)
{
    int cpuItemCost = 0;
    if (isConsolidatedIcon) {
        QSize size = key.size.toSize();
        cpuItemCost = size.width() * size.height() * PIXEL_DEPTH;
        return cpuItemCost;
    }
    HbIconSource *source = HbThemeServerUtils::getIconSource(key.filename);
    if (source) {
        if (iconType == BLOB) {
            QByteArray *sourceByteArray = source->byteArray();
            if (sourceByteArray) {
                cpuItemCost = sourceByteArray->length();
            }
        } else if (iconType == NVG) {
            QByteArray *sourceByteArray = source->byteArray();
            if (sourceByteArray) {
                QSizeF renderSize = source->defaultSize();
                if (!key.size.isNull()) {
                    renderSize.scale(key.size,key.aspectRatioMode);
                }
                QSize size = renderSize.toSize();
                cpuItemCost = size.width() * size.height() * PIXEL_DEPTH;
            }    
        } else if (iconType == SVG) {
            QSvgRenderer *svgRenderer = source->svgRenderer();
            if (svgRenderer && svgRenderer->isValid()) {
                QSizeF renderSize = QSizeF(svgRenderer->defaultSize());
                if (!key.size.isNull()) {
                    renderSize.scale(key.size, key.aspectRatioMode);
                }
                QSize size = renderSize.toSize();
                cpuItemCost = size.width() * size.height() * PIXEL_DEPTH;
                source->releaseSvgRenderer();
                
            }    
        } else {
            QPixmap pm = QPixmap::fromImage(*source->image());
            QSizeF renderSize = pm.size();
            if (!key.size.isNull() ) {
                renderSize.scale(key.size, key.aspectRatioMode);
            }
            QSize size = renderSize.toSize();
            cpuItemCost = size.width() * size.height() * PIXEL_DEPTH;
            
        }
            
    }
    return cpuItemCost;
}


int HbThemeServerUtils::computeGpuCost(const HbIconKey & key, HbIconFormatType iconType,
                                        bool isConsolidatedIcon)
{
    int gpuItemCost = 0;
    if (isConsolidatedIcon) {
        QSize size = key.size.toSize();
        int xTiles = ceil(size.width() / 32.0);
        int yTiles = ceil(size.height() / 32.0);
        int sizeInPixels =  xTiles * yTiles * TILE_SIZE_IN_PIXELS;
        int sizeInBytes = sizeInPixels * PIXEL_DEPTH;
        gpuItemCost = sizeInBytes + SGIMAGE_PIXEL_OVERHEAD ;
        return gpuItemCost;
    }
    HbIconSource *source = HbThemeServerUtils::getIconSource(key.filename);
    if (source) {
        if (iconType == NVG) {
            QByteArray *sourceByteArray = source->byteArray();
            if (sourceByteArray) {
                QSizeF renderSize = source->defaultSize();
                if (!key.size.isNull()) {
                    renderSize.scale(key.size,key.aspectRatioMode);
                }
                QSize size = renderSize.toSize();
                int xTiles = ceil(size.width() / 32.0);
                int yTiles = ceil(size.height() / 32.0);
                int sizeInPixels =  xTiles * yTiles * TILE_SIZE_IN_PIXELS;
                int sizeInBytes = sizeInPixels * PIXEL_DEPTH;
                gpuItemCost = sizeInBytes + SGIMAGE_PIXEL_OVERHEAD ;
            }
        }
    }
    return gpuItemCost;

}

int HbThemeServerUtils::computeFrameItemCpuCost(const HbIconKey & key, HbIconFormatType iconType, 
                                                HbRenderingMode rm)
{
    int cpuItemCost = 0;
    HbIconSource *source = HbThemeServerUtils::getIconSource(key.filename);
    if (source) {
        if (iconType == SVG) {
            QSvgRenderer *svgRenderer = source->svgRenderer();
            if (svgRenderer && svgRenderer->isValid()) {
                QSizeF renderSize = QSizeF(svgRenderer->defaultSize());
                if (!key.size.isNull()) {
                    renderSize.scale(key.size, key.aspectRatioMode);
                }
                QSize size = renderSize.toSize();
                cpuItemCost = size.width() * size.height() * PIXEL_DEPTH;
                source->releaseSvgRenderer();
                    
            }
        } else if (iconType == NVG) {
            QByteArray *sourceByteArray = source->byteArray();
            if (sourceByteArray) {    
                if (rm == EHWRendering ) {
                    cpuItemCost = sourceByteArray->length();
                } else {
                    QSizeF renderSize = source->defaultSize();
                    if (!key.size.isNull()) {
                        renderSize.scale(key.size,key.aspectRatioMode);
                    }
                    QSize size = renderSize.toSize();
                    cpuItemCost = size.width() * size.height() * PIXEL_DEPTH;
               }
            }
        }
    }    
    return cpuItemCost;
    
}
   

