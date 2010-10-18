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

#include "hbsplashgenerator_p.h"
#include "hbsplashdirs_p.h"
#include "hbsplashdefs_p.h"
#include "hbmainwindow.h"
#include "hbmainwindow_p.h"
#include "hbinstance.h"
#include "hbtheme.h"
#include "hbeffectinternal_p.h"
#include "hbview.h"
#include "hbdocumentloader.h"
#include "hbicon.h"
#include "hbaction.h"
#include "hbcolorscheme.h"
#include "hbstatusbar_p.h"
#include "hbstyle.h"
#include "hbbackgrounditem_p.h"
#include "hbframeitem.h"
#include <QCoreApplication>
#include <QPainter>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QTimer>

#if defined(Q_OS_SYMBIAN)
#include <f32file.h>
#include <coemain.h>
#endif

const char *last_theme_key = "lasttheme";
const char *last_lang_key = "lastlang";
const char *last_file_count_key = "lastfilecount";
const char *last_output_dir_key = "lastoutdir";

// Using invokeMethod with QueuedConnection is not good enough as it can
// cause starvation for Symbian active objects (at least in Qt versions
// before 4.7.2) so the theme change notification would get delayed until
// an entire regeneration cycle is finished, which is not acceptable. So
// use a QTimer with a small timeout instead. A small delay is beneficial
// anyway to divide the load a bit better so perf does not drop that much
// in apps while regeneration is on-going.
#define NEXT_STAGE(obj, func) QTimer::singleShot(10, this, SLOT(func()))

HbSplashGenerator::HbSplashGenerator()
    : mMainWindowLocked(false),
      mProcessQueuePending(false),
      mForceRegen(false),
      mMainWindow(0),
      mFirstRegenerate(true),
      mSaveSplFailed(false)
{
#if defined(Q_OS_SYMBIAN)
    CCoeEnv::Static()->FsSession().CreatePrivatePath(EDriveC);
    QString iniFileName = QString("c:/private/%1/hbsplashgen.ini")
                          .arg(QString::number(hbsplash_server_uid3.iUid, 16));
    mSettings = new QSettings(iniFileName, QSettings::IniFormat, this);
#else
    mSettings = new QSettings("Nokia", "Hb", this);
    mSettings->beginGroup("Splash");
#endif
    // Effects on decorators (started when they are shown) would ruin
    // the screenshot. So disable everything (except the orientation
    // switch effect which is needed for a proper rotated image).
    HbEffectInternal::setEffectsEnabled(false);
}

HbSplashGenerator::~HbSplashGenerator()
{
    delete mMainWindow;
    clearTranslators();
}

static QString orientationName(Qt::Orientation orientation)
{
    switch (orientation) {
    case Qt::Horizontal:
        return QLatin1String("lsc");
    case Qt::Vertical:
        return QLatin1String("prt");
    default:
        return QString();
    }
}

#define PRE "[hbsplashgenerator]"

static void log(const QString &msg, const QString &theme = QString(), int orientation = -1)
{
    const char *fmt = PRE " %s ('%s' '%s')";
    QString oriName = orientationName(static_cast<Qt::Orientation>(orientation));
    splDebug(fmt, qPrintable(msg), qPrintable(theme), qPrintable(oriName));
}

// To be called on startup and after each fully completed regeneration.
// Returns the number of files in the output directory.
int HbSplashGenerator::updateOutputDirContents(const QString &outDir)
{
    QDir dir(outDir);
    QStringList entries = dir.entryList(QDir::Files);
    emit outputDirContentsUpdated(outDir, entries);
    return entries.count();
}

void HbSplashGenerator::start(bool forceRegen)
{
    mForceRegen = forceRegen;
    // Let's have a certain delay for better load balancing during
    // boot. It is not strictly required, though.
    QTimer::singleShot(5000, this, SLOT(doStart()));
}

void HbSplashGenerator::doStart()
{
    splDeb() << PRE << "accessing theme";
    // Start listening to the theme-change-finished signal.
    HbTheme *theme = hbInstance->theme();
    connect(theme, SIGNAL(changeFinished()), SLOT(regenerate()));

    // Watch also the directories containing splashml files. Files may
    // be added/updated at any time.
    connect(&mFsWatcher, SIGNAL(directoryChanged(QString)), SLOT(onDirectoryChanged(QString)));
    foreach(const QString & dir, hbsplash_splashml_dirs()) {
        // Check for directory existence before calling addPath() to
        // avoid printing warnings.
        if (QDir(dir).exists()) {
            mFsWatcher.addPath(dir);
        }
    }

    // Regenerate screens, if needed.
    scheduleRegen();
}

void HbSplashGenerator::scheduleRegen()
{
    // Regenerate screens on startup only when the theme, the language, the
    // number of files in the splash screen directory, or the splash screen
    // directory path is different than the recorded values. (or when
    // regeneration is forced via command line arg)
    QString lastTheme = mSettings->value(QLatin1String(last_theme_key)).toString();
    QString lastLang = mSettings->value(QLatin1String(last_lang_key)).toString();
    int lastFileCount = mSettings->value(QLatin1String(last_file_count_key)).toInt();
    QString lastOutputDir = mSettings->value(QLatin1String(last_output_dir_key)).toString();
    QString currentTheme = hbInstance->theme()->name();
    QString currentLang = QLocale::system().name();
    QString currentOutputDir = hbsplash_output_dir();
    int currentFileCount = updateOutputDirContents(currentOutputDir);
    qDebug() << PRE << "last regen:" << lastTheme << lastLang << lastFileCount << lastOutputDir
             << "current:" << currentTheme << currentLang << currentFileCount << currentOutputDir;
    if (mForceRegen
            || currentFileCount == 0 // not having any files is wrong for sure
            || currentTheme != lastTheme
            || currentLang != lastLang
            || currentFileCount != lastFileCount
            || currentOutputDir != lastOutputDir) {
        NEXT_STAGE(this, regenerate);
        mForceRegen = false;
    }
}

void HbSplashGenerator::uncachedRegenerate()
{
    // Same as regenerate() but no caching is used so every file is
    // parsed again.
    mParsedSplashmls.clear();
    regenerate();
}

inline void delete_splash_screens()
{
    QDir outDir(hbsplash_output_dir());
    if (outDir.exists()) {
        QStringList names = outDir.entryList(QStringList() << "*", QDir::Files);
        foreach(const QString & name, names) {
            outDir.remove(name);
        }
    }
}

void HbSplashGenerator::regenerate()
{
    QString themeName = hbInstance->theme()->name();
    qDebug() << PRE << "regenerate() theme:" << themeName;
    if (!themeName.isEmpty()) {
        try {
            emit regenerateStarted();
            QTime queuePrepTime;
            queuePrepTime.start();
            // Delete existing splash screens. This is important because apps
            // should never pick up a screen with the previous theme or language
            // (not even when generating screens with the new theme fails). If
            // the generation of the new screens (at least the empty view) has
            // not finished when a new app is started then it is better to show
            // no splash screen at all.
            delete_splash_screens();
            // Clear the queue, generating screens with a non-current theme is
            // not possible anyway.
            mQueue.clear();
            // If this is the first invocation then put some requests for
            // screens we won't use. On certain platforms the very first
            // rendering (with a newly created mainwindow) may lead to
            // mysteriously scaled down output.
            if (mFirstRegenerate) {
                mFirstRegenerate = false;
                mQueue.enqueue(QueueItem(themeName, Qt::Vertical));
                mQueue.enqueue(QueueItem(themeName, Qt::Horizontal));
            }
            // Queue the screenshot request for both orientations.
            mQueue.enqueue(QueueItem(themeName, Qt::Vertical));
            mQueue.enqueue(QueueItem(themeName, Qt::Horizontal));
            queueAppSpecificItems(themeName, Qt::Vertical);
            queueAppSpecificItems(themeName, Qt::Horizontal);
            mSaveSplFailed = false;
            // If a previous regenerate is on-going, the mainwindow is locked
            // and we will not get to process the new queue entries before the
            // one screen, that is in progress, is done. However that one last
            // screen must not be rendered and written to file. To indicate
            // this, use a flag in the first queue entry.
            //
            // Note that removing the metacalls or timers is not an option as
            // that would lead to possibly leaving the mainwindow locked (and we
            // must never unlock it "manually" as it may be locked by some other
            // entity, e.g. the statusbar generator), so we have to let the last
            // screen to finish normally (excluding rendering and storage).
            mQueue.head().mFirstInRegen = true;
            splDeb() << PRE << "queue preparation time (ms):" << queuePrepTime.elapsed();
            NEXT_STAGE(this, processQueue);
        } catch (const std::bad_alloc &) {
            cleanup();
        }
    }
}

// This function is for the splashviewer tool only, do not use from elsewhere.
void HbSplashGenerator::regenerateOne(const QString &splashmlFileName, const QString &customTrDir)
{
    mQueue.clear();
    QueueItem item(hbInstance->theme()->name(), Qt::Vertical);
    QString path = QFileInfo(splashmlFileName).path();
    item.mCustomTrDirs.append(path);
    if (!customTrDir.isEmpty()) {
        item.mCustomTrDirs.append(customTrDir);
    }
    parseSplashml(splashmlFileName, item);
    item.mDocmlFileName = QDir(path).filePath(item.mDocmlFileName);
    mQueue.enqueue(item); // generate it regardless of the fixed orientation setting
    item.mOrientation = Qt::Horizontal;
    mQueue.enqueue(item);
    NEXT_STAGE(this, processQueue);
}

QImage HbSplashGenerator::renderView()
{
    log("renderView()", mItem.mThemeName, mItem.mOrientation);
    // Note: Do not use QPixmap::grabWidget() because our widget has never been
    // shown (it is not visible). QGraphicsView::render() seems to work better
    // in this respect. Also, let's use QImage because the stuff is going to a
    // file and is never drawn anywhere. This makes using graphics system
    // dependent things (e.g. some special filter effects) impossible but using
    // those for a splash screen would not be the best idea anyway.
    QTime t;
    t.start();
    // The image format must be the one that fits the OpenVG paint engine best.
    // (in order to avoid unnecessary conversions later when apps load and show
    // these images)
    QImage image(mMainWindow->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(Qt::transparent).rgba());
    QPainter painter(&image);
    mMainWindow->render(&painter);
    splDeb() << PRE << "rendering time (ms):" << t.elapsed();
    return image;
}

void HbSplashGenerator::processQueue()
{
    splDeb() << PRE << "processQueue()";
    // If the queue is empty then the splash regeneraton is complete so store
    // the current theme and language names as the last fully processed ones in
    // the settings and stop.
    if (mQueue.isEmpty()) {
        qDebug() << PRE << "queue is empty  regen finished";
        mSettings->setValue(last_theme_key, hbInstance->theme()->name());
        mSettings->setValue(last_lang_key, QLocale::system().name());
        QString outDir = hbsplash_output_dir();
        // Notify the server and get the number of generated files...
        int fileCount = updateOutputDirContents(outDir);
        // ...but store zero if some file writing failed at some point
        // so there will be a regeneration on next boot at least.
        if (mSaveSplFailed) {
            qWarning() << PRE << "some files not ok, ignoring file count";
            fileCount = 0;
            // Waiting until next boot is not always the best solution so try
            // again a bit later. This has to be limited, though, to prevent
            // continously flooding the system with regenerate requests in case
            // of an unusable drive. So retry only for a limited number of
            // times, if all else fails we will try again on next boot.
            static int retriesLeft = 3;
            if (retriesLeft-- > 0) {
                QTimer::singleShot(60000, this, SLOT(scheduleRegen())); // 1 min
            }
        } else {
            splDeb() << PRE << "all files ok";
        }
        mSettings->setValue(last_file_count_key, fileCount);
        mSettings->setValue(last_output_dir_key, outDir);
        emit finished();
        splDeb() << PRE << "processQueue() over";
        return;
    }
    // If a previous splash generation is still in progress or a compositor is
    // working then do nothing.
    if (!lockMainWindow()) {
        mProcessQueuePending = true;
        splDeb() << PRE << "still busy  processQueue() over";
        return;
    }
    try {
        mProcessQueuePending = false;
        mItem = mQueue.dequeue();
        mItemTime.start();
        log("generating splash screen", mItem.mThemeName, mItem.mOrientation);

        ensureMainWindow();
        mMainWindow->setOrientation(mItem.mOrientation, false);
        splDeb() << PRE << "mainwindow init time (ms):" << mItemTime.elapsed();

        QTime setupTime;
        setupTime.start();
        setupAppSpecificWindow();
        splDeb() << PRE << "content setup time (ms):" << setupTime.elapsed();

        // The async call chain goes like this:
        // processQueue -> finishWindow -> processWindow -> processQueue -> ...
        // finishWindow() cannot be called directly from here because that would
        // result in asserts in QGraphicsScene with certain Qt versions.
        NEXT_STAGE(this, finishWindow);

    } catch (const std::bad_alloc &) {
        cleanup();
    }
    splDeb() << PRE << "processQueue() over";
}

HbMainWindow *HbSplashGenerator::ensureMainWindow()
{
    if (!mMainWindow) {
        // The FixedVertical flag is used just to disable the sensor-based
        // orientation switching.
        mMainWindow = new HbMainWindow(0, Hb::WindowFlagFixedVertical);
        // Make sure that at least the 1st phase of the delayed
        // construction is done right now.
        HbMainWindowPrivate *mwd = HbMainWindowPrivate::d_ptr(mMainWindow);
        mwd->_q_delayedConstruction();
        mwd->mStatusBar->startClockTimer(); // may not start otherwise as it gets no visibility events
    }
    return mMainWindow;
}

void HbSplashGenerator::processWindow()
{
    // Take the screenshot, remove content, and move on to the next request in the queue.
    if (!newRegenPending()) {
        log("processWindow()  rendering splash screen", mItem.mThemeName, mItem.mOrientation);
        takeScreenshot();
        splDeb() << PRE << "total time for screen (ms):" << mItemTime.elapsed();
    }

    QList<HbView *> views = mMainWindow->views();
    foreach(HbView * view, views) {
        mMainWindow->removeView(view);
        delete view;
    }
    clearTranslators();

    unlockMainWindowInternal();
    NEXT_STAGE(this, processQueue);
    log("processWindow() over", mItem.mThemeName, mItem.mOrientation);
}

void HbSplashGenerator::takeScreenshot()
{
    log("takeScreenshot()", mItem.mThemeName, mItem.mOrientation);
    try {
        // Render the content. Note that this may use the wrong theme
        // graphics if there are quick theme changes on-going. We only
        // guarantee that there will always be a correct splash screen for
        // the currently set theme, and that can be fulfilled because even
        // though the screenshot we make here may be wrong in such a case
        // there will already be a new request queued due to the theme
        // change.
        QImage image = renderView();
        QTime t;
        t.start();
        QString splashFile = splashFileName();
        splDeb() << PRE << "saving to" << splashFile;
        if (saveSpl(splashFile, image, mItem.mFlagsToStore)) {
#if !defined(Q_OS_SYMBIAN) && defined(QT_DEBUG)
            image.save(splashFile + QLatin1String(".png"));
#endif
        } else {
            qWarning() << PRE << "file write failed for" << splashFile;
            mSaveSplFailed = true;
            // After setting the fail flag, clear the queue to stop processing
            // further screens because file writes would probably fail anyway.
            // Instead, processQueue() will schedule a retry at a later time.
            mQueue.clear();
        }
        splDeb() << PRE << "save time (ms):" << t.elapsed();
        log("takeScreenshot() over", mItem.mThemeName, mItem.mOrientation);
    } catch (const std::bad_alloc &) {
        cleanup();
    }
}

QString HbSplashGenerator::splashFileName()
{
    QString outDirName = hbsplash_output_dir();
    QDir dir(outDirName);
#ifdef Q_OS_SYMBIAN
    // Do not use QDir::mkpath() on Symbian. It is not able to create the
    // 'private' directory itself in case it does not exist (which is possible
    // during first boot because splashgen is started relatively early and the
    // eMMC may be totally empty at that point). RFs::MkDirAll() works better in
    // this respect.
    QString nativeOutPath = QDir::toNativeSeparators(outDirName);
    if (!nativeOutPath.endsWith('\\')) {
        nativeOutPath.append('\\');
    }
    TPtrC nativeOutPathDes(static_cast<const TUint16 *>(nativeOutPath.utf16()),
                           nativeOutPath.length());
    TInt err = CCoeEnv::Static()->FsSession().MkDirAll(nativeOutPathDes);
    if (err != KErrNone && err != KErrAlreadyExists) {
        qWarning() << PRE << "MkDirAll failed with" << err << "for" << nativeOutPath;
    }
#else
    if (!dir.exists()) {
        if (!QDir(".").mkpath(outDirName)) {
            qWarning() << PRE << "mkpath failed for" << outDirName;
        }
    }
#endif
    // "splash_<orientation>_<appid>_<screenid>"
    QString splashFile = dir.filePath("splash_");
    splashFile.append(orientationName(mItem.mOrientation));
    if (!mItem.mAppId.isEmpty()) {
        splashFile.append('_');
        splashFile.append(mItem.mAppId);
        if (!mItem.mScreenId.isEmpty()) {
            splashFile.append('_');
            splashFile.append(mItem.mScreenId);
        }
    }
    return splashFile;
}

bool HbSplashGenerator::saveSpl(const QString &nameWithoutExt, const QImage &image, quint32 extra)
{
    QString fn(nameWithoutExt);
    fn.append(".spl");
    QFile f(fn);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        quint32 w = (quint32) image.width();
        quint32 h = (quint32) image.height();
        quint32 bpl = (quint32) image.bytesPerLine();
        qint32 fmt = (qint32) image.format();
        f.write((char *) &w, sizeof(quint32));
        f.write((char *) &h, sizeof(quint32));
        f.write((char *) &bpl, sizeof(quint32));
        f.write((char *) &fmt, sizeof(qint32));
        f.write((char *) &extra, sizeof(quint32));
#ifdef HB_SPLASH_COMPRESSION
        QTime t;
        t.start();
        QByteArray compData = qCompress((const uchar *) image.bits(), bpl * h);
        quint32 len = compData.size();
        splDeb() << PRE << "compressed" << bpl * h << "to" << len << "in" << t.elapsed() << "ms";
        f.write((char *) &len, sizeof(quint32));
        qint64 wcount = f.write(compData.constData(), len);
#else
        quint32 len = 0; // 0 compressed length indicates uncompressed data
        f.write((char *) &len, sizeof(quint32));
        len = bpl * h;
        qint64 wcount = f.write((const char *) image.bits(), len);
#endif
        f.close();
        return wcount == len;
    }
    return false;
}

void HbSplashGenerator::cleanup()
{
    mQueue.clear();
    delete mMainWindow;
    mMainWindow = 0;
    clearTranslators();
    unlockMainWindowInternal();
    mProcessQueuePending = false;
}

QDebug operator<<(QDebug dbg, const HbSplashGenerator::QueueItem &item)
{
    dbg << "["
        << item.mDocmlFileName
        << item.mAppId
        << item.mScreenId
        << item.mDocmlWidgetName
        << item.mThemeName
        << orientationName(item.mOrientation)
        << item.mTsAppName
        << "]";
    return dbg;
}

HbSplashGenerator::QueueItem::QueueItem()
    : mOrientation(Qt::Vertical),
      mHideBackground(false),
      mFlagsToStore(0),
      mFirstInRegen(false)
{
}

HbSplashGenerator::QueueItem::QueueItem(const QString &themeName, Qt::Orientation orientation)
    : mThemeName(themeName),
      mOrientation(orientation),
      mHideBackground(false),
      mFlagsToStore(0),
      mFirstInRegen(false)
{
}

void HbSplashGenerator::addSplashmlItemToQueue(const QueueItem &item)
{
    bool fixedPortrait = item.mFixedOrientation == QLatin1String("portrait");
    bool fixedLandscape = item.mFixedOrientation == QLatin1String("landscape");
    if (fixedPortrait && item.mOrientation != Qt::Vertical) {
        return;
    }
    if (fixedLandscape && item.mOrientation != Qt::Horizontal) {
        return;
    }
    mQueue.enqueue(item);
}

void HbSplashGenerator::queueAppSpecificItems(const QString &themeName, Qt::Orientation orientation)
{
    QSet<QString> processedFileNames;
    QStringList dirNames(hbsplash_splashml_dirs());
    foreach(const QString & dirName, dirNames) {
        QDir dir(dirName);
        if (!dir.exists()) {
            continue;
        }
        QStringList entries = dir.entryList(QStringList() << "*.splashml", QDir::Files);
        foreach(const QString & entry, entries) {
            // Skip if a file with the same name has already been processed from
            // a different location.
            if (processedFileNames.contains(entry)) {
                splDeb() << PRE << "skipping splashml (already found at other location)" << dir.filePath(entry);
                continue;
            }
            processedFileNames.insert(entry);
            QString fullName = dir.filePath(entry);
            splDeb() << PRE << "parsing splashml" << fullName;
            if (mParsedSplashmls.contains(fullName)) {
                QueueItem item(mParsedSplashmls.value(fullName));
                item.mThemeName = themeName;
                item.mOrientation = orientation;
                splDeb() << PRE << "splashml already parsed  queuing request" << item;
                addSplashmlItemToQueue(item);
                continue;
            }
            QueueItem item(themeName, orientation);
            bool ok = parseSplashml(fullName, item);
            if (ok
                    && !item.mAppId.isEmpty()
                    && !item.mDocmlWidgetName.isEmpty()
                    && !item.mDocmlFileName.isEmpty()) {
                // Add the full path to the filename. The docml is supposed to
                // be in the same directory as the splashml.
                item.mDocmlFileName = dir.filePath(item.mDocmlFileName);
                splDeb() << PRE << "queuing request" << item;
                addSplashmlItemToQueue(item);
                mParsedSplashmls.insert(fullName, item);
            } else {
                qWarning() << PRE << "unable to parse" << fullName;
            }
        }
    }
}

inline void reportSplashmlError(const QString &fullFileName, int lineNumber, const QString &msg)
{
    qWarning("%s", qPrintable(QString(QLatin1String("%1 \"%2\":%3: %4"))
                              .arg(PRE).arg(fullFileName).arg(lineNumber).arg(msg)));
}

bool HbSplashGenerator::parseSplashml(const QString &fullFileName, QueueItem &item)
{
    QFile f(fullFileName);
    bool ok = f.open(QIODevice::ReadOnly);
    if (ok) {
        QXmlStreamReader xml(&f);
        bool docOk = false;
        while (!xml.atEnd()) {
            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::Invalid) {
                reportSplashmlError(fullFileName, xml.lineNumber(), xml.errorString());
                ok = false;
                break;
            } else if (token == QXmlStreamReader::StartElement
                       && xml.name() == QLatin1String("hbsplash")) {
                docOk = true;
            } else if (docOk) {
                parseSplashmlElements(xml, item, fullFileName);
            }
        }
        f.close();
    }
    return ok;
}

inline bool readBool(QXmlStreamReader &xml)
{
    QString text = xml.readElementText().trimmed();
    return text == QLatin1String("true") || text == QLatin1String("1");
}

void HbSplashGenerator::parseSplashmlElements(QXmlStreamReader &xml,
        QueueItem &item,
        const QString &fullFileName)
{
    if (xml.isStartElement()) {
        QStringRef name = xml.name();
        if (name == QLatin1String("docml")) {
            item.mDocmlFileName = xml.readElementText().trimmed();
        } else if (name == QLatin1String("widget")) {
            item.mDocmlWidgetName = xml.readElementText().trimmed();
        } else if (name == QLatin1String("appid") || name == QLatin1String("appuid")) {
            item.mAppId = xml.readElementText().trimmed();
            if (item.mAppId.startsWith(QLatin1String("0x"))) {
                item.mAppId.remove(0, 2);
            }
        } else if (name == QLatin1String("screenid")) {
            item.mScreenId = xml.readElementText().trimmed();
        } else if (name == QLatin1String("tsappname")) {
            item.mTsAppName = xml.readElementText().trimmed();
        } else if (name == QLatin1String("view-flags")) {
            item.mViewFlags = xml.readElementText().split(',', QString::SkipEmptyParts);
            for (int i = 0, ie = item.mViewFlags.count(); i != ie; ++i) {
                item.mViewFlags[i] = item.mViewFlags[i].trimmed().toLower();
            }
        } else if (name == QLatin1String("background-item-visible")) {
            item.mHideBackground = !readBool(xml);
        } else if (name == QLatin1String("navi-action-icon")) {
            item.mNaviActionIcon = xml.readElementText().trimmed();
        } else if (name == QLatin1String("background-brush-color")) {
            item.mBackgroundBrushColor = QColor(xml.readElementText().trimmed());
        } else if (name == QLatin1String("themed-background-brush-color")) {
            item.mThemedBackgroundBrushColor = xml.readElementText().trimmed();
        } else if (name == QLatin1String("background-image-name")) {
            QString whenToUse = xml.attributes().value("when").toString().trimmed();
            QString imageName = xml.readElementText().trimmed();
            if (whenToUse.isEmpty()) {
                whenToUse = QLatin1String("always");
            }
            item.mBackgroundImageName.insert(whenToUse, imageName);
        } else if (name == QLatin1String("use-section")) {
            QString whenToUse = xml.attributes().value("when").toString().trimmed();
            QString whichSection = xml.readElementText().trimmed();
            if (!whenToUse.isEmpty()) {
                item.mCondSections.insert(whenToUse, whichSection);
            } else {
                item.mForcedSections.append(whichSection);
            }
        } else if (name == QLatin1String("custom-widget-substitute")) {
            QString originalType = xml.attributes().value("for").toString().trimmed();
            QString substitutedType = xml.readElementText().trimmed();
            item.mCustomWidgetSubsts.insert(originalType, substitutedType);
        } else if (name == QLatin1String("fixed-orientation")) {
            item.mFixedOrientation = xml.readElementText().trimmed().toLower();
        } else if (name == QLatin1String("item-bg-graphics")) {
            QueueItem::ItemBgGraphicsRequest req;
            req.mTargetWidgetName = xml.attributes().value("for").toString().trimmed();
            QString type = xml.attributes().value("type").toString().trimmed();
            req.mFrameGraphicsType = HbFrameDrawer::Undefined;
            if (type == QLatin1String("1")) {
                req.mFrameGraphicsType = HbFrameDrawer::OnePiece;
            } else if (type == QLatin1String("3h")) {
                req.mFrameGraphicsType = HbFrameDrawer::ThreePiecesHorizontal;
            } else if (type == QLatin1String("3v")) {
                req.mFrameGraphicsType = HbFrameDrawer::ThreePiecesVertical;
            } else if (type == QLatin1String("9")) {
                req.mFrameGraphicsType = HbFrameDrawer::NinePieces;
            }
            QString z = xml.attributes().value("z").toString().trimmed();
            if (z.isEmpty()) {
                req.mZValue = -1;
            } else {
                req.mZValue = z.toFloat();
            }
            req.mOrientation = xml.attributes().value("when").toString().trimmed();
            req.mFrameGraphicsName = xml.readElementText().trimmed();
            if (!req.mTargetWidgetName.isEmpty() && !req.mFrameGraphicsName.isEmpty()) {
                item.mItemBgGraphics.append(req);
            }
        } else {
            reportSplashmlError(fullFileName, xml.lineNumber(),
                                QLatin1String("unknown element: ") + name.toString());
        }
    }
}

class CustomDocumentLoader : public HbDocumentLoader
{
public:
    CustomDocumentLoader(const HbMainWindow *window, const HbSplashGenerator::QueueItem &item)
        : HbDocumentLoader(window), mItem(item) { }
    QObject *createObject(const QString &type, const QString &name);
private:
    const HbSplashGenerator::QueueItem &mItem;
};

QObject *CustomDocumentLoader::createObject(const QString &type, const QString &name)
{
    QObject *obj = HbDocumentLoader::createObject(type, name);
    if (!obj) {
        splDeb() << PRE << "unsupported object" << type << name;
        // Cannot let parsing fail because of unknown custom widgets
        // so provide an empty HbWidget (or HbView if the splashml
        // prefers that).
        if (mItem.mCustomWidgetSubsts.contains(type)) {
            QString preferredType = mItem.mCustomWidgetSubsts.value(type);
            if (preferredType == QLatin1String("HbView")) {
                obj = new HbView;
            } else {
                qWarning() << PRE << "unsupported custom widget substitute type"
                           << type << "falling back to HbWidget";
            }
        }
        if (!obj) {
            obj = new HbWidget;
        }
        if (obj) {
            obj->setObjectName(name);
        }
    }
    return obj;
}

void HbSplashGenerator::setupAppSpecificWindow()
{
    // Check if the splash screen request is really application-specific.
    if (mItem.mDocmlFileName.isEmpty() || mItem.mAppId.isEmpty() || mItem.mDocmlWidgetName.isEmpty()) {
        return;
    }

    // Install translators if needed.
    addTranslator(QLatin1String("common"));
    if (!mItem.mTsAppName.isEmpty()) {
        addTranslator(mItem.mTsAppName);
    }

    // Parse the docml file and add the specified widget as a view.
    CustomDocumentLoader loader(mMainWindow, mItem);
    QStringList sections;
    if (!mItem.mCondSections.isEmpty()) {
        QLatin1String prtKey("portrait");
        QLatin1String lscKey("landscape");
        if (mItem.mCondSections.contains(prtKey) && mItem.mOrientation == Qt::Vertical) {
            sections << mItem.mCondSections.value(prtKey);
        } else if (mItem.mCondSections.contains(lscKey) && mItem.mOrientation == Qt::Horizontal) {
            sections << mItem.mCondSections.value(lscKey);
        }
    }
    sections << mItem.mForcedSections;
    splDeb() << PRE << "loading" << mItem.mDocmlFileName << "common section";
    bool ok;
    loader.load(mItem.mDocmlFileName, &ok);
    if (ok && !sections.isEmpty()) {
        foreach(const QString & section, sections) {
            splDeb() << PRE << "loading" << mItem.mDocmlFileName << "section" << section;
            loader.load(mItem.mDocmlFileName, section, &ok);
        }
    }
    if (ok) {
        // Apply child widget settings.
        setupNameBasedWidgetProps(loader);
        // Find the root view and add it to the mainwindow.
        QGraphicsWidget *widget = loader.findWidget(mItem.mDocmlWidgetName);
        if (widget) {
            splDeb() << PRE << "widget created from" << mItem;
            mMainWindow->addView(widget);
        } else {
            qWarning() << PRE << "widget creation failed from" << mItem;
        }
    } else {
        qWarning() << PRE << "unable to parse" << mItem.mDocmlFileName;
    }
}

void HbSplashGenerator::setupNameBasedWidgetProps(HbDocumentLoader &loader)
{
    // item-bg-graphics
    for (int i = 0, ie = mItem.mItemBgGraphics.count(); i != ie; ++i) {
        QueueItem::ItemBgGraphicsRequest req = mItem.mItemBgGraphics.at(i);
        if ((req.mOrientation == QLatin1String("portrait") && mItem.mOrientation != Qt::Vertical)
                || (req.mOrientation == QLatin1String("landscape") && mItem.mOrientation != Qt::Horizontal)) {
            continue;
        }
        HbWidget *widget = qobject_cast<HbWidget *>(loader.findWidget(req.mTargetWidgetName));
        if (widget) {
            splDeb() << PRE << "setting background item" << req.mFrameGraphicsName
                     << "for" << req.mTargetWidgetName;
            widget->setBackgroundItem(
                new HbFrameItem(req.mFrameGraphicsName, req.mFrameGraphicsType),
                (int) req.mZValue);
        }
    }
}

void HbSplashGenerator::finishWindow()
{
    QTime prepTime;
    prepTime.start();

    // There must be a view always in order to support view-specific settings.
    if (mMainWindow->views().isEmpty()) {
        mMainWindow->addView(new HbWidget);
    }

    QList<HbView *> views = mMainWindow->views();
    if (!views.isEmpty()) {
        HbView *view = views.at(0);

        // view-flags
        HbView::HbViewFlags viewFlags = view->viewFlags();
        if (mItem.mViewFlags.contains(QLatin1String("tb-minimizable"))) {
            viewFlags |= HbView::ViewTitleBarMinimizable;
        }
        if (mItem.mViewFlags.contains(QLatin1String("tb-minimized"))) {
            viewFlags |= HbView::ViewTitleBarMinimized;
        }
        if (mItem.mViewFlags.contains(QLatin1String("tb-hidden"))) {
            viewFlags |= HbView::ViewTitleBarHidden;
        }
        if (mItem.mViewFlags.contains(QLatin1String("tb-transparent"))) {
            viewFlags |= HbView::ViewTitleBarTransparent;
        }
        if (mItem.mViewFlags.contains(QLatin1String("tb-floating"))) {
            viewFlags |= HbView::ViewTitleBarFloating;
        }
        if (mItem.mViewFlags.contains(QLatin1String("sb-hidden"))) {
            viewFlags |= HbView::ViewStatusBarHidden;
        }
        if (mItem.mViewFlags.contains(QLatin1String("sb-transparent"))) {
            viewFlags |= HbView::ViewStatusBarTransparent;
        }
        if (mItem.mViewFlags.contains(QLatin1String("sb-floating"))) {
            viewFlags |= HbView::ViewStatusBarFloating;
        }
        view->setViewFlags(viewFlags);
        if (viewFlags.testFlag(HbView::ViewStatusBarHidden)
                || viewFlags.testFlag(HbView::ViewStatusBarTransparent)) {
            mItem.mFlagsToStore |= HbSplashNonStandardStatusBar;
        }

        // navi-action-icon
        if (!mItem.mNaviActionIcon.isEmpty()) {
            view->setNavigationAction(new HbAction(HbIcon(mItem.mNaviActionIcon), QString(), view));
        } // else will use the default navigation action (e.g. quit)
    }

    HbMainWindowPrivate *mwd = HbMainWindowPrivate::d_ptr(mMainWindow);
    // background-item-visible, background-brush-color, themed-background-brush-color
    if (mItem.mHideBackground) {
        mwd->removeBackgroundItem();
        QColor fillColor = mItem.mBackgroundBrushColor;
        if (!mItem.mThemedBackgroundBrushColor.isEmpty()) {
            QColor color = HbColorScheme::color(mItem.mThemedBackgroundBrushColor);
            if (color.isValid()) {
                fillColor = color;
            }
        }
        mMainWindow->scene()->setBackgroundBrush(fillColor.isValid() ? fillColor : Qt::black);
    } else {
        mwd->addBackgroundItem();
        mMainWindow->scene()->setBackgroundBrush(Qt::NoBrush);
        // background-image-name
        HbBackgroundItem *bgItem = mwd->mBgItem;
        if (bgItem) {
            QString backgroundImageName = mItem.mBackgroundImageName.value(
                                              mItem.mOrientation == Qt::Vertical ? "portrait" : "landscape");
            if (backgroundImageName.isEmpty()) {
                backgroundImageName = mItem.mBackgroundImageName.value("always");
                if (backgroundImageName.isEmpty()) {
                    backgroundImageName = bgItem->defaultImageName(mItem.mOrientation);
                }
            }
            mMainWindow->setBackgroundImageName(mItem.mOrientation, backgroundImageName);
        }
    }

    // Hide dynamic content from status bar (clock, indicators).
    setStatusBarElementsVisible(mMainWindow, false);

    splDeb() << PRE << "time spent in finishWindow() (ms):" << prepTime.elapsed();

    // Continue with rendering the graphics view in processWindow().
    NEXT_STAGE(this, processWindow);
}

void HbSplashGenerator::setStatusBarElementsVisible(HbMainWindow *mw, bool visible)
{
    HbMainWindowPrivate *mwd = HbMainWindowPrivate::d_ptr(mw);
    HbStatusBar *statusBar = mwd->mStatusBar;
    if (statusBar) {
        foreach(QGraphicsItem * item, statusBar->childItems()) {
            QString name = HbStyle::itemName(item);
            bool knownItem =
                name == QLatin1String("signal")
                || name == QLatin1String("battery")
                || name == QLatin1String("notificationindicators")
                || name == QLatin1String("settingsindicators")
                || name == QLatin1String("timetext");
            if (knownItem) {
                item->setVisible(visible);
            }
        }
    }
}

void HbSplashGenerator::addTranslator(const QString &name)
{
    QString lang = QLocale::system().name();
    QTranslator *translator = new QTranslator;
    bool ok = false;
    QStringList dirNames(hbsplash_translation_dirs());
    dirNames.append(mItem.mCustomTrDirs);
    foreach(const QString & dirName, dirNames) {
        QDir dir(dirName);
        QString fullName = dir.filePath(name + '_' + lang);
        // fullName is not necessarily an existing file, however the translator
        // may still pick up another suitable file based on this name.
        if (translator->load(fullName)) {
            QCoreApplication::installTranslator(translator);
            splDeb() << PRE << "translator installed:" << fullName;
            ok = true;
            break;
        }
    }
    if (ok) {
        mTranslators.append(translator);
    } else {
        qWarning() << PRE << "unable to find translations based on name" << name;
        delete translator;
    }
}

void HbSplashGenerator::clearTranslators()
{
    foreach(QTranslator * translator, mTranslators) {
        QCoreApplication::removeTranslator(translator);
    }
    qDeleteAll(mTranslators);
    mTranslators.clear();
}

void HbSplashGenerator::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path); // we are only watching directories containing splashml+docml
    mParsedSplashmls.clear();
    // Have some delay to avoid heavy system load in case of multiple
    // directory-changed notifications.
    QTimer::singleShot(1000, this, SLOT(regenerate()));
}

bool HbSplashGenerator::lockMainWindow()
{
    if (!mMainWindowLocked) {
        mMainWindowLocked = true;
        return true;
    }
    return false;
}

void HbSplashGenerator::unlockMainWindowInternal()
{
    mMainWindowLocked = false;
}

void HbSplashGenerator::unlockMainWindow()
{
    // This version is used by the compositors. Besides resetting the flag it
    // also queues a call to processQueue() if needed.
    unlockMainWindowInternal();
    if (mProcessQueuePending) {
        NEXT_STAGE(this, processQueue);
    }
}

bool HbSplashGenerator::newRegenPending() const
{
    return mQueue.isEmpty() ? false : mQueue.head().mFirstInRegen;
}
