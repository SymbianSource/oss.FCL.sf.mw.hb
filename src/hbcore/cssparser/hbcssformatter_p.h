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

#ifndef HBCSSFORMATTER_P_H
#define HBCSSFORMATTER_P_H

#include "hbcssparser_p.h"

class HB_AUTOTEST_EXPORT HbCssFormatter {

public:
    static QString weightedStyleRulesToString(const HbVector<HbCss::WeightedRule> &rules);
    static QString weightedDeclarationsToString(const HbVector<HbCss::WeightedDeclaration> &decls);
    static QString styleRulesToString(const HbVector<HbCss::StyleRule> &rules);
    static QString styleRulesToHtml(const HbVector<HbCss::StyleRule> &rules);
    static QString declarationsToString(const HbVector<HbCss::Declaration> &decls);
    
private:
    static QString styleRuleToString(const HbCss::StyleRule &rule, int specificity=-1);
    static QString styleRuleToHtml(const HbCss::StyleRule &rule, QVector<const HbCss::Declaration*> usedDecls, int specificity=-1);
    static QString declarationToString(const HbCss::Declaration &decl, bool html=false);
    static QString selectorsToString(const HbVector<HbCss::Selector> &selectors, int specificity=-1, bool html=false);
    static QString selectorToString(const HbCss::Selector &sel, bool html=false);
    
};

#endif // HBCSSFORMATTER_P_H
