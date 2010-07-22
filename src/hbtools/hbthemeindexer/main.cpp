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

#include <hbiconsource_p.h>
#include <hbthemeindex_p.h>
#include <assert.h>
#include <iostream>
#include <QApplication>
#include <QStringList>
#include <QTextStream>
#include <QFileInfo>
#include <QLibrary>
#include <QString>
#include <QFile>
#include <QMap>
#include <QDir>

#define RESOURCE_LIB_NAME "HbCore"
#define WIN32_DEBUG_SUFFIX "d"
#define MAC_DEBUG_SUFFIX "_debug"


// Global variables
static bool verboseOn = false;
static quint32 version = 1; // Current theme index format version

QList<HbThemeIndexItemData> IndexItems;
QMap<quint32, QString> AddedItems;

// Lists that hold icons to be automatically mirrored or icons that are locked
QStringList MirroredList;
QStringList LockedList;


// ------


void createMirroredList(const QString &fullThemePath)
{
    if (verboseOn) {
        std::cout << "Parsing mirrored list for theme " << fullThemePath.toStdString() << "\n";
    }
    // Find mirrored.txt file
    QString filename = fullThemePath + "/mirrored.txt";
    // Try to read file
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QString line;

        while(!file.atEnd()) {        
            QByteArray dirtyLine = file.readLine();
            line = QString(dirtyLine).trimmed();
            // Skip empty lines and comment lines
            if (line.isEmpty() || line.at(0) == '#') {
                continue; 
            }
            MirroredList.append(line);
        }
    }
}

void createLockedList(const QString &fullThemePath)
{
    if (verboseOn) {
        std::cout << "Parsing locked list for theme " << fullThemePath.toStdString() << "\n";
    }
    // Find locked.txt file
    QString filename = fullThemePath + "/locked.txt";
    // Try to read file
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QString line;

        while(!file.atEnd()) {        
            QByteArray dirtyLine = file.readLine();
            line = QString(dirtyLine).trimmed();
            // Skip empty lines and comment lines
            if (line.isEmpty() || line.at(0) == '#') {
                continue; 
            }
            LockedList.append(line);
        }
    }
}

QSize getDefaultSize(const QString &filename)
{
    HbIconSource source(filename);
    return source.defaultSize().toSize();
}

void appendItem(HbThemeIndexItemData &itemData, const QString &itemName)
{
    bool alreadyExists = false;

    if (AddedItems.contains(itemData.itemNameHash)) {
        // If there already is item with same hash, it means either:
        // - same icon is in both scalable and pixmap folder and then we won't append it again
        // - hashing has failed, it generated same hash for 2 different strings -> TODO: do we care? We could e.g. save hash seed in index...
        if (AddedItems.value(itemData.itemNameHash) == itemName) {
            alreadyExists = true;
        } else {
            // Two different strings have same hash!!!
            std::cout << "ERROR: Two different strings (" << AddedItems.value(itemData.itemNameHash).toStdString() << ", " << itemName.toStdString() << ") have same hash\n";
            alreadyExists = true;
        }
    }

    if (!alreadyExists) {
        IndexItems.append(itemData);
        AddedItems.insert(itemData.itemNameHash, itemName);
    }
    
    if (verboseOn) {
        if (!alreadyExists) {
            std::cout << "----------------------------------------------------------------\n";
            std::cout << "Added item" << IndexItems.count() << "\n";
            std::cout << "Item name: " << itemName.toStdString() << " hash: " << itemData.itemNameHash << "\n";
            std::cout << "Item type: " << itemData.itemType << "\n";
            if (itemData.itemType == HbThemeIndexItemData::SvgItem ||
                itemData.itemType == HbThemeIndexItemData::PngItem ||
                itemData.itemType == HbThemeIndexItemData::MngItem ||
                itemData.itemType == HbThemeIndexItemData::GifItem ||
                itemData.itemType == HbThemeIndexItemData::XpmItem ||
                itemData.itemType == HbThemeIndexItemData::JpgItem ||
                itemData.itemType == HbThemeIndexItemData::NvgItem ||
                itemData.itemType == HbThemeIndexItemData::SvgzItem ||
                itemData.itemType == HbThemeIndexItemData::QpicItem) {

                std::cout << "Default size: width: " << itemData.defaultWidth << " height: " << itemData.defaultHeight << "\n";
            
                if (itemData.mirroredItemType != HbThemeIndexItemData::NotDefined) {
                    std::cout << "Mirrored default size: width:" << itemData.mirroredWidth << " height: " << itemData.mirroredHeight << "\n";
                }

                if (itemData.flags & HbThemeIndexItemData::Mirrorable) {
                    std::cout << "Icon is automatically mirrored\n";
                }
            } else if (itemData.itemType == HbThemeIndexItemData::ColorItem) {
                std::cout << "Color value: " << itemData.colorValue << "\n";
                if (itemData.flags & HbThemeIndexItemData::Reference) {
                    std::cout << "Item is reference\n";
                }
            }
            if (itemData.flags & HbThemeIndexItemData::Locked) {
                std::cout << "Item is locked\n";
            }

            std::cout << "----------------------------------------------------------------\n\n";
        } else { // Item already added in index with some other extension, do not add duplicates
            std::cout << "----------------------------------------------------------------\n";
            std::cout << "WARNING! Skipped already existing item:" << itemName.toStdString() << "\n";
        }
    }
}

HbThemeIndexItemData::Type getItemType(const QString &itemName)
{
    if (itemName.endsWith(".svg")) {
        return HbThemeIndexItemData::SvgItem;
    } else if (itemName.endsWith(".png")) {
        return HbThemeIndexItemData::PngItem;
    } else if (itemName.endsWith(".mng")) {
        return HbThemeIndexItemData::MngItem;
    } else if (itemName.endsWith(".gif")) {
        return HbThemeIndexItemData::GifItem;
    } else if (itemName.endsWith(".xpm")) {
        return HbThemeIndexItemData::XpmItem;
    } else if (itemName.endsWith(".jpg")) {
        return HbThemeIndexItemData::JpgItem;
    } else if (itemName.endsWith(".nvg")) {
        return HbThemeIndexItemData::NvgItem;
    } else if (itemName.endsWith(".svgz")) {
        return HbThemeIndexItemData::SvgzItem;
    } else if (itemName.endsWith(".qpic")) {
        return HbThemeIndexItemData::QpicItem;
    } else if (itemName.endsWith(".fxml")) {
        return HbThemeIndexItemData::FxmlItem;
    } else if (itemName.endsWith(".axml")) {
        return HbThemeIndexItemData::AxmlItem;
    }

    return HbThemeIndexItemData::NotDefined;
}

void processFile(const QFileInfo &info) //, const QString &themename)
{
    QString fullFilename = info.absoluteFilePath();
    QString filename = info.fileName();

    HbThemeIndexItemData itemData;

    // First get correct item type
    itemData.itemType = getItemType(filename);

    switch (itemData.itemType) {
        case HbThemeIndexItemData::SvgItem: // fallback all icon types
        case HbThemeIndexItemData::PngItem:
        case HbThemeIndexItemData::MngItem:
        case HbThemeIndexItemData::GifItem:
        case HbThemeIndexItemData::XpmItem:
        case HbThemeIndexItemData::JpgItem:
        case HbThemeIndexItemData::NvgItem:
        case HbThemeIndexItemData::SvgzItem:
        case HbThemeIndexItemData::QpicItem:
            {
            // Define fileName (remove file extension)
            QString iconname;
            // If we come here, the filename must end with .* (e.g. .svg)
            iconname = filename.left(filename.lastIndexOf('.'));

            itemData.itemNameHash = HbThemeIndex::hash(iconname);

            // Define default size
            QSize defaultSize = getDefaultSize(fullFilename);
            itemData.defaultWidth = static_cast<quint32>(defaultSize.width());
            itemData.defaultHeight = static_cast<quint32>(defaultSize.height());

            QString mirroredFilepath = fullFilename;

            // Define mirrored filename if there is a separate mirrored version of the
            // icon in 'mirrored' folder and in that case get also its default size

            int index1 = mirroredFilepath.lastIndexOf('/');
            int index2 = mirroredFilepath.lastIndexOf('\\');

            int index = index1 < index2 ? index2 : index1;

            if (index>0) {
                mirroredFilepath = mirroredFilepath.left(index);
                mirroredFilepath.append(QString("/mirrored/"));

                QStringList extList;
                extList << ".svg" << ".png" << ".mng" << ".gif" << ".xpm" << ".jpg" << ".nvg" << ".svgz" << ".qpic";

                foreach(QString ext, extList) {
                    QString mirroredFilenameCandidate = mirroredFilepath + iconname + ext;

                    if (QFile::exists(mirroredFilenameCandidate)) {
                        itemData.mirroredItemType = getItemType(mirroredFilenameCandidate);
                        // Define mirrored icon size
                        QSize mirroredSize = getDefaultSize(mirroredFilenameCandidate);
                        itemData.mirroredWidth = static_cast<quint32>(mirroredSize.width());
                        itemData.mirroredHeight = static_cast<quint32>(mirroredSize.height());
                        break;
                    }
                }
            }

            if (MirroredList.contains(iconname)) {
                itemData.flags |= HbThemeIndexItemData::Mirrorable;
                // Remove all found items from the list so that in the end we can handle with missing items.
                MirroredList.removeOne(iconname);
            }

            if (LockedList.contains(iconname)) {
                itemData.flags |= HbThemeIndexItemData::Locked;
                // Remove all found items from the list so that in the end we can handle with missing items.
                LockedList.removeOne(iconname);
            }
            appendItem(itemData, iconname);
            break;
            }

        case HbThemeIndexItemData::AxmlItem: // fallback, these are handled same way
        case HbThemeIndexItemData::FxmlItem:
            {
            // Define fileName (file extension not removed)
            itemData.itemNameHash = HbThemeIndex::hash(filename);

            if (LockedList.contains(filename)) {
                itemData.flags |= HbThemeIndexItemData::Locked;
                // Remove all found items from the list so that in the end we can handle with missing items.
                LockedList.removeOne(filename);
            }
            appendItem(itemData, filename);
            break;
            }

        default:
            {
            // Don't append unknown items to the index.
            break;
            }
    } // end switch
}

bool themeIndexItemDataLessThan(const HbThemeIndexItemData &d1, const HbThemeIndexItemData &d2)
{
    return d1.itemNameHash < d2.itemNameHash;
}

void indexColorVariables(const QString &filename)
{
    QFile file(filename);

    // Index only if given CSS file exists in theme.
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        while(!in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (line.startsWith("qtc_")) {
                HbThemeIndexItemData itemData;

                // Extract name and value from line
                QString name = line.mid(0, line.indexOf(':')).trimmed();
                QString value;
                if (line.at(line.indexOf(':') + 1) == QChar('#')) {
                    // qtc_conv_list_received_normal:#B5B5B5;
                    int startIndex = line.indexOf('#');
                    int endIndex = line.indexOf(';');
                    value = line.mid(startIndex + 1, endIndex - startIndex - 1).trimmed();
                } else if (line.indexOf("var(") >= 0) {
                    // qtc_conv_list_received_normal:var(qtc_conv_list_sent_normal);
                    itemData.flags |= HbThemeIndexItemData::Reference;
                    int startIndex = line.indexOf("var(") + 4;
                    int endIndex = line.indexOf(')');
                    value = line.mid(startIndex, endIndex - startIndex).trimmed();
                }

                itemData.itemNameHash = HbThemeIndex::hash(name);
                itemData.itemType = HbThemeIndexItemData::ColorItem;
                bool ok = false;
                itemData.colorValue = (quint32)value.toUInt(&ok, 16);   // Might cause compiler warning in 64 bit systems
                appendItem(itemData, name);
            }
        }
        file.close();
    } else if (verboseOn) {
        std::cout << "No " << filename.toStdString() << " in theme!\n";
    }
    return;
}

void processDir(const QDir &dir, const QString &themename, const QString targetName, bool subDir = false)
{
    if (!subDir) {
        IndexItems.clear();
        AddedItems.clear();
        MirroredList.clear();
        LockedList.clear();
        createMirroredList(dir.absolutePath()+"/icons/"+themename);
        createLockedList(dir.absolutePath()+"/icons/"+themename);
    }

    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for (int i=0; i<entries.count(); i++) {
        QFileInfo info = entries.at(i);
        QString file = info.absoluteFilePath();
        if (info.isDir()) {
            // Process subdirs recursively
            QDir subDir(file);
            processDir(subDir, themename, targetName, true);
        }
        // Process file
        if (file.contains("/" + themename + "/", Qt::CaseInsensitive)) {
            processFile(info);
        }
    }

    if (!subDir) {
        // There might still be items in mirrored list (e.g. frames are not actual items, but they are still mirrored)
        // So just create empty items for the rest of mirrored items in list
        foreach (QString mirrored, MirroredList) {
            HbThemeIndexItemData itemData;
            itemData.itemNameHash = HbThemeIndex::hash(mirrored);
            itemData.flags |= HbThemeIndexItemData::Mirrorable;
            appendItem(itemData, mirrored);
        }

        // Read application and widget color group CSS files and index their content
        // Temporary check
        if (QFile::exists(dir.absolutePath() + "/style/" + themename + "/variables/color/hbapplicationcolorgroup.css") &&
            QFile::exists(dir.absolutePath() + "/style/" + themename + "/variables/color/hbwidgetcolorgroup.css")) {
            if (verboseOn) {
                std::cout << "Processing hbapplicationcolorgroup.css and hbwidgetcolorgroup.css";
            }
            indexColorVariables(dir.absolutePath() + "/style/" + themename + "/variables/color/hbapplicationcolorgroup.css");
            indexColorVariables(dir.absolutePath() + "/style/" + themename + "/variables/color/hbwidgetcolorgroup.css");
        } else {
            if (verboseOn) {
                std::cout << "Processing hbcolorgroup.css";
            }
            indexColorVariables(dir.absolutePath() + "/style/" + themename + "/variables/color/hbcolorgroup.css");
        }

        QDir targetDir;
        if (!targetDir.exists(targetName)) {
            targetDir.mkpath(targetName);
        }
        QString filename = targetName + themename + ".themeindex";

        QFile::remove(filename);
        QFile indexFile(filename);
        if (!indexFile.open(QIODevice::ReadWrite)) {
            std::cout << "ERROR: could not open index file!\n";
            return;
        }
        
        // Write the header in the beginning of the file
        HbThemeIndexHeaderV1 header;
        header.version = version;
        header.itemCount = IndexItems.count();

        if (verboseOn) {
            std::cout << "============================TOTALS==============================\n";
            std::cout << "Added " << header.itemCount << " items.\n";
            std::cout << "================================================================\n";
        }
 
        // Sort the list
        qStableSort(IndexItems.begin(), IndexItems.end(), themeIndexItemDataLessThan);
         
        // Write header info into the file stream
        qint64 ret = indexFile.write(reinterpret_cast<const char *>(&header), sizeof(HbThemeIndexHeaderV1));
        assert(ret == sizeof(HbThemeIndexHeaderV1));
        
        // Write items into the file stream
        foreach(const HbThemeIndexItemData &itemData, IndexItems) {
            ret = indexFile.write(reinterpret_cast<const char *>(&itemData), sizeof(HbThemeIndexItemData));
            assert(ret == sizeof(HbThemeIndexItemData));
        }
        
        indexFile.close();
    }
}

void showHelp() {
    std::cout << "Themeindexer.exe usage:\n\n";
    std::cout << "hbthemeindexer [-v] -f filename OR (-n themename) -s themes source directory -t theme index file target directory\n\n";

    std::cout << "-n \t\ttheme to be indexed (\"<themename>.themeindex\"). If omitted, all found\n";
    std::cout << "\t\tthemes are indexed.\n";
    std::cout << "-s \t\tthemes source directory is scanned recursively and all the recognized\n";
    std::cout << "\t\tresource files for given theme are aded in the theme index.\n";
    std::cout << "-t \t\ttarget directory for the index file.\n";

    std::cout << "-f <filename>\tfile which contains multiple themes to be indexed. Each in its own row.\n";
    std::cout << "-v \t\tverbose output\n\n";

    std::cout << "Example 1:\n";
    std::cout << "Themeindexer.exe -n mytheme -s c:/mythemes -t c:/mythemes\n";
    std::cout << "Example 2:\n";
    std::cout << "Themeindexer.exe -f c:/mythemes/themes.txt\n\n";
}

void loadHbResource()
{
    bool loadSuccess;
    // To load resources embedded in hb library
    QString resourceLibName(RESOURCE_LIB_NAME);
    QLibrary hbLib(resourceLibName);
    loadSuccess = hbLib.load();
    
    if ( !loadSuccess ) {
        // Library may not be loaded, if it was built in debug mode and the name in debug mode is
        // different, change the name to debug version in that scenario
#ifdef Q_OS_WIN32
        resourceLibName += WIN32_DEBUG_SUFFIX;
#elif defined(Q_OS_MAC)
        resourceLibName += MAC_DEBUG_SUFFIX;
#endif
        // On symbian library name in debug mode is same as that in release mode,
        // so no need to do anything for that
        hbLib.setFileName(resourceLibName);
        loadSuccess = hbLib.load();
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv, false); // GUIenabled=false

    if (argc <= 2) {
        showHelp();
    } else {
        // Load HbCore resource to be able to index hbdefault theme
        loadHbResource();

        QString filename;
        QString themename;
        QDir basedir;
        QString targetname;
        QStringList args(app.arguments());

        for (int n = 0; n < args.count(); n++) {
            if (args[n].toLower() == "-n") {
                themename = args[n+1];
                n++;
            } else if (args[n].toLower() == "-s") {
                basedir = QDir(args[n+1]);
                n++;
            } else if (args[n].toLower() == "-t") {
                targetname = args[n+1];
                n++;
            } else if (args[n].toLower() == "-v") {
                verboseOn = true;
            } else if (args[n].toLower() == "-f") {
                filename = args[n+1];
            }
        }

        if (filename.length() > 0) {
            if (!QFile::exists(filename)) {
                std::cout << "Error: file " << filename.toStdString() << " does not exist.\n";
            } else {
                // Open file and parse lines. Each line should have three value separated with:
                QFile themesToBeIndexed(filename);
                if (themesToBeIndexed.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&themesToBeIndexed);

                    while(!in.atEnd()) {
                        QString line = in.readLine();

                        QStringList values = line.split(' ');
                        if (values.count() == 3) {
                            themename = values[0];
                            basedir = values[1];
                            targetname = values[2];

                            targetname.replace('\\', '/');
                            // Check that targetname has / at the end
                            if (!targetname.endsWith('/')) {
                                targetname.append('/');
                            }
                            processDir(basedir, themename, targetname);
                        }
                    }

                    themesToBeIndexed.close();

                    // Loop through themes string list and call processDir
                } else {
                    std::cout << "Error: file " << filename.toStdString() << " could not be opened.\n";
                }
            }
        } else {
            // Index only given dir

            targetname.replace('\\', '/');
            // Check that targetname has / at the end
            if (!targetname.endsWith('/')) {
                targetname.append('/');
            }

            if (themename.isEmpty()) {
                // Theme name not given, determine available themes
                QDir icondir(basedir);
                if (icondir.cd("icons")) {
                    QStringList entries = icondir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    foreach (const QString &entry, entries) {
                        QDir entrydir(icondir.filePath(entry));
                        if (entrydir.exists("index.theme")) {
                            processDir(basedir, entrydir.dirName(), targetname);
                        }
                    }
                }
            } else {
                // Process only given theme
                processDir(basedir, themename, targetname);
            }

        }
    }

    IndexItems.clear();
    return 0;
}

