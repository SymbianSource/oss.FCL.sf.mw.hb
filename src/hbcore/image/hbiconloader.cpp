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

#include "hbiconloader_p.h"
#include "hbframedrawer_p.h"
#include "hbicontheme_p.h"
#include "hblayoutdirectionnotifier_p.h"
#include "hbinstance.h"
#include "hbinstance_p.h"
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
#include "hbiconimplcreator_p.h"
#include "hbiconsource_p.h"
#include "hbthemeindex_p.h"
#include "hbthemecommon_p.h"
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
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#ifdef HB_NVG_CS_ICON
#include "hbeglstate_p.h"
#include "hbnvgrasterizer_p.h"
#endif

// SVG animation is currently disabled because of bugs in QT's svg engine
#undef HB_SVG_ANIMATION

//#define HB_ICON_CACHE_DEBUG

// Icon name without extension
static const char *s_unknown = "unknown";

/*!
    \class HbIconLoader

    \brief HbIconLoader loads and caches vector and raster icons
    either via the Theme Server or directly from files/resources.

    \internal
*/

// Allocated dynamically so it can be deleted before the application object is destroyed.
// Deleting it later causes segmentation fault so Q_GLOBAL_STATIC cannot be used.
static HbIconLoader *theLoader = 0;
static bool loaderDestroyed = false;

// The max consumption for the icons held by the cachekeeper, assuming that each
// icon is 32bpp. Note that the cachekeeper's content is cleared also when
// losing visibility and when switching rendering mode.
const int MAX_KEEPALIVE_CACHE_SIZE_BYTES = 1024 * 1024; // 1 MB

// Icons with size above a certain limit are always ignored by the cachekeeper.
const int MAX_KEEPALIVE_ITEM_SIZE_BYTES = MAX_KEEPALIVE_CACHE_SIZE_BYTES / 2;

static void cleanupLoader()
{
    if (theLoader) {
        delete theLoader;
        theLoader = 0;
    }
}
    
class HbLocalLoaderThread : public QThread
{
public:
    void run();
};

class HbIconLoaderPrivate
{
public:
    HbIconLoaderPrivate();
    ~HbIconLoaderPrivate();

    static HbIconLoaderPrivate *global();

    static QString removeIconNameSuffix(const QString &iconName);

    /* This method is supposed to work the same way
       as the FindIconHelper in the Icon Theme Spec: */
    static QString findSharedResourceHelper(const QString &resourceName,
                                            bool mirrored,
                                            bool &mirroredIconFound,
                                            Hb::ResourceType itemType = Hb::IconResource,
                                            bool useThemeIndex = true);

    static QString findEffectHelper(const QString &effectName);

    bool isAutomaticallyMirrored(const QString &iconName);

    bool isLayoutMirrored();
    void setLayoutMirrored(bool mirrored);

    QByteArray createCacheKeyFrom(const QString &iconName,
                                  const QSizeF &size,
                                  Qt::AspectRatioMode aspectRatioMode,
                                  QIcon::Mode mode,
                                  bool mirrored,
                                  const QColor &color);

    void addItemToCache(const QByteArray &cacheKey, HbIconImpl *iconImpl);

    QString storedTheme;

    int sourceResolution;
    int resolution;
    qreal zoom;

    // Frame-by-frame animation support -------------------------------------->
    HbIconAnimationManager *animationManager;
    // Flag to prevent animation frames from loading new animations recursively
    bool animationLoading;
    // <-- Frame-by-frame animation support ------------------------------------

    HbIconSource *lastIconSource;

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

    struct AsyncParams {
        HbIconLoader::HbAsyncIconLoaderCallback mCallback;
        HbIconLoadingParams mLdParams;
        void *mParam;
    };

    QList<AsyncParams *> mActiveAsyncRequests;

    HbLocalLoaderThread mLocalLoaderThread;
    HbLocalIconLoader *mLocalLoader;
    QMutex mLocalLoadMutex;
    QMutex mIconSourceMutex;
    friend class HbLocalLoaderThread;

    /*
     * Client side caching of sgimage icon required, as sgimage lite cannot be
     * opened multiple times
     *
     * It is also beneficial for performance, because it reduces IPC in certain
     * cases.
     *
     * Note that by default this is not a permanent cache, i.e. when an iconimpl's
     * refcount reaches zero it is removed from the cache.  This means that the
     * cache is beneficial for having the same icon rendered by two or more HbIcons
     * at the same time (very typical in some itemview (e.g. list widget) cases),
     * but it would not benefit a load-unload-load scenario because the icon is
     * removed from the cache during the unload when there are no references
     * anymore.
     *
     * However the cachekeeper below will change this behavior, preventing refcounts
     * reaching zero in unLoadIcon(), so this cache may contain also icons that are
     * not really in use and are only referenced by the cachekeeper. This is
     * required for further reduction of IPC calls.
     */
    QHash<QByteArray, HbIconImpl *> iconImplCache;

    // The global cachekeeper instance will hold references to icons that would
    // normally be unloaded (i.e. mIcons will contain icons with refcount 1).
    //
    // Icons get added from unLoadIcon(), meaning that if the cachekeeper decides to
    // hold a reference then the icon is not really unloaded (and thus stays in
    // iconImplCache).
    //
    // When the icon gets referenced due to a cache hit, in loadIcon() and other
    // places, the icon is removed from the cachekeeper.
    class CacheKeeper {
    public:
        CacheKeeper(HbIconLoaderPrivate *p) : mConsumption(0), mIconLoaderPrivate(p) { }
        void ref(HbIconImpl *icon);
        void unref(HbIconImpl *icon);
        void clear();
    private:
        void del(HbIconImpl *icon, bool sendUnloadReq);
        QList<HbIconImpl *> mIcons;
        int mConsumption;
        HbIconLoaderPrivate *mIconLoaderPrivate;
        friend class HbIconLoaderPrivate;
    };

    CacheKeeper cacheKeeper;
};

void HbLocalLoaderThread::run()
{
    setPriority(QThread::LowPriority);
    exec();
    delete HbIconLoaderPrivate::global()->mLocalLoader;
}

HbIconLoaderPrivate::HbIconLoaderPrivate() :
    storedTheme(HbTheme::instance()->name()),
    sourceResolution(144), // This is about the resolution of a Nokia N95 8GB
    resolution(144),
    zoom(1.0),
    animationManager(HbIconAnimationManager::global()),
    animationLoading(false),
    lastIconSource(0),
    layoutMirrored(Unknown),
    mLocalLoadMutex(QMutex::Recursive),
    mIconSourceMutex(QMutex::Recursive),
    cacheKeeper(this)
{
    qRegisterMetaType<HbIconImpl *>();
    qRegisterMetaType<HbIconLoadingParams>();
    qRegisterMetaType<void *>();
    mLocalLoader = new HbLocalIconLoader;
    mLocalLoader->moveToThread(&mLocalLoaderThread);
    mLocalLoaderThread.start();
}

HbIconLoaderPrivate::~HbIconLoaderPrivate()
{
    // quit() cannot be called directly on mLocalLoaderThread because
    // there is a chance that start() has not yet executed.
    QMetaObject::invokeMethod(mLocalLoader, "doQuit", Qt::QueuedConnection);
    mLocalLoaderThread.wait();
    delete lastIconSource;
    qDeleteAll(mActiveAsyncRequests);
    cacheKeeper.clear();
    // There may be icons in iconImplCache at this point and they are not
    // necessarily leftovers so they must not be destroyed. Depending on how the
    // app is implemented there is a (small) chance that the HbIconLoader is
    // destroyed before the destructors of icon engines or framedrawers are run
    // so it must be left up to them to correctly unref all icons.
    iconImplCache.clear();
}

HbIconLoaderPrivate *HbIconLoaderPrivate::global()
{
    HbIconLoader *loader = HbIconLoader::global();
    return loader->d;
}

QString HbIconLoaderPrivate::removeIconNameSuffix(const QString &iconName)
{
    QString loweredIconName = iconName.toLower();
    if (loweredIconName.endsWith(QLatin1String(".svg"))
            || loweredIconName.endsWith(QLatin1String(".png"))
            || loweredIconName.endsWith(QLatin1String(".mng"))
            || loweredIconName.endsWith(QLatin1String(".gif"))
            || loweredIconName.endsWith(QLatin1String(".xpm"))
            || loweredIconName.endsWith(QLatin1String(".jpg"))
            || loweredIconName.endsWith(QLatin1String(".nvg"))) {
        return iconName.left(iconName.length() - 4);
    }

    if (loweredIconName.endsWith(QLatin1String(".svgz"))
            || loweredIconName.endsWith(QLatin1String(".qpic"))) {
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

QString HbIconLoaderPrivate::findSharedResourceHelper(const QString &resourceName, bool mirrored,
                                                      bool &mirroredIconFound, Hb::ResourceType itemType,
                                                      bool useThemeIndex)
{
    Q_UNUSED(useThemeIndex)
    Q_UNUSED(itemType)

    mirroredIconFound = false;
    QString iconPath;

    if (HbThemeUtils::isLogicalName(resourceName)) {
        // Try to get themed icon information from theme index
        HbThemeIndexResource resource(resourceName);
        if (resource.isValid()) {
            if (mirrored) {
                return resource.fullMirroredFileName(mirroredIconFound);
            } else {
                return resource.fullFileName();
            }
        } else {
            // Logical name not found in theme index - return empty string
            return QString();
        }
    } else {
        // Not a logical name. Check from file system.
        if (mirrored) {
            // If icon is mirrored, try to find the icon in a separate "mirrored" folder used for mirrored icons

            // Find the directory part from the full filename
            int index1 = resourceName.lastIndexOf('/');
            int index2 = resourceName.lastIndexOf('\\');

            int index = index1 > index2 ? index1 : index2;

            QString iconNameCopy(resourceName);

            if (index > 0) {
                iconNameCopy.insert(index + 1, QString("mirrored/"));
            }
            if (QFile::exists(iconNameCopy)) {
                iconPath = iconNameCopy;
            }

            if (!iconPath.isEmpty()) {
                mirroredIconFound = true;
                return iconPath;
            }
        }

        if (QFile::exists(resourceName)) {
            iconPath = resourceName;
        }
    }

    return iconPath;
}

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

bool HbIconLoaderPrivate::isAutomaticallyMirrored(const QString &iconName)
{
    // only place to get mirroring information is from themeindex
    // Try to get themed icon information from theme index
    HbThemeIndexResource resource(iconName);
    if (resource.isValid()) {
        return resource.isAutomaticallyMirrored();
    }

    return false;
}

bool HbIconLoaderPrivate::isLayoutMirrored()
{
    if (layoutMirrored == Unknown) {
        // The layout directionality is defined by asking it from the main window.
        QList<HbMainWindow *> allWindows = hbInstance->allMainWindows();
        HbMainWindow *primaryWindow = allWindows.value(0);
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

QByteArray HbIconLoaderPrivate::createCacheKeyFrom(const QString &iconName,
                                                   const QSizeF &size,
                                                   Qt::AspectRatioMode aspectRatioMode,
                                                   QIcon::Mode mode,
                                                   bool mirrored,
                                                   const QColor &color)
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

    // The rendering mode should not be included in the cache key to prevent
    // confusion when the requested and the received rendering modes are
    // different (i.e. to make life simple). Having a cache hit is always
    // preferable to anything else.

    cacheKey.append((char *)&(temp[0]), sizeof(int)*paramArraySize);

    const QChar *iconNamePtr = iconName.constData();
    cacheKey.append((char *)iconNamePtr, nameSize * sizeof(QChar));

    if (mirrored) {
        cacheKey.append('M');
    }

    return cacheKey;
}

inline HbThemeClient::IconReqInfo paramsToReqInfo(const HbIconLoadingParams &params)
{
    HbThemeClient::IconReqInfo reqInfo;
    reqInfo.iconPath = params.iconFileName;
    reqInfo.size = params.size;
    reqInfo.aspectRatioMode = params.aspectRatioMode;
    reqInfo.mode = params.mode;
    reqInfo.mirrored = (params.mirrored && !params.mirroredIconFound);
    reqInfo.options = params.options;
    reqInfo.color = params.color;
    reqInfo.renderMode = params.renderMode;
    return reqInfo;
}

inline HbThemeClient::IconReqInfo iconImplToReqInfo(HbIconImpl *icon)
{
    HbThemeClient::IconReqInfo reqInfo;
    reqInfo.iconPath = icon->iconFileName();
    reqInfo.size = icon->keySize();
    reqInfo.aspectRatioMode = icon->iconAspectRatioMode();
    reqInfo.mode = icon->iconMode();
    reqInfo.mirrored = icon->isMirrored();
    reqInfo.color = icon->color();
    reqInfo.renderMode = icon->iconRenderingMode();
    return reqInfo;
}

HbIconLoader::HbIconLoader(const QString &appName, QObject *parent)
    : QObject(parent)
{
    setObjectName(appName);
    d = new HbIconLoaderPrivate;
    connect(d->mLocalLoader, SIGNAL(ready(HbIconLoadingParams, void*)), SLOT(localLoadReady(HbIconLoadingParams, void*)));

    // Set default rendering mode to EHWRendering
    renderMode = EHWRendering;

    // Delete the icon loader during QCoreApplication destruction. Relying on
    // aboutToQuit() would cause destruction to happen too early, with Q_GLOBAL_STATIC it
    // would be too late.  Recreation is forbidden so HbIconLoader::global() will return 0
    // after destroying the application instance but at least in a typical application the
    // loader will still be available e.g. in widget (and thus graphics widget, hb view,
    // etc.) destructors.
    qAddPostRoutine(cleanupLoader);

    connect(HbLayoutDirectionNotifier::instance(), SIGNAL(layoutDirectionChangeStarted()),
            this, SLOT(updateLayoutDirection()));

    HbTheme *theme = hbInstance->theme();
    connect(&theme->d_ptr->iconTheme, SIGNAL(iconsUpdated(QStringList)), SLOT(themeChange(QStringList)));
    connect(theme, SIGNAL(changeFinished()), SLOT(themeChangeFinished()));
}

HbIconLoader::~HbIconLoader()
{
    delete d;
    loaderDestroyed = true;
}

HbIconLoader *HbIconLoader::global()
{
    // Allocated dynamically so it can be deleted before the application object is destroyed.
    // Deleting it later causes segmentation fault.
    // Once destroyed, creating the loader again must not be allowed (e.g. because there
    // may not be a QApplication instance anymore at this stage). It is normal to
    // return null pointer in this case, it can happen only during app shutdown. If anybody
    // is using HbIconLoader::global() from destructors, they have to be prepared for the
    // null ptr result too.
    if (!theLoader && !loaderDestroyed) {
        theLoader = new HbIconLoader;
    }
    if (!theLoader) {
        qWarning("HbIconLoader instance not present, returning null.");
    }
    return theLoader;
}

QSizeF HbIconLoader::defaultSize(const QString &iconName, const QString &appName, IconLoaderOptions options)
{
    Q_UNUSED(appName)
    QMutexLocker locker(&d->mIconSourceMutex);
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

    // Step 2: There was no animation definition,
    // try to get default size from theme index if it is a themed icon (logical name).

    if (HbThemeUtils::isLogicalName(iconName)) {
        // Try to get themed icon information from theme index
        HbThemeIndexResource resource(iconName);
        if (resource.isValid()) {
            // Try to get themed icon default size from theme index
            if (params.mirrored && resource.mirroredItemSize().isValid()) {
                size = resource.mirroredItemSize();
            } else {
                size = resource.defaultItemSize();
            }
        }
        // Returns invalid size if the index did not provide the size
        return size;
    } else { // Absolute path, use it directly without resolving anything.
        params.iconFileName = iconName;
    }

    // If icon filename could not be resolved, return
    if (params.iconFileName.isEmpty()) {
        return size;
    }

    QString format = formatFromPath(params.iconFileName);

    // Step 3: Get the default size from the icon file in the client side
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
    bool found = true;
    bool logicalName = HbThemeUtils::isLogicalName(iconName);

    int suffixIndex = iconName.length();
    if (!logicalName) {
        // If it is an absolute icon path, the suffix is inserted before the file extension
        int index = iconName.lastIndexOf(QChar('.'));
        if (index > 0) {
            suffixIndex = index;
        }
    }

    foreach(const QString & suffix, suffixList) {
        bool dummy = false;

        QString nameWithSuffix = iconName;
        nameWithSuffix.insert(suffixIndex, suffix);

        QString path = HbIconLoaderPrivate::findSharedResourceHelper(nameWithSuffix, false, dummy);
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

void HbIconLoader::themeChange(const QStringList &updatedFiles)
{
    // For icons, the content is dropped in HbIconEngine.
    // For framedrawers, HbFrameItem notifies the framedrawer when the theme changes.
    // This below is only needed to support partial theme updates for framedrawers in external tools.
    // (standalone framedrawers do not update automatically, except in tools)
#ifdef HB_TOOL_INTERFACE
    foreach(HbFrameDrawerPrivate * frameDrawer, frameDrawerInstanceList) {
        frameDrawer->themeChange(updatedFiles);
    }
#else
    Q_UNUSED(updatedFiles);
#endif
}

void HbIconLoader::themeChangeFinished()
{
#ifdef HB_ICON_CACHE_DEBUG
    qDebug("HbIconLoader::themeChangeFinished: dropping unused icons");
#endif
    // We need to drop unused icons to prevent reusing them now that the theme is different.
    // Doing it in themeChange() would not be right, it would be too early,
    // because unloading would make the dropped icons unused (but referenced) so
    // we would end up with reusing the old graphics. This here is safe.
    d->cacheKeeper.clear();
}

void HbIconLoader::destroy()
{
    cleanupLoader();
}

void HbIconLoader::updateLayoutDirection()
{
    // Update the new layout directionality.
    // This method is called upon the signal 'layoutDirectionChangeStarted',
    // which is emitted before the signal 'layoutDirectionChanged'. Icon
    // classes use that signal to update their pixmaps, so the new layout
    // directionality must be updated in the icon loader before that.
    // Thus, there are these separate signals.
    QList<HbMainWindow *> allWindows = hbInstance->allMainWindows();
    if (allWindows.count()) {
        HbMainWindow *primaryWindow = allWindows.value(0);
        d->setLayoutMirrored(primaryWindow->layoutDirection() == Qt::RightToLeft);
    }
}

void HbIconLoader::handleForegroundLost()
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    freeIconData();
    // delete the VGImage
    HbEglStates *eglStateInstance = HbEglStates::global();
    eglStateInstance->handleForegroundLost();
    // notify the server to clear the SGImage and NVG type of icons from the client's session
    HbThemeClient::global()->notifyForegroundLostToServer();
#endif
}

void HbIconLoaderPrivate::addItemToCache(const QByteArray &cacheKey, HbIconImpl *iconImpl)
{
#ifdef HB_ICON_CACHE_DEBUG
    if (iconImplCache.contains(cacheKey)) {
        qWarning() << "HbIconLoader::addItemToCache: Possible leak: Icon with same key for"
                   << iconImpl->iconFileName() << "is already in cache";
    }
#endif

    iconImplCache.insert(cacheKey, iconImpl);

#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbIconLoader::addItemToCache: added" << iconImpl->iconFileName()
             << "cache item count now" << iconImplCache.count();
#endif
}

/*!
 * Removes the iconimpl entry from the client side cache
 */
void HbIconLoader::removeItemInCache(HbIconImpl *iconImpl)
{
    if (iconImpl) {
        if (d->iconImplCache.remove(d->iconImplCache.key(iconImpl)) > 0) {
#ifdef HB_ICON_CACHE_DEBUG
            qDebug() << "HbIconLoader::removeItemInCache: Removed"
                     << iconImpl->iconFileName() << iconImpl->keySize();
#endif
        }
    }
}

/*!
 *  Cleans up (deletes) the HbIconImpl instances at the client side
 *  It also resets the engine's iconImpl and MaskableIcon's iconImpl
 */
void HbIconLoader::freeGpuIconData()
{
#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    d->cacheKeeper.clear(); // unref all unused icons
    for (int i = 0; i < iconEngineList.count(); i++) {
        HbIconEngine *engine = iconEngineList.at(i);
        if (engine->iconFormatType() == SGIMAGE || engine->iconFormatType() == NVG) {
            engine->resetIconImpl();
        }
    }
    for (int i = 0; i < frameDrawerInstanceList.count(); i++) {
        HbFrameDrawerPrivate *fd = frameDrawerInstanceList.at(i);
        if (fd->iconFormatType() == SGIMAGE || fd->iconFormatType() == NVG) {
            fd->resetMaskableIcon();
        }
    }
#endif
}

/*!
 *  Cleans up (deletes) the HbIconImpl instances at the client side
 *  It also resets the engine's iconImpl and MaskableIcon's iconImpl
 */
void HbIconLoader::freeIconData()
{
    d->cacheKeeper.clear(); // unref all unused icons
    for (int i = 0; i < iconEngineList.count(); i++) {
        HbIconEngine *engine = iconEngineList.at(i);
        engine->resetIconImpl();
    }
    for (int i = 0; i < frameDrawerInstanceList.count(); i++) {
        HbFrameDrawerPrivate *fd = frameDrawerInstanceList.at(i);
        fd->resetMaskableIcon();

    }
}


/*!
  \internal

  This is a wrapper for findSharedResourceHelper(). It is used for getting
  resources from the themeserver.

  The return value is either same as \a name, when the file is not found in the
  theme, or the full path and name to the file found in the theme. In certain
  situations the return value can also be an empty string, therefore it should
  not be trusted and used without any further examination.
 */
QString HbIconLoader::findSharedResource(const QString &name, Hb::ResourceType resType)
{
    bool temp;
    return HbIconLoaderPrivate::findSharedResourceHelper(name, false, temp, resType);
}

/*!
  This function is used to register the IconEngine instance to IconLoader
 */
void HbIconLoader::storeIconEngineInfo(HbIconEngine *iconEngine)
{
    iconEngineList.append(iconEngine);
}

/*!
  This function is used to unregister the Iconengine instance from Iconloader
 */
void HbIconLoader::removeIconEngineInfo(HbIconEngine *iconEngine)
{
    iconEngineList.removeOne(iconEngine);
}

/*!
  This function is used to register the FrameDrawerPrivate instance to IconLoader
 */
void HbIconLoader::storeFrameDrawerInfo(HbFrameDrawerPrivate *frameDrawer)
{
    frameDrawerInstanceList.append(frameDrawer);
}

/*!
  This function is used to unregister the FrameDrawerPrivate instance from IconLoader
 */
void HbIconLoader::removeFrameDrawerInfo(HbFrameDrawerPrivate *frameDrawer)
{
    frameDrawerInstanceList.removeOne(frameDrawer);
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
    return HbIconLoader::defaultSize(frameDefs.at(0).iconName, QString(), params.options);
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
                                  params.options | DoNotCache,
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
        params.image = frameList.at(0).pixmap.toImage();
        // Mirroring is already done when loading the frame.
        // Mode is handled for the image at the end of this function.
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
        QSizeF renderSize = QSizeF(params.image.size());

        if (!params.isDefaultSize) {
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
    QString iconPath = d->findSharedResourceHelper(params.cleanIconName, params.mirrored, params.mirroredIconFound);
    bool iconFound = !iconPath.isEmpty();
#ifdef HB_ICON_TRACES
    qDebug() << params.cleanIconName << " => " << iconPath;
#endif
    // Use the 'unknown' icon, if needed, when the queried icon was not found.
    if (!iconFound) {
        if (params.options.testFlag(ReturnUnknownIcon)) {
            iconPath = d->findSharedResourceHelper(s_unknown, false, params.mirroredIconFound);
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
#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader::getIconFromServer: req to server for" << params.iconFileName;
#endif

    HbSharedIconInfo iconInfo;
    iconInfo.type = INVALID_FORMAT;
    HbThemeClient *themeClient = HbThemeClient::global();
    // Initiate an IPC to themeserver to get the icon-data from the server via themeclient.
    iconInfo = themeClient->getSharedIconInfo(paramsToReqInfo(params));
    HbIconImpl *icon = finishGetIconFromServer(iconInfo, params);

#ifdef HB_ICON_TRACES
    qDebug() << "Image from server: " << params.iconFileName << " offset = " << iconInfo.pixmapData.offset << "icon ptr" << (int) icon;
#endif

    return icon;
}

HbIconImpl *HbIconLoader::finishGetIconFromServer(HbSharedIconInfo &iconInfo, HbIconLoadingParams &params)
{
    //Creates HbIconImpl instance based on the type of data returned by themeserver.
    //HbIconImpl thus created could be any one of the following impl-types:
    //1. HbSgImageIconImpl
    //2. HbNvgIconImpl
    //3. HbPixmapIconImpl
    HbIconImpl *icon = HbIconImplCreator::createIconImpl(iconInfo, params);

    if (icon && icon->initialize() == HbIconImpl::ErrorLowGraphicsMemory) {
        // initialisation failed due to low graphics memory, in sgimage icon case
        // try creating pixmap based icon

        // unload the created GPU icon
        unLoadIcon(icon, false, true);
        icon = 0;

        // create a pixmap based icon
        HbThemeClient *themeClient = HbThemeClient::global();
        HbThemeClient::IconReqInfo reqInfo = paramsToReqInfo(params);
        reqInfo.renderMode = ESWRendering;
        iconInfo = themeClient->getSharedIconInfo(reqInfo);
        icon = HbIconImplCreator::createIconImpl(iconInfo, params);
        // no need call initialize of this icon
    }

    return icon;
}

bool HbIconLoader::asyncCallback(const HbSharedIconInfo &info, void *param)
{
    HbIconLoader *self = theLoader;
    HbIconLoaderPrivate::AsyncParams *p = static_cast<HbIconLoaderPrivate::AsyncParams *>(param);
    HbSharedIconInfo iconInfo = info;
    if (!self->d->mActiveAsyncRequests.contains(p)) {
        // 'p' is destroyed already, this happens when canceling the loadIcon
        // request. Stop here and return false so the themeclient will issue an
        // unload request.
        return false;
    }
    bool result = true;
    // The icon may have been loaded via another request meanwhile so check the cache again.
    HbIconImpl *icon = self->lookupInCache(p->mLdParams, 0);
    if (icon) {
        // Found in the local iconimpl cache so use that and roll back, i.e. do
        // an unload for the data we just got (this is necessary to maintain
        // proper remote refcounts).
        result = false;
    } else {
        icon = self->finishGetIconFromServer(iconInfo, p->mLdParams);
        if (!icon) {
            self->loadLocal(p->mLdParams, self->formatFromPath(p->mLdParams.iconFileName));
            icon = self->finishLocal(p->mLdParams);
        }
        if (icon) {
            self->cacheIcon(p->mLdParams, icon, 0);
        }
    }
    if (p->mCallback) {
        p->mCallback(icon, p->mParam, false);
    }
    self->d->mActiveAsyncRequests.removeOne(p);
    delete p;
    return result;
}

void HbIconLoader::getIconFromServerAsync(HbIconLoadingParams &params,
                                          HbAsyncIconLoaderCallback callback,
                                          void *callbackParam)
{
    HbThemeClient *themeClient = HbThemeClient::global();
    HbIconLoaderPrivate::AsyncParams *p = new HbIconLoaderPrivate::AsyncParams;
    p->mCallback = callback;
    p->mLdParams = params;
    p->mParam = callbackParam;
    d->mActiveAsyncRequests.append(p);
    themeClient->getSharedIconInfo(paramsToReqInfo(params),
                                   asyncCallback,
                                   p);
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

        if (!params.isDefaultSize) {
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

        params.image = QImage(renderSize.toSize(), QImage::Format_ARGB32_Premultiplied);
        params.image.fill(QColor(Qt::transparent).rgba());
        QPainter painter;
        painter.begin(&params.image);
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

        if (!params.isDefaultSize) {
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

        params.image = QImage(renderSize.toSize(), QImage::Format_ARGB32_Premultiplied);
        params.image.fill(QColor(Qt::transparent).rgba());
        QPainter painter;
        painter.begin(&params.image);
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

        if (!params.isDefaultSize) {
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
        params.image = params.animator->d->animation->currentFrame().toImage();
        // Mirroring and mode are handled in HbIconAnimationImage::currentFrame()
        params.mirroringHandled = true;
        params.modeHandled = true;
    } else {
        QSize scaledSize = renderSize.toSize();
        if (imgRenderer->size() != scaledSize) {
            imgRenderer->setScaledSize(scaledSize);
        }
        params.image = imgRenderer->read();
    }

    source->releaseImageReader();
}

void HbIconLoader::loadPixmapIcon(HbIconLoadingParams &params, const QString &format)
{
    HbIconSource *source = getIconSource(params.iconFileName, format);

    // Render bitmap graphics into an image. We have to use QImage because this
    // code may run outside the gui (main) thread.
    params.image = *source->image();

    if (!params.image.isNull()) {
        // This implementation improves resize speed up to 5 times.
        if (!params.isDefaultSize && !params.size.isEmpty()) {
            // Smooth scaling is very expensive (size^2). Therefore we reduce the size
            // to 1.5 of the destination size and using fast transformation.
            // Therefore we speed up but don't loose quality..
            if (params.image.size().width() > (4 * params.size.toSize().width())) {
                // Improve scaling speed by add an intermediate fast transformation..
                QSize intermediate_size = QSize(params.size.toSize().width() * 2, params.size.toSize().height() * 2);
                params.image = params.image.scaled(
                    intermediate_size,
                    params.aspectRatioMode,
                    Qt::FastTransformation);  // Cheap operation
            }

            params.image = params.image.scaled(
                params.size.toSize(),
                params.aspectRatioMode,
                Qt::SmoothTransformation); // Expensive operation
        }
    }

    // Delete original pixmap if its size is large
    source->deleteImageIfLargerThan(IMAGE_SIZE_LIMIT);
}

void HbIconLoader::loadNvgIcon(HbIconLoadingParams &params )
{
#ifdef HB_NVG_CS_ICON
    HbIconSource *source = getIconSource(params.iconFileName, "NVG");
    if (!source) {
        return;
    }

    HbNvgRasterizer * nvgRasterizer = HbNvgRasterizer::global();
    QByteArray *sourceByteArray = source->byteArray();
    if( !sourceByteArray ) {
       return;
    }

    QByteArray nvgArray = *sourceByteArray;
    QSizeF renderSize = source->defaultSize();
    if (!params.isDefaultSize) {
        renderSize.scale(params.size, params.aspectRatioMode);
    } else if (params.options.testFlag(ResolutionCorrected)) {
        applyResolutionCorrection(renderSize);
    }

    QSize iconSize = renderSize.toSize();
    QImage image(iconSize, QImage::Format_ARGB32_Premultiplied);
    QImage::Format imageFormat = image.format();
    int stride = image.bytesPerLine();
    void * rasterizedData = image.bits();

    bool success = nvgRasterizer->rasterize(nvgArray, iconSize,
                                            params.aspectRatioMode,
                                            rasterizedData, stride,imageFormat);
    if (success) {
        params.image = image;
    }

#else
    Q_UNUSED(params)
#endif

}

/*!
 * \fn void HbIconLoader::switchRenderingMode()
 *
 * This function gets notified when the rendering mode of the application changes e.g
 * ( Hardware - Software rendering or vice versa ). If the mode is changed from
 *  Hardware to Software, all Hardware rendered icons will release the GPU resources.
 *  This function also initiates an IPC call to ThemeServer, so that the server
 *  can do its part of cleanup.
 *  \a newRenderMode new rendering mode of application
 */

void HbIconLoader::switchRenderingMode(HbRenderingMode newRenderMode)
{
#ifndef Q_OS_SYMBIAN
    Q_UNUSED(newRenderMode)
#endif

#if defined(HB_SGIMAGE_ICON) || defined(HB_NVG_CS_ICON)
    if (newRenderMode != renderMode) {
        d->cacheKeeper.clear(); // unref all unused icons
        if (newRenderMode == ESWRendering) {
            // switching from HW to SW mode
            freeGpuIconData();
        }
        if (HbThemeClient::global()->switchRenderingMode(newRenderMode)) {
            renderMode = newRenderMode;
        }
    }
#endif
}

void HbIconLoader::updateRenderingMode(QPaintEngine::Type type)
{
    if (type == QPaintEngine::OpenVG) {
        renderMode = EHWRendering;
    } else {
        renderMode = ESWRendering;
    }
}

// Note that this is not the same as HbThemeUtils::isLogicalName.
// A non-logical name can still indicate content that must go through themeserver.
inline bool isLocalContent(const QString &iconName)
{
    // Check if we have a simple file or embedded resource, given with full
    // or relative path. A filename like "x.png" is also local content
    // because logical icon names must never contain an extension.
    bool localContent = iconName.startsWith(':')
        || iconName.contains('/') || iconName.contains('\\')
        || (iconName.length() > 1 && iconName.at(1) == ':')
        || (iconName.contains('.') && !iconName.startsWith("qtg_", Qt::CaseInsensitive));

    return localContent;
}

inline bool serverUseAllowed(const QString &iconName, HbIconLoader::IconLoaderOptions options)
{
    bool allowServer = !options.testFlag(HbIconLoader::DoNotCache);

    // No local loading for NVG... The server must still be used.
    if (iconName.endsWith(".nvg", Qt::CaseInsensitive)) {
        allowServer = true;
    }
    // We have no choice but to assume that theme graphics (logical names) are
    // NVG in order to keep it working with DoNotCache.
    if (!iconName.contains('.') && !iconName.contains('/') && !iconName.contains('\\')) {
        allowServer = true;
    }

    return allowServer;
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
    const QColor &color,
    HbAsyncIconLoaderCallback callback,
    void *callbackParam)
{
#ifdef HB_ICON_TRACES
    qDebug() << "loadIcon" << iconName << size;
#endif
    Q_UNUSED(type)

    HbIconImpl *icon = 0;

    if (!size.isValid()) {
        if (callback) {
            callback(0, callbackParam, true);
        }
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
    params.isDefaultSize = (purpose == AnyPurpose) && size.isNull();
    params.mirrored = options.testFlag(HorizontallyMirrored);
    params.mirroredIconFound = false;
    params.canCache = true;
    params.animationCreated = false;
    params.mirroringHandled = false;
    params.modeHandled = false;
    params.renderMode = renderMode;
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
    QByteArray cacheKey;
    if (!params.animationCreated) {
        // First check in the local iconimpl cache.
        HbIconImpl *cachedIcon = lookupInCache(params, &cacheKey);
        if (cachedIcon) {
            if (callback) {
                callback(cachedIcon, callbackParam, true);
            }
            return cachedIcon;
        }

        // Resolve used icon filename. It uses themeindex for themed icons.
        params.iconFileName = resolveIconFileName(params);

        if (HbThemeUtils::isLogicalName(iconName)) {
            params.iconFileName = resolveIconFileName(params);
        }

        // If icon filename could not be resolved, return
        if (params.iconFileName.isEmpty()) {
#ifdef HB_ICON_TRACES
            qDebug() << "HbIconLoader::loadIcon (empty icon) END";
#endif
            icon = new HbPixmapIconImpl(QPixmap());
            if (callback) {
                callback(icon, callbackParam, true);
            }
            return icon;
        }

        QString format = formatFromPath(params.iconFileName);

#ifdef Q_OS_SYMBIAN
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory)
        // Try to take data from server if parameters don't prevent it
            if (serverUseAllowed(iconName, options)
            // Use the server only for theme graphics.
            // For local files, i.e. anything that is not a single logical name, use local loading.
            && !isLocalContent(iconName)
            && format != "MNG"
            && format != "GIF"
            && manager) {

            // Initiate an IPC to themeserver to get the icon-data from the server.

            if (callback) {
                getIconFromServerAsync(params, callback, callbackParam);
                return 0;
            }

            icon = getIconFromServer(params);

            // No check for DoNotCache here. If we decided to use the server regardless of
            // the flag then there's a chance that we have to cache (in case of SgImage
            // for example) for proper operation, no matter what.
            if (icon) {
                cacheIcon(params, icon, &cacheKey);
                return icon;
            }

        }
#endif // Q_OS_SYMBIAN

        // Step 3: Finally fall back to loading icon locally in the client side
        if (callback) {
            loadLocalAsync(params, format, callback, callbackParam);
            return 0;
        }
        loadLocal(params, format);
    }

    icon = finishLocal(params);
    if (!params.animationCreated && icon && !options.testFlag(DoNotCache)) {
        cacheIcon(params, icon, &cacheKey);
    }

    if (callback) {
        callback(icon, callbackParam, true);
    }

#ifdef HB_ICON_TRACES
    qDebug() << "HbIconLoader::loadIcon END";
#endif

    return icon;
}

HbIconImpl *HbIconLoader::lookupInCache(const HbIconLoadingParams &params, QByteArray *outCacheKey)
{
    // Stop right away for resolution corrected icons, these may get false cache
    // hits. The use of such icons should be very rare anyway.
    if (params.options.testFlag(ResolutionCorrected)) {
        return 0;
    }

    QByteArray cacheKey = d->createCacheKeyFrom(params.iconName,
                                                params.size,
                                                params.aspectRatioMode,
                                                params.mode,
                                                params.mirrored,
                                                params.color);
    if (outCacheKey) {
        *outCacheKey = cacheKey;
    }
    if (d->iconImplCache.contains(cacheKey)) {
        HbIconImpl *icon = d->iconImplCache.value(cacheKey);
        icon->incrementRefCount();
        d->cacheKeeper.unref(icon);
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "HbIconLoader::lookupInCache: Cache hit for" << params.iconName << params.size
                 << "Client refcount now" << icon->refCount();
#endif
        return icon;
    }
    return 0;
}

void HbIconLoader::loadLocal(HbIconLoadingParams &params, const QString &format)
{
    QMutexLocker loadLocker(&d->mLocalLoadMutex);
    QMutexLocker iconSourceLocker(&d->mIconSourceMutex);
    if (format == "SVG") {
        loadSvgIcon(params);
    } else if(format == "NVG") {
        //support for client side rendering of nvg icons
        loadNvgIcon(params);
    } else if (format == "PIC") {
        loadPictureIcon(params);
    } else if (format == "MNG" || format == "GIF") {
        loadAnimatedIcon(params, format);
    } else {
        loadPixmapIcon(params, format);
    }
}

void HbIconLoader::loadLocalAsync(const HbIconLoadingParams &params, const QString &format,
                                  HbAsyncIconLoaderCallback callback, void *callbackParam)
{
    HbIconLoaderPrivate::AsyncParams *p = new HbIconLoaderPrivate::AsyncParams;
    p->mCallback = callback;
    p->mLdParams = params;
    p->mParam = callbackParam;
    d->mActiveAsyncRequests.append(p);
    QMetaObject::invokeMethod(d->mLocalLoader, "load",
                              Qt::QueuedConnection,
                              Q_ARG(HbIconLoadingParams, params),
                              Q_ARG(QString, format),
                              Q_ARG(void *, p));
}

HbIconImpl *HbIconLoader::finishLocal(HbIconLoadingParams &params)
{
    // This is called on the main thread so QPixmap can be used.
    QPixmap pm = QPixmap::fromImage(params.image);

    if (!params.mirroringHandled) {
        // Apply mirroring if required
        if (params.mirrored && !params.mirroredIconFound) {
            QTransform t;
            t.scale(-1, 1);
            pm = pm.transformed(t);
        }
    }

    if (params.color.isValid()) {
        if (!pm.isNull()) {
            QPixmap mask = pm.alphaChannel();
            pm.fill(params.color);
            pm.setAlphaChannel(mask);
        }
    }

    if (!params.modeHandled) {
        // Apply mode
        if (params.mode != QIcon::Normal) {
            QStyleOption opt(0);
            opt.palette = QApplication::palette();
            pm = QApplication::style()->generatedIconPixmap(params.mode, pm, &opt);
        }
    }

    return HbIconImplCreator::createIconImpl(pm, params);
}

void HbIconLoader::cacheIcon(const HbIconLoadingParams &params, HbIconImpl *icon, QByteArray *existingCacheKey)
{
    QByteArray cacheKey;
    if (existingCacheKey) {
        cacheKey = *existingCacheKey;
    } else {
        cacheKey = d->createCacheKeyFrom(params.iconName,
                                         params.size,
                                         params.aspectRatioMode,
                                         params.mode,
                                         params.mirrored,
                                         params.color);
    }
    d->addItemToCache(cacheKey, icon);

#ifdef HB_ICON_CACHE_DEBUG
    qDebug() << "HbIconLoader:cacheIcon: " << params.iconName
             << "inserted into local cache, client refcount now" << icon->refCount();
#endif
}

/*!
  Cancels an outstanding loadIcon request.

  \a callback must match the callback parameter of a previous loadIcon call.
  All loadIcon requests using the same callback will be canceled and the
  callback will not be invoked.

  Has no effect if no matching loadIcon request is active.

  If \a callbackParam is not 0 then it is used in the matching too so only
  loadIcon requests where both \a callback and \a callbackParam match will be
  canceled. If \a callbackParam is 0 then only \a callback is used in the
  matching.
 */
void HbIconLoader::cancelLoadIcon(HbAsyncIconLoaderCallback callback, void *callbackParam)
{
    // The callback used with the themeclient api is always the same, but the
    // AsyncParams pointer in the custom parameter is different for each request
    // so it can be used for identification.
    foreach (HbIconLoaderPrivate::AsyncParams *p, d->mActiveAsyncRequests) {
        if (p->mCallback == callback && (!callbackParam || callbackParam == p->mParam)) {
            // Cancel the remote request. It will have no effect in case of
            // local content. This is fine because for the local threaded loader
            // the removal from mActiveAsyncRequests is enough.
            HbThemeClient::global()->cancelGetSharedIconInfo(asyncCallback, p);
            d->mActiveAsyncRequests.removeOne(p);
            delete p;
            return;
        }
    }
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
HbIconImpl *HbIconLoader::loadMultiPieceIcon(const QStringList &listOfIcons,
        HbMultiPartSizeData &multiPartIconData,
        const QSizeF &size,
        Qt::AspectRatioMode aspectRatioMode,
        QIcon::Mode mode,
        IconLoaderOptions options,
        QVector<HbIconImpl *> &multiPieceImpls,
        const QColor &color)
{
    Q_UNUSED(color);
    Q_UNUSED(multiPieceImpls);

    HbIconImpl *icon = 0;
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
    QByteArray cacheKey = d->createCacheKeyFrom(
        multiPartIconData.multiPartIconId,
        size,
        aspectRatioMode,
        mode,
        mirrored,
        color);
    //If consolidated icon found in the client's cache, increment ref-count and return
    if (d->iconImplCache.contains(cacheKey)) {
        HbIconImpl *ptr = d->iconImplCache.value(cacheKey);
        ptr->incrementRefCount();
        d->cacheKeeper.unref(ptr);
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "HbIconLoader::loadMultiPieceIcon: Cache hit" << multiPartIconData.multiPartIconId
                 << size << "Client refcount now" << ptr->refCount();
#endif
        return ptr;
    }

    QStringList iconPathList;

    for (int i = 0; i < listOfIcons.count(); i++) {
        QString path = d->findSharedResourceHelper(listOfIcons.at(i), mirrored, mirroredIconFound);
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
               (mirrored && !mirroredIconFound), options, color, renderMode);

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

        // Creating HbIconImpl for the consolidated icon-data returned from themeserver.
        icon = HbIconImplCreator::createIconImpl(iconInfo, params);
        if (icon) {
            icon->setMultiPieceIcon();
        }
    } else {
        //Consolidated (stitched) icon could not be loaded on themeserver side, taking
        //fallback path for creating the consolidated icon on the client side.
        icon = createLocalConsolidatedIcon(multiPartIconData,
                                   iconPathList,
                                   size,
                                   aspectRatioMode,
                                   mode,
                                   options,
                                   color);
    }
    
    if (icon) {
        // Not yet in local cache (was checked before the server request) so insert.
        d->addItemToCache(cacheKey, icon);
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "HbIconLoader::loadMultiPieceIcon: " << multiPartIconData.multiPartIconId
                 << " inserted into local cache, client refcount now" << icon->refCount();
#endif
    }
    return icon;
}

HbIconImpl * HbIconLoader::createLocalConsolidatedIcon(const HbMultiPartSizeData &multiPartIconData,
                           const QStringList & iconPathList,
                           const QSizeF &consolidatedSize,
                           Qt::AspectRatioMode aspectRatioMode,
                           QIcon::Mode mode,
                           const IconLoaderOptions & options,
                           const QColor &color)
{
    // load the icons in to QImage
    HbIconLoadingParams params;
    params.purpose = HbIconLoader::AnyPurpose;
    params.aspectRatioMode = aspectRatioMode;
    params.mode = mode;
    params.color = color;
    params.animator = 0;
    params.mirrored = options.testFlag(HorizontallyMirrored);
    params.mirroredIconFound = false;
    params.mirroringHandled = false;
    params.modeHandled = false;
    params.renderMode = renderMode;
    params.canCache = false;
    params.animationCreated = false;

    QStringList::const_iterator iterFiles = iconPathList.begin();
    QStringList::const_iterator iterFilesEnd = iconPathList.end();

    QImage finalImage(consolidatedSize.toSize(), QImage::Format_ARGB32_Premultiplied);
    finalImage.fill(QColor(Qt::transparent).rgba());
    QPainter painter(&finalImage);

    for (int i=0; iterFiles != iterFilesEnd; ++iterFiles, ++i) {

        // Do not paint if image target rect is null
        if (!multiPartIconData.targets[i].isNull()) {

            // Populate icon loading parameters
            params.iconFileName = *iterFiles;
            params.size = multiPartIconData.pixmapSizes[i];
            params.options = options;
            params.isDefaultSize = params.size.isNull();

            QString format = formatFromPath(params.iconFileName);
            loadLocal(params, format);
            painter.drawImage(multiPartIconData.targets[i].topLeft(),
                                   params.image);
            params.image = QImage();

        }

    }
    painter.end();

    params.image = finalImage;

    return finishLocal(params);
}

inline int iconImplConsumption(HbIconImpl *icon)
{
    QSize sz = icon->keySize().toSize();
    return sz.width() * sz.height() * 4;
}

void HbIconLoaderPrivate::CacheKeeper::ref(HbIconImpl *icon)
{
    int consumption = iconImplConsumption(icon);
    // Never hold something more than once and do not ref anything when icons
    // are unloaded immediately when becoming hidden. Ignore also icons that are
    // too large and those which had not been inserted into iconImplCache
    // (e.g. icons that were created with the DoNotCache option).
    if (!mIcons.contains(icon)
        && !HbInstancePrivate::d_ptr()->mDropHiddenIconData
        && consumption < MAX_KEEPALIVE_ITEM_SIZE_BYTES
        && !mIconLoaderPrivate->iconImplCache.key(icon).isEmpty())
    {
        icon->incrementRefCount();
        mIcons.append(icon);
        mConsumption += consumption;
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "CacheKeeper::ref: Accepted" << icon->iconFileName()
                 << icon->keySize() << consumption
                 << "Total consumption now" << mConsumption;
#endif
        // Now do some housekeeping.
        while (mConsumption > MAX_KEEPALIVE_CACHE_SIZE_BYTES) {
            HbIconImpl *oldest = mIcons.first();
            unref(oldest);
            if (oldest->refCount() == 0 && oldest != icon) {
                del(oldest, true);
            }
        }
    }
}

void HbIconLoaderPrivate::CacheKeeper::unref(HbIconImpl *icon)
{
    if (mIcons.contains(icon)) {
#ifdef HB_ICON_CACHE_DEBUG
        qDebug() << "CacheKeeper::unref: Releasing" << icon->iconFileName() << icon->keySize();
#endif
        mIcons.removeOne(icon);
        mConsumption -= iconImplConsumption(icon);
        icon->decrementRefCount();
    }
}

void HbIconLoaderPrivate::CacheKeeper::clear()
{
    // Get rid of all unused icons in the iconimplcache, regardless of
    // the icons' rendering mode. Note that the list may contain non-sgimage
    // iconimpls too that are created on the server, and unlike sgimage the
    // unload request for these can never be skipped.
    QVector<HbThemeClient::IconReqInfo> unloadList;
    foreach (HbIconImpl *icon, mIcons) {
        icon->decrementRefCount();
        if (icon->refCount() == 0) {
            if (icon->isCreatedOnServer()) {
                unloadList.append(iconImplToReqInfo(icon));
            }
            del(icon, false); // no unload request, do it at once at the end
        }
    }
    mConsumption = 0;
    mIcons.clear();
    // Now do batch unloading, to reduce IPC.
    if (!unloadList.isEmpty()) {
        HbThemeClient::global()->batchUnloadIcon(unloadList);
    }
}

void HbIconLoaderPrivate::CacheKeeper::del(HbIconImpl *icon, bool sendUnloadReq)
{
    HbIconLoader::global()->removeItemInCache(icon);
    if (sendUnloadReq && icon->isCreatedOnServer()) {
        HbThemeClient::global()->unloadIcon(iconImplToReqInfo(icon));
    }
    icon->dispose();
}

// Initiates an IPC call to the ThemeServer to unload (or at least decrement the
// server-side refcount of) the icon, unless the icon unload was initiated by
// the server itself. Also, if the local refcount reaches zero, the icon is
// removed from the local cache.
void HbIconLoader::unLoadIcon(HbIconImpl *icon, bool unloadedByServer, bool noKeep)
{
    if (!icon) {
        return;
    }

    icon->decrementRefCount();

    if (icon->refCount() == 0) {
        if (!unloadedByServer) {
            // Offer the icon to the cacheKeeper first.
            if (!noKeep) {
                d->cacheKeeper.ref(icon);
            }
            // If it was accepted then the refcount was increased so stop here.
            if (icon->refCount() > 0) {
                return;
            }
            // Otherwise continue with unloading.
            if (icon->isCreatedOnServer()) {
                HbThemeClient::global()->unloadIcon(iconImplToReqInfo(icon));
            }
        }
        removeItemInCache(icon);
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
    foreach(HbIconImpl * impl, multiPieceImpls) {
        impl->decrementRefCount();
        if (impl->refCount() == 0) {
            if (d->iconImplCache.remove(d->iconImplCache.key(impl)) > 0) {
#ifdef HB_ICON_CACHE_DEBUG
                qDebug() << "HbIconLoader::unLoadMultiIcon: Removed from cache"
                         << impl->iconFileName() << impl->keySize();
#endif
            }
            if (impl->isCreatedOnServer()) {
                // List of icons to be unloaded.
                iconNameList << impl->iconFileName();
                sizeList << impl->keySize();
            }
        }
    }

    if (iconNameList.count() > 0) {
        HbThemeClient::global()->unLoadMultiIcon(iconNameList,
                sizeList,
                multiPieceImpls[0]->iconAspectRatioMode(),
                multiPieceImpls[0]->iconMode(),
                multiPieceImpls[0]->isMirrored(),
                multiPieceImpls[0]->color(),
                multiPieceImpls[0]->iconRenderingMode()
                                                );
    }
}

bool HbIconLoader::isInPrivateDirectory(const QString &filename)
{
    bool isPrivate = false;

#ifdef Q_OS_SYMBIAN
    if (filename.length() > 11) {
        // Private dir starts with e.g. "z:/private/"
        if (filename[1] == ':' && (filename[2] == '/' || filename[2] == '\\') &&
                (filename[10] == '/' || filename[10] == '\\') && filename.mid(3, 7).compare("private"), Qt::CaseInsensitive) {
            isPrivate = true;
        }
    }
#else
    Q_UNUSED(filename);
#endif

    return isPrivate;
}

void HbIconLoader::localLoadReady(const HbIconLoadingParams &loadParams, void *reqParams)
{
    // This code is running on the main thread.
    HbIconLoaderPrivate::AsyncParams *p = static_cast<HbIconLoaderPrivate::AsyncParams *>(reqParams);
    // Stop right away if canceled.
    if (!d->mActiveAsyncRequests.contains(p)) {
        return;
    }
    // Do not use d->mLdParams, it is not up-to-date.
    HbIconLoadingParams params = loadParams;
    HbIconImpl *icon = finishLocal(params);
    if (icon && !params.options.testFlag(DoNotCache)) {
        cacheIcon(params, icon, 0);
    }
    if (p->mCallback) {
        p->mCallback(icon, p->mParam, false);
    }
    d->mActiveAsyncRequests.removeOne(p);
    delete p;
}

void HbLocalIconLoader::load(const HbIconLoadingParams &params, const QString &format, void *reqParams)
{
    // This is running on a separate thread (d->mLocalLoaderThread) and since
    // the HbLocalIconLoader instance is moved to that thread, the requests from
    // the main thread are delivered using queued connection so slots/signals
    // provide easy inter-thread communication in this case.
    HbIconLoaderPrivate::AsyncParams *p = static_cast<HbIconLoaderPrivate::AsyncParams *>(reqParams);
    // Stop right away if canceled.
    if (!HbIconLoaderPrivate::global()->mActiveAsyncRequests.contains(p)) {
        return;
    }
    HbIconLoadingParams loadParams = params;
    HbIconLoader::global()->loadLocal(loadParams, format);
    emit ready(loadParams, reqParams);
}

void HbLocalIconLoader::doQuit()
{
    QThread::currentThread()->quit();
}


// End of File
