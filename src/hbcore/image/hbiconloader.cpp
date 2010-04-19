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

#include <hbiconloader_p.h>

#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QPicture>
#include <QPainter>
#include <QStyleOption>
#include <QApplication> //krazy:exclude=qclasses
#include <QtAlgorithms>
#include <QTime>
#include <QSvgRenderer>
#include <QImageReader>
#include <QHash>

#include "hbicontheme_p.h"
#include "hbstandarddirs_p.h"
#include "hblayoutdirectionnotifier_p.h"
#include "hbinstance.h"
#include "hbiconanimation_p.h"
#include "hbiconanimator.h"
#include "hbiconanimator_p.h"
#include "hbtheme.h"
#include "hbtheme_p.h"
#include "hbthemeclient_p.h"
#include "hbthemeutils_p.h"
#include "hbiconanimationmanager.h"
#include "hbiconanimationdefinition.h"
#include "hbimagetraces_p.h"
#include "hbmemoryutils_p.h"
#include "hbpixmapiconimpl_p.h"
#include "hbiconsource_p.h"
#include "hbthemeindex_p.h"
#include "hbthemecommon_p.h"
#include "hbiconimplcreator_p.h"

#define HB_ICONIMPL_CACHE
/*
 * Client side caching of sgimage icon required,
 * as  sgimage lite cannot be opened multiple times
 */
#ifdef HB_SGIMAGE_ICON
#ifndef HB_ICONIMPL_CACHE
#define HB_ICONIMPL_CACHE
#endif
#endif

// Just an experimental implementation.. Disable if necessary.
#define ENABLE_EXPERIMENTAL_RESIZE_BOOST__

// SVG animation is currently disabled because of bugs in QT's svg engine
#undef HB_SVG_ANIMATION

static const char *s_unknown = "unknown";
    // Icon name without extension

/*!
    \class HbIconLoader

    \brief HbIconLoader loads icons according to the Freedesktop Icon Theme Specification

    \internal
*/

// Allocated dynamically so it can be deleted before the application object is destroyed.
// Deleting it later causes segmentation fault.
static HbIconLoader *theLoader = 0;

#ifdef HB_ICONIMPL_CACHE
static QHash<QByteArray, HbIconImpl*> iconImplCache;
#endif

class HbIconLoaderPrivate
{
public:
    HbIconLoaderPrivate();
    ~HbIconLoaderPrivate();

    static HbIconLoaderPrivate *global();

    static QString removeIconNameSuffix(const QString &iconName);

    /* This method is supposed to work the same way 
       as the FindIconHelper in the Icon Theme Spec: */
    static QString findIconHelper(const QString &iconName,
                                  bool mirrored,
                                  bool& mirroredIconFound,
                                  bool useThemeIndex = true);

    static QString findEffectHelper(const QString &effectName);

    bool isAutomaticallyMirrored(const QString &iconName);

    bool isLayoutMirrored();
    void setLayoutMirrored(bool mirrored);

#ifdef HB_ICONIMPL_CACHE
    QByteArray createCacheKeyFrom( const QString &iconName, 
                                   const QSizeF &size,
                                   Qt::AspectRatioMode aspectRatioMode, 
                                   QIcon::Mode mode, 
                                   bool mirrored, 
                                   const QColor &color);
#endif

private:
    void createMirroredList();

public: // data
    QString storedTheme;

    /*
    * List of icons that should be mirrored automatically in a mirrored layout.
    * This is initially read from a config file "mirrored.txt".
    */
    QStringList mirroredList;
    bool mirroredListCreated;
        
    int sourceResolution;
    int resolution;
    qreal zoom;

    // Frame-by-frame animation support -------------------------------------->
    HbIconAnimationManager *animationManager;
    // Flag to prevent animation frames from loading new animations recursively
    bool animationLoading;
    // <-- Frame-by-frame animation support ------------------------------------

    HbIconSource *lastIconSource;

private: // data
    enum {
        Unknown = 0,
        NotMirrored = 1,
        Mirrored = 2
    };
    /*
    * Information whether the layout is mirrored or not.
    * Flipped icons are used in the mirrored layout.
    */
    int layoutMirrored;
};

HbIconLoaderPrivate::HbIconLoaderPrivate() :
        storedTheme(HbIconTheme::global()->currentTheme()),
        mirroredListCreated(false),
        sourceResolution(144), // This is about the resolution of a Nokia N95 8GB
        resolution(144),
        zoom(1.0),
        animationManager(HbIconAnimationManager::global()),
        animationLoading(false),
        lastIconSource(0),
        layoutMirrored(Unknown)
{
}

HbIconLoaderPrivate::~HbIconLoaderPrivate()
{
    delete lastIconSource;
}

HbIconLoaderPrivate *HbIconLoaderPrivate::global()
{
    HbIconLoader *loader = HbIconLoader::global();
    return loader->d;
}

QString HbIconLoaderPrivate::removeIconNameSuffix(const QString &iconName)
{
    QString loweredIconName = iconName.toLower();
    if (loweredIconName.endsWith(".svg")
        || loweredIconName.endsWith(".png")
        || loweredIconName.endsWith(".mng")
        || loweredIconName.endsWith(".gif")
        || loweredIconName.endsWith(".xpm")
        || loweredIconName.endsWith(".jpg")
        || loweredIconName.endsWith(".nvg")) {
        return iconName.left(iconName.length() - 4);
    }

    if (loweredIconName.endsWith(".svgz")
        || loweredIconName.endsWith(".qpic")) {
        return iconName.left(iconName.length() - 5);
    }

    return iconName;
}

QString HbIconLoader::formatFromPath(const QString &iconPath)
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

QString HbIconLoaderPrivate::findIconHelper(const QString &iconName, bool mirrored, bool& mirroredIconFound, bool useThemeIndex)
{
    Q_UNUSED(useThemeIndex)

    mirroredIconFound = false;
    QString iconPath;

    if (QDir::isRelativePath(iconName)) {
        bool themeIndexUsed = false;

#ifdef Q_OS_SYMBIAN
        // Try to get themed icon information from theme index
        ThemeIndexTables &themeIndex = hbInstance->theme()->d_ptr->themeIndex;
        // Check if there is a valid theme index
        if (useThemeIndex && themeIndex.isValid()) {
            themeIndexUsed = true;
        
            int tableIndex;
            const HbThemeIndexItem *item = themeIndex.getItem(iconName, tableIndex);
        
            if (item) {
                const char *baseAddress = reinterpret_cast<const char *>(themeIndex.tables[tableIndex]);

                QString fullfilename;
            
                // Add drive letter of the used theme if the path does not point to a QT resource
                if (*(baseAddress + item->folderOffset) != ':') {
                    fullfilename.append(themeIndex.drives[tableIndex]);
                    fullfilename.append(':');
                }
                // Add folder
                fullfilename.append(baseAddress + item->folderOffset);

                if (mirrored) {
                    if (item->mirroredExtOffset >= 0) {
                        mirroredIconFound = true;
                                        
                        fullfilename.append("mirrored\\");
                        fullfilename.append(iconName);
                        fullfilename.append(baseAddress + item->mirroredExtOffset);

                        #ifdef THEME_INDEX_TRACES
                        qDebug() << "ThemeIndex: Resolved icon filename:" << fullfilename.toUtf8();
                        #endif

                        return fullfilename;
                    } 
                }
            
                fullfilename.append(iconName);
                fullfilename.append(baseAddress + item->extOffset);

                #ifdef THEME_INDEX_TRACES
                qDebug() << "ThemeIndex: Resolved icon filename:" << fullfilename.toUtf8();
                #endif

                return fullfilename;
            }
        }
#endif // Q_OS_SYMBIAN

        // If there was no theme index, search the icon in theme icon dirs (slow)
        if (!themeIndexUsed) {
        foreach( const QString &dir, HbIconTheme::global()->dirList() ) {
                if (mirrored) {
                    // If icon is mirrored, try to find the icon in a separate "mirrored" folder used for mirrored icons
                    iconPath =  HbStandardDirs::findResource( dir + "mirrored" + '/' + iconName, Hb::IconResource );
                    if( !iconPath.isEmpty() ) {
                        mirroredIconFound = true;
                        break;
                    }
                }

            iconPath =  HbStandardDirs::findResource( dir + iconName, Hb::IconResource );

                // If the file was found in this dir, return the filename.
            if ( !iconPath.isEmpty() ) {
                    break;
                }
            }
        }
    } else {
        // Absolute path. Do not scan through different theme directories.
        QString iconNameCopy(iconName);
        if (mirrored) {
            // If icon is mirrored, try to find the icon in a separate "mirrored" folder used for mirrored icons

            // Find the directory part from the full filename
            int index1 = iconName.lastIndexOf('/');
            int index2 = iconName.lastIndexOf('\\');
    
            int index = index1 > index2 ? index1 : index2;

            QString iconNameCopy(iconName);

            if (index > 0) {
                iconNameCopy.insert(index+1, QString("mirrored\\"));
            }
            iconPath = HbStandardDirs::findResource(iconNameCopy, Hb::IconResource);

            if( !iconPath.isEmpty() ) {
                mirroredIconFound = true;
                return iconPath;
            }
        }
    
        iconPath = HbStandardDirs::findResource(iconName, Hb::IconResource);
    }

    return iconPath;
/*
From Freedesktop.org:

 The exact algorithm (in pseudocode) for looking up an icon in a theme (if the implementation supports SVG) is:

FindIcon(icon, size) {
  filename = FindIconHelper(icon, size, user selected theme);
  if filename != none
    return filename
  return LookupFallbackIcon(icon)
}
FindIconHelper(icon, size, theme) {
  filename = LookupIcon(icon, size, theme)
  if filename != none
    return filename

  if theme has parents
    parents = theme.parents
  else if theme != hicolor
    parents = [hicolor]

  for parent in parents {
    filename = FindIconHelper(icon, size, parent)
    if filename != none
      return filename
  }
  return none
}
     

With the following helper functions:

LookupIcon(iconname, size, theme) {
  for each subdir in $(theme subdir list) {
    for each directory in $(basename list) {
      for extension in ("png", "svg", "xpm") {
        if DirectoryMatchesSize(subdir, size) {
          filename = directory/$(themename)/subdir/iconname.extension
          if exist filename
        return filename
        }
      }
    }
  }
  minimal_size = MAXINT
  for each subdir in $(theme subdir list) {
    for each directory in $(basename list) {
      for extension in ("png", "svg", "xpm") {
        filename = directory/$(themename)/subdir/iconname.extension
        if exist filename and DirectorySizeDistance(subdir, size) < minimal_size {
       closest_filename = filename
       minimal_size = DirectorySizeDistance(subdir, size)
        }
      }
    }
  }
  if closest_filename set
     return closest_filename
  return none
}

LookupFallbackIcon(iconname) {
  for each directory in $(basename list) {
    for extension in ("png", "svg", "xpm") {
      if exists directory/iconname.extension
        return directory/iconname.extension
    }
  }
  return none
}

DirectoryMatchesSize(subdir, iconsize) {
  read Type and size data from subdir
  if Type is Fixed
    return Size == iconsize
  if Type is Scaled
    return MinSize <= iconsize <= MaxSize
  if Type is Threshold
    return Size - Threshold <= iconsize <= Size + Threshold
}

DirectorySizeDistance(subdir, size) {
  read Type and size data from subdir
  if Type is Fixed
    return abs(Size - iconsize)
  if Type is Scaled
    if iconsize < MinSize
        return MinSize - iconsize
    if iconsize > MaxSize
        return iconsize - MaxSize
    return 0
  if Type is Threshold
    if iconsize < Size - Threshold
        return MinSize - iconsize
    if iconsize > Size + Threshold
        return iconsize - MaxSize
    return 0
}

In some cases you don't always want to fall back to an icon in an inherited theme. For instance, sometimes you look for a set of icons, prefering any of them before using an icon from an inherited theme. To support such operations implementations can contain a function that finds the first of a list of icon names in the inheritance hierarchy. I.E. It would look something like this:

FindBestIcon(iconList, size) {
  filename = FindBestIconHelper(iconList, size, user selected theme);
  if filename != none
    return filename
  for icon in iconList {
    filename = LookupFallbackIcon(icon)
    if filename != none
      return filename
  }
  return none;
}
FindBestIconHelper(iconList, size, theme) {
  for icon in iconList {
    filename = LookupIcon(icon, size, theme)
    if filename != none
      return filename
  }

  if theme has parents
    parents = theme.parents
  else if theme != hicolor
    parents = [hicolor]

  for parent in parents {
    filename = FindBestIconHelper(iconList, size, parent)
    if filename != none
      return filename
  }
  return none

    }
*/
}

bool HbIconLoaderPrivate::isAutomaticallyMirrored(const QString &iconName)
{
    if (!mirroredListCreated) {
        // Read the list of the mirrored icons from a file.
        createMirroredList();
        mirroredListCreated = true;
    }

    // Check if the given icon name is found in the list of mirrored icons
    QList<QString>::const_iterator j = qBinaryFind(mirroredList, iconName);
    return (j != mirroredList.constEnd());
}

bool HbIconLoaderPrivate::isLayoutMirrored()
{
    if (layoutMirrored == Unknown) {
        // The layout directionality is defined by asking it from the main window.
        QList<HbMainWindow*> allWindows = hbInstance->allMainWindows();
        HbMainWindow* primaryWindow = allWindows.value(0);
        if (primaryWindow) {
            layoutMirrored = primaryWindow->layoutDirection() == Qt::LeftToRight ? NotMirrored : Mirrored;
        } else {
            // Do not know mirroring state yet, return not mirrored.
            return NotMirrored;
        }
    }
    return layoutMirrored == Mirrored;
}

void HbIconLoaderPrivate::setLayoutMirrored(bool mirrored)
{
    layoutMirrored = mirrored ? Mirrored : NotMirrored;
}

void HbIconLoaderPrivate::createMirroredList()
{
    // Find mirrored.txt file
    QString filename = HbStandardDirs::findResource("themes/themes/mirrored.txt", Hb::ThemeResource);
    if (filename.endsWith("mirrored.txt")) {
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
                mirroredList.append(line);
            }

        // Sort the list
        qSort(mirroredList.begin(), mirroredList.end());
        }
    }
}

#ifdef HB_ICONIMPL_CACHE
QByteArray HbIconLoaderPrivate::createCacheKeyFrom(const QString &iconName, const QSizeF &size,
    Qt::AspectRatioMode aspectRatioMode, QIcon::Mode mode, bool mirrored, const QColor &color)
{
    static const int paramArraySize = 7;

    // This uses QByteArray to improve performance compared to QString.
    // It allows appending stuff with less heap allocations and conversions compared to using QString.
    QByteArray cacheKey;
    int nameSize = iconName.size();
    // Preallocate memory for the array so appending stuff in it does not cause new memory allocations
    cacheKey.reserve(sizeof(int)*paramArraySize + nameSize * sizeof(QChar) + 1);

    int temp[paramArraySize];
    // Store size of icon name first because its often different with different icons, so comparing
    // different cacheKeys is fast.
    temp[0] = nameSize;
    temp[1] = static_cast<int>(size.width());
    temp[2] = static_cast<int>(size.height());
    temp[3] = aspectRatioMode;
    temp[4] = mode;

    if (color.isValid()) {
        temp[5] = 1; // flag, color is valid
        temp[6] = color.rgba();
    } else {
        temp[5] = 0; // flag, color is invalid
        temp[6] = 0;
    }

    cacheKey.append((char*)&(temp[0]), sizeof(int)*paramArraySize);

    const QChar *iconNamePtr = iconName.constData();
    cacheKey.append((char*)iconNamePtr, nameSize * sizeof(QChar));

    if (mirrored) {
        cacheKey.append('M');
    }

    return cacheKey;
}
#endif

HbIconLoader::HbIconLoader(const QString &appName, QObject *parent)
    : QObject( parent )
{
    setObjectName( appName );
    d = new HbIconLoaderPrivate();

    // Delete the icon loader when the application is destroyed.
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(destroy()));

    connect(HbLayoutDirectionNotifier::instance(), SIGNAL(layoutDirectionChangeStarted()),
        this, SLOT(updateLayoutDirection()));
}

HbIconLoader::~HbIconLoader()
{
    delete d;
}

HbIconLoader *HbIconLoader::global()
{
    // Allocated dynamically so it can be deleted before the application object is destroyed.
    // Deleting it later causes segmentation fault.
    if (!theLoader) {
        theLoader = new HbIconLoader;
    }

    return theLoader;
}

QSizeF HbIconLoader::defaultSize(const QString &iconName, const QString &appName, IconLoaderOptions options)
{
    Q_UNUSED(appName)
    QSizeF size;

    // Populate parameters needed for getting the default size
    HbIconLoadingParams params;
    params.iconName = iconName;
    params.mirrored = options.testFlag(HorizontallyMirrored);
    params.mirroredIconFound = false;
    params.animationCreated = false;
    resolveCleanIconName(params);

    // Step 1: Check if the icon has animation definition

    // This check is to prevent animation frames from trying to load new animations
    if (!d->animationLoading) {
        // Check whether there is a frame-by-frame animation defined for the icon
        HbIconAnimationDefinition def = d->animationManager->getDefinition(params.cleanIconName);
        if (!def.isNull()) {
            d->animationLoading = true;
            // Load the animation
            size = getAnimationDefaultSize(def, params);
            d->animationLoading = false;
            return size;
        }
    }

    // Step 2: There was no animation definition, try to get default size from theme index if it is a themed icon
    
    // TODO: change this to simpler function call in Symbian OS env
    if (QDir::isRelativePath(iconName)) {
#ifdef Q_OS_SYMBIAN

        // Try to get themed icon default size from theme index
        ThemeIndexTables &themeIndex = hbInstance->theme()->d_ptr->themeIndex;
        // Check if there is a valid theme index
        if (themeIndex.isValid()) {
            int tableIndex;
            const HbThemeIndexItem *item = themeIndex.getItem(iconName, tableIndex);
            if (item) {
                if (params.mirrored && item->mirroredDefaultSize.isValid()) {
                    size = item->mirroredDefaultSize;
                } else {
                    size = item->defaultSize;
                }
            }
            // Returns invalid size if there is a valid theme index, but the item was not found there.
            return size;
        } else {
    		// Step 3: Theme index was not used, try to get icon's default size from theme server's default size cache.    		
            params.iconFileName = resolveIconFileName(params);		
		}

#else
        params.iconFileName = resolveIconFileName(params);
#endif		
    } else { // Absolute path, use it directly without resolving anything.
        params.iconFileName = iconName;
    }

    // If icon filename could not be resolved, return
    if (params.iconFileName.isEmpty()) {
        return size;
    }

    QString format = formatFromPath(params.iconFileName);

// Theme server on desktop was found very slow (probably due to IPC with QLocalServer/QLocalSocket).
// disabling icon sharing via theme server until theme server performance on desktop is improved
#ifdef  Q_OS_SYMBIAN
    GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
    // Try to take data from server if parameters don't prevent it
    if (manager && format != "MNG" && format != "GIF" &&
        !iconName.startsWith(':')) { // not using server for app's own resources (iconName is a logical name for theme elements)
#ifdef HB_ICON_TRACES
        qDebug() << "HbIconLoader::DefaultSize req to server " << params.iconFileName;
#endif
        QSizeF sizeFromServer = HbThemeClient::global()->getSharedIconDefaultSize(params.iconFileName);
#ifdef HB_ICON_TRACES
        qDebug() << "HbIconLoader::DefaultSize result from server" << sizeFromServer;
#endif
        if (sizeFromServer.isValid()) {
            return sizeFromServer;
        }
    }
#endif // Q_OS_SYMBIAN

    // Step 4: Get the default size from the icon file in the client side
    HbIconSource *source = getIconSource(params.iconFileName, format);
    size = source->defaultSize();

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader::DefaultSize from file" << params.iconFileName << size;
#endif

    return size;
}



HbIconSource *HbIconLoader::getIconSource(const QString &filename, const QString &format)
{
    if (d->lastIconSource && d->lastIconSource->filename() == filename) {
        return d->lastIconSource;
    } else {
        delete d->lastIconSource;
        d->lastIconSource = 0;
        d->lastIconSource = new HbIconSource(filename, format);
        return d->lastIconSource;
    }
}

bool HbIconLoader::iconsExist(const QString &iconName, const QStringList &suffixList)
{
    QString name = HbIconLoaderPrivate::removeIconNameSuffix(iconName);
    bool found = true;

    foreach (const QString &suffix, suffixList) {
        bool dummy = false;

        QString path = HbIconLoaderPrivate::findIconHelper(name + suffix, false, dummy);
        if (path.isEmpty()) {
            found = false;
            break;
        }
    }

    return found;
}

bool HbIconLoader::isAutomaticallyMirrored(const QString &iconName)
{
    return d->isAutomaticallyMirrored(iconName);
}

void HbIconLoader::setSourceResolution(int resolution)
{
    if (resolution != d->sourceResolution) {
        d->sourceResolution = resolution;
        emit defaultSizeAdjustmentChanged();
    }
}

int HbIconLoader::sourceResolution() const
{
    return d->sourceResolution;
}

void HbIconLoader::setResolution(int resolution)
{
    if (resolution != d->resolution) {
        d->resolution = resolution;
        emit defaultSizeAdjustmentChanged();
    }
}

int HbIconLoader::resolution() const
{
    return d->resolution;
}

void HbIconLoader::applyResolutionCorrection(QSizeF &size)
{
    size = size * (qreal)(d->resolution) / (qreal)(d->sourceResolution) * d->zoom; 
}

void HbIconLoader::destroy()
{
    if (theLoader) {
        delete theLoader;
        theLoader = 0;
    }
}

void HbIconLoader::updateLayoutDirection()
{
    // Update the new layout directionality.
    // This method is called upon the signal 'layoutDirectionChangeStarted',
    // which is emitted before the signal 'layoutDirectionChanged'. Icon
    // classes use that signal to update their pixmaps, so the new layout
    // directionality must be updated in the icon loader before that.
    // Thus, there are these separate signals.
    QList<HbMainWindow*> allWindows = hbInstance->allMainWindows();
    HbMainWindow* primaryWindow = allWindows.value(0);

    d->setLayoutMirrored(primaryWindow->layoutDirection() == Qt::RightToLeft);
}

/*!
  \internal

  This is a wrapper for findIconHelper() (when the resource type is
  Hb::EffectIcon). It is used for getting AXML and other content from the
  themeserver.

  If \a resType is Hb::EffectResource then the file is searched among the
  effects in the theme.

  The return value is either same as \a name, when the file is not found in the
  theme, or the full path and name to the file found in the theme. In certain
  situations the return value can also be an empty string, therefore it should
  not be trusted and used without any further examination.
 */
QString HbIconLoader::findSharedResource(const QString &name, Hb::ResourceType resType)
{
    bool temp;
    if (resType == Hb::EffectResource) {
        QMap<int, QString> effectHier = HbThemeUtils::constructHierarchyListWithPathInfo(
            name, HbIconTheme::global()->currentTheme(), Hb::EffectResource);
        QMap<int, QString> effectFiles =
            HbStandardDirs::findResourceList(effectHier, Hb::EffectResource);

        // Just take the first value from the map. (note that maps are sorted by the key)
        foreach (const QString &file, effectFiles) {
            return file;
        }

        // The map was empty, return empty string.
        return QString();
    } else {
        // Assume icon case (png, svg, axml, etc.)

        // TODO: theme index does not support currently axmls, so setting 'useThemeIndex' parameter to false
        return HbIconLoaderPrivate::findIconHelper(name, false, temp, false);
    }
}

void HbIconLoader::resolveCleanIconName(HbIconLoadingParams &params) const
{
    // Replace empty icon name with the "unknown" icon if needed.
    if (params.iconName.isEmpty() && params.options.testFlag(ReturnUnknownIcon)) {
        params.cleanIconName = QString(s_unknown);
    } else {
        params.cleanIconName = params.iconName; 
    }
}

QSizeF HbIconLoader::getAnimationDefaultSize(HbIconAnimationDefinition &def, HbIconLoadingParams &params)
{
    QList<HbIconAnimationDefinition::AnimationFrame> frameDefs = def.frameList();
    QList<HbIconAnimationFrameSet::FrameData> frameList;

    // Get the default size from the first animation frame
    params.cleanIconName = frameDefs.at(0).iconName;
    params.iconFileName = resolveIconFileName(params);
    QString format = formatFromPath(params.iconFileName);

    // Try to get the default size from server if the icon is not such that is loaded in client side.
    if (format != "MNG"
            && format != "GIF"
            && !params.iconName.startsWith(':')) {
        QSizeF defSize = HbThemeClient::global()->getSharedIconDefaultSize(params.iconFileName);
        if (defSize.isValid()) {
            return defSize;
        }
    }

    // Otherwise get if by calling HbIconLoader::defaultSize for the first animation frame
    return HbIconLoader::defaultSize(params.iconFileName, QString(), params.options);
}

void HbIconLoader::loadAnimation(HbIconAnimationDefinition &def, HbIconLoadingParams &params)
{
    Q_ASSERT(!def.isNull());

    QList<HbIconAnimationDefinition::AnimationFrame> frameDefs = def.frameList();
    QList<HbIconAnimationFrameSet::FrameData> frameList;
    
#ifdef HB_ICON_TRACES
    if (!params.animator) {
        qDebug() << "HbIconLoader: no animator ptr provided, loading only frame 1 out of" << frameDefs.count();
    } else {
        qDebug() << "HbIconLoader: loading" << frameDefs.count() << "frames";
    }
#endif
    // If animation pointer is not provided, load only first frame.
    int count = params.animator ? frameDefs.count() : 1;

    // Load each frame
    for (int i = 0; i < count; ++i) {
        const HbIconAnimationDefinition::AnimationFrame &frame = frameDefs.at(i);

        HbIconAnimationFrameSet::FrameData newFrame;
        bool frameReady = false;

        // If same frame pixmap has been already loaded, use that...
        for (int j = 0; j < i; ++j) {
            const HbIconAnimationDefinition::AnimationFrame &otherFrame = frameDefs.at(j);
            if (otherFrame.iconName == frame.iconName) {
                newFrame.pixmap = frameList.at(j).pixmap;
                newFrame.duration = frame.duration;
                newFrame.assignJumps(frame.jumps);
                frameReady = true;
                break;
            }
        }

        // ...otherwise load the frame with the loader
        if (!frameReady) {
            // Frame-by-frame animations are always loaded in normal mode.
            // The mode is applied when the icon is painted.
            newFrame.pixmap = HbIconLoader::loadIcon(
                frame.iconName,
                params.purpose,
                params.size,
                params.aspectRatioMode,
                QIcon::Normal,
                params.options,
                0,
                params.color);
            
            newFrame.duration = frame.duration;
            newFrame.assignJumps(frame.jumps);
        }
        // Append the frame to the animation frame list
        frameList.append(newFrame);
    }

    // Return the first frame in the canvas pixmap
    if (frameList.count()) {
        params.canvasPixmap = frameList.at(0).pixmap;
        // Mirroring is already done when loading the frame.
        // Mode is handled for canvasPixmap in the end of this function.
        params.mirroringHandled = true;
    }

    // If animator pointer has been provided, create animation object
    if (params.animator) {
        // Animator takes ownership of the created animation object
        HbIconAnimationFrameSet *newAnim = new HbIconAnimationFrameSet(params.animator, params.iconName, frameList);
        newAnim->setSize(params.size);
        newAnim->setAspectRatioMode(params.aspectRatioMode);

        if (params.options.testFlag(ResolutionCorrected)) {
            newAnim->setResolutionCorrected(true);
        }

        // Take default size from the first frame
        QSizeF renderSize = QSizeF(params.canvasPixmap.size());

        if(!params.isDefaultSize) {
            renderSize.scale(params.size, params.aspectRatioMode);
        } else if (params.options.testFlag(ResolutionCorrected)) {
            applyResolutionCorrection(renderSize);
        }

        // Small optimization, render size is initialized last so the previous sets do not recalculate it
        newAnim->setRenderSize(renderSize);

        // Set the play mode to the animation.
        newAnim->setPlayMode(def.playMode());

        // Auto-start, if needed.
        if (!params.options.testFlag(HbIconLoader::NoAutoStartAnimation)) {
            newAnim->start();
        }
    }
}

QString HbIconLoader::resolveIconFileName(HbIconLoadingParams &params)
{
    // Search in theme (assume that we received a logical icon name).
    QString iconPath = d->findIconHelper(params.cleanIconName, params.mirrored, params.mirroredIconFound);
    bool iconFound = !iconPath.isEmpty();
#ifdef HB_ICON_TRACES
    qDebug() << params.cleanIconName << " => " << iconPath;
#endif
    // If not found then it can still be a normal file specified with a relative path.
    if (!iconFound) {
        iconFound = QFile(params.iconName).exists();
        if (iconFound) {
            iconPath = params.iconName;
        }
    }
    // Use the 'unknown' icon, if needed, when the queried icon was not found.
    if (!iconFound) {
        if (params.options.testFlag(ReturnUnknownIcon)) {
            iconPath = d->findIconHelper(s_unknown, false, params.mirroredIconFound);
        }
    }

    return iconPath;
}

/*!
 * \fn HbIconImpl *HbIconLoader::getIconFromServer()
 * 
 * Initiate an IPC to themeserver to get the icon-data from the server.
 * 
 */
HbIconImpl *HbIconLoader::getIconFromServer(HbIconLoadingParams &params)
{

    HbIconImpl *icon = 0;

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader::getIconFromServer: req to server for" << params.iconFileName;
#endif
    HbSharedIconInfo iconInfo;
    iconInfo.type = INVALID_FORMAT;
    //Initiate an IPC to themeserver to get the icon-data from the server via themeclient.
    iconInfo = HbThemeClient::global()->getSharedIconInfo(
                   params.iconFileName,
                   params.size,
                   params.aspectRatioMode,
                   params.mode,
                   (params.mirrored && !params.mirroredIconFound),
                   params.options,
                   params.color);

    //Creates HbIconImpl instance based on the type of data returned by themeserver.
    //HbIconImpl thus created could be any one of the following impl-types:
    //1. HbSgImageIconImpl 
    //2. HbNvgIconImpl
    //3. HbPixmapIconImpl
    icon = HbIconImplCreator::createIconImpl(iconInfo, params);

#ifdef HB_ICON_TRACES
    qDebug() << "Image from server: " << params.iconFileName << " offset = " << iconInfo.pixmapData.offset << "icon ptr" << (int) icon;
#endif
    return icon;
}

void HbIconLoader::loadSvgIcon(HbIconLoadingParams &params)
{
    HbIconSource *source = getIconSource(params.iconFileName, "SVG");
    QSvgRenderer *svgRenderer = source->svgRenderer();

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader: SVG renderer created with file " << params.iconFileName;
#endif
    QSizeF renderSize;

    if (svgRenderer && svgRenderer->isValid()) {
        renderSize = QSizeF(svgRenderer->defaultSize());

        if(!params.isDefaultSize) {
            renderSize.scale(params.size, params.aspectRatioMode);
        } else if (params.options.testFlag(ResolutionCorrected)) {
            applyResolutionCorrection(renderSize);
        }

#ifdef HB_SVG_ANIMATION

        // Test whether the content is animated and return animation object if requested
        if (svgRenderer->animated()) {
            params.canCache = false; // Animated SVGs cannot be cached
            if (params.animator) {
                // Animator takes ownership of the created animation object
                HbIconAnimationSvg *newAnim = new HbIconAnimationSvg(
                    params.animator, params.iconName, svgRenderer, params.iconFileName);

                // svgRenderer pointer ownership was transferred to the animation object.
                source->takeSvgRenderer();

                newAnim->setDefaultSize(QSizeF(svgRenderer->defaultSize()));
                newAnim->setSize(params.size);
                newAnim->setAspectRatioMode(params.aspectRatioMode);
                newAnim->setMode(params.mode);
                if (params.mirrored && !params.mirroredIconFound) {
                    newAnim->setMirrored(true);
                }
                if (params.options.testFlag(ResolutionCorrected)) {
                    newAnim->setResolutionCorrected(true);
                }

                // Small optimization, render size is initialized last so the previous sets do not recalculate it
                newAnim->setRenderSize(renderSize);

                if (params.color.isValid()) {
                    newAnim->setColor(params.color);
                }
                animationCreated = true;
            }
        }

#endif // HB_SVG_ANIMATION

        QPixmap &pm = params.canvasPixmap;

        pm = QPixmap(renderSize.toSize());
        pm.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&pm);
        svgRenderer->render(&painter, QRectF(QPointF(), renderSize.toSize()));
        painter.end();
    }
    
    source->releaseSvgRenderer();
}

void HbIconLoader::loadPictureIcon(HbIconLoadingParams &params)
{
    HbIconSource *source = getIconSource(params.iconFileName, "PIC");
    QPicture *picture = source->picture();

    if (picture && !picture->boundingRect().size().isEmpty()) {
        QSizeF picSize = QSizeF(picture->boundingRect().size());
        QSizeF renderSize(picSize);

        qreal sx = 1.0;
        qreal sy = 1.0;
        bool scale = false;

        if(!params.isDefaultSize) {
            scale = true;
            renderSize.scale(params.size, params.aspectRatioMode);
        } else if (params.options.testFlag(ResolutionCorrected)) {
            qreal scaleFactor = (qreal)(d->resolution) / (qreal)(d->sourceResolution) * d->zoom;
            if (!qFuzzyCompare(scaleFactor, qreal(1.0))) {
                scale = true;
                renderSize *= scaleFactor;
            }
        }

        if (scale) {
            // Determine scale factor as QPicture doesn't allow for scaling 
            sx = renderSize.width() / picSize.width();
            sy = renderSize.height() / picSize.height();
        }

        QPixmap &pm = params.canvasPixmap;

        pm = QPixmap(renderSize.toSize());
        pm.fill(Qt::transparent);

        QPainter painter;
        painter.begin(&pm);
        if (scale) {
            painter.scale(sx, sy);
        }
        painter.drawPicture(QPointF(), *picture);
        painter.end();
    }
}

void HbIconLoader::loadAnimatedIcon(HbIconLoadingParams &params, const QString &format)
{
    HbIconSource *source = getIconSource(params.iconFileName, format);
    QImageReader *imgRenderer = source->imageReader();
    QSizeF renderSize;
    bool animationCreated = false;

    if (imgRenderer && imgRenderer->canRead()) {
        renderSize = QSizeF(imgRenderer->size());

        if(!params.isDefaultSize) {
            renderSize.scale(params.size, params.aspectRatioMode);
        } else if (params.options.testFlag(ResolutionCorrected)) {
            applyResolutionCorrection(renderSize);
        }

        params.canCache = false;
        if (params.animator) {
            // Animator takes ownership of the created animation object
            HbIconAnimationImage *newAnim = new HbIconAnimationImage(
                params.animator, params.iconName, params.iconFileName, imgRenderer,
                format == "MNG" ? HbIconAnimation::MNG : HbIconAnimation::GIF);

            // Image reader ownership was transferred to the animation object.
            source->takeImageReader();

            newAnim->setSize(params.size);
            newAnim->setAspectRatioMode(params.aspectRatioMode);
            newAnim->setMode(params.mode);
            if (params.mirrored && !params.mirroredIconFound) {
                newAnim->setMirrored(true);
            }
            if (params.options.testFlag(ResolutionCorrected)) {
                newAnim->setResolutionCorrected(true);
            }

            // Small optimization, render size is initialized last so the previous sets do not recalculate it
            newAnim->setRenderSize(renderSize);

            if (params.color.isValid()) {
                newAnim->setColor(params.color);
            }

            // Auto-start, if needed.
            if (!params.options.testFlag(HbIconLoader::NoAutoStartAnimation)) {
                newAnim->start();
            }

            animationCreated = true;
        }
    }

    // Get the first frame
    if (animationCreated) {
        params.canvasPixmap = params.animator->d->animation->currentFrame();
        // Mirroring and mode are handled in HbIconAnimationImage::currentFrame()
        params.mirroringHandled = true;
        params.modeHandled = true;
    } else {
        QSize scaledSize = renderSize.toSize();
        if (imgRenderer->size() != scaledSize) {
            imgRenderer->setScaledSize(scaledSize);
        }

        QImage img = imgRenderer->read();
        params.canvasPixmap = QPixmap::fromImage(img);
    }
    
    source->releaseImageReader();
}

void HbIconLoader::loadPixmapIcon(HbIconLoadingParams &params, const QString &format)
{
    HbIconSource *source = getIconSource(params.iconFileName, format);

    QPixmap &pm = params.canvasPixmap;
    // Render bitmap graphics onto pixmap
    pm = *source->pixmap();

    if (!pm.isNull()) {
#ifdef ENABLE_EXPERIMENTAL_RESIZE_BOOST__
        // This test implementation improves resize speed up to 5 times..
        if (!params.isDefaultSize && !params.size.isEmpty()) {
            // Smooth scaling is very expensive (size^2). Therefore we reduce the size
            // to 1.5 of the destination size and using fast transformation.
            // Therefore we speed up but don't loose quality..
            if ( pm.size().width() > ( 4 * params.size.toSize().width() ) ) {
                // Improve scaling speed by add an intermediate fast transformation..
                QSize intermediate_size = QSize( params.size.toSize().width() * 2, params.size.toSize().height() * 2 );
                pm = pm.scaled(
                    intermediate_size,
                    params.aspectRatioMode,
                    Qt::FastTransformation ); // Cheap operation!
            }
#endif // ENABLE_EXPERIMENTAL_RESIZE_BOOST__

            pm = pm.scaled(
                params.size.toSize(),
                params.aspectRatioMode,
                Qt::SmoothTransformation); // Expensive operation!

#ifdef ENABLE_EXPERIMENTAL_RESIZE_BOOST__
        }
#endif
    }

    // Delete original pixmap if its size is large
    source->deletePixmapIfLargerThan(PIXMAP_SIZE_LIMIT);
}

/*!
 * \fn HbIconImpl* HbIconLoader::loadIcon()
 * 
 * This function is responsible for loading a single-piece icon .
 * First it checks whether the icon is present on the application (client)cache,
 * if found it increments the ref-count of the HbIconImpl and returns. If the icon
 * is not found in the client's impl-cache, it initiates an IPC to themeserver
 * to load the icon. It receives HbSharedIconInfo from themeserver, creates a HbIconImpl
 * from this data, inserts this into client's icon-impl-cache and returns.
 * 
 */ 
HbIconImpl *HbIconLoader::loadIcon(
    const QString &iconName,
    IconDataType type, 
    HbIconLoader::Purpose purpose, 
    const QSizeF &size, 
    Qt::AspectRatioMode aspectRatioMode, 
    QIcon::Mode mode, 
    IconLoaderOptions options, 
    HbIconAnimator *animator,
    const QColor &color)
{
#ifdef HB_ICON_TRACES
    QString debugString = "HbIconLoader::loadIcon START - ";
    debugString.append(iconName);
    debugString.append(" @ ");
    if (size.isNull()) {
        debugString.append("DEFAULT SIZE");
    } else {
        debugString.append(QString::number(size.width()));
        debugString.append('x');
        debugString.append(QString::number(size.height()));
    }
    qDebug() << debugString;
#endif
    Q_UNUSED( type )

    HbIconImpl* icon = 0;

    if (!size.isValid()) {
        return 0;
    }

    // Populate icon loading parameters
    HbIconLoadingParams params;
    params.iconName = iconName;
    params.purpose = purpose;
    params.size = size;
    params.aspectRatioMode = aspectRatioMode;
    params.mode = mode;
    params.options = options;
    params.animator = animator;
    params.color = color;
    params.isDefaultSize = ( purpose == AnyPurpose ) && size.isNull();
    params.mirrored = options.testFlag(HorizontallyMirrored);
    params.mirroredIconFound = false;
    params.canCache = true;
    params.animationCreated = false;
    params.mirroringHandled = false;
    params.modeHandled = false;
    resolveCleanIconName(params);

    // Step 1: Check if the icon has animation definition

    // This check is to prevent animation frames from trying to load new animations
    if (!d->animationLoading) {
        // Check whether there is a frame-by-frame animation defined for the icon
        HbIconAnimationDefinition def = d->animationManager->getDefinition(params.cleanIconName);
#ifdef HB_ICON_TRACES
        qDebug() << "HbIconLoader: animation def:" << !def.isNull() << " for " << iconName;
#endif
        if (!def.isNull()) {
#ifdef HB_ICON_TRACES
            qDebug("loading anim %s", qPrintable(iconName));
#endif
            params.canCache = false; // The animation is not cacheable, its frames are cached separately

            d->animationLoading = true;
            // Load the animation
            loadAnimation(def, params);
            d->animationLoading = false;

            params.animationCreated = true;
        }
    }

    // Step 2: There was no animation definition, try get icon from server
    if (!params.animationCreated) {
    
#ifdef HB_ICONIMPL_CACHE
        QByteArray cacheKey = d->createCacheKeyFrom( params.iconName, params.size, params.aspectRatioMode,
                        params.mode, params.mirrored, params.color );
        //look up in the local iconImplCache. 
        //If found return the ptr directly
        if (iconImplCache.contains(cacheKey)) {
            HbIconImpl * ptr = iconImplCache.value(cacheKey);
            ptr->incrementRefCount();
#ifdef HB_ICON_CACHE_DEBUG
            qDebug() << "HbIconLoader::loadIcon(): " << "Cache hit in iconImplCache for" << params.iconName<<params.size.height() <<"X"<< params.size.width() ;
            qDebug() << "HbIconLoader::loadIcon(): Client RefCount now = " <<ptr->refCount();
#endif        
            return ptr;
        }
#endif
        if (QDir::isRelativePath(iconName)) {
            params.iconFileName = resolveIconFileName(params);
        } else {
            params.iconFileName = iconName;
        }

        // If icon filename could not be resolved, return
        if (params.iconFileName.isEmpty()) {
#ifdef HB_ICON_TRACES
            qDebug() << "HbIconLoader::loadIcon (empty icon) END";
#endif
            icon = new HbPixmapIconImpl(params.canvasPixmap);
            return icon;
        }

        QString format = formatFromPath(params.iconFileName);

// Theme server on desktop was found very slow (probably due to IPC with QLocalServer/QLocalSocket).
// disabling icon sharing via theme server until theme server performance on desktop is improved
#ifdef  Q_OS_SYMBIAN
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
        // Try to take data from server if parameters don't prevent it
        if (!options.testFlag(DoNotCache) 
            && format != "MNG" 
            && format != "GIF" 
            && !iconName.startsWith(':') // not using server for app's own resources (iconName is a logical name for theme elements)
            && manager ) {

            //Initiate an IPC to themeserver to get the icon-data from the server.
            icon = getIconFromServer(params);

            if (icon) {
#ifdef HB_ICONIMPL_CACHE
                iconImplCache.insert(cacheKey, icon);
#ifdef HB_ICON_CACHE_DEBUG
                qDebug() <<"HbIconLoader::loadIcon(): " << params.iconName << " inserted into impl-cache, ref-count now = " << icon->refCount();
#endif

#endif
                return icon;
            }

        }
#endif // Q_OS_SYMBIAN

        // Step 3: Finally fall back to loading icon locally in the client side
        if (!icon) {
            if (format == "SVG") {
                loadSvgIcon(params);
            } else if (format == "PIC") {
                loadPictureIcon(params);
            } else if (format == "MNG" || format == "GIF") {
                loadAnimatedIcon(params, format);
            } else {
                loadPixmapIcon(params, format);
            }
        }
    }

    if (!params.mirroringHandled) {
        // Apply mirroring if required
        if (params.mirrored && !params.mirroredIconFound) {
            QTransform t;
            t.scale(-1,1);
            params.canvasPixmap = params.canvasPixmap.transformed(t);
        }
    }

    if (!params.modeHandled) {
        // Apply mode
        if (mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            params.canvasPixmap = QApplication::style()->generatedIconPixmap(mode, params.canvasPixmap, &opt);
        }
    }
    
    if ((params.color.isValid()) && (params.mode != QIcon::Disabled)) {
        QPixmap &pm = params.canvasPixmap;
        if(!pm.isNull()) {
            QPixmap mask = pm.alphaChannel(); 
            pm.fill(color);
            pm.setAlphaChannel(mask);
        }
    }

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader::loadIcon END";
#endif

    icon = new HbPixmapIconImpl(params.canvasPixmap);
    return icon;
}

/*!
 * \fn HbIconImpl* HbIconLoader::loadMultiPieceIcon()
 * 
 * This function is responsible for loading a multi-piece icon (e.g. 3-piece or 9-piece).
 * First it checks whether the consolidated (stitched) icon is present in the application (client)
 * cache, if found it increments the ref-count of the HbIconImpl and returns. If the icon
 * is not found in the client's impl-cache, it initiates an IPC to themeserver
 * to try to load the consolidated icon. If the consolidated (stitched) icon fails in themeserver, the server
 * returns a list of icon-data for individual pieces. Each of these pieces will be painted separately
 * 
 */
HbIconImpl* HbIconLoader::loadMultiPieceIcon(const QStringList &listOfIcons,
                                    HbMultiPartSizeData &multiPartIconData,
                                    const QSizeF &size,
                                    Qt::AspectRatioMode aspectRatioMode,
                                    QIcon::Mode mode,
                                    IconLoaderOptions options,
                                    QVector<HbIconImpl *> &multiPieceImpls,
                                    const QColor &color)
{   
    Q_UNUSED(color);
    HbIconImpl* icon = 0;
    if (listOfIcons.count() == 0) {
        return icon;
    }
    
    // Whether the icon should be horizontally mirrored
    bool mirrored = options.testFlag(HorizontallyMirrored);

    // Whether mirrored version of the icon was found in the file system (otherwise it's mirrored by code).
    bool mirroredIconFound = false;

    // We don't want to get the consolidated icon for only NVG build, ie. without SGImage lite support.
    // Consolidated icon will be created for NVG with SGImage lite support.
    // and when NVG is not available.
#if defined(HB_ICONIMPL_CACHE)

    QByteArray cacheKey = d->createCacheKeyFrom(multiPartIconData.multiPartIconId, size,
                          aspectRatioMode, mode, mirrored, color);
    //If consolidated icon found in the client's cache, increment ref-count and return
    if (iconImplCache.contains(cacheKey)) {
        HbIconImpl * ptr = iconImplCache.value(cacheKey);
        ptr->incrementRefCount();
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "HbIconLoader::loadMultiPieceIcon()" << "Cache hit in iconImplCache " << multiPartIconData.multiPartIconId << size.height() << "X" << size.width() ;
        qDebug() << "HbIconLoader::loadMultiPieceIcon : Client RefCount now = " << ptr->refCount();
#endif
        return ptr;
    }
#endif

    QStringList iconPathList;

    for (int i = 0; i < listOfIcons.count(); i++) {
        QString path = d->findIconHelper(listOfIcons.at(i), mirrored, mirroredIconFound);
        if (!path.isEmpty()) {
            iconPathList.append(path);
        } else {
            return icon;
        }
    }

    HbSharedIconInfo iconInfo;
    iconInfo.type = INVALID_FORMAT;
#ifdef  Q_OS_SYMBIAN

    //If consolidated icon was not found in the client's cache, initiate an IPC to load
    //the consolidated icon on themeserver
    iconInfo = HbThemeClient::global()->getMultiPartIconInfo(iconPathList,
                                                             multiPartIconData, size, aspectRatioMode, mode,
                                                             (mirrored && !mirroredIconFound), options, color);

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader::getMultiPartIconInfo, offset from server: " << iconInfo.pixmapData.offset << iconPathList;
#endif

#endif //Q_OS_SYMBIAN

    //Consolidated (stitched) icon was successfully loaded on themeserver side
    if (iconInfo.type != INVALID_FORMAT) {
        int index = iconPathList[0].lastIndexOf("/");
        QString iconId = iconPathList[0].left(index + 1);
        iconId.append(multiPartIconData.multiPartIconId);

        HbIconLoadingParams params;
        params.iconFileName = iconId;
        params.size = size;
        params.aspectRatioMode = aspectRatioMode;
        params.mode = mode;
        params.mirrored = mirrored;
        params.mirroredIconFound = mirroredIconFound;

        //Creating HbIconImpl for the consolidated icon-data returned from themeserver
        icon = HbIconImplCreator::createIconImpl(iconInfo, params);
        if (icon) {
#ifdef HB_ICONIMPL_CACHE
            iconImplCache.insert(cacheKey, icon);
#ifdef HB_ICON_CACHE_DEBUG
            qDebug() << "HbIconLoader::loadMultiPieceIcon(): " << params.iconName << " inserted into impl-cache, ref-count now = " << icon->refCount();
#endif

#endif
            icon->setMultiPieceIcon();
        }
        return icon;
    } else {
        //themeserver wasn't successful in stitching of consolidated icon
        multiPieceImpls.clear();
        int count = iconPathList.count();
        QVector<QSizeF> sizeList;
        for (int i = 0; i < count; i++) {
            sizeList << multiPartIconData.pixmapSizes[i];
        }

#ifdef Q_OS_SYMBIAN
        //Since the consolidated icon-creation failed on themeserver, request loading of individual
        //frame-items in a single IPC request to themeserver
        getMultiIconImplFromServer(iconPathList, sizeList,
                                   aspectRatioMode,
                                   mode,
                                   (mirrored && !mirroredIconFound),
                                   options,
                                   color,
                                   HbIconLoader::AnyType,
                                   HbIconLoader::AnyPurpose,
                                   multiPieceImpls);
#else
        //For OS other than Symbian, call HbIconLoader::loadIcon to individually load icons
        for (int i = 0; i < count; i++) {
            HbIconImpl *impl = loadIcon(iconPathList[i], HbIconLoader::AnyType,
                                        HbIconLoader::AnyPurpose,
                                        sizeList.at(i),
                                        Qt::IgnoreAspectRatio,
                                        QIcon::Normal,
                                        (options | DoNotCache));
            impl->setMultiPieceIcon();
            if (impl) {
                multiPieceImpls.append(impl);
            }
        }

#endif

        return icon;
    }
}

// Initiates an IPC call to the ThemeServer to unload ( decrement ref count ) the icon
void HbIconLoader::unLoadIcon(HbIconImpl * icon)
{
    if (!icon) {
        return;
    }

    icon->decrementRefCount();

    if (icon->refCount() == 0 && icon->isCreatedOnServer()) {
        HbThemeClient::global()->unloadIcon(icon->iconFileName(),
                                            icon->keySize(),
                                            icon->iconAspectRatioMode(),
                                            icon->iconMode(),
                                            icon->isMirrored(),
                                            icon->color()
                                           );
#ifdef HB_ICONIMPL_CACHE
        int rem = iconImplCache.remove(iconImplCache.key(icon));
        if (rem > 0) {
#ifdef HB_ICON_TRACES
            qDebug() << "Removed from HbIconImpl Cache " << rem << icon->iconFileName() << icon->keySize().height() << "X" << icon->keySize().width() ;
#endif
        }
#endif

    }
}

/*!
 * This function is just a wrapper and has performance issues. Try to avoid using it.
 */
QPixmap HbIconLoader::loadIcon(const QString &iconName, HbIconLoader::Purpose purpose, const QSizeF &size,
                               Qt::AspectRatioMode aspectRatioMode, QIcon::Mode mode, IconLoaderOptions options,
                               HbIconAnimator *animator, const QColor &color)
{
    HbIconImpl *icon = loadIcon(iconName, AnyType, purpose, size, aspectRatioMode, mode, options, animator, color);
    QPixmap pixmap;
    if (icon) {
        pixmap = icon->pixmap();
        pixmap.detach();
        unLoadIcon(icon);
        icon->dispose();
    }
    return pixmap;
}

/*!
 * \fn void HbIconLoader::getMultiIconImplFromServer()
 * 
 * This function is responsible for loading individual pieces of a multi-piece icon.
 * This gets called if the consolidated icon-creation process on themeserver has failed.
 * This function initiates a single IPC to themeserver in which it sends out icon-parameters
 * for each of the frame-items and gets back a list of HbSharedIconInfo corresponding to 
 * individual pieces.
 * 
 */
void HbIconLoader::getMultiIconImplFromServer(QStringList &multiPartIconList,                                                             
                                QVector<QSizeF> &sizeList,
                                Qt::AspectRatioMode aspectRatioMode,
                                QIcon::Mode mode,
                                bool mirrored,
                                HbIconLoader::IconLoaderOptions options,
                                const QColor &color,
                                HbIconLoader::IconDataType type,
                                HbIconLoader::Purpose,
                                QVector<HbIconImpl *> & iconImplList)
{
	 Q_UNUSED(type);
	 QVector<int> posList;
#ifdef HB_ICONIMPL_CACHE
    // search the client cache first before asking the server
    for(int i = 0; i < multiPartIconList.count(); i++) {
        QByteArray cacheKey = d->createCacheKeyFrom( multiPartIconList[i], sizeList[i], aspectRatioMode,
                        mode, mirrored, color );
        //look up in the local iconImplCache. 
        //If found return the ptr directly
        HbIconImpl * ptr = 0;
        if (iconImplCache.contains(cacheKey)) {
            ptr = iconImplCache.value(cacheKey);
            // if a specific frame-item is found in local impl-cache, 
            // increment the ref count and remove the entry from the list that needs to be sent to server.
            ptr->incrementRefCount();
#ifdef HB_ICON_CACHE_DEBUG
            qDebug() << "HbIconLoader::getMultiIconImplFromServer()" << "Cache hit in iconImplCache ";
            qDebug() << "HbIconLoader::getMultiIconImplFromServer : Client RefCount now = " << ptr->refCount();
#endif
            iconImplList.append(ptr);
            multiPartIconList.replace(i, QString(""));
        } else {
            posList << i;
        }
    }
    for (int i = 0; i < multiPartIconList.count(); i++) {

        if (multiPartIconList[i].isEmpty()) {
            multiPartIconList.removeAt(i);
            sizeList.remove(i);
            i--;
        }
    }
#endif
    // If client-side cache is not enabled, ask server for all the pieces' information
    int count = multiPartIconList.count();
    if (count > 0) {
        HbSharedIconInfoList iconInfoList = HbThemeClient::global()->getMultiIconInfo(multiPartIconList, sizeList,
                                            aspectRatioMode, mode, mirrored, options, color);

        HbIconImpl* impl = 0;

        HbIconLoadingParams params;

        params.aspectRatioMode = aspectRatioMode;
        params.mode = mode;
        params.mirrored = mirrored;
        //params.mirroredIconFound = mirroredIconFound;


        for (int i = 0; i < count;  i++) {
            if (iconInfoList.icon[i].type != INVALID_FORMAT) {
                params.iconFileName = multiPartIconList[i];
                params.size = sizeList.at(i);

                impl = HbIconImplCreator::createIconImpl(iconInfoList.icon[i], params);

#ifdef HB_ICONIMPL_CACHE
                QByteArray cacheKey = d->createCacheKeyFrom(multiPartIconList[i], sizeList.at(i) , aspectRatioMode,
                                      mode, mirrored, color);
                iconImplCache.insert(cacheKey, impl);
#ifdef HB_ICON_CACHE_DEBUG
                qDebug() << "HbIconLoader::getMultiIconImplFromServer(): " << params.iconName << " inserted into impl-cache, ref-count now = " << impl->refCount();
#endif

#endif
            } else {
                //If for some reason individual frame-item's loading in themeserver fails, use HbIconLoader::loadIcon()
                // as a fallback option to load it.
                impl = loadIcon(multiPartIconList[i], HbIconLoader::AnyType,
                                HbIconLoader::AnyPurpose,
                                sizeList.at(i),
                                Qt::IgnoreAspectRatio,
                                QIcon::Normal,
                                (options | DoNotCache));
            }

            if (impl) {
                impl->setMultiPieceIcon();
#ifdef HB_ICONIMPL_CACHE
                if (posList.count() > 0) {
                    iconImplList.insert(posList.front(), impl);
                    posList.pop_front();
                }
#else
                iconImplList.append(impl);
#endif
            }
        }
    }
}

/*!
 * HbIconLoader::unLoadMultiIcon
 *
 * This function initiates a single IPC to unload each of the frame-items in a multi-piece icon.
 */
void HbIconLoader::unLoadMultiIcon(QVector<HbIconImpl *> &multiPieceImpls)
{
    QStringList iconNameList;
    QVector<QSizeF> sizeList;
     
    // Decrement the ref count. If its zero, remove it from the client cache (if defined) and 
    // then send to server for unload.
    foreach(HbIconImpl* impl, multiPieceImpls) {
        impl->decrementRefCount();
        if (impl->refCount() == 0 && impl->isCreatedOnServer()) {        
#ifdef HB_ICONIMPL_CACHE        
            int rem = iconImplCache.remove(iconImplCache.key(impl));
            if (rem > 0) {
#ifdef HB_ICON_TRACES
            qDebug()<<"HbIconLoader::unLoadMultiIcon :Removed from HbIconImpl Cache "<<rem<< impl->iconFileName()<< icon->keySize().height()<<"X"<<icon->keySize().width() ;
#endif 
            }
#endif      
            // List of icons to be unloaded.
            iconNameList<<impl->iconFileName();
            sizeList<<impl->keySize();
        }
    }
    
    if(iconNameList.count() > 0) {
        HbThemeClient::global()->unLoadMultiIcon(iconNameList, 
                                            sizeList,
                                            multiPieceImpls[0]->iconAspectRatioMode(),
                                            multiPieceImpls[0]->iconMode(),
                                            multiPieceImpls[0]->isMirrored(),
                                            multiPieceImpls[0]->color()
                                            );
    }
}

// End of File
