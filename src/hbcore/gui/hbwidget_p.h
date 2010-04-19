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

#ifndef HBWIDGET_P_H
#define HBWIDGET_P_H

#include "hbfontspec.h"
#include "hbnamespace.h"

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Hb API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <hbstyle.h>
#include "hbwidget.h"
#include "hbwidgetbase_p.h"

#ifdef HB_TESTABILITY
#include <hbtestabilitysignal_p.h>
#endif //HB_TESTABILITY

class HbWidget;
class HbStyle;
class HbTestabilitySignal_p;
class QGraphicsItem;
class HbFocusGroup;

namespace HbStyleRulesCacheId {
    const char hbStyleRulesForNodeCache[] = "__Hb__Style__Rules__For__Node__Cache__";
}

class HB_CORE_PRIVATE_EXPORT HbWidgetPrivate: public HbWidgetBasePrivate
{
	Q_DECLARE_PUBLIC(HbWidget)

public:
    HbWidgetPrivate();
    virtual ~HbWidgetPrivate();
    QGraphicsItem *focusPrimitive(HbWidget::FocusHighlight highlightType) const;

    void focusChangeEvent(HbWidget::FocusHighlight focusHighlight);

    void updateCurrentFocusChild();
    HbFocusGroup *getFocusGroup(bool propagate = true) const;
    bool delegateFocus(QFocusEvent *event) const;

    void updateBackgroundItemSize();

    bool isInputMethodNeutral();

    QGraphicsLayoutItem *createSpacerItem( const QString &name );
    
    HbWidget *q_ptr;
    HbStyle *style;

#ifdef HB_TESTABILITY
	HbTestabilitySignal_p *testabilitySignal;
#endif //HB_TESTABILITY

    HbStyle::Primitive backgroundPrimitiveType;
    bool polished;
    bool polishPending; 
    bool themingPending;
    bool repolishOutstanding;
    bool notifyScene;
    int pluginBaseId;
    HbFocusGroup *focusGroup;
    HbStyle::Primitive focusActiveType;
    HbStyle::Primitive focusResidualType;
    bool highlightExpired;
    QHash<QString,QGraphicsLayoutItem *> mSpacers;
private:
    mutable QGraphicsItem *backgroundItem;
    mutable QGraphicsItem *focusActiveItem;
    mutable QGraphicsItem *focusResidualItem;
};




#endif // HBWIDGET_P_H
