/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbUtils module of the UI Extensions for Mobile.
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

#include "hbdocumentloaderplugin.h"

/*!
    @stable
    @hbutils
    \class HbDocumentLoaderPlugin
    \brief HbDocumentLoaderPlugin is an abstract base class for custom widget plug-ins.

    The document loader can create all the widgets defined in the Hb library. Some applications
    may want to create custom widgets through their DocML. By implementing a 
    HbDocumentLoaderPlugin, the DocML can refer to custom widget.

    The plug-in API also allows the development tools to access the widget meta data.

    \sa HbDocumentLoader
*/

/*!
    \fn virtual QObject *HbDocumentLoaderPlugin::createObject(const QString& type, const QString &name) = 0

    Returns an object of \a type with given object \a name. Usually the implementation allocates the object.
    The other option is to return pre-allocated object.
    
    Returns 0 if the \a type is not supported.

    \param type - Class type of the requested object.

    \param name - Object name of the requested object.
*/


/*!
    \fn virtual QList<const QMetaObject *> HbDocumentLoaderPlugin::supportedObjects() = 0

    Returns the list of meta objects supported by the plug-in.
*/

// end of file



