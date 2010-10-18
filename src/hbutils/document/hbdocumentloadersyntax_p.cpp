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

#include "hbdocumentloadersyntax_p.h"
#include "hbdocumentloaderactions_p.h"
#include "hbdocumentloader_p.h"
#include "hbdocumentloader.h"

#include <hbfontspec.h>

#include <QMetaEnum>
#include <QTranslator>
#include <QDataStream>

#include <QDebug>


// Document loader version number
#define VERSION_MAJOR 1
#define VERSION_MINOR 3

#define MIN_SUPPORTED_VERSION_MAJOR 0
#define MIN_SUPPORTED_VERSION_MINOR 1


//#define LINE_DEBUGGING

#ifdef LINE_DEBUGGING
#include <QTime>
static QTime lineDebugTime;

#ifdef Q_OS_SYMBIAN
#include <e32debug.h>
#endif

void doDebugPrint(const char* text)
{
#ifdef Q_OS_SYMBIAN
    RDebug::Printf(text);
#else
    qDebug() << text;
#endif
}

// Takes standard c-format.
void debugPrint(const char* cformat, ...)
{
    va_list ap;
    va_start(ap, cformat);
    QString str = QString().vsprintf(cformat, ap);
    va_end(ap);
    doDebugPrint(str.toAscii().constData());
}

#endif // LINE_DEBUGGING

const char *ZValueProperty = "z";

class AccessToMetadata : public QObject
    {
    public:
        int getEnumValue( const char *enumeration, const char *str )
            {
                QMetaObject metaobject = staticQtMetaObject;
                QMetaEnum e = metaobject.enumerator( metaobject.indexOfEnumerator( enumeration ) );
                return e.keysToValue( str );
            }
    };

static bool toFontSpecRole(const QString &roleString, HbFontSpec::Role &role)
{
    bool success(false);
    int enumInt = HbFontSpec::staticMetaObject.enumerator(
            HbFontSpec::staticMetaObject.indexOfEnumerator("Role")).keyToValue(roleString.toLatin1());
    if (enumInt >= 0) {
        success = true;
        role = static_cast<HbFontSpec::Role>(enumInt);
    }
    return success;
}

/*
    \class HbDocumentLoaderSyntax
    \internal
    \proto
*/

HbDocumentLoaderSyntax::HbDocumentLoaderSyntax( HbXmlLoaderAbstractActions *actions )
: HbXmlLoaderBaseSyntax( actions )
{
}

HbDocumentLoaderSyntax::~HbDocumentLoaderSyntax()
{
}

bool HbDocumentLoaderSyntax::scanForSections( QIODevice *device, QList<QString> &sectionsList )
{
#ifdef LINE_DEBUGGING
    lineDebugTime.restart();
    debugPrint("MYTRACE: DocML scanForSections, start");
#endif
    bool ret = HbXmlLoaderBaseSyntax::scanForSections( device, sectionsList );
#ifdef LINE_DEBUGGING
    debugPrint("MYTRACE: DocML scanForSections, end: %d", lineDebugTime.elapsed());
#endif
    return ret;
}

bool HbDocumentLoaderSyntax::load( QIODevice *device, const QString &section )
{
#ifdef LINE_DEBUGGING
    lineDebugTime.restart();
    debugPrint("MYTRACE: DocML load, start");
#endif
    bool ret = HbXmlLoaderBaseSyntax::loadDevice( device, section );
#ifdef LINE_DEBUGGING
    debugPrint("MYTRACE: DocML load, end: %d", lineDebugTime.elapsed());
#endif
    return ret;
}

bool HbDocumentLoaderSyntax::readLayoutStartItem()
{
    bool result = false;
    switch( mCurrentLayoutType ) {
        case LAYOUT_ANCHOR:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: ANCHOR ITEM" );
            if( mReader.name() == lexemValue( AL_ANCHOR ) ) {
                result = readAnchorLayoutStartItem(false);
            } else if( mReader.name() == lexemValue( AL_MAPPING ) ) {
                const QString item = attribute( AL_MAPPING_ITEM );
                const QString id = attribute( AL_MAPPING_ID );
                const QString action = attribute( ATTR_ACTION );
                bool remove = false;
                if ( !action.isEmpty() ) {
                    if (!action.compare("remove", Qt::CaseInsensitive)) {
                        remove = true;
                    } else if (action.compare("set", Qt::CaseInsensitive)) {
                        qWarning() << "Invalid anchoritem action, line " << mReader.lineNumber();
                        return false;
                    }
                }

                result = mActions->setAnchorLayoutMapping( item, id, remove );
            }
            break;
        }
        case LAYOUT_GRID:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: GRID ITEM" );
            if( mReader.name() == lexemValue( GL_GRIDCELL ) ) {

                const QString src = attribute( GL_ITEMNAME );
                const QString row = attribute( GL_ROW );
                const QString column = attribute( GL_COLUMN );
                const QString rowspan = attribute( GL_ROWSPAN );
                const QString columnspan = attribute( GL_COLUMNSPAN );
                const QString alignment = attribute( TYPE_ALIGNMENT );

                int rownum = row.toInt( &result );
                if (!result) {
                    HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO ROW SPECIFIED" ) );
                    break;
                }

                int columnnum = column.toInt( &result );
                if (!result) {
                    HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO COLUMN SPECIFIED" ) );
                    break;
                }

                int rowspannum;
                int *rowspan_p = 0;
                if (!rowspan.isEmpty()) {
                    rowspannum = rowspan.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INVALID ROWSPAN" ) );
                        break;
                    } else {
                        rowspan_p = &rowspannum;
                    }
                }

                int columnspannum;
                int *columnspan_p = 0;
                if (!columnspan.isEmpty()) {
                    columnspannum = columnspan.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: INVALID ROWSPAN" ) );
                        break;
                    } else {
                        columnspan_p = &columnspannum;
                    }
                }

                Qt::Alignment align;
                Qt::Alignment *align_p = 0;
                if (!alignment.isEmpty()) {
                    AccessToMetadata myAccess;
                    int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
                    if (value == -1) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ALIGNMENT " ) + alignment );
                        result = false;
                        break;
                    } else {
                        align = (Qt::Alignment)value;
                        align_p = &align;
                    }
                }
                result = mActions->addGridLayoutCell( src, rownum, columnnum, rowspan_p, columnspan_p, align_p );
            } else if( mReader.name() == lexemValue( GL_GRIDROW ) ) {
                const QString row = attribute( GL_ROW );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );
                const QString alignment = attribute( TYPE_ALIGNMENT );

                const int rownum = row.toInt( &result );
                if (!result) {
                    HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO ROW NUMBER SPECIFIED FOR STRETCH FACTOR" ) );
                    break;
                }

                int stretchnum;
                int *stretch_p = 0;
                if( !stretchfactor.isEmpty() ) {
                    stretchnum = stretchfactor.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: UNABLE TO PARSE ROW STRETCH FACTOR VALUE" ) );
                        break;
                    } else {
                        stretch_p = &stretchnum;
                    }
                }

                Qt::Alignment align;
                Qt::Alignment *align_p = 0;
                if( !alignment.isEmpty() ) {
                    AccessToMetadata myAccess;

                    int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
                    if( value == -1 ) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH ROW ALIGNMENT " ) + alignment );
                        result = false;
                        break;
                    } else {
                        align = (Qt::Alignment)value;
                        align_p = &align;
                    }
                }

                result = mActions->setGridLayoutRowProperties( rownum, stretch_p, align_p );
                if (result) {
                    result = processRowHeights( rownum );
                }
            } else if( mReader.name() == lexemValue( GL_GRIDCOLUMN ) ) {
                const QString column = attribute( GL_COLUMN );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );
                const QString alignment = attribute( TYPE_ALIGNMENT );

                const int columnnum = column.toInt( &result );
                if (!result) {
                    HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO COLUMN NUMBER SPECIFIED FOR STRETCH FACTOR" ) );
                    break;
                }

                int stretchnum;
                int *stretch_p = 0;
                if (!stretchfactor.isEmpty()) {
                    stretchnum = stretchfactor.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: UNABLE TO PARSE COLUMN STRETCH FACTOR VALUE" ) );
                        break;
                    } else {
                        stretch_p = &stretchnum;
                    }
                }

                Qt::Alignment align;
                Qt::Alignment *align_p = 0;
                if (!alignment.isEmpty()) {
                    AccessToMetadata myAccess;

                    int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
                    if (value == -1) {
                        HB_DOCUMENTLOADER_PRINT( QString( "GRIDLAYOUT: NO SUCH COLUMN ALIGNMENT " ) + alignment );
                        result = false;
                        break;
                    } else {
                        align = (Qt::Alignment)value;
                        align_p = &align;
                    }
                }

                result = mActions->setGridLayoutColumnProperties( columnnum, stretch_p, align_p );
                if (result) {
                    result = processColumnWidths( columnnum );
                }
            } else if( mReader.name() == lexemValue( TYPE_CONTENTSMARGINS ) ) {
                result = processContentsMargins();
            }
            break;
        }
        case LAYOUT_LINEAR:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: LINEAR ITEM" );
            if( mReader.name() == lexemValue( LL_LINEARITEM ) ) {
                result = true;
                const QString itemname = attribute( LL_ITEMNAME );
                const QString index = attribute( LL_INDEX );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );
                const QString alignment = attribute( TYPE_ALIGNMENT );
                const QString spacing = attribute( LL_SPACING );

                int indexnum;
                int *index_p = 0;
                if (!index.isEmpty()) {
                    indexnum = index.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE ITEM INDEX" ) );
                        break;
                    } else {
                        index_p = &indexnum;
                    }
                }

                int stretchnum;
                int *stretch_p = 0;
                if (!stretchfactor.isEmpty()) {
                    stretchnum = stretchfactor.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE STRETCH VALUE" ) );
                        break;
                    } else {
                        stretch_p = &stretchnum;
                    }
                }

                Qt::Alignment align;
                Qt::Alignment *align_p = 0;
                if (!alignment.isEmpty()) {
                    AccessToMetadata myAccess;

                    int value = myAccess.getEnumValue( "Alignment", alignment.toLatin1().data() );
                    if (value == -1) {
                        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ITEM ALIGNMENT " ) + alignment );
                        result = false;
                        break;
                    } else {
                        align = (Qt::Alignment)value;
                        align_p = &align;
                    }
                }

                HbXmlLengthValue spacingValue;
                if (!spacing.isEmpty()) {
                    result = toLengthValue( spacing, spacingValue );
                }
                if (result) {
                    result = mActions->addLinearLayoutItem( itemname, index_p, stretch_p, align_p, spacingValue );
                }
            } else if( mReader.name() == lexemValue( LL_STRETCH ) ) {
                const QString index = attribute( LL_INDEX );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );

                int indexnum;
                int *index_p = 0;
                if (!index.isEmpty()) {
                    indexnum = index.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE STRETCH INDEX" ) );
                        break;
                    } else {
                        index_p = &indexnum;
                    }
                }

                int stretchnum;
                int *stretch_p = 0;
                if (!stretchfactor.isEmpty()) {
                    stretchnum = stretchfactor.toInt( &result );
                    if (!result) {
                        HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: UNABLE TO PARSE STRETCH VALUE" ) );
                        break;
                    } else {
                        stretch_p = &stretchnum;
                    }
                }

                result = mActions->addLinearLayoutStretch( index_p, stretch_p );
            } else if( mReader.name() == lexemValue( TYPE_CONTENTSMARGINS ) ) {
                result = processContentsMargins();
            }
            break;
        }
        case LAYOUT_STACK:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: STACK ITEM" );
            if( mReader.name() == lexemValue( SL_STACKITEM ) ) {

                const QString index = attribute( SL_INDEX );
                const QString itemname = attribute( SL_ITEMNAME );

                int indexnum;
                int *index_p = 0;
                if( !index.isEmpty() ) {
                    indexnum = index.toInt( &result );
                    if( !result ) {
                        HB_DOCUMENTLOADER_PRINT( QString( "STACKEDLAYOUT: UNABLE TO PARSE ITEM INDEX" ) );
                        break;
                    } else {
                        index_p = &indexnum;
                    }
                }
                result = mActions->addStackedLayoutItem( itemname, index_p );
            }
            break;

        }
        case LAYOUT_NULL:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: NULL ITEM (MUST NOT EXIST)" );
            break;
        }
        default:
        {
            qWarning() << "Internal error, wrong layout type, line " << mReader.lineNumber();
        }
    }
#ifdef LINE_DEBUGGING
    debugPrint("MYTRACE: --- after line %d, time: %d", (int)mReader.lineNumber(), lineDebugTime.elapsed());
#endif
    return result;
}

bool HbDocumentLoaderSyntax::processContentsMargins()
{
    const QString leftS = attribute( ATTR_LEFT );
    const QString topS = attribute( ATTR_TOP );
    const QString rightS = attribute( ATTR_RIGHT );
    const QString bottomS = attribute( ATTR_BOTTOM );

    bool result = true;
    HbXmlLengthValue left, top, right, bottom;
    if ( !leftS.isEmpty() ) {
        result = toLengthValue(leftS, left);
    }
    if ( result && !topS.isEmpty() ) {
        result = toLengthValue(topS, top);
    }
    if ( result && !rightS.isEmpty() ) {
        result = toLengthValue(rightS, right);
    }
    if ( result && !bottomS.isEmpty() ) {
        result = toLengthValue(bottomS, bottom);
    }

    if ( result ) {
        result = mActions->setLayoutContentsMargins( left, top, right, bottom );
    }

    if (!result) {
        qWarning() << "Invalid contents margins, line " << mReader.lineNumber();
    }

    return result;
}

bool HbDocumentLoaderSyntax::processRowHeights( int row )
{
    const QString minHeightS = attribute( GL_MINHEIGHT );
    const QString maxHeightS = attribute( GL_MAXHEIGHT );
    const QString prefHeightS = attribute( GL_PREFHEIGHT );
    const QString fixedHeightS = attribute( GL_FIXEDHEIGHT );
    const QString rowSpacingS = attribute( GL_SPACING );
    HbXmlLengthValue minHeight, maxHeight, prefHeight, fixedHeight, rowSpacing;

    bool result = true;

    if ( !minHeightS.isEmpty() ) {
        result = toLengthValue(minHeightS, minHeight);
    }

    if ( result && !maxHeightS.isEmpty() ) {
        result = toLengthValue(maxHeightS, maxHeight);
    }

    if ( result && !prefHeightS.isEmpty() ) {
        result = toLengthValue(prefHeightS, prefHeight);
    }

    if ( result && !fixedHeightS.isEmpty() ) {
        result = toLengthValue(fixedHeightS, fixedHeight);
    }

    if ( result && !rowSpacingS.isEmpty() ) {
        result = toLengthValue(rowSpacingS, rowSpacing);
    }

    if ( result ) {
        result = mActions->setGridLayoutRowHeights(
            row, minHeight, maxHeight, prefHeight, fixedHeight, rowSpacing);
    }

    return result;
}

bool HbDocumentLoaderSyntax::processColumnWidths( int column )
{
    const QString minWidthS = attribute( GL_MINWIDTH );
    const QString maxWidthS = attribute( GL_MAXWIDTH );
    const QString prefWidthS = attribute( GL_PREFWIDTH );
    const QString fixedWidthS = attribute( GL_FIXEDWIDTH );
    const QString columnSpacingS = attribute( GL_SPACING );
    HbXmlLengthValue minWidth, maxWidth, prefWidth, fixedWidth, columnSpacing;

    bool result = true;

    if ( !minWidthS.isEmpty() ) {
        result = toLengthValue(minWidthS, minWidth);
    }

    if ( result && !maxWidthS.isEmpty() ) {
        result = toLengthValue(maxWidthS, maxWidth);
    }

    if ( result && !prefWidthS.isEmpty() ) {
        result = toLengthValue(prefWidthS, prefWidth);
    }

    if ( result && !fixedWidthS.isEmpty() ) {
        result = toLengthValue(fixedWidthS, fixedWidth);
    }

    if ( result && !columnSpacingS.isEmpty() ) {
        result = toLengthValue(columnSpacingS, columnSpacing);
    }

    if ( result ) {
        result = mActions->setGridLayoutColumnWidths(
            column, minWidth, maxWidth, prefWidth, fixedWidth, columnSpacing);
    }

    return result;
}

bool HbDocumentLoaderSyntax::checkEndElementCorrectness()
{
    return HbXmlLoaderBaseSyntax::checkEndElementCorrectness();
}



bool HbDocumentLoaderSyntax::readContainerStartItem()
{
    bool result = false;
    switch ( mCurrentElementType ) {
         case HbXml::PROPERTY:
         {
            HB_DOCUMENTLOADER_PRINT( "CONTAINER START ITEM: PROPERTY" );

            switch( mCurrentContainerType ) {
                case CONTAINER_STRINGLIST:
                {
                    // check that we are only trying to put strings into a string list

                    HB_DOCUMENTLOADER_PRINT( "GENERAL CONTAINER START ITEM: STRING LIST" );
                    if( mReader.name() == lexemValue( TYPE_STRING )
                        || mReader.name() == lexemValue( TYPE_ENUMS )
                        || mReader.name() == lexemValue( TYPE_LOCALIZED_STRING ) ) {
                        result = processContainedProperty();
                        }
                    break;
                }
                case CONTAINER_NULL:
                {
                    HB_DOCUMENTLOADER_PRINT( "GENERAL CONTAINER START ITEM: NULL ITEM (MUST NOT EXIST)" );
                    break;
                }
                default:
                {
                    qWarning() << "Internal error, wrong container type, line " << mReader.lineNumber();
                    break;
                }
            }
            break;
         }
         default:
         {
             break;
         }
    }

    return result;
}

bool HbDocumentLoaderSyntax::readContainerEndItem()
{

    bool result = false;
    QString currentPropertyName;
    QVariant variant;

    switch( mCurrentElementType ) {
        case HbXml::CONTAINER:
        {
            currentPropertyName = mCurrentContainerNames.back();
            mCurrentContainerNames.removeLast();

            result = mActions->pushContainer(currentPropertyName.toLatin1(), mCurrentContainerType, mCurrentContainer);

            HB_DOCUMENTLOADER_PRINT( "CONTAINER END ITEM : SWITCHING TO GENERAL ITEM PROCESSING MODE" );
            mElementState = ES_GENERAL_ITEM;
            break;
        }
        default:
        {
            result = HbXmlLoaderBaseSyntax::readGeneralEndItem();
            break;
        }
    }
    return result;
}

bool HbDocumentLoaderSyntax::readGeneralStartItem()
{
    bool result = false;
    switch( mCurrentElementType ) {
         case HbXml::OBJECT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: OBJECT" );
            result = processObject();
            break;
         }
         case HbXml::WIDGET:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: WIDGET" );
            result = processWidget();
            break;
         }
         case HbXml::SPACERITEM:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SPACERITEM" );
            qWarning() << "spaceritem is deprecated " << mReader.lineNumber();
            result = true;
            break;
         }
         case HbXml::CONNECT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: CONNECT" );
            result = processConnect();
            break;
         }
         case HbXml::PROPERTY:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: PROPERTY" );
            result = processProperty();
            break;
         }
         case HbXml::REF:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: REF" );
            result = processRef();
            break;
         }
         case HbXml::VARIABLE:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: VARIABLE" );
            result = processVariable();
            break;
         }
         case HbXml::DEPRECATED:
         {
             HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: DEPRECATED" );
             result = true;
             break;
         }
         default:
         {
              result = HbXmlLoaderBaseSyntax::readGeneralStartItem();
         }
    }
#ifdef LINE_DEBUGGING
    debugPrint("MYTRACE: --- after line %d, time: %d", (int)mReader.lineNumber(), lineDebugTime.elapsed());
#endif
    return result;
}

bool HbDocumentLoaderSyntax::processDocument()
{
    bool ok, ok1, ok2, res = true;

    QString ver_str =  attribute( ATTR_VERSION );

    ver_str.toDouble( &ok );
    QStringList ver = ver_str.split( '.' );

    if( ( !ok ) || ( ver.size() != 2 ) ) {
        qWarning() << "Wrong document version format " << mReader.lineNumber();
        return false;
    }

    int major = ver.at(0).toInt( &ok1 );
    int minor = ver.at(1).toInt( &ok2 );

    if( ( !ok1 ) || ( !ok2 ) ) {
        qWarning() << "Wrong document version format " << mReader.lineNumber();
        return false;
    }


    if( ( major > VERSION_MAJOR ) || ( major < MIN_SUPPORTED_VERSION_MAJOR ) ) {
        res = false;
    } else if( ( ( major == VERSION_MAJOR ) && ( minor > VERSION_MINOR )  ) ||
               ( ( major == MIN_SUPPORTED_VERSION_MAJOR ) && ( minor < MIN_SUPPORTED_VERSION_MINOR ) ) ) {
        res = false;
    }

    if( ! res ) {
        qWarning() << "Not supported document version " + ver_str + ". Current parser version is: " + version();
        return false;

    }
    return mActions->pushDocument( attribute( ATTR_CONTEXT ) );
}

bool HbDocumentLoaderSyntax::processObject()
{
    const QString type = attribute( ATTR_TYPE );
    const QString name = attribute( ATTR_NAME );

    bool pushOK = mActions->pushObject( type, name );
    if( !pushOK ) {
        qWarning() << "Error in object processing, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processWidget()
{
    const QString type = attribute( ATTR_TYPE );
    const QString name = attribute( ATTR_NAME );
    const QString role = attribute( ATTR_ROLE );
    const QString plugin = attribute( ATTR_PLUGIN );
    bool pushOK = mActions->pushWidget( type, name, role, plugin);
    if( !pushOK ) {
        qWarning() << "Error in widget processing, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processLayout()
{
    bool result = false;
    const QString layout_type = attribute( ATTR_TYPE );
    const QString widget = attribute( ATTR_WIDGET );
    const QString action = attribute( ATTR_ACTION );
    bool modify = false;
    if ( !action.isEmpty() ) {
        if (!action.compare("modify", Qt::CaseInsensitive)) {
            modify = true;
        } else if (action.compare("create", Qt::CaseInsensitive)) {
            qWarning() << "Invalid layout action, line " << mReader.lineNumber();
            return false;
        }
    }

    if( layout_type == lexemValue( LAYOUT_ANCHOR ) ) {

        mCurrentLayoutType = LAYOUT_ANCHOR;
        result = mActions->createAnchorLayout( widget, modify );

    } else if( layout_type == lexemValue( LAYOUT_GRID ) ) {

        result = true;
        mCurrentLayoutType = LAYOUT_GRID;
        const QString spacing = attribute( GL_SPACING );
        HbXmlLengthValue spacingValue;
        if( !spacing.isEmpty() ) {
            result = toLengthValue( spacing, spacingValue );
        }
        if (result) {
            result = mActions->createGridLayout( widget, spacingValue, modify );
        }

    } else if( layout_type == lexemValue( LAYOUT_LINEAR ) ) {

        result = true;
        mCurrentLayoutType = LAYOUT_LINEAR;
        const QString orientation = attribute( LL_ORIENTATION );


        Qt::Orientation orient;
        Qt::Orientation *orient_p = 0;
        if (!orientation.isEmpty()) {
            AccessToMetadata myAccess;
            int value = myAccess.getEnumValue( "Orientation", orientation.toLatin1().data() );
            if (value == -1) {
                HB_DOCUMENTLOADER_PRINT( QString( "LINEARLAYOUT: NO SUCH ORIENTATION " ) + orientation );
                result = false;
            } else {
                orient = (Qt::Orientation)value;
                orient_p = &orient;
            }
        }

        const QString spacing = attribute( LL_SPACING );
        HbXmlLengthValue spacingValue;
        if( result && !spacing.isEmpty() ) {
            result = toLengthValue( spacing, spacingValue );
        }
        if (result) {
            result = mActions->createLinearLayout( widget, orient_p, spacingValue, modify );
        }

    } else if( layout_type == lexemValue( LAYOUT_STACK ) ) {

        mCurrentLayoutType = LAYOUT_STACK;
        result = mActions->createStackedLayout( widget, modify );

    } else if( layout_type == lexemValue( LAYOUT_NULL ) ) {

        mCurrentLayoutType = LAYOUT_NULL;
        result = mActions->createNullLayout( widget );

    } else {
        return HbXmlLoaderBaseSyntax::processLayout();
    }

    if( !result ) {
        qWarning() << "Unable to create layout, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processConnect()
{
    const QString srcName = attribute( ATTR_SRC );
    const QString signalName = attribute( ATTR_SIGNAL );
    const QString dstName = attribute( ATTR_DST );
    const QString slotName = attribute( ATTR_SLOT );

    bool pushOK = mActions->pushConnect( srcName, signalName, dstName, slotName );
    if( !pushOK ) {
        qWarning() << "Error in connect processing, line " << mReader.lineNumber();
        return false;

    }
    return true;
}

bool HbDocumentLoaderSyntax::processContainer()
{
    bool result = true;
    const QString container_type = attribute( ATTR_TYPE );

    if( container_type == lexemValue( CONTAINER_STRINGLIST ) ) {

        mCurrentContainerType = CONTAINER_STRINGLIST;

        const QString propertyName = attribute ( ATTR_NAME );
        if (propertyName.isEmpty()) {
            qWarning() << "No property name defined, line " << mReader.lineNumber();
            result = false;
        }
        if (result) {
            mCurrentContainerNames << propertyName;
            qDeleteAll(mCurrentContainer);
            mCurrentContainer.clear();
        }
    } else {
        result = HbXmlLoaderBaseSyntax::processContainer();
        if( !result ) {
            qWarning() << "Unable to create container, line " << mReader.lineNumber();
            return false;
        }
    }
    return result;
}

bool HbDocumentLoaderSyntax::processContainedProperty()
{
    HbXmlVariable *variable = new HbXmlVariable();
    if ( !createVariable(*variable) ) {
        qWarning() << "Invalid property, line " << mReader.lineNumber();
        delete variable;
        return false;
    }

    mCurrentContainer.append(variable);
    return true;
}

bool HbDocumentLoaderSyntax::processProperty()
{
    HbXmlVariable variable;
    if ( !createVariable(variable) ) {
        qWarning() << "Invalid property, line " << mReader.lineNumber();
        return false;
    }

    const QString propertyName = attribute( ATTR_NAME );

    if( propertyName.isEmpty() ) {
        qWarning() << "No property name defined, line " << mReader.lineNumber();
        return false;
    }

    bool pushOK = mActions->pushProperty( propertyName.toLatin1(), variable );
    if( !pushOK ) {
        qWarning() << "Unable to set property, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processRef()
{
    const QString objectName = attribute( ATTR_OBJECT );
    const QString role = attribute( ATTR_ROLE );

    bool pushOK = mActions->pushRef( objectName, role );
    if( !pushOK ) {
        qWarning() << "Error in reference processing, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processVariable()
{
    bool result = false;
    const QString type = mReader.name().toString();

    if( type == lexemValue( TYPE_CONTENTSMARGINS ) ) {

        const QString leftS = attribute( ATTR_LEFT );
        const QString topS = attribute( ATTR_TOP );
        const QString rightS = attribute( ATTR_RIGHT );
        const QString bottomS = attribute( ATTR_BOTTOM );

        result = true;
        HbXmlLengthValue left, top, right, bottom;
        if ( !leftS.isEmpty() ) {
            result = toLengthValue(leftS, left);
        }
        if ( result && !topS.isEmpty() ) {
            result = toLengthValue(topS, top);
        }
        if ( result && !rightS.isEmpty() ) {
            result = toLengthValue(rightS, right);
        }
        if ( result && !bottomS.isEmpty() ) {
            result = toLengthValue(bottomS, bottom);
        }

        if ( result ) {
            result = mActions->setContentsMargins( left, top, right, bottom );
        }

        if (!result) {
            qWarning() << "Invalid contents margins, line " << mReader.lineNumber();
        }

    } else if ( type == lexemValue( TYPE_SIZEPOLICY ) ) {
        const QString horizontalPolicyS = attribute( ATTR_HORIZONTALPOLICY );
        const QString verticalPolicyS = attribute( ATTR_VERTICALPOLICY );
        const QString horizontalStretchS = attribute( ATTR_HORIZONTALSTRETCH );
        const QString verticalStretchS = attribute( ATTR_VERTICALSTRETCH );

        result = true;

        QSizePolicy::Policy hPol;
        QSizePolicy::Policy *hPolP = 0;
        if ( !horizontalPolicyS.isEmpty() ) {
            result = toSizePolicy( horizontalPolicyS, hPol );
            hPolP = &hPol;
        }

        QSizePolicy::Policy vPol;
        QSizePolicy::Policy *vPolP = 0;
        if ( result && !verticalPolicyS.isEmpty() ) {
            result = toSizePolicy( verticalPolicyS, vPol );
            vPolP = &vPol;
        }

        int hStretch;
        int *hStretchP = 0;
        if ( result && !horizontalStretchS.isEmpty() ) {
            const int intValue = horizontalStretchS.toInt( &result );
            if ( result ) {
                if ( intValue >= 0 && intValue < 256 ) {
                    hStretch = intValue;
                    hStretchP = &hStretch;
                } else {
                    result = false;
                }
            }
        }

        int vStretch;
        int *vStretchP = 0;
        if ( result && !verticalStretchS.isEmpty() ) {
            const int intValue = verticalStretchS.toInt( &result );
            if ( result ) {
                if ( intValue >= 0 && intValue < 256 ) {
                    vStretch = intValue;
                    vStretchP = &vStretch;
                } else {
                    result = false;
                }
            }
        }

        if ( result ) {
            result = mActions->setSizePolicy( hPolP, vPolP, hStretchP, vStretchP );
        }

        if (!result) {
            qWarning() << "Invalid size policy, line " << mReader.lineNumber();
        }

    } else if ( type == lexemValue( TYPE_SIZEHINT ) ) {

        Qt::SizeHint hint = Qt::PreferredSize;
        bool fixed = false;

        if (convertSizeHintType(attribute( ATTR_TYPE ), hint, fixed)) {

            result = true;

            HbXmlLengthValue sizeHintWidth;
            const QString width = attribute( ATTR_WIDTH );
            if (!width.isEmpty()) {
                result = toLengthValue(width, sizeHintWidth);
            }

            HbXmlLengthValue sizeHintHeight;
            const QString height = attribute( ATTR_HEIGHT );
            if (result && !height.isEmpty()) {
                result = toLengthValue(height, sizeHintHeight);
            }

            if (result) {
                result = mActions->setSizeHint(hint, sizeHintWidth, sizeHintHeight, fixed);
            }
        }

        if (!result) {
            qWarning() << "Invalid size hint, line " << mReader.lineNumber();
        }
    } else if ( type == lexemValue( TYPE_ZVALUE ) ) {
        const QString value = attribute( ATTR_VALUE );
        if (!value.isEmpty()) {
            HbXmlLengthValue *value_res = new HbXmlLengthValue();
            result = toLengthValue( value, *value_res );
            if( result ) {
                HbXmlVariable variable;
                variable.mType = HbXmlVariable::REAL;
                variable.mParameters.append( value_res );
                result = mActions->pushProperty( ZValueProperty, variable );
            } else {
                delete value_res;
            }
        }

        if (!result) {
            qWarning() << "Invalid z value, line " << mReader.lineNumber();
        }
    } else if ( type == lexemValue( TYPE_TOOLTIP ) ) {
        const QString value = attribute( ATTR_VALUE );
        const QString comment = attribute( ATTR_COMMENT );
        const QString locId = attribute( ATTR_LOCID );

        HbXmlVariable variable;

        QString *param1 = new QString();
        QString *param2 = new QString();

        if (!locId.isEmpty()) {
            variable.mType = HbXmlVariable::STRING;
            *param1 = value;
            *param2 = locId;
        } else {
            variable.mType = HbXmlVariable::LOCALIZED_STRING;
            *param1 = value;
            *param2 = comment;
        }
        variable.mParameters.append(param1);
        variable.mParameters.append(param2);

        result = mActions->setToolTip(variable);

        if (!result) {
            qWarning() << "Invalid tooltip, line " << mReader.lineNumber();
        }
    } else if ( type == lexemValue( TYPE_BACKGROUND ) ) {
        const QString name = attribute( ATTR_NAME );
        const QString type = attribute( ATTR_TYPE );

        result = true;

        HbFrameDrawer::FrameType frameType(HbFrameDrawer::Undefined);
        if (!type.isEmpty()) {
            result = convertFrameType(type, frameType);
        }
        if (result) {
            result = mActions->setBackground(name, frameType);
        }
        if (!result) {
            qWarning() << "Invalid background, line " << mReader.lineNumber();
        }
    }

    return result;
}

HbXml::ElementType
    HbDocumentLoaderSyntax::elementType( QStringRef name ) const
{
    const QString stringName = name.toString();

    if( stringName == lexemValue(TYPE_DOCUMENT) ){
        return HbXml::DOCUMENT;
    }
    return HbXmlLoaderBaseSyntax::elementType( name );
}

bool HbDocumentLoaderSyntax::createVariable( HbXmlVariable& variable )
{
    const QString type = mReader.name().toString();
    bool ok = true;

    if( type == lexemValue( TYPE_INT ) ) {
        const QString value = attribute( ATTR_VALUE );
        qint16 *int_res = new qint16();
        *int_res = value.toInt( &ok );
        if ( ok ) {
            variable.mType = HbXmlVariable::INT;
            variable.mParameters.append(int_res);
        } else {
            delete int_res;
        }
    } else if( type == lexemValue( TYPE_REAL ) ) {
        const QString value = attribute( ATTR_VALUE );
        HbXmlLengthValue *value_res = new HbXmlLengthValue();
        ok = toLengthValue( value, *value_res );
        if( ok ) {
            variable.mType = HbXmlVariable::REAL;
            variable.mParameters.append(value_res);
        } else {
            delete value_res;
        }
    } else if( type == lexemValue( TYPE_LOCALIZED_STRING ) ) {
        QString *value = new QString();
        QString *attr = new QString();
        *value = attribute( ATTR_VALUE );
        *attr = attribute( ATTR_COMMENT );
        variable.mType = HbXmlVariable::LOCALIZED_STRING;
        variable.mParameters.append(value);
        variable.mParameters.append(attr);
    } else if( type == lexemValue( TYPE_STRING ) ) {
        QString *value = new QString();
        QString *locId = new QString();
        *value = attribute( ATTR_VALUE );
        *locId = attribute( ATTR_LOCID );
        variable.mType = HbXmlVariable::STRING;
        variable.mParameters.append(value);
        variable.mParameters.append(locId);
    } else if( type == lexemValue( TYPE_ENUMS ) || type == lexemValue(TYPE_ALIGNMENT) || type == lexemValue(LL_ORIENTATION) ) {
        QString *value = new QString();
        *value = attribute( ATTR_VALUE );
        variable.mType = HbXmlVariable::ENUMS;
        variable.mParameters.append(value);
    } else if ( type == lexemValue( TYPE_BOOL ) ) {
        bool *boolVal = new bool();
        const QString value = attribute( ATTR_VALUE );
        ok = toBool( value, *boolVal );
        if (ok) {
            variable.mType = HbXmlVariable::BOOL;
            variable.mParameters.append(boolVal);
        } else {
            delete boolVal;
        }
    } else if ( type == lexemValue( TYPE_ICON ) ) {

        QString *iconName = new QString(attribute( ATTR_ICONNAME ));
        HbXmlLengthValue *desiredWidth = new HbXmlLengthValue();
        HbXmlLengthValue *desiredHeight = new HbXmlLengthValue();

        // Read optional width attribute
        const QString width = attribute( ATTR_WIDTH );
        if (!width.isEmpty()) {
            ok = toLengthValue( width, *desiredWidth );
        }

        // Read optional height attribute
        const QString height = attribute( ATTR_HEIGHT );
        if (ok && !height.isEmpty()) {
            ok = toLengthValue( height, *desiredHeight );
        }

        if (ok) {
            variable.mType = HbXmlVariable::ICON;
            variable.mParameters.append(iconName);
            variable.mParameters.append(desiredWidth);
            variable.mParameters.append(desiredHeight);
        } else {
            delete iconName;
            delete desiredWidth;
            delete desiredHeight;
        }
    } else if ( type == lexemValue(TYPE_SIZE) ) {

        const QString width = attribute( ATTR_WIDTH );
        const QString height = attribute( ATTR_HEIGHT );

        if (!width.isEmpty() && !height.isEmpty()) {
            HbXmlLengthValue *widthVal = new HbXmlLengthValue();
            HbXmlLengthValue *heightVal = new HbXmlLengthValue();
            ok = toLengthValue(width, *widthVal);
            if (ok) {
                ok = toLengthValue(height, *heightVal);
            }
            if (ok) {
                variable.mType = HbXmlVariable::SIZE;
                variable.mParameters.append(widthVal);
                variable.mParameters.append(heightVal);
            } else {
                delete widthVal;
                delete heightVal;
            }
        } else {
            ok = false;
        }

    } else if ( type == lexemValue(TYPE_RECT) ) {

        const QString posx = attribute( ATTR_X );
        const QString posy = attribute( ATTR_Y );
        const QString width = attribute( ATTR_WIDTH );
        const QString height = attribute( ATTR_HEIGHT );

        if (!width.isEmpty() && !height.isEmpty() && !posx.isEmpty() && !posy.isEmpty()) {
            HbXmlLengthValue *widthVal = new HbXmlLengthValue();
            HbXmlLengthValue *heightVal = new HbXmlLengthValue();
            HbXmlLengthValue *posxVal = new HbXmlLengthValue();
            HbXmlLengthValue *posyVal = new HbXmlLengthValue();
            ok = toLengthValue(width, *widthVal);
            if (ok) {
                ok = toLengthValue(height, *heightVal);
            }
            if (ok) {
                ok = toLengthValue(posx, *posxVal);
            }
            if (ok) {
                ok = toLengthValue(posy, *posyVal);
            }
            if (ok) {
                variable.mType = HbXmlVariable::RECT;
                variable.mParameters.append(widthVal);
                variable.mParameters.append(heightVal);
                variable.mParameters.append(posxVal);
                variable.mParameters.append(posyVal);
            } else {
                delete widthVal;
                delete heightVal;
                delete posxVal;
                delete posyVal;
            }
        } else {
            ok = false;
        }

    } else if ( type == lexemValue(TYPE_POINT) ) {

        const QString posx = attribute( ATTR_X );
        const QString posy = attribute( ATTR_Y );
        if (!posx.isEmpty() && !posy.isEmpty()) {
            HbXmlLengthValue *posxVal = new HbXmlLengthValue();
            HbXmlLengthValue *posyVal = new HbXmlLengthValue();
            ok = toLengthValue(posx, *posxVal);
            if (ok) {
                ok = toLengthValue(posy, *posyVal);
            }
            if (ok) {
                variable.mType = HbXmlVariable::POINT;
                variable.mParameters.append(posxVal);
                variable.mParameters.append(posyVal);
            } else {
                delete posxVal;
                delete posyVal;
            }
        } else {
            ok = false;
        }

    } else if ( type == lexemValue(TYPE_COLOR) ) {

        const QString curColor = attribute( ATTR_VALUE  );
        if (!curColor.isEmpty() ) {
            QColor *colorVal = new QColor(curColor);
            variable.mType = HbXmlVariable::COLOR;
            variable.mParameters.append(colorVal);
        } else {
            ok = false;
        }

    } else if ( type == lexemValue(TYPE_FONTSPEC) ) {
        QString roleString = attribute( ATTR_FONTSPECROLE );
        HbFontSpec::Role role(HbFontSpec::Undefined);
        if (!roleString.isEmpty()) {
            ok = toFontSpecRole(roleString, role); // sets role if ok
        }
        if (ok) {
            quint8 *role_b = new quint8();
            *role_b = (quint8)role;
            HbXmlLengthValue *height = new HbXmlLengthValue();
            QString textHeightString = attribute( ATTR_TEXTHEIGHT );
            if (textHeightString.isEmpty()) {
                // Deprecated.
                textHeightString = attribute( ATTR_TEXTPANEHEIGHT );
            }
            if (!textHeightString.isEmpty()) {
                ok = toLengthValue(textHeightString, *height);
            }
            if (ok) {
                variable.mType = HbXmlVariable::FONTSPEC;
                variable.mParameters.append(role_b);
                variable.mParameters.append(height);
            } else {
                delete role_b;
                delete height;
            }
        }
    }

    else {
        // unknown property.
        ok = false;
    }
    return ok;
}

bool HbDocumentLoaderSyntax::convertSizeHintType(
    const QString &type,
    Qt::SizeHint &resultHint,
    bool &resultFixed)
{
    bool ok = true;
    resultFixed = false;

    if (type == QLatin1String("MINIMUM")) {
        resultHint = Qt::MinimumSize;
    } else if (type == QLatin1String("MAXIMUM")) {
        resultHint = Qt::MaximumSize;
    } else if (type == QLatin1String("PREFERRED")) {
        resultHint = Qt::PreferredSize;
    } else if (type == QLatin1String("FIXED")) {
        resultHint = Qt::PreferredSize;
        resultFixed = true;
    } else {
        ok = false;
    }
    return ok;
}

bool HbDocumentLoaderSyntax::convertFrameType(
    const QString &type,
    HbFrameDrawer::FrameType &resultType)
{
    bool ok = true;
    if (type == QLatin1String("OnePiece")) {
        resultType = HbFrameDrawer::OnePiece;
    } else if (type == QLatin1String("ThreePiecesHorizontal")) {
        resultType = HbFrameDrawer::ThreePiecesHorizontal;
    } else if (type == QLatin1String("ThreePiecesVertical")) {
        resultType = HbFrameDrawer::ThreePiecesVertical;
    } else if (type == QLatin1String("NinePieces")) {
        resultType = HbFrameDrawer::NinePieces;
    } else {
        ok = false;
    }
    return ok;
}


QString HbDocumentLoaderSyntax::version()
{
    return ( QString::number( VERSION_MAJOR ) + QString( "." )
            + QString::number( VERSION_MINOR ) + QString( " (" )
            + QString::number( MIN_SUPPORTED_VERSION_MAJOR ) + QString( "." )
            + QString::number( MIN_SUPPORTED_VERSION_MINOR ) + QString( ")" ) );
}

