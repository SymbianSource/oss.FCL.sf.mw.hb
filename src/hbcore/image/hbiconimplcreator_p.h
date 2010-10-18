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

#ifndef HBICONIMPLCREATOR_P_H
#define HBICONIMPLCREATOR_P_H

#include "hbiconimpl_p.h"
#include "hbiconloader_p.h"

struct HbIconLoadingParams {
    // Parameters given to LoadIcon function
    QString iconName;
    HbIconLoader::Purpose purpose;
    QSizeF size;
    Qt::AspectRatioMode aspectRatioMode;
    QIcon::Mode mode;
    HbIconLoader::IconLoaderOptions options;
    HbIconAnimator *animator;
    QColor color;
    // Whether icon's default size should be used
    bool isDefaultSize;
    // Whether the icon should be horizontally mirrored
    bool mirrored;
    // Whether mirrored version of the icon was found in the file system (otherwise it's mirrored by code).
    bool mirroredIconFound;
    // Whether the loaded pixmap can be cached
    bool canCache;
    // Whether HbIconAnimation object has been created
    bool animationCreated;
    // Whether mirroring has been handled
    bool mirroringHandled;
    // Whether mode has been handled
    bool modeHandled;
    // Icon name without extension
    QString cleanIconName;
    // Resolved filename for the icon
    QString iconFileName;
    // Image where the icon is rasterized
    QImage image; // cannot use QPixmap, must support non-gui threads too
    // Icon's rendering mode ( Hardware / Software )
    HbRenderingMode renderMode;
};

Q_DECLARE_METATYPE(HbIconLoadingParams)

class HB_AUTOTEST_EXPORT HbIconImplCreator
{
public:
    static HbIconImpl *createIconImpl(HbSharedIconInfo &iconImplInfo,
                                      HbIconLoadingParams &params);

    static HbIconImpl *createIconImpl(const QPixmap &pixmap,
                                      HbIconLoadingParams &params);
};

#endif  //HBICONIMPLCREATOR_P_H
