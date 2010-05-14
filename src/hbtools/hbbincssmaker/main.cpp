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

// Global variables
QString AppName("hbbincssmaker");
static bool verboseOn = false;
QTextStream out(stdout);
QTextStream verboseOut(stderr);
QTextStream err(stderr);

#define VERBOSE(statement) if(verboseOn) { verboseOut << statement; }
#define VERBOSEIF(test, statement) if(verboseOn && test) { verboseOut << statement; }
#define VERBOSELN(statement) if(verboseOn) { verboseOut << statement << endl; }


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

class CssMap
{
public:
    CssMap() {}
    void add(const QString &cssName, unsigned int offset)
    {
        HbOffsetItem mapItem = _mapItems.value(cssName, HbOffsetItem());
        if (mapItem.nameOffset < 0) {
            mapItem.nameOffset = _cssNameBuffer.size();
            mapItem.offset = offset;
            _cssNameBuffer.append(cssName.toLatin1()).append('\0');
            _mapItems.insert(cssName, mapItem);
        } else {
            err << "warning: duplicate cache key for " << cssName << endl;
        }
    }

    void registerOffsetHolders() {
        foreach(const HbOffsetItem &mapItem, _mapItems) {
            HbCssConverterUtils::registerOffsetHolder(const_cast<int *>(&mapItem.offset));
        }
    }

    QByteArray data() const {
        QByteArray dataArray;
        int count = _mapItems.size();
        int adjustment = count * sizeof(HbOffsetItem);
        foreach(const HbOffsetItem &mapItem, _mapItems) {
            HbOffsetItem tmp(mapItem);
            // Fix offsets in the items to be based on the beginning of the css map instead of
            // the beginning of the css name buffer.
            tmp.nameOffset += adjustment;
            dataArray.append(reinterpret_cast<const char*>(&tmp), sizeof(HbOffsetItem));
        }
        dataArray.append(_cssNameBuffer);
        return dataArray;
    }
    int size() const { return _mapItems.count(); }

private:
    QMap<QString, HbOffsetItem> _mapItems;
    QByteArray _cssNameBuffer;
};

struct InputFile
{
    InputFile(const QString &cacheName, const QFileInfo &file) : cacheName(cacheName), file(file)
    {
    }

    QString cacheName;
    QFileInfo file;
};
typedef QList<InputFile> InputFileList;

struct InputFileInfo
{
    QString base;
    QString path;
    QString prefix;
};
typedef QList<InputFileInfo> InputFileInfoList;

bool operator < (const InputFile & if1, const InputFile & if2)
{
    return if1.file.size() > if2.file.size();
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

InputFileList collectCssFiles(const QStringList &inputFilePaths)
{
    QStringList filters;
    filters << "*.css";

    InputFileList inputFiles;
    QFileInfoList inputPath;
    inputPath.append(QFileInfo());
    Q_FOREACH(const QString &path, inputFilePaths) {
        inputPath[0].setFile(path);
        QFileInfoList allFiles = collectFiles(inputPath, filters);
        Q_FOREACH(const QFileInfo &info, allFiles) {
            inputFiles.append(InputFile(info.fileName(), info));
        }
    }
    return inputFiles;
}

bool writeCssBinary(const QStringList &inputFiles, const QString &targetFile)
{
    if (inputFiles.isEmpty()) return false;
    InputFileList cssFiles = collectCssFiles(inputFiles);
    if (cssFiles.isEmpty()) return false;
    qSort(cssFiles);

    HbCss::Parser parser;
    HbCss::StyleSheet *styleSheet = 0;
    bool success = false;
    CssMap cssMap;
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
    Q_FOREACH(const InputFile &inputFile, cssFiles) {
        const QFileInfo &file = inputFile.file;
        VERBOSE("processing " << file.absoluteFilePath() << "...");
        success = false;
        int offset = manager->alloc(sizeof(HbCss::StyleSheet));
        if (offset >= 0) {
            styleSheet = new (static_cast<char*>(manager->base()) + offset)
                         HbCss::StyleSheet(HbMemoryManager::SharedMemory);
            parser.init(file.absoluteFilePath(), true);
            success = parser.parse(styleSheet);
            cssMap.add(inputFile.cacheName, offset);
            VERBOSE("cache key = " << inputFile.cacheName << "...");
        }
        if (success) {
            VERBOSELN("ok");
        } else {
            VERBOSELN("failed");
            err << "Failed to parse: " << file.absoluteFilePath() << endl;
            break;
        }
    }
    if (success) {
        HbSharedMemoryManager *shared = static_cast<HbSharedMemoryManager*>(manager);

        // Create shared cache to shared memory.
        QByteArray data(cssMap.data());

        if (shared->createSharedCache(data.data(), data.size(), cssMap.size())) {

            // Defragment the chunk contents before dumping it in a file
            int endOffset = HbCssConverterUtils::defragmentChunk();

            if (verboseOn) {
                HbSharedCache *cache = shared->cache();
        
                for (int k=0; k<cache->mOffsetItemCount; ++k) {
                    HbCss::StyleSheet *sheet = HbMemoryUtils::getAddress<HbCss::StyleSheet>(
                        HbMemoryManager::SharedMemory, cache->mOffsetItems[k].offset);
                
                    QString name(QLatin1String( ((char*)(cache->mOffsetItems)) + cache->mOffsetItems[k].nameOffset));

                    VERBOSE("Cssmap item ");
                    VERBOSE(k);
                    VERBOSE("- name: \"");
                    VERBOSE(name);
                    VERBOSELN("\"");

                    // Tests the stylesheet offsets and prints info to verbose out
                    testStyleSheet(sheet);
                }
            }

            VERBOSELN("writing the binary file");
            QFile binFile(targetFile);
            if (!binFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                err << "failed to open target binary file: " << binFile.fileName() << endl;
                return false;
            }
            //int size = shared->size();
            if (binFile.write(static_cast<char*>(manager->base()), endOffset) >= 0) {
                VERBOSELN("Wrote target binary file: " << binFile.fileName());
            } else {
                err << "failed to write to target binary file: " << binFile.fileName() << endl;
            }
        } else {
            err << "failed to create shared cache." << endl;
        }
    }
    return success;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

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
        } else {
            QString parentDir(QFileInfo(targetFile).absolutePath());
            if (QDir::current().mkpath(parentDir)) {
                writeCssBinary(inputFiles, targetFile);
            } else {
                err << "failed to create path: " << parentDir << endl;
            }
        }
    }
    return 0;
}

