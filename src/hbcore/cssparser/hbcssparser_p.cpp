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

#include "hbcssparser_p.h"

#include <new>
#include <qdebug.h>
#include <qcolor.h>
#include <qfont.h>
#include <qfileinfo.h>
#include <qfontmetrics.h>
#include <qbrush.h>
#include <qimagereader.h>
#include <qgraphicswidget.h>
#include <hbfontspec.h>

//QT_BEGIN_NAMESPACE

//#define CSSSTACKS_DEBUG 

#include "hbcssscanner_p.cpp"
#include "hbmemoryutils_p.h"
#include "hblayeredstyleloader_p.h"

using namespace HbCss;

const QString GLOBAL_CSS_SELECTOR = "*";

const char *Scanner::tokenName(HbCss::TokenType t)
{
    switch (t) {
        case NONE: return "NONE";
        case S: return "S";
        case CDO: return "CDO";
        case CDC: return "CDC";
        case INCLUDES: return "INCLUDES";
        case DASHMATCH: return "DASHMATCH";
        case LBRACE: return "LBRACE";
        case PLUS: return "PLUS";
        case GREATER: return "GREATER";
        case COMMA: return "COMMA";
        case STRING: return "STRING";
        case INVALID: return "INVALID";
        case IDENT: return "IDENT";
        case HASH: return "HASH";
        case ATKEYWORD_SYM: return "ATKEYWORD_SYM";
        case EXCLAMATION_SYM: return "EXCLAMATION_SYM";
        case LENGTH: return "LENGTH";
        case PERCENTAGE: return "PERCENTAGE";
        case NUMBER: return "NUMBER";
        case FUNCTION: return "FUNCTION";
        case COLON: return "COLON";
        case SEMICOLON: return "SEMICOLON";
        case RBRACE: return "RBRACE";
        case SLASH: return "SLASH";
        case MINUS: return "MINUS";
        case DOT: return "DOT";
        case STAR: return "STAR";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case EQUAL: return "EQUAL";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case OR: return "OR";
    }
    return "";
}

struct HbCssKnownValue
{
    const char *name;
    quint64 id;
};

static const HbCssKnownValue properties[NumProperties - 1] = {
    { "-qt-background-role", QtBackgroundRole },
    { "-qt-block-indent", QtBlockIndent },
    { "-qt-list-indent", QtListIndent },
    { "-qt-paragraph-type", QtParagraphType },
    { "-qt-style-features", QtStyleFeatures },
    { "-qt-table-type", QtTableType },
    { "-qt-user-state", QtUserState },
    { "alternate-background-color", QtAlternateBackground },
    { "aspect-ratio", HbAspectRatio },
    { "background", Background },
    { "background-attachment", BackgroundAttachment },
    { "background-clip", BackgroundClip },
    { "background-color", BackgroundColor },
    { "background-image", BackgroundImage },
    { "background-origin", BackgroundOrigin },
    { "background-position", BackgroundPosition },
    { "background-repeat", BackgroundRepeat },
    { "border", Border },
    { "border-bottom", BorderBottom },
    { "border-bottom-color", BorderBottomColor },
    { "border-bottom-left-radius", BorderBottomLeftRadius },
    { "border-bottom-right-radius", BorderBottomRightRadius },
    { "border-bottom-style", BorderBottomStyle },
    { "border-bottom-width", BorderBottomWidth },
    { "border-color", BorderColor },
    { "border-image", BorderImage },
    { "border-left", BorderLeft },
    { "border-left-color", BorderLeftColor },
    { "border-left-style", BorderLeftStyle },
    { "border-left-width", BorderLeftWidth },
    { "border-radius", BorderRadius },
    { "border-right", BorderRight },
    { "border-right-color", BorderRightColor },
    { "border-right-style", BorderRightStyle },
    { "border-right-width", BorderRightWidth },
    { "border-style", BorderStyles },
    { "border-top", BorderTop },
    { "border-top-color", BorderTopColor },
    { "border-top-left-radius", BorderTopLeftRadius },
    { "border-top-right-radius", BorderTopRightRadius },
    { "border-top-style", BorderTopStyle },
    { "border-top-width", BorderTopWidth },
    { "border-width", BorderWidth },
    { "bottom", Bottom },
    { "center-horizontal", HbCenterHorizontal },
    { "center-vertical", HbCenterVertical },
    { "color", Color },
    { "column-narrow-width", HbColumnNarrowWidth },
    { "column-wide-width", HbColumnWideWidth },
    { "fixed-height", HbFixedHeight },
    { "fixed-width", HbFixedWidth },
    { "float", Float },
    { "font", Font },
    { "font-family", FontFamily },
    { "font-size", FontSize },
    { "font-style", FontStyle },
    { "font-variant", FontVariant },
    { "font-weight", FontWeight },
    { "height", Height },
    { "icon-left-alignment-weight", HbIconLeftAlignmentWeight },
    { "image", QtImage },
    { "image-position", QtImageAlignment },
    { "indent", HbIndent },
    { "large-icon-size", HbLargeIconSize },
    { "layout", HbLayout },
    { "left", Left },
    { "list-style", ListStyle },
    { "list-style-type", ListStyleType },
    { "margin" , Margin },
    { "margin-bottom", MarginBottom },
    { "margin-left", MarginLeft },
    { "margin-right", MarginRight },
    { "margin-top", MarginTop },
    { "margin-top-weight", HbTopMarginWeight },
    { "max-height", MaximumHeight },
    { "max-width", MaximumWidth },
    { "min-height", MinimumHeight },
    { "min-width", MinimumWidth },
    { "mirroring", Mirroring },
    { "outline", Outline },
    { "outline-bottom-left-radius", OutlineBottomLeftRadius },
    { "outline-bottom-right-radius", OutlineBottomRightRadius },
    { "outline-color", OutlineColor },
    { "outline-offset", OutlineOffset },
    { "outline-radius", OutlineRadius },
    { "outline-style", OutlineStyle },
    { "outline-top-left-radius", OutlineTopLeftRadius },
    { "outline-top-right-radius", OutlineTopRightRadius },
    { "outline-width", OutlineWidth },
    { "padding", Padding },
    { "padding-bottom", PaddingBottom },
    { "padding-left", PaddingLeft },
    { "padding-right", PaddingRight },
    { "padding-top", PaddingTop },
    { "page-break-after", PageBreakAfter },
    { "page-break-before", PageBreakBefore },
    { "position", Position },
    { "pref-height", HbPreferredHeight },
    { "pref-width", HbPreferredWidth },
    { "right", Right },
    { "section", HbSection },
    { "selection-background-color", QtSelectionBackground },
    { "selection-color", QtSelectionForeground },
    { "size-policy", HbSizePolicy },
    { "size-policy-horizontal", HbSizePolicyHorizontal },
    { "size-policy-vertical", HbSizePolicyVertical },
    { "small-icon-size", HbSmallIconSize },
    { "spacing", QtSpacing },
    { "spacing-horizontal", HbSpacingHorizontal },
    { "spacing-vertical", HbSpacingVertical },
    { "stretchable", HbStretchable },
    { "subcontrol-origin", QtOrigin },
    { "subcontrol-position", QtPosition },
    { "text-align", TextAlignment },
    { "text-decoration", TextDecoration },
    { "text-height", HbTextHeight },
    { "text-indent", TextIndent },
    { "text-line-count-max", HbTextLineCountMax },
    { "text-line-count-min", HbTextLineCountMin },
    { "text-transform", TextTransform },
    { "text-underline-style", TextUnderlineStyle },
    { "text-wrap-mode", HbTextWrapMode },
    { "top", Top },
    { "vertical-align", VerticalAlignment },
    { "white-space", Whitespace },
    { "width", Width },
    { "zvalue", ZValue }
};

static const HbCssKnownValue values[NumKnownValues - 1] = {
    { "active", Value_Active },
    { "alternate-base", Value_AlternateBase },
    { "always", Value_Always },
    { "auto", Value_Auto },
    { "base", Value_Base },
    { "bold", Value_Bold },
    { "bottom", Value_Bottom },
    { "bright-text", Value_BrightText },
    { "button", Value_Button },
    { "button-text", Value_ButtonText },
    { "center", Value_Center },
    { "circle", Value_Circle },
    { "dark", Value_Dark },
    { "dashed", Value_Dashed },
    { "decimal", Value_Decimal },
    { "digital", Value_Digital },
    { "disabled", Value_Disabled },
    { "disc", Value_Disc },
    { "dot-dash", Value_DotDash },
    { "dot-dot-dash", Value_DotDotDash },
    { "dotted", Value_Dotted },
    { "double", Value_Double },
    { "expanding", Value_Expanding },
    { "fixed", Value_Fixed },
    { "groove", Value_Groove },
    { "highlight", Value_Highlight },
    { "highlighted-text", Value_HighlightedText },
    { "ignore", Value_Ignore },
    { "ignored", Value_Ignored },
    { "inset", Value_Inset },
    { "italic", Value_Italic },
    { "keep", Value_Keep },
    { "keep-expand", Value_KeepExpand },
    { "large", Value_Large },
    { "left", Value_Left },
    { "light", Value_Light },
    { "line-through", Value_LineThrough },
    { "link", Value_Link },
    { "link-visited", Value_LinkVisited },
    { "lower-alpha", Value_LowerAlpha },
    { "lowercase", Value_Lowercase },
    { "maximum", Value_Maximum },
    { "medium", Value_Medium },
    { "mid", Value_Mid },
    { "middle", Value_Middle },
    { "midlight", Value_Midlight },
    { "minimum", Value_Minimum },
    { "minimum-expanding", Value_MinimumExpanding },
    { "mirrored", Value_Mirrored },
    { "native", Value_Native },
    { "no-wrap", Value_NoWrap },
    { "none", Value_None },
    { "normal", Value_Normal },
    { "oblique", Value_Oblique },
    { "off", Value_Off },
    { "on", Value_On },
    { "outset", Value_Outset },
    { "overline", Value_Overline },
    { "pre", Value_Pre },
    { "preferred", Value_Preferred },
    { "primary", Value_Primary },
    { "primary-small", Value_PrimarySmall },
    { "ridge", Value_Ridge },
    { "right", Value_Right },
    { "secondary", Value_Secondary },
    { "selected", Value_Selected },
    { "shadow", Value_Shadow },
    { "small" , Value_Small },
    { "small-caps", Value_SmallCaps },
    { "solid", Value_Solid },
    { "square", Value_Square },
    { "sub", Value_Sub },
    { "super", Value_Super },
    { "text", Value_Text },
    { "title", Value_Title },
    { "top", Value_Top },
    { "transparent", Value_Transparent },
    { "underline", Value_Underline },
    { "upper-alpha", Value_UpperAlpha },
    { "uppercase", Value_Uppercase },
    { "wave", Value_Wave },
    { "window", Value_Window },
    { "window-text", Value_WindowText },
    { "word-wrap", Value_WordWrap },
    { "wrap-anywhere", Value_WrapAnywhere },
    { "x-large", Value_XLarge },
    { "xx-large", Value_XXLarge }
};

static const HbCssKnownValue pseudos[NumPseudos - 1] = {
    { "active", PseudoClass_Active },
    { "adjoins-item", PseudoClass_Item },
    { "alternate", PseudoClass_Alternate },
    { "bottom", PseudoClass_Bottom },
    { "checked", PseudoClass_Checked },
    { "closable", PseudoClass_Closable },
    { "closed", PseudoClass_Closed },
    { "default", PseudoClass_Default },
    { "disabled", PseudoClass_Disabled },
    { "edit-focus", PseudoClass_EditFocus },
    { "editable", PseudoClass_Editable },
    { "enabled", PseudoClass_Enabled },
    { "exclusive", PseudoClass_Exclusive },
    { "first", PseudoClass_First },
    { "flat", PseudoClass_Flat },
    { "floatable", PseudoClass_Floatable },
    { "focus", PseudoClass_Focus },
    { "has-children", PseudoClass_Children },
    { "has-siblings", PseudoClass_Sibling },
    { "horizontal", PseudoClass_Horizontal },
    { "hover", PseudoClass_Hover },
    { "indeterminate" , PseudoClass_Indeterminate },
    { "landscape", PseudoClass_Landscape },
    { "last", PseudoClass_Last },
    { "left", PseudoClass_Left },
    { "maximized", PseudoClass_Maximized },
    { "middle", PseudoClass_Middle },
    { "minimized", PseudoClass_Minimized },
    { "movable", PseudoClass_Movable },
    { "next-selected", PseudoClass_NextSelected },
    { "no-frame", PseudoClass_Frameless },
    { "non-exclusive", PseudoClass_NonExclusive },
    { "off", PseudoClass_Unchecked },
    { "on", PseudoClass_Checked },
    { "only-one", PseudoClass_OnlyOne },
    { "open", PseudoClass_Open },
    { "portrait", PseudoClass_Portrait },
    { "pressed", PseudoClass_Pressed },
    { "previous-selected", PseudoClass_PreviousSelected },
    { "read-only", PseudoClass_ReadOnly },
    { "right", PseudoClass_Right },
    { "selected", PseudoClass_Selected },
    { "top", PseudoClass_Top },
    { "unchecked" , PseudoClass_Unchecked },
    { "vertical", PseudoClass_Vertical },
    { "window", PseudoClass_Window }
};

static const HbCssKnownValue origins[NumKnownOrigins - 1] = {
    { "border", Origin_Border },
    { "content", Origin_Content },
    { "margin", Origin_Margin }, // not in css
    { "padding", Origin_Padding }
};

static const HbCssKnownValue repeats[NumKnownRepeats - 1] = {
    { "no-repeat", Repeat_None },
    { "repeat-x", Repeat_X },
    { "repeat-xy", Repeat_XY },
    { "repeat-y", Repeat_Y }
};

static const HbCssKnownValue tileModes[NumKnownTileModes - 1] = {
    { "repeat", TileMode_Repeat },
    { "round", TileMode_Round },
    { "stretch", TileMode_Stretch },
};

static const HbCssKnownValue positions[NumKnownPositionModes - 1] = {
    { "absolute", PositionMode_Absolute },
    { "fixed", PositionMode_Fixed },
    { "relative", PositionMode_Relative },
    { "static", PositionMode_Static }
};

static const HbCssKnownValue attachments[NumKnownAttachments - 1] = {
    { "fixed", Attachment_Fixed },
    { "scroll", Attachment_Scroll }
};

static const HbCssKnownValue styleFeatures[NumKnownStyleFeatures - 1] = {
    { "background-color", StyleFeature_BackgroundColor },
    { "background-gradient", StyleFeature_BackgroundGradient },
    { "none", StyleFeature_None }
};

inline bool operator<(const QString &name, const HbCssKnownValue &prop)
{
    return QString::compare(name, QLatin1String(prop.name), Qt::CaseInsensitive) < 0;
}

inline bool operator<(const HbCssKnownValue &prop, const QString &name)
{
    return QString::compare(QLatin1String(prop.name), name, Qt::CaseInsensitive) < 0;
}

static quint64 findKnownValue(const QString &name, const HbCssKnownValue *start, int numValues)
{
    const HbCssKnownValue *end = &start[numValues - 1];
    const HbCssKnownValue *prop = qBinaryFind(start, end, name);
    if (prop == end)
        return 0;
    return prop->id;
}

///////////////////////////////////////////////////////////////////////////////
// Value Extractor
ValueExtractor::ValueExtractor(const HbVector<Declaration> &decls, const HbDeviceProfile &profile, const QPalette &pal)
: declarations(decls), adjustment(0), fontExtracted(false), pal(pal), currentProfile(profile)
{
}

ValueExtractor::ValueExtractor(const HbVector<Declaration> &decls, const HbVector<Declaration> &varDeclarations,
                               const HbDeviceProfile &profile, const QPalette &pal)
: declarations(decls), variableDeclarations(varDeclarations), adjustment(0), 
  fontExtracted(false), pal(pal), currentProfile(profile)
{
}

ValueExtractor::ValueExtractor(const HbVector<Declaration> &varDecls, bool isVariable, const HbDeviceProfile &profile)
: variableDeclarations(varDecls), adjustment(0), fontExtracted(false), currentProfile(profile)
{
    Q_UNUSED(isVariable)
    // Initialize to some profile.
    if ( currentProfile.isNull() )
        currentProfile = HbDeviceProfile::current();
}

int ValueExtractor::lengthValue(const Value& v)
{
    return qRound(asReal(v));
}

qreal ValueExtractor::asReal(const Value& v)
{
    QString s = v.variant.toString();
    s.reserve(s.length());

    if (v.type == Value::Expression || v.type == Value::ExpressionNegative) {
        qreal factor = (v.type == Value::Expression) ? 1.0 : -1.0;
        qreal value = 0;
        extractExpressionValue(s, value);
        return factor * value;
    }

    return asReal(s, v.type);
}

qreal ValueExtractor::asReal(QString &s, Value::Type type)
{
    if (type == Value::Variable || type == Value::VariableNegative) {
        qreal factor = (type == Value::Variable) ? 1.0 : -1.0;
        HbVector<HbCss::Value> values;
        if (extractValue(s, values))
            return factor * asReal(values.first());
        else
            return 0;
    }

    enum { None, Px, Un, Mm } unit = None;
    if (s.endsWith(QLatin1String("un"), Qt::CaseInsensitive)) {
        unit = Un;
    } else if (s.endsWith(QLatin1String("px"), Qt::CaseInsensitive)) {
        unit = Px;
    } else if (s.endsWith(QLatin1String("mm"), Qt::CaseInsensitive)) {
        unit = Mm;
    }

    if (unit != None) {
        // Assuming all unit identifiers have two characters
        s.chop(2);
    }

    bool ok;
    qreal result = s.toDouble(&ok);
    if (!ok) {
        return 0;
    }

    if (unit == Un) {
        result = currentProfile.unitValue() * result;
    } else if (unit == Mm) {
        result = currentProfile.ppmValue() * result;
    } // else -> already in pixels
    return result;
}

bool ValueExtractor::asReal(QString &s, qreal &value)
{
    enum { None, Px, Un, Mm } unit = None;
    if (s.endsWith(QLatin1String("un"), Qt::CaseInsensitive)) {
        unit = Un;
    } else if (s.endsWith(QLatin1String("px"), Qt::CaseInsensitive)) {
        unit = Px;
    } else if (s.endsWith(QLatin1String("mm"), Qt::CaseInsensitive)) {
        unit = Mm;
    }

    if (unit != None) {
        // Assuming all unit identifiers have two characters
        s.chop(2);
    }

    bool ok;
    value = s.toDouble(&ok);
    if (!ok) {
        return false;
    }

    if (unit == Un) {
        value = currentProfile.unitValue() * value;
    } else if (unit == Mm) {
        value = currentProfile.ppmValue() * value;
    } // else -> already in pixels
    return true;
}

qreal ValueExtractor::asReal(const Declaration &decl)
{
    if (decl.values.count() < 1)
        return 0;
    return asReal(decl.values.first());
}


void ValueExtractor::asReals(const Declaration &decl, qreal *m)
{
    int i;
    for (i = 0; i < qMin(decl.values.count(), 4); i++)
        m[i] = asReal(decl.values[i]);

    if (i == 0) m[0] = m[1] = m[2] = m[3] = 0;
    else if (i == 1) m[3] = m[2] = m[1] = m[0];
    else if (i == 2) m[2] = m[0], m[3] = m[1];
    else if (i == 3) m[3] = m[1];
}

bool ValueExtractor::asBool(const Declaration &decl)
{
    if (decl.values.size()) {
        bool result = (decl.values.at(0).variant.toString() == QString("true"));
        return result;
    }
    return false;
}

QSizePolicy ValueExtractor::asSizePolicy(const Declaration &decl)
{
    QSizePolicy pol;
    if (decl.values.count() > 0)
        pol.setHorizontalPolicy(asPolicy(decl.values.at(0)));
    if (decl.values.count() > 1)
        pol.setHorizontalPolicy(asPolicy(decl.values.at(1)));
    return pol;
}

QSizePolicy::Policy ValueExtractor::asPolicy(const Value& v)
{
    QSizePolicy::Policy pol(QSizePolicy::Preferred);
    switch (v.variant.toInt())
        {
        case Value_Fixed: pol = QSizePolicy::Fixed; break;
        case Value_Minimum: pol = QSizePolicy::Minimum; break;
        case Value_Maximum: pol = QSizePolicy::Maximum; break;
        case Value_Preferred: pol = QSizePolicy::Preferred; break;
        case Value_Expanding: pol = QSizePolicy::Expanding; break;
        case Value_MinimumExpanding: pol = QSizePolicy::MinimumExpanding; break;
        case Value_Ignored: pol = QSizePolicy::Ignored; break;
        default: break;
        }
    return pol;
}



int ValueExtractor::lengthValue(const Declaration &decl)
{
    if (decl.values.count() < 1)
        return 0;
    return lengthValue(decl.values.first());
}

void ValueExtractor::lengthValues(const Declaration &decl, int *m)
{
    int i;
    for (i = 0; i < qMin(decl.values.count(), 4); i++)
        m[i] = lengthValue(decl.values[i]);

    if (i == 0) m[0] = m[1] = m[2] = m[3] = 0;
    else if (i == 1) m[3] = m[2] = m[1] = m[0];
    else if (i == 2) m[2] = m[0], m[3] = m[1];
    else if (i == 3) m[3] = m[1];
}

bool ValueExtractor::extractGeometry(GeometryValues &geomValues)
{
    GeometryValueFlags flags(0);
    extractFont();
    bool hit = false;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case MinimumWidth: geomValues.mMinW = asReal(decl); flags|=ExtractedMinW; break;
        case MinimumHeight: geomValues.mMinH = asReal(decl); flags|=ExtractedMinH; break;
        case MaximumWidth: geomValues.mMaxW = asReal(decl); flags|=ExtractedMaxW; break;
        case MaximumHeight: geomValues.mMaxH = asReal(decl); flags|=ExtractedMaxH; break;
        case HbPreferredWidth: geomValues.mPrefW = asReal(decl); flags|=ExtractedPrefW; break;
        case HbPreferredHeight: geomValues.mPrefH = asReal(decl); flags|=ExtractedPrefH; break;
        case HbFixedWidth:
            geomValues.mPrefW = asReal(decl); flags|=ExtractedPrefW;
            geomValues.mSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed); flags|=ExtractedPolHor;
            break;
        case HbFixedHeight:
            geomValues.mPrefH = asReal(decl); flags|=ExtractedPrefH;
            geomValues.mSizePolicy.setVerticalPolicy(QSizePolicy::Fixed); flags|=ExtractedPolVer;
            break;
        case HbSizePolicy:
            geomValues.mSizePolicy.setHorizontalPolicy(asPolicy(decl.values.at(0)));
            if (decl.values.count() > 1) {
                geomValues.mSizePolicy.setVerticalPolicy(asPolicy(decl.values.at(1)));
            } else {
                geomValues.mSizePolicy.setVerticalPolicy(asPolicy(decl.values.at(0)));
            }
            flags|=ExtractedPolHor;
            flags|=ExtractedPolVer;
            break;
        case HbSizePolicyHorizontal:
            geomValues.mSizePolicy.setHorizontalPolicy(asPolicy(decl.values.at(0)));
            flags|=ExtractedPolHor;
            break;
        case HbSizePolicyVertical:
            geomValues.mSizePolicy.setVerticalPolicy(asPolicy(decl.values.at(0)));
            flags|=ExtractedPolVer;
            break;
        default: continue;
        }
        hit = true;
    }
    geomValues.mFlags = flags;
    return hit;
}

static bool parseMirroringValue(const Value v)
{
    if(v.type == Value::KnownIdentifier) {
        switch(v.variant.toInt()) {
        case Value_Mirrored:
            return true;
        case Value_Disabled:
            return false;
        default:
            break;
        }
    }
    return true; // Enabled as default
}

bool ValueExtractor::extractPosition(PositionValues &posValues)
{
    PositionValueFlags flags(0);
    extractFont();
    bool hit = false;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case Left: posValues.mLeft = asReal(decl); flags|=ExtractedLeft; break;
        case Top: posValues.mTop = asReal(decl); flags|=ExtractedTop; break;
        case Right: posValues.mRight = asReal(decl); flags|=ExtractedRight; break;
        case Bottom: posValues.mBottom = asReal(decl); flags|=ExtractedBottom; break;
        case HbCenterHorizontal: posValues.mCenterH = asReal(decl); flags|=ExtractedCenterH; break;
        case HbCenterVertical: posValues.mCenterV = asReal(decl); flags|=ExtractedCenterV; break;
        case QtOrigin: posValues.mOrigin = decl.originValue(); flags|=ExtractedOrigin; break;
        case QtPosition: posValues.mPosition = decl.alignmentValue(); flags|=ExtractedAlign; break;
        case TextAlignment: posValues.mTextAlignment = decl.alignmentValue(); flags|=ExtractedTextAlign; break;
        case Position: posValues.mPositionMode = decl.positionValue(); flags|=ExtractedMode; break;
        case Mirroring: posValues.mMirroring = parseMirroringValue(decl.values.at(0)); flags|=ExtractedMirroring; break;
        case ZValue: posValues.mZ = asReal(decl); flags|=ExtractedZValue; break;
        case HbTextWrapMode: posValues.mTextWrapMode = decl.wrapModeValue(); flags|=ExtractedWrapMode; break;
        default: continue;
        }
        hit = true;
    }
    posValues.mFlags = flags;
    return hit;
}

bool ValueExtractor::extractTextValues( TextValues &textValues )
{
    textValues.mFlags = 0;
    bool hit = false;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case HbTextLineCountMin: textValues.mLineCountMin = decl.values.first().variant.toInt(); textValues.mFlags|=ExtractedLineCountMin; break;
        case HbTextLineCountMax: textValues.mLineCountMax = decl.values.first().variant.toInt(); textValues.mFlags|=ExtractedLineCountMax; break;
        default: continue;
        }
        hit = true;
    }
    return hit;
}

bool ValueExtractor::extractBox(qreal *margins, qreal *paddings, qreal *spacing)
{
    extractFont();
    bool hit = false;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case PaddingLeft: paddings[LeftEdge] = asReal(decl); break;
        case PaddingRight: paddings[RightEdge] = asReal(decl); break;
        case PaddingTop: paddings[TopEdge] = asReal(decl); break;
        case PaddingBottom: paddings[BottomEdge] = asReal(decl); break;
        case Padding: asReals(decl, paddings); break;

        case MarginLeft: margins[LeftEdge] = asReal(decl); break;
        case MarginRight: margins[RightEdge] = asReal(decl); break;
        case MarginTop: margins[TopEdge] = asReal(decl); break;
        case MarginBottom: margins[BottomEdge] = asReal(decl); break;
        case Margin: asReals(decl, margins); break;
        case QtSpacing: if (spacing) *spacing = asReal(decl); break;

        default: continue;
        }
        hit = true;
    }

    return hit;
}

int ValueExtractor::extractStyleFeatures()
{
    int features = StyleFeature_None;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        if (decl.propertyId == QtStyleFeatures)
            features = decl.styleFeaturesValue();
    }
    return features;
}

QSize ValueExtractor::sizeValue(const Declaration &decl)
{
    int x[2] = { 0, 0 };
    if (decl.values.count() > 0)
        x[0] = lengthValue(decl.values.at(0));
    if (decl.values.count() > 1)
        x[1] = lengthValue(decl.values.at(1));
    else
        x[1] = x[0];
    return QSize(x[0], x[1]);
}

void ValueExtractor::sizeValues(const Declaration &decl, QSize *radii)
{
    radii[0] = sizeValue(decl);
    for (int i = 1; i < 4; i++)
        radii[i] = radii[0];
}

bool ValueExtractor::extractBorder(qreal *borders, QBrush *colors, BorderStyle *styles,
                                   QSize *radii)
{
    extractFont();
    bool hit = false;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case BorderLeftWidth: borders[LeftEdge] = asReal(decl); break;
        case BorderRightWidth: borders[RightEdge] = asReal(decl); break;
        case BorderTopWidth: borders[TopEdge] = asReal(decl); break;
        case BorderBottomWidth: borders[BottomEdge] = asReal(decl); break;
        case BorderWidth: asReals(decl, borders); break;

        case BorderLeftColor: colors[LeftEdge] = decl.brushValue(pal); break;
        case BorderRightColor: colors[RightEdge] = decl.brushValue(pal); break;
        case BorderTopColor: colors[TopEdge] = decl.brushValue(pal); break;
        case BorderBottomColor: colors[BottomEdge] = decl.brushValue(pal); break;
        case BorderColor: decl.brushValues(colors, pal); break;

        case BorderTopStyle: styles[TopEdge] = decl.styleValue(); break;
        case BorderBottomStyle: styles[BottomEdge] = decl.styleValue(); break;
        case BorderLeftStyle: styles[LeftEdge] = decl.styleValue(); break;
        case BorderRightStyle: styles[RightEdge] = decl.styleValue(); break;
        case BorderStyles:  decl.styleValues(styles); break;

        case BorderTopLeftRadius: radii[0] = sizeValue(decl); break;
        case BorderTopRightRadius: radii[1] = sizeValue(decl); break;
        case BorderBottomLeftRadius: radii[2] = sizeValue(decl); break;
        case BorderBottomRightRadius: radii[3] = sizeValue(decl); break;
        case BorderRadius: sizeValues(decl, radii); break;

        case BorderLeft:
            borderValue(decl, &borders[LeftEdge], &styles[LeftEdge], &colors[LeftEdge]);
            break;
        case BorderTop:
            borderValue(decl, &borders[TopEdge], &styles[TopEdge], &colors[TopEdge]);
            break;
        case BorderRight:
            borderValue(decl, &borders[RightEdge], &styles[RightEdge], &colors[RightEdge]);
            break;
        case BorderBottom:
            borderValue(decl, &borders[BottomEdge], &styles[BottomEdge], &colors[BottomEdge]);
            break;
        case Border:
            borderValue(decl, &borders[LeftEdge], &styles[LeftEdge], &colors[LeftEdge]);
            borders[TopEdge] = borders[RightEdge] = borders[BottomEdge] = borders[LeftEdge];
            styles[TopEdge] = styles[RightEdge] = styles[BottomEdge] = styles[LeftEdge];
            colors[TopEdge] = colors[RightEdge] = colors[BottomEdge] = colors[LeftEdge];
            break;

        default: continue;
        }
        hit = true;
    }

    return hit;
}

bool ValueExtractor::extractOutline(qreal *borders, QBrush *colors, BorderStyle *styles,
                                   QSize *radii, qreal *offsets)
{
    extractFont();
    bool hit = false;
    for (int i = 0; i < declarations.count(); i++) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case OutlineWidth: asReals(decl, borders); break;
        case OutlineColor: decl.brushValues(colors, pal); break;
        case OutlineStyle:  decl.styleValues(styles); break;

        case OutlineTopLeftRadius: radii[0] = sizeValue(decl); break;
        case OutlineTopRightRadius: radii[1] = sizeValue(decl); break;
        case OutlineBottomLeftRadius: radii[2] = sizeValue(decl); break;
        case OutlineBottomRightRadius: radii[3] = sizeValue(decl); break;
        case OutlineRadius: sizeValues(decl, radii); break;
        case OutlineOffset: asReals(decl, offsets); break;

        case Outline:
            borderValue(decl, &borders[LeftEdge], &styles[LeftEdge], &colors[LeftEdge]);
            borders[TopEdge] = borders[RightEdge] = borders[BottomEdge] = borders[LeftEdge];
            styles[TopEdge] = styles[RightEdge] = styles[BottomEdge] = styles[LeftEdge];
            colors[TopEdge] = colors[RightEdge] = colors[BottomEdge] = colors[LeftEdge];
            break;

        default: continue;
        }
        hit = true;
    }

    return hit;
}

static Qt::Alignment parseAlignment(const Value *values, int count)
{
    Qt::Alignment a[2] = { 0, 0 };
    for (int i = 0; i < qMin(2, count); i++) {
        if (values[i].type != Value::KnownIdentifier)
            break;
        switch (values[i].variant.toInt()) {
        case Value_Left: a[i] = Qt::AlignLeft; break;
        case Value_Right: a[i] = Qt::AlignRight; break;
        case Value_Top: a[i] = Qt::AlignTop; break;
        case Value_Bottom: a[i] = Qt::AlignBottom; break;
        case Value_Center: a[i] = Qt::AlignCenter; break;
        default: break;
        }
    }

    if (a[0] == Qt::AlignCenter && a[1] != 0 && a[1] != Qt::AlignCenter)
        a[0] = (a[1] == Qt::AlignLeft || a[1] == Qt::AlignRight) ? Qt::AlignVCenter : Qt::AlignHCenter;
    if ((a[1] == 0 || a[1] == Qt::AlignCenter) && a[0] != Qt::AlignCenter)
        a[1] = (a[0] == Qt::AlignLeft || a[0] == Qt::AlignRight) ? Qt::AlignVCenter : Qt::AlignHCenter;
    return a[0] | a[1];
}

static Hb::TextWrapping parseWrapMode(const Value v)
{
    Hb::TextWrapping mode(Hb::TextNoWrap);
    if (v.type == Value::KnownIdentifier) {
        switch(v.variant.toInt()) {
        case Value_WordWrap: mode = Hb::TextWordWrap; break;
        case Value_WrapAnywhere: mode = Hb::TextWrapAnywhere; break;
        case Value_NoWrap: // fall-through
        default: break;
        }
    }
    return mode;
}

static QColor parseColorValue(Value v, const QPalette &pal)
{
    if (v.type == Value::Identifier || v.type == Value::String || v.type == Value::Color)
        return v.variant.toColor();

    if (v.type == Value::KnownIdentifier && v.variant.toInt() == Value_Transparent)
        return Qt::transparent;

    if (v.type != Value::Function)
        return QColor();

    QStringList lst = v.variant.toStringList();
    if (lst.count() != 2)
        return QColor();

    if ((lst.at(0).compare(QLatin1String("palette"), Qt::CaseInsensitive)) == 0) {
        int role = findKnownValue(lst.at(1), values, NumKnownValues);
        if (role >= Value_FirstColorRole && role <= Value_LastColorRole)
            return pal.color((QPalette::ColorRole)(role-Value_FirstColorRole));

        return QColor();
    }

    bool rgb = lst.at(0).startsWith(QLatin1String("rgb"));

    Parser p(lst.at(1));
    if (!p.testExpr())
        return QColor();

    HbVector<Value> colorDigits(v.memoryType);
    if (!p.parseExpr(&colorDigits))
        return QColor();

    for (int i = 0; i < qMin(colorDigits.count(), 7); i += 2) {
        if (colorDigits.at(i).type == Value::Percentage) {
            colorDigits[i].variant = colorDigits.at(i).variant.toDouble() * 255. / 100.;
            colorDigits[i].type = Value::Number;
        } else if (colorDigits.at(i).type != Value::Number) {
            return QColor();
        }
    }

    int v1 = colorDigits.at(0).variant.toInt();
    int v2 = colorDigits.at(2).variant.toInt();
    int v3 = colorDigits.at(4).variant.toInt();
    int alpha = colorDigits.count() >= 7 ? colorDigits.at(6).variant.toInt() : 255;

    return rgb ? QColor::fromRgb(v1, v2, v3, alpha)
               : QColor::fromHsv(v1, v2, v3, alpha);
}

static QBrush parseBrushValue(Value v, const QPalette &pal)
{
    QColor c = parseColorValue(v, pal);
    if (c.isValid())
        return QBrush(c);

    if (v.type != Value::Function)
        return QBrush();

    QStringList lst = v.variant.toStringList();
    if (lst.count() != 2)
        return QBrush();

    QStringList gradFuncs;
    gradFuncs << QLatin1String("qlineargradient") << QLatin1String("qradialgradient") << QLatin1String("qconicalgradient") << QLatin1String("qgradient");
    int gradType = -1;

    if ((gradType = gradFuncs.indexOf(lst.at(0).toLower())) == -1)
        return QBrush();

    QHash<QString, qreal> vars;
    QVector<QGradientStop> stops;

    int spread = -1;
    QStringList spreads;
    spreads << QLatin1String("pad") << QLatin1String("reflect") << QLatin1String("repeat");

    Parser parser(lst.at(1));
    while (parser.hasNext()) {
        parser.skipSpace();
        if (!parser.test(IDENT))
            return QBrush();
        QString attr = parser.lexem();
        parser.skipSpace();
        if (!parser.test(COLON))
            return QBrush();
        parser.skipSpace();
        if (attr.compare(QLatin1String("stop"), Qt::CaseInsensitive) == 0) {
            Value stop, color;
            parser.next();
            if (!parser.parseTerm(&stop)) return QBrush();
            parser.skipSpace();
            parser.next();
            if (!parser.parseTerm(&color)) return QBrush();
            stops.append(QGradientStop(stop.variant.toDouble(), parseColorValue(color, pal)));
        } else {
            parser.next();
            Value value;
            parser.parseTerm(&value);
            if (attr.compare(QLatin1String("spread"), Qt::CaseInsensitive) == 0) {
                spread = spreads.indexOf(value.variant.toString());
            } else {
                vars[attr] = value.variant.toString().toDouble();
            }
        }
        parser.skipSpace();
        parser.test(COMMA);
    }

    if (gradType == 0) {
        QLinearGradient lg(vars.value(QLatin1String("x1")), vars.value(QLatin1String("y1")),
                           vars.value(QLatin1String("x2")), vars.value(QLatin1String("y2")));
        lg.setCoordinateMode(QGradient::ObjectBoundingMode);
        lg.setStops(stops);
        if (spread != -1)
            lg.setSpread(QGradient::Spread(spread));
        return QBrush(lg);
    }

    if (gradType == 1) {
        QRadialGradient rg(vars.value(QLatin1String("cx")), vars.value(QLatin1String("cy")),
                           vars.value(QLatin1String("radius")), vars.value(QLatin1String("fx")),
                           vars.value(QLatin1String("fy")));
        rg.setCoordinateMode(QGradient::ObjectBoundingMode);
        rg.setStops(stops);
        if (spread != -1)
            rg.setSpread(QGradient::Spread(spread));
        return QBrush(rg);
    }

    if (gradType == 2) {
        QConicalGradient cg(vars.value(QLatin1String("cx")), vars.value(QLatin1String("cy")),
                            vars.value(QLatin1String("angle")));
        cg.setCoordinateMode(QGradient::ObjectBoundingMode);
        cg.setStops(stops);
        if (spread != -1)
            cg.setSpread(QGradient::Spread(spread));
        return QBrush(cg);
    }

    return QBrush();
}

static BorderStyle parseStyleValue(Value v)
{
    if (v.type == Value::KnownIdentifier) {
        switch (v.variant.toInt()) {
        case Value_None:
            return BorderStyle_None;
        case Value_Dotted:
            return BorderStyle_Dotted;
        case Value_Dashed:
            return BorderStyle_Dashed;
        case Value_Solid:
            return BorderStyle_Solid;
        case Value_Double:
            return BorderStyle_Double;
        case Value_DotDash:
            return BorderStyle_DotDash;
        case Value_DotDotDash:
            return BorderStyle_DotDotDash;
        case Value_Groove:
            return BorderStyle_Groove;
        case Value_Ridge:
            return BorderStyle_Ridge;
        case Value_Inset:
            return BorderStyle_Inset;
        case Value_Outset:
            return BorderStyle_Outset;
        case Value_Native:
            return BorderStyle_Native;
        default:
            break;
        }
    }

    return BorderStyle_Unknown;
}

void ValueExtractor::borderValue(const Declaration &decl, qreal *width, HbCss::BorderStyle *style, QBrush *color)
{
    *width = 0;
    *style = BorderStyle_None;
    *color = QColor();

    if (decl.values.isEmpty())
        return;

    int i = 0;
    if (decl.values.at(i).type == Value::Length || decl.values.at(i).type == Value::Number) {
        *width = asReal(decl.values.at(i));
        if (++i >= decl.values.count())
            return;
    }

    *style = parseStyleValue(decl.values.at(i));
    if (*style != BorderStyle_Unknown) {
        if (++i >= decl.values.count())
            return;
    } else {
        *style = BorderStyle_None;
    }

    *color = parseBrushValue(decl.values.at(i), pal);
}

static void parseShorthandBackgroundProperty(const HbVector<Value> &values, QBrush *brush, QString *image, Repeat *repeat, Qt::Alignment *alignment, const QPalette &pal)
{
    *brush = QBrush();
    image->clear();
    *repeat = Repeat_XY;
    *alignment = Qt::AlignTop | Qt::AlignLeft;

    for (int i = 0; i < values.count(); ++i) {
        const Value v = values.at(i);
        if (v.type == Value::Uri) {
            *image = v.variant.toString();
            continue;
        } else if (v.type == Value::KnownIdentifier && v.variant.toInt() == Value_None) {
            image->clear();
            continue;
        } else if (v.type == Value::KnownIdentifier && v.variant.toInt() == Value_Transparent) {
            *brush = QBrush(Qt::transparent);
        }

        Repeat repeatAttempt = static_cast<Repeat>(findKnownValue(v.variant.toString(),
                                                   repeats, NumKnownRepeats));
        if (repeatAttempt != Repeat_Unknown) {
            *repeat = repeatAttempt;
            continue;
        }

        if (v.type == Value::KnownIdentifier) {
            const int start = i;
            int count = 1;
            if (i < values.count() - 1
                && values.at(i + 1).type == Value::KnownIdentifier) {
                ++i;
                ++count;
            }
            Qt::Alignment a = parseAlignment(values.constData() + start, count);
            if (int(a) != 0) {
                *alignment = a;
                continue;
            }
            i -= count - 1;
        }

        *brush = parseBrushValue(v, pal);
    }
}

bool ValueExtractor::extractBackground(QBrush *brush, QString *image, Repeat *repeat,
                                       Qt::Alignment *alignment, Origin *origin, Attachment *attachment,
                                       Origin *clip)
{
    bool hit = false;
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        if (decl.values.isEmpty())
            continue;
        const Value val = decl.values.first();
        switch (decl.propertyId) {
            case BackgroundColor:
                *brush = parseBrushValue(val, pal);
                break;
            case BackgroundImage:
                if (val.type == Value::Uri)
                    *image = val.variant.toString();
                break;
            case BackgroundRepeat:
                *repeat = static_cast<Repeat>(findKnownValue(val.variant.toString(),
                                              repeats, NumKnownRepeats));
                break;
            case BackgroundPosition:
                *alignment = decl.alignmentValue();
                break;
            case BackgroundOrigin:
                *origin = decl.originValue();
                break;
            case BackgroundClip:
                *clip = decl.originValue();
                break;
            case Background:
                parseShorthandBackgroundProperty(decl.values, brush, image, repeat, alignment, pal);
                break;
            case BackgroundAttachment:
                *attachment = decl.attachmentValue();
                break;
            default: continue;
        }
        hit = true;
    }
    return hit;
}

static bool setFontSizeFromValue(Value value, QFont *font, int *fontSizeAdjustment)
{
    if (value.type == Value::KnownIdentifier) {
        bool valid = true;
        switch (value.variant.toInt()) {
            case Value_Small: *fontSizeAdjustment = -1; break;
            case Value_Medium: *fontSizeAdjustment = 0; break;
            case Value_Large: *fontSizeAdjustment = 1; break;
            case Value_XLarge: *fontSizeAdjustment = 2; break;
            case Value_XXLarge: *fontSizeAdjustment = 3; break;
            default: valid = false; break;
        }
        return valid;
    }
    if (value.type != Value::Length)
        return false;

    bool valid = false;
    QString s = value.variant.toString();
    if (s.endsWith(QLatin1String("pt"), Qt::CaseInsensitive)) {
        s.chop(2);
        value.variant = s;
        if (value.variant.convert(HbVariant::Double)) {
            font->setPointSizeF(value.variant.toDouble());
            valid = true;
        }
    } else if (s.endsWith(QLatin1String("px"), Qt::CaseInsensitive)) {
        s.chop(2);
        value.variant = s;
        if (value.variant.convert(HbVariant::Int)) {
            font->setPixelSize(value.variant.toInt());
            valid = true;
        }
    }
    return valid;
}

static bool setFontStyleFromValue(const Value &value, QFont *font)
{
    if (value.type != Value::KnownIdentifier)
        return false ;
    switch (value.variant.toInt()) {
        case Value_Normal: font->setStyle(QFont::StyleNormal); return true;
        case Value_Italic: font->setStyle(QFont::StyleItalic); return true;
        case Value_Oblique: font->setStyle(QFont::StyleOblique); return true;
        default: break;
    }
    return false;
}

static bool setFontWeightFromValue(const Value &value, QFont *font)
{
    if (value.type == Value::KnownIdentifier) {
        switch (value.variant.toInt()) {
            case Value_Normal: font->setWeight(QFont::Normal); return true;
            case Value_Bold: font->setWeight(QFont::Bold); return true;
            default: break;
        }
        return false;
    }
    if (value.type != Value::Number)
        return false;
    font->setWeight(qMin(value.variant.toInt() / 8, 99));
    return true;
}

static bool setFontFamilyFromValues(const HbVector<Value> &values, QFont *font)
{
    QString family;
    for (int i = 0; i < values.count(); ++i) {
        const Value &v = values.at(i);
        if (v.type == Value::TermOperatorComma)
            break;
        const QString str = v.variant.toString();
        if (str.isEmpty())
            break;
        family += str;
        family += QLatin1Char(' ');
    }
    family = family.simplified();
    if (family.isEmpty())
        return false;
    font->setFamily(family);
    return true;
}

static void setTextDecorationFromValues(const HbVector<Value> &values, QFont *font)
{
    for (int i = 0; i < values.count(); ++i) {
        if (values.at(i).type != Value::KnownIdentifier)
            continue;
        switch (values.at(i).variant.toInt()) {
            case Value_Underline: font->setUnderline(true); break;
            case Value_Overline: font->setOverline(true); break;
            case Value_LineThrough: font->setStrikeOut(true); break;
            case Value_None:
                font->setUnderline(false);
                font->setOverline(false);
                font->setStrikeOut(false);
                break;
            default: break;
        }
    }
}

static void parseShorthandFontProperty(const HbVector<Value> &values, QFont *font, int *fontSizeAdjustment)
{
    font->setStyle(QFont::StyleNormal);
    font->setWeight(QFont::Normal);
    *fontSizeAdjustment = 0;

    int i = 0;
    while (i < values.count()) {
        if (setFontStyleFromValue(values.at(i), font)
            || setFontWeightFromValue(values.at(i), font))
            ++i;
        else
            break;
    }

    if (i < values.count()) {
        setFontSizeFromValue(values.at(i), font, fontSizeAdjustment);
        ++i;
    }

    if (i < values.count()) {
        QString fam = values.at(i).variant.toString();
        if (!fam.isEmpty())
            font->setFamily(fam);
    }
}

static void setFontVariantFromValue(const Value &value, HbFontSpec *fontSpec, QFont *font )
{
    // Sets font variants. Some set the fontspec and some the HbFontSpec
    HbFontSpec::Role role( HbFontSpec::Undefined );
    if (value.type == Value::KnownIdentifier) {
        switch (value.variant.toInt()) {
            case Value_Normal: font->setCapitalization(QFont::MixedCase); break;
            case Value_SmallCaps: font->setCapitalization(QFont::SmallCaps); break;
            case Value_Primary: role = HbFontSpec::Primary; break;
            case Value_Secondary: role = HbFontSpec::Secondary; break;
            case Value_Title: role = HbFontSpec::Title; break;
            case Value_PrimarySmall: role = HbFontSpec::PrimarySmall; break;
            case Value_Digital: role = HbFontSpec::Digital; break;
            default: break;
        }
    }
    if (role != HbFontSpec::Undefined) {
        fontSpec->setRole( role );
    }
}

static void setTextTransformFromValue(const Value &value, QFont *font)
{
    if (value.type == Value::KnownIdentifier) {
        switch (value.variant.toInt()) {
            case Value_None: font->setCapitalization(QFont::MixedCase); break;
            case Value_Uppercase: font->setCapitalization(QFont::AllUppercase); break;
            case Value_Lowercase: font->setCapitalization(QFont::AllLowercase); break;
            default: break;
        }
    }
}
bool ValueExtractor::extractValue(const QString& variableName, HbVector<HbCss::Value>& values) const
{
    bool variableFound = false;
    for (int i=variableDeclarations.count()-1; i>=0;i--) {
        if (variableDeclarations[i].property == variableName ) {
            values = variableDeclarations[i].values;
            variableFound = true;
            break;
        }
    }
    return variableFound;
}

bool ValueExtractor::extractValue(const QString& variableName, qreal& value)
{
    bool variableFound = false;
    HbVector<Value> values;
    if (extractValue(variableName, values)) {
        value = asReal(values.first());
        variableFound = true;
    }
    return variableFound;    
}

bool ValueExtractor::extractValue( const QString& variableName, HbCss::Value &val ) const
{
    bool variableFound = false;
    HbVector<HbCss::Value> values;
    for ( int i=variableDeclarations.count()-1; i>=0;i-- ) {
        if ( variableDeclarations[i].property == variableName ) {
            values = variableDeclarations[i].values;
            variableFound = true;
            break;
        }
    }
    //for variable cascading support
    if ( variableFound ) {
        val = values.first();
        if ( val.type == Value::Variable ){
            variableFound = extractValue ( val.variant.toString (), val );
        }
    }else {
        HbLayeredStyleLoader *styleLoader = HbLayeredStyleLoader::getStack(HbLayeredStyleLoader::Concern_Colors);
        if (styleLoader) {
            variableFound = styleLoader->findInDefaultVariables(variableName, val);
        }
    }
    return variableFound;
}

bool ValueExtractor::extractExpressionValue(QString &expression, qreal &value)
{
    // todo: invalid global variables are not checked because asReal() doesn't check them
    value = 0;
    const QChar SPACE(' ');
    const QChar LPARENTHESIS('(');
    const QChar RPARENTHESIS(')');
    const QChar MINUS('-');
    const QChar PLUS('+');
    const QChar STAR('*');
    const QChar SLASH('/');

    int position = 0;
    int begin = -1;
    // + and - are level 0, * and / in level 1, unary - is level 2
    // every parenthesis add 10 to (base) precedence level
    int precedenceLevel = 0; 
    bool parseVariable = false;
    bool endMark = false;
    int operatorCount = 1; // there can only be 2 sequental operators if the latter one is unary '-'
    while (position < expression.size()) {
        endMark = false;
        if (expression.at(position) == SPACE) {
            endMark = true;
        } else if ((expression.at(position) == LPARENTHESIS) && !parseVariable) {
            precedenceLevel+=10;
            position++;
            operatorCount = 1;
            continue;
        } else if (expression.at(position) == RPARENTHESIS) {
            if (parseVariable) {
                parseVariable = false;
                operatorCount = 0;
                position++;
                continue;
            }
            precedenceLevel-=10;
            operatorCount = 0;
            endMark = true;
        } else if ((expression.at(position) == MINUS) && !parseVariable) {
            endMark = true;
        } else if ((expression.at(position) == PLUS) ||
                   (expression.at(position) == STAR) ||
                   (expression.at(position) == SLASH)) {
            endMark = true;
        }

        if (endMark) {
            if (begin >= 0) {
                // parse value
                QString valueString = expression.mid(begin, position - begin);
                qreal val = 0;
                if (valueString.startsWith("var(") && valueString.endsWith(")")) {
                    // remove var( and last )
                    QString variableString = valueString.mid(4, valueString.size()-5);
                    if (!extractValue(variableString, val)) {
                        expressionValues.clear();
                        return false;
                    }
                } else {
                    if (!asReal(valueString, val)) {
                        expressionValues.clear();
                        return false;
                    }
                }
                expressionValues.append(ExpressionValue(ExpressionValue::None, 0, val));
                operatorCount = 0;
            }
            begin = -1;
            if (expression.at(position) == MINUS) {
                if (operatorCount == 1) {
                    expressionValues.append(ExpressionValue(ExpressionValue::UnaryMinus,precedenceLevel+2,0));
                } else if (operatorCount > 1) {
                    expressionValues.clear();
                    return false;
                } else {
                    expressionValues.append(ExpressionValue(ExpressionValue::Minus,precedenceLevel,0));
                }
                operatorCount++;
            } else if (expression.at(position) == PLUS) {
                if (operatorCount > 0) {
                    expressionValues.clear();
                    return false;
                }
                expressionValues.append(ExpressionValue(ExpressionValue::Plus,precedenceLevel,0));
                operatorCount++;
            } else if (expression.at(position) == STAR) {
                if (operatorCount > 0) {
                    expressionValues.clear();
                    return false;
                }
                expressionValues.append(ExpressionValue(ExpressionValue::Star,precedenceLevel+1,0));
                operatorCount++;
            } else if (expression.at(position) == SLASH) {
                if (operatorCount > 0) {
                    expressionValues.clear();
                    return false;
                }
                expressionValues.append(ExpressionValue(ExpressionValue::Slash,precedenceLevel+1,0));
                operatorCount++;
            }
            position++;
            continue;
        }

        if (begin == -1) {
            begin = position;
        }

        // flag variable parsing (variable syntax contains parenthesis)
        if ((expression.at(position) == QChar('v')) && !parseVariable) {
            parseVariable = true;
            position++;
            continue;
        }
        position++;
    }

    // check for unmatching parentheses
    if (precedenceLevel != 0) {
        expressionValues.clear();
        return false;
    }

    // parse last value
    if (begin >= 0) {
        QString valueString = expression.mid(begin, position - begin);
        qreal val = 0;
        if (valueString.startsWith("var(") && valueString.endsWith(")")) {
            // remove var( and last )
            QString variableString = valueString.mid(4, valueString.size()-5);
            if (!extractValue(variableString, val)) {
                expressionValues.clear();
                return false;
            }
        } else {
            if (!asReal(valueString, val)) {
                expressionValues.clear();
                return false;
            }
        }
        expressionValues.append(ExpressionValue(ExpressionValue::None, 0, val));
    }

    if(expressionValues.isEmpty()) {
        expressionValues.clear();
        return false;
    }
        
    // if last value is operator, fail
    if (expressionValues[expressionValues.size()-1].mToken != ExpressionValue::None) {
        expressionValues.clear();
        return false;
    }


    while (expressionValues.size() > 1) { // we have an answer when size = 1
        int maxPrecedence = -1;
        int calculateIndex = -1;
        for (int i = 0; i < expressionValues.size(); i++) {
            if ((expressionValues[i].mToken != ExpressionValue::None) &&
                (expressionValues[i].mPrecedence > maxPrecedence)) {
                maxPrecedence = expressionValues[i].mPrecedence;
                calculateIndex = i; // contains operator with highest precedence
            }
        }
        qreal answer = 0;

        if(calculateIndex < 0){
            return false;
        }

        switch (expressionValues[calculateIndex].mToken) {
            case ExpressionValue::Minus:
                answer = expressionValues[calculateIndex-1].mValue -
                         expressionValues[calculateIndex+1].mValue;
                break;
            case ExpressionValue::Plus:
                answer = expressionValues[calculateIndex-1].mValue +
                         expressionValues[calculateIndex+1].mValue;
                break;
            case ExpressionValue::Star:
                answer = expressionValues[calculateIndex-1].mValue *
                         expressionValues[calculateIndex+1].mValue;
                break;
            case ExpressionValue::Slash:
                if (expressionValues[calculateIndex+1].mValue == 0) {
                    expressionValues.clear();
                    return false;
                }
                answer = expressionValues[calculateIndex-1].mValue /
                         expressionValues[calculateIndex+1].mValue;
                break;
            default:
                break;
        }
        if (expressionValues[calculateIndex].mToken == ExpressionValue::UnaryMinus) {
            expressionValues[calculateIndex+1].mValue = -expressionValues[calculateIndex+1].mValue;
            expressionValues.removeAt(calculateIndex);
        } else {
            expressionValues[calculateIndex-1].mValue = answer;
            expressionValues.removeAt(calculateIndex+1);
            expressionValues.removeAt(calculateIndex);
        }
    }

    value = expressionValues[0].mValue;
    expressionValues.clear();

    return true;    
}


bool ValueExtractor::extractParameters( const QList<QString> &params, QList<QVariant> &values )
{
    if ( params.count() != values.count() ) {
        return false;
    }
    for ( int i = 0; i < declarations.count(); i++ ) {
        for( int j = 0; j < params.count(); j++ ) {
            if (declarations[i].property == params[j] ) {
                Value val = declarations[i].values.last();
                switch (val.type) {
                    case Value::Length:
                    case Value::Variable:
                    case Value::VariableNegative:
                    case Value::Expression:
                    case Value::ExpressionNegative:
                        values[j] = asReal(val);
                        break;
                    case Value::Percentage:
                        values[j] = val.variant.toDouble() / 100;
                        break;
                    case Value::KnownIdentifier:
                        values[j] = (QString)val.original;
                        break;
                    default:
                        values[j] = val.variant;
                        break;
                }
                break;
            }
        }
    }
    return true;
}

bool ValueExtractor::extractFont(QFont *font, HbFontSpec *fontSpec, int *fontSizeAdjustment)
{
    if (fontExtracted) {
        *font = f;
        *fontSizeAdjustment = adjustment;
        *fontSpec = fSpec;
        return fontExtracted == 1;
    }

    bool hit = false;
    bool tphSet = false;
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        if (decl.values.isEmpty())
            continue;
        const Value val = decl.values.first();
        switch (decl.propertyId) {
            case FontSize: setFontSizeFromValue(val, font, fontSizeAdjustment); break;
            case FontStyle: setFontStyleFromValue(val, font); break;
            case FontWeight: setFontWeightFromValue(val, font); break;
            case FontFamily: setFontFamilyFromValues(decl.values, font); break;
            case TextDecoration: setTextDecorationFromValues(decl.values, font); break;
            case Font: parseShorthandFontProperty(decl.values, font, fontSizeAdjustment); break;
            case FontVariant: setFontVariantFromValue(val, fontSpec, font); break;
            case TextTransform: setTextTransformFromValue(val, font); break;
            // Text-height alone is not enough to make 'hit' true.
            case HbFixedHeight: if (!tphSet) fontSpec->setTextHeight(asReal(decl)); continue;
            case HbTextHeight: tphSet = true; fontSpec->setTextHeight(asReal(decl)); continue;
            default: continue;
        }
        hit = true;
    }

    f = *font;
    fSpec = *fontSpec;
    adjustment = *fontSizeAdjustment;
    fontExtracted = hit ? 1 : 2;
    return hit;
}

bool ValueExtractor::extractPalette(QBrush *fg, QBrush *sfg, QBrush *sbg, QBrush *abg)
{
    bool hit = false;
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case Color: *fg = decl.brushValue(pal); break;
        case QtSelectionForeground: *sfg = decl.brushValue(pal); break;
        case QtSelectionBackground: *sbg = decl.brushValue(pal); break;
        case QtAlternateBackground: *abg = decl.brushValue(pal); break;
        default: continue;
        }
        hit = true;
    }
    return hit;
}

void ValueExtractor::extractFont()
{
    if (fontExtracted)
        return;
    int dummy = -255;
    // Values extracted into the object's own member variables.
    extractFont(&f, &fSpec, &dummy);
}

bool ValueExtractor::extractImage(QIcon *icon, Qt::Alignment *a, QSize *size)
{
    bool hit = false;
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        switch (decl.propertyId) {
        case QtImage:
            *icon = decl.iconValue();
            if (decl.values.count() > 0 && decl.values.at(0).type == Value::Uri) {
                // try to pull just the size from the image...
                QImageReader imageReader(decl.values.at(0).variant.toString());
                if ((*size = imageReader.size()).isNull()) {
                    // but we'll have to load the whole image if the
                    // format doesn't support just reading the size
                    *size = imageReader.read().size();
                }
            }
            break;
        case QtImageAlignment: *a = decl.alignmentValue();  break;
        default: continue;
        }
        hit = true;
    }
    return hit;
}

bool ValueExtractor::extractLayout(QString *layoutName, QString *sectionName)
{
    QString tempSectionName;
    if ( !layoutName || !sectionName ) {
        return false;
    }
    bool hit = false;
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        if ( decl.propertyId == HbLayout ) {
            if( decl.values.count() == 1 ) {
                *layoutName = decl.values.at(0).variant.toString();
                hit = true; 
            }
        }
        else if ( decl.propertyId == HbSection ) {
            if (decl.values.count() == 1 ) {
                tempSectionName = decl.values.at(0).variant.toString();
                //  a section without a layout doesn't count as a hit
            }
        }
    }
    if(hit)
        *sectionName = tempSectionName;
    return hit;
}


bool ValueExtractor::extractAspectRatioMode(Qt::AspectRatioMode *mode)
{
    bool hit = false;
    for (int i = 0; i < declarations.count(); ++i) {
        const Declaration &decl = declarations.at(i);
        if ( decl.propertyId == HbAspectRatio && decl.values.count() == 1 ) {
            switch (decl.values.at(0).variant.toInt())
            {
            case Value_Ignore:
                *mode = Qt::IgnoreAspectRatio;
                break;
            case Value_Keep:
                *mode = Qt::KeepAspectRatio;
                break;
            case Value_KeepExpand:
                *mode = Qt::KeepAspectRatioByExpanding;
                break;
            default:
                continue;
            }
            hit = true;
        }
    }
    return hit;
}

bool ValueExtractor::extractColor( QColor *col ) const
{
    bool hit = false;
    const int declarationsCount = declarations.count();
    for ( int i = 0; i < declarationsCount; ++i ) {
        const Declaration &decl = declarations.at(i);
        switch(decl.propertyId) {
        case Color: 
            if( decl.values.at(0).type == Value::Variable ) {
                HbCss::Value value;
                hit = extractValue( decl.values.at(0).variant.toString (), value );
                if (hit) {
                    *col = value.variant.toColor();
                }
            }
            else {
                *col = decl.values.at(0).variant.toColor();
                hit = true;
            }
        default:
            break;           
        }
    }
    return hit;
}


QColor Declaration::colorValue(const QPalette &pal) const
{
    if (values.count() != 1)
        return QColor();

    return parseColorValue(values.first(), pal);
}

QBrush Declaration::brushValue(const QPalette &pal) const
{
    if (values.count() != 1)
        return QBrush();

    return parseBrushValue(values.first(), pal);
}

void Declaration::brushValues(QBrush *c, const QPalette &pal) const
{
    int i;
    for (i = 0; i < qMin(values.count(), 4); i++)
        c[i] = parseBrushValue(values.at(i), pal);
    if (i == 0) c[0] = c[1] = c[2] = c[3] = QBrush();
    else if (i == 1) c[3] = c[2] = c[1] = c[0];
    else if (i == 2) c[2] = c[0], c[3] = c[1];
    else if (i == 3) c[3] = c[1];
}

bool Declaration::realValue(qreal *real, const char *unit) const
{
    if (values.count() != 1)
        return false;
    const Value &v = values.first();
    if (unit && v.type != Value::Length)
        return false;
    QString s = v.variant.toString();
    if (unit) {
        if (!s.endsWith(QLatin1String(unit), Qt::CaseInsensitive))
            return false;
        s.chop(qstrlen(unit));
    }
    bool ok = false;
    qreal val = s.toDouble(&ok);
    if (ok)
        *real = val;
    return ok;
}

static bool intValueHelper(const Value &v, int *i, const char *unit)
{
    if (unit && v.type != Value::Length)
        return false;
    QString s = v.variant.toString();
    if (unit) {
        if (!s.endsWith(QLatin1String(unit), Qt::CaseInsensitive))
            return false;
        s.chop(qstrlen(unit));
    }
    bool ok = false;
    int val = s.toInt(&ok);
    if (ok)
        *i = val;
    return ok;
}

bool Declaration::intValue(int *i, const char *unit) const
{
    if (values.count() != 1)
        return false;
    return intValueHelper(values.first(), i, unit);
}

QSize Declaration::sizeValue() const
{
    int x[2] = { 0, 0 };
    if (values.count() > 0)
        intValueHelper(values.at(0), &x[0], "px");
    if (values.count() > 1)
        intValueHelper(values.at(1), &x[1], "px");
    else
        x[1] = x[0];
    return QSize(x[0], x[1]);
}

QRect Declaration::rectValue() const
{
    if (values.count() != 1)
        return QRect();
    const Value &v = values.first();
    if (v.type != Value::Function)
        return QRect();
    QStringList func = v.variant.toStringList();
    if (func.count() != 2 || func.first().compare(QLatin1String("rect")) != 0)
        return QRect();
    QStringList args = func[1].split(QLatin1String(" "), QString::SkipEmptyParts);
    if (args.count() != 4)
        return QRect();
    return QRect(args[0].toInt(), args[1].toInt(), args[2].toInt(), args[3].toInt());
}

void Declaration::colorValues(QColor *c, const QPalette &pal) const
{
    int i;
    for (i = 0; i < qMin(values.count(), 4); i++)
        c[i] = parseColorValue(values.at(i), pal);
    if (i == 0) c[0] = c[1] = c[2] = c[3] = QColor();
    else if (i == 1) c[3] = c[2] = c[1] = c[0];
    else if (i == 2) c[2] = c[0], c[3] = c[1];
    else if (i == 3) c[3] = c[1];
}

BorderStyle Declaration::styleValue() const
{
    if (values.count() != 1)
        return BorderStyle_None;
    return parseStyleValue(values.first());
}

void Declaration::styleValues(BorderStyle *s) const
{
    int i;
    for (i = 0; i < qMin(values.count(), 4); i++)
        s[i] = parseStyleValue(values.at(i));
    if (i == 0) s[0] = s[1] = s[2] = s[3] = BorderStyle_None;
    else if (i == 1) s[3] = s[2] = s[1] = s[0];
    else if (i == 2) s[2] = s[0], s[3] = s[1];
    else if (i == 3) s[3] = s[1];
}

Repeat Declaration::repeatValue() const
{
    if (values.count() != 1)
        return Repeat_Unknown;
    return static_cast<Repeat>(findKnownValue(values.first().variant.toString(),
                                repeats, NumKnownRepeats));
}

Origin Declaration::originValue() const
{
    if (values.count() != 1)
        return Origin_Unknown;
    return static_cast<Origin>(findKnownValue(values.first().variant.toString(),
                               origins, NumKnownOrigins));
}

PositionMode Declaration::positionValue() const
{
    if (values.count() != 1)
        return PositionMode_Unknown;
    return static_cast<PositionMode>(findKnownValue(values.first().variant.toString(),
                                     positions, NumKnownPositionModes));
}

Attachment Declaration::attachmentValue() const
{
    if (values.count() != 1)
        return Attachment_Unknown;
    return static_cast<Attachment>(findKnownValue(values.first().variant.toString(),
                                   attachments, NumKnownAttachments));
}

int Declaration::styleFeaturesValue() const
{
    int features = StyleFeature_None;
    for (int i = 0; i < values.count(); i++) {
        features |= static_cast<int>(findKnownValue(values.value(i).variant.toString(),
                                     styleFeatures, NumKnownStyleFeatures));
    }
    return features;
}

QString Declaration::uriValue() const
{
    if (values.isEmpty() || values.first().type != Value::Uri)
        return QString();
    return values.first().variant.toString();
}

Qt::Alignment Declaration::alignmentValue() const
{
    if (values.isEmpty() || values.count() > 2)
        return Qt::AlignLeft | Qt::AlignTop;

    return parseAlignment(values.constData(), values.count());
}

Hb::TextWrapping Declaration::wrapModeValue() const
{
    if (values.isEmpty() || values.count() > 1)
        return Hb::TextNoWrap;

    return parseWrapMode(values.at(0));
}

void Declaration::borderImageValue(QString *image, int *cuts,
                                   TileMode *h, TileMode *v) const
{
    *image = uriValue();
    for (int i = 0; i < 4; i++)
        cuts[i] = -1;
    *h = *v = TileMode_Stretch;

    if (values.count() < 2)
        return;

    if (values.at(1).type == Value::Number) { // cuts!
        int i;
        for (i = 0; i < qMin(values.count()-1, 4); i++) {
            const Value& v = values.at(i+1);
            if (v.type != Value::Number)
                break;
            cuts[i] = v.variant.toString().toInt();
        }
        if (i == 0) cuts[0] = cuts[1] = cuts[2] = cuts[3] = 0;
        else if (i == 1) cuts[3] = cuts[2] = cuts[1] = cuts[0];
        else if (i == 2) cuts[2] = cuts[0], cuts[3] = cuts[1];
        else if (i == 3) cuts[3] = cuts[1];
    }

    if (values.last().type == Value::Identifier) {
        *v = static_cast<TileMode>(findKnownValue(values.last().variant.toString(),
                                      tileModes, NumKnownTileModes));
    }
    if (values[values.count() - 2].type == Value::Identifier) {
        *h = static_cast<TileMode>
                        (findKnownValue(values[values.count()-2].variant.toString(),
                                        tileModes, NumKnownTileModes));
    } else
        *h = *v;
}

QIcon Declaration::iconValue() const
{
    QIcon icon;
    for (int i = 0; i < values.count();) {
        Value value = values.at(i++);
        if (value.type != Value::Uri)
            break;
        QString uri = value.variant.toString();
        QIcon::Mode mode = QIcon::Normal;
        QIcon::State state = QIcon::Off;
        for (int j = 0; j < 2; j++) {
            if (i != values.count() && values.at(i).type == Value::KnownIdentifier) {
                switch (values.at(i).variant.toInt()) {
                case Value_Disabled: mode = QIcon::Disabled; break;
                case Value_Active: mode = QIcon::Active; break;
                case Value_Selected: mode = QIcon::Selected; break;
                case Value_Normal: mode = QIcon::Normal; break;
                case Value_On: state = QIcon::On; break;
                case Value_Off: state = QIcon::Off; break;
                default: break;
                }
                ++i;
            } else {
                break;
            }
        }

        // QIcon is soo broken
        if (icon.isNull())
            icon = QIcon(uri);
        else
            icon.addPixmap(uri, mode, state);

        if (i == values.count())
            break;

        if (values.at(i).type == Value::TermOperatorComma)
            i++;
    }
    return icon;
}

///////////////////////////////////////////////////////////////////////////////
// Selector
int Selector::specificity() const
{
    int val = 0;
    for (int i = 0; i < basicSelectors.count(); ++i) {
        const BasicSelector &sel = basicSelectors.at(i);
        if (!sel.elementName.isEmpty())
            val += 1;

        val += (sel.pseudos.count() + sel.attributeSelectors.count()) * 0x10;
        val += sel.ids.count() * 0x100;
    }
    return val;
}

QString Selector::pseudoElement() const
{
    const BasicSelector& bs = basicSelectors.last();
    if (!bs.pseudos.isEmpty() && bs.pseudos.first().type == PseudoClass_Unknown)
        return bs.pseudos.first().name;
    return QString();
}

quint64 Selector::pseudoClass(quint64 *negated) const
{
    const BasicSelector& bs = basicSelectors.last();
    if (bs.pseudos.isEmpty())
        return PseudoClass_Unspecified;
    quint64 pc = PseudoClass_Unknown;
    for (int i = !pseudoElement().isEmpty(); i < bs.pseudos.count(); i++) {
        const Pseudo &pseudo = bs.pseudos.at(i);
        if (pseudo.type == PseudoClass_Unknown)
            return PseudoClass_Unknown;
        if (!pseudo.negated)
            pc |= pseudo.type;
        else if (negated)
            *negated |= pseudo.type;
    }
    return pc;
}

///////////////////////////////////////////////////////////////////////////////
// StyleSelector
StyleSelector::StyleSelector()
{
}

StyleSelector::StyleSelector(const StyleSelector &copy)
{
    medium = copy.medium;
    QVectorIterator<StyleSheet*> iter(copy.styleSheets);
    while(iter.hasNext()){
        StyleSheet *newSheet = HbMemoryUtils::create<HbCss::StyleSheet>(*(iter.next()),
                                                                    HbMemoryManager::HeapMemory);
        addStyleSheet(newSheet);
    }
}

StyleSelector::~StyleSelector()
{
    for(int i=0; i<styleSheets.count(); i++){
        HbMemoryUtils::release<HbCss::StyleSheet>(styleSheets.at(i));
    }
    styleSheets.clear();
    widgetSheets.clear();
}

int StyleSelector::selectorMatches(const Selector &selector, NodePtr node, bool nameCheckNeeded) const
{
    if (selector.basicSelectors.isEmpty()) {
        return -1;
    }

    if (selector.basicSelectors.first().relationToNext == BasicSelector::NoRelation) {
        if (selector.basicSelectors.count() != 1) {
            return -1;
        }
        return basicSelectorMatches(selector.basicSelectors.first(), node, nameCheckNeeded);
    }

    if (selector.basicSelectors.count() <= 1) {
        return -1;
    }

    int i = selector.basicSelectors.count() - 1;
    int matchLevel(-1);
    int firstMatchLevel(-1);

    BasicSelector sel = selector.basicSelectors.at(i);
    bool firstLoop = true;
    do {
        matchLevel = basicSelectorMatches(sel, node, (nameCheckNeeded || !firstLoop));
        if (firstLoop) {
            firstMatchLevel = matchLevel;
        }
        if (matchLevel < 0) {
            if (sel.relationToNext == BasicSelector::MatchNextSelectorIfParent
                || i == selector.basicSelectors.count() - 1) { // first element must always match!
                break;
            }
        }

        if (matchLevel >= 0 || sel.relationToNext != BasicSelector::MatchNextSelectorIfAncestor)
            --i;

        if (i < 0) {
            break;
        }

        sel = selector.basicSelectors.at(i);
        if (sel.relationToNext == BasicSelector::MatchNextSelectorIfAncestor
            || sel.relationToNext == BasicSelector::MatchNextSelectorIfParent) {
            node = parentNode(node);
        } else if (sel.relationToNext == BasicSelector::MatchNextSelectorIfPreceeds) {
            node = previousSiblingNode(node);
        }
        if (isNullNode(node)) {
            matchLevel = -1;
            break;
        }
        firstLoop = false;
   } while (i >= 0 && (matchLevel >= 0 || sel.relationToNext == BasicSelector::MatchNextSelectorIfAncestor));

    return (matchLevel < 0) ? -1 : firstMatchLevel;
}

int StyleSelector::inheritanceDepth(NodePtr node, HbString &elementName) const
{
    if (elementName == GLOBAL_CSS_SELECTOR)
        return 0;

    static QHash<QString, int> depths;
    if (depths.contains(elementName)) {
        return depths[elementName];
    } else {
        int result = nodeNameEquals(node, elementName);
        depths[elementName] = result;
        return result;
    }
}

int StyleSelector::basicSelectorMatches(const BasicSelector &sel, NodePtr node, bool nameCheckNeeded) const
{
    int matchLevel = 0;
    HbString elementName(HbMemoryManager::HeapMemory);

    if (!sel.attributeSelectors.isEmpty()) {
        if (!hasAttributes(node))
            return -1;

        for (int i = 0; i < sel.attributeSelectors.count(); ++i) {
            const AttributeSelector &a = sel.attributeSelectors.at(i);
            if (a.name == QLatin1String("class")) {
                elementName = a.value;
            }
            if (!attributeMatches(node, a)) {
                return -1;
            }
        }
    }
    if ( elementName.isEmpty() ) {
        elementName = sel.elementName;
    }

    if (!elementName.isEmpty()) {
        matchLevel = nameCheckNeeded 
            ? nodeNameEquals(node, elementName)
            : inheritanceDepth(node, elementName);
        if ( matchLevel < 0 ) {
            return -1;
        }
    }

    if (!sel.ids.isEmpty() && sel.ids != nodeIds(node)) {
        return -1;
    }

    return matchLevel;
}

static inline bool qcss_selectorStyleRuleLessThan(const QPair<int, HbCss::StyleRule> &lhs, const QPair<int, HbCss::StyleRule> &rhs)
{
    return lhs.first < rhs.first;
}

static inline bool qcss_selectorDeclarationLessThan(const QPair<int, HbCss::Declaration> &lhs, const QPair<int, HbCss::Declaration> &rhs)
{
    return lhs.first < rhs.first;
}

void StyleSelector::matchRules(NodePtr node, const HbVector<StyleRule> &rules, StyleSheetOrigin origin,
                               int depth, QVector<WeightedRule> *weightedRules, bool nameCheckNeeded) const
{
    for (int i = 0; i < rules.count(); ++i) {
        const StyleRule &rule = rules.at(i);
        for (int j = 0; j < rule.selectors.count(); ++j) {
            const Selector& selector = rule.selectors.at(j);
            int matchLevel = selectorMatches(selector, node, nameCheckNeeded);
            if ( matchLevel >= 0 ) {
                WeightedRule wRule;
                wRule.first = selector.specificity()
                    + 0x1000* matchLevel
                    + (origin == StyleSheetOrigin_Inline)*0x10000*depth;
                wRule.second.selectors.append(selector);
                wRule.second.declarations = rule.declarations;
                weightedRules->append(wRule);
            }
        }
    }
}

// Returns style rules that are in ascending order of specificity
// Each of the StyleRule returned will contain exactly one Selector
HbVector<StyleRule> StyleSelector::styleRulesForNode(NodePtr node, const Qt::Orientation orientation) const
{
    HbVector<StyleRule> rules;
    if (styleSheets.isEmpty())
        return rules;

    QVector<WeightedRule> weightedRules = weightedStyleRulesForNode(node, orientation);

    qStableSort(weightedRules.begin(), weightedRules.end(), qcss_selectorStyleRuleLessThan);

    for (int j = 0; j < weightedRules.count(); j++)
        rules += weightedRules.at(j).second;

    return rules;
}


// Returns style rules and specificity values (unordered)
QVector<WeightedRule> StyleSelector::weightedStyleRulesForNode(NodePtr node, const Qt::Orientation orientation) const
{
    initNode(node);
    QVector<WeightedRule> weightedRules; // (spec, rule) that will be sorted below

    // Generate inheritance list (reverse order)
    QStringList classNames;
    QGraphicsWidget *widgetPtr = static_cast<QGraphicsWidget*> (node.ptr);
    const QMetaObject *metaObject = widgetPtr->metaObject();
    do {
        const QString className = metaObject->className();
        classNames << className;
        metaObject = metaObject->superClass();
    } while (metaObject != 0);
    classNames << GLOBAL_CSS_SELECTOR;

    // Iterate backwards through list to append most-derived classes last
    int count = classNames.count();
    bool firstLoop = true;
    while(count--){
        const QString &className = classNames.at(count);
        QVectorIterator<StyleSheet*> iter(widgetSheets[className]);
        while (iter.hasNext()) {
            const StyleSheet *styleSheet = iter.next();
            if(styleSheet) {
                WidgetStyleRules* widgetStack = styleSheet->widgetStack(className);
                if (widgetStack) {
                    matchRules(node, widgetStack->styleRules, styleSheet->origin, styleSheet->depth, &weightedRules, false);
                    // Append orientation-specific rules
                    if (orientation == Qt::Vertical) {
                        matchRules(node, widgetStack->portraitRules, styleSheet->origin, styleSheet->depth, &weightedRules, false);
                    }else if (orientation == Qt::Horizontal) {
                        matchRules(node, widgetStack->landscapeRules, styleSheet->origin, styleSheet->depth, &weightedRules, false);
                    }
                }
                if (firstLoop && !medium.isEmpty()) { // Media rules are only added to global widget stack
                    int mediaRuleCount = styleSheet->mediaRules.count();
                    for (int i = 0; i < mediaRuleCount; ++i) {
                        if (styleSheet->mediaRules.at(i).media.contains(
                                HbString(medium, HbMemoryManager::HeapMemory),
                                Qt::CaseInsensitive)) {
                            matchRules(node, styleSheet->mediaRules.at(i).styleRules, styleSheet->origin,
                                    styleSheet->depth, &weightedRules);
                        }
                    }
                }// End medium.isEmpty loop
            }// End styleSheet
        }
        firstLoop = false;
    }
    cleanupNode(node);
    return weightedRules;
}

bool StyleSelector::hasOrientationSpecificStyleRules(NodePtr node) const
{
    // Generate inheritance list (reverse order)
    QStringList classNames;
    QGraphicsWidget *widgetPtr = static_cast<QGraphicsWidget*> (node.ptr);
    const QMetaObject *metaObject = widgetPtr->metaObject();
    do {
        const QString className = metaObject->className();
        classNames << className;
        metaObject = metaObject->superClass();
    } while (metaObject != 0);
    classNames << GLOBAL_CSS_SELECTOR;

    int count = classNames.count();
    while (count--) {
        const QString &className = classNames.at(count);
        QVectorIterator<StyleSheet*> iter(widgetSheets[className]);
        while (iter.hasNext()) {
            const StyleSheet *styleSheet = iter.next();
            if (styleSheet) {
                WidgetStyleRules* widgetStack = styleSheet->widgetStack(className);
                if (widgetStack) {
                    if (widgetStack->portraitRules.count() ||
                            widgetStack->landscapeRules.count()) {
                        return true;
                    }
                }
            }// End styleSheet
        }
    }
    return false;
}



// Returns declarations and specificity values (unordered)
QVector<WeightedDeclaration> StyleSelector::weightedDeclarationsForNode(NodePtr node, const Qt::Orientation orientation,
        const char *extraPseudo) const
{
    QVector<WeightedDeclaration> decls;
    QVector<WeightedRule> rules = weightedStyleRulesForNode(node, orientation);
    for (int i = 0; i < rules.count(); i++) {
        const Selector& selector = rules.at(i).second.selectors.at(0);
        const QString pseudoElement = selector.pseudoElement();

        bool pseudoElementMatches = (extraPseudo && pseudoElement == QLatin1String(extraPseudo));

        // skip rules with non-matching pseudo elements
        if (!pseudoElement.isEmpty() && !pseudoElementMatches)
            continue;

        quint64 pseudoClass = selector.pseudoClass();
        bool pseudoClassIsValid = 
            pseudoClass == PseudoClass_Enabled 
            || pseudoClass == PseudoClass_Unspecified
            || pseudoClass == PseudoClass_Landscape 
            || pseudoClass == PseudoClass_Portrait;

        if (pseudoClassIsValid || pseudoElementMatches) {
            HbVector<Declaration> ruleDecls = rules.at(i).second.declarations;
            for (int j=0; j<ruleDecls.count(); j++) {
                WeightedDeclaration wDecl;
                wDecl.first = rules.at(i).first;
                wDecl.second = ruleDecls.at(j);
                decls.append(wDecl);
            }
        }
    }
    return decls;
}

// for qtexthtmlparser which requires just the declarations with Enabled state
// and without pseudo elements
HbVector<Declaration> StyleSelector::declarationsForNode(NodePtr node, const Qt::Orientation orientation,
    const char *extraPseudo) const
{
    HbVector<Declaration> decls;
    if (styleSheets.isEmpty())
        return decls;

    QVector<WeightedDeclaration> weightedDecls = weightedDeclarationsForNode(node, orientation, extraPseudo);

    qStableSort(weightedDecls.begin(), weightedDecls.end(), qcss_selectorDeclarationLessThan);

    for (int j = 0; j < weightedDecls.count(); j++)
        decls += weightedDecls.at(j).second;

    return decls;
}



HbVector<Declaration> StyleSelector::variableRuleSets() const
{
 HbVector<Declaration> decls;
    if ( styleSheets.count() != 0) {
        for (int i=0;i<styleSheets.count();i++) {
            const StyleSheet *styleSheet = styleSheets.at(i);
            //check if any variable rule exists or not
            if (styleSheet->variableRules.count() != 0) {
                for (int j=0;j<styleSheet->variableRules.count();j++) {
                    decls +=styleSheet->variableRules[j].declarations;
                }
            }
        }
    }
    return decls;
}

void StyleSelector::addStyleSheet( StyleSheet* styleSheet )
{
    styleSheets.append(styleSheet);
    foreach (const HbCss::WidgetStyleRules &wsr, styleSheet->widgetRules) {
        widgetSheets[wsr.widgetName].append(styleSheet);
    }
}

void StyleSelector::removeStyleSheet( StyleSheet* styleSheet )
{
    styleSheets.remove(styleSheets.indexOf(styleSheet));
    QHash<QString, QVector<HbCss::StyleSheet*> >::iterator iter = widgetSheets.begin();
    while (iter != widgetSheets.end()) {
        int index = iter.value().indexOf(styleSheet);
        if (index != -1) {
            iter.value().remove(index);
        }        
        ++iter;
    }
    HbMemoryUtils::release<HbCss::StyleSheet>(styleSheet);
}



static inline bool isHexDigit(const char c)
{
    return (c >= '0' && c <= '9')
           || (c >= 'a' && c <= 'f')
           || (c >= 'A' && c <= 'F')
           ;
}

QString Scanner::preprocess(const QString &input, bool *hasEscapeSequences)
{
    QString output = input;

    if (hasEscapeSequences)
        *hasEscapeSequences = false;

    int i = 0;
    while (i < output.size()) {
        if (output.at(i) == QLatin1Char('\\')) {

            ++i;
            // test for unicode hex escape
            int hexCount = 0;
            const int hexStart = i;
            while (i < output.size()
                   && isHexDigit(output.at(i).toLatin1())
                   && hexCount < 7) {
                ++hexCount;
                ++i;
            }
            if (hexCount == 0) {
                if (hasEscapeSequences)
                    *hasEscapeSequences = true;
                continue;
            }

            hexCount = qMin(hexCount, 6);
            bool ok = false;
            ushort code = output.mid(hexStart, hexCount).toUShort(&ok, 16);
            if (ok) {
                output.replace(hexStart - 1, hexCount + 1, QChar(code));
                i = hexStart;
            } else {
                i = hexStart;
            }
        } else {
            ++i;
        }
    }
    return output;
}

int QCssScanner_Generated::handleCommentStart()
{
    while (pos < input.size() - 1) {
        if (input.at(pos) == QLatin1Char('*')
            && input.at(pos + 1) == QLatin1Char('/')) {
            pos += 2;
            break;
        }
        ++pos;
    }
    return S;
}

void Scanner::scan(const QString &preprocessedInput, QVector<Symbol> *symbols)
{
    QCssScanner_Generated scanner(preprocessedInput);
    Symbol sym;
    int tok = scanner.lex();
    while (tok != -1) {
        sym.token = static_cast<HbCss::TokenType>(tok);
        sym.text = scanner.input;
        sym.start = scanner.lexemStart;
        sym.len = scanner.lexemLength;
        symbols->append(sym);
        tok = scanner.lex();
    }
}

QString Symbol::lexem() const
{
    QString result;
    if (len > 0)
        result.reserve(len);
    for (int i = 0; i < len; ++i) {
        if (text.at(start + i) == QLatin1Char('\\') && i < len - 1)
            ++i;
        result += text.at(start + i);
    }
    return result;
}

Parser::Parser(const QString &css, bool isFile)
{
    init(css, isFile);
}

Parser::Parser()
{
    index = 0;
    errorIndex = -1;
    hasEscapeSequences = false;
}

void Parser::init(const QString &css, bool isFile)
{
    QString styleSheet = css;
    if (isFile) {
        QFile file(css);
        if (file.open(QFile::ReadOnly)) {
            sourcePath = QFileInfo(styleSheet).absolutePath() + QLatin1String("/");
            QTextStream stream(&file);
            styleSheet = stream.readAll();
        } else {
            qWarning() << "HbCss::Parser - Failed to load file " << css;
            styleSheet.clear();
        }
    } else {
        sourcePath.clear();
    }

    hasEscapeSequences = false;
    symbols.resize(0);
    Scanner::scan(Scanner::preprocess(styleSheet, &hasEscapeSequences), &symbols);
    index = 0;
    errorIndex = -1;
    errorCode = NoError;
    symbols.reserve(qMax(symbols.capacity(), symbols.size()));
}

bool Parser::parse(StyleSheet *styleSheet)
{
    errorCode = Parser::UnknownError;
    try {
        if (testTokenAndEndsWith(ATKEYWORD_SYM, QLatin1String("charset"))) {
            if (!next(STRING)) return false;
            if (!next(SEMICOLON)) return false;
        }

        while (test(S) || test(CDO) || test(CDC)) {}

        while (testImport()) {
            ImportRule rule(styleSheet->memoryType);
            if (!parseImport(&rule)) return false;
#ifdef CSS_PARSER_TRACES
            rule.print();
#endif
            styleSheet->importRules.append(rule);
            while (test(S) || test(CDO) || test(CDC)) {}
        }

        do {
            //newly added for variable support
            if (testVariable()) {
                VariableRule rule(styleSheet->memoryType);
                if (!parseVariableset(&rule)) return false;
#ifdef CSS_PARSER_TRACES
                rule.print();
#endif
                styleSheet->variableRules.append(rule);
            }else if (testMedia()) {
                MediaRule rule(styleSheet->memoryType);
                if (!parseMedia(&rule)) return false;
                styleSheet->mediaRules.append(rule);
            } else if (testPage()) {
                PageRule rule(styleSheet->memoryType);
                if (!parsePage(&rule)) return false;
                styleSheet->pageRules.append(rule);
            } else if (testRuleset()) {
                StyleRule rule(styleSheet->memoryType);
                if (!parseRuleset(&rule)) return false;
#ifdef CSS_PARSER_TRACES
                rule.print();
#endif
                if(rule.selectors.count() > 1){
                    foreach(const HbCss::Selector &selector, rule.selectors){
                        QString stackName = selector.basicSelectors.last().elementName;
                        if(stackName.length() < 1){
                            stackName = GLOBAL_CSS_SELECTOR;
                        }
                        StyleRule newRule(rule.memoryType);
                        newRule.declarations = rule.declarations;
                        newRule.selectors.append(selector);
                        addRuleToWidgetStack(styleSheet, stackName, newRule);
                    }
                } else {
                    QString stackName = rule.selectors.at(0).basicSelectors.last().elementName;
                    if(stackName.length() < 1){
                        stackName = GLOBAL_CSS_SELECTOR;
                    }
                    addRuleToWidgetStack(styleSheet, stackName, rule);
                }
            } else if (test(ATKEYWORD_SYM)) {
                if (!until(RBRACE)) return false;
            } else if (hasNext()) {
                return false;
            }
            while (test(S) || test(CDO) || test(CDC)) {}
        } while (hasNext());

    } catch(std::bad_alloc &badAlloc) {
        //an exception happened, probably due to OOM
        Q_UNUSED(badAlloc)
        errorCode = OutOfMemoryError;
        return false;
    }

    errorCode = NoError;
    return true;
}

void Parser::addRuleToWidgetStack(StyleSheet *sheet, const QString &stackName, StyleRule &rule)
{
    WidgetStyleRules* widgetStack = sheet->widgetStack(stackName);

    if (!widgetStack) {
#ifdef CSSSTACKS_DEBUG
        qDebug() << "Creating stack for classname" << stackName;
#endif
        HbCss::WidgetStyleRules rules(stackName, sheet->memoryType);
        widgetStack = sheet->addWidgetStack(rules);
    }

    // Add rule into correct (portrait/landscape/any) list
    quint64 negated = 0;
    quint64 pseudo = rule.selectors.last().pseudoClass(&negated);
  
    if (((pseudo & HbCss::PseudoClass_Portrait) && ((negated & HbCss::PseudoClass_Portrait) == 0))
            || (negated & HbCss::PseudoClass_Landscape)) {
        widgetStack->portraitRules.append(rule);
    } else if (((pseudo & HbCss::PseudoClass_Landscape) && ((negated & HbCss::PseudoClass_Landscape) == 0))
            || (negated & HbCss::PseudoClass_Portrait)) {
        widgetStack->landscapeRules.append(rule);
    } else {
        widgetStack->styleRules.append(rule);
    }
}

Symbol Parser::errorSymbol()
{
    if (errorIndex == -1) return Symbol();
    return symbols.at(errorIndex);
}

static inline void removeOptionalQuotes(HbString *str)
{
    if (!str->startsWith(QLatin1Char('\''))
        && !str->startsWith(QLatin1Char('\"')))
        return;
    str->remove(0, 1);
    str->chop(1);
}

bool Parser::parseImport(ImportRule *importRule)
{
    skipSpace();

    if (test(STRING)) {
        importRule->href = lexem();
    } else {
        if (!testAndParseUri(&importRule->href)) return false;
    }
    removeOptionalQuotes(&importRule->href);

    skipSpace();

    if (testMedium()) {
        if (!parseMedium(&importRule->media)) return false;

        while (test(COMMA)) {
            skipSpace();
            if (!parseNextMedium(&importRule->media)) return false;
        }
    }

    if (!next(SEMICOLON)) return false;

    skipSpace();
    return true;
}

bool Parser::parseMedia(MediaRule *mediaRule)
{
    do {
        skipSpace();
        if (!parseNextMedium(&mediaRule->media)) return false;
    } while (test(COMMA));

    if (!next(LBRACE)) return false;
    skipSpace();

    while (testRuleset()) {
        StyleRule rule(mediaRule->memoryType);
        if (!parseRuleset(&rule)) return false;
#ifdef CSS_PARSER_TRACES        
        rule.print();
#endif
        mediaRule->styleRules.append(rule);
    }

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;
}

bool Parser::parseMedium(HbStringVector *media)
{
    media->append(HbString(lexem(), media->memoryType()));
    skipSpace();
    return true;
}

bool Parser::parsePage(PageRule *pageRule)
{
    skipSpace();

    if (testPseudoPage())
        if (!parsePseudoPage(&pageRule->selector)) return false;

    skipSpace();
    if (!next(LBRACE)) return false;

    do {
        skipSpace();
        Declaration decl(pageRule->memoryType);
        if (!parseNextDeclaration(&decl)) return false;
        if (!decl.isEmpty())
            pageRule->declarations.append(decl);
    } while (test(SEMICOLON));

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;
}

bool Parser::parsePseudoPage(HbString *selector)
{
    if (!next(IDENT)) return false;
    *selector = lexem();
    return true;
}

bool Parser::parseNextOperator(Value *value)
{
    if (!hasNext()) return true;
    switch (next()) {
        case SLASH: value->type = Value::TermOperatorSlash; skipSpace(); break;
        case COMMA: value->type = Value::TermOperatorComma; skipSpace(); break;
        default: prev(); break;
    }
    return true;
}

bool Parser::parseCombinator(BasicSelector::Relation *relation)
{
    *relation = BasicSelector::NoRelation;
    if (lookup() == S) {
        *relation = BasicSelector::MatchNextSelectorIfAncestor;
        skipSpace();
    } else {
        prev();
    }
    if (test(PLUS)) {
        *relation = BasicSelector::MatchNextSelectorIfPreceeds;
    } else if (test(GREATER)) {
        *relation = BasicSelector::MatchNextSelectorIfParent;
    }
    skipSpace();
    return true;
}

bool Parser::parseProperty(Declaration *decl)
{
    decl->property = lexem();
    decl->propertyId = static_cast<Property>(findKnownValue(decl->property, properties, NumProperties));
    skipSpace();
    return true;
}




//new function added for varibale support
bool Parser::parseVariableset(VariableRule *variableRule)
{
//no selector needs to be identified
//declarations part

    skipSpace();
    if (!next(LBRACE)) return false;
    const int declarationStart = index;

    do {
        skipSpace();
        Declaration decl(variableRule->memoryType);
        const int rewind = index;
        if (!parseNextDeclaration(&decl)) {
            index = rewind;
            const bool foundSemicolon = until(SEMICOLON);
            const int semicolonIndex = index;

            index = declarationStart;
            const bool foundRBrace = until(RBRACE);

            if (foundSemicolon && semicolonIndex < index) {
                decl = Declaration(variableRule->memoryType);
                index = semicolonIndex - 1;
            } else {
                skipSpace();
                return foundRBrace;
            }
        }
        if (!decl.isEmpty())
        {
#ifdef CSS_PARSER_TRACES
            decl.print();
#endif
            variableRule->declarations.append(decl);
        }

    } while (test(SEMICOLON));

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;

}



bool Parser::parseRuleset(StyleRule *styleRule)
{
    Selector sel(styleRule->memoryType);
    if (!parseSelector(&sel)) return false;
#ifdef CSS_PARSER_TRACES    
    sel.print();
#endif
    styleRule->selectors.append(sel);

    while (test(COMMA)) {
        skipSpace();
        Selector sel(styleRule->memoryType);
        if (!parseNextSelector(&sel)) return false;
#ifdef CSS_PARSER_TRACES
        sel.print();
#endif
        styleRule->selectors.append(sel);
    }

    skipSpace();
    if (!next(LBRACE)) return false;
    const int declarationStart = index;

    do {
        skipSpace();
        Declaration decl(styleRule->memoryType);
        const int rewind = index;
        if (!parseNextDeclaration(&decl)) {
            index = rewind;
            const bool foundSemicolon = until(SEMICOLON);
            const int semicolonIndex = index;

            index = declarationStart;
            const bool foundRBrace = until(RBRACE);

            if (foundSemicolon && semicolonIndex < index) {
                decl = Declaration(styleRule->memoryType);
                index = semicolonIndex - 1;
            } else {
                skipSpace();
                return foundRBrace;
            }
        }
        if (!decl.isEmpty())
        {    
#ifdef CSS_PARSER_TRACES
            decl.print();
#endif
            styleRule->declarations.append(decl);
        }
    } while (test(SEMICOLON));

    if (!next(RBRACE)) return false;
    skipSpace();
    return true;
}

bool Parser::parseSelector(Selector *sel)
{
    BasicSelector basicSel(sel->memoryType);
    if (!parseSimpleSelector(&basicSel)) return false;
    while (testCombinator()) {
        if (!parseCombinator(&basicSel.relationToNext)) return false;

        if (!testSimpleSelector()) break;
        sel->basicSelectors.append(basicSel);

        basicSel = BasicSelector(sel->memoryType);
        if (!parseSimpleSelector(&basicSel)) return false;
    }
#ifdef CSS_PARSER_TRACES
    basicSel.print();
#endif
    sel->basicSelectors.append(basicSel);
    return true;
}

bool Parser::parseSimpleSelector(BasicSelector *basicSel)
{
    int minCount = 0;
    if (lookupElementName()) {
        if (!parseElementName(&basicSel->elementName)) return false;
    } else {
        prev();
        minCount = 1;
    }
    bool onceMore;
    int count = 0;
    do {
        onceMore = false;
        if (test(HASH)) {
            QString id = lexem();
            // chop off leading #
            id.remove(0, 1);
            basicSel->ids.append(HbString(id, basicSel->memoryType));
            onceMore = true;
        } else if (testClass()) {
            onceMore = true;
            AttributeSelector a(basicSel->memoryType);
            a.name = QLatin1String("class");
            a.valueMatchCriterium = AttributeSelector::MatchContains;
            if (!parseClass(&a.value)) return false;
#ifdef CSS_PARSER_TRACES
            a.print();
#endif
            basicSel->attributeSelectors.append(a);
        } else if (testAttrib()) {
            onceMore = true;
            AttributeSelector a(basicSel->memoryType);
            if (!parseAttrib(&a)) return false;
#ifdef CSS_PARSER_TRACES
            a.print();
#endif
            basicSel->attributeSelectors.append(a);
        } else if (testPseudo()) {
            onceMore = true;
            Pseudo ps(basicSel->memoryType);
            if (!parsePseudo(&ps)) return false;
#ifdef CSS_PARSER_TRACES
            ps.print();
#endif
            basicSel->pseudos.append(ps);
        }
        if (onceMore) ++count;
    } while (onceMore);
    return count >= minCount;
}

bool Parser::parseClass(HbString *name)
{
    if (!next(IDENT)) return false;
    *name = lexem();
    return true;
}

bool Parser::parseElementName(HbString *name)
{
    switch (lookup()) {
        case STAR: name->clear(); break;
        case IDENT: *name = lexem(); break;
        default: return false;
    }
    return true;
}

bool Parser::parseAttrib(AttributeSelector *attr)
{
    skipSpace();
    attr->negated = false;
    if (test(EXCLAMATION_SYM)) {
        attr->negated = true;
    }

    if (!next(IDENT)) return false;
    attr->name = lexem();
    skipSpace();

    if (test(EXCLAMATION_SYM)) {
        attr->negated = !(attr->negated);
    }

    if (test(EQUAL)) {
        attr->valueMatchCriterium = AttributeSelector::MatchEqual;
    } else if (test(INCLUDES)) {
        attr->valueMatchCriterium = AttributeSelector::MatchContains;
    } else if (test(DASHMATCH)) {
        attr->valueMatchCriterium = AttributeSelector::MatchBeginsWith;
    } else {
        return next(RBRACKET);
    }

    skipSpace();

    if (!test(IDENT) && !test(STRING)) return false;
    attr->value = unquotedLexem();

    skipSpace();
    return next(RBRACKET);
}

bool Parser::parsePseudo(Pseudo *pseudo)
{
    test(COLON);
    pseudo->negated = test(EXCLAMATION_SYM);
    if (test(IDENT)) {
        pseudo->name = lexem();
        pseudo->type = static_cast<quint64>(findKnownValue((pseudo->name), pseudos, NumPseudos));
        return true;
    }
    if (!next(FUNCTION)) return false;
    pseudo->function = lexem();
    // chop off trailing parenthesis
    pseudo->function.chop(1);
    skipSpace();
    if (!test(IDENT)) return false;
    pseudo->name = lexem();
    skipSpace();
    return next(RPAREN);
}

bool Parser::parseNextDeclaration(Declaration *decl)
{
    if (!testProperty())
        return true; // not an error!
    if (!parseProperty(decl)) return false;
    if (!next(COLON)) return false;
    skipSpace();
    if (!parseNextExpr(&decl->values)) return false;
    if (testPrio())
        if (!parsePrio(decl)) return false;
    return true;
}

bool Parser::testPrio()
{
    const int rewind = index;
    if (!test(EXCLAMATION_SYM)) return false;
    skipSpace();
    if (!test(IDENT)) {
        index = rewind;
        return false;
    }
    if (lexem().compare(QLatin1String("important"), Qt::CaseInsensitive) != 0) {
        index = rewind;
        return false;
    }
    return true;
}

bool Parser::parsePrio(Declaration *declaration)
{
    declaration->important = true;
    skipSpace();
    return true;
}

bool Parser::parseExpr(HbVector<Value> *values)
{
    Value val(values->memoryType());
    if (!parseTerm(&val)) return false;
#ifdef CSS_PARSER_TRACES
    val.print();
#endif
    values->append(val);

    bool onceMore;
    do {
        onceMore = false;
        val = Value(values->memoryType());
        if (!parseNextOperator(&val)) return false;
        if (val.type != HbCss::Value::Unknown) {
#ifdef CSS_PARSER_TRACES
            val.print();
#endif
            values->append(val);
        }
        if (testTerm()) {
            onceMore = true;
            val = Value(values->memoryType());
            if (!parseTerm(&val)) return false;
#ifdef CSS_PARSER_TRACES
            val.print();
#endif
            values->append(val);
        }
    } while (onceMore);
    return true;
}

bool Parser::testTerm()
{
    return test(PLUS) || test(MINUS)
           || test(NUMBER)
           || test(PERCENTAGE)
           || test(LENGTH)
           || test(STRING)
           || test(IDENT)
           || testHexColor()
           || testFunction();
}

bool Parser::parseTerm(Value *value)
{
    QString str = lexem();
    bool haveUnary = false;
    if (lookup() == PLUS || lookup() == MINUS) {
        haveUnary = true;
        if (!hasNext()) return false;
        next();
        str += lexem();
    }

    value->variant = str;
    value->type = HbCss::Value::String;
    switch (lookup()) {
        case NUMBER:
            value->type = Value::Number;
            value->variant.convert(HbVariant::Double);
            break;
        case PERCENTAGE:
            value->type = Value::Percentage;
            str.chop(1); // strip off %
            value->variant = str;
            break;
        case LENGTH:
            value->type = Value::Length;
            break;

        case STRING:
            if (haveUnary) return false;
            value->type = Value::String;
            str.chop(1);
            str.remove(0, 1);
            value->variant = str;
            break;
        case IDENT: {
            if (haveUnary) return false;
            value->type = Value::Identifier;
            const int id = findKnownValue(str, values, NumKnownValues);
            if (id != 0) {
                value->type = Value::KnownIdentifier;
                value->variant = id;
                value->original = str;
            }
            break;
        }
        default: {
            if (haveUnary) return false;
            prev();
            if (testHexColor()) {
                QColor col;
                if (!parseHexColor(&col)) return false;
                value->type = Value::Color;
                value->variant = col;
            } else if (testFunction()) {
                HbString name(value->memoryType), args(value->memoryType);
                if (!parseFunction(&name, &args)) return false;
                if (name == QLatin1String("url")) {
                    value->type = Value::Uri;
                    removeOptionalQuotes(&args);
                    if (QFileInfo(args).isRelative() && !sourcePath.isEmpty()) {
                        args.prepend(sourcePath);
                    }
                    value->variant = args;
                }
                //changes for variable support
                else if (name == QLatin1String("var")) {
                    value->type = Value::Variable;
                    value->variant = args;
                } else if (name == QLatin1String("-var")) {                    
                    value->type = Value::VariableNegative;
                    value->variant = args;
                } //change end
                //changes for expression support
                else if (name == QLatin1String("expr")) {
                    value->type = Value::Expression;
                    value->variant = args;
                } else if (name == QLatin1String("-expr")) {
                    value->type = Value::ExpressionNegative;
                    value->variant = args;
                } //change end
                else {
                    value->type = Value::Function;
                    value->variant = QStringList() << name << args;
                }
            } else {
                return recordError();
            }
            return true;
        }
    }
    skipSpace();
    return true;
}

bool Parser::parseFunction(HbString *name, HbString *args)
{
    *name = lexem();
    name->chop(1);
    skipSpace();
    const int start = index;
    if (!until(RPAREN)) return false;
    for (int i = start; i < index - 1; ++i)
        args->append(symbols.at(i).lexem());
    /*
    if (!nextExpr(&arguments)) return false;
    if (!next(RPAREN)) return false;
    */
    skipSpace();
    return true;
}

bool Parser::parseHexColor(QColor *col)
{
    col->setNamedColor(lexem());
    if (!col->isValid()) return false;
    skipSpace();
    return true;
}

bool Parser::testAndParseUri(HbString *uri)
{
    const int rewind = index;
    if (!testFunction()) return false;

    HbString name(uri->memoryType()), args(uri->memoryType());
    if (!parseFunction(&name, &args)) {
        index = rewind;
        return false;
    }
    if (name.toLower() != QLatin1String("url")) {
        index = rewind;
        return false;
    }
    *uri = args;
    removeOptionalQuotes(uri);
    return true;
}

bool Parser::testSimpleSelector()
{
    return testElementName()
           || (test(HASH))
           || testClass()
           || testAttrib()
           || testPseudo();
}

bool Parser::next(HbCss::TokenType t)
{
    if (hasNext() && next() == t)
        return true;
    return recordError();
}

bool Parser::test(HbCss::TokenType t)
{
    if (index >= symbols.count())
        return false;
    if (symbols.at(index).token == t) {
        ++index;
        return true;
    }
    return false;
}

QString Parser::unquotedLexem() const
{
    QString s = lexem();
    if (lookup() == STRING) {
        s.chop(1);
        s.remove(0, 1);
    }
    return s;
}

QString Parser::lexemUntil(HbCss::TokenType t)
{
    QString lexem;
    while (hasNext() && next() != t)
        lexem += symbol().lexem();
    return lexem;
}

bool Parser::until(HbCss::TokenType target, HbCss::TokenType target2)
{
    int braceCount = 0;
    int brackCount = 0;
    int parenCount = 0;
    if (index) {
        switch(symbols.at(index-1).token) {
        case LBRACE: ++braceCount; break;
        case LBRACKET: ++brackCount; break;
        case FUNCTION:
        case LPAREN: ++parenCount; break;
        default: ;
        }
    }
    while (index < symbols.size()) {
        HbCss::TokenType t = symbols.at(index++).token;
        switch (t) {
        case LBRACE: ++braceCount; break;
        case RBRACE: --braceCount; break;
        case LBRACKET: ++brackCount; break;
        case RBRACKET: --brackCount; break;
        case FUNCTION:
        case LPAREN: ++parenCount; break;
        case RPAREN: --parenCount; break;
        default: break;
        }
        if ((t == target || (target2 != NONE && t == target2))
            && braceCount <= 0
            && brackCount <= 0
            && parenCount <= 0)
            return true;

        if (braceCount < 0 || brackCount < 0 || parenCount < 0) {
            --index;
            break;
        }
    }
    return false;
}

bool Parser::testTokenAndEndsWith(HbCss::TokenType t, const QLatin1String &str)
{
    if (!test(t)) return false;
    if (!lexem().endsWith(str, Qt::CaseInsensitive)) {
        prev();
        return false;
    }
    return true;
}

//QT_END_NAMESPACE
