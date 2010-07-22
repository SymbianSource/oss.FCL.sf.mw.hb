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

#include <QColor>
#include <QDataStream>
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
    "action",           // ATTR_ACTION

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
    "mesh",             // ...deprecated... LAYOUT_MESH
    "grid",             // LAYOUT_GRID
    "linear",           // LAYOUT_LINEAR
    "stacked",          // LAYOUT_STACK
    "null",             // LAYOUT_NULL
    "",                 // LAYOUT_ALIEN

    "stringlist",       // CONTAINER_STRINGLIST
    "null",             // CONTAINER_NULL

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
    "srcId",            // AL_SRC_ID
    "srcEdge",          // AL_SRC_EDGE
    "dst",              // AL_DST_NAME
    "dstId",            // AL_DST_ID
    "dstEdge",          // AL_DST_EDGE
    "minLength",        // AL_MIN_LENGTH
    "prefLength",       // AL_PREF_LENGTH
    "maxLength",        // AL_MAX_LENGTH
    "sizepolicy",       // AL_SIZEPOLICY
    "direction",        // AL_DIRECTION
    "anchorId",         // AL_ANCHOR_ID
    "spacing",          // AL_SPACING
    "spacer",           // AL_SPACER

    "anchormapping",    // AL_MAPPING
    "item",             // AL_MAPPING_ITEM
    "id",               // AL_MAPPING_ID

    "meshitem",         // ...deprecated... ML_MESHITEM

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

QDataStream &operator<<(QDataStream &stream, const HbXmlLengthValue &value)
{
    quint8 type = value.mType;
    stream << type;
    switch ( value.mType ) {
        case HbXmlLengthValue::None:
            break;
        case HbXmlLengthValue::PlainNumber:
        case HbXmlLengthValue::Pixel:
        case HbXmlLengthValue::Unit:
        case HbXmlLengthValue::Millimeter:
            {
            double f_value = (double)value.mValue; //krazy:exclude=typedefs
            stream << f_value;
            break;
            }
        case HbXmlLengthValue::Variable:
        case HbXmlLengthValue::Expression:
            stream << value.mString;
            break;
        default:
            break;
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, HbXmlLengthValue &value)
{
    quint8 type;
    stream >> type;
    value.mType = (HbXmlLengthValue::Type)type;
    switch ( value.mType ) {
        case HbXmlLengthValue::None:
            break;
        case HbXmlLengthValue::PlainNumber:
        case HbXmlLengthValue::Pixel:
        case HbXmlLengthValue::Unit:
        case HbXmlLengthValue::Millimeter:
            {
            double f_value; //krazy:exclude=typedefs
            stream >> f_value;
            value.mValue = f_value;
            break;
            }
        case HbXmlLengthValue::Variable:
        case HbXmlLengthValue::Expression:
            stream >> value.mString;
            break;
        default:
            break;
    }
    return stream;
}


HbXmlVariable::HbXmlVariable() : mType(UNKNOWN)
{
}

HbXmlVariable::~HbXmlVariable()
{
    switch (mType) {
        case HbXmlVariable::INT:
        {
        if (mParameters.count()) delete (qint16*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::REAL:
        {
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::LOCALIZED_STRING:
        {
        if (mParameters.count()) delete (QString*)mParameters.takeAt(0);
        if (mParameters.count()) delete (QString*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::STRING:
        {
        if (mParameters.count()) delete (QString*)mParameters.takeAt(0);
        if (mParameters.count()) delete (QString*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::BOOL:
        {
        if (mParameters.count()) delete (bool*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::ICON:
        {
        if (mParameters.count()) delete (QString*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::SIZE:
        {
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::RECT:
        {
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::POINT:
        {
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::ENUMS:
        {
        if (mParameters.count()) delete (QString*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::COLOR:
        {
        if (mParameters.count()) delete (QColor*)mParameters.takeAt(0);
        break;
        }

        case HbXmlVariable::FONTSPEC:
        {
        if (mParameters.count()) delete (quint8*)mParameters.takeAt(0);
        if (mParameters.count()) delete (HbXmlLengthValue*)mParameters.takeAt(0);
        break;
        }

        default:
        {
        break;
        }
    }
    Q_ASSERT(!mParameters.count());
}

QDataStream &operator<<(QDataStream &stream, const HbXmlVariable &variable)
{
    stream << (quint8)variable.mType;
    switch (variable.mType) {
        case HbXmlVariable::INT:
        {
        stream << *((qint16*)variable.mParameters.at(0));
        break;
        }

        case HbXmlVariable::REAL:
        {
        stream << *((HbXmlLengthValue*)variable.mParameters.at(0));
        break;
        }

        case HbXmlVariable::LOCALIZED_STRING:
        {
        stream << *((QString*)variable.mParameters.at(0))
               << *((QString*)variable.mParameters.at(1));
        break;
        }

        case HbXmlVariable::STRING:
        {
        stream << *((QString*)variable.mParameters.at(0))
               << *((QString*)variable.mParameters.at(1));
        break;
        }

        case HbXmlVariable::BOOL:
        {
        stream << *((bool*)variable.mParameters.at(0));
        break;
        }

        case HbXmlVariable::ICON:
        {
        stream << *((QString*)variable.mParameters.at(0))
               << *((HbXmlLengthValue*)variable.mParameters.at(1))
               << *((HbXmlLengthValue*)variable.mParameters.at(2));
        break;
        }

        case HbXmlVariable::SIZE:
        {
        stream << *((HbXmlLengthValue*)variable.mParameters.at(0))
               << *((HbXmlLengthValue*)variable.mParameters.at(1));
        break;
        }

        case HbXmlVariable::RECT:
        {
        stream << *((HbXmlLengthValue*)variable.mParameters.at(0))
               << *((HbXmlLengthValue*)variable.mParameters.at(1))
               << *((HbXmlLengthValue*)variable.mParameters.at(2))
               << *((HbXmlLengthValue*)variable.mParameters.at(3));
        break;
        }

        case HbXmlVariable::POINT:
        {
        stream << *((HbXmlLengthValue*)variable.mParameters.at(0))
               << *((HbXmlLengthValue*)variable.mParameters.at(1));
        break;
        }

        case HbXmlVariable::ENUMS:
        {
        // Relies on implicit conversion.
        stream << *((QString*)variable.mParameters.at(0));
        break;
        }

        case HbXmlVariable::COLOR:
        {
        stream << *((QColor*)variable.mParameters.at(0));
        break;
        }

        case HbXmlVariable::FONTSPEC:
        {
        stream << *((quint8*)variable.mParameters.at(0))
               << *((HbXmlLengthValue*)variable.mParameters.at(1));
        break;
        }

        default:
        {
        break;
        }

    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, HbXmlVariable &variable)
{
    quint8 type_b;
    stream >> type_b;
    variable.mType = (HbXmlVariable::Type)type_b;
    switch (variable.mType) {
        case HbXmlVariable::INT:
        {
        qint16 *int_b = new qint16();
        stream >> *int_b;
        variable.mParameters.append(int_b);
        break;
        }

        case HbXmlVariable::REAL:
        {
        HbXmlLengthValue *realVal = new HbXmlLengthValue();
        stream >> *realVal;
        variable.mParameters.append(realVal);
        break;
        }

        case HbXmlVariable::LOCALIZED_STRING:
        {
        QString *value = new QString();
        QString *comment = new QString();
        stream >> *value >> *comment;
        variable.mParameters.append(value);
        variable.mParameters.append(comment);
        break;
        }

        case HbXmlVariable::STRING:
        {
        QString *value = new QString();
        QString *locId = new QString();
        stream >> *value >> *locId;
        variable.mParameters.append(value);
        variable.mParameters.append(locId);
        break;
        }

        case HbXmlVariable::BOOL:
        {
        bool *bool_b = new bool();
        stream >> *bool_b;
        variable.mParameters.append(bool_b);
        break;
        }

        case HbXmlVariable::ICON:
        {
        QString *iconName = new QString();
        HbXmlLengthValue *widthVal = new HbXmlLengthValue();
        HbXmlLengthValue *heightVal = new HbXmlLengthValue();
        stream >> *iconName >> *widthVal >> *heightVal;
        variable.mParameters.append(iconName);
        variable.mParameters.append(widthVal);
        variable.mParameters.append(heightVal);
        break;
        }

        case HbXmlVariable::SIZE:
        {
        HbXmlLengthValue *widthVal = new HbXmlLengthValue();
        HbXmlLengthValue *heightVal = new HbXmlLengthValue();
        stream >> *widthVal >> *heightVal;
        variable.mParameters.append(widthVal);
        variable.mParameters.append(heightVal);
        break;
        }

        case HbXmlVariable::RECT:
        {
        HbXmlLengthValue *widthVal = new HbXmlLengthValue();
        HbXmlLengthValue *heightVal = new HbXmlLengthValue();
        HbXmlLengthValue *posxVal = new HbXmlLengthValue();
        HbXmlLengthValue *posyVal = new HbXmlLengthValue();
        stream >> *widthVal >> *heightVal >> *posxVal >> *posyVal;
        variable.mParameters.append(widthVal);
        variable.mParameters.append(heightVal);
        variable.mParameters.append(posxVal);
        variable.mParameters.append(posyVal);
        break;
        }

        case HbXmlVariable::POINT:
        {
        HbXmlLengthValue *posxVal = new HbXmlLengthValue();
        HbXmlLengthValue *posyVal = new HbXmlLengthValue();
        stream >> *posxVal >> *posyVal;
        variable.mParameters.append(posxVal);
        variable.mParameters.append(posyVal);
        break;
        }

        case HbXmlVariable::ENUMS:
        {
        // Relies on implicit conversion.
        QString *string = new QString();
        stream >> *string;
        variable.mParameters.append(string);
        break;
        }

        case HbXmlVariable::COLOR:
        {
        QColor *color = new QColor();
        stream >> *color;
        variable.mParameters.append(color);
        break;
        }

        case HbXmlVariable::FONTSPEC:
        {
        quint8 *role_b = new quint8();
        HbXmlLengthValue *textHeightVal = new HbXmlLengthValue();
        stream >> *role_b >> *textHeightVal;
        variable.mParameters.append(role_b);
        variable.mParameters.append(textHeightVal);
        break;
        }

        default:
        {
        break;
        }

    }
    return stream;

}


HbXmlLoaderAbstractSyntax::HbXmlLoaderAbstractSyntax( HbXmlLoaderAbstractActions *actions )
: mActions(actions)
{
}

HbXmlLoaderAbstractSyntax::~HbXmlLoaderAbstractSyntax()
{
}

bool HbXmlLoaderAbstractSyntax::load( QIODevice *device, const QString &section )
{
    Q_UNUSED(device);
    Q_UNUSED(section);
    return false;
}

void HbXmlLoaderAbstractSyntax::setActions( HbXmlLoaderAbstractActions *actions )
{
    mActions = actions;
}

bool HbXmlLoaderAbstractSyntax::toReal(const QString &value, qreal& result) const
{
    bool ok = false;
    result = qreal(value.toDouble(&ok));
    if ( !ok ) {
        qWarning() << "Could not convert value to real";
        result = (qreal)0.0;
    }
    return ok;
}

bool HbXmlLoaderAbstractSyntax::toLengthValue(const QString &value, HbXmlLengthValue& lengthVal) const
{
    bool retVal(true);
    QString val = value;
    val.reserve(val.length());
    HbXmlLengthValue::Type type = HbXmlLengthValue::PlainNumber;
    if ( val.endsWith(lexemValue(UNIT_UNIT), Qt::CaseInsensitive) ) {
        type = HbXmlLengthValue::Unit;
    } else if ( val.endsWith(lexemValue(UNIT_PIXEL), Qt::CaseInsensitive) ) {
        type = HbXmlLengthValue::Pixel;
    } else if ( val.endsWith(lexemValue(UNIT_MILLIMETER), Qt::CaseInsensitive) ) {
        type = HbXmlLengthValue::Millimeter;
    } else if ( (val.startsWith( lexemValue(UNIT_VAR_START) ) ||
                val.startsWith( lexemValue(UNIT_VAR_NEG_START) ) ) && val.endsWith( lexemValue(UNIT_VAR_END) ) ) {
        type = HbXmlLengthValue::Variable;
    } else if ( (val.startsWith( lexemValue(UNIT_EXPR_START) ) ||
                val.startsWith( lexemValue(UNIT_EXPR_NEG_START) )) && val.endsWith( lexemValue(UNIT_EXPR_END) ) ) {
        type = HbXmlLengthValue::Expression;
    }
    lengthVal.mType = type;
    if ( type == HbXmlLengthValue::Variable || type == HbXmlLengthValue::Expression ) {
        lengthVal.mString = value;
        lengthVal.mValue = 0;
    } else {
        lengthVal.mString = QString();
        if (type != HbXmlLengthValue::PlainNumber) {
            // Assuming all unit identifiers have two characters
            val.chop(2);
        }
        retVal = toReal( val, lengthVal.mValue );
    }
    return retVal;
}

const char *HbXmlLoaderAbstractSyntax::lexemValue(DocumentLexems lex) const
{
    return knownLexems[lex];
}

