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

#ifndef HBCSSPARSER_P_H
#define HBCSSPARSER_P_H

#include <QStringList>
#include <QVector>
#include <QVariant>
#include <QPair>
#include <QSize>
#include <QFont>
#include <QPalette>
#include <QIcon>
#include <QSizePolicy>
#include <QHash>

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbdeviceprofile.h>
#include <hbfontspec.h>

// smart containers and memory manager inclusion
#include "hbmemorymanager_p.h"
#include "hbvector_p.h"
#include "hbstring_p.h"
#include "hbvariant_p.h"
#include "hbstringvector_p.h"

class HbFontSpec;

//QT_BEGIN_NAMESPACE

namespace HbCss
{

enum Property {
    UnknownProperty,
    BackgroundColor,
    Color,
    Float,
    Font,
    FontFamily,
    FontSize,
    FontStyle,
    FontWeight,
    Margin,
    MarginBottom,
    MarginLeft,
    MarginRight,
    MarginTop,
    QtBlockIndent,
    QtListIndent,
    QtParagraphType,
    QtTableType,
    QtUserState,
    TextDecoration,
    TextIndent,
    TextUnderlineStyle,
    VerticalAlignment,
    Whitespace,
    QtSelectionForeground,
    QtSelectionBackground,
    Border,
    BorderLeft,
    BorderRight,
    BorderTop,
    BorderBottom,
    Padding,
    PaddingLeft,
    PaddingRight,
    PaddingTop,
    PaddingBottom,
    PageBreakBefore,
    PageBreakAfter,
    QtAlternateBackground,
    BorderLeftStyle,
    BorderRightStyle,
    BorderTopStyle,
    BorderBottomStyle,
    BorderStyles,
    BorderLeftColor,
    BorderRightColor,
    BorderTopColor,
    BorderBottomColor,
    BorderColor,
    BorderLeftWidth,
    BorderRightWidth,
    BorderTopWidth,
    BorderBottomWidth,
    BorderWidth,
    BorderTopLeftRadius,
    BorderTopRightRadius,
    BorderBottomLeftRadius,
    BorderBottomRightRadius,
    BorderRadius,
    Background,
    BackgroundOrigin,
    BackgroundClip,
    BackgroundRepeat,
    BackgroundPosition,
    BackgroundAttachment,
    BackgroundImage,
    BorderImage,
    QtSpacing,
    Width,
    Height,
    MinimumWidth,
    MinimumHeight,
    MaximumWidth,
    MaximumHeight,
    QtImage,
    Left,
    Right,
    Top,
    Bottom,
    QtOrigin,
    QtPosition,
    Position,
    QtStyleFeatures,
    QtBackgroundRole,
    ListStyleType,
    ListStyle,
    QtImageAlignment,
    TextAlignment,
    Outline,
    OutlineOffset,
    OutlineWidth,
    OutlineColor,
    OutlineStyle,
    OutlineRadius,
    OutlineTopLeftRadius,
    OutlineTopRightRadius,
    OutlineBottomLeftRadius,
    OutlineBottomRightRadius,
    FontVariant,
    TextTransform,
    HbSpacingHorizontal,
    HbSpacingVertical,
    HbColumnNarrowWidth,
    HbColumnWideWidth,
    HbIndent,
    HbSmallIconSize,
    HbLargeIconSize,
    HbTopMarginWeight,
    HbIconLeftAlignmentWeight,
    HbStretchable,
    HbLayout,
    HbAspectRatio,
    HbPreferredWidth,
    HbPreferredHeight,
    HbPreferredSize,
    HbFixedWidth,
    HbFixedHeight,
    HbFixedSize,
    HbMinimumSize,
    HbMaximumSize,
    HbSizePolicy,
    HbSizePolicyHorizontal,
    HbSizePolicyVertical,
    HbCenterHorizontal,
    HbCenterVertical,
    HbSection,
    HbTextLineCountMin,
    HbTextLineCountMax,
    HbTextHeight,
    HbTextWrapMode,
    Mirroring, // deprecated
    HbLayoutDirection,
    ZValue,
    NumProperties
};

enum KnownValue {
    UnknownValue,
    Value_Normal,
    Value_Pre,
    Value_Small,
    Value_Medium,
    Value_Large,
    Value_XLarge,
    Value_XXLarge,
    Value_Italic,
    Value_Oblique,
    Value_Bold,
    Value_Underline,
    Value_Overline,
    Value_LineThrough,
    Value_Sub,
    Value_Super,
    Value_Left,
    Value_Right,
    Value_Top,
    Value_Bottom,
    Value_Center,
    Value_Native,
    Value_Solid,
    Value_Dotted,
    Value_Dashed,
    Value_DotDash,
    Value_DotDotDash,
    Value_Double,
    Value_Groove,
    Value_Ridge,
    Value_Inset,
    Value_Outset,
    Value_Wave,
    Value_Middle,
    Value_Auto,
    Value_Always,
    Value_None,
    Value_Transparent,
    Value_Disc,
    Value_Circle,
    Value_Square,
    Value_Decimal,
    Value_LowerAlpha,
    Value_UpperAlpha,
    Value_SmallCaps,
    Value_Uppercase,
    Value_Lowercase,

    /* keep these in same order as QPalette::ColorRole */
    Value_FirstColorRole,
    Value_WindowText = Value_FirstColorRole,
    Value_Button,
    Value_Light,
    Value_Midlight,
    Value_Dark,
    Value_Mid,
    Value_Text,
    Value_BrightText,
    Value_ButtonText,
    Value_Base,
    Value_Window,
    Value_Shadow,
    Value_Highlight,
    Value_HighlightedText,
    Value_Link,
    Value_LinkVisited,
    Value_AlternateBase,
    Value_LastColorRole = Value_AlternateBase,

    Value_Disabled,
    Value_Active,
    Value_Selected,
    Value_On,
    Value_Off,

    Value_Ignore,
    Value_Keep,
    Value_KeepExpand,

    Value_Primary,
    Value_Secondary,
    Value_Title,
    Value_PrimarySmall,
    Value_Digital,

    Value_Fixed,
    Value_Minimum,
    Value_Maximum,
    Value_Preferred,
    Value_Expanding,
    Value_MinimumExpanding,
    Value_Ignored,
    
    Value_Mirrored, // deprecated
    Value_LeftToRight,
    Value_RightToLeft,
    Value_Parent,

    Value_NoWrap,
    Value_WordWrap,
    Value_WrapAnywhere,

    NumKnownValues
};

enum BorderStyle {
    BorderStyle_Unknown,
    BorderStyle_None,
    BorderStyle_Dotted,
    BorderStyle_Dashed,
    BorderStyle_Solid,
    BorderStyle_Double,
    BorderStyle_DotDash,
    BorderStyle_DotDotDash,
    BorderStyle_Groove,
    BorderStyle_Ridge,
    BorderStyle_Inset,
    BorderStyle_Outset,
    BorderStyle_Native,
    NumKnownBorderStyles
};

enum Edge {
    TopEdge,
    RightEdge,
    BottomEdge,
    LeftEdge,
    NumEdges
};

enum Corner {
    TopLeftCorner,
    TopRightCorner,
    BottomLeftCorner,
    BottomRightCorner
};

enum TileMode {
    TileMode_Unknown,
    TileMode_Round,
    TileMode_Stretch,
    TileMode_Repeat,
    NumKnownTileModes
};

enum Repeat {
    Repeat_Unknown,
    Repeat_None,
    Repeat_X,
    Repeat_Y,
    Repeat_XY,
    NumKnownRepeats
};

enum Origin {
    Origin_Unknown,
    Origin_Padding,
    Origin_Border,
    Origin_Content,
    Origin_Margin,
    NumKnownOrigins
};

enum PositionMode {
    PositionMode_Unknown,
    PositionMode_Static,
    PositionMode_Relative,
    PositionMode_Absolute,
    PositionMode_Fixed,
    NumKnownPositionModes
};

enum LayoutDirection {
    LayoutDirection_LeftToRight,
    LayoutDirection_RightToLeft,
    LayoutDirection_Parent,
    NumKnownLayoutDirections
};

enum Attachment {
    Attachment_Unknown,
    Attachment_Fixed,
    Attachment_Scroll,
    NumKnownAttachments
};

enum StyleFeature {
    StyleFeature_None = 0,
    StyleFeature_BackgroundColor = 1,
    StyleFeature_BackgroundGradient = 2,
    NumKnownStyleFeatures = 4
};

struct HB_CORE_PRIVATE_EXPORT Value
{
    enum Type {
        Unknown,
        Number,
        Percentage,
        Length,
        String,
        Identifier,
        KnownIdentifier,
        Uri,
        Color,
        Function,
        TermOperatorSlash,
        TermOperatorComma,
        Variable,  //added for variable support
        VariableNegative,  //added for variable support
        Expression, //added for expression support
        ExpressionNegative //added for expression support
    };

    inline Value(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory) 
        : memoryType(memType),
          type(Unknown),
          original(memType),
          variant(memType)
    { }

    //for debug only
#ifdef CSS_PARSER_TRACES
    const QString what(Type t) const
    {
        QString returnString;
        switch(t) {
        case Unknown: 
            returnString = QString("Unknown");
            break;
        case Number:
            returnString = QString("Number");
            break;
        case Percentage:
            returnString = QString("Percentage");
            break;
        case Length:
            returnString = QString("Length");
            break;
        case String:
            returnString = QString("String");
            break;
        case Identifier:
            returnString = QString("Identifier");
            break;
        case KnownIdentifier:
            returnString = QString("KnownIdentifier");
            break;
        case Uri:
            returnString = QString("Uri");
            break;
        case Color:
            returnString = QString("Color");
            break;
        case Function:
            returnString = QString("Function");
            break;
        case TermOperatorSlash:
            returnString = QString("TermOperatorSlash");
            break;
        case TermOperatorComma:
            returnString = QString("TermOperatorComma");
            break;
        case Variable:
            returnString = QString("Variable");
            break;
        default:
            break;
        }
        return returnString;
    }

    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"\t \t \t"<<"==============Value::Print():Begin==================";
        qDebug() <<"\t \t \t"<< "Value::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() <<"\t \t \t"<< "Value::Type type = " << what(type);
        qDebug() <<"\t \t \t"<< "Value::HbString original = " << original;
        qDebug() <<"\t \t \t"<< "Value::HbVariant variant = " << variant.toString();
        qDebug() <<"\t \t \t"<<"==============Value::Print():End====================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    Type type;
    HbString original;
    HbVariant variant;
};

// 1. StyleRule - x:hover, y:clicked > z:checked { prop1: value1; prop2: value2; }
// 2. QVector<Selector> - x:hover, y:clicked z:checked
// 3. QVector<BasicSelector> - y:clicked z:checked
// 4. QVector<Declaration> - { prop1: value1; prop2: value2; }
// 5. Declaration - prop1: value1;

struct HB_CORE_PRIVATE_EXPORT Declaration
{
    inline Declaration(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        :memoryType(type),
        property(type),
        propertyId(UnknownProperty),
        values(type),
        important(false)
    {}

    inline bool isEmpty() const { return property.isEmpty() && propertyId == UnknownProperty; }

    // helper functions
    QColor colorValue(const QPalette & = QPalette()) const;
    void colorValues(QColor *c, const QPalette & = QPalette()) const;
    QBrush brushValue(const QPalette & = QPalette()) const;
    void brushValues(QBrush *c, const QPalette & = QPalette()) const;

    BorderStyle styleValue() const;
    void styleValues(BorderStyle *s) const;

    Origin originValue() const;
    Repeat repeatValue() const;
    Qt::Alignment alignmentValue() const;
    Hb::TextWrapping wrapModeValue() const;
    PositionMode positionValue() const;
    Attachment attachmentValue() const;
    int styleFeaturesValue() const;

    bool intValue(int *i, const char *unit = 0) const;
    bool realValue(qreal *r, const char *unit = 0) const;

    QSize sizeValue() const;
    QRect rectValue() const;
    QString uriValue() const;
    QIcon iconValue() const;

    void borderImageValue(QString *image, int *cuts, TileMode *h, TileMode *v) const;

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"\t"<<"==============Declaration::Print():Begin==================";
        qDebug() <<"\t"<<"Declaration::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "\t"<< "Declaration::HbString property = " << property;
        qDebug() << "\t"<< "Declaration::Property propertyId = " << propertyId;
        qDebug() << "\t"<< "Declaration::HbVector<Value> values = " ;
        values.print();
        qDebug() <<"\t"<<"==============Declaration::Print():End====================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString property;
    Property propertyId;
    HbVector<Value> values;
    bool important;
};

typedef QPair<int, Declaration> WeightedDeclaration;

const quint64 PseudoClass_Unknown          = Q_UINT64_C(0x0000000000000000);
const quint64 PseudoClass_Enabled          = Q_UINT64_C(0x0000000000000001);
const quint64 PseudoClass_Disabled         = Q_UINT64_C(0x0000000000000002);
const quint64 PseudoClass_Pressed          = Q_UINT64_C(0x0000000000000004);
const quint64 PseudoClass_Focus            = Q_UINT64_C(0x0000000000000008);
const quint64 PseudoClass_Hover            = Q_UINT64_C(0x0000000000000010);
const quint64 PseudoClass_Checked          = Q_UINT64_C(0x0000000000000020);
const quint64 PseudoClass_Unchecked        = Q_UINT64_C(0x0000000000000040);
const quint64 PseudoClass_Indeterminate    = Q_UINT64_C(0x0000000000000080);
const quint64 PseudoClass_Unspecified      = Q_UINT64_C(0x0000000000000100);
const quint64 PseudoClass_Selected         = Q_UINT64_C(0x0000000000000200);
const quint64 PseudoClass_Horizontal       = Q_UINT64_C(0x0000000000000400);
const quint64 PseudoClass_Vertical         = Q_UINT64_C(0x0000000000000800);
const quint64 PseudoClass_Window           = Q_UINT64_C(0x0000000000001000);
const quint64 PseudoClass_Children         = Q_UINT64_C(0x0000000000002000);
const quint64 PseudoClass_Sibling          = Q_UINT64_C(0x0000000000004000);
const quint64 PseudoClass_Default          = Q_UINT64_C(0x0000000000008000);
const quint64 PseudoClass_First            = Q_UINT64_C(0x0000000000010000);
const quint64 PseudoClass_Last             = Q_UINT64_C(0x0000000000020000);
const quint64 PseudoClass_Middle           = Q_UINT64_C(0x0000000000040000);
const quint64 PseudoClass_OnlyOne          = Q_UINT64_C(0x0000000000080000);
const quint64 PseudoClass_PreviousSelected = Q_UINT64_C(0x0000000000100000);
const quint64 PseudoClass_NextSelected     = Q_UINT64_C(0x0000000000200000);
const quint64 PseudoClass_Flat             = Q_UINT64_C(0x0000000000400000);
const quint64 PseudoClass_Left             = Q_UINT64_C(0x0000000000800000);
const quint64 PseudoClass_Right            = Q_UINT64_C(0x0000000001000000);
const quint64 PseudoClass_Top              = Q_UINT64_C(0x0000000002000000);
const quint64 PseudoClass_Bottom           = Q_UINT64_C(0x0000000004000000);
const quint64 PseudoClass_Exclusive        = Q_UINT64_C(0x0000000008000000);
const quint64 PseudoClass_NonExclusive     = Q_UINT64_C(0x0000000010000000);
const quint64 PseudoClass_Frameless        = Q_UINT64_C(0x0000000020000000);
const quint64 PseudoClass_ReadOnly         = Q_UINT64_C(0x0000000040000000);
const quint64 PseudoClass_Active           = Q_UINT64_C(0x0000000080000000);
const quint64 PseudoClass_Closable         = Q_UINT64_C(0x0000000100000000);
const quint64 PseudoClass_Movable          = Q_UINT64_C(0x0000000200000000);
const quint64 PseudoClass_Floatable        = Q_UINT64_C(0x0000000400000000);
const quint64 PseudoClass_Minimized        = Q_UINT64_C(0x0000000800000000);
const quint64 PseudoClass_Maximized        = Q_UINT64_C(0x0000001000000000);
const quint64 PseudoClass_On               = Q_UINT64_C(0x0000002000000000);
const quint64 PseudoClass_Off              = Q_UINT64_C(0x0000004000000000);
const quint64 PseudoClass_Editable         = Q_UINT64_C(0x0000008000000000);
const quint64 PseudoClass_Item             = Q_UINT64_C(0x0000010000000000);
const quint64 PseudoClass_Closed           = Q_UINT64_C(0x0000020000000000);
const quint64 PseudoClass_Open             = Q_UINT64_C(0x0000040000000000);
const quint64 PseudoClass_EditFocus        = Q_UINT64_C(0x0000080000000000);
const quint64 PseudoClass_Alternate        = Q_UINT64_C(0x0000100000000000);
const quint64 PseudoClass_Landscape        = Q_UINT64_C(0x0000200000000000);
const quint64 PseudoClass_Portrait         = Q_UINT64_C(0x0000400000000000);
const quint64 PseudoClass_LeftToRight      = Q_UINT64_C(0x0000800000000000);
const quint64 PseudoClass_RightToLeft      = Q_UINT64_C(0x0001000000000000);
// The Any specifier is never generated, but can be used as a wildcard in searches.
const quint64 PseudoClass_Any              = Q_UINT64_C(0x0002000000000000);
const int NumPseudos = 50;

struct HB_CORE_PRIVATE_EXPORT Pseudo
{
    Pseudo(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory) 
        : memoryType(memType),
          negated(false),
          name(memType),
          function(memType)
    { }

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}
    void print() const
    {    qDebug() <<"==============Pseudo::Print():Begin==================";
        qDebug() << "Pseudo::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "Pseudo::HbString name= " << name;
        qDebug() << "Pseudo::HbString function = " << function;
        qDebug() <<"==============Pseudo::Print():End==================";
        
    }
#endif
    //Data
    HbMemoryManager::MemoryType memoryType;
    bool negated;
    quint64 type;
    HbString name;
    HbString function;
};

struct HB_CORE_PRIVATE_EXPORT AttributeSelector
{
    enum ValueMatchType {
        NoMatch,
        MatchEqual,
        MatchContains,
        MatchBeginsWith
    };

    inline AttributeSelector(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
          name(type),
          value(type),
          valueMatchCriterium(NoMatch),
          negated(false)         
    {}
    HbMemoryManager::MemoryType memoryType;
    HbString name;
    HbString value;
    ValueMatchType valueMatchCriterium;
    bool negated;

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"==============AttributeSelector::Print():Begin==================";
        qDebug() << "AttributeSelector::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "AttributeSelector::HbString name= " << name;
        qDebug() << "AttributeSelector::HbString value = " << value;
        qDebug() <<"==============AttributeSelector::Print():End==================";
    }
#endif
};

struct HB_CORE_PRIVATE_EXPORT BasicSelector
{
    inline BasicSelector(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory) 
        : memoryType(type),
          elementName(type),
          ids(type),
          pseudos(type),
          attributeSelectors(type),
          relationToNext(NoRelation)
    {}

    inline BasicSelector(const BasicSelector &other) 
        : memoryType(other.memoryType),
          elementName(other.elementName),
          ids(other.ids),
          pseudos(other.pseudos),
          attributeSelectors(other.attributeSelectors),
          relationToNext(other.relationToNext)
    {
        GET_MEMORY_MANAGER(other.memoryType)
        if (!manager->isWritable()) {
            memoryType = HbMemoryManager::HeapMemory;
        }
    }

    enum Relation {
        NoRelation,
        MatchNextSelectorIfAncestor,
        MatchNextSelectorIfParent,
        MatchNextSelectorIfPreceeds
    };

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}
    void print() const
    {
        qDebug() <<"\t \t"<<"==============BasicSelector::Print():Begin==================";
        qDebug() <<"\t \t"<<"BasicSelector::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() <<"\t \t"<<"BasicSelector::HbString elementName= " << elementName;
//        qDebug() <<"\t \t"<<"BasicSelector::QStringList ids = " << ids;
        qDebug() <<"\t \t"<<"BasicSelector::PseudoVector pseudos = ";
        pseudos.print();
        qDebug() <<"\t \t"<< "BasicSelector::AttributeSelectorVector attributeSelectors = ";
        attributeSelectors.print();
        qDebug() <<"\t \t"<<"==============BasicSelector::Print():End====================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString elementName;

    HbStringVector ids;
    HbVector<Pseudo> pseudos;
    HbVector<AttributeSelector> attributeSelectors;
    Relation relationToNext;
};

struct HB_CORE_PRIVATE_EXPORT Selector
{
    Selector() 
        : memoryType(HbMemoryManager::HeapMemory),
          basicSelectors(HbMemoryManager::HeapMemory)
    {}

    Selector(HbMemoryManager::MemoryType type) 
        : memoryType(type),
          basicSelectors(type)
    {}

    int specificity() const;
    quint64 pseudoClass(quint64 *negated = 0) const;
    QString pseudoElement() const;

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"\t "<<"==============Selector::Print():Begin==================";
        qDebug() <<"\t "<<"Selector::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() <<"\t "<<"Selector::BasicSelectorVector basicSelectors= ";
        basicSelectors.print();    
        qDebug() <<"\t "<<"==============Selector::Print():End==================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbVector<BasicSelector> basicSelectors;
};

enum PositionValueFlag {
    ExtractedLeft = 0x0001,
    ExtractedRight = 0x0002,
    ExtractedTop = 0x0004,
    ExtractedBottom = 0x0008,
    ExtractedOrigin = 0x0010,
    ExtractedAlign = 0x0020,
    ExtractedMode = 0x0040,
    ExtractedTextAlign = 0x0080,
    ExtractedCenterH = 0x0100,
    ExtractedCenterV = 0x0200,
    ExtractedLayoutDirection = 0x0400,
    ExtractedZValue = 0x0800,
    ExtractedWrapMode = 0x1000
};
Q_DECLARE_FLAGS(PositionValueFlags, PositionValueFlag)

enum GeometryValueFlag {
    ExtractedPrefW = 0x0001,
    ExtractedPrefH = 0x0002,
    ExtractedMinW = 0x0004,
    ExtractedMinH = 0x0008,
    ExtractedMaxW = 0x0010,
    ExtractedMaxH = 0x0020,
    ExtractedPolHor = 0x0040,
    ExtractedPolVer = 0x0080
};
Q_DECLARE_FLAGS(GeometryValueFlags, GeometryValueFlag)

enum TextValueFlag {
    ExtractedLineCountMin = 0x0001,
    ExtractedLineCountMax = 0x0002
};
Q_DECLARE_FLAGS(TextValueFlags, TextValueFlag)

struct GeometryValues
{
    qreal mPrefW, mPrefH, mMinW, mMinH, mMaxW, mMaxH;
    QSizePolicy mSizePolicy;
    GeometryValueFlags mFlags;
};

struct PositionValues
{
    qreal mLeft, mTop, mRight, mBottom, mCenterH, mCenterV, mZ;
    Qt::Alignment mPosition;
    HbCss::Origin mOrigin;
    HbCss::PositionMode mPositionMode;
    Qt::Alignment mTextAlignment;
    HbCss::LayoutDirection mLayoutDirection;    
    Hb::TextWrapping mTextWrapMode;
    PositionValueFlags mFlags;
};

struct TextValues
{
    int mLineCountMin;
    int mLineCountMax;
    TextValueFlags mFlags;
};


struct StyleRule;
struct MediaRule;
struct PageRule;
struct ImportRule;
struct VariableRule;  //new added for variable support

struct HB_CORE_PRIVATE_EXPORT ValueExtractor
{
    ValueExtractor(const HbVector<Declaration> &declarations, const HbDeviceProfile &profile, const QPalette & = QPalette());
    ValueExtractor(const HbVector<Declaration> &declarations, const QHash<QString, HbCss::Declaration> &varDeclarations,
                   const HbDeviceProfile &profile, const QPalette & = QPalette());
    ValueExtractor(const HbVector<Declaration> &varDeclarations, bool isVariable, const HbDeviceProfile &profile = HbDeviceProfile());
    ValueExtractor(const QHash<QString, HbCss::Declaration> &varDecls, bool isVariable, const HbDeviceProfile &profile = HbDeviceProfile());

    bool extractFont(QFont *font, HbFontSpec *fontSpec, int *fontSizeAdjustment);
    bool extractValue(const QString& variableName, HbVector<HbCss::Value>& values) const;
    bool extractValue(const QString& variableName, qreal& value);
    bool extractValue( const QString &variableName, HbCss::Value &value ) const;
    bool extractExpressionValue(QString &expression, qreal &value);
    bool extractBackground(QBrush *, QString *, Repeat *, Qt::Alignment *, HbCss::Origin *, HbCss::Attachment *,
                           HbCss::Origin *);
    bool extractGeometry(GeometryValues &geomValues);
    bool extractPosition(PositionValues &posValues);
    bool extractBox(qreal *margins, qreal *paddings, qreal *spacing = 0);
    bool extractBorder(qreal *borders, QBrush *colors, BorderStyle *Styles, QSize *radii);
    bool extractOutline(qreal *borders, QBrush *colors, BorderStyle *Styles, QSize *radii, qreal *offsets);
    bool extractPalette(QBrush *fg, QBrush *sfg, QBrush *sbg, QBrush *abg);
    int  extractStyleFeatures();
    bool extractImage(QIcon *icon, Qt::Alignment *a, QSize *size);
    bool extractLayout(QString *layoutName, QString *sectionName);
    bool extractAspectRatioMode(Qt::AspectRatioMode *mode);
    bool extractParameters( const QList<QString> &params, QList<QVariant> &values );
    bool extractColor( QColor *col ) const;
    bool extractTextValues( TextValues &textValues );

    bool asBool(const Declaration &decl);
    qreal asReal(const Declaration &decl);

    int lengthValue(const Declaration &decl);

private:
    void extractFont();
    void borderValue(const Declaration &decl, qreal *width, HbCss::BorderStyle *style, QBrush *color);
    int lengthValue(const Value& v);
    qreal asReal(const Value& v);
    qreal asReal(QString &s, Value::Type type);
    bool asReal(QString &s, qreal &value);
    void asReals(const Declaration &decl, qreal *m);
    QSizePolicy asSizePolicy(const Declaration &decl);
    QSizePolicy::Policy asPolicy(const Value& v);
    void lengthValues(const Declaration &decl, int *m);
    QSize sizeValue(const Declaration &decl);
    void sizeValues(const Declaration &decl, QSize *radii);

    struct ExpressionValue
    {
        enum Token {
            None = 0,
            Minus = 1,
            Plus = 2,
            Star = 3,
            Slash = 4,
            UnaryMinus = 5
        };

        ExpressionValue() : mToken(None), mPrecedence(0), mValue(0) {}
        ExpressionValue(Token token, int precedence, qreal value) : mToken(token), mPrecedence(precedence), mValue(value) {}
        Token mToken;
        int mPrecedence;
        qreal mValue;
    };

    HbVector<Declaration> declarations;
    HbVector<Declaration> variableDeclarations; //for variables
    QHash<QString, HbCss::Declaration> variableDeclarationsHash;
    QFont f;
    HbFontSpec fSpec;
    int adjustment;
    int fontExtracted;
    QPalette pal;
    HbDeviceProfile currentProfile;
    QList<ExpressionValue> expressionValues; // for parsed expression string
};

struct StyleSheet;

struct HB_CORE_PRIVATE_EXPORT StyleRule
{
    StyleRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
          selectors(type),
          declarations(type)
#ifdef HB_CSS_INSPECTOR
          , owningStyleSheet(0, type)
#endif
    {}

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"==============StyleRule::Print():Begin==================";
        qDebug() << "StyleRule::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "StyleRule::SelectorVector selectors = ";
        selectors.print();
        qDebug() << "StyleRule::DeclarationVector declarations = ";
        declarations.print();
        qDebug() <<"==============StyleRule::Print():End==================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbVector<Selector> selectors;
    HbVector<Declaration> declarations;
#ifdef HB_CSS_INSPECTOR
    smart_ptr<StyleSheet> owningStyleSheet;
#endif
};

typedef QPair<int, StyleRule> WeightedRule;

struct HB_CORE_PRIVATE_EXPORT VariableRule
{
    VariableRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType (type),
          declarations(type)
    {}

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}
    void print() const
    {
        qDebug() <<"==============VariableRule::Print():Begin==================";
        qDebug() << "VariableRule::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "VariableRule::DeclarationVector declarations = ";
        declarations.print();
        qDebug() <<"==============VariableRule::Print():End==================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbVector<Declaration> declarations;
};

struct HB_CORE_PRIVATE_EXPORT MediaRule
{
    MediaRule(HbMemoryManager::MemoryType type=HbMemoryManager::HeapMemory)
        : memoryType(type),
          media(type),
          styleRules (type)
    {}

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}
    void print() const
    {
        qDebug() <<"==============MediaRule::Print():Begin==================";
        qDebug() << "MediaRule::HbMemoryManager::MemoryType memoryType = " << memoryType;
//        qDebug() << "MediaRule::QStringList media = " << media;
        qDebug() << "MediaRule::StyleRuleVector styleRules = ";
        styleRules.print();
        qDebug() <<"==============MediaRule::Print():End==================";
            
    }
#endif
    // data
    HbMemoryManager::MemoryType memoryType;
    //ToDo: Replace it with HbStringList if we have it in future
    HbStringVector media;
    HbVector<StyleRule> styleRules;
};

struct HB_CORE_PRIVATE_EXPORT PageRule
{
    PageRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
          selector(type),
          declarations (type)
    {}

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"==============PageRule::Print():Begin==================";
        qDebug() << "PageRule::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "PageRule::HbString selector = " << selector;
        qDebug() << "PageRule::DeclarationVector declarations = ";
        declarations.print();
        qDebug() <<"==============PageRule::Print():End==================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString selector;
    HbVector<Declaration> declarations;
};

struct HB_CORE_PRIVATE_EXPORT ImportRule
{
    ImportRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
          href(type),
          media(type)
    {}

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"==============ImportRule::Print():Begin==================";
        qDebug() << "ImportRule::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "ImportRule::HbString href = " << href;
//        qDebug() << "ImportRule::QStringList media = " << media;
        qDebug() <<"==============ImportRule::Print():End==================";
    }
#endif
    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString href;
    // ToDo: Replace it with HbStringList if we have it in future
    HbStringVector media;
};

enum StyleSheetOrigin {
    StyleSheetOrigin_Unspecified,
    StyleSheetOrigin_UserAgent,
    StyleSheetOrigin_User,
    StyleSheetOrigin_Author,
    StyleSheetOrigin_Inline
};

struct WidgetStyleRules 
{
WidgetStyleRules(uint widgetNameHash, HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : classNameHash(widgetNameHash), styleRules(type), portraitRules(type), landscapeRules(type)
    { 
    }
#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"==============WidgetStyleRules::Print():Begin==================";
        qDebug() << "Generic rules:";
        styleRules.print();
        qDebug() << "Portrait rules:";
        portraitRules.print();
        qDebug() << "Landscape rules:";
        landscapeRules.print();
        qDebug() <<"==============WidgetStyleRules::Print():End==================";
    }
#endif
    // Data
	uint classNameHash;
    HbVector<StyleRule> styleRules;
    HbVector<StyleRule> portraitRules;
    HbVector<StyleRule> landscapeRules;
};

struct HB_AUTOTEST_EXPORT StyleSheet
{
StyleSheet(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
        variableRules(type),
        widgetRules(type),
        mediaRules(type),
        pageRules(type),
        importRules(type),
        origin(StyleSheetOrigin_Unspecified),
        depth(0)
#ifdef HB_CSS_INSPECTOR
        , fileName(type)
#endif
    { }

StyleSheet(const StyleSheet &other, HbMemoryManager::MemoryType type) 
        : memoryType(type),
        variableRules(type),
        widgetRules(type),
        mediaRules(type),
        pageRules(type),
        importRules(type),
        origin(other.origin),
        depth(other.depth)
#ifdef HB_CSS_INSPECTOR
        , fileName(type)
#endif
    {
        variableRules = other.variableRules;
        widgetRules = other.widgetRules;
        mediaRules = other.mediaRules;
        pageRules = other.pageRules;
        importRules = other.importRules;
#ifdef HB_CSS_INSPECTOR
        fileName = other.fileName;
#endif
    }

#ifdef CSS_PARSER_TRACES
    bool supportsPrinting() const {return true;}

    void print() const
    {
        qDebug() <<"==============StyleSheet::Print():Begin==================";
        qDebug() << "StyleSheet::HbMemoryManager::MemoryType memoryType = " << memoryType;
        qDebug() << "StyleSheet::VariableRuleVector variableRules = ";
        variableRules.print();
        qDebug() << "StyleSheet::WidgetStyleRuleVector widgetRules = ";
        widgetRules.print();
        qDebug() << "StyleSheet::MediaRuleVector mediaRules = ";
        mediaRules.print();
        qDebug() << "StyleSheet::PageRulesVector pageRules = ";
        pageRules.print();
        qDebug() << "StyleSheet::ImportRuleVector importRules = ";
        importRules.print();
        qDebug() <<"==============StyleSheet::Print():End==================";
    }
#endif

    // Utility functions
    WidgetStyleRules* widgetStack(uint classNameHash) const
    {
        for (int i=0; i<widgetRules.count(); i++) {
            if (classNameHash == widgetRules.at(i).classNameHash) {
                return &(widgetRules.at(i));
            }
        }
        return 0;
    }
    WidgetStyleRules* addWidgetStack(const WidgetStyleRules &rules)
    {
        widgetRules.append(rules);
        int lastIndex = widgetRules.count() - 1;
        return &(widgetRules.at(lastIndex));
    }

    // Data
    
    HbMemoryManager::MemoryType memoryType;
    HbVector<VariableRule> variableRules;
    HbVector<WidgetStyleRules> widgetRules;
    HbVector<MediaRule> mediaRules;
    HbVector<PageRule> pageRules;
    HbVector<ImportRule> importRules;

    StyleSheetOrigin origin;
    int depth; // applicable only for inline style sheets
#ifdef HB_CSS_INSPECTOR
    HbString fileName;
#endif
};

class HB_AUTOTEST_EXPORT StyleSelector
{
public:
    StyleSelector();
    StyleSelector(const StyleSelector &copy);
    virtual ~StyleSelector();

    union NodePtr {
        void *ptr;
        int id;
    };

    bool hasOrientationSpecificStyleRules(NodePtr node) const;
    QVector<WeightedRule> weightedStyleRulesForNode(NodePtr node, const Qt::Orientation orientation) const;
    QVector<WeightedDeclaration> weightedDeclarationsForNode(NodePtr node, const Qt::Orientation orientation, const char *extraPseudo = 0) const;
    HbVector<StyleRule> styleRulesForNode(NodePtr node, const Qt::Orientation orientation) const;
    HbVector<Declaration> declarationsForNode(NodePtr node, const Qt::Orientation orientation, const char *extraPseudo = 0) const;
    void variableRuleSets(QHash<QString, HbCss::Declaration> *variables) const;

    virtual int nodeNameEquals(NodePtr node, const HbString& nodeName) const = 0;
    virtual bool attributeMatches(NodePtr node, const AttributeSelector &attr) const = 0;
    virtual bool hasAttributes(NodePtr node) const = 0;
    virtual QStringList nodeIds(NodePtr node) const = 0;
    virtual bool isNullNode(NodePtr node) const = 0;
    virtual NodePtr parentNode(NodePtr node) const = 0;
    virtual NodePtr previousSiblingNode(NodePtr node) const = 0;
    virtual void initNode(NodePtr node) const = 0;
    virtual void cleanupNode(NodePtr node) const = 0;

    void addStyleSheet( StyleSheet* styleSheet );
    void removeStyleSheet( StyleSheet* styleSheet );

    QVector<StyleSheet*> styleSheets;
    QHash<uint, QVector<StyleSheet*> > widgetSheets;
    QString medium;
private:
    void matchRules(NodePtr node, const HbVector<StyleRule> &rules, StyleSheetOrigin origin,
                    int depth, QVector<WeightedRule> *weightedRules, bool nameCheckNeeded=true) const;
    int selectorMatches(const Selector &rule, NodePtr node, bool nameCheckNeeded) const;
    int basicSelectorMatches(const BasicSelector &rule, NodePtr node, bool nameCheckNeeded) const;
    int inheritanceDepth(NodePtr node, HbString &elementName) const;
};

enum TokenType {
    NONE,

    S,

    CDO,
    CDC,
    INCLUDES,
    DASHMATCH,

    LBRACE,
    PLUS,
    GREATER,
    COMMA,

    STRING,
    INVALID,

    IDENT,

    HASH,

    ATKEYWORD_SYM,

    EXCLAMATION_SYM,

    LENGTH,

    PERCENTAGE,
    NUMBER,

    FUNCTION,

    COLON,
    SEMICOLON,
    RBRACE,
    SLASH,
    MINUS,
    DOT,
    STAR,
    LBRACKET,
    RBRACKET,
    EQUAL,
    LPAREN,
    RPAREN,
    OR
};

struct HB_CORE_PRIVATE_EXPORT Symbol
{
    inline Symbol() : start(0), len(-1) {}
    TokenType token;
    QString text;
    int start, len;
    QString lexem() const;
};

class /*Q_AUTOTEST_EXPORT*/ Scanner
{
public:
    static QString preprocess(const QString &input, bool *hasEscapeSequences = 0);
    static void scan(const QString &preprocessedInput, QVector<Symbol> *symbols);
    static const char *tokenName(TokenType t);
};

class HB_CORE_PRIVATE_EXPORT Parser
{
public:
	enum Error{
		NoError,
		OutOfMemoryError,
		UnknownError
	};
    Parser();
    explicit Parser(const QString &css, bool file = false);

    void init(const QString &css, bool file = false);
    bool parse(StyleSheet *styleSheet);
    Symbol errorSymbol();

    bool parseImport(ImportRule *importRule);
    bool parseMedia(MediaRule *mediaRule);
    bool parseMedium(HbStringVector *media);
    bool parsePage(PageRule *pageRule);
    bool parsePseudoPage(HbString *selector);
    bool parseNextOperator(Value *value);
    bool parseCombinator(BasicSelector::Relation *relation);
    bool parseProperty(Declaration *decl);
    bool parseVariableset(VariableRule *variableRule);
    bool parseRuleset(StyleRule *styleRule);
    bool parseSelector(Selector *sel);
    bool parseSimpleSelector(BasicSelector *basicSel);
    bool parseClass(HbString *name);
    bool parseElementName(HbString *name);
    bool parseAttrib(AttributeSelector *attr);
    bool parsePseudo(Pseudo *pseudo);
    bool parseNextDeclaration(Declaration *declaration);
    bool parsePrio(Declaration *declaration);
    bool parseExpr(HbVector<Value> *values);
    bool parseTerm(Value *value);
    bool parseFunction(HbString *name, HbString *args);
    bool parseHexColor(QColor *col);
    bool testAndParseUri(HbString *uri);
	
	void addRuleToWidgetStack(StyleSheet *sheet, const QString &stackName, StyleRule &rule);

    inline bool testRuleset() { return testSelector(); }
    inline bool testSelector() { return testSimpleSelector(); }
    inline bool parseNextSelector(Selector *sel) { if (!testSelector()) return recordError(); return parseSelector(sel); }
    bool testSimpleSelector();
    inline bool parseNextSimpleSelector(BasicSelector *basicSel) { if (!testSimpleSelector()) return recordError(); return parseSimpleSelector(basicSel); }
    inline bool testElementName() { return test(IDENT) || test(STAR); }
    inline bool testClass() { return test(DOT); }
    inline bool testAttrib() { return test(LBRACKET); }
    inline bool testPseudo() { return test(COLON); }
    inline bool testMedium() { return test(IDENT); }
    inline bool parseNextMedium(HbStringVector *media) { if (!testMedium()) return recordError(); return parseMedium(media); }
    inline bool testPseudoPage() { return test(COLON); }
    inline bool testImport() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("import")); }
    inline bool testMedia() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("media")); }
    inline bool testVariable() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("variables")); }  //new addition for variable support
    inline bool testPage() { return testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("page")); }
    inline bool testCombinator() { return test(PLUS) || test(GREATER) || test(S); }
    inline bool testProperty() { return test(IDENT); }
    bool testTerm();
    inline bool testExpr() { return testTerm(); }
    inline bool parseNextExpr(HbVector<Value> *values) { if (!testExpr()) return recordError(); return parseExpr(values); }
    bool testPrio();
    inline bool testHexColor() { return test(HASH); }
    inline bool testFunction() { return test(FUNCTION); }
    inline bool parseNextFunction(HbString *name, HbString *args) { if (!testFunction()) return recordError(); return parseFunction(name, args); }

    inline bool lookupElementName() const { return lookup() == IDENT || lookup() == STAR; }

    inline void skipSpace() { while (test(S)) {}; }

    inline bool hasNext() const { return index < symbols.count(); }
    inline TokenType next() { return symbols.at(index++).token; }
    bool next(TokenType t);
    bool test(TokenType t);
    inline void prev() { index--; }
    inline const Symbol &symbol() const { return symbols.at(index - 1); }
    inline QString lexem() const { return symbol().lexem(); }
    QString unquotedLexem() const;
    QString lexemUntil(TokenType t);
    bool until(TokenType target, TokenType target2 = NONE);
    inline TokenType lookup() const {
        return (index - 1) < symbols.count() ? symbols.at(index - 1).token : NONE;
    }

    bool testTokenAndEndsWith(TokenType t, const QLatin1String &str);

    inline bool recordError() { errorIndex = index; return false; }

    QVector<Symbol> symbols;
    int index;
    int errorIndex;
	Error errorCode;
    bool hasEscapeSequences;
    QString sourcePath;
    QString sourceFile;
};

} // namespace HbCss

//QT_END_NAMESPACE

#endif
