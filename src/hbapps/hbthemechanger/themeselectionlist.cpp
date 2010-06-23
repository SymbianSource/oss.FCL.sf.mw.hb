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
                        oldItemIndex(-1),
                        themelist(new HbListWidget(this)),
                        rightMark(new HbIcon(QString("qtg_small_tick"))),
                        noMark(new HbIcon(QString(""))),
                        previewView(0)
{
    mMainWindow = mainWindow;
    connect(themelist, SIGNAL(activated(HbListWidgetItem *)),this, SLOT(setChosen(HbListWidgetItem *)));
    connect(themelist, SIGNAL(longPressed(HbListWidgetItem*, QPointF)),
            this, SLOT(onLongPressed(HbListWidgetItem*, QPointF)));

    setWidget(themelist);

    // Automatic updation of the themelist when some theme is installed or uninstalled
    // when the hbthemechanger app is open
    watcher=new QFileSystemWatcher();
    foreach(const QString &KThemeRootPath, rootPaths()) {
        if(!KThemeRootPath.contains("/romthemes")){
        watcher->addPath(KThemeRootPath+"/themes/icons/");
        }
    }
    connect(watcher,SIGNAL(directoryChanged(const QString &)),this,SLOT(updateThemeList(const QString &)));
    QObject::connect(this,SIGNAL(newThemeSelected(QString)),this,SLOT(sendThemeName(QString)));
    grabGesture(Qt::TapAndHoldGesture);
#ifdef THEME_CHANGER_TIMER_LOG
    idleTimer = new QTimer(this);
    connect(idleTimer, SIGNAL(timeout()), this, SLOT(processWhenIdle()));
    connect(hbInstance->theme(),SIGNAL(changeFinished()), this, SLOT(themeChanged()));
    idleTimer->start(0); // to make a connection to server
#endif
}

/**
 * Destructor
 */
ThemeSelectionList::~ThemeSelectionList()
{
    // Set the theme to the applied theme before exiting.
    setChosen(themelist->item(oldItemIndex));
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
    bool entryAdded = false;
    bool themePresent = false;
    foreach(const QString &KThemeRootPath, rootPaths()){
        dir.setPath(KThemeRootPath) ;
        QStringList list = dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
        if(list.contains("themes",Qt::CaseInsensitive )) {
            themePresent = true;
            QDir root = KThemeRootPath;
            dir.setPath(root.path()+"/themes/icons/") ;
            QStringList iconthemeslist=dir.entryList(QDir::AllDirs|QDir::NoDotAndDotDot,QDir::Name);
            foreach(QString themefolder, iconthemeslist) {
                QDir iconThemePath(root.path()+"/themes/icons/"+themefolder);
                QFile themeIndexFile(root.path()+"/themes/"+themefolder+".themeindex");
                if(themeIndexFile.exists() && iconThemePath.exists("index.theme")) {
                    QSettings iniSetting(iconThemePath.path()+"/index.theme",QSettings::IniFormat);
                    iniSetting.beginGroup("Icon Theme");
                    QString hidden = iniSetting.value("Hidden").toString();
                    iniSetting.endGroup();
                    if((hidden == "true") ||( hidden == "")) {
                        iconthemeslist.removeOne(themefolder);
                    }
                }
                else {
                     iconthemeslist.removeOne(themefolder);
                }
            
            }
            if(!entryAdded){
                //adding one default entry
                HbListWidgetItem *item = new HbListWidgetItem();
                item->setText("hbdefault");
                item->setSecondaryText("hbdefault");
                QString thumbPath(":/themes/icons/hbdefault/scalable/qtg_graf_theme_preview_thumbnail.svg");                    
                if (!QFile::exists(thumbPath)) {
                    thumbPath = "qtg_large_corrupted";
                }
                HbIcon *icon = new HbIcon(thumbPath);
                thumbnails.append(icon);
                item->setIcon(*icon);                                
                if (HbInstance::instance()->theme()->name() == "hbdefault") {
                    item->setSecondaryIcon(*rightMark);
                    themelist->addItem(item);                
                    oldItemIndex=themelist->count()-1;
                    themelist->setCurrentRow(oldItemIndex);                    
                } else {
                    item->setSecondaryIcon(*noMark);
                    themelist->addItem(item);                
                }
                entryAdded = true;
            }
            list=iconthemeslist;
            for (int i=0; i <list.count();i++) {
                // populate theme list with existing themes
                HbListWidgetItem *item = new HbListWidgetItem();

                QSettings iniSetting(root.path()+"/themes/icons/"+list.at(i)+"/index.theme",QSettings::IniFormat);
                iniSetting.beginGroup("Icon Theme");
                QString name = iniSetting.value("Name").toString();
                iniSetting.endGroup();
                item->setText(name);

                item->setSecondaryText(root.path()+"/themes/icons/"+list.at(i));
                QString thumbPath(root.path()+"/themes/icons/"+list.at(i)+"/scalable/qtg_graf_theme_preview_thumbnail.");
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
                    oldItemIndex=themelist->count()-1;
                    themelist->setCurrentRow(oldItemIndex);
                }
                else {
                    item->setSecondaryIcon(*noMark);
                    themelist->addItem(item);
                }
            }
        }
    }
    //    else{//add a case for no theme ,make hbdefault entry
    if(!themePresent) {
            QStringList defaultList;
            defaultList.insert(0,"hbdefault"); //adding one default entry
            HbListWidgetItem *item = new HbListWidgetItem();
            item->setText(defaultList.at(0));
            item->setSecondaryText(defaultList.at(0));
            QString thumbPath(":/themes/icons/hbdefault/scalable/qtg_graf_theme_preview_thumbnail.svg");                    
            if (!QFile::exists(thumbPath)) {
                thumbPath = "qtg_large_corrupted";
            }
            HbIcon *icon = new HbIcon(thumbPath);
            thumbnails.append(icon);
            item->setIcon(*icon);            
            item->setSecondaryIcon(*rightMark);
            themelist->addItem(item);
            QString themeName=HbInstance::instance()->theme()->name();
            if (themeName != "hbdefault")
            {
                emit newThemeSelected("hbdefault");
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
    else
    {
        applySelection(); //double tap //put a tick
    }
}


/**
 * applySelection
 */
void ThemeSelectionList::applySelection()
{
    if(oldItemIndex!=themelist->currentRow()) {
        HbListWidgetItem *item = themelist->item(themelist->currentRow()); 
        item->setSecondaryIcon(*rightMark);
        if(oldItemIndex >= 0) {
            HbListWidgetItem *olditem = themelist->item(oldItemIndex); 
            olditem->setSecondaryIcon(*noMark);
        }
        oldItemIndex = themelist->currentRow();
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
    rootDirs << "c:/resource/hb"
             << "z:/resource/hb"
             << "e:/resource/hb"
             << "f:/resource/hb";
#else
    QString envDir = qgetenv("HB_THEMES_DIR");
    if (!envDir.isEmpty())
        rootDirs << envDir;
#endif
#if defined(Q_OS_MAC)
    rootDirs << QDir::homePath() + "/Library/UI Extensions for Mobile";
#elif !defined(Q_OS_SYMBIAN)
    rootDirs << HB_RESOURCES_DIR;
#endif
    return rootDirs;
}

void ThemeSelectionList::onLongPressed(HbListWidgetItem* listViewItem, const QPointF& coords)
{
    Q_UNUSED(coords);
    if (previewView) {
        delete previewView;
        previewView = 0;
    }
    previewItem = listViewItem;
    QString theme;
    if (listViewItem->secondaryText() != "hbdefault") {
        theme = listViewItem->secondaryText();
    } else {
        theme = ":/themes/icons/hbdefault";
    }
    previewView = new PreviewView(listViewItem->text(), theme, this);
    mMainWindow->addView(previewView);

    HbAction *act = new HbAction(tr("Ok"));
    connect(act, SIGNAL(triggered()), this, SLOT(applyTheme()));
    previewView->toolBar()->addAction(act);
    act = new HbAction(tr("Cancel"));
    connect(act, SIGNAL(triggered()), this, SLOT(cancelTheme()));
    previewView->toolBar()->addAction(act);

    mMainWindow->setCurrentView(previewView);
}

void ThemeSelectionList::cancelTheme()
{
    mMainWindow->removeView(mMainWindow->currentView());
}

void ThemeSelectionList::applyTheme()
{
    if (previewItem) {
        setChosen(previewItem);
        applySelection();
    }
    mMainWindow->removeView(mMainWindow->currentView());
}

#ifdef THEME_CHANGER_TIMER_LOG
void ThemeSelectionList::processWhenIdle()
{    
    qDebug() << "Theme changed applied in " << timer.elapsed() << " msec";
    idleTimer->stop();
    QThread::currentThread()->setPriority(QThread::NormalPriority);    
}

void ThemeSelectionList::themeChanged()
{
    idleTimer->start(0);
}
#endif //THEME_CHANGER_TIMER_LOG
