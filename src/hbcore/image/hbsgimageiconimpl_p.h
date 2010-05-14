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

#ifndef HBSGIMAGEICONIMPL_P_H
#define HBSGIMAGEICONIMPL_P_H

#include "hbiconimpl_p.h"
#include <sgresource/sgimage.h>
#include <VG/openvg.h>

#include <QBitmap>

class HbEglStates;
class HbVgImageIconRenderer;
class HbPixmapIconRenderer;

class HbSgimageIconImpl : public HbIconImpl
{
public :
    HbSgimageIconImpl(const HbSharedIconInfo &iconData,
                      const QString& name,
                      const QSizeF& keySize,
                      Qt::AspectRatioMode aspectRatioMode,
                      QIcon::Mode mode,
                      bool mirrored,
                      HbRenderingMode renderMode);

    ~HbSgimageIconImpl();
    QPixmap pixmap();
    void paint(QPainter* painter,
               const QRectF &rect,
               Qt::Alignment alignment,
               const QPainterPath &clipPath = QPainterPath(),
               HbMaskableIconImpl * maskIconData = 0);

    QSize defaultSize() const;
    QSize size();
    void destroyMaskedData(HbIconMaskedData *data);

private :
    void retrieveSgImageData();
    VGImage getVgImageFromSgImage();
    QPointF setAlignment(const QRectF& rect,
                         QSizeF& renderSize,
                         Qt::Alignment alignment);
    void updatePainterTransformation(QPainter * painter, const QPointF & pos);
    static VGImage getVgImage(HbIconImpl * impl, QPainter * painter);


private:
    TSgDrawableId sgImageId;
    QPixmap currentPixmap;
    QSize contentSize;
    HbVgImageIconRenderer * vgImageRenderer;
    HbPixmapIconRenderer * pixmapIconRenderer;
};

#endif // HBSGIMAGEICONIMPL_P_H

