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

#include "hbngnormalnumber_p.h"
#include "hbnumbergrpxmlreader_p.h"

namespace NgNormalNumber {
	QHash<QLocale::Country, HbNgNormalNumber::CountryData> countryList;
}

/*!
    Function for general grouping.  
          
    \param number QString which contains numbers
    \param country locale for grouping
    \return grouped numbers
*/ 
QString HbNgNormalNumber::normalNumberGrouping( const QString &number, 
												const QLocale::Country &country )
{
	QString decimal;
	QString group;
	int firstNumber;
	int lastNumber;
	int dot;
	bool sign;
	QList<int> patternBlockSizes;
	
	// Checking received 'number' parameter...
	// Here we initialize variables...
	if ( HbNgNormalNumber::checkingNumber(number, firstNumber, lastNumber, dot, sign) == false ) {
	    return QString();
	}
	
	// Checking XML...
	// Here we initialize (more) variables...
	if ( HbNgNormalNumber::checkingXML(country, decimal, group, patternBlockSizes) == false ) {
	    return QString();
	}
		
	// Formating number group...
	// Here we use those variables...
	return HbNgNormalNumber::formatingGroup(number, firstNumber, lastNumber, dot, sign, decimal, group, patternBlockSizes);
}

/*!
    Verifies that number string is valid.  
          
    \param number QString which contains numbers
    \param firstNumber index of first number
    \param lastNumber index of last number
    \param dot index of dot
    \param sign boolean which tells is there sign used
    \return true if valid
*/ 
bool HbNgNormalNumber::checkingNumber( const QString &number, 
									   int &firstNumber,
									   int &lastNumber, 
									   int &dot,
									   bool &sign ) 
{
	firstNumber = -1;
	lastNumber = -1;
	dot = -1;
	sign = false;
	int index = 0;
	int nrSize = number.size();
	
	for ( index = 0; index < nrSize; index++ ) {
		switch ( number.at(index).toAscii() ) {
    	case 43: // "+"
    	case 45: // "-"
		    if ( index < number.size()-1 ) {
		         if ( index > 0 ) {
		         	// If sign (+/-) present then it should be a first or last (for arabic) character 
		         	return false;
		         } else {
		         	// Sign (+/-) is the first character
		            sign=true;
		         }
		    } else {
		    	if ( sign ) {
		        	// Sign (+/-) already exists as the first character
		    		// Sign cannot be in both ends same time
		            return false;
		   		} else {
		        	// Sign (+/-) is the last character
		            sign=true;
		       	}
		    }
    		break;
    	case 46: // "."
			if ( firstNumber == -1 ) {
				// We don't support '.x' format
				// At least zero should be a first character, example 0.x
				return false;
			}
			if ( dot != -1 ) {
				// We don't support multiple dots
				// Only one dot is allowed
				return false;
			}
			if ( index == (nrSize-2) ) {
				if ( number.at(index+1).toAscii() == 43) {
					// We don't support 'x.+' format
					// At least zero should be a second last character, example x.0+
					return false;
				}
				if ( number.at(index+1).toAscii() == 45) {
					// We don't support 'x.-' format
					// At least zero should be a second last character, example x.0-
					return false;
				}
			}
			if ( index == (nrSize-1) ) {
				// We don't support 'x.' format
				// At least zero should be a last character, example x.0
				return false;
			}
			dot = index;
			break;
    	case 48: // "0"
    	case 49: // "1"
    	case 50: // "2"
    	case 51: // "3"
    	case 52: // "4"
    	case 53: // "5"
    	case 54: // "6"
    	case 55: // "7"
    	case 56: // "8"
    	case 57: // "9"
    		if ( firstNumber == -1 ) {
    			firstNumber = index;
    		}
    		lastNumber = index;
    		break;
    	default:
    		// String contains unknown or invalid character(s)
    		return false;
		}
	}
	
	if ( firstNumber == -1 ) {
		// String doesn't contain any number for formating
    	return false;
	}
	
    return true;
}

/*!
    Reads locale data from XML if needed.  
          
    \param country locale for grouping
    \param decimal character(s) for decimal
    \param group character(s) for group separator
    \param patternBlockSizes list of blocks
    \return true if successful
*/ 
bool HbNgNormalNumber::checkingXML( const QLocale::Country &country, 
									QString &decimal, 
									QString &group, 
									QList<int> &patternBlockSizes )
{
	decimal = "";
	group = "";
	QString pattern = "";
	
	// Firstly we try to find needed data from the list...
	QHash<QLocale::Country, HbNgNormalNumber::CountryData>::iterator iter = NgNormalNumber::countryList.find(country);
	if ( iter == NgNormalNumber::countryList.end() ) { 
		// Data not found. So now we read XML file...
	    HbNumberGrpXmlReader numberGrpReader(country);
	    pattern = numberGrpReader.getPattern();
	    decimal = numberGrpReader.getDecimal();  
	    group = numberGrpReader.getGroup();
	    
	    if ( (pattern.isEmpty()) || (decimal.isEmpty()) ) {
	    	// XML doesn't contain all needed information or reading has failed
	    	return false;
	    }
	    	    
		// Checking pattern...
	    patternBlockSizes = HbNgNormalNumber::checkingPattern(pattern);
	    if ( patternBlockSizes.isEmpty() ) {
	    	// Pattern was invalid or corrupted
	    	return false;
	    }
	    
	    // Updating the list with new information... 
	    HbNgNormalNumber::CountryData tempCountryData;
	    tempCountryData.pattern = pattern;
	    tempCountryData.decimal = decimal;
	    tempCountryData.group = group;
	    tempCountryData.patternBlockSizes = patternBlockSizes;
	    NgNormalNumber::countryList.insert(country, tempCountryData);  
	} else { 
		// Data found
	    HbNgNormalNumber::CountryData tempCountryData2 = iter.value();
	    pattern = tempCountryData2.pattern;
	    decimal = tempCountryData2.decimal;
	    group = tempCountryData2.group;
	    patternBlockSizes = tempCountryData2.patternBlockSizes;
	}

    return true;
}

/*!
    Verifies that pattern for gouping is valid.  
          
    \param pattern rules for grouping
    \return size of (number) block
*/ 
QList<int> HbNgNormalNumber::checkingPattern( const QString &pattern )
{
	int index = 0;
	int comma = -1;
	int patternBlockSize = 0;
	QList<int> blockSizes;
	
	for ( index = 0; index < pattern.size(); index++ ) {
		if ( pattern.at(index).toAscii() == 44 ) { // Comma
			if ( comma == -1 ) {
				comma = index;		
			} else {
				// Calculating size of block...
				patternBlockSize = index - comma - 1;
			    if ( patternBlockSize < 1 ) {
			    	// Pattern was invalid or corrupted
			    	return QList<int>();
			    }
				blockSizes.append(patternBlockSize);
				comma = index;
			}
		} else if ( pattern.at(index).toAscii() == 46 ) { // Dot
			if ( comma == -1 ) {
				patternBlockSize = 99999;
			} else {
				// Calculating size of block...
				patternBlockSize = index - comma - 1;
			    if ( patternBlockSize < 1 ) {
			    	// Pattern was invalid or corrupted
			    	return QList<int>();
			    }		
			}
			blockSizes.append(patternBlockSize);
			break;
		}
	}
	
	return blockSizes;
}

/*!
    Grouping numbers  
          
    \param number QString which contains numbers
    \param firstNumber index of first number
    \param lastNumber index of last number
    \param dot index of dot
    \param sign boolean which tells is there sign used
    \param decimal character(s) for decimal
    \param group character(s) for group separator
    \param patternBlockSizes list of blocks
    \return grouped numbers
*/ 
QString HbNgNormalNumber::formatingGroup( const QString &number, 
										  const int &firstNumber, 
										  const int &lastNumber, 
										  const int &dot, 
										  const bool &sign, 
										  const QString &decimal, 
										  const QString &group, 
										  const QList<int> &patternBlockSizes )
{
	QString formated = "";
	int index = 0;
	int patternBlockSize = 0;
	int blockCounter = 0;
	int numberOfBlocks = 0;
	int nrSize = number.size();
	int tempDot = dot;

	if ( tempDot == -1 ) {
		if ( (sign == true) && (lastNumber < nrSize-1) ) {
			tempDot = nrSize - 1;
			
			// Inserting sign (xxx-/+)...
			formated.insert(0, number.at(nrSize-1)); 
		} else {
			tempDot = nrSize;
		}
	} else {
		// Inserting decimals (and sign x.xxx-/+)...
		formated.append(decimal);
		formated.append(number.right(nrSize-tempDot-1));
	}
	
	// Checking amount of blocks and size of first (last in the list) block for using...
	numberOfBlocks = patternBlockSizes.length();
	patternBlockSize = patternBlockSizes.last(); 
	
	for ( index = tempDot-1; index > firstNumber-1; --index ) {
		if ( blockCounter < patternBlockSize ) {
			// Inserting number...
			formated.insert(0, number.at(index));
			blockCounter++;
		} else {
			// Inserting number and group separator...
			formated.insert(0, group);
			formated.insert(0, number.at(index));
			blockCounter = 1;
			
			// Take next size of block in use
			numberOfBlocks--;
			// Updating the size of block if needed...
			if ( numberOfBlocks > 0 ) {
				patternBlockSize = patternBlockSizes.at(numberOfBlocks-1);
			}
		}
	}
	
	if ( firstNumber > 0 ) {
		// Inserting sign (+/-xxx)...
		formated.insert(0, number.at(0)); 
	}
	
	return formated;
}
