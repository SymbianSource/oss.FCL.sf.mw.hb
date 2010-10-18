/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbApps module of the UI Extensions for Mobile.
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
#include <QSettings>
#include <QStringList>
#include <QDir>
#include <QTimer>
#include <hbinstance.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbicon.h>
#include <hblistwidgetitem.h>
#include <restricted/hbthemeservices_r.h>
#include <hblabel.h>
#include <hbiconitem.h>
#include <hbtoolbar.h>
#include <QDebug>
#include <QTime>
#include <QThread>
#include <QGesture>
#include <QGraphicsLinearLayout>

#include "themeselectionlist.h"
#include "themechangerdefs.h"
#include "settingsview.h"
#include "resourceview.h"

class PreviewView : public HbView
{
public:

    PreviewView(const QString &name, const QString &theme, QGraphicsItem* parent=0):
       HbView(parent), preview(0), themePath(theme)
    {
        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
        HbLabel *label = new HbLabel(tr("Apply theme %1?").arg(name));
        label->setFontSpec(HbFontSpec(HbFontSpec::Primary));
        label->setElideMode(Qt::ElideNone);
        layout->addItem(label);
        QString previewPath("");
        if (HbDeviceProfile::profile(this).orientation() == Qt::Horizontal) {
            previewPath = themePath + "/scalable/qtg_graf_theme_preview_lsc.";
        } else {
            previewPath = themePath + "/scalable/qtg_graf_theme_preview_prt.";
        }
        QString nvgPreview(previewPath + "nvg");
        QString svgPreview(previewPath + "svg");
        
        if (QFile::exists(nvgPreview)) {
            previewPath = nvgPreview;
        } else if (QFile::exists(svgPreview)) {
            previewPath = svgPreview;
        } else {
            previewPath = "qtg_large_corrupted";
        }
        preview = new HbIconItem(previewPath);
        preview->setAspectRatioMode(Qt::KeepAspectRatio);
        QSize s = HbDeviceProfile::profile(this).logicalSize();
        preview->setPreferredSize(s);
        layout->addItem(preview);
        this->setLayout(layout);
    }

    ~PreviewView()
    {
        delete preview;
    }

    void resizeEvent(QGraphicsSceneResizeEvent *event)
    {
        Q_UNUSED(event);
        if (preview) {
            QString previewPath("");
            if (HbDeviceProfile::profile(this).orientation() == Qt::Horizontal) {
                previewPath = themePath + "/scalable/qtg_graf_theme_preview_lsc.";
            } else {
                previewPath = themePath + "/scalable/qtg_graf_theme_preview_prt.";
            }
            QString nvgPreview(previewPath + "nvg");
            QString svgPreview(previewPath + "svg");
            
            if (QFile::exists(nvgPreview)) {
                previewPath = nvgPreview;
            } else if (QFile::exists(svgPreview)) {
                previewPath = svgPreview;
            } else {
                previewPath = "qtg_large_corrupted";
            }

            preview->setIconName(previewPath);
            preview->setAspectRatioMode(Qt::KeepAspectRatio);
            QSize s = HbDeviceProfile::profile(this).logicalSize();
            preview->setPreferredSize(s);
        }
    }
private:
    HbIconItem *preview;
    QString themePath;
};

/**
 * Constructor
 */
ThemeSelectionList::ThemeSelectionList(HbMainWindow *mainWindow):
                        themelist(new HbListWidget(this)),
                        rightMark(new HbIcon(QString("qtg_small_tick"))),
                        noMark(new HbIcon(QString(""))),
                        previewView(0),
                        settingsView(0),
                        resourceView(0)
{
    mMainWindow = mainWindow;
    connect(themelist, SIGNAL(activated(HbListWidgetItem *)),this, SLOT(setChosen(HbListWidgetItem *)));
    connect(themelist, SIGNAL(longPressed(HbListWidgetItem*, QPointF)),
            this, SLOT(onLongPressed(HbListWidgetItem*, QPointF)));

    setWidget(themelist);

    // Automatic updation of the themelist when some theme is installed or uninstalled
    // when the hbthemechanger app is open
    watcher=new QFileSystemWatcher();
    foreach(const QString &themePath, themePaths()) {
        watcher->addPath(themePath);
    }
    connect(watcher,SIGNAL(directoryChanged(const QString &)),this,SLOT(updateThemeList(const QString &)));
    QObject::connect(this,SIGNAL(newThemeSelected(QString)),this,SLOT(sendThemeName(QString)));
    grabGesture(Qt::TapAndHoldGesture);

    HbAction *a = this->toolBar()->addAction(HbIcon("qtg_mono_settings"),tr("Settings"));
    connect(a, SIGNAL(triggered()), this, SLOT(showSettingsView()));

    a = this->toolBar()->addAction(HbIcon("qtg_mono_search"),tr("Resources"));
    connect(a, SIGNAL(triggered()), this, SLOT(showResourceView()));

    connect(hbInstance->theme(),SIGNAL(changeFinished()), this, SLOT(themeChanged()));

#ifdef THEME_CHANGER_TIMER_LOG
    idleTimer = new QTimer(this);
    connect(idleTimer, SIGNAL(timeout()), this, SLOT(processWhenIdle()));
    idleTimer->start(0); // to make a connection to server
#endif
}

/**
 * Destructor
 */
ThemeSelectionList::~ThemeSelectionList()
{
    // Set the theme to the applied theme before exiting.
    delete noMark;
    noMark=NULL;
    delete rightMark;
    rightMark=NULL;

    // Reset the item view
    themelist->reset();
    delete themelist;
    themelist=NULL;
    
    // Delete preview thubnails
    qDeleteAll(thumbnails);
    thumbnails.clear();
}


/**
 * displayThemes
 */
void ThemeSelectionList::displayThemes()
{
    QList<QPair<QString, QString> > themes;

    // First add default entry
    themes.append(QPair<QString,QString>("hbdefault", ":/themes/icons/hbdefault"));
    // And then rest of the themes
    themes.append(HbThemeServices::availableThemes());

    for (int i=0; i <themes.count();i++) {
        // populate theme list with existing themes
        HbListWidgetItem *item = new HbListWidgetItem();
        item->setText(themes.at(i).first);
        item->setSecondaryText(themes.at(i).second);

        QString thumbPath(themes.at(i).second+"/scalable/qtg_graf_theme_preview_thumbnail.");
        QString nvgPath(thumbPath + "nvg");
        QString svgPath(thumbPath + "svg");
        if (QFile::exists(nvgPath)) {
            thumbPath = nvgPath;
        } else if (QFile::exists(svgPath)) {
            thumbPath = svgPath;
        }
        else {
            thumbPath = "qtg_large_corrupted";
        }
        HbIcon *icon = new HbIcon(thumbPath);
        thumbnails.append(icon);
        item->setIcon(*icon);


        if (QFileInfo(HbThemeServices::themePath()) == QFileInfo(item->secondaryText())) {
            item->setSecondaryIcon(*rightMark);
            themelist->addItem(item);
            themelist->setCurrentRow(themelist->count()-1);
        }
        else {
            item->setSecondaryIcon(*noMark);
            themelist->addItem(item);
        }
    }
}

/**
 * setChosen
 */
void ThemeSelectionList::setChosen(HbListWidgetItem *item)
{
#ifdef THEME_CHANGER_TRACES
    qDebug() << "ThemeSelectionList::Setchosen with ThemeName: "<<item->secondaryText();
#endif
    if(iCurrentTheme != item->secondaryText()) {
#ifdef THEME_CHANGER_TIMER_LOG
        timer.start();
        qDebug() << "Selected theme: " << item->secondaryText();
#endif
        iCurrentTheme = item->secondaryText();
        QThread::currentThread()->setPriority(QThread::HighPriority);
        emit newThemeSelected(item->secondaryText());
    }
}

/**
 * updateThemeList
 */
void ThemeSelectionList::updateThemeList(const QString &path)
{
    Q_UNUSED(path);
    themelist->clear();
    this->displayThemes();
}


/**
 * sendThemeName
 */
void ThemeSelectionList::sendThemeName(const QString& name)
{
    HbThemeServices::setTheme(name);
}

/**
 * \internal
 */
QStringList ThemeSelectionList::rootPaths()
{
    QStringList rootDirs;
#if defined(Q_OS_SYMBIAN)
    rootDirs << "z:/resource/hb/themes";
#else
    QString envDir =  QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR"));
    if (!envDir.isEmpty())
        rootDirs << envDir + "/themes";
#endif
    rootDirs << ":/themes";
    return rootDirs;
}

void ThemeSelectionList::updateRootThemes()
{
    QDir dir;
    mRootThemes.clear();
    foreach(const QString &rootPath, rootPaths()){
        dir.setPath(rootPath+"/icons");
        QStringList iconthemeslist = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
        foreach(const QString &themefolder, iconthemeslist) {
            QDir iconThemePath(dir.path()+'/'+themefolder);
            QFile themeIndexFile(rootPath+'/'+themefolder+".themeindex");
            if(themeIndexFile.exists() && iconThemePath.exists("index.theme")) {
                mRootThemes.append(iconThemePath.absolutePath());
            }
        }
    }
}

QStringList ThemeSelectionList::themePaths()
{
    QStringList themeDirs;
#if defined(Q_OS_SYMBIAN)
    themeDirs << "z:/resource/hb/themes"
             << "c:/resource/hb/themes"
             << "e:/resource/hb/themes"
             << "f:/resource/hb/themes";
#else
    QString envDir = QDir::fromNativeSeparators(qgetenv("HB_THEMES_DIR"));
    if (!envDir.isEmpty()) {
        themeDirs << envDir + "/themes";
    }
#endif
    return themeDirs;
}

void ThemeSelectionList::onLongPressed(HbListWidgetItem* listViewItem, const QPointF& coords)
{
    Q_UNUSED(coords);
    if (previewView) {
        delete previewView;
        previewView = 0;
    }
    previewItem = listViewItem;
    QString theme = listViewItem->secondaryText();
    previewView = new PreviewView(listViewItem->text(), theme, this);
    mMainWindow->addView(previewView);

    HbAction *act = new HbAction(tr("Ok"));
    connect(act, SIGNAL(triggered()), this, SLOT(applyTheme()));
    previewView->toolBar()->addAction(act);
    act = new HbAction(tr("Cancel"));
    connect(act, SIGNAL(triggered()), this, SLOT(cancel()));
    previewView->toolBar()->addAction(act);
    act = new HbAction(Hb::BackNaviAction);
    connect(act, SIGNAL(triggered()), this, SLOT(cancel()));
    previewView->setNavigationAction(act);

    mMainWindow->setCurrentView(previewView);
}

void ThemeSelectionList::cancel()
{
    mMainWindow->setCurrentView(this);
    mMainWindow->removeView(previewView);
}

void ThemeSelectionList::applyTheme()
{
    if (previewItem) {
        setChosen(previewItem);
    }
    mMainWindow->setCurrentView(this);
    mMainWindow->removeView(previewView);
}

void ThemeSelectionList::showSettingsView()
{
    if (!settingsView) {
        // update root themes list
        updateRootThemes();
        settingsView = new SettingsView(mRootThemes, mMainWindow, this);
    }
    mMainWindow->setCurrentView(settingsView);
}

void ThemeSelectionList::showResourceView()
{
    if (!resourceView) {
        resourceView = new ResourceView(mMainWindow, this);
    }
    mMainWindow->setCurrentView(resourceView);
}


void ThemeSelectionList::themeChanged()
{
    QString currentTheme = HbThemeServices::themePath();
    for (int i=0;i<themelist->count();i++) {
        HbListWidgetItem *item = themelist->item(i);
        if (item->secondaryText() == currentTheme) {
            item->setSecondaryIcon(*rightMark);
        } else {
            item->setSecondaryIcon(*noMark);
        }
    }

#ifdef THEME_CHANGER_TIMER_LOG
    idleTimer->start(0);
#endif //THEME_CHANGER_TIMER_LOG
}

#ifdef THEME_CHANGER_TIMER_LOG
void ThemeSelectionList::processWhenIdle()
{    
    qDebug() << "Theme changed applied in " << timer.elapsed() << " msec";
    idleTimer->stop();
    QThread::currentThread()->setPriority(QThread::NormalPriority);    
}
#endif //THEME_CHANGER_TIMER_LOG
