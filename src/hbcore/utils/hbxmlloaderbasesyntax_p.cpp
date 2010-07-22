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

#include "hbxmlloaderbasesyntax_p.h"

#include <QMetaEnum>
#include <QDebug>

/*
    \class HbXmlLoaderBaseSyntax
    \internal
    \proto
*/

HbXmlLoaderBaseSyntax::HbXmlLoaderBaseSyntax( HbXmlLoaderAbstractActions *actions )
: HbXmlLoaderAbstractSyntax(actions)
{
}

HbXmlLoaderBaseSyntax::~HbXmlLoaderBaseSyntax()
{
    qDeleteAll(mCurrentContainer);
}

bool HbXmlLoaderBaseSyntax::load( QIODevice *device, const QString &section )
{
    return loadDevice(device, section);
}

bool HbXmlLoaderBaseSyntax::scanForSections( QIODevice *device, QList<QString> &sectionsList ) {
    const QChar separator(' ');
    bool exit = false;
    bool result = true;

    mReader.setDevice( device );

    mCurrentSection.clear();

    mTopState = TS_READ_DOCUMENT;


    while( !exit ) {
        switch( mTopState ) {
            case TS_READ_DOCUMENT:
            {
                mCurrentTokenType = mReader.readNext();
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE READ_ELEMENT " + mReader.name().toString() );
                switch( mCurrentTokenType ) {
                    case QXmlStreamReader::EndElement:
                    case QXmlStreamReader::StartElement:
                    {
                        mCurrentElementType = elementType( mReader.name() );
                        if( ( mCurrentElementType == HbXml::METADATA ) && ( mCurrentTokenType == QXmlStreamReader::StartElement ) ) {
                            mTopState = TS_READ_METADATA;
                            break;
                        }

                        if( mCurrentElementType == HbXml::SECTION ) {

                            if( mCurrentTokenType == QXmlStreamReader::StartElement ) {

                                QString name = attribute( ATTR_NAME );
                                if( name.isEmpty() ) {
                                    qWarning() << "Section without a name, line " << mReader.lineNumber();
                                    mTopState = TS_ERROR;
                                    break;
                                }

                                mCurrentSection << name;
                                sectionsList.append( mCurrentSection.join( separator ) );

                            } else if( mCurrentTokenType == QXmlStreamReader::EndElement ) {
                                mCurrentSection.removeLast();
                            }
                        }
                        break;
                    }
                    case QXmlStreamReader::EndDocument:
                    {
                        mTopState = TS_EXIT;
                        break;
                    }
                    case QXmlStreamReader::Characters:
                    {
                        if( mReader.isWhitespace() ) {
                            break;
                        }
                        HB_DOCUMENTLOADER_PRINT( "Characters" );
                    }
                    case QXmlStreamReader::NoToken:
                    HB_DOCUMENTLOADER_PRINT( "NoToken" );
                    case QXmlStreamReader::Invalid:
                    HB_DOCUMENTLOADER_PRINT( "Invalid" );
                    case QXmlStreamReader::EntityReference:
                    {
                        qWarning() << "Parse error, line " << mReader.lineNumber();
                        mTopState = TS_ERROR;
                        break;
                    }
                    case QXmlStreamReader::StartDocument:
                    case QXmlStreamReader::Comment:
                    case QXmlStreamReader::DTD:
                    case QXmlStreamReader::ProcessingInstruction:
                    default:
                    {
                        break;
                    }
                }
                break;
            }

            case TS_READ_METADATA:
            {
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE READ_METADATA" );
                mCurrentTokenType = mReader.readNext();
                switch( mCurrentTokenType ) {
                    case QXmlStreamReader::EndElement:
                    {
                        mCurrentElementType = elementType( mReader.name() );
                        if( mCurrentElementType == HbXml::METADATA ) {
                            mTopState = TS_READ_DOCUMENT;
                        }
                        break;
                    }
                    case QXmlStreamReader::NoToken:
                    case QXmlStreamReader::Invalid:
                    {
                        qWarning() << "Parse error, line " << mReader.lineNumber();
                        mTopState = TS_ERROR;
                        break;
                    }
                    case QXmlStreamReader::EndDocument:
                    {
                        qWarning() << "Unexpected end of document, line " << mReader.lineNumber();
                        mTopState = TS_ERROR;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                break;
            }

            case TS_ERROR:
            {
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE ERROR" );
                result = false;
                mTopState = TS_EXIT;
                break;
            }
            case TS_EXIT:
            {
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE EXIT" );
                exit = true;
                break;
            }
            default:
            {
                HB_DOCUMENTLOADER_PRINT( "INTERNAL ERROR" );
                mTopState = TS_ERROR;
                break;
            }
        }
    }
    mReader.clear();
    return result;

}

bool HbXmlLoaderBaseSyntax::loadDevice(QIODevice *device, const QString &section)
{
    const QChar separator(' ');
    mReader.setDevice( device );

    bool exit = false;
    bool result = true;
    mCurrentSection.clear();
    mRequiredSection.clear();

    mTopState = TS_READ_DOCUMENT;

    mDocumentState = DS_START_DOCUMENT;
    mElementState = ES_GENERAL_ITEM;

    mCurrentLayoutType = LAYOUT_NULL;
    mCurrentContainerType = CONTAINER_NULL;

    if( section != 0 ) {
        mRequiredSection = section.split( separator, QString::SkipEmptyParts );
    }

    while( !exit ) {
        switch( mTopState ) {
            case TS_READ_DOCUMENT:
            {
                mCurrentTokenType = mReader.readNext();
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE READ_ELEMENT " + mReader.name().toString() );
                switch( mCurrentTokenType ) {
                    case QXmlStreamReader::EndElement:
                    case QXmlStreamReader::StartElement:
                    {
                        mCurrentElementType = elementType( mReader.name() );
                        if( ( mCurrentElementType == HbXml::METADATA ) && ( mCurrentTokenType == QXmlStreamReader::StartElement ) ) {
                            mTopState = TS_READ_METADATA;
                            break;
                        }
                        if( ! readDocument() ) {
                            qWarning() << "Error in document, line " << mReader.lineNumber();
                            mTopState = TS_ERROR;
                        }
                        break;
                    }
                    case QXmlStreamReader::EndDocument:
                    {
                        if( mDocumentState == DS_END_DOCUMENT ) {
                            mTopState = TS_EXIT;
                        } else {
                            qWarning() << "Unexpected end of document, line " << mReader.lineNumber();
                            mTopState = TS_ERROR;
                        }
                        break;
                    }
                    case QXmlStreamReader::Characters:
                    {
                        if( mReader.isWhitespace() ) {
                            break;
                        }
                        HB_DOCUMENTLOADER_PRINT( "Characters" );
                    }
                    case QXmlStreamReader::NoToken:
                    HB_DOCUMENTLOADER_PRINT( "NoToken" );
                    case QXmlStreamReader::Invalid:
                    HB_DOCUMENTLOADER_PRINT( "Invalid" );
                    case QXmlStreamReader::EntityReference:
                    {
                        qWarning() << "Parse error, line " << mReader.lineNumber();
                        mTopState = TS_ERROR;
                        break;
                    }
                    case QXmlStreamReader::StartDocument:
                    case QXmlStreamReader::Comment:
                    case QXmlStreamReader::DTD:
                    case QXmlStreamReader::ProcessingInstruction:
                    default:
                    {
                        break;
                    }
                }
                break;
            }

            case TS_READ_METADATA:
            {
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE READ_METADATA" );
                mCurrentTokenType = mReader.readNext();
                switch( mCurrentTokenType ) {
                    case QXmlStreamReader::EndElement:
                    {
                        mCurrentElementType = elementType( mReader.name() );
                        if( mCurrentElementType == HbXml::METADATA ) {
                            mTopState = TS_READ_DOCUMENT;
                        }
                        break;
                    }
                    case QXmlStreamReader::NoToken:
                    case QXmlStreamReader::Invalid:
                    {
                        qWarning() << "Parse error, line " << mReader.lineNumber();
                        mTopState = TS_ERROR;
                        break;
                    }
                    case QXmlStreamReader::EndDocument:
                    {
                        qWarning() << "Unexpected end of document, line " << mReader.lineNumber();
                        mTopState = TS_ERROR;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                break;
            }

            case TS_ERROR:
            {
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE ERROR" );
                result = false;
                mActions->cleanUp();
                mActions->deleteAll();
                exit = true;
                break;
            }
            case TS_EXIT:
            {
                mActions->cleanUp();
                HB_DOCUMENTLOADER_PRINT( "TOP_STATE EXIT" );
                exit = true;
                break;
            }
            default:
            {
                HB_DOCUMENTLOADER_PRINT( "INTERNAL ERROR" );
                mTopState = TS_ERROR;
                break;
            }
        }
    }
    mReader.clear();
    return result;
}

bool HbXmlLoaderBaseSyntax::readDocument()
{
    bool result = false;
    switch( mDocumentState ) {
        case DS_START_DOCUMENT:
        {
            HB_DOCUMENTLOADER_PRINT( "DOCUMENT_STATE START DOCUMENT" );
            if( mCurrentElementType == HbXml::DOCUMENT ) {
                result = processDocument();
                HB_DOCUMENTLOADER_PRINT( "SWITCHING TO READ SECTIONS STATE" );
                mDocumentState = DS_READ_SECTIONS;
            }
            break;
        }
        case DS_READ_SECTIONS:
        {
            if( ( mCurrentElementType == HbXml::DOCUMENT ) && ( mCurrentTokenType == QXmlStreamReader::EndElement ) ) {
                HB_DOCUMENTLOADER_PRINT( "SWITCHING TO END DOCUMENT STATE" );
                mDocumentState = DS_END_DOCUMENT;
                result = checkEndElementCorrectness();
                break;
            }

            if( ( mCurrentSection != mRequiredSection ) ) {
                HB_DOCUMENTLOADER_PRINT( "DOCUMENT_STATE READ ALIEN SECTIONS" );
                result = readAlienSection();
            } else {
                HB_DOCUMENTLOADER_PRINT( "DOCUMENT_STATE READ TARGET SECTIONS" );
                result = readTargetSection();
            }
            break;
        }
        case DS_END_DOCUMENT:
        {
            HB_DOCUMENTLOADER_PRINT( "DOCUMENT_STATE END DOCUMENT - ERROR" );
            break;
        }
    }

    return result;
}

bool HbXmlLoaderBaseSyntax::readAlienSection()
{
    if( mCurrentElementType == HbXml::SECTION ) {

        if( mCurrentTokenType == QXmlStreamReader::StartElement ) {

            QString name = attribute( ATTR_NAME );
            if( name.isEmpty() ) {
                qWarning() << "Section without a name, line " << mReader.lineNumber();
                return false;
            }

            HB_DOCUMENTLOADER_PRINT( QString( "READ ALIEN SECTION: LEAVING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );
            mCurrentSection << name;
            HB_DOCUMENTLOADER_PRINT( QString("READ ALIEN SECTION: ENTERING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );

        } else if( mCurrentTokenType == QXmlStreamReader::EndElement ) {

            HB_DOCUMENTLOADER_PRINT( QString( "READ ALIEN SECTION: LEAVING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );
            mCurrentSection.removeLast();
            HB_DOCUMENTLOADER_PRINT( QString( "READ ALIEN SECTION: ENTERING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );

        } else {

            HB_DOCUMENTLOADER_PRINT( "READ ALIEN SECTION: UNEXPECTED TOKEN TYPE" );

        }
    }
    return true;
}

bool HbXmlLoaderBaseSyntax::readTargetSection()
{
    bool result = false;


    if( mCurrentTokenType == QXmlStreamReader::StartElement ) {
        HB_DOCUMENTLOADER_PRINT( QString( "READ TARGET SECTION: START ELEMENT " ) + mReader.name().toString() );
        switch( mElementState ) {
            case ES_GENERAL_ITEM:
            {
                result = readGeneralStartItem();
                break;
            }
            case ES_LAYOUT_ITEM:
            {
                result = readLayoutStartItem();
                break;
            }
            case ES_CONTAINER_ITEM:
            {
                result = readContainerStartItem();
                break;
            }
        }

    } else if( mCurrentTokenType == QXmlStreamReader::EndElement ) {
        HB_DOCUMENTLOADER_PRINT( QString( "READ TARGET SECTION: END ELEMENT " ) + mReader.name().toString() );

        switch( mElementState ) {
            case ES_GENERAL_ITEM:
            {
                result = readGeneralEndItem();
                break;
            }
            case ES_LAYOUT_ITEM:
            {
                result = readLayoutEndItem();
                break;
            }
            case ES_CONTAINER_ITEM:
            {
                result = readContainerEndItem();
                break;
            }
        }

    } else {
        HB_DOCUMENTLOADER_PRINT( "READ TARGET SECTION: UNEXPECTED TOKEN TYPE" );
    }

    return result;
}

bool HbXmlLoaderBaseSyntax::readContainerStartItem()
{
	qWarning() << "Internal error, wrong container type, line " << mReader.lineNumber();
    return false;
}

bool HbXmlLoaderBaseSyntax::readContainerEndItem()
{
    HB_DOCUMENTLOADER_PRINT( "GENERAL CONTAINER END ITEM" );
    if( mCurrentElementType == HbXml::CONTAINER ) {
        HB_DOCUMENTLOADER_PRINT( "GENERAL CONTAINER END ITEM : SWITCHING TO GENERAL ITEM PROCESSING MODE" );
        mElementState = ES_GENERAL_ITEM;
    }
    return true;
}

bool HbXmlLoaderBaseSyntax::readLayoutStartItem()
{
	qWarning() << "Internal error, wrong layout type, line " << mReader.lineNumber();
    return false;
}

bool HbXmlLoaderBaseSyntax::readAnchorLayoutStartItem(bool idBased)
{
    bool result = true;

    HbXmlLengthValue minVal, prefVal, maxVal;
    QSizePolicy::Policy sizepolicyVal;
    QSizePolicy::Policy *sizepolicyValP = 0;
    HbAnchor::Direction directionVal;
    HbAnchor::Direction *directionValP = 0;
    QString anchorId;

    const QString src = attribute( AL_SRC_NAME );
    QString srcId = attribute( AL_SRC_ID );
    if ( src.isNull() && srcId.isNull() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO SRC NOR SRCID SPECIFIED" ) );
        result = false;
    } else if ( !src.isNull() && !srcId.isNull() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: BOTH SRC AND SRCID SPECIFIED" ) );
        result = false;
    } else if ( idBased && srcId.isNull() ) {
        srcId = src;
    }

    const QString dst = attribute( AL_DST_NAME );
    QString dstId = attribute( AL_DST_ID );
    if ( dst.isNull() && dstId.isNull() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: NO DST NOR DSTID SPECIFIED" ) );
        result = false;
    } else if ( !dst.isNull() && !dstId.isNull() ) {
        HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: BOTH DST AND DSTID SPECIFIED" ) );
        result = false;
    } else if ( idBased && dstId.isNull() ) {
        dstId = dst;
    }

    const QString srcEdgeStr = attribute( AL_SRC_EDGE );
    const QString dstEdgeStr = attribute( AL_DST_EDGE );

    Hb::Edge srcEdge, dstEdge;
    result &= toAnchorEdge( srcEdgeStr, srcEdge );
    result &= toAnchorEdge( dstEdgeStr, dstEdge );

    // Check the convenience notation first. 
    QString spacing = attribute( AL_SPACING );
    if ( result && !spacing.isEmpty() ) {
        directionVal = HbAnchor::Positive;
        directionValP = &directionVal;
        if (spacing.at(0) == '-') {
            directionVal = HbAnchor::Negative;
            spacing = spacing.mid(1);
        }
        sizepolicyVal = QSizePolicy::Fixed;
        sizepolicyValP = &sizepolicyVal;
        result = toLengthValue( spacing, prefVal );
    }

    const QString direction = attribute( AL_DIRECTION );
    if ( result && !direction.isNull() ) {
        // intentionally overriding possible previously set value
        directionValP = &directionVal;
        result = toAnchorDir( direction, directionVal );
    }

    const QString spacer = attribute( AL_SPACER );
    if ( result && !spacer.isEmpty() ) {
        // intentionally overriding possible previously set value
        sizepolicyVal = QSizePolicy::Preferred;
        sizepolicyValP = &sizepolicyVal;
        anchorId = spacer;
        if (prefVal.mType != HbXmlLengthValue::None) {
            // previously set by "spacing"
            minVal = prefVal;
        } else {
            prefVal.mType = HbXmlLengthValue::PlainNumber;
            prefVal.mValue = 0;
        }
        if ( !directionValP ) {
            // direction not yet set, use heuristics
            bool srcIsLayout = src.isEmpty() && srcId.isEmpty();
            bool negativeEdge = srcEdge == Hb::LeftEdge || srcEdge == Hb::TopEdge;
            directionVal = ( (!srcIsLayout && negativeEdge) || (srcIsLayout && !negativeEdge) )
                ? HbAnchor::Negative
                : HbAnchor::Positive;
            directionValP = &directionVal;
            HB_DOCUMENTLOADER_PRINT( QString( "ANCHORLAYOUT: ANCHOR DIRECTION SET BY HEURISTICS" ) );
        }
    }

    const QString minLength = attribute( AL_MIN_LENGTH );
    const QString prefLength = attribute( AL_PREF_LENGTH );
    const QString maxLength = attribute( AL_MAX_LENGTH );
    const QString sizepolicy = attribute( AL_SIZEPOLICY );
    
    const QString anchorIdTemp = attribute( AL_ANCHOR_ID );
    if ( !anchorIdTemp.isNull() ) {
        // intentionally overriding possible previously set value
        anchorId = anchorIdTemp;
    }

    if ( result && !minLength.isNull() ) {
        // intentionally overriding possible previously set value
        result = toLengthValue( minLength, minVal );
    }
    if ( result && !prefLength.isNull() ) {
        // intentionally overriding possible previously set value
        result = toLengthValue( prefLength, prefVal );
    }
    if ( result && !maxLength.isNull() ) {
        result = toLengthValue( maxLength, maxVal );
    }
    if ( result && !sizepolicy.isNull() ) {
        // intentionally overriding possible previously set value
        sizepolicyValP = &sizepolicyVal;
        result = toSizePolicy( sizepolicy, sizepolicyVal );
    }
    if ( result ) {
        result = mActions->addAnchorLayoutItem( src, srcId, srcEdge, dst, dstId, dstEdge, minVal, prefVal, maxVal, sizepolicyValP, directionValP, anchorId );
    }
    return result;
}

bool HbXmlLoaderBaseSyntax::readLayoutEndItem()
{
    HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT END ITEM" );
    if( mCurrentElementType == HbXml::LAYOUT ) {
        HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT END ITEM : SWITCHING TO GENERAL ITEM PROCESSING MODE" );
        mElementState = ES_GENERAL_ITEM;
    }
    return true;
}

bool HbXmlLoaderBaseSyntax::readGeneralStartItem()
{
    bool result = false;
    switch( mCurrentElementType ) {
         case HbXml::DOCUMENT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: DOCUMENT" );
            result = processDocument();
            break;
         }
         case HbXml::LAYOUT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: LAYOUT" );
            result = processLayout();

            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SWITCHING TO LAYOUT PROCESSING MODE" );
            mElementState = ES_LAYOUT_ITEM;
            break;
         }
         case HbXml::CONTAINER:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: CONTAINER" );
            result = processContainer();

            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SWITCHING TO CONTAINER PROCESSING MODE" );
            mElementState = ES_CONTAINER_ITEM;
            break;
         }
         case HbXml::SECTION:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SECTION" );
            QString name = attribute( ATTR_NAME );
            if( name.isEmpty() ) {
                qWarning() << "Section witout a name, line " << mReader.lineNumber();
                break;
            }

            HB_DOCUMENTLOADER_PRINT( QString( "GENERAL START ITEM: LEAVING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );
            mCurrentSection << name;
            HB_DOCUMENTLOADER_PRINT( QString( "GENERAL START ITEM: ENTERING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );

            result = true;
            break;
         }
         default:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: ERROR" );
            qWarning() << "Unknown element, line " << mReader.lineNumber();
            break;
         }
    }
    return result;
}

bool HbXmlLoaderBaseSyntax::readGeneralEndItem()
{

    bool result = false;

    switch( mCurrentElementType ) {
         case HbXml::LAYOUT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: LAYOUT - ERROR, CANNOT BE IN THIS STATE" );
            result = false;
            break;
         }
         case HbXml::CONTAINER:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: CONTAINER - ERROR, CANNOT BE IN THIS STATE" );
            result = false;
            break;
         }
         case HbXml::SECTION:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL END ITEM: SECTION" );

            HB_DOCUMENTLOADER_PRINT( QString( "GENERAL END ITEM: LEAVING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );
            mCurrentSection.removeLast();
            HB_DOCUMENTLOADER_PRINT( QString( "GENERAL END ITEM: ENTERING SECTION " ) + "'" + mCurrentSection.join(" ") + "'" );

            result = true;
            break;
         }

         default:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL END ITEM: CHECKING ELEMENT CORRECTNESS" );
            result = checkEndElementCorrectness();
         }
    }
    return result;
}


bool HbXmlLoaderBaseSyntax::processDocument()
{
    return false;
}


bool HbXmlLoaderBaseSyntax::processLayout()
{
	qWarning() << "Unknown layout type, line " << mReader.lineNumber();
    return false;
}

bool HbXmlLoaderBaseSyntax::processContainer()
{
	qWarning() << "Unknown container type, line " << mReader.lineNumber();
    return false;
}



bool HbXmlLoaderBaseSyntax::checkEndElementCorrectness()
{
    if ( !mActions->pop( elementType( mReader.name() ) ) ) {
        qWarning() << "Error in end element, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

HbXml::ElementType
    HbXmlLoaderBaseSyntax::elementType( QStringRef name ) const
{
    const QString stringName = name.toString();

    if ( stringName == lexemValue(TYPE_WIDGET) ) {
        return HbXml::WIDGET;
    } else if ( stringName == lexemValue(TYPE_OBJECT) ) {
        return HbXml::OBJECT;
    } else if ( stringName == lexemValue(TYPE_CONNECT) ) {
        return HbXml::CONNECT;
    } else if ( stringName == lexemValue(TYPE_LAYOUT) ) {
        return HbXml::LAYOUT;
    } else if ( stringName == lexemValue(TYPE_SPACERITEM) ) {
        return HbXml::SPACERITEM;
    } else if ( stringName == lexemValue(TYPE_CONTAINER) ) {
        return HbXml::CONTAINER;
    } else if ( stringName == lexemValue(TYPE_SECTION) ) {
        return HbXml::SECTION;
    } else if ( stringName == lexemValue(TYPE_REF) ) {
        return HbXml::REF;
    } else if ( ( stringName == lexemValue(TYPE_ICON) ) ||
                ( stringName == lexemValue(TYPE_INT) ) ||
                ( stringName == lexemValue(TYPE_REAL) ) ||
                ( stringName == lexemValue(TYPE_LOCALIZED_STRING) ) ||
                ( stringName == lexemValue(TYPE_STRING) ) ||
                ( stringName == lexemValue(TYPE_ENUMS) ) ||
                ( stringName == lexemValue(TYPE_BOOL) ) ||
                ( stringName == lexemValue(TYPE_SIZE) ) ||
                ( stringName == lexemValue(TYPE_RECT) ) ||
                ( stringName == lexemValue(TYPE_POINT) ) ||
                ( stringName == lexemValue(TYPE_COLOR) ) ||
                ( stringName == lexemValue(TYPE_FONTSPEC) )||
                ( stringName == lexemValue(LL_ORIENTATION) )||
                ( stringName == lexemValue(TYPE_ALIGNMENT) )) {
        return HbXml::PROPERTY;
    } else if ( ( stringName == lexemValue(TYPE_CONTENTSMARGINS) ) ||
                 ( stringName == lexemValue(TYPE_SIZEPOLICY) ) ||
                ( stringName == lexemValue(TYPE_SIZEHINT) ) ||
                ( stringName == lexemValue(TYPE_TOOLTIP) ) ) {
        return HbXml::VARIABLE;
    } else if ( stringName == lexemValue(TYPE_ZVALUE) ) {
        qWarning() << "zvalue variable in docml is deprecated. Use z property instead.";
        //return HbXml::DEPRECATED;
        return HbXml::VARIABLE;
    } else if ( stringName == lexemValue(TYPE_METADATA) ) {
        return HbXml::METADATA;
    }

    return HbXml::UNKNOWN;
}


QString HbXmlLoaderBaseSyntax::attribute( DocumentLexems lexem ) const
{
    return mReader.attributes().value( lexemValue(lexem) ).toString();
}

bool HbXmlLoaderBaseSyntax::toBool( const QString &boolString, bool &value ) const
{
    bool retVal(true);
    if (!boolString.compare("TRUE", Qt::CaseInsensitive)) {
        value = true;
    } else if (!boolString.compare("FALSE", Qt::CaseInsensitive)) {
        value = false;
    } else {
        retVal = false;
    }
    return retVal;
}

bool HbXmlLoaderBaseSyntax::toAnchorEdge( const QString &edgeString, Hb::Edge &edge ) const
{
    bool retVal(true);
    if( !edgeString.compare("TOP", Qt::CaseInsensitive) ) {
        edge = Hb::TopEdge;
    } else if( !edgeString.compare("BOTTOM", Qt::CaseInsensitive) ) {
        edge = Hb::BottomEdge;
    } else if( !edgeString.compare("LEFT", Qt::CaseInsensitive) ) {
        edge = Hb::LeftEdge;
    } else if( !edgeString.compare("RIGHT", Qt::CaseInsensitive) ) {
        edge = Hb::RightEdge;
    } else if( !edgeString.compare("CENTERH", Qt::CaseInsensitive) ) {
        edge = Hb::CenterHEdge;
    } else if( !edgeString.compare("CENTERV", Qt::CaseInsensitive) ) {
        edge = Hb::CenterVEdge;
    } else {
        retVal = false;
    }
    return retVal;
}

bool HbXmlLoaderBaseSyntax::toAnchorDir( const QString &dirString, HbAnchor::Direction &dir ) const
{
    bool retVal(true);
    if( !dirString.compare("NEGATIVE", Qt::CaseInsensitive) ) {
        dir = HbAnchor::Negative;
    } else if( !dirString.compare("POSITIVE", Qt::CaseInsensitive) ) {
        dir = HbAnchor::Positive;
    } else {
        retVal = false;
    }
    return retVal;
}

bool HbXmlLoaderBaseSyntax::toSizePolicy( const QString& policyS, QSizePolicy::Policy &policy ) const
{
    if ( policyS.isEmpty() ) {
        return false;
    }

    const QMetaObject *meta = &QSizePolicy::staticMetaObject;
    const int enumIndex = meta->indexOfEnumerator("Policy");
    Q_ASSERT( enumIndex != -1 );
    QMetaEnum metaEnum = meta->enumerator(enumIndex);
    const QByteArray byteArray = policyS.toUtf8();
    const int policyI = metaEnum.keyToValue(byteArray.data());

    if ( policyI == -1 ) {
        return false;
    }

    policy = (QSizePolicy::Policy)policyI;

    return true;
}

