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


#include "hbtypefaceinfodatabase_p.h"
#include <QFontMetrics>
#include <QDir>
#include <QDebug>
#include <QXmlStreamWriter>

// internal debug
#undef HBTYPEFACEINFO_DEBUG_ENABLE
//#define HBTYPEFACEINFO_DEBUG_ENABLE


#ifdef Q_OS_SYMBIAN
	#define HB_FONTS_WRITABLE_PATH QString("c:\\hb\\fonts")
#else
	#ifndef Q_OS_UNIX
		#define HB_FONTS_WRITABLE_PATH QString("c:\\Hb\\lib")
	#endif
#endif

// Recognized texts for values of role strings
#define PRIMARY_STRING "primary"
#define SECONDARY_STRING "secondary"
#define PRIMARYSMALL_STRING "primarysmall"
#define TITLE_STRING "title"
#define DIGITAL_STRING "digital"
#define UNDEFINED_STRING "undefined"

#define TYPEFACE_METRICS_FILE_STEM "typeface_metrics_"
#define LARGEST_SIZE 100
// Following must be greater than 0
#define SMALLEST_SIZE 1




static bool encodeRole(const QString& roleName, HbFontSpec::Role &role)
{
    bool encoded(true); // return value

    QString name = roleName.toLower();

    if (name == PRIMARY_STRING) {
        role = HbFontSpec::Primary;
    }
    else if (name == SECONDARY_STRING) {
        role = HbFontSpec::Secondary;
    }
    else if (name == TITLE_STRING) {
        role = HbFontSpec::Title;
    }
    else if (name == PRIMARYSMALL_STRING) {
        role = HbFontSpec::PrimarySmall;
    }
    else if (name == DIGITAL_STRING) {
        role = HbFontSpec::Digital;
    }
    else if (name == UNDEFINED_STRING) {
        role = HbFontSpec::Undefined; // Translated if someone passes the string
    }
    else { // Really undefined
        role = HbFontSpec::Undefined;
        encoded = false;
    }

    return encoded;
}

/*!
Returns path to a writable location that should be used as a base storage folder for
dynamic metric creation.
*/
static QString writablePath()
{
#ifdef Q_OS_SYMBIAN
    return HB_FONTS_WRITABLE_PATH;
#else
    if (QString(HB_BUILD_DIR) == QString(HB_INSTALL_DIR)) {
        // This is local build so also use local writable path.
        return QString(HB_INSTALL_DIR) + QDir::separator() + QString(".hb") 
				+ QDir::separator() + QString("fonts");
    } else {
#ifdef Q_OS_UNIX
    return QDir::homePath() + QDir::separator() + QString(".hb") 
				+ QDir::separator() + QString("fonts");
#else
    return HB_FONTS_WRITABLE_PATH ;
#endif
    }
#endif
}


HbTypefaceInfoDatabase *HbTypefaceInfoDatabase::instance(HbMemoryManager::MemoryType type)
{
    static HbTypefaceInfoDatabase info(type);
    return &info;
}

/*!
    Constructor.
*/
HbTypefaceInfoDatabase::HbTypefaceInfoDatabase(HbMemoryManager::MemoryType type)
    : mTypefaceInfoVector(0),mTypefaceInfoVectorOffset(-1),mType(type)
{
    GET_MEMORY_MANAGER(mType);
    try {
        mTypefaceInfoVectorOffset = manager->alloc(sizeof(HbTypefaceInfoVector));
        mTypefaceInfoVector = new((char*)manager->base() + mTypefaceInfoVectorOffset)
                HbTypefaceInfoVector(mType);
        init();
    } catch(std::exception &) {
        if (mTypefaceInfoVectorOffset != -1) {
            if (mTypefaceInfoVector) {
                mTypefaceInfoVector->~HbTypefaceInfoVector();
                mTypefaceInfoVector = 0;
            }
            manager->free(mTypefaceInfoVectorOffset);
            mTypefaceInfoVectorOffset = -1;
        }
    }
}

int HbTypefaceInfoDatabase::typefaceInfoVectorOffset()
{
    return mTypefaceInfoVectorOffset;
}


/*!
    Initialization method.
*/
void HbTypefaceInfoDatabase::init()
{
    HbTypefaceXmlParser *parser = new HbTypefaceXmlParser();

    if (!(parser->init() && parser->readAndPositionTypefaceSet())){
        qWarning( "HbTypefaceInfoDatabase: init failed, unable to initialize parser" );
        return;
    }
    else {
        QString role;
        QString family;
        bool isBold;
        while (parser->readMapping( role, family, isBold)) {
            HbTypefaceInfoItem item( mType );
            HbFontSpec::Role roleEnum;
            if (encodeRole(role, roleEnum)){
                item.mRoleEnum = roleEnum;
                item.mFamily = family;
                item.mIsBold = isBold;
                mTypefaceInfoVector->append( item );
            }
            else {
                // role might be an alias.  Not required functionality. Ignore
            }
        }

        HbTypefaceInfoItem *undefinedFontspec = 0;
        HbTypefaceInfoItem *secondaryFontspec = 0;

        for( int i = 0; i < mTypefaceInfoVector->size(); i++ ) {
            HbTypefaceInfoItem *item = &(mTypefaceInfoVector->at(i));
            if( item->mRoleEnum == HbFontSpec::Secondary ) {
                secondaryFontspec = item;
            }
            if( item->mRoleEnum == HbFontSpec::Undefined ) {
                undefinedFontspec = item;
            }
        }

        // Secondary chosen as default it not specified in the xml
        if ( !undefinedFontspec && secondaryFontspec ) {
            HbTypefaceInfoItem item( mType );
            item.mRoleEnum = HbFontSpec::Undefined;
            item.mFamily = secondaryFontspec->mFamily;
            item.mIsBold = secondaryFontspec->mIsBold;
            mTypefaceInfoVector->append( item );
        }
    }

    parser->close();

    /*
     * here should be filling of downsize table.
     */


    for( int i = 0; i < mTypefaceInfoVector->size(); i++ ) {
        HbTypefaceInfoItem *typeFaceInfoItem = &mTypefaceInfoVector->at(i);

        if( !readTypefaceMetricsFile( parser, typeFaceInfoItem ) ) {
            autoGenerateMetrics( typeFaceInfoItem );
            outputMetrics( typeFaceInfoItem );
        }
    }

    delete parser;
}




bool HbTypefaceInfoDatabase::readTypefaceMetricsFile( HbTypefaceXmlParser *parser, HbTypefaceInfoItem *typeFaceInfoItem )
{
    int numPoints(0);

    parser->init();

	QString typefaceMetricsFileName;

	typefaceMetricsFileName.append(TYPEFACE_RESOURCE_FOLDER);
	typefaceMetricsFileName.append(QDir::separator());
	typefaceMetricsFileName.append(TYPEFACE_METRICS_FILE_STEM);
    // replace whitespace with underscores and append
    QString temp = typeFaceInfoItem->mFamily;
    temp = temp.toLower().replace(QRegExp("\\s+"), QString("_"));
    typefaceMetricsFileName.append( temp );
    typefaceMetricsFileName.append(".xml");

	QFile *file = new QFile(typefaceMetricsFileName);
	if( !(file && file->exists()) ) {
		typefaceMetricsFileName = writablePath();
		typefaceMetricsFileName.append(QDir::separator());
		typefaceMetricsFileName.append(TYPEFACE_METRICS_FILE_STEM);
        typefaceMetricsFileName.append( temp );
        typefaceMetricsFileName.append(".xml");
	}
	delete file;

#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
    qDebug("HbDownsizeInfo::readTypefaceMetricsFile: typeface metric filename: %s", typefaceMetricsFileName.toAscii().constData());
#endif
    parser->setFilePath(typefaceMetricsFileName);

    if (parser->init()){
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
    qDebug( "HbDownsizeInfo::readTypefaceMetricsFile: parser init() ok");
#endif
        QString family;
        HbTypefaceXmlParser::StartElement startEl(HbTypefaceXmlParser::Undefined);
        while ((startEl = parser->readToStartElement()) != HbTypefaceXmlParser::NoStartElement) {
            if (startEl == HbTypefaceXmlParser::TypefaceMetrics) {
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
                qDebug( "HbDownsizeInfo::readTypefaceMetricsFile: start element: typeface metrics");
#endif
                if (parser->metricsTypefaceFamily().toLower()==typeFaceInfoItem->mFamily.toLower()) {
                    int textHeight(0);
                    int baseline(0);
                    HbTypefaceInfoItem::HbTypefaceMeasureInfoStruct readMetrics;
                    while (parser->readMetric(textHeight, readMetrics.pixelSize, baseline)) {
                        numPoints++;
                        readMetrics.textHeight = textHeight;
                        typeFaceInfoItem->mDownSizeTable.append( readMetrics );


                        typeFaceInfoItem->mLowestExtent = qMin(typeFaceInfoItem->mLowestExtent, textHeight); // Stored so that we can extrapolate from the lowest value
                        typeFaceInfoItem->mHighestExtent = qMax(typeFaceInfoItem->mHighestExtent, textHeight); // Stored so that we can extrapolate from the highest value

#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
            qDebug( "HbDownsizeInfo::readTypefaceMetricsFile: metric %d: extent = %d size = %d baseline = %d", numPoints, textHeight, readMetrics.pixelSize, baseline);
#endif
                    }
                }
            }
        }
    }

    parser->close();
    return numPoints > 0;
}



void HbTypefaceInfoDatabase::autoGenerateMetrics( HbTypefaceInfoItem *typeFaceInfoItem )
{
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
    qDebug( "HbDownsizeInfo::autoGenerateMetrics: WARNING: auto generating metrics, this is inefficient!");
#endif
    QFont f(typeFaceInfoItem->mFamily);

    HbTypefaceInfoItem::HbTypefaceMeasureInfoStruct lastMetrics;
    HbTypefaceInfoItem::HbTypefaceMeasureInfoStruct measuredMetrics;

    int lastExtentFilled = 0;
    lastMetrics.textHeight = lastExtentFilled;
    typeFaceInfoItem->mDownSizeTable.append(lastMetrics);

#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
    qDebug( "HbDownsizeInfo::autoGenerateMetrics: extent = %d size = %d measured ascent = %d", lastExtentFilled, lastMetrics.pixelSize, lastMetrics.measuredAscent);
#endif

    int size;

    for (size = SMALLEST_SIZE; size <= LARGEST_SIZE; size++) {
        measuredMetrics.pixelSize = size;
        f.setPixelSize(size);
        QFontMetrics fm(f);
        int thisExtent = fm.height();
        measuredMetrics.measuredAscent = fm.ascent(); // not actually measuring it, just taking it from the metrics for now
        for (int extent = lastExtentFilled+1; extent < thisExtent; extent++){
            lastMetrics.textHeight = extent;
            typeFaceInfoItem->mDownSizeTable.append( lastMetrics );
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
            qDebug( "HbDownsizeInfo::autoGenerateMetrics: fill in: extent = %d size = %d measured ascent = %d", extent, lastMetrics.pixelSize, lastMetrics.measuredAscent);
#endif
        }
        // fill in this one
        measuredMetrics.textHeight = thisExtent;
        typeFaceInfoItem->mDownSizeTable.append(measuredMetrics);
#ifdef HBTYPEFACEINFO_DEBUG_ENABLE
        qDebug( "HbDownsizeInfo::autoGenerateMetrics: extent = %d size = %d measured ascent = %d", thisExtent, measuredMetrics.pixelSize, measuredMetrics.measuredAscent);
#endif
        lastExtentFilled = thisExtent;
        lastMetrics.pixelSize = measuredMetrics.pixelSize;
        lastMetrics.measuredAscent = measuredMetrics.measuredAscent;
    }
    typeFaceInfoItem->mHighestExtent = lastExtentFilled; // Stored so that we can extrapolate from the last mapped value
}


void HbTypefaceInfoDatabase::outputMetrics( HbTypefaceInfoItem *typeFaceInfoItem ) const
{
    QString filePath = writablePath();
	QDir dir(filePath);
	if(!dir.exists()) {
		dir.mkpath(filePath + QDir::separator() );
	}

    filePath.append(QDir::separator());
    filePath.append("typeface_metrics_");

    QString temp = typeFaceInfoItem->mFamily;
    temp = temp.toLower().replace(QRegExp("\\s+"), QString("_"));

    filePath.append( temp );
    filePath.append(".xml");

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        return;
    }

    QXmlStreamWriter xmlWriter(&file);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeComment("this file is automatically generated by HBTYPEFACEINFO");
    xmlWriter.writeCharacters("\n"); // is this platform neutral?

    xmlWriter.writeStartElement("typeface_information");

    xmlWriter.writeStartElement("typeface_metrics");
    xmlWriter.writeAttribute("family", typeFaceInfoItem->mFamily );

    for( int i = 0; i < typeFaceInfoItem->mDownSizeTable.size(); i++ ) {
        xmlWriter.writeStartElement("metric");
        xmlWriter.writeAttribute("textheight", QString::number( typeFaceInfoItem->mDownSizeTable.at(i).textHeight ));
        xmlWriter.writeAttribute("size", QString::number( typeFaceInfoItem->mDownSizeTable.at(i).pixelSize) );
        xmlWriter.writeAttribute("baseline", QString::number(0));

        xmlWriter.writeEndElement();
    }

    xmlWriter.writeEndDocument();
    file.close();

}

