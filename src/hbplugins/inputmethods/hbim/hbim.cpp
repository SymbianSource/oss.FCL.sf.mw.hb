/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbPlugins module of the UI Extensions for Mobile.
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

#include "hbim.h"

#include <private/hbmainwindow_p.h>
#include <hbinputmethod.h>
#include <hbinputmethod_p.h>

bool HbInputInitializer::mRecursive = false;
// ---------------------------------------------------------------------------
// HbInputInitializer::HbInputInitializer
// 
// Constructs HbInputInitializer
// ---------------------------------------------------------------------------
//
HbInputInitializer::HbInputInitializer(QObject *parent)
    : QInputContextPlugin(parent)
{
}

// ---------------------------------------------------------------------------
// HbInputInitializer::~HbInputInitializer
// 
// ---------------------------------------------------------------------------
//
HbInputInitializer::~HbInputInitializer()
{
}

// ---------------------------------------------------------------------------
// Virtual12KeyImpl::create
// 
// ---------------------------------------------------------------------------
//
QInputContext* HbInputInitializer::create(const QString& key)
{
    if (key == QString("hbim")) {
        // this function is called from Qt framework's QApplication::inputContext()
        // now if inside this function or any function which is called from this function.
        // calls QApplication::inputContext() it will result in infinite recursion.
        // to guard this we are using this with mRecursive.
        // also setting HbMainWindowPrivate::initializeInputs to false will avoid 
        // re-initialization of inputfw when HbMainWindow is launched.
        if (!mRecursive) {
            HbMainWindowPrivate::initializeInputs = false;
            mRecursive = true;
            HbInputMethod::initializeFramework(*qApp);
            QInputContext *ic = qApp->inputContext();
            mRecursive = false;
            return ic;
        } else {
            // it was a recursive call, so for clarity return 0 from here.
            // this function is called only when QApplicaion's inputContext is null.
            // so returning a null for a recursive call.
            return 0;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// HbInputInitializer::description
// 
// ---------------------------------------------------------------------------
//
QString HbInputInitializer::description(const QString& key)
{
    if (key == QString("hbim")) {
        return QString("Hb Input Initialization");
    } else {
        return QString("");
    }
}

// ---------------------------------------------------------------------------
// HbInputInitializer::displayName
// 
// ---------------------------------------------------------------------------
//
QString HbInputInitializer::displayName(const QString& key)
{
    if (key == QString("hbim")) {
        return QString("Hb Input Initialization");
    } else {
        return QString("");
    }
}

// ---------------------------------------------------------------------------
// HbInputInitializer::keys
// 
// ---------------------------------------------------------------------------
//
QStringList HbInputInitializer::keys() const
{
    QStringList keys;
    keys.append(QString("hbim"));
    return keys;
}

// ---------------------------------------------------------------------------
// HbInputInitializer::languages
// 
// ---------------------------------------------------------------------------
//
QStringList HbInputInitializer::languages(const QString& /*key*/)
{
    return QStringList();
}

//
// Make plugin loadable.
//
Q_EXPORT_PLUGIN2(OrbitInputInit, HbInputInitializer)

// End of file
