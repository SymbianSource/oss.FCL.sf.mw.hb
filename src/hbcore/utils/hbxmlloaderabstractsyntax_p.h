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

#ifndef HBXMLLOADERABSTRACTSYNTAX_P_H
#define HBXMLLOADERABSTRACTSYNTAX_P_H

#include "hbxmlloaderabstractactions_p.h"

#include <hbglobal.h>
#include <hbdeviceprofile.h>

#include <QHash>
#include <QList>
#include <QXmlStreamReader>
#include <QPointer>

class HbXmlLoaderAbstractActions;
class HbWidget;

class HB_CORE_EXPORT HbXmlLoaderAbstractSyntax
{

public:

    enum DocumentLexems {
        ATTR_NAME,
        ATTR_TYPE,
        ATTR_VALUE,
        ATTR_ICONNAME,
        ATTR_WIDTH,
        ATTR_HEIGHT,
        ATTR_SRC,
        ATTR_SIGNAL,
        ATTR_DST,
        ATTR_SLOT,
        ATTR_X,
        ATTR_Y,
        ATTR_PLUGIN,
        ATTR_ROLE,
        ATTR_OBJECT,
        ATTR_CONTEXT,
        ATTR_LEFT,
        ATTR_RIGHT,
        ATTR_TOP,
        ATTR_BOTTOM,
        ATTR_HORIZONTALPOLICY,
        ATTR_VERTICALPOLICY,
        ATTR_HORIZONTALSTRETCH,
        ATTR_VERTICALSTRETCH,
		ATTR_COMMENT,
        ATTR_WIDGET,
        ATTR_VERSION,
        ATTR_FONTSPECROLE,
        ATTR_TEXTHEIGHT,
        ATTR_STRETCHFACTOR,

        // Deprecated.
        ATTR_TEXTPANEHEIGHT,
        ATTR_LOCID,
        
        TYPE_DOCUMENT,
        TYPE_HBWIDGET,
        TYPE_OBJECT,
        TYPE_WIDGET,
        TYPE_SPACERITEM,
        TYPE_CONNECT,
        TYPE_LAYOUT,
        TYPE_SECTION,
        TYPE_REF,
        TYPE_CONTENTSMARGINS,
        TYPE_SIZEPOLICY,
        TYPE_SIZEHINT,
        TYPE_ZVALUE,
        TYPE_TOOLTIP,
        TYPE_METADATA,
        TYPE_CONTAINER,
        TYPE_INT,
        TYPE_REAL,
        TYPE_LOCALIZED_STRING,
        TYPE_STRING,
        TYPE_ENUMS,
        TYPE_BOOL,
        TYPE_ICON,
        TYPE_SIZE,
        TYPE_RECT,
        TYPE_POINT,
        TYPE_COLOR,
        TYPE_ALIGNMENT,
        TYPE_FONTSPEC,
        
        LAYOUT_ANCHOR,
        LAYOUT_MESH,
        LAYOUT_MESH_TARGET,
        LAYOUT_MESH_ALIEN,
        LAYOUT_GRID,
        LAYOUT_LINEAR,
        LAYOUT_STACK,
        LAYOUT_NULL,
        
        CONTAINER_STRINGLIST,
        CONTAINER_NULL,
        
        VALUE_BOOL_TRUE,
        VALUE_BOOL_FALSE,

        UNIT_UNIT,
        UNIT_PIXEL,
        UNIT_MILLIMETER,
        UNIT_VAR_START,
        UNIT_VAR_NEG_START,
        UNIT_VAR_END,
        UNIT_EXPR_START,
        UNIT_EXPR_NEG_START,
        UNIT_EXPR_END,

        AL_ANCHOR,
        AL_SRC_NAME,
        AL_SRC_EDGE,
        AL_DST_NAME,
        AL_DST_EDGE,
        AL_SPACING,
        AL_SPACER,
        
        ML_MESHITEM, 
        ML_SRC_NAME, 
        ML_SRC_EDGE,
        ML_DST_NAME,
        ML_DST_EDGE,
        ML_SPACING,
        ML_SPACER,
        
        GL_GRIDCELL, 
        GL_ITEMNAME, 
        GL_ROW,
        GL_COLUMN,
        GL_ROWSPAN,
        GL_COLUMNSPAN,
        GL_SPACING,
        GL_GRIDROW,
        GL_GRIDCOLUMN,
        GL_MINWIDTH,
        GL_MAXWIDTH,
        GL_PREFWIDTH,
        GL_FIXEDWIDTH,
        GL_MINHEIGHT,
        GL_MAXHEIGHT,
        GL_PREFHEIGHT,
        GL_FIXEDHEIGHT,
        
        LL_ORIENTATION,
        LL_LINEARITEM, 
        LL_STRETCH, 
        LL_ITEMNAME, 
        LL_INDEX,
        LL_SPACING,
        
        SL_STACKITEM, 
        SL_ITEMNAME, 
        SL_INDEX,

        NUMBER_OF_LEXEMS // Keep this last!
    };
    
    enum TopState {
        TS_READ_DOCUMENT,
        TS_READ_METADATA,
        TS_ERROR,
        TS_EXIT
    };
    
    enum DocumentState {
        DS_START_DOCUMENT,
        DS_READ_SECTIONS,
        DS_END_DOCUMENT
    };
    
    enum ElementState {
        ES_GENERAL_ITEM,
        ES_LAYOUT_ITEM,
        ES_CONTAINER_ITEM
    };
    
    
 
public:

    HbXmlLoaderAbstractSyntax( HbXmlLoaderAbstractActions *actions );
    virtual ~HbXmlLoaderAbstractSyntax();
    
    virtual bool load( QIODevice *device, const QString &section );
        
public:
    

    virtual bool processDocument();
    virtual bool processLayout();
    virtual bool processContainer();
    virtual bool checkEndElementCorrectness();
    
        
    virtual ElementType elementType( QStringRef name ) const;    
    
    
    virtual QString attribute( DocumentLexems lexem ) const;
    
    virtual bool toReal(const QString &value, qreal& result) const;
    virtual bool toPixels(const QString &value, qreal& result) const;
        
    virtual bool readDocument();
    virtual bool readAlienSection();
    virtual bool readTargetSection();
    
    virtual bool readGeneralStartItem();
    virtual bool readGeneralEndItem();
    virtual bool readLayoutStartItem();
    virtual bool readLayoutEndItem();
    virtual bool readContainerStartItem();
    virtual bool readContainerEndItem();

public :
    bool toPixels(const HbDeviceProfile &deviceProfile, const QString &value, qreal& result) const;
    const char *lexemValue(DocumentLexems lex) const;
                  
protected :
    bool loadDevice(QIODevice *device, const QString &section);
    
public:
    
    TopState mTopState;
    DocumentState mDocumentState;
    ElementState mElementState;
    
    QStringList mCurrentSection;
    QStringList mRequiredSection;
    QStringList mCurrentContainer;
    
    QXmlStreamReader::TokenType mCurrentTokenType;
    ElementType mCurrentElementType;
    
    DocumentLexems mCurrentLayoutType;
    DocumentLexems mCurrentContainerType;

    HbXmlLoaderAbstractActions *mActions;

    QXmlStreamReader mReader;
    HbDeviceProfile mCurrentProfile;

private:

    Q_DISABLE_COPY(HbXmlLoaderAbstractSyntax)
};

#endif // HBXMLLOADERABSTRACTSYNTAX_P_H
