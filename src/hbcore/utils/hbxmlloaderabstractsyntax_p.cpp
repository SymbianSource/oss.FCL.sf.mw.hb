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

#include "hbxmlloaderabstractsyntax_p.h"


#include <hbicon.h>
#include <hbinstance.h>
#include <QDebug>

/*
    \class HbXmlLoaderAbstractSyntax
    \internal
    \proto
*/

// Static table of lexems.
// These must be kept in sync with the DocumentLexems enum. 
static const char *knownLexems[HbXmlLoaderAbstractSyntax::NUMBER_OF_LEXEMS] = {
    "name",             // ATTR_NAME
    "type",             // ATTR_TYPE
    "value",            // ATTR_VALUE
    "iconName",         // ATTR_ICONNAME
    "width",            // ATTR_WIDTH
    "height",           // ATTR_HEIGHT
    "sender",           // ATTR_SRC
    "signal",           // ATTR_SIGNAL
    "receiver",         // ATTR_DST
    "slot",             // ATTR_SLOT
    "x",                // ATTR_X
    "y",                // ATTR_Y
    "plugin",           // ATTR_PLUGIN
    "role",             // ATTR_ROLE
    "object",           // ATTR_OBJECT
    "context",          // ATTR_CONTEXT
    "left",             // ATTR_LEFT
    "right",            // ATTR_RIGHT
    "top",              // ATTR_TOP
    "bottom",           // ATTR_BOTTOM
    "horizontalPolicy",     // ATTR_HORIZONTALPOLICY
    "verticalPolicy",       // ATTR_VERTICALPOLICY
    "horizontalStretch",    // ATTR_HORIZONTALSTRETCH
    "verticalStretch",      // ATTR_VERTICALSTRETCH
    "comment",          // ATTR_COMMENT
    "widget",           // ATTR_WIDGET
    "version",          // ATTR_VERSION
    "role",             // ATTR_FONTSPECROLE
    "textheight",       // ATTR_TEXTHEIGHT
    "stretchfactor",    // ATTR_STRETCHFACTOR


    "textpaneheight",   // ...Deprecated... ATTR_TEXTPANEHEIGHT
    "locid",            // ATTR_LOCID

    "hbdocument",       // TYPE_DOCUMENT
    "hbwidget",         // TYPE_HBWIDGET
    "object",           // TYPE_OBJECT
    "widget",           // TYPE_WIDGET
    "spaceritem",       // TYPE_SPACERITEM
    "connect",          // TYPE_CONNECT
    "layout",           // TYPE_LAYOUT
    "section",          // TYPE_SECTION
    "ref",              // TYPE_REF
    "contentsmargins",  // TYPE_CONTENTSMARGINS
    "sizepolicy",       // TYPE_SIZEPOLICY
    "sizehint",         // TYPE_SIZEHINT
    "zvalue",           // ...Deprecated... TYPE_ZVALUE
    "tooltip",          // TYPE_TOOLTIP
    "metadata",         // TYPE_METADATA
    "container",        // TYPE_CONTAINER
    "integer",          // TYPE_INT
    "real",             // TYPE_REAL
    "localizedstring",  // TYPE_LOCALIZED_STRING
    "string",           // TYPE_STRING
    "enums",            // TYPE_ENUMS
    "bool",             // TYPE_BOOL
    "icon",             // TYPE_ICON
    "size",             // TYPE_SIZE
    "rect",             // TYPE_RECT
    "point",            // TYPE_POINT
    "color",            // TYPE_COLOR
    "alignment",        // TYPE_ALIGNMENT
    "fontspec",         // TYPE_FONTSPEC

    "anchor",           // LAYOUT_ANCHOR
    "mesh",             // LAYOUT_MESH
    "",                 // LAYOUT_MESH_TARGET
    "",                 // LAYOUT_MESH_ALIEN
    "grid",             // LAYOUT_GRID
    "linear",           // LAYOUT_LINEAR
    "stacked",          // LAYOUT_STACK
    "null",             // LAYOUT_NULL

    "stringlist",       // CONTAINER_STRINGLIST
    "null",             // CONTAINER_NULL

    "TRUE",             // VALUE_BOOL_TRUE
    "FALSE",            // VALUE_BOOL_FALSE

    "un",               // UNIT_UNIT
    "px",               // UNIT_PIXEL
    "mm",               // UNIT_MILLIMETER
    "var(",             // UNIT_VAR_START
    "-var(",            // UNIT_VAR_NEG_START
    ")",                // UNIT_VAR_END
    "expr(",            // UNIT_EXPR_START
    "-expr(",           // UNIT_EXPR_NEG_START
    ")",                // UNIT_EXPR_END

    "anchoritem",       // AL_ANCHOR
    "src",              // AL_SRC_NAME
    "srcEdge",          // AL_SRC_EDGE
    "dst",              // AL_DST_NAME
    "dstEdge",          // AL_DST_EDGE
    "spacing",          // AL_SPACING
    "spacer",           // AL_SPACER

    "meshitem",         // ML_MESHITEM
    "src",              // ML_SRC_NAME
    "srcEdge",          // ML_SRC_EDGE
    "dst",              // ML_DST_NAME
    "dstEdge",          // ML_DST_EDGE
    "spacing",          // ML_SPACING
    "spacer",           // ML_SPACER

    "griditem",         // GL_GRIDCELL
    "itemname",         // GL_ITEMNAME
    "row",              // GL_ROW
    "column",           // GL_COLUMN
    "row_span",         // GL_ROWSPAN
    "column_span",      // GL_COLUMNSPAN
    "spacing",          // GL_SPACING
    "gridrow",          // GL_GRIDROW
    "gridcolumn",       // GL_GRIDCOLUMN
    "minwidth",         // GL_MINWIDTH
    "maxwidth",         // GL_MAXWIDTH
    "prefwidth",        // GL_PREFWIDTH
    "fixedwidth",       // GL_FIXEDWIDTH
    "minheight",        // GL_MINHEIGHT
    "maxheight",        // GL_MAXHEIGHT
    "prefheight",       // GL_PREFHEIGHT
    "fixedheight",      // GL_FIXEDHEIGHT

    "orientation",      // LL_ORIENTATION
    "linearitem",       // LL_LINEARITEM
    "stretchitem",      // LL_STRETCH
    "itemname",         // LL_ITEMNAME
    "index",            // LL_INDEX
    "spacing",          // LL_SPACING

    "stackitem",        // SL_STACKITEM
    "itemname",         // SL_ITEMNAME
    "index"             // SL_INDEX
};

HbXmlLoaderAbstractSyntax::HbXmlLoaderAbstractSyntax( HbXmlLoaderAbstractActions *actions )
: mActions(actions)
{
}

HbXmlLoaderAbstractSyntax::~HbXmlLoaderAbstractSyntax()
{
}

bool HbXmlLoaderAbstractSyntax::load( QIODevice *device, const QString &section )
{   
    // Initialize to some profile.
    mCurrentProfile = HbDeviceProfile::current(); 
    return loadDevice(device, section);
}

bool HbXmlLoaderAbstractSyntax::loadDevice(QIODevice *device, const QString &section)
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
                        if( ( mCurrentElementType == METADATA ) && ( mCurrentTokenType == QXmlStreamReader::StartElement ) ) {
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
                        if( mCurrentElementType == METADATA ) {
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
                mActions->deleteAll();
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

bool HbXmlLoaderAbstractSyntax::readDocument()
{
    bool result = false;
    switch( mDocumentState ) {
        case DS_START_DOCUMENT:
        {
            HB_DOCUMENTLOADER_PRINT( "DOCUMENT_STATE START DOCUMENT" );
            if( mCurrentElementType == DOCUMENT ) {
                result = processDocument();
                HB_DOCUMENTLOADER_PRINT( "SWITCHING TO READ SECTIONS STATE" );
                mDocumentState = DS_READ_SECTIONS;
            }
            break;
        }
        case DS_READ_SECTIONS:
        {
            if( ( mCurrentElementType == DOCUMENT ) && ( mCurrentTokenType == QXmlStreamReader::EndElement ) ) {
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

bool HbXmlLoaderAbstractSyntax::readAlienSection()
{
    if( mCurrentElementType == SECTION ) {

        if( mCurrentTokenType == QXmlStreamReader::StartElement ) {

            QString name = attribute( ATTR_NAME );
            if( name.isEmpty() ) {
                qWarning() << "Section witout a name, line " << mReader.lineNumber();
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

bool HbXmlLoaderAbstractSyntax::readTargetSection()
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

bool HbXmlLoaderAbstractSyntax::readContainerStartItem()
{
	qWarning() << "Internal error, wrong container type, line " << mReader.lineNumber();
    return false;
}

bool HbXmlLoaderAbstractSyntax::readContainerEndItem()
{
    HB_DOCUMENTLOADER_PRINT( "GENERAL CONTAINER END ITEM" );
    if( mCurrentElementType == CONTAINER ) {
        HB_DOCUMENTLOADER_PRINT( "GENERAL CONTAINER END ITEM : SWITCHING TO GENERAL ITEM PROCESSING MODE" );
        mElementState = ES_GENERAL_ITEM;
    }    
    return true;
}

bool HbXmlLoaderAbstractSyntax::readLayoutStartItem()
{
	qWarning() << "Internal error, wrong layout type, line " << mReader.lineNumber();
    return false;
}

bool HbXmlLoaderAbstractSyntax::readLayoutEndItem()
{
    HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT END ITEM" );
    if( mCurrentElementType == LAYOUT ) {
        HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT END ITEM : SWITCHING TO GENERAL ITEM PROCESSING MODE" );
        mElementState = ES_GENERAL_ITEM;
    }    
    return true;
}

bool HbXmlLoaderAbstractSyntax::readGeneralStartItem()
{
    bool result = false;
    switch( mCurrentElementType ) {
         case DOCUMENT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: DOCUMENT" );
            result = processDocument();
            break;
         }
         case LAYOUT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: LAYOUT" );
            result = processLayout();
            
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SWITCHING TO LAYOUT PROCESSING MODE" );
            mElementState = ES_LAYOUT_ITEM;
            break;
         }
         case CONTAINER:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: CONTAINER" );
            result = processContainer();
            
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SWITCHING TO CONTAINER PROCESSING MODE" );
            mElementState = ES_CONTAINER_ITEM;
            break;
         }
         case SECTION:
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

bool HbXmlLoaderAbstractSyntax::readGeneralEndItem()
{
    
    bool result = false;
    
    switch( mCurrentElementType ) {
         case LAYOUT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: LAYOUT - ERROR, CANNOT BE IN THIS STATE" );
            result = false;
            break;
         }
         case CONTAINER:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: CONTAINER - ERROR, CANNOT BE IN THIS STATE" );
            result = false;
            break;
         }
         case SECTION:
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


bool HbXmlLoaderAbstractSyntax::processDocument()
{
    return false;
}


bool HbXmlLoaderAbstractSyntax::processLayout()
{
	qWarning() << "Unknown layout type, line " << mReader.lineNumber();
    return false;
}

bool HbXmlLoaderAbstractSyntax::processContainer()
{
	qWarning() << "Unknown container type, line " << mReader.lineNumber();
    return false;
}



bool HbXmlLoaderAbstractSyntax::checkEndElementCorrectness()
{        
    if ( ! mActions->pop( elementType( mReader.name() ) ) ) {
        qWarning() << "Error in end element, line " << mReader.lineNumber(); 
        return false;
    }
    return true;
}

ElementType 
    HbXmlLoaderAbstractSyntax::elementType( QStringRef name ) const
{
    const QString stringName = name.toString();
    
    if( stringName == lexemValue(TYPE_WIDGET) ) {
        return WIDGET;
    } else  if( stringName == lexemValue(TYPE_OBJECT) ) {
        return OBJECT;
/*    } else  if( ( ( stringName == lexemValue(TYPE_DOCUMENT] ) && ( mMode == MODE_APPLICATION ) ) || 
                ( ( stringName == lexemValue(TYPE_HBWIDGET] ) && ( mMode == MODE_WIDGET ) ) ) {
        return DOCUMENT;*/
    } else  if( stringName == lexemValue(TYPE_CONNECT) ) {
        return CONNECT;
    } else  if( stringName == lexemValue(TYPE_LAYOUT) ) {
        return LAYOUT;
    } else  if( stringName == lexemValue(TYPE_SPACERITEM) ) {
        return SPACERITEM;
    } else  if( stringName == lexemValue(TYPE_CONTAINER) ) {
        return CONTAINER;
    } else  if( stringName == lexemValue(TYPE_SECTION) ) {
        return SECTION;
    } else  if( stringName == lexemValue(TYPE_REF) ) {
        return REF;
    } else  if( ( stringName == lexemValue(TYPE_ICON) ) || 
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
        return PROPERTY; 
    } else  if( ( stringName == lexemValue(TYPE_CONTENTSMARGINS) ) ||
                 ( stringName == lexemValue(TYPE_SIZEPOLICY) ) ||
                ( stringName == lexemValue(TYPE_SIZEHINT) ) || 
                ( stringName == lexemValue(TYPE_TOOLTIP) ) ) {
        return VARIABLE;
    } else if( stringName == lexemValue(TYPE_ZVALUE) ) {
        qWarning() << "zvalue variable in docml is deprecated. Use z property instead.";
        //return DEPRECATED;
        return VARIABLE;
    } else if( stringName == lexemValue(TYPE_METADATA) ) {
        return METADATA;
    }
    
    return UNKNOWN;
}


QString HbXmlLoaderAbstractSyntax::attribute( DocumentLexems lexem ) const
{
    return mReader.attributes().value( lexemValue(lexem) ).toString();
}

bool HbXmlLoaderAbstractSyntax::toReal(const QString &value, qreal& result) const
{
    bool ok = false;
    result = qreal(value.toDouble(&ok));
    if ( !ok ) {
        qWarning() << "Could not convert value to real, line " << mReader.lineNumber(); 
        result = (qreal)0.0;
    }
    return ok;
}

bool HbXmlLoaderAbstractSyntax::toPixels(const QString &value, qreal& result) const
{
	// call the toPixels function with the mCurrentProfile.
    return toPixels(mCurrentProfile,value,result);
}

bool HbXmlLoaderAbstractSyntax::toPixels(const HbDeviceProfile &deviceProfile,
                                         const QString &value, qreal& result) const
{
    QString val = value;
    val.reserve(val.length());
    enum { None, Px, Un, Mm } unit = None;
    if( val.endsWith(lexemValue(UNIT_UNIT), Qt::CaseInsensitive) ) {
        unit = Un;
    } else if( val.endsWith(lexemValue(UNIT_PIXEL), Qt::CaseInsensitive) ) {
        unit = Px;
    } else if( val.endsWith(lexemValue(UNIT_MILLIMETER), Qt::CaseInsensitive) ) {
        unit = Mm;
    } else if( (val.startsWith( lexemValue(UNIT_VAR_START) ) ||
                val.startsWith( lexemValue(UNIT_VAR_NEG_START) ) ||
                val.startsWith( lexemValue(UNIT_EXPR_START) ) ||
                val.startsWith( lexemValue(UNIT_EXPR_NEG_START) )) && val.endsWith( lexemValue(UNIT_VAR_END) ) ) {
        //variable or expression is decided by the start of value string
        return hbInstance->style()->parameter( val, result, deviceProfile );
    }

    if (unit != None) {
        // Assuming all unit identifiers have two characters
        val.chop(2);
    }

    if ( !toReal( val, result ) ) {
        return false;
    }

    if (unit == Un) {
        result = deviceProfile.unitValue() * result;
    } else if (unit == Mm) {
        result = deviceProfile.ppmValue() * result;
    } // else -> already in pixels
    return true;
}

const char *HbXmlLoaderAbstractSyntax::lexemValue(DocumentLexems lex) const
{
    return knownLexems[lex];
}

