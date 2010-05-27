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

#ifndef HBSTYLESELECTOR_H
#define HBSTYLESELECTOR_H

#include <hbglobal.h>
#include <hbcssparser_p.h>

#include "hbstring_p.h"

class HB_AUTOTEST_EXPORT HbStyleSelector : public HbCss::StyleSelector
{
public:
    HbStyleSelector();

    int nodeNameEquals(NodePtr node, const HbString& name) const;
    bool attributeMatches(NodePtr node, const HbCss::AttributeSelector &attr) const;
    bool hasAttributes(NodePtr) const;
    QStringList nodeIds(NodePtr node) const;
    bool isNullNode(NodePtr node) const;
    NodePtr parentNode(NodePtr node) const;
    NodePtr previousSiblingNode(NodePtr) const;
    void initNode(NodePtr) const;
    void cleanupNode(NodePtr node) const;

private:
    struct AttributeValue {
        QString mValue1;
        QString mValue2;
        bool mEmptyValue;
    };
    mutable QHash<const QGraphicsWidget *, QHash<uint, AttributeValue> > mAttributeCache;

};

#endif // HBSTYLESELECTOR_H
