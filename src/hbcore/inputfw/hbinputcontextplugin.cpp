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

#include "hbinputcontextplugin.h"

/*!
@proto
@hbcore
\class HbInputContextPlugin
\brief Extends QInputContextPlugin to support HbInputMethod input method selection UI elements.

Sometimes an input method may want to display several options in input method selection dialog. 
This class extends QInputContextPlugin and provides a way to specify several display names instead of one.
Standard input method selection UI sidplays them all and notifies the HbInputMethod in question 
which one was selected. This class also makes it possible to asign an icon to each display name.
The icons are then displayed by the default input method selection dialog.
*/

HbInputContextPlugin::HbInputContextPlugin(QObject *parent) : QInputContextPlugin(parent)
{
}

/*!
\fn QStringList HbInputContextPlugin::displayNames(const QString &key)

Sometimes an input context may want to display more than one option in input method selection
dialog (or in similar mechanism). This method is a version of base class method displayName and returns a list of strings
instead of just one string. See input method selection widget for details how the default input method selection
dialog passes selected item to the input context.

\sa displayName
*/

/*!
\fn HbIcon HbInputContextPlugin::icon(const QString &key)

Returns input context specific icon. It may be shown for example in input method selection dialog.
*/

/*!
\fn QList<HbIcon> HbInputContextPlugin::icons(const QString &key)

In case input context has more than one display name, it may also want to asign individual icon to
those display names. This method returns a list of icons. It should return same number of items as displayNames method.

\sa displayNames
\sa icons
*/

// End of file

