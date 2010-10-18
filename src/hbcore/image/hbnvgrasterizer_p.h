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

#ifndef HBNVGRASTERIZER_P_H_
#define HBNVGRASTERIZER_P_H_


#include <QImage>
#include <hbglobal.h>
class CNvgEngine;

class HB_CORE_PRIVATE_EXPORT HbNvgRasterizer
{
public :
    HbNvgRasterizer();
    ~HbNvgRasterizer();

    static HbNvgRasterizer* global();

    bool rasterize(const QByteArray &nvgData,
                   const QSizeF &renderSize,
                   Qt::AspectRatioMode aspectRatioMode,
                   void * destination, int stride,
                   QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied);

    bool rasterize(const QByteArray &nvgData,
                       const QSizeF &renderSize,
                       Qt::AspectRatioMode aspectRatioMode,
                       QImage & destination,
                       QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied);

    bool initialize(int width, int height);
    bool terminate();

private:
    bool vgiSymbianInit;
    CNvgEngine * nvgEngine;
};

#endif // end of HBNVGRASTERIZER_P_H_

