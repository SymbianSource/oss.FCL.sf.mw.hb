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

#include "hbxmlloaderbinaryactions_p.h"
#include "hbxmlloaderbinarysyntax_p.h"
#include "hbxmlloaderabstractsyntax_p.h"

#include <QCoreApplication>

#define VERSION_MAJOR 0
#define VERSION_MINOR 2

/*
    \class HbXmlLoaderBinaryActions
    \internal
    \proto
*/

HbXmlLoaderBinaryActions::HbXmlLoaderBinaryActions() : HbXmlLoaderAbstractActions()
{
}

HbXmlLoaderBinaryActions::~HbXmlLoaderBinaryActions()
{
}

void HbXmlLoaderBinaryActions::setOutputDevice( QIODevice *device )
{
    mOut.setDevice(device);
}

void HbXmlLoaderBinaryActions::cleanUp()
{
    mOut << (quint8)HbXml::ActionCleanUp;
}

void HbXmlLoaderBinaryActions::reset()
{
    mOut << (quint8)HbXml::ActionReset;
}

void HbXmlLoaderBinaryActions::deleteAll()
{
    mOut << (quint8)HbXml::ActionDeleteAll;
}

bool HbXmlLoaderBinaryActions::pushDocument( const QString& context)
{
    if( mOut.device()->pos() != 0 ) {
        mOut << (quint8)HbXml::ActionPushDocument << context;
        return true;
    }
    mOut.device()->write(HbXmlLoaderBinarySyntax::signature(), strlen(HbXmlLoaderBinarySyntax::signature()));
    mOut << (qint8)VERSION_MAJOR << (qint8)VERSION_MINOR;
    mOut << (quint8)HbXml::ActionPushDocument << context;
    return true;
}

bool HbXmlLoaderBinaryActions::pushObject( const QString& type, const QString &name )
{
    mOut << (quint8)HbXml::ActionPushObject << type << name;
    return true;
}

bool HbXmlLoaderBinaryActions::pushWidget(
    const QString &type,
    const QString &name,
    const QString &role,
    const QString &plugin )
{
    mOut << (quint8)HbXml::ActionPushWidget << type << name << role << plugin;
    return true;
}

bool HbXmlLoaderBinaryActions::pushConnect(
    const QString &srcName,
    const QString &signalName,
    const QString &dstName,
    const QString &slotName )
{
    mOut << (quint8)HbXml::ActionPushConnect << srcName << signalName << dstName << slotName;
    return true;
}

bool HbXmlLoaderBinaryActions::pushProperty( const char *propertyName, const HbXmlVariable &variable )
{
    mOut << (quint8)HbXml::ActionPushProperty << propertyName << variable;
    return true;
}

bool HbXmlLoaderBinaryActions::pushRef( const QString &name, const QString &role )
{
    mOut << (quint8)HbXml::ActionPushRef << name << role;
    return true;
}

bool HbXmlLoaderBinaryActions::pushContainer(
    const char *propertyName,
    HbXmlLoaderAbstractSyntax::DocumentLexems type,
    const QList<HbXmlVariable*> &container )
{
    mOut << (quint8)HbXml::ActionPushContainer << propertyName << (quint8)type << (quint8)container.count();
    for (int i=0; i < container.count(); i++) {
        mOut << *(container.at(i));
    }
    return true;
}


bool HbXmlLoaderBinaryActions::pop( const HbXml::ElementType type )
{
    mOut << (quint8)HbXml::ActionPop << (quint8)type;
    return true;
}


bool HbXmlLoaderBinaryActions::setContentsMargins(
    const HbXmlLengthValue &left,
    const HbXmlLengthValue &top,
    const HbXmlLengthValue &right,
    const HbXmlLengthValue &bottom )
{
    mOut << (quint8)HbXml::ActionSetContentsMargins << left << top << right << bottom;
    return true;
}

bool HbXmlLoaderBinaryActions::setSizePolicy(
    QSizePolicy::Policy *horizontalPolicy,
    QSizePolicy::Policy *verticalPolicy,
    int *horizontalStretch,
    int *verticalStretch )
{
    mOut << (quint8)HbXml::ActionSetSizePolicy;
    if ( horizontalPolicy ) {
        mOut << true << (quint8)*horizontalPolicy;
    } else {
        mOut << false;
    }
    if ( verticalPolicy ) {
        mOut << true << (quint8)*verticalPolicy;
    } else {
        mOut << false;
    }
    if ( horizontalStretch ) {
        mOut << true << (qint16)*horizontalStretch;
    } else {
        mOut << false;
    }
    if ( verticalStretch ) {
        mOut << true << (qint16)*verticalStretch;
    } else {
        mOut << false;
    }
    return true;
}

bool HbXmlLoaderBinaryActions::setSizeHint(
    Qt::SizeHint hint,
    const HbXmlLengthValue &hintWidth,
    const HbXmlLengthValue &hintHeight,
    bool fixed)
{
    mOut << (quint8)HbXml::ActionSetSizeHint << (quint8)hint << hintWidth << hintHeight << fixed;
    return true;
}

bool HbXmlLoaderBinaryActions::setToolTip( const HbXmlVariable &tooltip )
{
    mOut << (quint8)HbXml::ActionSetToolTip << tooltip;
    return true;
}

bool HbXmlLoaderBinaryActions::setBackground( const QString &name, HbFrameDrawer::FrameType type )
{
    mOut << (quint8)HbXml::ActionSetBackground << name << (quint8)type;
    return true;
}


bool HbXmlLoaderBinaryActions::createAnchorLayout( const QString &widget, bool modify )
{
    mOut << (quint8)HbXml::ActionCreateAnchorLayout << widget << modify;
    return true;
}

bool HbXmlLoaderBinaryActions::addAnchorLayoutItem(
    const QString &src,
    const QString &srcId,
    Hb::Edge srcEdge, 
    const QString &dst,
    const QString &dstId,
    Hb::Edge dstEdge,
    const HbXmlLengthValue &minLength,
    const HbXmlLengthValue &prefLength,
    const HbXmlLengthValue &maxLength,
    QSizePolicy::Policy *policy, 
    HbAnchor::Direction *dir,
    const QString &anchorId )
{
    mOut << (quint8)HbXml::ActionAddAnchorLayoutItem;
    mOut << src << srcId << (quint8)srcEdge;
    mOut << dst << dstId << (quint8)dstEdge;
    mOut << minLength << prefLength << maxLength;
    if (policy) {
        mOut << true << (quint8)*policy;
    } else {
        mOut << false;
    }
    if (dir) {
        mOut << true << (quint8)*dir;
    } else {
        mOut << false;
    }
    mOut << anchorId;
    return true;
}

bool HbXmlLoaderBinaryActions::setAnchorLayoutMapping(
    const QString &item,
    const QString &id,
    bool remove)
{
    mOut << (quint8)HbXml::ActionSetAnchorLayoutMapping;
    mOut << item << id << remove;
    return true;
}


bool HbXmlLoaderBinaryActions::createGridLayout(
    const QString &widget,
    const HbXmlLengthValue &spacing,
    bool modify )
{
    mOut << (quint8)HbXml::ActionCreateGridLayout << widget << spacing << modify;
    return true;
}

bool HbXmlLoaderBinaryActions::addGridLayoutCell(
    const QString &src,
    int row,
    int column,
    int *rowspan,
    int *columnspan,
    Qt::Alignment *alignment )
{
    mOut << (quint8)HbXml::ActionAddGridLayoutCell << src << (qint16)row << (qint16)column;
    if ( rowspan ) {
        mOut << true << (qint16)*rowspan;
    } else {
        mOut << false;
    }
    if ( columnspan ) {
        mOut << true << (qint16)*columnspan;
    } else {
        mOut << false;
    }
    if ( alignment ) {
        mOut << true << (quint8)*alignment;
    } else {
        mOut << false;
    }
    return true;
}

bool HbXmlLoaderBinaryActions::setGridLayoutRowProperties(
    int row,
    int *rowStretchFactor,
    Qt::Alignment *alignment )
{
    mOut << (quint8)HbXml::ActionSetGridLayoutRowProperties << (qint16)row;
    if ( rowStretchFactor ) {
        mOut << true << (qint16)*rowStretchFactor;
    } else {
        mOut << false;
    }
    if ( alignment ) {
        mOut << true << (quint8)*alignment;
    } else {
        mOut << false;
    }
    return true;
}

bool HbXmlLoaderBinaryActions::setGridLayoutColumnProperties(
    int column,
    int *columnStretchFactor,
    Qt::Alignment *alignment )
{
    mOut << (quint8)HbXml::ActionSetGridLayoutColumnProperties << (qint16)column;
    if ( columnStretchFactor ) {
        mOut << true << (qint16)*columnStretchFactor;
    } else {
        mOut << false;
    }
    if ( alignment ) {
        mOut << true << (quint8)*alignment;
    } else {
        mOut << false;
    }
    return true;
}

bool HbXmlLoaderBinaryActions::setGridLayoutRowHeights(
    int row,
    const HbXmlLengthValue &minHeight,
    const HbXmlLengthValue &maxHeight,
    const HbXmlLengthValue &prefHeight,
    const HbXmlLengthValue &fixedHeight,
    const HbXmlLengthValue &rowSpacing )
{
    mOut << (quint8)HbXml::ActionSetGridLayoutRowHeights << (qint16)row << minHeight << maxHeight << prefHeight << fixedHeight << rowSpacing;
    return true;
}

bool HbXmlLoaderBinaryActions::setGridLayoutColumnWidths(
    int column,
    const HbXmlLengthValue &minWidth,
    const HbXmlLengthValue &maxWidth,
    const HbXmlLengthValue &prefWidth,
    const HbXmlLengthValue &fixedWidth,
    const HbXmlLengthValue &columnSpacing )
{
    mOut << (quint8)HbXml::ActionSetGridLayoutColumnWidths << (qint16)column << minWidth << maxWidth << prefWidth << fixedWidth << columnSpacing;
    return true;
}
bool HbXmlLoaderBinaryActions::createLinearLayout(
    const QString &widget,
    Qt::Orientation *orientation,
    const HbXmlLengthValue &spacing,
    bool modify )
{
    mOut << (quint8)HbXml::ActionCreateLinearLayout << widget;
    if ( orientation ) {
        mOut << true << (quint8)*orientation;
    } else {
        mOut << false;
    }
    mOut << spacing << modify;
    return true;
}

bool HbXmlLoaderBinaryActions::addLinearLayoutItem(
    const QString &itemname,
    int *index,
    int *stretchfactor,
    Qt::Alignment *alignment,
    const HbXmlLengthValue &spacing )
{
    mOut << (quint8)HbXml::ActionAddLinearLayoutItem << itemname;
    if ( index ) {
        mOut << true << (qint16)*index;
    } else {
        mOut << false;
    }
    if ( stretchfactor ) {
        mOut << true << (qint16)*stretchfactor;
    } else {
        mOut << false;
    }
    if ( alignment ) {
        mOut << true << (quint8)*alignment;
    } else {
        mOut << false;
    }
    mOut << spacing;
    return true;
}

bool HbXmlLoaderBinaryActions::addLinearLayoutStretch(
    int *index,
    int *stretchfactor )
{
    mOut << (quint8)HbXml::ActionAddLinearLayoutStretch;
    if ( index ) {
        mOut << true << (qint16)*index;
    } else {
        mOut << false;
    }
    if ( stretchfactor ) {
        mOut << true << (qint16)*stretchfactor;
    } else {
        mOut << false;
    }
    return true;
}

bool HbXmlLoaderBinaryActions::setLayoutContentsMargins(
    const HbXmlLengthValue &left,
    const HbXmlLengthValue &top,
    const HbXmlLengthValue &right,
    const HbXmlLengthValue &bottom )
{
    mOut << (quint8)HbXml::ActionSetLayoutContentsMargins << left << top << right << bottom;
    return true;
}
bool HbXmlLoaderBinaryActions::createStackedLayout( const QString &widget, bool modify )
{
    mOut << (quint8)HbXml::ActionCreateStackedLayout << widget << modify;
    return true;
}

bool HbXmlLoaderBinaryActions::addStackedLayoutItem( const QString &itemname, int *index )
{
    mOut << (quint8)HbXml::ActionAddStackedLayoutItem << itemname;
    if ( index ) {
        mOut << true << (qint16)*index;
    } else {
        mOut << false;
    }
    return true;
}

bool HbXmlLoaderBinaryActions::createNullLayout( const QString &widget )
{
    mOut << (quint8)HbXml::ActionCreateNullLayout << widget;
    return true;
}

