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
#include <hbinputmodeproperties.h>
#include <hbinputmethod.h>
#include <hbinputkeymapfactory.h>

#include "touchinputplugin.h"
#include "virtual12key.h"
#include "virtualqwerty.h"
#include "hbinputmodeproperties.h"

const QString HbTouchInput12KeyIcon("qtg_small_itut");
const QString HbTouchInputQwertyIcon("qtg_small_keyboard");

HbTouchInputPlugin::HbTouchInputPlugin(QObject *parent)
    : HbInputContextPlugin(parent)
{
}


HbTouchInputPlugin::~HbTouchInputPlugin()
{
}

/*!
\reimp
*/
QInputContext* HbTouchInputPlugin::create(const QString& key)
{
    if (key == QString("HbVirtual12Key")) {
        return new HbVirtual12Key();
    } else if (key == QString("HbVirtualQwerty")) {
        return new HbVirtualQwerty();
    } else {
        return 0;
    }
}

/*!
\reimp
*/
QString HbTouchInputPlugin::description(const QString& key)
{
    if (key == QString("HbVirtual12Key")) {
        return QString("Virtual phone keypad input");
    } else if (key == QString("HbVirtualQwerty")) {
        return QString("Virtual qwerty keyboard input");
    } else {
        return QString("");
    }
}

/*!
\reimp
*/
QString HbTouchInputPlugin::displayName(const QString& key)
{
    if (key == QString("HbVirtual12Key")) {
        return QString("Virtual 12-key");
    } else if (key == QString("HbVirtualQwerty")) {
        return QString("Virtual Qwerty");
    } else {
        return QString("");
    }
}

/*!
\reimp
*/
QStringList HbTouchInputPlugin::keys() const
{
    QStringList keys;
    keys.append(QString("HbVirtual12Key"));
    keys.append(QString("HbVirtualQwerty"));
    return keys;
}

/*!
\reimp
*/
QStringList HbTouchInputPlugin::languages(const QString& key)
{
    QStringList result;

    if (key == "HbVirtual12Key") {
        // Empty language means that any language that has key mappings matches.
        HbInputModeProperties properties(HbInputModeDefault, HbInputLanguage(), HbKeyboardVirtual12Key);        
        result.append(properties.asString());

        properties = HbInputModeProperties(HbInputModeNumeric, HbInputLanguage(), HbKeyboardVirtual12Key);
        result.append(properties.asString());
    } else if (key == "HbVirtualQwerty") {
        HbInputModeProperties properties(HbInputModeDefault, HbInputLanguage(), HbKeyboardVirtualQwerty);
        result.append(properties.asString());

        properties = HbInputModeProperties(HbInputModeNumeric, HbInputLanguage(), HbKeyboardVirtualQwerty);
        result.append(properties.asString());
    }

    return QStringList(result);
}

/*!
\reimp
*/
QStringList HbTouchInputPlugin::displayNames(const QString &key)
{
    Q_UNUSED(key);
    return QStringList();
}

/*!
\reimp
*/
HbIcon HbTouchInputPlugin::icon(const QString &key)
{
    if (key == "HbVirtual12Key") {
        return HbIcon(HbTouchInput12KeyIcon);
    } else if (key == "HbVirtualQwerty") {
        return HbIcon(HbTouchInputQwertyIcon);
    }

    return HbIcon();
}

/*!
\reimp
*/
QList<HbIcon> HbTouchInputPlugin::icons(const QString &key)
{
    Q_UNUSED(key);
    return QList<HbIcon>();
}

//
// Make plugin loadable.
//
Q_EXPORT_PLUGIN2(TouchInput, HbTouchInputPlugin)

// End of file
