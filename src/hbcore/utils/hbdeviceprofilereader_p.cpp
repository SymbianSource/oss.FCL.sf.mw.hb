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

#include "hbdeviceprofilereader_p.h"
#include "hbdeviceprofiledatabase_p.h"

#define FROM_INCHES_TO_MM 25.4
/*
    \class HbDeviceProfileReader

    \brief The HbDeviceProfileReader class implements a stream parser
    that reads well-formed XML documents containing display definitions.

    \warning This class is a part of internal library implementation and may
    be removed from the public API!

    \internal
    \proto
*/

/*! Constructor.
*/
HbDeviceProfileReader::HbDeviceProfileReader(HbDeviceProfileList *profileList,
                                             HbMemoryManager::MemoryType type
                                             )
    : QXmlStreamReader(), mDeviceProfileList(profileList),mType(type)
{
}

/*! Destructor.
*/
HbDeviceProfileReader::~HbDeviceProfileReader()
{
}

/*! Reads a well-formed display definitions XML document from the given QIODevice and
    adds the metrics found to the device profile list.

    If no error occurred, returns true; otherwise returns false.
*/
bool HbDeviceProfileReader::read(QIODevice *device)
{
    if (!mDeviceProfileList) {
        raiseError("HbDeviceProfileReader::read No device profile object assigned.");
        return false;
    }

    setDevice(device);

    while (!atEnd()) {
        readNext();

        if (isStartElement()) {
            if ( name() == "displayDefinition" ) {
                readDisplayDefinitions();
            } else {
                raiseError(
                    "HbDeviceProfileReader::read The document is not a display definition document.");
            }
        }
    }

    return !error();
}

/*! Reads a well-formed display definitions XML document and
    adds the metrics found to the device profile list.
*/

void HbDeviceProfileReader::readDisplayDefinitions()
{
    Q_ASSERT(isStartElement() && name() == "displayDefinition");
    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            continue;
        }   

        if (isStartElement()) {
            if (name() == "display")
                readAttributes();
            else
                readUnknownElement();
        }
    }
}

/*! Parses attributes from the display definition and sets values to the device profile.

*/

void HbDeviceProfileReader::readAttributes()
    {
    Q_ASSERT(isStartElement() && name() == "display");
    QXmlStreamAttributes attrs = attributes();
 
    DeviceProfile deviceProfile(mType);
    deviceProfile.mName=attrs.value("name").toString();
    const QSize logicalSize(attrs.value("resolutionWidth").toString().toInt(),
                          attrs.value("resolutionHeight").toString().toInt());
    deviceProfile.mLogicalSize.setWidth(logicalSize.width());
    deviceProfile.mLogicalSize.setHeight(logicalSize.height());
    const qreal ppiValue(attrs.value("ppiValue").toString().toFloat());
    if(ppiValue) {
        deviceProfile.mPhysicalSize = (QSizeF((logicalSize.width() / ppiValue) * FROM_INCHES_TO_MM,
                                         (logicalSize.height() / ppiValue * FROM_INCHES_TO_MM)) );
    } else {
        deviceProfile.mPhysicalSize = QSizeF(0,0);
    }
    deviceProfile.mOrientation = (logicalSize.width() <= logicalSize.height() ?
                                  Qt::Vertical : Qt::Horizontal);
    deviceProfile.mAltName = attrs.value("alternateProfileName").toString();
    deviceProfile.mUnitValue = attrs.value("unitValue").toString().toFloat();
    deviceProfile.mOrientationAngle= attrs.value("orientationAngle").toString().toInt();

    bool defaultMode = false;
    defaultMode = attrs.value("defaultMode").toString() == "true";
    
    if (defaultMode) {
        mDeviceProfileList->insert(0, deviceProfile);
    }
    else {
        mDeviceProfileList->append(deviceProfile);
    }
}

void HbDeviceProfileReader::readUnknownElement()
{
    Q_ASSERT(isStartElement());

    while (!atEnd()) {
        readNext();

        if (isEndElement()) {
            break;
        }

        if (isStartElement()) {
            readUnknownElement();
        }
    }
}

