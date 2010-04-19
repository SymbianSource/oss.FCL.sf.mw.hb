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

#include "hbstyleloader.h"
#include "hblayeredstyleloader_p.h"
#include "hbwidgetstyleloader_p.h"

/*!
	@beta
    @hbcore
    \class HbStyleLoader
    \brief allows an application to override platform layout and stylesheet definitions

    HbStyleLoader allows an application to register and unregister stylesheet files (CSS)
    and layout definition files (WidgetML). 

    Any stylesheet and layout definitions registered will be then be accessed by subsequent
    attempts to apply widget styles using HbStyle, (in cases where the style depends on the
    defined stylesheet or layout properties). So, once you have registered a stylesheet or
    a layout definition, you will need to cause widgets in your application to be repolished
    before they will detect the changes.

    For any stylesheet or layout definitions registered that correspond to similar properties
    defined by the platform, the properties registered by the application will take precedence.
    In case of CSS normal rules for stylesheet precedence apply. For example, selectors that
    are at least equally specific and define the same properties will override those equivalent
    platform property definitions.

    In order to remove stylesheets or layout definitions that were previously registered,
    the same filename should be provided to the corresponding unregister method. Again,
    a repolish is needed in order to detect the changes.

    Any stylesheets and layout definitions that remain registered will automatically be
    destroyed on application exit.
*/

/*!
    Register application specific WidgetML or CSS file, a directory,
    or a file path pattern.
    
    \a filePath can be
    - a single CSS file, where the file name must end with ".css", e.g. "myapplication.css", or
    - a single WidgetML, where the file name must end with "classname.widgetml", e.g. "mywidget.widgetml", or
    - a directory, when all files matching to above rules will be registered, or
    - a file path pattern, where the file path must contain (one or several occurances of) "%w". When
    searching for the style files the wildcard ("%w") will be replaced by classname and the file
    extensions will be added to the path. This happens for all the widgets existing in the application
    (even for the standard widgets) E.g. if the file path pattern is "myapp\%w\%w" the following
    files will be searched for: "myapp\mywidget\mywidget.css", "myapp\mywidget\mywidget.widgetml",
    "myapp\hbpushbutton\hbpushbutton.css", "myapp\hbpushbutton\hbpushbutton.widgetml", etc. Normally
    file path pattern is used only for platform style definition. Other kind of usage may cause 
    significant perfomrance hit.
    \sa unregisterFilePath
*/
bool HbStyleLoader::registerFilePath(const QString &filePath)
{
    return HbWidgetStyleLoader::instance()->addFilePath(
        filePath,
        HbLayeredStyleLoader::Concern_All,
        HbLayeredStyleLoader::Priority_Application);
}

/*!
    Register application specific WidgetML or CSS file, a directory,
    or a file path pattern.

    \a filePath should be the same as with registerFilePath
    \sa registerFilePath
*/
bool HbStyleLoader::unregisterFilePath(const QString &filePath)
{
    return HbWidgetStyleLoader::instance()->removeFilePath(
        filePath,
        HbLayeredStyleLoader::Concern_All,
        HbLayeredStyleLoader::Priority_Application);
}
