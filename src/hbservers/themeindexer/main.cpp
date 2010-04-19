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

#include <hbiconsource_p.h>
#include <hbthemeindex_p.h>
#include <QtGui>
#include <assert.h>
#include <iostream>

#define RESOURCE_LIB_NAME "HbCore"
#define WIN32_DEBUG_SUFFIX "d"
#define MAC_DEBUG_SUFFIX "_debug"

// For being able to sort the index items based on the iconname
class IndexItemInfo
{
public:
    QString iconname;
    HbThemeIndexItem item;
};

bool operator<(const IndexItemInfo &left, const IndexItemInfo &right)
{
    return left.iconname < right.iconname;
}

// Global variables

static int counter = 0;
static bool verboseOn = false;

static int version = 1; // Current theme index format version

QList<IndexItemInfo> IndexItems;

QMap<QString, int> Strings;
QByteArray StringBuffer;

// ------

QSize getDefaultSize(const QString &filename)
{
    HbIconSource source(filename);
    return source.defaultSize().toSize();
}

int getStringOffset(const QString &string)
{
    int offset = Strings.value(string, -1);
    if (offset < 0) {
        // Allocate new string in the string buffer
        offset = StringBuffer.size();
        StringBuffer.append(string.toLatin1());
        StringBuffer.append('\0');
        // Add offset to the target paths list
        Strings.insert(string, offset);
    }
    return offset;
}

void processFile(const QFileInfo &info, const QString &themename)
{    
    QString fullFilename = info.absoluteFilePath();
    QString filename = info.fileName();

    if (filename.endsWith(".svg") ||
        filename.endsWith(".png") ||
        filename.endsWith(".mng") ||
        filename.endsWith(".gif") ||
        filename.endsWith(".xpm") ||
        filename.endsWith(".jpg") ||
        filename.endsWith(".nvg") ||
        filename.endsWith(".svgz") ||
        filename.endsWith(".qpic")) {

        IndexItemInfo itemInfo;

        QString targetPath;

        // If not "hbdefault", which is in resource file, resolve target path for the icon
        if (!fullFilename.startsWith(':') && 
            !fullFilename.contains("icons/hbdefault") &&
            !fullFilename.contains("icons\\hbdefault")) {

            if (fullFilename.contains("scalable")) {
                targetPath = "/resource/hb/themes/icons/" + themename + "/scalable/";
            } else {
                targetPath = "/resource/hb/themes/icons/" + themename + "/pixmap/";
            }
        } else {
            // Resource file target path, used with "hbdefault" theme
            if (fullFilename.contains("scalable")) {
                targetPath = ":/themes/icons/hbdefault/scalable/";
            } else {
                targetPath = ":/themes/icons/hbdefault/pixmap/";
            }
        }

        itemInfo.item.folderOffset = getStringOffset(targetPath);
        itemInfo.item.extOffset = getStringOffset(filename.mid(filename.lastIndexOf('.')));

        // Define iconname (remove file extension)
        QString iconname;

        int extIndex = filename.lastIndexOf('.');
        if (extIndex > 0) {
            iconname = filename.left(extIndex);
        } else {
            iconname = filename;
        }

        itemInfo.item.iconnameOffset = getStringOffset(iconname);
        itemInfo.iconname = iconname;

        // Define default size
        itemInfo.item.defaultSize = getDefaultSize(fullFilename);

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
                    itemInfo.item.mirroredExtOffset = getStringOffset(ext);
                    itemInfo.item.mirroredDefaultSize = getDefaultSize(mirroredFilenameCandidate);
                    break;
                }
            }
        }

        bool alreadyExists = false;

        // Check if there is already an item with the same iconname in the index
        foreach(const IndexItemInfo &info, IndexItems) {
            if (info.iconname == itemInfo.iconname) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            IndexItems.append(itemInfo);

            if (verboseOn) {
            std::cout << "----------------------------------------------------------------\n";
            std::cout << "Added item" << counter << "\n";
            std::cout << "Iconname:" << &StringBuffer.data()[itemInfo.item.iconnameOffset] << "\n";
            std::cout << "Folder:" << &StringBuffer.data()[itemInfo.item.folderOffset] << "\n";
            std::cout << "Extension:" << &StringBuffer.data()[itemInfo.item.extOffset] << "\n";
            std::cout << "Default size: width: " << itemInfo.item.defaultSize.width() << " height: " << itemInfo.item.defaultSize.height() << "\n";
                if (itemInfo.item.mirroredExtOffset >= 0) {
                std::cout << "Mirrored extension:" << &StringBuffer.data()[itemInfo.item.mirroredExtOffset] << "\n";
                } else {
                std::cout << "Mirrored extension: <empty>\n";
                }
            std::cout << "Mirrored default size: width:" << itemInfo.item.mirroredDefaultSize.width() << " height: " << itemInfo.item.mirroredDefaultSize.height() << "\n";
            }
            counter++;
        } else { // Icon already added in index with some other extension, do not add duplicates
            if (verboseOn) {
                std::cout << "----------------------------------------------------------------\n";
                std::cout << "WARNING! Skipped already existing icon:" << fullFilename.toStdString() << "\n";
            }
        }
    }
}

void adjustOffsets() {
    int adjustment = sizeof(HbThemeIndexHeaderV1) + IndexItems.count() * sizeof(HbThemeIndexItem);

    for (int i = 0; i<IndexItems.count(); ++i) {
        IndexItems[i].item.iconnameOffset += adjustment;
        IndexItems[i].item.folderOffset += adjustment;        
        IndexItems[i].item.extOffset += adjustment;

        if (IndexItems[i].item.mirroredExtOffset >= 0) {
            IndexItems[i].item.mirroredExtOffset += adjustment;
        }
    }
}

void processDir(const QDir &dir, const QString &themename, const QString targetName, bool subDir = false)
{
    if (!subDir) {
        IndexItems.clear();
        Strings.clear();
        StringBuffer.clear();
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
        processFile(info, themename);
    }

    if (!subDir) {
        QDir targetDir(targetName);
        if (!targetDir.exists()) {
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
        header.count = IndexItems.count();
        
        qint64 ret = indexFile.write(reinterpret_cast<const char *>(&header), sizeof(HbThemeIndexHeaderV1));
        assert(ret == sizeof(HbThemeIndexHeaderV1));

        // Sort the list
        qSort(IndexItems);

        // Fix offsets in the items to be based on the beginning of the theme index instead of
        // the beginning of the string buffer area.
        adjustOffsets();

        // Write the items in the file stream
        foreach(const IndexItemInfo &itemInfo, IndexItems) {
            ret = indexFile.write(reinterpret_cast<const char *>(&itemInfo.item), sizeof(HbThemeIndexItem));
            assert(ret == sizeof(HbThemeIndexItem));
        }

        // Write the string buffer in the stream
        ret = indexFile.write(StringBuffer.constData(), StringBuffer.size());
        assert(ret == StringBuffer.size());
        indexFile.close();    
    }
}

void showHelp() {
    std::cout << "Themeindexer.exe usage:\n\n";
    std::cout << "themeindexer [-v] -f filename OR -n themename -s theme icons source directory -t theme index file target directory\n\n";

    std::cout << "-n \t\tname of index file (\"<themename>.themeindex\").\n";
    std::cout << "-s \t\ticons source directory is scanned recursively and all the";
    std::cout << "\t\t\trecognized icon files are aded in the theme index.\n";
    std::cout << "-t \t\ttarget directory for the index file.\n";

    std::cout << "-f <filename>\tfile which contains multiple themes to be indexed. Each in its own row.\n";
    std::cout << "-v \t\tverbose output\n\n";

    std::cout << "Example 1:\n";
    std::cout << "Themeindexer.exe -n theme1 -s c:/themes/icons/theme1/ -t c:/temp/\n\n";
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
    QApplication app(argc, argv);

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
            // Index only given theme

            targetname.replace('\\', '/');
            // Check that targetname has / at the end
            if (!targetname.endsWith('/')) {
                targetname.append('/');
            }

            processDir(basedir, themename, targetname);

        }
    }

    return 0;
}

