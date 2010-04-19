/****************************************************************************
**
** Copyright (C) 2008-2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (developer.feedback@nokia.com)
**
** This file is part of the HbServers module of the UI Extensions for Mobile.
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

#include "hbpiciconprocessor_p.h"

#include <QtSvg/QSvgRenderer>
#include <QPainter>

#include "hbmemoryutils_p.h"
#include "hbiconsource_p.h"
#include "hbthemeserverutils_p.h"

/*!
    @hbserver
    \class HbPicIconProcessor
    \brief HbPicIconProcessor is responsible for retrieving the PIC vector data from the PIC icon file,
    and copy the icon data to the cpu shared memory.

*/


/*!
    \fn HbPicIconProcessor::HbPicIconProcessor()
    Constructor
    \a key denotes the unique identifier for the cache item
    \a options indicate different ways of loading icons
    \a format indicates the icon format e.g. svg/nvg etc.
 */

HbPicIconProcessor::HbPicIconProcessor(
    const HbIconKey &key,
    const HbIconLoader::IconLoaderOptions &options,
    const QString &type)
{
    iconKey = key;
    iconOptions = options;
    iconType = type;
    defaultSize = QSize(0, 0);
}


/*!
    \fn HbPicIconProcessor::~HbPicIconProcessor()
    Destructor
 */
HbPicIconProcessor::~HbPicIconProcessor()
{
}

/*!
    \fn HbPicIconProcessor::createPictureData()
    Retrieves the PIC data from the file
    \a iconPath the path to the icon file
 */
bool HbPicIconProcessor::createIconData(const QString& iconPath)
{
    bool isIconCreated = false;
    if (!iconKey.size.isValid()) {
        return false;
    }

    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    if (!source) {
        return false;
    }

    if (iconType == "SVG") {
        QSvgRenderer *svgRenderer = source->svgRenderer();
        if (svgRenderer && svgRenderer->isValid()) {
            QSizeF renderSize = QSizeF(svgRenderer->defaultSize());
            //Save the default size
            defaultSize = renderSize.toSize();
            QPainter painter;
            picture = QPicture();
            painter.begin(&picture);
            svgRenderer->render(&painter);
            painter.end();
            isIconCreated = true;
        }
        source->releaseSvgRenderer();
    } else {
        picture = *source->picture();
        QSizeF picSize = QSizeF(picture.boundingRect().size());
        //Save the default size
        defaultSize = picSize.toSize();
        //The icon is successfully created if the picSize returns not Null;
        if (!picSize.isNull()) {
            isIconCreated = true;
        }
    }
    return isIconCreated;
}

/*!
    \fn HbPicIconProcessor::sharedIconDataCost()
    Returns the size of the PIC icon data in bytes
 */
int HbPicIconProcessor::sharedIconDataCost() const
{
    int size = picture.size();
    return size;
}

/*!
    \fn HbPicIconProcessor::sharedIconData()
    Copies the PIC vector data to the shared memory
 */
HbSharedIconInfo HbPicIconProcessor::sharedIconData()const
{
    HbSharedIconInfo data;
    data.type = INVALID_FORMAT;
    if (!picture.isNull()) {
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        try {
            //alloc function of memory manager can throw
            data.picData.offset =  manager->alloc(picture.size());
            data.type = PIC;
            data.picData.dataSize = picture.size();
            data.picData.defaultWidth = defaultSize.width();
            data.picData.defaultHeight = defaultSize.height();
            memcpy(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, data.picData.offset),
                   picture.data(), picture.size());
        } catch (std::bad_alloc &badAlloc) {
            Q_UNUSED(badAlloc)
        }
    }
    return data;
}
