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

#include <QMetaEnum>

#include "hbactionmanager_p.h"
#include "hbviewactionmanager_p.h"
#include "hbactionmanagerxmlparser_p.h"

/*
    @proto
    @hbcore
    \class HbActionManagerXmlParser
    \brief HbActionManagerXmlParser is the class that is responsible of parsing
    xml file containing UI commands distribution guide or container template.

    \internal
*/

/*! Constructor.

    The action manager object that will receive the data read must be given as a
    parameter. The ownership of store is not transferred and store must be
    valid during the entire lifetime of this object.
*/
HbActionManagerXmlParser::HbActionManagerXmlParser(HbActionManager *actionManager)
    : actionManager(actionManager)
{

}

/*! Constructor.

    The action manager object that will receive the data read must be given as a
    parameter. The ownership of store is not transferred and store must be
    valid during the entire lifetime of this object.
*/
HbActionManagerXmlParser::HbActionManagerXmlParser(HbViewActionManager *viewActionManager)
    : viewActionManager(viewActionManager)
{

}

/*! Destructor.
*/
HbActionManagerXmlParser::~HbActionManagerXmlParser()
{

}

/*! Reads a well-formed distribution template or guide XML document 
    from the given QIODevice and adds the data found to the 
    action manager associated with this object.

    If no error occurred, returns true; otherwise returns false.
*/
bool HbActionManagerXmlParser::read(QIODevice *device)
{
    setDevice(device);

    while (!atEnd()) {
        readNext();

        if (isStartElement()) {
            if (name() == "distributiontemplate") {
                if (!actionManager) {
                    raiseError("HbActionManagerXmlParser::read No actionmanager object assigned.");
                    return false;
                }
                readTemplate();
            }
            else if (name() == "distributionguide") {
                if (!viewActionManager) {
                    raiseError("HbActionManagerXmlParser::read No actionmanager object assigned.");
                    return false;
                }
                readGuide();
            }
            else {
                raiseError("HbActionManagerXmlParser::read The document is not a action template.");
            }
        }

    }
    return !error();
}

/*
    Reads UI commands distribution template.
*/
void HbActionManagerXmlParser::readTemplate()
{
    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            break;
        }

        if (isStartElement()) {
            if (name() == "action") {
                QXmlStreamAttributes attrs = attributes();
                QString menuRoleString = attrs.value("commandrole").toString();
                actionManager->addItem(
                    enumValue(HbAction::staticMetaObject, "commandRole", menuRoleString));
                readElementText();
            }    
        }
    }
}

/*
    Reads UI commands distribution guide.
*/
void HbActionManagerXmlParser::readGuide()
{
    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            break;
        }

        if (isStartElement()) {
            if (name() == "toolbar") {
                QXmlStreamAttributes attrs = attributes();
                int maxCount = attrs.value("maxcount").toString().toInt();
                viewActionManager->setToolBarmaxCount(maxCount);
                readElementText();
            }
            if (name() == "default") {
                QXmlStreamAttributes attrs = attributes();
                QString container = attrs.value("container").toString();
                viewActionManager->setDefaultContainer(container);
                readElementText();
            }
            if (name() == "distributionitem") {
                readDistributionItem();
            }    
        }
    }
}

/*
    Reads UI commands distribution guide item.
*/
void HbActionManagerXmlParser::readDistributionItem()
{
    HbViewActionManager::GuideItem guideItem;
    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            break;
        }

        if (isStartElement()) {
            if (name() == "action") {
                QXmlStreamAttributes attrs = attributes();
                QString menuRoleString = attrs.value("commandrole").toString();
                HbActionManager::TemplateItem item(
                    enumValue(HbAction::staticMetaObject, "commandRole", menuRoleString));
                guideItem.setTemplateItem(item);
                readElementText();
            }
            else if (name() == "placement") {
                QXmlStreamAttributes attrs = attributes();
                QString containerString = attrs.value("container").toString();
                HbViewActionManager::Placement placement(containerString);
                guideItem.addPlacement(placement);
                readElementText();
            }
        }
    }
    viewActionManager->addGuideItem(guideItem);
}

HbAction::CommandRole HbActionManagerXmlParser::enumValue(const QMetaObject &metaObject, 
    const char* propertyName, const QString &enumKeyString) const
{
    int roleEnumValue = -1;
    const int propertyIndex = metaObject.indexOfProperty(propertyName);

    if (propertyIndex >= 0) {
        const QMetaProperty metaProperty = metaObject.property(propertyIndex);

        if (metaProperty.isEnumType()) {
            const QMetaEnum metaEnum = metaProperty.enumerator();
            const QByteArray enumKeyByteArray = enumKeyString.toAscii();
            const char* enumKey = enumKeyByteArray.constData();

            roleEnumValue = metaEnum.keyToValue(enumKey);
        }
    }
    HbAction::CommandRole role(HbAction::NoRole);
    if (roleEnumValue >= 0) {
        role = HbAction::CommandRole(roleEnumValue);          
    }
    return role;
}
