/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbTools module of the UI Extensions for Mobile.
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
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QDataStream>

#include <hbmemoryutils_p.h>
#include <hbcssparser_p.h>
#include <hbsharedmemorymanager_p.h>
#include <hbsharedcache_p.h>
#include <hbcssconverterutils_p.h>
#include <hboffsetmapbuilder_p.h>
#include <hbwidgetloader_p.h>

// Global variables
static const QString CSSFileExtension = ".css";
static const QString WMLFileExtension = ".widgetml";
static const QString ColorCSSEnding = "_color.css";

static const QString AppName = "hbbincssmaker";
static bool verboseOn = false;

QTextStream out(stdout);
QTextStream verboseOut(stderr);
QTextStream err(stderr);

#define VERBOSE(statement) if(verboseOn) { verboseOut << statement; }
#define VERBOSEIF(test, statement) if(verboseOn && test) { verboseOut << statement; }
#define VERBOSELN(statement) if(verboseOn) { verboseOut << statement << endl; }

struct WidgetMLParseInput
{
    QString filename;
    QList<CSSLayoutInfo> layouts;

    void clear()
    {
        layouts.clear();
        filename.clear();
    }
};

void testDeclarations(const HbVector<HbCss::Declaration> &decls)
{
    VERBOSE("declarations count: ");
    VERBOSELN(decls.count());
    foreach(const HbCss::Declaration &decl, decls) {
        QString prop = decl.property;
        VERBOSELN(prop);
        VERBOSE("declarations values: ");
        VERBOSELN(decl.values.count());
        foreach(const HbCss::Value &value, decl.values) {
            QString declVal = value.variant.toString();
            VERBOSELN(declVal);
        }
    }
}

void testStyleRules(const HbVector<HbCss::StyleRule> &styleRules)
{
    foreach(const HbCss::StyleRule &rule,styleRules) {
        int sels = rule.selectors.count();
        VERBOSE("rule selectors count: ");
        VERBOSELN(sels);
        foreach(const HbCss::Selector &sel, rule.selectors) {
            int basicSel = sel.basicSelectors.count();
            VERBOSE("Basic Selectors: ");
            VERBOSELN(basicSel);
            foreach(const HbCss::BasicSelector &bSel, sel.basicSelectors) {
                QString name = bSel.elementName;
                VERBOSELN(name);
                int pseudos = bSel.pseudos.count();
                VERBOSE("Basic Selectors pseudos: ");
                VERBOSELN(pseudos);
                foreach(const HbCss::Pseudo &pseudo, bSel.pseudos) {
                    quint64 pseudoType = (quint64)pseudo.type; Q_UNUSED(pseudoType);
                    QString name = pseudo.name;
                    QString function = pseudo.function;
                    VERBOSE(name);
                    VERBOSELN(function);
                }
                int selectors = bSel.attributeSelectors.count();
                VERBOSE("Basic Selectors attribute selectors: ");
                VERBOSELN(selectors);
                foreach(const HbCss::AttributeSelector &aSel, bSel.attributeSelectors) {
                    QString sel = aSel.name;
                    VERBOSELN(sel);
                }
            }
        }
        testDeclarations(rule.declarations);
    }
}

void testStyleSheet(HbCss::StyleSheet *styleSheet)
{
    int ruleCount = styleSheet->variableRules.count();
    VERBOSE("rules count: ");
    VERBOSELN(ruleCount);
    foreach(const HbCss::VariableRule &rule, styleSheet->variableRules) {
        testDeclarations(rule.declarations);
    }
    int widgetRules = styleSheet->widgetRules.count();
    VERBOSE("widget rules count: ");
    VERBOSELN(widgetRules);
    foreach(const HbCss::WidgetStyleRules &rule, styleSheet->widgetRules) {
        testStyleRules(rule.styleRules);
        testStyleRules(rule.portraitRules);
        testStyleRules(rule.landscapeRules);
    }
}

void testLayoutDef(const HbWidgetLoader::LayoutDefinition *layoutDef)
{
    VERBOSELN("anchor items count: " << layoutDef->anchorItems.count());
    for (int i = 0; i < layoutDef->anchorItems.count(); ++i) {
        const HbWidgetLoader::AnchorItem &anchorItem = layoutDef->anchorItems.at(i);
        VERBOSELN("srcId: " << anchorItem.srcId);
        VERBOSELN("dstId: " << anchorItem.dstId);
        VERBOSELN("prefLength text: " << anchorItem.prefText);
        VERBOSELN("anchorId: " << anchorItem.anchorId);
    }
}

bool testCss()
{
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);
    HbSharedCache *cache = shared->cache();

    for (int k = 0; k < cache->mOffsetItemCount; ++k) {
        if (cache->mOffsetItems[k].offsetCSS >= 0) {
            HbCss::StyleSheet *sheet = HbMemoryUtils::getAddress<HbCss::StyleSheet>(
                HbMemoryManager::SharedMemory, cache->mOffsetItems[k].offsetCSS);
            VERBOSELN("Cssmap item " << k
                    << "- hash value: \"" << cache->mOffsetItems[k].offsetCSS << "\"");
            // Tests the stylesheet offsets and prints info to verbose out
            testStyleSheet(sheet);

            //test layout definition.
            int tableSize = 0;
            const HbLayoutIndexItem *ptr = cache->layoutIndexItemBegin(
                    cache->mOffsetItems[k].offsetLayoutIndexTable, &tableSize);
            for (; tableSize > 0; --tableSize, ++ptr) {
                HbWidgetLoader::LayoutDefinition *layoutDef =
                    HbMemoryUtils::getAddress<HbWidgetLoader::LayoutDefinition>(
                            HbMemoryManager::SharedMemory, ptr->offset);
                testLayoutDef(layoutDef);
            }
        }
    }
    return true;
}

/*!
    Collects and return layout names from \a styleRules.
    \a layoutSet - map of already found layoutnames and sections to prevent duplicate layouts
    to be added.

*/
QList<CSSLayoutInfo> collectLayoutNames(
        QSet<QPair<QString, QString> > &layoutSet,
        const HbVector<HbCss::StyleRule> &styleRules)
{
    QList<CSSLayoutInfo> layouts;
    layouts.append(CSSLayoutInfo());

    foreach(const HbCss::StyleRule &rule, styleRules) {
        foreach(const HbCss::Declaration &decl, rule.declarations) {
            if (decl.propertyId == HbCss::Property_Layout) {
                if (decl.values.count() == 1) {
                    layouts.last().layoutname = decl.values.at(0).variant.toString();
                }
            } else if (decl.propertyId == HbCss::Property_Section) {
                if (decl.values.count() == 1) {
                    layouts.last().section = decl.values.at(0).variant.toString();
                }
            }
        }
        const CSSLayoutInfo &infoRef = layouts.last();
        if (!infoRef.layoutname.isEmpty()) {
            QPair<QString, QString> layout = qMakePair(infoRef.layoutname,
                                                       infoRef.section);
            //only add layout, if not already collected before.
            if (!layoutSet.contains(layout)) {
                layoutSet.insert(layout);
                layouts.append(CSSLayoutInfo());
            }
        }
    }
    layouts.removeLast();
    return layouts;
}

/*!
    Collects all the layouts defined in \a stylesheet and add the result to \a input.
    returns true, if at least one layout is found.
*/
bool collectLayouts(const QString &cssFilePath,
                    HbCss::StyleSheet *styleSheet,
                    WidgetMLParseInput &input)
{
    input.clear();
    QSet<QPair<QString, QString> > layoutSet; //for removing duplicate layout names.
    foreach(const HbCss::WidgetStyleRules &rule, styleSheet->widgetRules) {
        layoutSet.clear();
        input.layouts += collectLayoutNames(layoutSet, rule.styleRules);
        input.layouts += collectLayoutNames(layoutSet, rule.portraitRules);
        input.layouts += collectLayoutNames(layoutSet, rule.landscapeRules);
    }
    bool hasLayoutDef = false;
    if (!input.layouts.isEmpty()) {
        //if css contains at least 1 layout declaration, it might have .widgetml file.
        QString filePath(cssFilePath);
        filePath.replace(filePath.size() - CSSFileExtension.size(),
                         CSSFileExtension.size(), WMLFileExtension);
        input.filename = filePath;
        hasLayoutDef = true;
    }
    return hasLayoutDef;
}

void printHelp()
{
    out << AppName << " usage:\n\n";
    out << AppName << " [-v] -i <css filename or directory> -o <target binary file>";
    out << "-v \t\tverbose output\n\n";
}

QFileInfoList collectFiles(const QFileInfoList &inputFiles, const QStringList &filters)
{
    QFileInfoList collected;
    Q_FOREACH(const QFileInfo &inputFile, inputFiles) {
        if (inputFile.isDir()) {
            QDir dir(inputFile.absoluteFilePath());
            QFileInfoList entries = dir.entryInfoList(filters,
                    QDir::AllDirs | QDir::Readable | QDir::Files | QDir::NoDotAndDotDot);
            collected += collectFiles(entries, filters);
        } else {
            collected << inputFile;
        }
    }
    return collected;
}

QMap<QString, QFileInfo> collectCssFiles(const QStringList &inputFilePaths)
{
    QStringList filters;
    filters << "*.css";

    QMap<QString, QFileInfo> cssFileMap;
    QFileInfoList inputPath;
    inputPath.append(QFileInfo());
    Q_FOREACH(const QString &path, inputFilePaths) {
        inputPath[0].setFile(path);
        QFileInfoList allFiles = collectFiles(inputPath, filters);
        Q_FOREACH(const QFileInfo &info, allFiles) {
            QMap<QString, QFileInfo>::const_iterator i = cssFileMap.find(info.fileName());
            if (i == cssFileMap.end()) {
                cssFileMap.insert(info.fileName(), info);
            } else {
                err << "duplicate css filenames found: " << i.value().absoluteFilePath() <<
                       " & " << info.absoluteFilePath();
            }
        }
    }
    return cssFileMap;
}

/*!
    Collects the css files from \a inputFiles, parses them to shared memory, stores
    offsets to \a offsetMap.
    returns true on success.
*/
bool parseCss(const QStringList &inputFiles, HbOffsetMapBuilder &offsetMap)
{
    if (inputFiles.isEmpty()) return false;
    QMap<QString, QFileInfo> cssFiles = collectCssFiles(inputFiles);

    HbCss::Parser parser;
    HbCss::StyleSheet *styleSheet = 0;
    bool success = false;
    
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    while (!cssFiles.isEmpty()) {
        QMap<QString, QFileInfo>::iterator first = cssFiles.begin();
        QMap<QString, QFileInfo>::iterator CSSFiles[CSSFileTypeEnd];

        QString widgetName(first.key());
        if (widgetName.endsWith(ColorCSSEnding)) {
            //color css file, find the layout css pair.
            CSSFiles[ColorCSSFile] = first;
            widgetName.remove(widgetName.size() - ColorCSSEnding.size(),
                              ColorCSSEnding.size());
            CSSFiles[CSSFile] = cssFiles.find(widgetName + CSSFileExtension);
        } else {
            //layout css file, find the color css pair.
            CSSFiles[CSSFile] = first;
            widgetName.remove(widgetName.size() - CSSFileExtension.size(),
                              CSSFileExtension.size());
            CSSFiles[ColorCSSFile] = cssFiles.find(widgetName + ColorCSSEnding);
        }
        int offsets[] = {-1, -1};

        for (int i = 0; i < CSSFileTypeEnd; ++i) {
            if (CSSFiles[i] != cssFiles.end()) {
                const QFileInfo &file = CSSFiles[i].value();
                VERBOSE("processing " << file.absoluteFilePath() << "...");
                offsets[i] = manager->alloc(sizeof(HbCss::StyleSheet));
                if (offsets[i] >= 0) {
                    styleSheet = new (static_cast<char*>(manager->base()) + offsets[i])
                                 HbCss::StyleSheet(HbMemoryManager::SharedMemory);
                    parser.init(file.absoluteFilePath(), true);
                    success = parser.parse(styleSheet);
                    VERBOSE("cache key = " << CSSFiles[i].key() << "...");
                }
                if (success) {
                    VERBOSELN("ok");
                } else {
                    VERBOSELN("failed");
                    err << "Failed to parse: " << file.absoluteFilePath() << endl;
                    break;
                }
            }
        }

        const QFileInfo *info = 0;
        QString tmp;
        if (CSSFiles[CSSFile] != cssFiles.end()) {
            tmp = CSSFiles[CSSFile].key();
            info = &CSSFiles[CSSFile].value();
        }
        if (!offsetMap.addWidgetOffsets(widgetName, info, offsets)) {
            return false;
        }

        //remove processed files from the map.
        cssFiles.erase(CSSFiles[ColorCSSFile]);
        if (!tmp.isEmpty()) {
            cssFiles.remove(tmp);
        }
    }
    return success;
}

/*!
    Parses widgetml file and all the layouts using the info from \a parseInput for a widget,
    which hash is \a widgetHash, add offsets to \a offsetMap.
*/
bool parseWidgetML(HbOffsetMapBuilder &offsetMap,
                   quint32 widgetHash,
                   const WidgetMLParseInput &parseInput)
{
    HbWidgetLoader loader;

    VERBOSELN("processing: " << parseInput.filename);
    QFile file(parseInput.filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        VERBOSELN("unable to open file: " << parseInput.filename);
        return false;
    }
    HbWidgetLoader::LayoutDefinition *layoutDef = 0;
    int layoutDefOffset = -1;
    bool success = true;

    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    QList<LayoutItem> layoutInfoList;
    Q_FOREACH(const CSSLayoutInfo &info, parseInput.layouts) {
        VERBOSE("layout: " << info.layoutname << ", " << "section: " << info.section << "...");

        layoutDefOffset = manager->alloc(sizeof(HbWidgetLoader::LayoutDefinition));
        layoutDef = new(static_cast<char *>(manager->base()) + layoutDefOffset)
            HbWidgetLoader::LayoutDefinition(HbMemoryManager::SharedMemory);
        success = loader.loadLayoutDefinition(layoutDef, &file, info.layoutname, info.section);

        if (success) {
            layoutInfoList.append(LayoutItem(&info));
            layoutInfoList.last().offset = layoutDefOffset;
            VERBOSELN("ok");
        } else {
            VERBOSELN("failed");
            break;
        }
        file.seek(0);
    }
    success = offsetMap.addWidgetMLOffsets(parseInput.filename, widgetHash, layoutInfoList);
    return success;
}

/*!
    Parse all the widgetmls to shared memory for widget's found in \a offsetMap,
    store the offsets to \a offsetMap.
*/
bool parseWidgetML(HbOffsetMapBuilder &offsetMap)
{
    QList<HbBinMakerOffsetItem> itemList = offsetMap.items();
    Q_FOREACH(const HbBinMakerOffsetItem &item, itemList) {
        if (item.offsetCSS >= 0) {
            HbCss::StyleSheet *sheet = HbMemoryUtils::getAddress<HbCss::StyleSheet>(
                HbMemoryManager::SharedMemory, item.offsetCSS);
            WidgetMLParseInput file;
            if (collectLayouts(item.name, sheet, file)) {
                parseWidgetML(offsetMap, item.widgetHash, file);
            }
        }
    }
    return true;
}

bool writeCssBinary(const QStringList &inputFiles, const QString &targetFile)
{
    HbOffsetMapBuilder offsetMap;
    if (!(parseCss(inputFiles, offsetMap)
        && parseWidgetML(offsetMap))) {
        return false;
    }
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);

    // Create shared cache to shared memory.
    QByteArray data(offsetMap.result());
    bool success = false;

    if (shared->createSharedCache(data.data(), data.size(), offsetMap.size())) {
        // Defragment the chunk contents before dumping it in a file
        int endOffset = HbCssConverterUtils::defragmentChunk();
        if (verboseOn) testCss();

        VERBOSELN("writing the binary file");
        QFile binFile(targetFile);
        success = binFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        if (success) {
            success = (binFile.write(static_cast<char*>(manager->base()), endOffset) >= 0);
            if (success) {
                VERBOSELN("Wrote target binary file: " << binFile.fileName());
            } else {
                err << "failed to write to target binary file: " << binFile.fileName() << endl;
            }
        } else {
            err << "failed to open target binary file: " << binFile.fileName() << endl;
        }
    } else {
        err << "failed to create shared cache." << endl;
    }
    return success;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    int returnValue = 0;

    if(argc < 3) {
        printHelp();
    } else {
        QStringList inputFiles;
        QString targetFile;
        QStringList args(app.arguments());

        for(int count(0) ; count < args.count(); count++) {
            if (args[count].toLower() == "-i") {
                QString filePath = args.value(count + 1);
                if (!filePath.isEmpty()) {
                    inputFiles.append(QDir::fromNativeSeparators(filePath));
                }
                count++;
            } else if (args[count].toLower() == "-o") {
                targetFile = QDir::fromNativeSeparators(args.value(count+1));
                count++;
            } else if (args[count].toLower() == "-v") {
                verboseOn = true;
            }
        }
        if (targetFile.isEmpty()) {
            err << "target filename needed" << endl << endl;
            printHelp();
            returnValue = 1;
        } else {
            QString parentDir(QFileInfo(targetFile).absolutePath());
            if (QDir::current().mkpath(parentDir)) {
                if (!writeCssBinary(inputFiles, targetFile)) {
                    returnValue = 3;
                }
            } else {
                err << "failed to create path: " << parentDir << endl;
                returnValue = 2;
            }
        }
    }
    return returnValue;
}
