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

#include "hbiconimplcreator_p.h"
#include "hbpixmapiconimpl_p.h"
#ifdef HB_NVG_CS_ICON
#include "hbnvgiconimpl_p.h"
#endif
#ifdef HB_SGIMAGE_ICON
#include "hbsgimageiconimpl_p.h"
#endif

/*!
    \class HbIconImplCreator

    \brief HbIconImplCreator class is a simple utility class to serve as a factory
    \      for different types of iconImpl, based on iconinfo's type.

    \internal
*/
HbIconImpl *HbIconImplCreator::createIconImpl(HbSharedIconInfo& iconImplInfo,
                                              HbIconLoadingParams &params )
{
    HbIconImpl* iconImpl = 0;
    switch(iconImplInfo.type){
        case NVG:
#ifdef HB_NVG_CS_ICON
            iconImpl = new HbNvgIconImpl(iconImplInfo,
                                         params.iconFileName,
                                         params.size,
                                         params.aspectRatioMode,
                                         params.mode,
                                         (params.mirrored && !params.mirroredIconFound));
#endif
            break;
        case SGIMAGE:
#ifdef HB_SGIMAGE_ICON
            iconImpl = new HbSgimageIconImpl(iconImplInfo,
                                            params.iconFileName,
                                            params.size,
                                            params.aspectRatioMode,
                                            params.mode,
                                            (params.mirrored && !params.mirroredIconFound));
#endif
            break;
        case OTHER_SUPPORTED_FORMATS:
            iconImpl = new HbPixmapIconImpl(iconImplInfo,
                                            params.iconFileName,
                                            params.size,
                                            params.aspectRatioMode,
                                            params.mode,
                                            (params.mirrored && !params.mirroredIconFound));
            break;
        default:
            break;
    }
    
	if (iconImpl && params.color.isValid()){
		iconImpl->setColor(params.color);
	}

    return iconImpl;
}
