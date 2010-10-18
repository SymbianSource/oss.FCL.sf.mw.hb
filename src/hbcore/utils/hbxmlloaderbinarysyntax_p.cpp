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

#include "hbxmlloaderbinarysyntax_p.h"
#include "hbxmlloaderabstractactions_p.h"

#include <QDebug>

// Document loader version number
#define VERSION_MAJOR 0
#define VERSION_MINOR 2

#define MIN_SUPPORTED_VERSION_MAJOR 0
#define MIN_SUPPORTED_VERSION_MINOR 2

// <start-of-text> + "hbBIN" + <end-of-text>
const char *BINARYFORMATSIGNATURE = "\x02\x68\x62\x42\x49\x4e\x03";

//#define BINARY_SYNTAX_DEBUG

/*
    \class HbXmlLoaderBinarySyntax
    \internal
    \proto
*/

HbXmlLoaderBinarySyntax::HbXmlLoaderBinarySyntax( HbXmlLoaderAbstractActions *actions )
    : HbXmlLoaderAbstractSyntax(actions)
{
}

HbXmlLoaderBinarySyntax::~HbXmlLoaderBinarySyntax()
{
}

const char* HbXmlLoaderBinarySyntax::signature()
{
    return BINARYFORMATSIGNATURE;
}

QString HbXmlLoaderBinarySyntax::version()
{
    return ( QString::number( VERSION_MAJOR ) + QString( "." )
            + QString::number( VERSION_MINOR ) + QString( " (" )
            + QString::number( MIN_SUPPORTED_VERSION_MAJOR ) + QString( "." )
            + QString::number( MIN_SUPPORTED_VERSION_MINOR ) + QString( ")" ) );
}

bool HbXmlLoaderBinarySyntax::isBinary( QIODevice *device )
{
    qint64 pos = device->pos();
    QByteArray bytes = device->read( strlen(HbXmlLoaderBinarySyntax::signature()) );
    bool result = !strcmp( bytes.constData(), HbXmlLoaderBinarySyntax::signature() );
    device->seek( pos );
    return result;
}

bool HbXmlLoaderBinarySyntax::load( QIODevice *device, const QString &section )
{
    bool result(true);
    mIn.setDevice( device );

    result = validateDocument();

    // section support start
    if( !section.isEmpty() && result ) {
        qint64 fileSize = mIn.device()->pos() + mIn.device()->bytesAvailable();
        qint64 nextPos;
        QHash< QString, qint64 > sectionsList;

        mIn.device()->seek( fileSize - sizeof( qint64 ) );
        mIn >> nextPos;

        mIn.device()->seek( nextPos );
        mIn >> sectionsList;

        if( sectionsList.contains( section ) ) {
            mIn.device()->seek( sectionsList.value( section ) );
        } else {
            // error
            qWarning() << "No such section " << section;
            result = false;
        }
    }
    // section support end


    if ( result ) {
        result = processDocument(section);
    }
    return result;
}

bool HbXmlLoaderBinarySyntax::processDocument( const QString &section )
{
    Q_UNUSED(section);
    bool result(true);

#ifdef BINARY_SYNTAX_DEBUG
    QMap<HbXml::ActionType, int> hitList;
#endif

    HbXml::ActionType action;
    quint8 temp;
    mIn >> temp;
    action = (HbXml::ActionType)temp;
    while ( result && action != HbXml::ActionEnd ) {
#ifdef BINARY_SYNTAX_DEBUG
        if (hitList.contains(action)) {
            hitList[action]++;
        } else {
            hitList.insert(action, 1);
        }

        qDebug() << "HbXmlLoaderBinarySyntax, action:" << action;
#endif
        switch (action) {
        case HbXml::ActionReset: result = parseReset(); break;
        case HbXml::ActionCleanUp: result = parseCleanUp(); break;
        case HbXml::ActionDeleteAll: result = parseDeleteAll(); break;
        case HbXml::ActionPushDocument: result = parsePushDocument(); break;
        case HbXml::ActionPushObject: result = parsePushObject(); break;
        case HbXml::ActionPushWidget: result = parsePushWidget(); break;
        case HbXml::ActionPushConnect: result = parsePushConnect(); break;
        case HbXml::ActionPushProperty: result = parsePushProperty(); break;
        case HbXml::ActionPushRef: result = parsePushRef(); break;
        case HbXml::ActionPushContainer: result = parsePushContainer(); break;
        case HbXml::ActionPop: result = parsePop(); break;
        case HbXml::ActionSetContentsMargins: result = parseSetContentsMargins(); break;
        case HbXml::ActionSetSizePolicy: result = parseSetSizePolicy(); break;
        case HbXml::ActionSetSizeHint: result = parseSetSizeHint(); break;
        case HbXml::ActionSetToolTip: result = parseSetToolTip(); break;
        case HbXml::ActionSetBackground: result = parseSetBackground(); break;
        case HbXml::ActionCreateAnchorLayout: result = parseCreateAnchorLayout(); break;
        case HbXml::ActionAddAnchorLayoutItem: result = parseAddAnchorLayoutItem(); break;
        case HbXml::ActionSetAnchorLayoutMapping: result = parseSetAnchorLayoutMapping(); break;
        case HbXml::ActionCreateGridLayout: result = parseCreateGridLayout(); break;
        case HbXml::ActionAddGridLayoutCell: result = parseAddGridLayoutCell(); break;
        case HbXml::ActionSetGridLayoutRowProperties: result = parseSetGridLayoutRowProperties(); break;
        case HbXml::ActionSetGridLayoutColumnProperties: result = parseSetGridLayoutColumnProperties(); break;
        case HbXml::ActionSetGridLayoutRowHeights: result = parseSetGridLayoutRowHeights(); break;
        case HbXml::ActionSetGridLayoutColumnWidths: result = parseSetGridLayoutColumnWidths(); break;
        case HbXml::ActionCreateLinearLayout: result = parseCreateLinearLayout(); break;
        case HbXml::ActionAddLinearLayoutItem: result = parseAddLinearLayoutItem(); break;
        case HbXml::ActionAddLinearLayoutStretch: result = parseAddLinearLayoutStretch(); break;
        case HbXml::ActionSetLayoutContentsMargins: result = parseSetLayoutContentsMargins(); break;
        case HbXml::ActionCreateStackedLayout: result = parseCreateStackedLayout(); break;
        case HbXml::ActionAddStackedLayoutItem: result = parseAddStackedLayoutItem(); break;
        case HbXml::ActionCreateNullLayout: result = parseCreateNullLayout(); break;
        default:
            // Unknown action
            result = false;
            break;
        }
        if (result) {
            if ( mIn.atEnd() || action == HbXml::ActionCleanUp ) {
                action = HbXml::ActionEnd;
            } else {
                mIn >> temp;
                action = (HbXml::ActionType)temp;
            }
        } else {
            // Failure
            qDebug() << "HbXmlLoaderBinarySyntax, failed at action" << action;
            mActions->deleteAll();
        }
    }
#ifdef BINARY_SYNTAX_DEBUG
    qDebug() << "HbXmlLoaderBinarySyntax, action statistics";
    QMap<HbXml::ActionType, int>::iterator it = hitList.begin();
    while (it != hitList.end()) {
        qDebug() << "-- Action:" << it.key() << ", hits:" << it.value();
        it++;
    }
#endif
    return result;
}

bool HbXmlLoaderBinarySyntax::validateDocument()
{
    QByteArray bytes = mIn.device()->read( strlen(HbXmlLoaderBinarySyntax::signature()) );
    if ( strcmp( bytes.constData(), HbXmlLoaderBinarySyntax::signature() )) {
        qWarning() << "Not a binary file.";
        return false;
    }

    bool supportedVersion(true);
    qint8 major, minor;
    mIn >> major >> minor;
    if ( ( major > VERSION_MAJOR ) || ( major < MIN_SUPPORTED_VERSION_MAJOR ) ) {
        supportedVersion = false;
    } else if( ( ( major == VERSION_MAJOR ) && ( minor > VERSION_MINOR )  ) ||
               ( ( major == MIN_SUPPORTED_VERSION_MAJOR ) && ( minor < MIN_SUPPORTED_VERSION_MINOR ) ) ) {
        supportedVersion = false;
    }
    if (!supportedVersion) {
        QString ver_str;
        ver_str.append(QString::number(major));
        ver_str.append('.');
        ver_str.append(QString::number(minor));
        qWarning() << "Not supported document version " + ver_str + ". Current parser version is: " + version();
        return false;
    }
    return true;
}


bool HbXmlLoaderBinarySyntax::parseReset()
{
    mActions->reset();
    return true;
}

bool HbXmlLoaderBinarySyntax::parseCleanUp()
{
    mActions->cleanUp();
    return true;
}

bool HbXmlLoaderBinarySyntax::parseDeleteAll()
{
    mActions->deleteAll();
    return true;
}

bool HbXmlLoaderBinarySyntax::parsePushDocument()
{
    QString context;
    mIn >> context;
    return mActions->pushDocument(context);
}

bool HbXmlLoaderBinarySyntax::parsePushObject()
{
    QString type, name;
    mIn >> type >> name;
    return mActions->pushObject(type, name);
}

bool HbXmlLoaderBinarySyntax::parsePushWidget()
{
    QString type, name, role, plugin;
    mIn >> type >> name >> role >> plugin;
    return mActions->pushWidget(type, name, role, plugin);
}

bool HbXmlLoaderBinarySyntax::parsePushConnect()
{
    QString srcName, signalName, dstName, slotName;
    mIn >> srcName >> signalName >> dstName >> slotName;
    return mActions->pushConnect(srcName, signalName, dstName, slotName);
}

bool HbXmlLoaderBinarySyntax::parsePushProperty()
{
    char *propertyName;
    HbXmlVariable buffer;
    mIn >> propertyName >> buffer;
    bool res = mActions->pushProperty(propertyName, buffer);
    if ( !res ) {
        qDebug() << "HbXmlLoaderBinarySyntax, failed at pushProperty " << propertyName;
    }
    delete[] propertyName;
    return res;
}

bool HbXmlLoaderBinarySyntax::parsePushRef()
{
    QString name, role;
    mIn >> name >> role;
    return mActions->pushRef(name, role);
}

bool HbXmlLoaderBinarySyntax::parsePushContainer()
{
    char *propertyName;
    quint8 type, count;
    QList<HbXmlVariable*> container;
    mIn >> propertyName >> type >> count;
    for (int i=0; i < count; i++) {
        HbXmlVariable *variable = new HbXmlVariable();
        mIn >> *variable;
        container.append(variable);
    }

    bool res =  mActions->pushContainer(propertyName, (HbXmlLoaderAbstractSyntax::DocumentLexems)type, container);

    delete[] propertyName;
    qDeleteAll(container);

    return res;
}

bool HbXmlLoaderBinarySyntax::parsePop()
{
    quint8 type;
    mIn >> type;
    return mActions->pop((HbXml::ElementType)type);
}

bool HbXmlLoaderBinarySyntax::parseSetContentsMargins()
{
    HbXmlLengthValue left, top, right, bottom;
    mIn >> left >> top >> right >> bottom;
    return mActions->setContentsMargins(left, top, right, bottom);
}

bool HbXmlLoaderBinarySyntax::parseSetSizePolicy()
{
    QSizePolicy::Policy horizontalPolicy, verticalPolicy;
    QSizePolicy::Policy *horizontalPolicy_p = 0, *verticalPolicy_p = 0;
    int horizontalStretch, verticalStretch;
    int *horizontalStretch_p = 0, *verticalStretch_p = 0;

    bool temp;
    qint16 tempInt;
    quint8 tempEnum;
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        horizontalPolicy = (QSizePolicy::Policy)tempEnum;
        horizontalPolicy_p = &horizontalPolicy;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        verticalPolicy = (QSizePolicy::Policy)tempEnum;
        verticalPolicy_p = &verticalPolicy;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        horizontalStretch = (int)tempInt;
        horizontalStretch_p = &horizontalStretch;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        verticalStretch = (int)tempInt;
        verticalStretch_p = &verticalStretch;
    }
    return mActions->setSizePolicy(horizontalPolicy_p, verticalPolicy_p, horizontalStretch_p, verticalStretch_p);
}

bool HbXmlLoaderBinarySyntax::parseSetSizeHint()
{
    quint8 hint;
    HbXmlLengthValue hintWidth, hintHeight;
    bool fixed;
    mIn >> hint >> hintWidth >> hintHeight >> fixed;
    return mActions->setSizeHint((Qt::SizeHint)hint, hintWidth, hintHeight, fixed);
}

bool HbXmlLoaderBinarySyntax::parseSetToolTip()
{
    HbXmlVariable tooltip;
    mIn >> tooltip;
    return mActions->setToolTip(tooltip);
}

bool HbXmlLoaderBinarySyntax::parseSetBackground()
{
    QString name;
    quint8 type;
    mIn >> name >> type;
    return mActions->setBackground(name, (HbFrameDrawer::FrameType)type);
}

bool HbXmlLoaderBinarySyntax::parseCreateAnchorLayout()
{
    QString widget;
    bool modify;
    mIn >> widget >> modify;
    return mActions->createAnchorLayout(widget, modify);
}

bool HbXmlLoaderBinarySyntax::parseAddAnchorLayoutItem()
{
    QString src, srcId, dst, dstId, anchorId;
    Hb::Edge srcEdge, dstEdge;
    HbXmlLengthValue minLength, prefLength, maxLength;
    QSizePolicy::Policy policy; 
    QSizePolicy::Policy *policy_p = 0;
    HbAnchor::Direction dir;
    HbAnchor::Direction *dir_p = 0;

    bool temp;
    quint8 tempEnum;

    mIn >> src >> srcId >> tempEnum;
    srcEdge = (Hb::Edge)tempEnum;

    mIn >> dst >> dstId >> tempEnum;
    dstEdge = (Hb::Edge)tempEnum;

    mIn >> minLength >> prefLength >> maxLength;

    // Optional parameters
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        policy = (QSizePolicy::Policy)tempEnum;
        policy_p = &policy;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        dir = (HbAnchor::Direction)tempEnum;
        dir_p = &dir;
    }
    mIn >> anchorId;

    return mActions->addAnchorLayoutItem( src, srcId, srcEdge, dst, dstId, dstEdge, minLength, prefLength, maxLength, policy_p, dir_p, anchorId );
}

bool HbXmlLoaderBinarySyntax::parseSetAnchorLayoutMapping()
{
    QString item, id;
    bool remove;
    mIn >> item >> id >> remove;
    return mActions->setAnchorLayoutMapping(item, id, remove);
}

bool HbXmlLoaderBinarySyntax::parseCreateGridLayout()
{
    QString widget;
    HbXmlLengthValue spacing;
    bool modify;
    mIn >> widget >> spacing >> modify;
    return mActions->createGridLayout(widget, spacing, modify);
}

bool HbXmlLoaderBinarySyntax::parseAddGridLayoutCell()
{
    QString src;
    int row, column, rowspan, columnspan;
    int *rowspan_p = 0, *columnspan_p = 0;
    Qt::Alignment alignment;
    Qt::Alignment *alignment_p = 0;
    mIn >> src;

    bool temp;
    qint16 tempInt;
    quint8 tempEnum;

    mIn >> tempInt;
    row = (int)tempInt;

    mIn >> tempInt;
    column = (int)tempInt;

    // Optional parameters
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        rowspan = (int)tempInt;
        rowspan_p = &rowspan;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        columnspan = (int)tempInt;
        columnspan_p = &columnspan;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        alignment = (Qt::Alignment)tempEnum;
        alignment_p = &alignment;
    }
    return mActions->addGridLayoutCell(src, row, column, rowspan_p, columnspan_p, alignment_p);
}

bool HbXmlLoaderBinarySyntax::parseSetGridLayoutRowProperties()
{
    int row, rowStretchFactor;
    int *rowStretchFactor_p = 0;
    Qt::Alignment alignment;
    Qt::Alignment *alignment_p = 0;

    bool temp;
    qint16 tempInt;
    quint8 tempEnum;

    mIn >> tempInt;
    row = (int)tempInt;

    // Optional parameters
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        rowStretchFactor = (int)tempInt;
        rowStretchFactor_p = &rowStretchFactor;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        alignment = (Qt::Alignment)tempEnum;
        alignment_p = &alignment;
    }

    return mActions->setGridLayoutRowProperties(row, rowStretchFactor_p, alignment_p);
}

bool HbXmlLoaderBinarySyntax::parseSetGridLayoutColumnProperties()
{
    int column, columnStretchFactor;
    int *columnStretchFactor_p = 0;
    Qt::Alignment alignment;
    Qt::Alignment *alignment_p = 0;

    bool temp;
    qint16 tempInt;
    quint8 tempEnum;

    mIn >> tempInt;
    column = (int)tempInt;

    // Optional parameters
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        columnStretchFactor = (int)tempInt;
        columnStretchFactor_p = &columnStretchFactor;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        alignment = (Qt::Alignment)tempEnum;
        alignment_p = &alignment;
    }
    return mActions->setGridLayoutColumnProperties(column, columnStretchFactor_p, alignment_p);
}

bool HbXmlLoaderBinarySyntax::parseSetGridLayoutRowHeights()
{
    qint16 tempInt;
    int row;
    HbXmlLengthValue minHeight, maxHeight, prefHeight, fixedHeight, rowSpacing;
    mIn >> tempInt >> minHeight >> maxHeight >> prefHeight >> fixedHeight >> rowSpacing;
    row = (int)tempInt;
    return mActions->setGridLayoutRowHeights(row, minHeight, maxHeight, prefHeight, fixedHeight, rowSpacing);
}

bool HbXmlLoaderBinarySyntax::parseSetGridLayoutColumnWidths()
{
    qint16 tempInt;
    int column;
    HbXmlLengthValue minWidth, maxWidth, prefWidth, fixedWidth, columnSpacing;
    mIn >> tempInt >> minWidth >> maxWidth >> prefWidth >> fixedWidth >> columnSpacing;
    column = (int)tempInt;
    return mActions->setGridLayoutColumnWidths(column, minWidth, maxWidth, prefWidth, fixedWidth, columnSpacing);
}

bool HbXmlLoaderBinarySyntax::parseCreateLinearLayout()
{
    QString widget;
    Qt::Orientation orientation;
    Qt::Orientation *orientation_p = 0;
    HbXmlLengthValue spacing;
    bool modify;
    mIn >> widget;

    // Optional parameter
    bool temp;
    quint8 tempEnum;
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        orientation = (Qt::Orientation)tempEnum;
        orientation_p = &orientation;
    }
    mIn >> spacing >> modify;

    return mActions->createLinearLayout(widget, orientation_p, spacing, modify);
}

bool HbXmlLoaderBinarySyntax::parseAddLinearLayoutItem()
{
    QString itemname;
    int index, stretchfactor;
    int *index_p = 0, *stretchfactor_p = 0;
    Qt::Alignment alignment;
    Qt::Alignment *alignment_p = 0;
    HbXmlLengthValue spacing;
    mIn >> itemname;

    // Optional parameters
    bool temp;
    qint16 tempInt;
    quint8 tempEnum;
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        index = (int)tempInt;
        index_p = &index;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        stretchfactor = (int)tempInt;
        stretchfactor_p = &stretchfactor;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempEnum;
        alignment = (Qt::Alignment)tempEnum;
        alignment_p = &alignment;
    }
    mIn >> spacing;

    return mActions->addLinearLayoutItem(itemname, index_p, stretchfactor_p, alignment_p, spacing);
}

bool HbXmlLoaderBinarySyntax::parseAddLinearLayoutStretch()
{
    int index, stretchfactor;
    int *index_p = 0, *stretchfactor_p = 0;

    // Optional parameters
    bool temp;
    qint16 tempInt;
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        index = (int)tempInt;
        index_p = &index;
    }
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        stretchfactor = (int)tempInt;
        stretchfactor_p = &stretchfactor;
    }

    return mActions->addLinearLayoutStretch(index_p, stretchfactor_p);
}

bool HbXmlLoaderBinarySyntax::parseSetLayoutContentsMargins()
{
    HbXmlLengthValue left, top, right, bottom;
    mIn >> left >> top >> right >> bottom;
    return mActions->setLayoutContentsMargins(left, top, right, bottom);
}

bool HbXmlLoaderBinarySyntax::parseCreateStackedLayout()
{
    QString widget;
    bool modify;
    mIn >> widget >> modify;
    return mActions->createStackedLayout(widget, modify);
}

bool HbXmlLoaderBinarySyntax::parseAddStackedLayoutItem()
{
    QString itemname;
    int index;
    int *index_p = 0;

    mIn >> itemname;

    // Optional parameters
    bool temp;
    qint16 tempInt;
    mIn >> temp;
    if ( temp ) {
        mIn >> tempInt;
        index = (int)tempInt;
        index_p = &index;
    }
    return mActions->addStackedLayoutItem(itemname, index_p);
}

bool HbXmlLoaderBinarySyntax::parseCreateNullLayout()
{
    QString widget;
    mIn >> widget;
    return mActions->createNullLayout(widget);
}
