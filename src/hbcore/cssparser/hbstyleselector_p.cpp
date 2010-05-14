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

#include "hbstyleselector_p.h"
#ifndef HB_BIN_CSS
#include <hbicon.h>
#endif
#include <QObject>
#include <QGraphicsWidget>
#include <QMetaProperty>
#include <QMetaEnum>

#define WIDGET(x) (static_cast<QGraphicsWidget *>(x.ptr))

HbStyleSelector::HbStyleSelector() 
{ 
}

int HbStyleSelector::nodeNameEquals(NodePtr node, const HbString& name) const
{
    if (isNullNode(node)) {
        return -1;
    }

    const bool checkNs = name.contains(QLatin1Char('-'));
    const QMetaObject *metaObject = WIDGET(node)->metaObject();
    int level(-1);
    do {
        if ( level >= 0 ) {
            level++;
        } else {
            if (name.compare(QLatin1String(metaObject->className())) == 0) {
                level++;
            } else if (checkNs) {
                QString className = QString::fromUtf8(metaObject->className());
                className.replace(QLatin1Char(':'), QLatin1Char('-'));
                if (className == name) {
                    level++;
                }
            }
        }
        metaObject = metaObject->superClass();
    } while (metaObject != 0);

    return level;
}
    
bool HbStyleSelector::attributeMatches(NodePtr node, const HbCss::AttributeSelector &attr) const
{
    if (isNullNode(node)) {
        return false;
    }

    AttributeValue aVal;
    aVal.mEmptyValue = false;

    QGraphicsWidget *widget = WIDGET(node);

    QHash<QString, AttributeValue> &cache = mAttributeCache[widget];
    QHash<QString, AttributeValue>::const_iterator cacheIt = cache.constFind(attr.name);
    if (cacheIt != cache.constEnd()) {
        aVal = cacheIt.value();
    } else {
        const QMetaObject *metaObject = WIDGET(node)->metaObject();

        QVariant value = widget->property(attr.name.toLatin1());
        if (!value.isValid()) {
            if (attr.name == QLatin1String("class")) {
                QString className = QString::fromLatin1(metaObject
                    ->className());
                if (className.contains(QLatin1Char(':')))
                    className.replace(QLatin1Char(':'), QLatin1Char('-'));
                aVal.mValue1 = className;
            } else {
                // Requested property not found.
                // TODO: Cache miss.
                return false;
            }
        } else {
            aVal.mValue1 = value.toString();
            if (value.type() == QVariant::String) {
                aVal.mEmptyValue = aVal.mValue1.isEmpty();
            } else if (value.type() == QVariant::Bool) {
                aVal.mEmptyValue = !value.toBool();
            } else if (value.userType() == QMetaType::type("HbIcon")) {
#ifdef HB_BIN_CSS
                aVal.mEmptyValue = true;
#else
                HbIcon icon = value.value<HbIcon>();
                aVal.mEmptyValue = icon.isNull();
#endif
            }
            const QMetaProperty metaProperty = metaObject->property(metaObject->indexOfProperty(attr.name.toLatin1()));
            if (metaProperty.isEnumType()) {
                aVal.mValue2 = metaProperty.enumerator().valueToKey(value.toInt());
            }
        }
        cache[attr.name] = aVal;
    }

    bool match(false);
    if (attr.valueMatchCriterium == HbCss::AttributeSelector::MatchContains) {
        QStringList lst = aVal.mValue1.split(QLatin1Char(' ')) + aVal.mValue2.split(QLatin1Char(' '));
        match = lst.contains(attr.value);
    } else if (attr.valueMatchCriterium == HbCss::AttributeSelector::MatchEqual) {
        match = (aVal.mValue1 == attr.value || aVal.mValue2 == attr.value);
    } else if (attr.valueMatchCriterium == HbCss::AttributeSelector::MatchBeginsWith) {
        match = (aVal.mValue1.startsWith(attr.value) || aVal.mValue2.startsWith(attr.value));
    } else if (attr.valueMatchCriterium == HbCss::AttributeSelector::NoMatch) {
        match = !aVal.mEmptyValue;
    }
    return ((!match && attr.negated) || (match && !attr.negated));
}
         
bool HbStyleSelector::hasAttributes(NodePtr) const
{ 
    return true; 
}

QStringList HbStyleSelector::nodeIds(NodePtr node) const
{ 
    return isNullNode(node) ? QStringList() : QStringList(WIDGET(node)->objectName()); 
}

bool HbStyleSelector::isNullNode(NodePtr node) const
{ 
    return node.ptr == 0; 
}

HbStyleSelector::NodePtr HbStyleSelector::parentNode(NodePtr node) const
{ 
    NodePtr n; 
    n.ptr = isNullNode(node) ? 0 : WIDGET(node)->parentWidget(); 
    return n; 
}
HbStyleSelector::NodePtr HbStyleSelector::previousSiblingNode(NodePtr) const
{ 
    NodePtr n; 
    n.ptr = 0; 
    return n; 
}

void HbStyleSelector::initNode(NodePtr) const
{
}

void HbStyleSelector::cleanupNode(NodePtr node) const
{
    mAttributeCache.remove(WIDGET(node));
}

