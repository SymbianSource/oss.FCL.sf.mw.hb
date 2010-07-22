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
#include <QIcon>
#include <QSizePolicy>
#include <QHash>

#include <hbglobal.h>
#include <hbnamespace.h>
#include <hbdeviceprofile.h>
#include <hbfontspec.h>
#include <hbanchor.h>

// smart containers and memory manager inclusion
#include "hbmemorymanager_p.h"
#include "hbvector_p.h"
#include "hbstring_p.h"
#include "hbvariant_p.h"
#include "hbstringvector_p.h"

namespace HbCss
{

enum Property {
    Property_Unknown,
    Property_Alignment,
    Property_AnchorDirection,
    Property_AspectRatio,
    Property_BorderWidth,
    Property_BorderWidthBottom,
    Property_BorderWidthLeft,
    Property_BorderWidthRight,
    Property_BorderWidthTop,
    Property_Bottom,
    Property_CenterHorizontal,
    Property_CenterVertical,
    Property_Color,
    Property_FixedHeight,
    Property_FixedLength,
    Property_FixedSize,
    Property_FixedWidth,
    Property_Font,
    Property_FontFamily,
    Property_FontSize,
    Property_FontStyle,
    Property_FontVariant,
    Property_FontWeight,
    Property_Layout,
    Property_LayoutDirection,
    Property_Left,
    Property_MaximumHeight,
    Property_MaximumLength,
    Property_MaximumSize,
    Property_MaximumWidth,
    Property_MinimumHeight,
    Property_MinimumLength,
    Property_MinimumSize,
    Property_MinimumWidth,
    Property_PreferredHeight,
    Property_PreferredLength,
    Property_PreferredSize,
    Property_PreferredWidth,
    Property_Right,
    Property_Section,
    Property_SizePolicy,
    Property_SizePolicyHorizontal,
    Property_SizePolicyVertical,
    Property_TextAlignment,
    Property_TextDecoration,
    Property_TextHeight,
    Property_TextLineCountMax,
    Property_TextLineCountMin,
    Property_TextTransform,
    Property_TextWrapMode,
    Property_Top,
    Property_ZValue,
    NumProperties
};

enum KnownValue {
    UnknownValue,
    Value_Normal,
    Value_Italic,
    Value_Oblique,
    Value_Bold,
    Value_Underline,
    Value_Overline,
    Value_LineThrough,
    Value_Left,
    Value_Right,
    Value_Top,
    Value_Bottom,
    Value_Center,
    Value_None,
    Value_Transparent,
    Value_SmallCaps,
    Value_Uppercase,
    Value_Lowercase,

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
    
    Value_LeftToRight,
    Value_RightToLeft,
    Value_Parent,

    Value_NoWrap,
    Value_WordWrap,
    Value_WrapAnywhere,

    Value_Positive,
    Value_Negative,

    NumKnownValues
};

enum Edge {
    TopEdge,
    RightEdge,
    BottomEdge,
    LeftEdge,
    NumEdges
};

enum LayoutDirection {
    LayoutDirection_LeftToRight,
    LayoutDirection_RightToLeft,
    LayoutDirection_Parent,
    NumKnownLayoutDirections
};

struct HB_CORE_PRIVATE_EXPORT Value //krazy:exclude=multiclasses
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

#ifdef CSS_PARSER_TRACES
    void print() const;
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

struct HB_CORE_PRIVATE_EXPORT Declaration //krazy:exclude=multiclasses
{
    inline Declaration(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        :memoryType(type),
        property(type),
        propertyId(Property_Unknown),
        values(type),
        important(false)
    {}

    inline bool isEmpty() const { return property.isEmpty() && propertyId == Property_Unknown; }

#ifdef CSS_PARSER_TRACES
    void print() const;
#endif

    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString property;
    Property propertyId;
    HbVector<Value> values;
    bool important;
};

typedef QPair<int, Declaration> WeightedDeclaration;

const quint32 PseudoClass_Unknown          = 0x00000000;
const quint32 PseudoClass_Unspecified      = 0x00000001;
const quint32 PseudoClass_Landscape        = 0x00000002;
const quint32 PseudoClass_Portrait         = 0x00000004;
const quint32 PseudoClass_LeftToRight      = 0x00000008;
const quint32 PseudoClass_RightToLeft      = 0x00000010;
// The Any specifier is never generated, but can be used as a wildcard in searches.
const quint32 PseudoClass_Any              = 0x00000020;
const int NumPseudos = 7;

struct HB_CORE_PRIVATE_EXPORT Pseudo //krazy:exclude=multiclasses
{
    Pseudo(HbMemoryManager::MemoryType memType = HbMemoryManager::HeapMemory) 
        : memoryType(memType),
          negated(false),
          name(memType),
          function(memType)
    { }

#ifdef CSS_PARSER_TRACES
    void print() const;
#endif

    //Data
    HbMemoryManager::MemoryType memoryType;
    bool negated;
    quint64 type;
    HbString name;
    HbString function;
};

struct HB_CORE_PRIVATE_EXPORT AttributeSelector //krazy:exclude=multiclasses
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
          nameHash(0),
          value(type),
          valueMatchCriterium(NoMatch),
          negated(false)         
    {}
#ifdef CSS_PARSER_TRACES
    void print() const;
#endif

    HbMemoryManager::MemoryType memoryType;
    HbString name;
    uint nameHash;
    HbString value;
    ValueMatchType valueMatchCriterium;
    bool negated;

};

struct HB_CORE_PRIVATE_EXPORT BasicSelector //krazy:exclude=multiclasses
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
    void print() const;
#endif

    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString elementName;

    HbStringVector ids;
    HbVector<Pseudo> pseudos;
    HbVector<AttributeSelector> attributeSelectors;
    Relation relationToNext;
};

struct HB_CORE_PRIVATE_EXPORT Selector //krazy:exclude=multiclasses
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
    void print() const;
#endif

    // Data
    HbMemoryManager::MemoryType memoryType;
    HbVector<BasicSelector> basicSelectors;
};

enum KnownPropertyFlag {

    // Generic
    ExtractedLeft       = 0x00000001,
    ExtractedRight      = 0x00000002,
    ExtractedTop        = 0x00000004,
    ExtractedBottom     = 0x00000008,
    ExtractedCenterH    = 0x00000010,
    ExtractedCenterV    = 0x00000020,

    ExtractedPrefW      = 0x00000100,
    ExtractedPrefH      = 0x00000200,
    ExtractedMinW       = 0x00000400,
    ExtractedMinH       = 0x00000800,
    ExtractedMaxW       = 0x00001000,
    ExtractedMaxH       = 0x00002000,
    ExtractedPolHor     = 0x00004000,
    ExtractedPolVer     = 0x00008000,

    ExtractedLayoutDir  = 0x00010000,
    ExtractedZValue     = 0x00020000,
    ExtractedAlignment  = 0x00040000,

    // Text specific
    ExtractedWrapMode   = 0x00200000,
    ExtractedMinLines   = 0x00400000,
    ExtractedMaxLines   = 0x00800000,
    ExtractedFont       = 0x01000000,
    ExtractedFontSpec   = 0x02000000,

    // Icon specific
    ExtractedAspectRatioMode    = 0x10000000,

    // Frame specific
    ExtractedBorderWidths       = 0x20000000,

    // Anchor specific
    ExtractedAnchorDir          = 0x80000000
};
Q_DECLARE_FLAGS(KnownPropertyFlags, KnownPropertyFlag)

struct KnownProperties
{
    qreal mLeft, mTop, mRight, mBottom, mCenterH, mCenterV, mZ;
    qreal mPrefW, mPrefH, mMinW, mMinH, mMaxW, mMaxH;
    QSizePolicy mSizePolicy;

    HbCss::LayoutDirection mLayoutDir;    
    Qt::Alignment mAlignment;

    Hb::TextWrapping mTextWrapMode;
    int mMinLines, mMaxLines;

    QFont mFont;
    HbFontSpec mFontSpec;

    Qt::AspectRatioMode mAspectRatioMode;

    qreal mBorderWidths[HbCss::NumEdges];

    HbAnchor::Direction mAnchorDir;

    KnownPropertyFlags mFlags;
};

struct HB_CORE_PRIVATE_EXPORT ValueExtractor //krazy:exclude=multiclasses
{
    ValueExtractor(const HbVector<Declaration> &declarations, const HbDeviceProfile &profile);
    ValueExtractor(const HbVector<Declaration> &declarations, const QHash<QString, HbCss::Declaration> &varDeclarations,
                   const HbDeviceProfile &profile);
    ValueExtractor(const HbVector<Declaration> &varDeclarations, bool isVariable, const HbDeviceProfile &profile = HbDeviceProfile());
    ValueExtractor(const QHash<QString, HbCss::Declaration> &varDecls, bool isVariable, const HbDeviceProfile &profile = HbDeviceProfile());

    bool extractVariableValue(const QString &variableName, HbVector<HbCss::Value>& values) const;
    bool extractVariableValue(const QString &variableName, qreal& value);
    bool extractVariableValue(const QString &variableName, HbCss::Value &value) const;
    bool extractExpressionValue(QString &expression, qreal &value);

    bool extractKnownProperties(KnownProperties &prop);
    bool extractCustomProperties(const QList<QString> &keys, QList<QVariant> &values);
    bool extractLayout(QString &layoutName, QString &sectionName);
    bool extractColor(QColor &color) const;

private:

    qreal asReal(const Declaration &decl, bool *ok = 0);
    qreal asReal(const Value &v, bool *ok = 0);
    qreal asReal(QString &s, Value::Type type, bool *ok = 0);
    bool asReals(const Declaration &decl, qreal *m);

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
    HbDeviceProfile currentProfile;
    QList<ExpressionValue> expressionValues; // for parsed expression string
};

struct StyleSheet;

struct HB_CORE_PRIVATE_EXPORT StyleRule //krazy:exclude=multiclasses
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
    void print() const;
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

struct HB_CORE_PRIVATE_EXPORT VariableRule //krazy:exclude=multiclasses
{
    VariableRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType (type),
          declarations(type)
    {}

#ifdef CSS_PARSER_TRACES
    void print() const;
#endif

    // Data
    HbMemoryManager::MemoryType memoryType;
    HbVector<Declaration> declarations;
};

struct HB_CORE_PRIVATE_EXPORT MediaRule //krazy:exclude=multiclasses
{
    MediaRule(HbMemoryManager::MemoryType type=HbMemoryManager::HeapMemory)
        : memoryType(type),
          media(type),
          styleRules (type)
    {}

#ifdef CSS_PARSER_TRACES
    void print() const;
#endif

    // data
    HbMemoryManager::MemoryType memoryType;
    //ToDo: Replace it with HbStringList if we have it in future
    HbStringVector media;
    HbVector<StyleRule> styleRules;
};

struct HB_CORE_PRIVATE_EXPORT PageRule //krazy:exclude=multiclasses
{
    PageRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
          selector(type),
          declarations (type)
    {}

#ifdef CSS_PARSER_TRACES
    void print() const;
#endif

    // Data
    HbMemoryManager::MemoryType memoryType;
    HbString selector;
    HbVector<Declaration> declarations;
};

struct HB_CORE_PRIVATE_EXPORT ImportRule //krazy:exclude=multiclasses
{
    ImportRule(HbMemoryManager::MemoryType type = HbMemoryManager::HeapMemory)
        : memoryType(type),
          href(type),
          media(type)
    {}

#ifdef CSS_PARSER_TRACES
    void print() const;
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
    void print() const;
#endif

    // Data
	uint classNameHash;
    HbVector<StyleRule> styleRules;
    HbVector<StyleRule> portraitRules;
    HbVector<StyleRule> landscapeRules;
};

struct HB_AUTOTEST_EXPORT StyleSheet //krazy:exclude=multiclasses
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
    void print() const;
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

class HB_AUTOTEST_EXPORT StyleSelector //krazy:exclude=multiclasses
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

struct HB_CORE_PRIVATE_EXPORT Symbol //krazy:exclude=multiclasses
{
    inline Symbol() : start(0), len(-1) {}
    TokenType token;
    QString text;
    int start, len;
    QString lexem() const;
};

class Scanner
{
public:
    static QString preprocess(const QString &input, bool *hasEscapeSequences = 0);
    static void scan(const QString &preprocessedInput, QVector<Symbol> *symbols);
};

class HB_CORE_PRIVATE_EXPORT Parser //krazy:exclude=multiclasses
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

#endif // HBCSSPARSER_P_H
