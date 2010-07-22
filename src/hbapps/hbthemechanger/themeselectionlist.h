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
#ifndef THEMESELECTIONLIST_H
#define THEMESELECTIONLIST_H

#include <QDir>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QFileSystemWatcher>
#include <QTime>
#include <hbview.h>
#include <hblistwidget.h>

#include "themechangerdefs.h"

class HbIcon;

class ThemeSelectionList:public HbView
{
Q_OBJECT
public:

    ThemeSelectionList(HbMainWindow *mainWindow);
    ~ThemeSelectionList();
signals:
    void newThemeSelected(const QString &newthemepath);
public slots:
    void displayThemes();
    void setChosen(HbListWidgetItem *item);
    void onLongPressed(HbListWidgetItem* listViewItem, const QPointF& coords);
    void updateThemeList(const QString &path);
    void sendThemeName(const QString& name);
    void cancel();
    void applyTheme();
    void showSettingsView();
    void showResourceView();
    void themeChanged();
#ifdef THEME_CHANGER_TIMER_LOG
    void processWhenIdle();
#endif

protected:
    void resizeEvent(QResizeEvent* event);
private:
    static QStringList rootPaths();
    QStringList rootThemes;
    QDir dir; 
    HbListWidget *themelist;
    HbIcon* rightMark;
    HbIcon* noMark;
    HbAction *action;
    QList<HbIcon*> thumbnails;
    HbAction* mOkAction;
    HbAction* mCancelAction;
    HbMainWindow *mMainWindow;
    HbListWidgetItem* previewItem;
    HbView *previewView;
    HbView *settingsView;
    HbView *resourceView;

    QFileSystemWatcher *watcher;
    QString iCurrentTheme;
#ifdef THEME_CHANGER_TIMER_LOG
    QTime timer;
    QTimer *idleTimer;
#endif
};
#endif //THEMESELECTIONLIST_H
