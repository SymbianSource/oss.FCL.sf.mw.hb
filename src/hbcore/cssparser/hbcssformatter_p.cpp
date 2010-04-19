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

#include "hbcssformatter_p.h"

/*!
    \class HbCssFormatter
    \brief Internal class for converting in-memory CSS structures (declarations and rules)
	to string representations.
	
	These functions only work in builds configured with the --developer flag; in other
	builds the implementation is defined-out to reduce ROM size, and an empty string
	is returned from all functions.
	
	Example usage would be:
       HbVector<HbCss::WeightedRule> rules;
       [...]
       qDebug() << HbCssFormatter::weightedStyleRulesToString(rules);
*/



/*!
 @internal
*/
QString HbCssFormatter::weightedStyleRulesToString(const HbVector<HbCss::WeightedRule> &rules)
{
	QString str;
#ifdef BUILD_HB_INTERNAL
	foreach (const HbCss::WeightedRule &rule, rules) {
		if (str.length() > 0) {
			str.append("\n");
		}
		str.append(styleRuleToString(rule.second, rule.first));
	}
#else
    Q_UNUSED( rules );
#endif
    return str;
}



/*!
 @internal
*/
QString HbCssFormatter::weightedDeclarationsToString(const HbVector<HbCss::WeightedDeclaration> &decls)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    foreach (const HbCss::WeightedDeclaration &decl, decls) {
        if (str.length() > 0) {
            str.append("\n");
        }
        str.append(declarationToString(decl.second));
        str.append("(");
        str.append(QString::number(decl.first));
        str.append(")");
    }
#else
    Q_UNUSED( decls );
#endif
    return str;
}



/*!
 @internal
*/
QString HbCssFormatter::styleRulesToString(const HbVector<HbCss::StyleRule> &rules)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    foreach (const HbCss::StyleRule &rule, rules) {
        if (str.length() > 0) {
            str.append("\n\n");
        }
        str.append(styleRuleToString(rule));
    }
#else
    Q_UNUSED( rules );
#endif
    return str;
}



/*!
 @internal
*/
QString HbCssFormatter::styleRulesToHtml(const HbVector<HbCss::StyleRule> &rules)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    QMap<QString, QVector<QString> > usedProperties;
    QVector<const HbCss::Declaration*> usedDecls;
    for (int i=rules.count()-1; i>=0; i--) {
        const HbCss::StyleRule &rule = rules.at(i);
        foreach (const HbCss::Selector &sel, rule.selectors) {
            foreach (const HbCss::BasicSelector &basi, sel.basicSelectors ) {
                QVector<QString> names;
                if (basi.pseudos.count() == 0) {
                    names.append("*");
                } else {
                    foreach (const HbCss::Pseudo &pseu, basi.pseudos) {
                        names.append(pseu.name);
                    }
                }
                foreach (const QString &name, names) {
                    for (int j=rule.declarations.count()-1; j>=0; j--) {
                        const HbCss::Declaration &decl = rule.declarations.at(j);
                        QString prop = decl.property;
                        if (usedProperties[name].contains(prop)) {
                            usedDecls.append(&decl);
                        } else {
                            usedProperties[name].append(prop);
                            // TODO: Add grouped properties here, eg border-left
                        }
                    }
                }
            }
        }
    }

    foreach (const HbCss::StyleRule &rule, rules) {
        if (str.length() > 0) {
            str.append("<br/><br/>");
        }
        str.append(styleRuleToHtml(rule, usedDecls));
    }
#else
    Q_UNUSED(rules);
#endif
    return str;
}



/*!
 @internal
*/
QString HbCssFormatter::declarationsToString(const HbVector<HbCss::Declaration> &decls)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    foreach (const HbCss::Declaration &decl, decls ) {
        str.append("\n    ");
        str.append(declarationToString(decl));
    }
#else
    Q_UNUSED( decls );
#endif
    return str;
}



// Worker function
QString HbCssFormatter::styleRuleToString(const HbCss::StyleRule &rule, int specificity)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    str.append(selectorsToString(rule.selectors, specificity));
    str.append("\n{");
    str.append(declarationsToString(rule.declarations));
    str.append("\n}");
#else
    Q_UNUSED(rule);
    Q_UNUSED(specificity);
#endif
    return str;
}



// Worker function
QString HbCssFormatter::styleRuleToHtml(const HbCss::StyleRule &rule, QVector<const HbCss::Declaration*> usedDecls, int specificity)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    str.append("<p class=\"selectors\">");
    str.append(selectorsToString(rule.selectors, specificity, true));
    str.append("</p>{");
    for (int i=0; i<rule.declarations.count(); i++) {
        str.append("<br/>&nbsp;&nbsp;&nbsp;&nbsp;");
        const HbCss::Declaration &decl = rule.declarations.at(i);
        bool overridden = usedDecls.contains(&decl);
        if(overridden) str.append("<span class=\"overridden\">");
        str.append(declarationToString(decl, true));
        if(overridden) str.append("</span>");
    }
    str.append("<br/>}");
#else
    Q_UNUSED(rule);
    Q_UNUSED(usedDecls);
    Q_UNUSED(specificity);
#endif
    return str;
}



// Worker function
QString HbCssFormatter::selectorsToString(const HbVector<HbCss::Selector> &selectors, int specificity, bool html)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    foreach (const HbCss::Selector &sel, selectors) {
        if (str.length() > 0) {
            str.append(',');
        }
        if (html) str.append("<span class=\"selector\">");
        str.append(selectorToString(sel, html));
        if (specificity >= 0) {
            str.append(" (specificity: 0x");
            str.append(QString::number(specificity, 16));
            str.append(")");
        }
        if (html) str.append("</span>");
    }
#else
    Q_UNUSED( selectors );
    Q_UNUSED( specificity );
    Q_UNUSED( html );
#endif
    return str;
}



// Worker function
QString HbCssFormatter::declarationToString(const HbCss::Declaration &decl, bool html)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    if(html) str.append("<span class=\"property\">");
    str.append(decl.property);
    if(html) str.append("</span>");
    str.append(":");
    if(html) str.append("<span class=\"value\">");
    foreach (const HbCss::Value &val, decl.values) {
        str.append(" ");
        switch (val.type) {
            case HbCss::Value::ExpressionNegative:
                str.append("-");
            case HbCss::Value::Expression:
                str.append("expr(");
                str.append(val.variant.toString());
                str.append(")");
                break;
            case HbCss::Value::VariableNegative:
                str.append("-");
            case HbCss::Value::Variable:
                str.append("var(");
                str.append(val.variant.toString());
                str.append(")");
                break;
            case HbCss::Value::KnownIdentifier:
                str.append(val.original);
                break;
            default:
                str.append(val.variant.toString());
        }
    }
    if(html) str.append("</span>");
    str.append(";");
#else
    Q_UNUSED( decl );
    Q_UNUSED( html );
#endif
    return str;
}



// Worker function
QString HbCssFormatter::selectorToString(const HbCss::Selector &sel, bool html)
{
    QString str;
#ifdef BUILD_HB_INTERNAL
    foreach (const HbCss::BasicSelector &basi, sel.basicSelectors ) {
        if (basi.elementName.length() > 0) {
            str.append( basi.elementName );
        }else{
            str.append("*");
        }
        foreach (const HbString &iidd, basi.ids ) {
            str.append('#');
            str.append(iidd);
        }
        foreach (const HbCss::AttributeSelector &atri, basi.attributeSelectors) {
            if(html) str.append("<span class=\"attr\">");
            str.append('[');
            if (atri.negated) {
                str.append('!');
            }
            if (atri.valueMatchCriterium == HbCss::AttributeSelector::MatchEqual) {
                str.append(atri.name);
                str.append('=');
                str.append(atri.value);
            } else if (atri.valueMatchCriterium == HbCss::AttributeSelector::MatchBeginsWith) {
                str.append("startsWith(@");
                str.append(atri.name);
                str.append(',');
                str.append(atri.value);
                str.append(')');
            } else if (atri.valueMatchCriterium == HbCss::AttributeSelector::MatchContains) {
                str.append("contains(@");
                str.append(atri.name);
                str.append(',');
                str.append(atri.value);
                str.append(')');
            } else {
                str.append(atri.name);
            }
            str.append(']');
            if(html) str.append("</span>");
        }
        foreach (const HbCss::Pseudo &pseu, basi.pseudos) {
            if(html) str.append("<span class=\"pseudo\">");
            if (pseu.type == HbCss::PseudoClass_Unknown) {
                str.append("::");
            } else {
                str.append(":");
            }
            if (pseu.negated) {
                str.append('!');
            }
            str.append(pseu.name);
            if(html) str.append("</span>");
        }
        if (basi.relationToNext == HbCss::BasicSelector::MatchNextSelectorIfAncestor) {
            str.append(' ');
        } else if (basi.relationToNext == HbCss::BasicSelector::MatchNextSelectorIfParent) {
            str.append(html ? " &gt; " : " > ");
        } else if (basi.relationToNext == HbCss::BasicSelector::MatchNextSelectorIfPreceeds) {
            str.append(" + ");
        }
    }
#else
    Q_UNUSED( sel );
    Q_UNUSED( html );
#endif
    return str;
}
