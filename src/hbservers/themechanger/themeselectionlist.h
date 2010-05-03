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
#ifndef THEMESELECTIONLIST_H
#define THEMESELECTIONLIST_H

#include <QDir>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QFileSystemWatcher>
#include <QTime>
#include <hbview.h>
#include <hblistwidget.h>

#ifdef Q_OS_SYMBIAN
#include "themeclientsymbian.h"
#else
#include "themeclientqt.h"
#endif
#include "themechangerdefs.h"

class HbIcon;

class ThemeSelectionList:public HbView
{
Q_OBJECT
public:

#ifdef Q_OS_SYMBIAN
    ThemeSelectionList(ThemeClientSymbian* client);
#else
    ThemeSelectionList(ThemeClientQt* client);
#endif

    ~ThemeSelectionList();
signals:
    void newThemeSelected(const QString &newthemepath);
public slots:
    void displayThemes();
    void setChosen(HbListWidgetItem *item);
    void applySelection();
    void updateThemeList(const QString &path);
    void sendThemeName(const QString& name);
#ifdef THEME_CHANGER_TIMER_LOG
    void processWhenIdle();
    void themeChanged();
#endif

protected:
    bool event(QEvent *e);
    void resizeEvent(QResizeEvent* event);
private:
    static QStringList rootPaths();
    QDir dir; 
    int oldItemIndex;
    HbListWidget *themelist;
    HbIcon* rightMark;
    HbIcon* noMark;
    HbAction *action;
#ifdef Q_OS_SYMBIAN
    ThemeClientSymbian* client;
#else
    ThemeClientQt* client;
#endif

    QFileSystemWatcher *watcher;
    QString iCurrentTheme;
#ifdef THEME_CHANGER_TIMER_LOG
    QTime timer;
    QTimer *idleTimer;
#endif
};
#endif //THEMESELECTIONLIST_H
