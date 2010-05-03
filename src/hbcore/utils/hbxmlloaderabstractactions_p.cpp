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

#include "hbxmlloaderabstractactions_p.h"
#include "hbxmlloaderabstractsyntax_p.h"

#include <QCoreApplication>

#define RETURNVALUE true

/*
    \class HbXmlLoaderAbstractActions
    \internal
    \proto
*/

HbXmlLoaderAbstractActions::HbXmlLoaderAbstractActions()
{
}

HbXmlLoaderAbstractActions::~HbXmlLoaderAbstractActions()
{
}

void HbXmlLoaderAbstractActions::cleanUp()
{
}

void HbXmlLoaderAbstractActions::reset()
{
}

void HbXmlLoaderAbstractActions::deleteAll()
{
}

bool HbXmlLoaderAbstractActions::pushDocument( const QString& context)
{
    Q_UNUSED(context);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushObject( const QString& type, const QString &name )
{
    Q_UNUSED(type);
    Q_UNUSED(name);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushWidget(
    const QString& type,
    const QString &name,
    const QString &role,
    const QString &plugin )
{
    Q_UNUSED(type);
    Q_UNUSED(name);
    Q_UNUSED(role);
    Q_UNUSED(plugin);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushSpacerItem( const QString &name, const QString &widget )
{
    Q_UNUSED(name);
    Q_UNUSED(widget);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushConnect(
    const QString &srcName,
    const QString &signalName,
    const QString &dstName,
    const QString &slotName )
{
    Q_UNUSED(srcName);
    Q_UNUSED(signalName);
    Q_UNUSED(dstName);
    Q_UNUSED(slotName);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushProperty( const char *propertyName, const HbXmlVariable &variable )
{
    Q_UNUSED(propertyName);
    Q_UNUSED(variable);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushRef( const QString &name, const QString &role )
{
    Q_UNUSED(name);
    Q_UNUSED(role);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::pushContainer(
    const char *propertyName,
    HbXmlLoaderAbstractSyntax::DocumentLexems type,
    const QList<HbXmlVariable*> &container )
{
    Q_UNUSED(propertyName);
    Q_UNUSED(type);
    Q_UNUSED(container);
    return RETURNVALUE;
}

        
bool HbXmlLoaderAbstractActions::pop( const HbXml::ElementType type )
{
    Q_UNUSED(type);
    return RETURNVALUE;
}


bool HbXmlLoaderAbstractActions::setContentsMargins( 
    const HbXmlLengthValue &left,
    const HbXmlLengthValue &top,
    const HbXmlLengthValue &right,
    const HbXmlLengthValue &bottom )
{
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setSizePolicy(
    QSizePolicy::Policy *horizontalPolicy, 
    QSizePolicy::Policy *verticalPolicy, 
    int *horizontalStretch,
    int *verticalStretch )
{
    Q_UNUSED(horizontalPolicy);
    Q_UNUSED(verticalPolicy);
    Q_UNUSED(horizontalStretch);
    Q_UNUSED(verticalStretch);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setSizeHint(
    Qt::SizeHint hint,
    const HbXmlLengthValue &hintWidth,
    const HbXmlLengthValue &hintHeight,
    bool fixed)
{
    Q_UNUSED(hint);
    Q_UNUSED(hintWidth);
    Q_UNUSED(hintHeight);
    Q_UNUSED(fixed);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setToolTip( const HbXmlVariable &tooltip )
{
    Q_UNUSED(tooltip);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::createAnchorLayout( const QString &widget )
{
    Q_UNUSED(widget);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::addAnchorLayoutEdge(
    const QString &src,
    Hb::Edge srcEdge, 
    const QString &dst,
    Hb::Edge dstEdge,
    const HbXmlLengthValue &spacing,
    const QString &spacer )
{
    Q_UNUSED(src);
    Q_UNUSED(srcEdge);
    Q_UNUSED(dst);
    Q_UNUSED(dstEdge);
    Q_UNUSED(spacing);
    Q_UNUSED(spacer);
    return RETURNVALUE;
}


bool HbXmlLoaderAbstractActions::createMeshLayout( const QString &widget )
{
    Q_UNUSED(widget);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::addMeshLayoutEdge(
    const QString &src,
    Hb::Edge srcEdge, 
    const QString &dst,
    Hb::Edge dstEdge,
    const HbXmlLengthValue &spacing,
    const QString &spacer )
{
    Q_UNUSED(src);
    Q_UNUSED(srcEdge);
    Q_UNUSED(dst);
    Q_UNUSED(dstEdge);
    Q_UNUSED(spacing);
    Q_UNUSED(spacer);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::createGridLayout( const QString &widget, const HbXmlLengthValue &spacing )
{
    Q_UNUSED(widget);
    Q_UNUSED(spacing);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::addGridLayoutCell(
    const QString &src,
    int row, 
    int column,
    int *rowspan,
    int *columnspan,
    Qt::Alignment *alignment )
{
    Q_UNUSED(src);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(rowspan);
    Q_UNUSED(columnspan);
    Q_UNUSED(alignment);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setGridLayoutRowProperties(
    int row,
    int *rowStretchFactor,
    Qt::Alignment *alignment )
{
    Q_UNUSED(row);
    Q_UNUSED(rowStretchFactor);
    Q_UNUSED(alignment);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setGridLayoutColumnProperties(
    int column,
    int *columnStretchFactor,
    Qt::Alignment *alignment )
{
    Q_UNUSED(column);
    Q_UNUSED(columnStretchFactor);
    Q_UNUSED(alignment);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setGridLayoutRowHeights(
    int row,
    const HbXmlLengthValue &minHeight,
    const HbXmlLengthValue &maxHeight, 
    const HbXmlLengthValue &prefHeight,
    const HbXmlLengthValue &fixedHeight, 
    const HbXmlLengthValue &rowSpacing )
{
    Q_UNUSED(row);
    Q_UNUSED(minHeight);
    Q_UNUSED(maxHeight);
    Q_UNUSED(prefHeight);
    Q_UNUSED(fixedHeight);
    Q_UNUSED(rowSpacing);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setGridLayoutColumnWidths(
    int column,
    const HbXmlLengthValue &minWidth,
    const HbXmlLengthValue &maxWidth,
    const HbXmlLengthValue &prefWidth,
    const HbXmlLengthValue &fixedWidth,
    const HbXmlLengthValue &columnSpacing )
{
    Q_UNUSED(column);
    Q_UNUSED(minWidth);
    Q_UNUSED(maxWidth);
    Q_UNUSED(prefWidth);
    Q_UNUSED(fixedWidth);
    Q_UNUSED(columnSpacing);
    return RETURNVALUE;
}
bool HbXmlLoaderAbstractActions::createLinearLayout(
    const QString &widget,
    Qt::Orientation *orientation, 
    const HbXmlLengthValue &spacing )
{
    Q_UNUSED(widget);
    Q_UNUSED(orientation);
    Q_UNUSED(spacing);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::addLinearLayoutItem(
    const QString &itemname,
    int *index,
    int *stretchfactor, 
    Qt::Alignment *alignment,
    const HbXmlLengthValue &spacing )
{
    Q_UNUSED(itemname);
    Q_UNUSED(index);
    Q_UNUSED(stretchfactor);
    Q_UNUSED(alignment);
    Q_UNUSED(spacing);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::addLinearLayoutStretch(
    int *index,
    int *stretchfactor )
{
    Q_UNUSED(index);
    Q_UNUSED(stretchfactor);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::setLayoutContentsMargins(
    const HbXmlLengthValue &left,
    const HbXmlLengthValue &top,
    const HbXmlLengthValue &right,
    const HbXmlLengthValue &bottom )
{
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);
    return RETURNVALUE;
}
bool HbXmlLoaderAbstractActions::createStackedLayout( const QString &widget )
{
    Q_UNUSED(widget);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::addStackedLayoutItem( const QString &itemname, int *index )
{
    Q_UNUSED(itemname);
    Q_UNUSED(index);
    return RETURNVALUE;
}

bool HbXmlLoaderAbstractActions::createNullLayout( const QString &widget )
{
    Q_UNUSED(widget);
    return RETURNVALUE;
}

