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

#include "hbstandarddirs_p.h"
#include "hbicontheme_p.h"
#include "hblayeredstyleloader_p.h"
#include "hbthemeutils_p.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/qdebug.h>
#include <QtAlgorithms>
#include <QResource>
#include <sys/stat.h>

// Standard theme root dirs
const char *coreResourcesRootDir = ":";

// Private API
// WARNING: This API is at prototype level and shouldn't be used before
// the resource fetching with theming is fully implemented
class HbStandardDirsInstance
{
public:
    HbStandardDirsInstance();
    ~HbStandardDirsInstance();

    void clearIconDirs();

    // Explicitly add/remove dir in cached dir contents
    void addIconDir( const QString &dir );
    void removeIconDir( const QString &dir );

    // Return icon filename if it exists in file system,
    // otherwise return empty string.
    QString findIcon( const QString &fullFileName );
    QStringList rootPaths() const { return rootPathList; }
private:
    // Root Directory Path sepecific to the Platform
    void constructRootPathList();
    QStringList additionalRootPath();
    int fileSize(QByteArray fileName);
private:
    // Cached contents of icon directories
    QMap<QString, QStringList> iconDirs;
    QStringList rootPathList;
    QStringList extList;
};

// Static instance
static HbStandardDirsInstance instance;

class HbResource : public QResource
{
public:
    HbResource( const QString &dir ):QResource( dir ){}
    QStringList entryList() {
        return children();
    }
};

HbStandardDirsInstance::HbStandardDirsInstance()
{
    constructRootPathList();
#ifdef HB_NVG_CS_ICON
    extList << ".nvg";
#endif
    extList << ".svg" << ".qpic" << ".png" << ".mng" << ".gif" << ".xpm" << ".jpg";
}

HbStandardDirsInstance::~HbStandardDirsInstance()
{
}

void HbStandardDirsInstance::clearIconDirs()
{
    iconDirs.clear();
}

void HbStandardDirsInstance::addIconDir( const QString &dir )
{
    QStringList list;
    if (dir.startsWith(QLatin1Char( ':' ))) {
        // try to find all files assuming its a resource directory
        HbResource resource(dir);
        if (resource.isValid()) {
            list = resource.entryList();
        }
    }
    else {

        QDir directory(dir);
        // File list is sorted by filename so binary search used in findIcon function works.
        // The sorting of QDirSortItems is slow, so we sort the resulting strings instead:
        // QStringList list = directory.entryList(QDir::Files, QDir::Name | QDir::IgnoreCase);
        list = directory.entryList(QDir::Files, QDir::Unsorted);
    }
    list.sort();

    iconDirs.insert( dir, list );
}

void HbStandardDirsInstance::removeIconDir( const QString &dir )
{
    iconDirs.remove(dir);
}

QString HbStandardDirsInstance::findIcon( const QString &fullFileName )
{
    // Enable this for profiling memory consumption.
    // It counts how many characters are stored in the directory contents cache.
#if 0
    int charSize = 0;

    for (QMap<QString, QStringList>::const_iterator z = iconDirs.constBegin(); z != iconDirs.constEnd(); ++z) {
        QStringList list = z.value();
        charSize += iconDirs.key(list).length();

        for (int zz=0; zz<list.count(); ++zz) {
            charSize += list[zz].length();
        }
    }
#endif
    
    // Store path
    QString fullFileNameLocal = fullFileName;

    // Find the directory part from the full filename
    int index1 = fullFileNameLocal.lastIndexOf('/');
    int index2 = fullFileNameLocal.lastIndexOf('\\');
    
    int index = index1 > index2 ? index1 : index2;
    if (index > 0) {
        QString dir = fullFileNameLocal.left(index);
        
        // Try to fetch the dir from the cached dir contents
        QMap<QString, QStringList>::const_iterator i = iconDirs.find(dir);
        // Dir not found, add it
        if (i == iconDirs.constEnd()) {
            QDir LocalDir(dir) ;
            if(LocalDir.exists()) {
                addIconDir(dir);
                i = iconDirs.find(dir);
            }
        }
        if(i != iconDirs.constEnd()) {
            QStringList list = i.value();
            // Search for the file from the cached dir contents
            if (!list.isEmpty()) {
                QString file = fullFileNameLocal.right(fullFileNameLocal.length() - index - 1);
                // Remove extension
                int extensionIndex = file.lastIndexOf('.');
                int extensionLength = 0;
                bool searchAllIconExtensions = true;
                if (extensionIndex > 0) {
                        searchAllIconExtensions = false;
                }
                // If no extension was given or given extension is one of the supported ones, search
                // for icons with all the supported icon file extensions
                if (searchAllIconExtensions) {
                    foreach (const QString &extension, extList) {
                        if (qBinaryFind(list, file + extension) != list.constEnd()) {
                            // Icon found, return its full path with the correct extension
                            if (extensionLength) {
                                return fullFileName.left(fullFileName.length() - extensionLength) + extension;
                            }
                            else {
                                return fullFileName + extension;
                            }
                        }
                    }
                } else {
                    // Some other extension was given (e.g. ".theme"), search only for that file.
                    if (qBinaryFind(list, file) != list.constEnd()) {
                        return fullFileName;
                    }
                }
            }
        }
    } 
    return QString();
}

void HbStandardDirsInstance::constructRootPathList()
{
#if defined(Q_OS_SYMBIAN)
    rootPathList << QLatin1String("z:/resource/hb")
                 << QLatin1String("c:/resource/hb")
                 << QLatin1String("e:/resource/hb")
                 << QLatin1String("f:/resource/hb");
#else
    const QString &mainThemesDir = HbStandardDirs::themesDir();
    if (!mainThemesDir.isEmpty()) {
        rootPathList << mainThemesDir;
    }
#endif
#if defined(Q_OS_MAC)
    rootPathList << QDir::homePath() + QLatin1String("/Library/UI Extensions for Mobile");
#endif

    // Add core resource dir as well
    rootPathList << coreResourcesRootDir;
}

int HbStandardDirsInstance::fileSize( QByteArray fileName ) 
{ 
    struct stat fileStat; 
    const char *fname = fileName.data();
    int err = stat( fname, &fileStat ); 
    if (0 != err) return 0; 
    return fileStat.st_size; 
}

QStringList HbStandardDirsInstance::additionalRootPath()
{
    QFile rootPathFile(rootPathsFile);
    static QStringList rootpaths;
    
    QByteArray filePath = rootPathsFile.toLatin1().constData();
    int size = fileSize(filePath);
    
    if(size > 0) {
        if (rootPathFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&rootPathFile);
            rootpaths.clear();
            while (!in.atEnd()) {
                QString line = in.readLine();
                QDir rootdir(line);
                if (rootdir.exists()) {
                    rootpaths.append(line);
                }                
            }        
        }
    }
    return rootpaths;
}

/*
  typeOfResource 
  1. "icons"
  only one supported right now

// Important: This function should just return a directory if the resource was found. Otherwise
//            return empty string! Calling functions will trust it!!     

*/
QString HbStandardDirs::findResource(const QString &name, Hb::ResourceType resType)
{
    if (resType == Hb::IconResource || 
        resType == Hb::ThemeResource || 
        resType == Hb::EffectResource)
    {
        QString absolutePath;

        QStringList rootDirs;

        if (QDir::isRelativePath(name)) {
#ifdef HB_TOOL_INTERFACE
            // Additional root directory support Currently used by tools only.
            // This may not be needed if themes tool start using HB_THEMES_DIR to set their
            // root dir
            rootDirs << instance.additionalRootPath();
#endif
            rootDirs << instance.rootPaths();
        } else {
            // Given filename has an absolute path, use that.
            absolutePath = name;
        }
        if (resType == Hb::IconResource ) {
            // Relative path was given, search in the standard icon folders
            if (absolutePath.isEmpty()) {
                foreach ( const QString &prefix, rootDirs) {
                    absolutePath =  prefix + '/' + name;
                    // Check file existence from instance, it caches directory contents 
                    // to speed up the lookup
                    QString ret = instance.findIcon(absolutePath);
                    if (!ret.isEmpty()) {
                        return ret;
                    }
                }
            } else {
                // Absolute path was given, only search in that folder
                return instance.findIcon(absolutePath);
            }
        }
        else if (resType == Hb::ThemeResource) {
            QFile file;
            bool fileExists = false;
            foreach ( const QString &prefix, rootDirs ) {
                if (absolutePath.isEmpty()) {
                    absolutePath = prefix + '/' + name;
                }
                // Check for the availability of the file, as QFile::Exists takes more
                // time this method is used
                file.setFileName(absolutePath);
                fileExists = file.open(QIODevice::ReadOnly);
                file.close();
                // E.g. in hardware absolutepath is not found...
                // ToDo: mirrored.txt will be refactored
                //if(name.endsWith("mirrored.txt") && !fileExists) {
                //    return QString(":/themes/icons/hbdefault/mirrored.txt");       
                //} else {
                    return absolutePath;
                //}
            }
        }
        else if (resType == Hb::EffectResource) {
            QFile file;
            bool fileExists = false;
            foreach ( const QString &prefix, rootDirs ) {
                if (absolutePath.isEmpty()) {
                    absolutePath = prefix + '/' + name;
                }
                // Check for the availability of the file, as QFile::Exists takes more
                // time this method is used
                file.setFileName(absolutePath);
                fileExists = file.open(QIODevice::ReadOnly);
                file.close();
                if( fileExists ) {
                    return absolutePath;
                }
            }
        }
    }
    return QString();
}

/* @param pathList. List of paths, relative or absolute. Modified to absolute paths.
 * @param typeOfResource Type of Resource, can be Hb::StyleSheetResource or Hb::EffectResource.
 * 
 */
void HbStandardDirs::findResourceList(QMap<int, QString> &pathList,
                                      Hb::ResourceType resType, bool assumeAbsolutesExists)
{
    QString absolutePath;
    QString path;
    if (resType == Hb::StyleSheetResource || resType == Hb::EffectResource) {
        QStringList rootDirs;
        QMutableMapIterator<int, QString> i(pathList);
        rootDirs
#ifdef HB_TOOL_INTERFACE
        // Additional root directory support Currently used by tools only.
        // This may not be needed if themes tool start using HB_THEMES_DIR to set their
        // root dir
        << instance.additionalRootPath()
#endif
        << instance.rootPaths();
        QFile file;
        bool fileExists = false;
        while (i.hasNext()) {
            i.next();
            if (QDir::isAbsolutePath(i.value())) {
                fileExists = (assumeAbsolutesExists) ? true : QFile::exists(i.value());
            } else {
                fileExists = false;
            foreach ( const QString &prefix, rootDirs ) {
                    absolutePath =  prefix + '/' + i.value();
                    // Check for the availability of the file, as QFile::Exists takes more
                    // time this method is used
                    file.setFileName(absolutePath);
                    fileExists = file.open(QIODevice::ReadOnly);
                    file.close();
                    if (fileExists) {
                        i.setValue(absolutePath);
                        break;
                    }
             }
            }
            if (!fileExists) {
                i.remove();
            }
        }
    }
}


QStringList HbStandardDirs::findExistingFolderList(const QStringList &relativeFolderPaths, 
                                                   const QString &currentThemeName,
                                                   Hb::ResourceType resType)
{
    QString absolutePath;
    QStringList existingPaths;

    QStringList rootDirs;
    rootDirs
#ifdef HB_TOOL_INTERFACE
    // Additional root directory support Currently used by tools only.
    // This may not be needed if themes tool start using HB_THEMES_DIR to set their
    // root dir
    << instance.additionalRootPath()
#endif
    << instance.rootPaths();

    foreach (const QString &path, relativeFolderPaths) {
        if (QDir::isAbsolutePath(path)) {
            if(QFile::exists(path)) {
                existingPaths.append(path);
            }
        } else {
            foreach( const QString &prefix, rootDirs) {
                absolutePath =  prefix + '/' + path;
                // Check for the availability of the file
                if( QFile::exists(absolutePath) ) {
                    existingPaths.append(absolutePath);
                    // Assuming each path will be there only in one root directory
                    // (not supporting a single theme scattered in multiple root dirs)
                    break;
                }
            }
        }
    }
    // Appending base theme folder
    const HbThemeInfo &themeInfo = HbThemeUtils::baseTheme();
    if (themeInfo.name != currentThemeName && resType == Hb::EffectResource) {
        existingPaths.append(themeInfo.rootDir + '/' + HbThemeUtils::platformHierarchy + '/' + HbThemeUtils::effectsResourceFolder + '/' + themeInfo.name + '/');
    }

    return existingPaths;
}

const QString &HbStandardDirs::themesDir()
{
#ifdef Q_OS_SYMBIAN
    static QString mainThemesDir("Z:\\resource\\hb\\");
#else
    static QString mainThemesDir = QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR"));
#endif
    return mainThemesDir;
}

