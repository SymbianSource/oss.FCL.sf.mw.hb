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

#include <QDir>
#include <QApplication>
#include <qglobal.h>

#include "hbdirectorynamelocalizer.h"
#include "hbdntxmlreader_p.h"
#include "hbdirectorynamelocalizer_p.h"

class HbDNTXmlReader;

/*!
    @stable
    @hbcore
    \class HbDirectoryNameLocalizer
    \brief HbDirectoryNameLocalizer is used for localising directory names.
    
    Localisation is done if directory path matches one of the (pre)defined paths in hbdirectorylocalizer.xml
    If no localisation is available for directory empty string is returned.
    
    Example:

    \snippet{unittest_hbdirectorynamelocalizer/unittest_hbdirectorynamelocalizer.cpp,1}
    
 */

/*!
    Constructor of HbDirectoryNameLocalizer.
    
    \attention Cross-Platform API
 */
HbDirectoryNameLocalizer::HbDirectoryNameLocalizer()
{
    HbDNTXmlReader* dirDataReader = new HbDNTXmlReader();
    d = new HbDirectoryNameLocalizerPrivate();
    
    d->localizedNames = dirDataReader->getFullPaths();
       
    delete dirDataReader;
}

/*
    Destructor
 */
HbDirectoryNameLocalizer::~HbDirectoryNameLocalizer()
{
    delete d;
}

/*!
    Translates the given source path to a localized string if possible.
    If localized version is not found returns an empty string.
    Should be only used for localizing directory names. 
    
    \attention Symbian specific API

    \param sourceText The path that is to be translated.
    
    \return Symbian - The translated string. If translation is not found returns
    an empty string. 
    \return other platforms - empty QString    
 */
QString HbDirectoryNameLocalizer::translate( QString& sourceText ) const
{
#if defined(Q_OS_SYMBIAN)
    if ( sourceText.length() == 0 ) {
        return "";
    }

    QString result;
    
    if ( d->localizedNames.count() > 0 ) {
        QString searchKey = sourceText;     

        searchKey = searchKey.replace("\\","/");
        searchKey = searchKey.replace("\\\\","/");
        searchKey = searchKey.toLower();

        result = d->localizedNames.value(searchKey);
        if ( result.isEmpty() && 
            !searchKey.startsWith(DirectoryLocalizerPhoneMemDrive) && 
            !searchKey.startsWith(DirectoryLocalizerRomDrive) &&
            searchKey.indexOf(':') > 0) {
            result = d->localizedNames.value(searchKey.remove(0,searchKey.indexOf(':')+1));
        }
    }

    return result;
#else
    Q_UNUSED(sourceText);
    return QString();
#endif
}
