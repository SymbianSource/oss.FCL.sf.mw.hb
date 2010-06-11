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

#include "hbthetestwidget_p.h"
#include "hbthetestutility_p.h"
#include "hbinstance.h"
#include "hbnamespace_p.h"

#include <QGraphicsWidget>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QDate>
#include <QTimer>
#include <QFont>
#include <QObject>
#include <hbstyle.h>
#include <hbaction.h>

#include <QDebug> // for qWarning

#ifndef Q_OS_SYMBIAN
#include <iostream>
#endif

const QChar KComment('#');
const QString KDriveFPath("F:\\data\\log\\");
const QString KDriveEPath("E:\\data\\log\\");
const QString KDriveCPath("C:\\data\\log\\");
const QString KTheConfigureFile("TheTestUtility.txt"); // todo: proper name for configuration file


class HbTheTestUtilityPrivate
{
public:
    HbTheTestUtilityPrivate(HbMainWindow *mainWindow);
    bool checkConfigFile();
    bool doCheckConfigFile(const QString &path);
    QObject *receiver(const QString &name);
public:
    HbMainWindow *mMainWindow;
    HbTheTestWidget *mTestWidget;
    bool useE;
    bool useF;
    bool button1used;
    bool button2used;
    bool button3used;
    bool button4used;
};

HbTheTestUtilityPrivate::HbTheTestUtilityPrivate(HbMainWindow *mainWindow):
mMainWindow(mainWindow),
mTestWidget(0),
useE(false),
useF(false),
button1used(false),
button2used(false),
button3used(false),
button4used(false)
{
}

/*!
    Check the existence of configuration file.
    Test button is only created if configuration file can be found and
    there are at least one slot configured to any button.
    \internal
*/
bool HbTheTestUtilityPrivate::checkConfigFile()
{
#if defined (Q_OS_SYMBIAN)
    bool useC = false;
    useF = doCheckConfigFile(KDriveFPath);
    if (!useF) {
        useE = doCheckConfigFile(KDriveEPath);
        if (!useE) {
            useC = doCheckConfigFile(KDriveCPath);
        }
    }
    return (useC || useE || useF);
#elif defined (Q_OS_WIN32)
    return doCheckConfigFile(KDriveCPath);
#else
    return false; // only Symbian and Windows are supported
#endif
}

/*!
    Called from checkConfigFile()
    Does the actual configuration file check from given path.
    \internal
*/
bool HbTheTestUtilityPrivate::doCheckConfigFile(const QString &path)
{
    QString filePath = path;
    filePath = QDir::toNativeSeparators(filePath);

    QDir dir(filePath);
    if (!dir.exists()) {
        return false;
    }

    filePath.append(KTheConfigureFile);
    QFile file(filePath);
    if (!file.exists()) {
        return false;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
#ifndef Q_OS_SYMBIAN
        std::cerr << "Error: Cannot read from file " << qPrintable(file.fileName());
        std::cerr << qPrintable(filePath);
        std::cerr << qPrintable(file.errorString()) << std::endl;
#endif
        return false;
    }

    QTextStream configReader(&file);
    QString line;
    do {
        line = configReader.readLine();
        if (line.size() > 0) {
            if (line.at(0).digitValue() == 1) {
                button1used = true;
            } else if (line.at(0).digitValue() == 2) {
                button2used = true;
            } else if (line.at(0).digitValue() == 3) {
                button3used = true;
            } else if (line.at(0).digitValue() == 4) {
                button4used = true;
            }
        }
    } while (!line.isNull());

    if (button1used || button2used || button3used || button4used) {
        return true;
    }

    return false;
}



/*!
    Returns first QObject from mainwindow's scene or 0,
    if no object with given name found.
    \internal
*/

QObject *HbTheTestUtilityPrivate::receiver(const QString& name)
{
    QGraphicsScene* scene = mMainWindow->scene(); //krazy:exclude=qclasses
    QList<QGraphicsItem*> sceneItems = scene->items();
    foreach (QGraphicsItem* sceneItem, sceneItems ) {
        if (sceneItem->isWidget()) {
            QGraphicsWidget *obj = static_cast<QGraphicsWidget *>(sceneItem);
            // in 4.6 this code could be used
            //QGraphicsObject *obj = sceneItem->toGraphicsObject();
            //if (!obj) continue;
            QString className = QString::fromUtf8(obj->metaObject()->className());
            if (className == name) {
                return qobject_cast<QObject*>(obj);
            }
        }
    }

    return 0;
}


/*!
    \class HbTheTestUtility
    \internal
    \proto
*/

HbTheTestUtility::HbTheTestUtility(HbMainWindow *mainWindow)
{
    d = new HbTheTestUtilityPrivate(mainWindow);
    QTimer::singleShot(3000,this,SLOT(initialize())); // random 3 second delay
}

void HbTheTestUtility::initialize()
{
    if (d->checkConfigFile()) {
        d->mTestWidget = new HbTheTestWidget(d->mMainWindow);
        QObject::connect(d->mTestWidget->button1(), SIGNAL(clicked()),
            this, SLOT(button1clicked()));
        QObject::connect(d->mTestWidget->button2(), SIGNAL(clicked()),
            this, SLOT(button2clicked()));
        QObject::connect(d->mTestWidget->button3(), SIGNAL(clicked()),
            this, SLOT(button3clicked()));
        QObject::connect(d->mTestWidget->button4(), SIGNAL(clicked()),
            this, SLOT(button4clicked()));
        d->mTestWidget->setZValue(HbPrivate::PopupZValueRangeEnd+1);
        // hide buttons, that are not used
        if (!d->button1used) {
            d->mTestWidget->button1()->setVisible(false);
        }
        if (!d->button2used) {
            d->mTestWidget->button2()->setVisible(false);
        }
        if (!d->button3used) {
            d->mTestWidget->button3()->setVisible(false);
        }
        if (!d->button4used) {
            d->mTestWidget->button4()->setVisible(false);
        }
        d->mTestWidget->setLayoutDirection(Qt::LeftToRight);

        d->mMainWindow->scene()->addItem(d->mTestWidget);
    }
}

void HbTheTestUtility::button1clicked()
{
    invokeSlots(1);
}

void HbTheTestUtility::button2clicked()
{
    invokeSlots(2);
}

void HbTheTestUtility::button3clicked()
{
    invokeSlots(3);
}

void HbTheTestUtility::button4clicked()
{
    invokeSlots(4);
}

/*!
    Called, when TheTestButton is clicked.
    This method parses configuration file and calls all available slots.
    \internal
*/
void HbTheTestUtility::invokeSlots(const int button)
{
#if defined (Q_OS_SYMBIAN)
    QString filePath;
    if (d->useF) {
        filePath = "F:\\data\\log\\";
    } else if (d->useE) {
        filePath = "E:\\data\\log\\";
    } else {
        filePath = "C:\\data\\log\\";
    }
#elif defined (Q_OS_WIN)
    QString filePath("C:\\data\\log\\");
#else
    QString filePath(QDir::tempPath());
#endif

    filePath = QDir::toNativeSeparators(filePath);

    filePath.append(KTheConfigureFile);
    QFile file(filePath);
    if (!file.exists()) {
        return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
#ifndef Q_OS_SYMBIAN
        std::cerr << "Error: Cannot read from file " << qPrintable(file.fileName());
        std::cerr << qPrintable(filePath);
        std::cerr << qPrintable(file.errorString()) << std::endl;
#endif
        return;
    }

    QTextStream configReader(&file);
    QString line;
    do {
        line = configReader.readLine();
        if (line.isEmpty()) {
            continue;
        }
        // line not empty -> at least one character found
        if (line.at(0) == KComment) {
            continue;
        }
        if (line.at(0).digitValue() == button) {
            QString object = line.section(" ", 1, 1);
            QString method = line.section(" ", 2, 2);
            QObject *temp = d->receiver(object);
            if (temp) {
                QMetaObject::invokeMethod(temp, method.toLatin1(), Qt::DirectConnection);
            }
        }
    } while (!line.isNull());
}


HbTheTestUtility::~HbTheTestUtility()
{
    delete d;
}

