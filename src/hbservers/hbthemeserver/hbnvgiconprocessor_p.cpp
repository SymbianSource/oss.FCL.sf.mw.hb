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

#include "hbnvgiconprocessor_p.h"
#include "hbnvg_p.h"
#include <QFile>
#include "hbiconsource_p.h"
#include "hbmemoryutils_p.h"
#include "hbthemeserverutils_p.h"


/*!
    @hbserver
    \class HbNvgIconProcessor
    \brief HbNvgIconProcessor is responsible for retrieving the Nvg vector data from the NVG icon file,
    and copy the icon data to the cpu shared memory.

*/


/*!
    \fn HbNvgIconProcessor::HbNvgIconProcessor()
    Constructor
    \a key denotes the unique identifier for the cache item
    \a options indicate different ways of loading icons
    \a format indicates the icon format e.g. svg/nvg etc.
 */
HbNvgIconProcessor::HbNvgIconProcessor(
    const HbIconKey &key,
    const HbIconLoader::IconLoaderOptions &options,
    const QString &type)
{
    iconKey = key;
    iconOptions = options;
    iconType = type;
    defaultSize = QSize(0, 0);
    size = QSize(0, 0);
}


/*!
    \fn HbNvgIconProcessor::~HbNvgIconProcessor()
    Destructor
 */
HbNvgIconProcessor::~HbNvgIconProcessor()
{
}


/*!
    \fn HbNvgIconProcessor::createNvgData()
    Retrieves the NVG data from the file
    \a iconPath the path to the icon file
 */
bool HbNvgIconProcessor::createIconData(const QString& iconPath)
{
    if (!iconKey.size.isValid()) {
        return false;
    }
    bool isDefaultSize =  iconKey.size.isNull();
    
    HbIconSource *source = HbThemeServerUtils::getIconSource(iconPath);
    QByteArray *sourceByteArray = source->byteArray();
    if( !sourceByteArray ) {
        return false;
    }
    byteArray = *sourceByteArray;
    QSizeF renderSize = source->defaultSize();
    defaultSize = renderSize.toSize();
    if (!isDefaultSize) {
        renderSize.scale(iconKey.size, iconKey.aspectRatioMode);
    }
    size = renderSize.toSize();
    
    return true;
}

/*!
    \fn HbNvgIconProcessor::sharedIconDataCost()
    Returns the size of the Nvg icon data in bytes
 */
int HbNvgIconProcessor::sharedIconDataCost() const
{
    int size = byteArray.size();
    return size;
}

/*!
    \fn HbNvgIconProcessor::sharedIconData()
    Copies the Nvg vector data to the shared memory
 */
HbSharedIconInfo HbNvgIconProcessor::sharedIconData()const
{
    HbSharedIconInfo data;
    data.type = INVALID_FORMAT;
    if (!byteArray.isNull()) {
        GET_MEMORY_MANAGER(HbMemoryManager::SharedMemory);
        try {
            //alloc function of memory manager can throw
            data.nvgData.offset =  manager->alloc(byteArray.size());
            data.nvgData.defaultWidth = defaultSize.width();
            data.nvgData.defaultHeight = defaultSize.height();
            data.nvgData.width = size.width();
            data.nvgData.height = size.height();
            data.type = NVG;
            data.nvgData.dataSize = byteArray.size();
            memcpy(HbMemoryUtils::getAddress<char>(HbMemoryManager::SharedMemory, data.nvgData.offset),
                   byteArray.data(), byteArray.size());
        }

        catch (std::bad_alloc &badAlloc) {
            Q_UNUSED(badAlloc)
        }
    }
    return data;
}
