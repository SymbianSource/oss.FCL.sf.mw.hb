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

#ifndef HBDNTXMLREADER_P_H
#define HBDNTXMLREADER_P_H

#include <QMap>
#include <QString>
#include <QApplication>
#include <QXmlDefaultHandler>


#ifndef HBDNTXMLREADER_STRINGS
#define HBDNTXMLREADER_STRINGS 

#define DirectoryLocalizerDirectoryLocazationStr "HbDirectoryLocalization"
#define DirectoryLocalizerPathStringsStr "pathStrings"
#define DirectoryLocalizerFullDirectoryPathsStr "fullDirectoryPaths"
#define DirectoryLocalizerNameStr "name"
#define DirectoryLocalizerPathStr "path"
#define DirectoryLocalizerTranslationStr "translation"
#define DirectoryLocalizerRootPathStr "rootPath"
#define DirectoryLocalizerLocalizedPathStr "localizedPath"
#define DirectoryLocalizerLocalizedNameStr "localizedName"
#define DirectoryLocalizerDirectorySeparatorX2 "\\"
#define DirectoryLocalizerDirectorySeparatorX1 "/"
#define DirectoryLocalizerPhoneMemDrive "c:"
#define DirectoryLocalizerRomDrive "z:"
#endif


class HbDNTXmlReader : public QXmlDefaultHandler
{
public:
    HbDNTXmlReader();
    ~HbDNTXmlReader();
    
    bool startDocument();
    bool startElement( const QString & namespaceURI,
                       const QString & localName,
                       const QString & qName,
                       const QXmlAttributes & atts);
    bool characters(const QString& text);
    bool endElement( const QString & namespaceURI,
                     const QString & localName,
                     const QString & qName );
    bool endDocument();

    QMap<QString,QString> getFullPaths();
private:
    QTranslator* trans;
    QMap<QString, QString>* fullPaths;
    QMap<QString, QString>* pathStrings;
    
    bool parseDirectoryPaths; 
    bool parsePathStrings;
    int elementNumber;
    bool validElement;
        
    QStringList elements;
    QString temp;
    bool twoDone;
};

#endif // HBDNTXMLREADER_P_H
