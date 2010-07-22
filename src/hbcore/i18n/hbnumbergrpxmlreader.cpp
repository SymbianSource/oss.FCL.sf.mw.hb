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
#include "hbnumbergrpxmlreader_p.h"
#define NumberGroupingFile ":/i18n/hbngnormalnumbers.xml"

using namespace HbNumberGrpXmlReaderHelp;

/*!
    Constructor of class  
    
    \param localeId identifier for locale
*/ 
HbNumberGrpXmlReader::HbNumberGrpXmlReader( int localeId )
{
	locale.setNum(localeId);
	locale.insert(0, "C");
	found = false;
	done = false;
	phase = 0;
	decimal = "";
	pattern = ""; 
	group = "";

	QFile xmlFile(NumberGroupingFile);

	if ( xmlFile.exists() )	{
		QXmlInputSource source(&xmlFile);
		QXmlSimpleReader reader; 
		reader.setContentHandler(this);  
		reader.parse(source) ;
	}	
} 	

/*!
    \return pattern for grouping 
*/ 
QString HbNumberGrpXmlReader::getPattern()
{
	return pattern;
}

/*!
    \return character(s) for group 
*/ 
QString HbNumberGrpXmlReader::getGroup()
{
	return group;
}

/*!
    \return character(s) for decimal 
*/ 
QString HbNumberGrpXmlReader::getDecimal()
{
	return decimal;
}

/*!
    Destructor of class. 
*/ 
HbNumberGrpXmlReader::~HbNumberGrpXmlReader()
{

}

/*!
    This function is needed by XML reader.  
     
    \return true 
*/ 
bool HbNumberGrpXmlReader::startDocument()
{
    return true;
}

/*!
    This function is needed by XML reader.  
     
    \param qName element which will be readed
    \return true 
*/ 
bool HbNumberGrpXmlReader::startElement( const QString &,
				   const QString &,
				   const QString &qName,
				   const QXmlAttributes & )
{
	if ( done ) {
		return true;
	}

	if ( found ) {		
		if ( qName == DecimalStr ) {
			phase = 1;
		} else if ( qName == GroupStr ) {
			phase = 2;
		} else if ( qName == PatternStr ) {
			phase = 3;
		}
	} else if ( locale == qName ) {
		found = true;
	}
    return true;	
}

/*!
    This function is needed by XML reader.  
     
    \param text readed element
    \return true 
*/ 
bool HbNumberGrpXmlReader::characters( const QString &text )
{  
    if ( done ) {
        return true;
    }

    QString tmpText;

	// This little trick is needed because of limitation in XML reader
	// XML reader doesn't read space character corretly
    if ( text.at(0).toAscii() == 32 ) {  // " "
        tmpText = ' ';    
    } else {
        tmpText = text;
    }
    
    if ( found ) {
		if ( phase == 1 ) {  //handle Decimal
			decimal = tmpText;
		} else if ( phase == 2 ) { // handle group
			group = tmpText;
		} else if ( phase == 3 ) { // handle pattern
			 pattern = tmpText;
		} else {}
		
		phase = 0;
    }
    return true;	
}

/*!
    This function is needed by XML reader.  
     
    \param qName element which was readed
    \return true 
*/ 
bool HbNumberGrpXmlReader::endElement( const QString &,
        const QString &,
        const QString &qName )
{
	if ( locale == qName ) {
		done = true;
	}

	return true;
}

/*!
    This function is needed by XML reader.  
     
    \return true 
*/ 
bool HbNumberGrpXmlReader::endDocument()
{
    return true;
}


