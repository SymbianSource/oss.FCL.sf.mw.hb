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
#include <QDebug>
#include <QTime>
#include <QThread>

#include "themeselectionlist.h"
#include "themechangerdefs.h"

/**
 * Constructor
 */
ThemeSelectionList::ThemeSelectionList(
#ifdef Q_OS_SYMBIAN
    ThemeClientSymbian* client
#else
    ThemeClientQt* client
#endif
                        ): 
                        oldItemIndex(-1),
                        themelist(new HbListWidget(this)),
                        rightMark(new HbIcon(QString("qtg_small_tick"))),
                        noMark(new HbIcon(QString(""))),
                        client(client)
{
    connect(themelist, SIGNAL(activated(HbListWidgetItem *)),this, SLOT(setChosen(HbListWidgetItem *)));
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
                if(iconThemePath.exists("index.theme")) {
                    QSettings iniSetting(iconThemePath.path()+"/index.theme",QSettings::IniFormat);
                    iniSetting.beginGroup("Icon Theme");
                    QString hidden = iniSetting.value("Hidden").toString();
                    QString name = iniSetting.value("Name").toString();
                    iniSetting.endGroup();
                    if((hidden == "true") ||( hidden == "")||(name!=themefolder) ) {
                        iconthemeslist.removeOne(themefolder);
                    }
                }
                else {
                     iconthemeslist.removeOne(themefolder);
                }
            
            }
            if(!entryAdded){
                iconthemeslist.insert(0,"hbdefault"); //adding one default entry
                entryAdded = true;
            }
            list=iconthemeslist;
            for (int i=0; i <list.count();i++) {
                // populate theme list with existing themes
                if((HbInstance::instance()->theme()->name())==(list.at(i))) { 
                    themelist->addItem(*rightMark,list.at(i));
                    oldItemIndex=themelist->count()-1;
                    themelist->setCurrentRow(oldItemIndex);
                }
                else {
                    themelist->addItem(*noMark,list.at(i));
                }
            }
        }
    }
    //    else{//add a case for no theme ,make hbdefault entry
    if(!themePresent) {
            QStringList defaultList;
            defaultList.insert(0,"hbdefault"); //adding one default entry
            themelist->addItem(*rightMark,defaultList.at(0));
            QString themeName=HbInstance::instance()->theme()->name();
            if (themeName != "hbdefault")
            {
                if (!client->isConnected()) {
                    bool success = client->connectToServer();
                    if (success) {
                        emit newThemeSelected("hbdefault");
                    }
                }
                else {
                    emit newThemeSelected("hbdefault");
                }
            }

        }
}

/**
 * setChosen
 */
void ThemeSelectionList::setChosen(HbListWidgetItem *item)
{
    QString str=item->text();

#ifdef THEME_CHANGER_TRACES
    qDebug() << "ThemeSelectionList::Setchosen with ThemeName: "<<str;
#endif
    if(iCurrentTheme != str ) {
#ifdef THEME_CHANGER_TIMER_LOG
        timer.start();
        qDebug() << "Selected theme: " << str;
#endif
        iCurrentTheme = str;
        if (!client->isConnected()) {
            bool success = client->connectToServer();
            if (success) {
                QThread::currentThread()->setPriority(QThread::HighPriority);
                emit newThemeSelected(str);
            }
        }
        else {
            QThread::currentThread()->setPriority(QThread::HighPriority);
            emit newThemeSelected(str); 
        }
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
        themelist->setIcon(themelist->currentRow(),*rightMark);
        if(oldItemIndex >= 0) {
            themelist->setIcon(oldItemIndex,*noMark);
        }
        oldItemIndex = themelist->currentRow();
    }
}


/**
 * event
 */
bool ThemeSelectionList::event(QEvent *e)
{
    if((e->type()==QEvent::ShortcutOverride)||(e->type()==QEvent::WindowDeactivate)) {        
        // save old applied theme
        themelist->setCurrentRow(oldItemIndex);
        themelist->setFocus();
        setChosen(themelist->item(oldItemIndex));
        return true;
    }
    return (HbView::event(e));
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
    client->changeTheme(name);
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

#ifdef THEME_CHANGER_TIMER_LOG
void ThemeSelectionList::processWhenIdle()
{    
    qDebug() << "Theme changed applied in " << timer.elapsed() << " msec";
    idleTimer->stop();
    QThread::currentThread()->setPriority(QThread::NormalPriority);    
    if (!client->isConnected()) {
        client->connectToServer();
    }
}

void ThemeSelectionList::themeChanged()
{
    idleTimer->start(0);
}
#endif //THEME_CHANGER_TIMER_LOG
