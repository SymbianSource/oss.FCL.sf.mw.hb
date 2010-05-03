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

#ifndef HBSPLASHGENERATOR_P_H
#define HBSPLASHGENERATOR_P_H

#include <QObject>
#include <QStringList>
#include <QQueue>
#include <QImage>
#include <QTime>
#include <QColor>
#include <QHash>
#include <QSettings>
#include <QDebug>
#include <QXmlStreamReader>
#include <QFileSystemWatcher>
#include "hbframedrawer.h"

QT_BEGIN_NAMESPACE
class QTranslator;
QT_END_NAMESPACE

class HbMainWindow;
class HbDocumentLoader;

class HbSplashGenerator : public QObject
{
    Q_OBJECT

public:
    HbSplashGenerator();
    ~HbSplashGenerator();

    void start(bool forceRegen);

signals:
    void outputDirContentsUpdated(const QString &dir, const QStringList &entries);

public slots:
    void regenerate();

private slots:
    void processQueue();
    void processWindow();
    void onDirectoryChanged(const QString &path);

public:
    struct QueueItem {
        QueueItem();
        QueueItem(const QString &themeName, Qt::Orientation orientation);
        QString mThemeName;
        Qt::Orientation mOrientation;
        QString mAppId;
        QString mScreenId;
        QString mDocmlFileName;
        QString mDocmlWidgetName;
        QString mTsAppName;
        QStringList mViewFlags;
        bool mHideBackground;
        QString mNaviActionIcon;
        QColor mBackgroundBrushColor;
        QString mThemedBackgroundBrushColor;
        QHash<QString, QString> mBackgroundImageName;
        QHash<QString, QString> mCondSections;
        QList<QString> mForcedSections;
        QHash<QString, QString> mCustomWidgetSubsts;
        QString mFixedOrientation;
        struct ItemBgGraphicsRequest {
            QString mTargetWidgetName;
            HbFrameDrawer::FrameType mFrameGraphicsType;
            QString mFrameGraphicsName;
            qreal mZValue;
            QString mOrientation;
        };
        QList<ItemBgGraphicsRequest> mItemBgGraphics;
    };

private:
    void takeScreenshot();
    void cleanup();
    QImage renderView();
    QString splashFileName();
    bool saveSpl(const QString &nameWithoutExt, const QImage &image);
    void addSplashmlItemToQueue(const QueueItem &item);
    void queueAppSpecificItems(const QString &themeName, Qt::Orientation orientation);
    void processSplashml(QXmlStreamReader &xml, QueueItem &item);
    void setupAppSpecificWindow();
    void setupNameBasedWidgetProps(HbDocumentLoader &loader);
    void finishWindow();
    void addTranslator(const QString &name);
    void clearTranslators();
    int updateOutputDirContents(const QString &outDir);

    bool mBusy;
    HbMainWindow *mMainWindow;
    QQueue<QueueItem> mQueue;
    QueueItem mItem;
    QList<QTranslator *> mTranslators;
    QTime mItemTime;
    bool mFirstRegenerate;
    QHash<QString, QueueItem> mParsedSplashmls;
    QSettings mSettings;
    QFileSystemWatcher mFsWatcher;
};

QDebug operator<<(QDebug dbg, const HbSplashGenerator::QueueItem& item);

#endif // HBSPLASHGENERATOR_P_H
