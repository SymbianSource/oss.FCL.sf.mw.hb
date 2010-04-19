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
#include "hardwareinputplugin.h"
//Required Qt Headers
#include <QStringList>
#include <QInputContextPlugin>

//User Includes
#include "hbhardware12key.h"
#include "hbhardwareqwerty.h"

// ---------------------------------------------------------------------------
// HbHardwareInputPlugin::HbHardwareInputPlugin
// 
// Constructs HbHardwareInputPlugin
// ---------------------------------------------------------------------------
//
HbHardwareInputPlugin::HbHardwareInputPlugin(QObject *parent)
: QInputContextPlugin(parent)
{
}

// ---------------------------------------------------------------------------
// HbHardwareInputPlugin::~HbHardwareInputPlugin
// 
// ---------------------------------------------------------------------------
//
HbHardwareInputPlugin::~HbHardwareInputPlugin()
{
}

// ---------------------------------------------------------------------------
// HW12KeyImpl::create
// 
// ---------------------------------------------------------------------------
//
QInputContext* HbHardwareInputPlugin::create(const QString& key)
{
	if (key == QString("HbHardware12key")) {
		return new HbHardware12key();
	} else if (key == QString("HbHardwareQwerty")) {
        return new HbHardwareQwerty();
	} else {
		return 0;
	}
}

// ---------------------------------------------------------------------------
// HbHardwareInputPlugin::description
// 
// ---------------------------------------------------------------------------
//
QString HbHardwareInputPlugin::description(const QString& key)
{	
	if (key == QString("HbHardware12key")) {
		return QString("HW phone keypad input");
    } else if (key == QString("HbHardwareQwerty")) {
        return QString("HW phone Qwerty keypad input");
	} else {
		return QString("");
	}
}

// ---------------------------------------------------------------------------
// HbHardwareInputPlugin::displayName
// 
// ---------------------------------------------------------------------------
//
QString HbHardwareInputPlugin::displayName(const QString& key)
{
	if (key == QString("HbHardware12key")) {
		return QString("HW 12-key");
    } else if (key == QString("HbHardwareQwerty")) {
        return QString("HW phone Qwerty keypad input");
	} else {
		return QString("");
	}
}

// ---------------------------------------------------------------------------
// HbHardwareInputPlugin::keys
// 
// ---------------------------------------------------------------------------
//
QStringList HbHardwareInputPlugin::keys() const
{
	QStringList keys;
	keys.append(QString("HbHardware12key"));
	keys.append(QString("HbHardwareQwerty"));
	return keys;
}

// ---------------------------------------------------------------------------
// HbHardwareInputPlugin::languages
// 
// ---------------------------------------------------------------------------
//
QStringList HbHardwareInputPlugin::languages(const QString& /*key*/)
{
	return QStringList();
}

//
// Make plugin loadable.
//
Q_EXPORT_PLUGIN2(HardwareInput, HbHardwareInputPlugin)

// End of file
