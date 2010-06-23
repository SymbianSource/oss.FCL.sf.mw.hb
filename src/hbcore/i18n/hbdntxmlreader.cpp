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

#include <QTranslator>
#if defined(Q_OS_SYMBIAN)
#include <e32base.h> 
#endif // Q_OS_SYMBIAN
#include "hbdntxmlreader_p.h"

#define DirectoryLocalizerFile ":/i18n/hbdirectorylocalizer.xml"
#define DirectoryLocalizerTranslationsFile "/resource/hbi18n/translations/directorylocalizer_"

/*!
    Constructor of class.  
*/ 
HbDNTXmlReader::HbDNTXmlReader()
{
	fullPaths = NULL;
	pathStrings = NULL;
	
	
	parseDirectoryPaths = false; 
	parsePathStrings = false;
	elementNumber = 0;
	validElement = false;
	twoDone = false;
    QLocale locale = QLocale::system();
    
    trans = new QTranslator();
	QString languageStr;
	languageStr.append(DirectoryLocalizerTranslationsFile);

	languageStr.append(locale.name());

// Support for checking both Z and C drives 
	
#if defined(Q_OS_SYMBIAN)
	bool succeed = false;
	succeed = trans->load("z:" + languageStr);
	if ( succeed == false ) {
		trans->load("c:" + languageStr);
	}
#else // Q_OS_SYMBIAN
	trans->load(languageStr);
#endif // Q_OS_SYMBIAN

	QCoreApplication::installTranslator(trans);
	validElement = false;
	
	QFile xmlFile(DirectoryLocalizerFile);

	pathStrings = new QMap<QString, QString>;
	fullPaths = new QMap<QString,QString>;

	if( xmlFile.exists() )
		{
			QXmlInputSource source( &xmlFile );
			QXmlSimpleReader reader; 
			reader.setContentHandler( this );  
			reader.parse( source ) ;
		}
	
}

/*!
    Destructor of class. 
*/ 
HbDNTXmlReader::~HbDNTXmlReader()
{
    delete fullPaths;
    delete pathStrings;
	QCoreApplication::removeTranslator(trans);
	delete trans;
}

/*!
    This function is needed by XML reader.  
     
    \return true 
*/ 
bool HbDNTXmlReader::startDocument()
{
    return true;
}

/*!
    This function is needed by XML reader.  
     
    \param qName element which will be readed
    \return true 
*/ 
bool HbDNTXmlReader::startElement( const QString & ,
				   const QString & ,
				   const QString & qName,
				   const QXmlAttributes & )
{
    if( qName == DirectoryLocalizerPathStringsStr ) {
    	parsePathStrings = true;  // set to parse Path 
    	parseDirectoryPaths = false;
    	elements.clear();
    	elementNumber = 0;  // Set current path element number as undefined
    	twoDone = false;
    } else if( qName == DirectoryLocalizerFullDirectoryPathsStr ) {
    	parsePathStrings = false;
    	parseDirectoryPaths = true;  // set to parse localized path
    	elements.clear();
    	elementNumber = 0;  // set current path element as undefined
    	twoDone = false;
    } else if( qName == DirectoryLocalizerNameStr ) {
    	elementNumber = 1;  // 
    	validElement = true;
    	twoDone = false;
    } else if( qName == DirectoryLocalizerPathStr ){
    	elementNumber = 2;
    	validElement = true;
    	twoDone = false;
    } else if( qName == DirectoryLocalizerTranslationStr){
    	elementNumber = 3;
    	validElement = true;	
    } else if( qName == DirectoryLocalizerRootPathStr ){
    	elementNumber = 1;
    	validElement = true;
    	twoDone = false;
    } else if( qName == DirectoryLocalizerLocalizedPathStr ){
    	elementNumber = 2;
    	validElement = true;
    	twoDone = false;
    } else if( qName == DirectoryLocalizerLocalizedNameStr ){
    	elementNumber = 3;
    	validElement = true;
    	twoDone = false;
    } else {
        elementNumber++;
    }
	
    return true;	
}

/*!
    This function is needed by XML reader.  
     
    \param text readed element
    \return true 
*/ 
bool HbDNTXmlReader::characters(const QString& text)
{
    QString t = text;
    QString x;
    QString y;
    QString localizedStr;
    QString path;
    char* charPtr(NULL); 
    QByteArray* ba(NULL);
    if(validElement && elementNumber == 3 ) {
        ba = new QByteArray(t.toAscii());
        charPtr = ba->data();
    }
    
    if( validElement ) {
		if(parsePathStrings) {
			switch(elementNumber) {
            case 1:  // DirectoryLocalizerNameStr     
                t=t.toLower();
                elements.insert(0,t);
                break;
            case 2:  // DirectoryLocalizerPathStr  
                t= t.replace(DirectoryLocalizerDirectorySeparatorX2,DirectoryLocalizerDirectorySeparatorX1);					
                t=t.toLower();
                if ( elements.count() > 1 ) {
                    elements.insert(1,t);
                } else { 
                    elements.append(t);
                }
                break;
            case 3:  // DirectoryLocalizerTranslationStr 			
                elements.insert(0,t);
                
                localizedStr = QCoreApplication::translate(0,charPtr);
                delete ba;
                
                if( localizedStr == t ){
					localizedStr = "";                
                }
                elements.append(localizedStr);
                break;
            default:
                elements.append(t); 
                break;
            }
            validElement = false;    	
		} else if(parseDirectoryPaths) {			
			switch(elementNumber) {
            case 1: // DirectoryLocalizerRootPathStr
                temp = t;
                break;
            case 2: // DirectoryLocalizerLocalizedPathStr							
                x = pathStrings->value(temp);
                y = pathStrings->value(t);
                    
                path.append( x );
                path.append( y );

                elements.insert(0,path);
                
                break;
            case 3: // DirectoryLocalizerLocalizedNameStr
                 x = pathStrings->value(t);
                 elements.insert(1, x);
                break;
            default:			
                break;
			}
			validElement = false;    				
		}
	}
    return true;
}

/*!
    This function is needed by XML reader.  
     
    \param qName element which was readed
    \return true 
*/ 
bool HbDNTXmlReader::endElement( const QString &,
        const QString &,
        const QString & qName )
{
	if( qName == DirectoryLocalizerPathStringsStr ) {
		parsePathStrings = false;
		parseDirectoryPaths = false;
		elements.clear();
		elementNumber = 0;
	} else if( qName == DirectoryLocalizerFullDirectoryPathsStr ) {
		parsePathStrings = false;
		
		parseDirectoryPaths = false;
		elements.clear();
		elementNumber = 0;
	}
	
	if( parsePathStrings && elements.count() > 1 ) {
        pathStrings->insertMulti(elements.at(0), elements.at(1));
        elements.clear();		
    } else if(parseDirectoryPaths && elements.count() > 1) {
		QString element1, element2;
		switch(elements.count()) {				
        case 2:					
            if(!twoDone){
                fullPaths->insertMulti(elements.at(0),elements.at(1));
                twoDone = true;
                elements.clear();
            }
            break;
        default:
            break;
		}
	}
	return true;
}

/*!
    This function is needed by XML reader.  
     
    \return true 
*/ 
bool HbDNTXmlReader::endDocument()
{
    return true;
}

/*!
    \return pointer to list of full paths 
*/ 
QMap<QString,QString> HbDNTXmlReader::getFullPaths()
{
	return *fullPaths;
}


