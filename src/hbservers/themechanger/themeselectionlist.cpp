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
#include <QSettings>
#include <QStringList>
#include <QDir>
#include <hbinstance.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbicon.h>
#include <QDebug>

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
                        themelist(new HbListView(this)),
                        model(new QStandardItemModel(this)),
                        rightMark(new HbIcon(QString("qtg_small_tick"))),
                        noMark(new HbIcon(QString(""))),
                        client(client)
{
    connect(themelist, SIGNAL(activated(const QModelIndex&)),this, SLOT(setChosen(const QModelIndex&)));
    setWidget(themelist);

    // Automatic updation of the themelist when some theme is installed or uninstalled
    // when the themechanger app is open
    watcher=new QFileSystemWatcher();
    foreach(const QString &KThemeRootPath, rootPaths()) {
        if(!KThemeRootPath.contains("/romthemes")){
        watcher->addPath(KThemeRootPath+"/themes/icons/");
        }
    }
    connect(watcher,SIGNAL(directoryChanged(const QString &)),this,SLOT(updateThemeList(const QString &)));
    QObject::connect(this,SIGNAL(newThemeSelected(QString)),this,SLOT(sendThemeName(QString)));    
}

/**
 * Destructor
 */
ThemeSelectionList::~ThemeSelectionList()
{
    // Set the theme to the applied theme before exiting.
    setChosen(oldItemIndex);
    delete noMark;

    noMark=NULL;
    delete rightMark;
    rightMark=NULL;

    // Remove all the items from model, then delete
    model->clear();
    delete model;
    model=NULL;

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
        if(list.contains("themes",Qt::CaseSensitive )) {
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
                // Items are populated to create the model
                QStandardItem *item=new QStandardItem(list.at(i));
                model->appendRow(item);
                if((HbInstance::instance()->theme()->name())==(list.at(i))) { 
                    item->setIcon(rightMark->qicon());
                    oldItemIndex=item->index();
                    themelist->setCurrentIndex(oldItemIndex,QItemSelectionModel::Select);
                }
                else {
                    item->setIcon(noMark->qicon());
                }
            }
        }
    }
    //    else{//add a case for no theme ,make hbdefault entry
    if(!themePresent) {
            QStringList defaultList;
            defaultList.insert(0,"hbdefault"); //adding one default entry
            QStandardItem *item=new QStandardItem(defaultList.at(0));
            model->appendRow(item);
            item->setIcon(rightMark->qicon());
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

    // Set the this model for the list 
    themelist->setModel(model);
}

/**
 * setChosen
 */
void ThemeSelectionList::setChosen(const QModelIndex &index)
{
     // Extract the string from the model index
    QVariant variant=index.data();
    QString str=variant.toString();

#ifdef THEME_CHANGER_TRACES
    qDebug() << "ThemeSelectionList::Setchosen with ThemeName: "<<str;
#endif
    if(iCurrentTheme != str ) {
        iCurrentTheme = str;
        if (!client->isConnected()) {
            bool success = client->connectToServer();
            if (success) {
                emit newThemeSelected(str); 
            }
        }
        else {
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
    QModelIndex currentItemIndex = (themelist)->currentIndex();
    if(oldItemIndex!=currentItemIndex) {
        (model->itemFromIndex(currentItemIndex))->setIcon(rightMark->qicon());
        if(oldItemIndex.isValid()) {
            (model->itemFromIndex(oldItemIndex))->setIcon(noMark->qicon());
        }
        oldItemIndex = currentItemIndex;
    }

}


/**
 * event
 */
bool ThemeSelectionList::event(QEvent *e)
{
    if((e->type()==QEvent::ShortcutOverride)||(e->type()==QEvent::WindowDeactivate)) {        
        themelist->setCurrentIndex(oldItemIndex,QItemSelectionModel::Select);
        themelist->setFocus();
        setChosen(oldItemIndex);
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
    model->clear();
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
             << "z:/resource/hb";
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
