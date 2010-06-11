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

#include "hbtypefacexmlparser_p.h"
#include <QString>
#include <QFile>
#include <QStringList>
#include <QXmlStreamAttribute>

#include <QDebug>

#define TYPEFACE_INFO "typeface_information"
#define TYPEFACE_METRICS "typeface_metrics"
#define TYPEFACE_SET "typeface_set"
#define TYPEFACE_MAPPING "typeface_mapping"
#define TYPEFACE_ROLE_ATT "role"
#define TYPEFACE_FAMILY_ATT "family"
#define TYPEFACE_WEIGHT_ATT "weight"
#define TYPEFACE_METRICS_POINT "metric"
#define TYPEFACE_TEXTHEIGHT_ATT "textheight"
#define TYPEFACE_SIZE_ATT "size"
#define TYPEFACE_BASELINE_ATT "baseline"
#define TYPEFACE_LANGUAGE_ATT "language"
#define TYPEFACE_COUNTRY_ATT "country"


#define TYPEFACE_ATT_VAL_BOLD "bold"


/*!
    \class HbTypefaceXmlParser

    \brief The HbTypefaceXmlParser class is responsible for parsing xml file containing the list of system typefaces.

    \warning This class is a part of internal library implementation and should not be public

    \internal
*/

/*
    Constructor.
*/
HbTypefaceXmlParser::HbTypefaceXmlParser()
: mFile(0)
{
}

/*
    Destructor.
*/
HbTypefaceXmlParser::~HbTypefaceXmlParser()
{
	if (mFile) {
		delete mFile;
        mFile = 0;
	}
}

/*!
    \brief Sets the path of the file to be parsed, over-riding the system typefaces.xml file
    Used for testing. Must be called before \c init()
    \param path File path to use
*/
void HbTypefaceXmlParser::setFilePath(const QString& path)
{
	mFilePath = path;
}

/*!
    \brief Initiates reading of the system typefaces.xml file, or another typeface file if setFilePath has been called.
    After returning parser's read position is left at the start of the first typeface mapping, ready for a call to
    \c readNextMapping().

    \return false if there was any error.
*/
bool HbTypefaceXmlParser::init()
{
	QString filePath(":typefaces.xml");

	// If one has been set by API
	if ( !mFilePath.isEmpty() ) {
		filePath = mFilePath;
	}
	else {
	// Check if resource directory has got the typefaces.xml file.
	// If yes, then use the one present in the resource directory
		QString resourceFilePath(TYPEFACE_RESOURCE_FOLDER);
		resourceFilePath.append(QDir::separator());
		resourceFilePath.append("typefaces.xml"); 

		QFile *file = new QFile(resourceFilePath);
		if( file && file->exists() ) {
			filePath = resourceFilePath;
		}
		delete file;
	}	

	mFile = new QFile(filePath);
	// Trying to see if the absolute path can be determined
	QString name2 = mFile->fileName();

	if( !mFile->exists() ) {
		qWarning("HbTypefaceXmlParser Warning: XML-file at %s does not exist", qPrintable(filePath));
		return false;
	}

	// not yet parsed, parse and add data
	if (!mFile->open(QFile::ReadOnly | QFile::Text)) {
		qWarning("HbEffect Warning: XML-file at %s opening failed", qPrintable(filePath));
		return false;
	}

	setDevice(mFile);
	bool positioned(false);
	while (!atEnd() && !positioned) {
    	QXmlStreamReader::TokenType type = readNext();
        if(type == QXmlStreamReader::Invalid)
            break;

		if (isStartElement()) {
			if (name() == TYPEFACE_INFO) {
				positioned = true;
			}
			else {
				raiseError("HbTypefaceXmlParser::read The document is not a valid typeface information xml file.");
			}
		}
	}

	return !error() && positioned;
}

/*!
    Advances the read position of the parser to a point where it can read the <typeface_mapping> set.
*/
bool HbTypefaceXmlParser::readAndPositionTypefaceSet()
{
	Q_ASSERT(isStartElement() && name() == TYPEFACE_INFO);
	bool positioned(false);
	while (!atEnd() && !positioned) {
    	QXmlStreamReader::TokenType type = readNext();
        if(type == QXmlStreamReader::Invalid)
            break;

		if (isStartElement()) {
			if (name() == TYPEFACE_SET) {
				// find the correct language and country
				positioned = matchLanguageAndCountry();
			}
		}
	}
	return positioned;
}

/*
 * Parses information inside one <typeface_set> field.
 */
bool HbTypefaceXmlParser::matchLanguageAndCountry() const
{
	Q_ASSERT(isStartElement() && name() == TYPEFACE_SET);
	QString language, country;
	bool positioned(false);

	HbExtendedLocale systemLocale = HbExtendedLocale::system();
	QString attrName, typefaceLocaleName, systemLocaleName;
	QXmlStreamAttributes attrs = attributes();

	foreach (const QXmlStreamAttribute &attr, attrs) {
		attrName = attr.name().toString();

		if (attrName == TYPEFACE_LANGUAGE_ATT) {
			language = attr.value().toString().toLower();
		}
		else if (attrName == TYPEFACE_COUNTRY_ATT) {
			country = attr.value().toString().toUpper();
		}
		else {
			qDebug("Unrecognized attribute");
		}
	}
	

	// Construct the locale with the typeface locale info
	if (!language.isEmpty()) {
		if (!country.isEmpty()) {
			typefaceLocaleName = language + '_' + country;
			systemLocaleName = systemLocale.name();
		}
		else {
			typefaceLocaleName = language;
			// Language is a lowercase, two-letter, ISO 639 language code
			systemLocaleName = systemLocale.name().left(2);
		}
	}
	else {
		positioned = true;
	}

	if (!positioned && typefaceLocaleName.compare(systemLocaleName, Qt::CaseInsensitive) == 0) {
		positioned = true;
	}
	return positioned;
}

/*!
    Advances the read position of the parser to a point where it has just read a principal start element.
    The start elements that are valid for this are enumerated in \c StartElement, and the enum value is returned.
    If no relevant elements are encountered and the end of file is reached, then \c NoStartElement is returned.
*/
HbTypefaceXmlParser::StartElement HbTypefaceXmlParser::readToStartElement()
{
	StartElement startElement(NoStartElement);

	bool atEnd(false);

    while (!atEnd && startElement == NoStartElement) {
    	QXmlStreamReader::TokenType type = readNext();

        if(type == QXmlStreamReader::Invalid)
            break;

		if (isStartElement()) {

			if (name() == TYPEFACE_METRICS) {
				startElement = TypefaceMetrics;
				// Grab the typeface name as "current"
				QXmlStreamAttributes attrs = attributes();
				foreach (const QXmlStreamAttribute &attr, attrs) {

					if (attr.name().toString() == TYPEFACE_FAMILY_ATT) {
						mCurrentFamily = attr.value().toString();
			            if (!mCurrentFamily.isEmpty()) {
							startElement = TypefaceMetrics; // Now we have a workable start element
			            }
					}
				}

			}
			else if ( name() == TYPEFACE_SET) {
				startElement = TypefaceSet;
			}
		}

		else {
			// Nothing. Some other type of element;
		}
    }

    return startElement;
}

 /*!
    \brief Read the next map item from the xml file.
    \c init() must have been called successfully previously and
    \c readToStartElement must have been called and returned \c TypefaceSet
    The parameters are all output parameters. They define a mapping from a role
    name to a typeface family.
    The mapping also indicates if the typeface should be bold by default

	\sa init()
    \param role The name of the role mapped from
    \param family The typeface family name mapped to the role
    \param bold true if typeface is to be used bold by default
    \return false iff there was an error. Output parameters not defined
*/
bool HbTypefaceXmlParser::readMapping(QString& role, QString& family, bool& bold)
{
	bool atEnd(false);
	bool success(false);

    while (!atEnd && !success ) {
    	QXmlStreamReader::TokenType type = readNext();
        if(type == QXmlStreamReader::Invalid)
            break;

		if (isEndElement()) {
        	if(name() == TYPEFACE_SET) {
    			atEnd = true;
        	}
		}
		else if (isStartElement()) {
			if (name() == TYPEFACE_MAPPING) {
				success = readMapItem(role, family, bold);
			}
		}

		else {
			// Nothing. Some other type of element;
		}
    }

    return success;
}

/*
 * Parses information inside one <typeface_mapping> field.
 */
bool HbTypefaceXmlParser::readMapItem(QString& role, QString& family, bool& isBold)
{
    Q_ASSERT(isStartElement() && name() == TYPEFACE_MAPPING);
    isBold = false;
    bool haveRole(false);
    bool haveFamily(false);

    QString attrName;
	QXmlStreamAttributes attrs = attributes();

	foreach (const QXmlStreamAttribute &attr, attrs) {
		attrName = attr.name().toString();

		if (attrName == TYPEFACE_ROLE_ATT) {
			role = attr.value().toString();
            if (!role.isEmpty()) {
    			haveRole = true;
            }
		}
		else if (attrName == TYPEFACE_FAMILY_ATT) {
			family = attr.value().toString();
            if (!role.isEmpty()) {
    			haveFamily = true;
            }
		}
		else if (attrName == TYPEFACE_WEIGHT_ATT) {
			if (attr.value().toString() == TYPEFACE_ATT_VAL_BOLD)
				isBold = true;
		}
		else {
			qDebug("Unrecognized attribute");
		}
    }

    return haveRole && haveFamily;
}

/*!
    \brief Method to processes one typeface_metrics element.

    \c readToStartElement must have been called previously and returned \c TypefaceMetrics
    Returns false when there is a problem, or the current block of typeface_metrics elements ends.

    \param textHeight text pane height of the metrics measurement point
    \param size the size in pixels corresponding to that text pane height
    \param baseline the distance from the top of the textheight at which to set the glyphs' baselines
    \return false iff there was an error. Output parameters values not defined
*/
bool HbTypefaceXmlParser::readMetric(int &textHeight, int &size, int&baseline)
{
	bool atEnd(false);
	bool success(false);

	textHeight = -1;
	size = -1;
	baseline = -1;

    while (!atEnd && !success) {
    	QXmlStreamReader::TokenType type = readNext();
        if(type == QXmlStreamReader::Invalid)
            break;

		if (isEndElement()) {
        	if(name() == TYPEFACE_METRICS) {
    			atEnd = true;
        	}
		}
		else if (isStartElement()) {
			if (name() == TYPEFACE_METRICS_POINT) {
				success = readMetricItem(textHeight, size, baseline);
			}
		}
		else {
			// Nothing. Some other type of element;
		}
    }

    return success;
}

/*!
    Processes one typeface_metrics element.
    Attributes are unpacked for the downsized size, and the baseline value relative to the top of text pane.
*/
bool HbTypefaceXmlParser::readMetricItem(int &textHeight, int &size, int &baseline)
{
    Q_ASSERT(isStartElement() && (name()==TYPEFACE_METRICS_POINT));

    QString attrName;
    QString attrValue;
	QXmlStreamAttributes attrs = attributes();

	foreach (const QXmlStreamAttribute &attr, attrs) {
		attrName = attr.name().toString();
		attrValue = attr.value().toString();

		if (attrName == TYPEFACE_TEXTHEIGHT_ATT) {
			textHeight = attrValue.toInt();
		}
		else if (attrName == TYPEFACE_SIZE_ATT) {
			size = attrValue.toInt();
		}
		else if (attrName == TYPEFACE_BASELINE_ATT) {
			baseline = attrValue.toInt();
		}
		else {
			qDebug() << "Unrecognized attribute: " << attrName;
		}
    }

    return (textHeight>0 && size>0);
}

/*!
    Closes the file i/o and resets the parser.
*/
void HbTypefaceXmlParser::close()
{
	clear();
	if (mFile && mFile->isOpen()) {
			mFile->close(); // Device is owned externally from streamreader
	}
}

/*!
    Returns the current typeface family name.
    This returns the name of the family attribute of the most recently read typeface_metrics element.
    It will return an empty string if no such element has been processed.
*/
const QString HbTypefaceXmlParser::metricsTypefaceFamily() const
{
	return mCurrentFamily;
}

// End of File
