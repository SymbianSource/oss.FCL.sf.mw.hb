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

#include <hbicon.h>
#include <hbfontspec.h>
#include <QDebug>
#include <QMetaEnum>

#include <QTranslator>
#include <hbmainwindow.h>


// Document loader version number
#define VERSION_MAJOR 1
#define VERSION_MINOR 0

#define MIN_SUPPORTED_VERSION_MAJOR 0
#define MIN_SUPPORTED_VERSION_MINOR 1

/*
    \class HbDocumentLoaderSyntax
    \internal
    \proto
*/

HbDocumentLoaderSyntax::HbDocumentLoaderSyntax( HbDocumentLoaderActions *actions, const HbMainWindow *window )
: HbXmlLoaderAbstractSyntax( actions ), mRealActions( actions ), mMainWindow(window)
{
}

HbDocumentLoaderSyntax::~HbDocumentLoaderSyntax()
{
}

bool HbDocumentLoaderSyntax::load( QIODevice *device, const QString &section )
{
    mCurrentProfile = HbDeviceProfile::profile(mMainWindow);
    return HbXmlLoaderAbstractSyntax::loadDevice( device, section );
}

bool HbDocumentLoaderSyntax::readLayoutStartItem()
{
    bool result = false;
    switch( mCurrentLayoutType ) {
        case LAYOUT_ANCHOR:
        {
            HB_DOCUMENTLOADER_PRINT( "GENERAL LAYOUT START ITEM: ANCHOR ITEM" );
            if( mReader.name() == lexemValue( AL_ANCHOR ) ) {

                const QString src = attribute( AL_SRC_NAME );
                const QString dst = attribute( AL_DST_NAME );
                const QString srcEdge = attribute( AL_SRC_EDGE );
                const QString dstEdge = attribute( AL_DST_EDGE );
                const QString spacing = attribute( AL_SPACING );
                const QString spacer = attribute( AL_SPACER );
                qreal spacingVal = 0;
                result = true;
                if( !spacing.isEmpty() ) {
                    result = toPixels( spacing, spacingVal );
                }
                if (result) {
                    result = mRealActions->addAnchorLayoutEdge( src, srcEdge, dst, dstEdge, spacingVal, spacer );
                }
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
                result = mRealActions->addGridLayoutCell( src, row, column, rowspan, columnspan, alignment );
            } else if( mReader.name() == lexemValue( GL_GRIDROW ) ) {
                const QString row = attribute( GL_ROW );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );
                const QString alignment = attribute( TYPE_ALIGNMENT );
                result = mRealActions->setGridLayoutRowProperties( row, stretchfactor, alignment );
                if (result) {
                    result = processRowHeights( row );
                }
            } else if( mReader.name() == lexemValue( GL_GRIDCOLUMN ) ) {
                const QString column = attribute( GL_COLUMN );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );
                const QString alignment = attribute( TYPE_ALIGNMENT );
                result = mRealActions->setGridLayoutColumnProperties( column, stretchfactor, alignment );
                if (result) {
                    result = processColumnWidths( column );
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
                const QString index = attribute( LL_INDEX );
                const QString itemname = attribute( LL_ITEMNAME );
                const QString spacing = attribute( LL_SPACING );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );
                const QString alignment = attribute( TYPE_ALIGNMENT );
                
                qreal spacingValue(0);
                qreal *spacingPtr(0);
                if( !spacing.isEmpty() ) {
                    result = toPixels( spacing, spacingValue );
                    spacingPtr = &spacingValue;
                }
                if (result) {
                    result = mRealActions->addLinearLayoutItem( itemname, index, stretchfactor, alignment, spacingPtr );
                }
            } else if( mReader.name() == lexemValue( LL_STRETCH ) ) {
                const QString index = attribute( LL_INDEX );
                const QString stretchfactor = attribute( ATTR_STRETCHFACTOR );

                result = mRealActions->addLinearLayoutStretch( index, stretchfactor );
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

                result =  mRealActions->addStackedLayoutItem( itemname, index );
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
    return result;
}

bool HbDocumentLoaderSyntax::processContentsMargins()
{
    const QString leftS = attribute( ATTR_LEFT );
    const QString topS = attribute( ATTR_TOP );
    const QString rightS = attribute( ATTR_RIGHT );
    const QString bottomS = attribute( ATTR_BOTTOM );

    bool result = true;
    qreal left = 0, top = 0, right = 0, bottom = 0;
    if ( !leftS.isEmpty() ) {
        result = toPixels(leftS, left);
    }
    if ( result && !topS.isEmpty() ) {
        result = toPixels(topS, top);
    }
    if ( result && !rightS.isEmpty() ) {
        result = toPixels(rightS, right);
    }
    if ( result && !bottomS.isEmpty() ) {
        result = toPixels(bottomS, bottom);
    }

    if ( result ) {
        result = mRealActions->setLayoutContentsMargins( left, top, right, bottom );
    }

    if (!result) {
        qWarning() << "Invalid contents margins, line " << mReader.lineNumber();
    }

    return result;
}

bool HbDocumentLoaderSyntax::processRowHeights( const QString &row )
{
    const QString minHeightS = attribute( GL_MINHEIGHT );
    const QString maxHeightS = attribute( GL_MAXHEIGHT );
    const QString prefHeightS = attribute( GL_PREFHEIGHT );
    const QString fixedHeightS = attribute( GL_FIXEDHEIGHT );
    const QString rowSpacingS = attribute( GL_SPACING );
    qreal minHeight = -1;
    qreal maxHeight = -1;
    qreal prefHeight = -1;
    qreal fixedHeight = -1;
    qreal rowSpacing = -1;

    bool result = true;
    int propertyAvailable = 0;

    if ( !minHeightS.isEmpty() ) {
        result = toPixels(minHeightS, minHeight);
        propertyAvailable |= HbDocumentLoaderActions::propertyMin;
    }

    if ( result && !maxHeightS.isEmpty() ) {
        result = toPixels(maxHeightS, maxHeight);
        propertyAvailable |= HbDocumentLoaderActions::propertyMax;
    }

    if ( result && !prefHeightS.isEmpty() ) {
        result = toPixels(prefHeightS, prefHeight);
        propertyAvailable |= HbDocumentLoaderActions::propertyPref;
    }

    if ( result && !fixedHeightS.isEmpty() ) {
        result = toPixels(fixedHeightS, fixedHeight);
        propertyAvailable |= HbDocumentLoaderActions::propertyFixed;
    }

    if ( result && !rowSpacingS.isEmpty() ) {
        result = toPixels(rowSpacingS, rowSpacing);
        propertyAvailable |= HbDocumentLoaderActions::propertySpacing;
    }

    if ( result && propertyAvailable ) {
        result = mRealActions->setGridLayoutRowHeights( row, minHeight, maxHeight, 
                                                        prefHeight, fixedHeight, 
                                                        rowSpacing, propertyAvailable);
    }

    return result;
}

bool HbDocumentLoaderSyntax::processColumnWidths( const QString &column )
{
    const QString minWidthS = attribute( GL_MINWIDTH );
    const QString maxWidthS = attribute( GL_MAXWIDTH );
    const QString prefWidthS = attribute( GL_PREFWIDTH );
    const QString fixedWidthS = attribute( GL_FIXEDWIDTH );
    const QString columnSpacingS = attribute( GL_SPACING );
    qreal minWidth = -1;
    qreal maxWidth = -1;
    qreal prefWidth = -1;
    qreal fixedWidth = -1;
    qreal columnSpacing = -1;

    bool result = true;
    int propertyAvailable = 0;

    if ( !minWidthS.isEmpty() ) {
        result = toPixels(minWidthS, minWidth);
        propertyAvailable |= HbDocumentLoaderActions::propertyMin;
    }

    if ( result && !maxWidthS.isEmpty() ) {
        result = toPixels(maxWidthS, maxWidth);
        propertyAvailable |= HbDocumentLoaderActions::propertyMax;
    }

    if ( result && !prefWidthS.isEmpty() ) {
        result = toPixels(prefWidthS, prefWidth);
        propertyAvailable |= HbDocumentLoaderActions::propertyPref;
    }

    if ( result && !fixedWidthS.isEmpty() ) {
        result = toPixels(fixedWidthS, fixedWidth);
        propertyAvailable |= HbDocumentLoaderActions::propertyFixed;
    }

    if ( result && !columnSpacingS.isEmpty() ) {
        result = toPixels(columnSpacingS, columnSpacing);
        propertyAvailable |= HbDocumentLoaderActions::propertySpacing;
    }

    if ( result && propertyAvailable ) {
        result = mRealActions->setGridLayoutColumnWidths( column, minWidth, maxWidth, 
                                                          prefWidth, fixedWidth, 
                                                          columnSpacing, propertyAvailable);
    }

    return result;
}

bool HbDocumentLoaderSyntax::readContainerStartItem()
{
    bool result = false;
    switch ( mCurrentElementType ) {
         case PROPERTY:
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
        case CONTAINER:
        {
            currentPropertyName = mCurrentContainer.back();
            mCurrentContainer.removeLast();

            if (mRealActions->mCurrentContainer) {
                // in order for the conversion to work, all of the contained types need to be suitable and equivalent, e.g. strings
                QVariant variantContainer = QVariant(*(mRealActions->mCurrentContainer));
                if (variantContainer.isValid()) {
                    switch(mCurrentContainerType) {
                        case CONTAINER_STRINGLIST:
                        {
                            QStringList list = variantContainer.toStringList();
                            variant = QVariant(list);
                            break;
                        }
                        default:
                        {
                            variant = variantContainer;
                            break;
                        }
                    }
                }
            }

            result = mRealActions->pushProperty(currentPropertyName, variant);

            HB_DOCUMENTLOADER_PRINT( "CONTAINER END ITEM : SWITCHING TO GENERAL ITEM PROCESSING MODE" );
            mElementState = ES_GENERAL_ITEM;

            result = true;
            break;
        }
        default:
        {
            result = HbXmlLoaderAbstractSyntax::readGeneralEndItem();
            break;
        }
    }
    return result;
}

bool HbDocumentLoaderSyntax::readGeneralStartItem()
{
    bool result = false;
    switch( mCurrentElementType ) {
         case OBJECT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: OBJECT" );
            result = processObject();
            break;
         }
         case WIDGET:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: WIDGET" );
            result = processWidget();
            break;
         }
         case SPACERITEM:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: SPACERITEM" );
            result = processSpacerItem();
            break;
         }
         case CONNECT:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: CONNECT" );
            result = processConnect();
            break;
         }
         case PROPERTY:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: PROPERTY" );
            result = processProperty();
            break;
         }
         case REF:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: REF" );
            result = processRef();
            break;
         }
         case VARIABLE:
         {
            HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: VARIABLE" );
            result = processVariable();
            break;
         }
         case DEPRECATED:
         {
             HB_DOCUMENTLOADER_PRINT( "GENERAL START ITEM: DEPRECATED" );
             result = true;
             break;
         }
         default:
         {
              result = HbXmlLoaderAbstractSyntax::readGeneralStartItem();
         }
    }
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
    return mRealActions->pushDocument( attribute( ATTR_CONTEXT ) );
}

bool HbDocumentLoaderSyntax::processObject()
{
    const QString type = attribute( ATTR_TYPE );
    const QString name = attribute( ATTR_NAME );

    if( !mRealActions->pushObject( type, name ) ) {
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
    if( !mRealActions->pushWidget( type, name, role, plugin) ) {
        qWarning() << "Error in widget processing, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processSpacerItem()
{
    const QString name = attribute( ATTR_NAME );
    const QString widget = attribute( ATTR_WIDGET );

    if( !mRealActions->pushSpacerItem( name, widget ) ) {
        qWarning() << "Error in object processing, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processLayout()
{
    bool result = false;
    const QString layout_type = attribute( ATTR_TYPE );
    const QString widget = attribute( ATTR_WIDGET );

    if( layout_type == lexemValue( LAYOUT_ANCHOR ) ) {

        mCurrentLayoutType = LAYOUT_ANCHOR;
        result = mRealActions->createAnchorLayout( widget );

    } else if( layout_type == lexemValue( LAYOUT_GRID ) ) {

        result = true;
        mCurrentLayoutType = LAYOUT_GRID;
        const QString spacing = attribute( GL_SPACING );
        qreal spacingValue(0);
        qreal *spacingPtr(0);
        if( !spacing.isEmpty() ) {
            result = toPixels( spacing, spacingValue );
            spacingPtr = &spacingValue;
        }
        if (result) {
            result = mRealActions->createGridLayout( widget, spacingPtr );
        }

    } else if( layout_type == lexemValue( LAYOUT_LINEAR ) ) {

        result = true;
        mCurrentLayoutType = LAYOUT_LINEAR;
        const QString orientation = attribute( LL_ORIENTATION );
        const QString spacing = attribute( LL_SPACING );
        qreal spacingValue(0);
        qreal *spacingPtr(0);
        if( !spacing.isEmpty() ) {
            result = toPixels( spacing, spacingValue );
            spacingPtr = &spacingValue;
        }
        if (result) {
            result = mRealActions->createLinearLayout( widget, orientation, spacingPtr );
        }

    } else if( layout_type == lexemValue( LAYOUT_STACK ) ) {

        mCurrentLayoutType = LAYOUT_STACK;
        result = mRealActions->createStackedLayout( widget );

    } else if( layout_type == lexemValue( LAYOUT_NULL ) ) {

        mCurrentLayoutType = LAYOUT_NULL;
        result = mRealActions->createNullLayout( widget );

    } else {
        return HbXmlLoaderAbstractSyntax::processLayout();
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

    if( !mRealActions->pushConnect( srcName, signalName, dstName, slotName ) ) {
        qWarning() << "Error in connect processing, line " << mReader.lineNumber();
        return false;

    }
    return true;
}

bool HbDocumentLoaderSyntax::processContainer()
{
    bool result = false;
    const QString container_type = attribute( ATTR_TYPE );

    if( container_type == lexemValue( CONTAINER_STRINGLIST ) ) {

        mCurrentContainerType = CONTAINER_STRINGLIST;

        const QString propertyName = attribute ( ATTR_NAME );
        mCurrentContainer << propertyName;
        result = mRealActions->createContainer();

    } else {
        return HbXmlLoaderAbstractSyntax::processContainer();
    }

    if( !result ) {
        qWarning() << "Unable to create container, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processContainedProperty()
{
    const QVariant value = decodeValue();
    if( ! value.isValid() ) {
        qWarning() << "Invalid property, line " << mReader.lineNumber();
        return false;
    }

    if( !mRealActions->appendPropertyToContainer( value ) ) {
        qWarning() << "Unable to set property, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processProperty()
{
    const QVariant value = decodeValue();
    if( ! value.isValid() ) {
        qWarning() << "Invalid property, line " << mReader.lineNumber();
        return false;
    }

    const QString propertyName = attribute( ATTR_NAME );

    if( !mRealActions->pushProperty( propertyName, value ) ) {
        qWarning() << "Unable to set property, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

bool HbDocumentLoaderSyntax::processRef()
{
    const QString objectName = attribute( ATTR_OBJECT );
    const QString role = attribute( ATTR_ROLE );

    if( !mRealActions->pushRef( objectName, role ) ) {
        qWarning() << "Error in reference processing, line " << mReader.lineNumber();
        return false;
    }
    return true;
}

static bool convertSizePolicy_Policy( const QString& policyS, QSizePolicy::Policy *&policy )
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

    policy = (QSizePolicy::Policy *)new int(policyI);
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
        qreal left = 0, top = 0, right = 0, bottom = 0;
        if ( !leftS.isEmpty() ) {
            result = toPixels(leftS, left);
        }
        if ( result && !topS.isEmpty() ) {
            result = toPixels(topS, top);
        }
        if ( result && !rightS.isEmpty() ) {
            result = toPixels(rightS, right);
        }
        if ( result && !bottomS.isEmpty() ) {
            result = toPixels(bottomS, bottom);
        }

        if ( result ) {
            result = mRealActions->setContentsMargins( left, top, right, bottom );
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

        QSizePolicy::Policy *hPol = 0;
        if ( !horizontalPolicyS.isEmpty() ) {
            result = convertSizePolicy_Policy( horizontalPolicyS, hPol );
        }

        QSizePolicy::Policy *vPol = 0;
        if ( result && !verticalPolicyS.isEmpty() ) {
            result = convertSizePolicy_Policy( verticalPolicyS, vPol );
        }

        int *hStretch = 0;
        if ( result && !horizontalStretchS.isEmpty() ) {
            const int intValue = horizontalStretchS.toInt( &result );
            if ( result ) {
                if ( intValue >= 0 && intValue < 256 ) {
                    hStretch = new int( intValue );
                } else {
                    result = false;
                }
            }
        }

        int *vStretch = 0;
        if ( result && !verticalStretchS.isEmpty() ) {
            const int intValue = verticalStretchS.toInt( &result );
            if ( result ) {
                if ( intValue >= 0 && intValue < 256 ) {
                    vStretch = new int( intValue );
                } else {
                    result = false;
                }
            }
        }

        if ( result ) {
            result = mRealActions->setSizePolicy( hPol, vPol, hStretch, vStretch );
        }
        delete hPol;
        delete vPol;
        delete hStretch;
        delete vStretch;

        if (!result) {
            qWarning() << "Invalid size policy, line " << mReader.lineNumber();
        }

    } else if ( type == lexemValue( TYPE_SIZEHINT ) ) {

        Qt::SizeHint hint = Qt::PreferredSize;
        bool fixed = false;

        if (convertSizeHintType(attribute( ATTR_TYPE ), hint, fixed)) {

            result = true;

            qreal *sizeHintWidth = 0;
            const QString width = attribute( ATTR_WIDTH );
            if (!width.isEmpty()) {
                qreal widthInPixels;
                result = toPixels(width, widthInPixels);
                if (result) {
                    sizeHintWidth = new qreal;
                    *sizeHintWidth = widthInPixels;
                }
            }

            qreal *sizeHintHeight = 0;
            const QString height = attribute( ATTR_HEIGHT );
            if (result && !height.isEmpty()) {
                qreal heightInPixels;
                result = toPixels(height, heightInPixels);
                if (result) {
                    sizeHintHeight = new qreal;
                    *sizeHintHeight = heightInPixels;
                }
            }

            if (result) {
                result = mRealActions->setSizeHint(hint, sizeHintWidth, sizeHintHeight, fixed);
            }
        }

        if (!result) {
            qWarning() << "Invalid size hint, line " << mReader.lineNumber();
        }
    } else if ( type == lexemValue( TYPE_ZVALUE ) ) {
        const QString zValueAsString = attribute( ATTR_VALUE );
        if (!zValueAsString.isEmpty()) {
            qreal zValueAsReal;
            result = toReal(zValueAsString, zValueAsReal);
            if ( result ) {
                result = mRealActions->setZValue( zValueAsReal );
            }
        }

        if (!result) {
            qWarning() << "Invalid z value, line " << mReader.lineNumber();
        }
    } else if ( type == lexemValue( TYPE_TOOLTIP ) ) {
        const QString value = attribute( ATTR_VALUE );
        const QString comment = attribute( ATTR_COMMENT );
        const QString locId = attribute( ATTR_LOCID );

        if (!locId.isEmpty()) {
            QByteArray locIdUtf8(locId.toUtf8());
            const QString translated = hbTrId(locIdUtf8);
            result = mRealActions->setToolTip( translated );
        } else {
            const QString translated = mRealActions->translate( value, comment );
            result = mRealActions->setToolTip( translated );
        }

        if (!result) {
            qWarning() << "Invalid tooltip, line " << mReader.lineNumber();
        }
    }

    return result;
}

ElementType
    HbDocumentLoaderSyntax::elementType( QStringRef name ) const
{
    const QString stringName = name.toString();

    if( stringName == lexemValue(TYPE_DOCUMENT) ){
        return DOCUMENT;
    }
    return HbXmlLoaderAbstractSyntax::elementType( name );
}

QVariant HbDocumentLoaderSyntax::decodeValue()
{
    QVariant result = QVariant::Invalid;

    const QString type = mReader.name().toString();

    bool ok = false;
    if( type == lexemValue( TYPE_INT ) ) {
        const QString value = attribute( ATTR_VALUE );
        int int_res = value.toInt( &ok );
        if( ok ) {
            result = int_res;
        }
    } else if( type == lexemValue( TYPE_REAL ) ) {
        const QString value = attribute( ATTR_VALUE );
        qreal qreal_res;
        ok = toPixels( value, qreal_res );
        if( ok ) {
            result = qreal_res;
        }
    } else if( type == lexemValue( TYPE_LOCALIZED_STRING ) ) {
        const QString value =
            mRealActions->translate( attribute( ATTR_VALUE ), attribute( ATTR_COMMENT ) );
        result = value;
    } else if( type == lexemValue( TYPE_STRING ) ) {
        const QString value = attribute( ATTR_VALUE );
        const QString locId = attribute( ATTR_LOCID );
        if (!locId.isEmpty()) {
            QByteArray locIdUtf8(locId.toUtf8());
            result = hbTrId(locIdUtf8);
        } else {
            result = value;
        }
    } else if( type == lexemValue( TYPE_ENUMS ) ) {
        result = attribute( ATTR_VALUE );
    } else if ( type == lexemValue( TYPE_BOOL ) ) {
        const QString value = attribute( ATTR_VALUE );
        if (value == lexemValue( VALUE_BOOL_TRUE ) ) {
            result = QVariant(true);
        } else if (value == lexemValue( VALUE_BOOL_FALSE ) ) {
            result = QVariant(false);
        }
    } else if ( type == lexemValue( TYPE_ICON ) ) {

        HbIcon icon;
        ok = true;

        // Read optional iconName attribute (if not given, it's null icon)
        const QString iconName = attribute( ATTR_ICONNAME );
        if ( !iconName.isEmpty() ) {
            icon.setIconName( iconName );
        }

        qreal desiredWidth = 0;
        qreal desiredHeight = 0;

        // Read optional width attribute
        const QString width = attribute( ATTR_WIDTH );
        if (!width.isEmpty()) {
            ok = toPixels( width, desiredWidth );
        }

        // Read optional height attribute
        const QString height = attribute( ATTR_HEIGHT );
        if (ok && !height.isEmpty()) {
            ok = toPixels( height, desiredHeight );
        }

        if (ok) {
            if (!width.isEmpty() && !height.isEmpty()) {
                icon.setSize(QSizeF(desiredWidth, desiredHeight));
            } else if (!width.isEmpty()) {
                icon.setWidth(desiredWidth);
            } else if (!height.isEmpty()) {
                icon.setHeight(desiredHeight);
            } else {
                // neither defined.
            }

            result = icon;
        }
    } else if ( type == lexemValue(TYPE_SIZE) ) {

        const QString width = attribute( ATTR_WIDTH );
        const QString height = attribute( ATTR_HEIGHT );

        if (!width.isEmpty() && !height.isEmpty()) {
            ok = true;
            QSizeF size;
            qreal widthVal, heightVal;
            ok = toPixels(width, widthVal);
            if (ok) {
                size.setWidth(widthVal);
                ok = toPixels(height, heightVal);
            }
            if (ok) {
                size.setHeight(heightVal);
                result = size;
            }
        }

    } else if ( type == lexemValue(TYPE_RECT) ) {

        const QString posx = attribute( ATTR_X );
        const QString posy = attribute( ATTR_Y );
        const QString width = attribute( ATTR_WIDTH );
        const QString height = attribute( ATTR_HEIGHT );

        if (!width.isEmpty() && !height.isEmpty() && !posx.isEmpty() && !posy.isEmpty()) {
            ok = true;
            QSizeF size;
            QPointF point;
            qreal widthVal, heightVal, posxVal, posyVal;
            ok = toPixels(width, widthVal);
            if (ok) {
                size.setWidth(widthVal);
                ok = toPixels(height, heightVal);
            }
            if (ok) {
                size.setHeight(heightVal);
                ok = toPixels(posx, posxVal);
            }
            if (ok) {
                point.setX(posxVal);
                ok = toPixels(posy, posyVal);
            }
            if (ok) {
                point.setY(posyVal);
                result = QRectF(point, size);
            }
        }

    } else if ( type == lexemValue(TYPE_POINT) ) {

        const QString posx = attribute( ATTR_X );
        const QString posy = attribute( ATTR_Y );
        if (!posx.isEmpty() && !posy.isEmpty()) {
            ok = true;
            QPointF point;
            qreal posxVal, posyVal;
            ok = toPixels(posx, posxVal);
            if (ok) {
                point.setX(posxVal);
                ok = toPixels(posy, posyVal);
            }
            if (ok) {
                point.setY(posyVal);
                result = point;
            }
        }

    } else if ( type == lexemValue(TYPE_ALIGNMENT) ) {

        const QString alignment = attribute( ATTR_VALUE );
        if (!alignment.isEmpty() ) {
            result = alignment;
        }

    } else if ( type == lexemValue(LL_ORIENTATION) ) {

        const QString orientation = attribute( ATTR_VALUE );
        if (!orientation.isEmpty() ) {
            result = orientation;
        }

    } else if ( type == lexemValue(TYPE_COLOR) ) {

        const QString curColor = attribute( ATTR_VALUE  );
        if (!curColor.isEmpty() ) {
            ok = true;
            result = QColor(curColor);
        }

    } else if ( type == lexemValue(TYPE_FONTSPEC) ) {
        QString roleString = attribute( ATTR_FONTSPECROLE );
        HbFontSpec::Role role(HbFontSpec::Undefined);
        ok = true;
        if (!roleString.isEmpty()) {
            ok = toFontSpecRole(roleString, role); // sets role if ok
        }
        if (ok) {
            HbFontSpec spec(role);
            QString textHeightString = attribute( ATTR_TEXTHEIGHT );
            if (textHeightString.isEmpty()) {
                // Deprecated.
                textHeightString = attribute( ATTR_TEXTPANEHEIGHT );
            }
            if (!textHeightString.isEmpty()) {
                qreal height(0);
                ok = toPixels(textHeightString, height);
                if (ok) {
                    spec.setTextHeight(qRound(height));
                }
            }
            if (ok) {
                result = spec;
            }
        }
    }

    else {
        // unknown property.
    }

    return result;
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

QString HbDocumentLoaderSyntax::version()
{
    return ( QString::number( VERSION_MAJOR ) + QString( "." )
            + QString::number( VERSION_MINOR ) + QString( " (" )
            + QString::number( MIN_SUPPORTED_VERSION_MAJOR ) + QString( "." )
            + QString::number( MIN_SUPPORTED_VERSION_MINOR ) + QString( ")" ) );
}

bool HbDocumentLoaderSyntax::toFontSpecRole(const QString &roleString, HbFontSpec::Role &role)
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
